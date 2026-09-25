/* ── Código de máquina x86-64, gerado na primeira execução de cada proto ──
 *
 * Incluído UMA vez por jinga_vm.c, antes do laço da VM: usa os tipos e as
 * constantes de lá (VM, Proto, Value, os V_*, os TIPO_*, OBJ_STRING).
 *
 * O DESENHO, numa frase: o código nativo faz em linha o que é quente e
 * garantido — carregar/guardar, int com int, comparação+desvio, `n++`,
 * `range` — e pra QUALQUER outra coisa (opcode sem gabarito, operando que
 * não é int, estouro, global não definido, coleta de lixo devida) ele SAI
 * pro interpretador, que executa exatamente aquela instrução com o tratador
 * de sempre e volta pro nativo na instrução seguinte (`entra_nativo` e
 * `L_uma` no laço da VM). Consequências:
 *   - nenhuma regra é duplicada: erro, chamada, retorno, try, gerador,
 *     fibra, import, depurador — tudo continua sendo do interpretador, com
 *     as mesmas frases;
 *   - todo caminho rápido CONFERE antes de MEXER: quando desiste, a pilha
 *     está exatamente como o interpretador espera pra refazer a instrução;
 *   - o estado que o nativo carrega é só o topo da pilha (rbx) e, ao sair,
 *     o `ip` da instrução em que parou (já com o +2, como o interpretador
 *     conta);
 *   - a compilação é PREGUIÇOSA: um proto vira código de máquina na
 *     primeira vez que roda. Funct nunca chamada não custa nada.
 *
 * Registradores (System V): r12 = vm, r13 = base da pilha de valores,
 * r14 = locals + lbase, r15 = JitCtx, rbx = próximo slot LIVRE da pilha
 * (ponteiro, não índice). Value tem 16 bytes: tag em +0, união em +8.
 * Nenhuma chamada sai do código nativo: a pilha C fica como está, então
 * uma troca de fibra ou um import nunca acontece com nativo no meio.
 *
 * Desliga: JINGA_JIT=0 (ou `jinga --sem-jit`). JINGA_JIT_LOG=1 diz no
 * stderr cada proto compilado; JINGA_JIT_FALHA=1 simula a recusa do
 * `mmap`/`mprotect` — o programa segue interpretado, nunca aborta. */
#ifndef PS_JIT_X64_H
#define PS_JIT_X64_H

#include <sys/mman.h>
#include <stddef.h>

/* O que o interpretador entrega ao nativo e recebe de volta. */
typedef struct {
    int32_t fp, lbase, sp, locals_top, ip;
} JitCtx;
typedef void (*JitFn)(VM *vm, JitCtx *cx, const void *retoma);

/* um trecho executável por proto compilado; solto no `libera_vm` */
typedef struct JitTrecho {
    void  *mem;
    size_t tam;
    struct JitTrecho *prox;
} JitTrecho;

static int g_jit_ativo = -1;   /* -1 = ainda não leu o ambiente */
static int g_jit_log   = 0;
static int g_jit_falha = 0;

static int jit_ligado(void)
{
    if (g_jit_ativo < 0) {
        const char *e = getenv("JINGA_JIT");
        g_jit_ativo = !(e && e[0] == '0');
        g_jit_log   = getenv("JINGA_JIT_LOG") != NULL;
        g_jit_falha = getenv("JINGA_JIT_FALHA") != NULL;
    }
    return g_jit_ativo;
}

/* ── codificador x86-64 mínimo ──────────────────────────────────────────── */
enum { RAX = 0, RCX, RDX, RBX, RSP, RBP, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15 };
enum { CC_O = 0, CC_NO = 1, CC_B = 2, CC_AE = 3, CC_E = 4, CC_NE = 5, CC_BE = 6, CC_A = 7,
       CC_L = 0xC, CC_GE = 0xD, CC_LE = 0xE, CC_G = 0xF };

typedef struct { uint8_t *b; size_t n, cap; int falhou; } JitBuf;

static void jb_byte(JitBuf *j, uint8_t v)
{
    if (j->n == j->cap) {
        size_t nc = j->cap ? j->cap * 2 : 2048;
        uint8_t *nb = realloc(j->b, nc);
        if (!nb) { j->falhou = 1; return; }
        j->b = nb; j->cap = nc;
    }
    j->b[j->n++] = v;
}
static void jb_u32(JitBuf *j, uint32_t v) { for (int i = 0; i < 4; i++) jb_byte(j, (uint8_t)(v >> (8 * i))); }
static void jb_u64(JitBuf *j, uint64_t v) { for (int i = 0; i < 8; i++) jb_byte(j, (uint8_t)(v >> (8 * i))); }

/* prefixo REX: W = operando de 64 bits, R = bit alto de `reg`, B = de `base` */
static void jb_rex(JitBuf *j, int w, int reg, int base)
{
    uint8_t r = 0x40 | (w ? 8 : 0) | (((reg >> 3) & 1) << 2) | ((base >> 3) & 1);
    if (r != 0x40) jb_byte(j, r);
}
/* ModRM de [base + disp], sempre com deslocamento (mod 01/10): evita o caso
 * especial de rbp/r13 sem disp; rsp/r12 como base pedem o SIB 0x24 */
static void jb_mem(JitBuf *j, int reg, int base, int32_t disp)
{
    int rl = reg & 7, bl = base & 7;
    if (disp >= -128 && disp <= 127) {
        jb_byte(j, (uint8_t)(0x40 | (rl << 3) | bl));
        if (bl == 4) jb_byte(j, 0x24);
        jb_byte(j, (uint8_t)(int8_t)disp);
    } else {
        jb_byte(j, (uint8_t)(0x80 | (rl << 3) | bl));
        if (bl == 4) jb_byte(j, 0x24);
        jb_u32(j, (uint32_t)disp);
    }
}
static void jb_regreg(JitBuf *j, int reg, int rm) { jb_byte(j, (uint8_t)(0xC0 | ((reg & 7) << 3) | (rm & 7))); }

static void x_mov_r64_imm64(JitBuf *j, int reg, uint64_t imm)
{ jb_byte(j, (uint8_t)(0x48 | ((reg >> 3) & 1))); jb_byte(j, (uint8_t)(0xB8 | (reg & 7))); jb_u64(j, imm); }
static void x_mov_r64_mem(JitBuf *j, int reg, int base, int32_t d)  { jb_rex(j, 1, reg, base); jb_byte(j, 0x8B); jb_mem(j, reg, base, d); }
static void x_mov_mem_r64(JitBuf *j, int base, int32_t d, int reg)  { jb_rex(j, 1, reg, base); jb_byte(j, 0x89); jb_mem(j, reg, base, d); }
static void x_mov_r32_mem(JitBuf *j, int reg, int base, int32_t d)  { jb_rex(j, 0, reg, base); jb_byte(j, 0x8B); jb_mem(j, reg, base, d); }
static void x_mov_mem_r32(JitBuf *j, int base, int32_t d, int reg)  { jb_rex(j, 0, reg, base); jb_byte(j, 0x89); jb_mem(j, reg, base, d); }
static void x_mov_mem32_imm(JitBuf *j, int base, int32_t d, int32_t imm) { jb_rex(j, 0, 0, base); jb_byte(j, 0xC7); jb_mem(j, 0, base, d); jb_u32(j, (uint32_t)imm); }
static void x_mov_r64_r64(JitBuf *j, int dst, int src)              { jb_rex(j, 1, src, dst); jb_byte(j, 0x89); jb_regreg(j, src, dst); }
static void x_lea_r64_mem(JitBuf *j, int reg, int base, int32_t d)  { jb_rex(j, 1, reg, base); jb_byte(j, 0x8D); jb_mem(j, reg, base, d); }
static void x_add_r64_imm8(JitBuf *j, int reg, int8_t imm)          { jb_rex(j, 1, 0, reg); jb_byte(j, 0x83); jb_regreg(j, 0, reg); jb_byte(j, (uint8_t)imm); }
static void x_add_r64_r64(JitBuf *j, int dst, int src)              { jb_rex(j, 1, src, dst); jb_byte(j, 0x01); jb_regreg(j, src, dst); }
static void x_sub_r64_r64(JitBuf *j, int dst, int src)              { jb_rex(j, 1, src, dst); jb_byte(j, 0x29); jb_regreg(j, src, dst); }
static void x_add_r64_mem(JitBuf *j, int reg, int base, int32_t d)  { jb_rex(j, 1, reg, base); jb_byte(j, 0x03); jb_mem(j, reg, base, d); }
static void x_sub_r64_mem(JitBuf *j, int reg, int base, int32_t d)  { jb_rex(j, 1, reg, base); jb_byte(j, 0x2B); jb_mem(j, reg, base, d); }
static void x_imul_r64_mem(JitBuf *j, int reg, int base, int32_t d) { jb_rex(j, 1, reg, base); jb_byte(j, 0x0F); jb_byte(j, 0xAF); jb_mem(j, reg, base, d); }
static void x_cmp_r64_mem(JitBuf *j, int reg, int base, int32_t d)  { jb_rex(j, 1, reg, base); jb_byte(j, 0x3B); jb_mem(j, reg, base, d); }
static void x_cmp_mem32_imm8(JitBuf *j, int base, int32_t d, int8_t imm) { jb_rex(j, 0, 0, base); jb_byte(j, 0x83); jb_mem(j, 7, base, d); jb_byte(j, (uint8_t)imm); }
static void x_cmp_mem64_imm8(JitBuf *j, int base, int32_t d, int8_t imm) { jb_rex(j, 1, 0, base); jb_byte(j, 0x83); jb_mem(j, 7, base, d); jb_byte(j, (uint8_t)imm); }
static void x_shl_r64(JitBuf *j, int reg, uint8_t n)                { jb_rex(j, 1, 0, reg); jb_byte(j, 0xC1); jb_regreg(j, 4, reg); jb_byte(j, n); }
static void x_shr_r64(JitBuf *j, int reg, uint8_t n)                { jb_rex(j, 1, 0, reg); jb_byte(j, 0xC1); jb_regreg(j, 5, reg); jb_byte(j, n); }
static void x_setcc_al(JitBuf *j, int cc)                           { jb_byte(j, 0x0F); jb_byte(j, (uint8_t)(0x90 | cc)); jb_byte(j, 0xC0); }
static void x_movzx_eax_al(JitBuf *j)                               { jb_byte(j, 0x0F); jb_byte(j, 0xB6); jb_byte(j, 0xC0); }
static void x_push(JitBuf *j, int reg) { if (reg >= 8) jb_byte(j, 0x41); jb_byte(j, (uint8_t)(0x50 | (reg & 7))); }
static void x_pop(JitBuf *j, int reg)  { if (reg >= 8) jb_byte(j, 0x41); jb_byte(j, (uint8_t)(0x58 | (reg & 7))); }
static void x_ret(JitBuf *j)           { jb_byte(j, 0xC3); }
static void x_ud2(JitBuf *j)           { jb_byte(j, 0x0F); jb_byte(j, 0x0B); }
static void x_jmp_r64(JitBuf *j, int reg) { if (reg >= 8) jb_byte(j, 0x41); jb_byte(j, 0xFF); jb_regreg(j, 4, reg); }
/* saltos com rel32 a preencher: devolvem o offset do campo */
static size_t x_jcc(JitBuf *j, int cc) { jb_byte(j, 0x0F); jb_byte(j, (uint8_t)(0x80 | cc)); jb_u32(j, 0); return j->n - 4; }
static size_t x_jmp(JitBuf *j)         { jb_byte(j, 0xE9); jb_u32(j, 0); return j->n - 4; }

/* ── gerador ────────────────────────────────────────────────────────────── */
enum { FIX_INSTR = 0, FIX_ILHA = 1, FIX_SAI = 2 };
typedef struct { size_t campo; int32_t alvo; int tipo; } JitFix;

typedef struct {
    JitBuf   j;
    JitFix  *fx; int nfx, capfx;
    int32_t *pos;     /* offset nativo de cada instrução */
    uint8_t *ilha;    /* 1 = a instrução precisa de ilha (saída pro interpretador) */
    int32_t  ninstr;
} JitGen;

static void jg_fix(JitGen *g, size_t campo, int tipo, int32_t alvo)
{
    if (g->nfx == g->capfx) {
        int nc = g->capfx ? g->capfx * 2 : 64;
        JitFix *nf = realloc(g->fx, sizeof(JitFix) * (size_t)nc);
        if (!nf) { g->j.falhou = 1; return; }
        g->fx = nf; g->capfx = nc;
    }
    g->fx[g->nfx].campo = campo; g->fx[g->nfx].tipo = tipo; g->fx[g->nfx].alvo = alvo;
    g->nfx++;
}
/* desvia pra ilha da instrução `i` quando `cc` */
static void jg_ilha_se(JitGen *g, int cc, int32_t i) { g->ilha[i] = 1; jg_fix(g, x_jcc(&g->j, cc), FIX_ILHA, i); }
static void jg_ilha_sempre(JitGen *g, int32_t i)     { g->ilha[i] = 1; jg_fix(g, x_jmp(&g->j), FIX_ILHA, i); }
static void jg_salta_se(JitGen *g, int cc, int32_t i) { jg_fix(g, x_jcc(&g->j, cc), FIX_INSTR, i); }
static void jg_salta(JitGen *g, int32_t i)            { jg_fix(g, x_jmp(&g->j), FIX_INSTR, i); }

/* Copia um Value (16 bytes) em DUAS metades de 8, com rcx/rdx — não com um
 * movdqu: o `n++` grava só os 8 bytes do inteiro, e uma leitura de 16 bytes
 * em cima de uma gravação de 8 não recebe o dado adiantado (store
 * forwarding falha, ~15 ciclos parado). Medido: o laço tipado ficou com
 * 26 ciclos por volta pra 29 instruções; em metades, o dado adianta. */
static void jg_copia(JitGen *g, int dbase, int32_t dd, int sbase, int32_t sd)
{
    x_mov_r64_mem(&g->j, RCX, sbase, sd);
    x_mov_r64_mem(&g->j, RDX, sbase, sd + 8);
    x_mov_mem_r64(&g->j, dbase, dd, RCX);
    x_mov_mem_r64(&g->j, dbase, dd + 8, RDX);
}
/* empilha o Value que está em [base + d] */
static void jg_empilha(JitGen *g, int base, int32_t d)
{
    jg_copia(g, RBX, 0, base, d);
    x_add_r64_imm8(&g->j, RBX, 16);
}
/* `lea rbx, [rbx + d]` — mexe no topo sem tocar nas flags */
static void jg_topo(JitGen *g, int32_t d) { x_lea_r64_mem(&g->j, RBX, RBX, d); }

/* os dois operandos do topo têm que ser int; senão, ilha */
static void jg_exige_int2(JitGen *g, int32_t i)
{
    x_cmp_mem32_imm8(&g->j, RBX, -32, (int8_t)V_INT); jg_ilha_se(g, CC_NE, i);
    x_cmp_mem32_imm8(&g->j, RBX, -16, (int8_t)V_INT); jg_ilha_se(g, CC_NE, i);
}

/* Um opcode → código nativo. Todo caminho rápido confere ANTES de mexer na
 * pilha: ao desistir, a instrução é refeita inteira pelo interpretador. */
static void jg_instr(JitGen *g, const Proto *p, int32_t i)
{
    JitBuf *j = &g->j;
    int32_t ip = i * 2, op = p->code[ip], arg = p->code[ip + 1];
    const int32_t OFF_GLOBAIS = (int32_t)offsetof(VM, globals);
    g->pos[i] = (int32_t)j->n;
    switch (op) {
        case OP_LOAD_CONST:
            if (arg < 0 || arg >= p->nconsts) { jg_ilha_sempre(g, i); break; }
            x_mov_r64_imm64(j, RAX, (uint64_t)(uintptr_t)&p->consts[arg]);
            jg_empilha(g, RAX, 0);
            break;
        case OP_LOAD_LOCAL:
            jg_empilha(g, R14, arg * 16);
            break;
        case OP_STORE_LOCAL:
            jg_topo(g, -16);
            jg_copia(g, R14, arg * 16, RBX, 0);
            break;
        case OP_CLEAR_LOCAL:
            x_mov_mem32_imm(j, R14, arg * 16, V_UNSET);
            break;
        case OP_LOAD_GLOBAL:
            x_mov_r64_mem(j, RAX, R12, OFF_GLOBAIS);
            x_cmp_mem32_imm8(j, RAX, arg * 16, (int8_t)V_UNSET);
            jg_ilha_se(g, CC_E, i);                     /* NameError: quem diz é o interpretador */
            jg_empilha(g, RAX, arg * 16);
            break;
        case OP_STORE_GLOBAL:
            x_mov_r64_mem(j, RAX, R12, OFF_GLOBAIS);
            jg_topo(g, -16);
            jg_copia(g, RAX, arg * 16, RBX, 0);
            break;
        case OP_LOAD_NAME: {
            /* [.., slot] -> [.., valor]: o local do slot, se já foi escrito
             * (tag != UNSET); senão o global `arg` (UNSET = NameError, com o
             * interpretador). É o que o corpo de uma funct usa pra alcançar
             * um nome que não é parâmetro nem declarado — inclusive a
             * própria funct numa recursão. */
            x_mov_r64_mem(j, RAX, RBX, -8);                  /* slot (int) */
            x_shl_r64(j, RAX, 4);
            x_add_r64_r64(j, RAX, R14);                      /* &locals[lbase + slot] */
            x_cmp_mem32_imm8(j, RAX, 0, (int8_t)V_UNSET);
            size_t global = x_jcc(j, CC_E);
            jg_copia(g, RBX, -16, RAX, 0);
            size_t fim = x_jmp(j);
            size_t aqui = j->n;
            j->b[global] = (uint8_t)(aqui - (global + 4)); j->b[global + 1] = (uint8_t)((aqui - (global + 4)) >> 8);
            j->b[global + 2] = (uint8_t)((aqui - (global + 4)) >> 16); j->b[global + 3] = (uint8_t)((aqui - (global + 4)) >> 24);
            x_mov_r64_mem(j, RSI, R12, OFF_GLOBAIS);
            x_cmp_mem32_imm8(j, RSI, arg * 16, (int8_t)V_UNSET);
            jg_ilha_se(g, CC_E, i);
            jg_copia(g, RBX, -16, RSI, arg * 16);
            size_t aqui2 = j->n;
            j->b[fim] = (uint8_t)(aqui2 - (fim + 4)); j->b[fim + 1] = (uint8_t)((aqui2 - (fim + 4)) >> 8);
            j->b[fim + 2] = (uint8_t)((aqui2 - (fim + 4)) >> 16); j->b[fim + 3] = (uint8_t)((aqui2 - (fim + 4)) >> 24);
            break;
        }
        case OP_STORE_NAME: {
            /* [.., valor, slot] -> [..]: escreve no global `arg` se ele JÁ
             * existe (tag != UNSET), senão no local do slot — a regra do
             * interpretador, que faz a funct escrever a global de fora. */
            x_mov_r64_mem(j, RSI, R12, OFF_GLOBAIS);
            x_cmp_mem32_imm8(j, RSI, arg * 16, (int8_t)V_UNSET);
            size_t no_global = x_jcc(j, CC_NE);
            x_mov_r64_mem(j, RAX, RBX, -8);                  /* slot (int) */
            x_shl_r64(j, RAX, 4);
            x_add_r64_r64(j, RAX, R14);
            jg_copia(g, RAX, 0, RBX, -32);
            size_t fim = x_jmp(j);
            size_t aqui = j->n;
            j->b[no_global] = (uint8_t)(aqui - (no_global + 4)); j->b[no_global + 1] = (uint8_t)((aqui - (no_global + 4)) >> 8);
            j->b[no_global + 2] = (uint8_t)((aqui - (no_global + 4)) >> 16); j->b[no_global + 3] = (uint8_t)((aqui - (no_global + 4)) >> 24);
            jg_copia(g, RSI, arg * 16, RBX, -32);
            size_t aqui2 = j->n;
            j->b[fim] = (uint8_t)(aqui2 - (fim + 4)); j->b[fim + 1] = (uint8_t)((aqui2 - (fim + 4)) >> 8);
            j->b[fim + 2] = (uint8_t)((aqui2 - (fim + 4)) >> 16); j->b[fim + 3] = (uint8_t)((aqui2 - (fim + 4)) >> 24);
            jg_topo(g, -32);
            break;
        }
        case OP_CLEAR_GLOBAL:
            x_mov_r64_mem(j, RAX, R12, OFF_GLOBAIS);
            x_mov_mem32_imm(j, RAX, arg * 16, V_UNSET);
            break;
        case OP_DUP:
            jg_empilha(g, RBX, -16);
            break;
        case OP_POP_TOP:
            jg_topo(g, -16);
            break;
        case OP_JUMP: {
            int32_t alvo = arg / 2;
            if (arg < 0 || (arg & 1) || alvo >= g->ninstr) { jg_ilha_sempre(g, i); break; }
            if (arg < ip) {
                /* salto de RECUO = volta de laço: se o GC está devendo, o
                 * interpretador faz este salto (e a coleta no ponto seguro) */
                x_mov_r64_mem(j, RAX, R12, (int32_t)offsetof(VM, alocado));
                x_cmp_r64_mem(j, RAX, R12, (int32_t)offsetof(VM, proximo_gc));
                jg_ilha_se(g, CC_A, i);
            }
            jg_salta(g, alvo);
            break;
        }
        case OP_JUMP_IF_FALSE:
        case OP_JUMP_IF_TRUE: {
            int32_t alvo = arg / 2;
            if (arg < 0 || (arg & 1) || alvo >= g->ninstr) { jg_ilha_sempre(g, i); break; }
            int cc = (op == OP_JUMP_IF_FALSE) ? CC_E : CC_NE;
            /* bool: testa os 4 bytes de `as.b`; int: os 8 de `as.i`; resto: ilha */
            x_cmp_mem32_imm8(j, RBX, -16, (int8_t)V_BOOL);
            size_t nao_bool = x_jcc(j, CC_NE);
            x_cmp_mem32_imm8(j, RBX, -8, 0);
            jg_topo(g, -16);
            jg_salta_se(g, cc, alvo);
            size_t fim = x_jmp(j);
            size_t aqui = j->n;
            j->b[nao_bool] = (uint8_t)(aqui - (nao_bool + 4)); j->b[nao_bool + 1] = (uint8_t)((aqui - (nao_bool + 4)) >> 8);
            j->b[nao_bool + 2] = (uint8_t)((aqui - (nao_bool + 4)) >> 16); j->b[nao_bool + 3] = (uint8_t)((aqui - (nao_bool + 4)) >> 24);
            x_cmp_mem32_imm8(j, RBX, -16, (int8_t)V_INT);
            jg_ilha_se(g, CC_NE, i);
            x_cmp_mem64_imm8(j, RBX, -8, 0);
            jg_topo(g, -16);
            jg_salta_se(g, cc, alvo);
            size_t aqui2 = j->n;
            j->b[fim] = (uint8_t)(aqui2 - (fim + 4)); j->b[fim + 1] = (uint8_t)((aqui2 - (fim + 4)) >> 8);
            j->b[fim + 2] = (uint8_t)((aqui2 - (fim + 4)) >> 16); j->b[fim + 3] = (uint8_t)((aqui2 - (fim + 4)) >> 24);
            break;
        }
        case OP_JF_LT: case OP_JF_GT: case OP_JF_LE: case OP_JF_GE: case OP_JF_EQ: case OP_JF_NE: {
            int32_t alvo = arg / 2;
            if (arg < 0 || (arg & 1) || alvo >= g->ninstr) { jg_ilha_sempre(g, i); break; }
            /* salta quando a comparação dá FALSO: a condição negada */
            int cc = op == OP_JF_LT ? CC_GE : op == OP_JF_GT ? CC_LE : op == OP_JF_LE ? CC_G
                   : op == OP_JF_GE ? CC_L : op == OP_JF_EQ ? CC_NE : CC_E;
            jg_exige_int2(g, i);
            x_mov_r64_mem(j, RAX, RBX, -24);
            x_cmp_r64_mem(j, RAX, RBX, -8);
            jg_topo(g, -32);
            jg_salta_se(g, cc, alvo);
            break;
        }
        case OP_LT: case OP_GT: case OP_LE: case OP_GE: case OP_EQ: case OP_NE: {
            int cc = op == OP_LT ? CC_L : op == OP_GT ? CC_G : op == OP_LE ? CC_LE
                   : op == OP_GE ? CC_GE : op == OP_EQ ? CC_E : CC_NE;
            jg_exige_int2(g, i);
            x_mov_r64_mem(j, RAX, RBX, -24);
            x_cmp_r64_mem(j, RAX, RBX, -8);
            x_setcc_al(j, cc);
            x_movzx_eax_al(j);
            jg_topo(g, -16);
            x_mov_mem32_imm(j, RBX, -16, V_BOOL);
            x_mov_mem_r64(j, RBX, -8, RAX);
            break;
        }
        case OP_ADD: case OP_SUB: case OP_MUL:
            jg_exige_int2(g, i);
            x_mov_r64_mem(j, RAX, RBX, -24);
            if (op == OP_ADD)      x_add_r64_mem(j, RAX, RBX, -8);
            else if (op == OP_SUB) x_sub_r64_mem(j, RAX, RBX, -8);
            else                   x_imul_r64_mem(j, RAX, RBX, -8);
            jg_ilha_se(g, CC_O, i);                     /* estouro: bigint é com o interpretador */
            x_mov_mem_r64(j, RBX, -24, RAX);
            jg_topo(g, -16);
            break;
        case OP_INC_GLOBAL_INT: case OP_DEC_GLOBAL_INT:
        case OP_INC_LOCAL_INT:  case OP_DEC_LOCAL_INT: {
            int global = (op == OP_INC_GLOBAL_INT || op == OP_DEC_GLOBAL_INT);
            int base = global ? RAX : R14;
            if (global) x_mov_r64_mem(j, RAX, R12, OFF_GLOBAIS);
            x_cmp_mem32_imm8(j, base, arg * 16, (int8_t)V_INT);
            jg_ilha_se(g, CC_NE, i);
            x_mov_r64_mem(j, RCX, base, arg * 16 + 8);
            x_add_r64_imm8(j, RCX, (op == OP_INC_GLOBAL_INT || op == OP_INC_LOCAL_INT) ? 1 : -1);
            jg_ilha_se(g, CC_O, i);
            x_mov_mem_r64(j, base, arg * 16 + 8, RCX);
            break;
        }
        case OP_COERCE_DECL: {
            int tipo = arg & 15;
            if (tipo == TIPO_INT)       { x_cmp_mem32_imm8(j, RBX, -16, (int8_t)V_INT);   jg_ilha_se(g, CC_NE, i); }
            else if (tipo == TIPO_FLO)  { x_cmp_mem32_imm8(j, RBX, -16, (int8_t)V_FLOAT); jg_ilha_se(g, CC_NE, i); }
            else if (tipo == TIPO_BOOL) { x_cmp_mem32_imm8(j, RBX, -16, (int8_t)V_BOOL);  jg_ilha_se(g, CC_NE, i); }
            else if (tipo == TIPO_STR) {
                x_cmp_mem32_imm8(j, RBX, -16, (int8_t)V_OBJ); jg_ilha_se(g, CC_NE, i);
                x_mov_r64_mem(j, RAX, RBX, -8);
                x_cmp_mem32_imm8(j, RAX, 0, (int8_t)OBJ_STRING); jg_ilha_se(g, CC_NE, i);
            } else jg_ilha_sempre(g, i);
            break;
        }
        case OP_NOT:
            x_cmp_mem32_imm8(j, RBX, -16, (int8_t)V_BOOL); jg_ilha_se(g, CC_NE, i);
            x_cmp_mem32_imm8(j, RBX, -8, 0);
            x_setcc_al(j, CC_E);
            x_movzx_eax_al(j);
            x_mov_mem_r64(j, RBX, -8, RAX);
            break;
        case OP_TO_BOOL:
            x_cmp_mem32_imm8(j, RBX, -16, (int8_t)V_BOOL); jg_ilha_se(g, CC_NE, i);
            break;
        case OP_ITER_RANGE: {
            /* [ini, quant, passo, i] em rbx-64..rbx-16, tudo int (RANGE_PREPARA) */
            int32_t alvo = arg / 2;
            if (arg < 0 || (arg & 1) || alvo >= g->ninstr) { jg_ilha_sempre(g, i); break; }
            x_mov_r64_mem(j, RAX, RBX, -8);            /* i */
            x_cmp_r64_mem(j, RAX, RBX, -40);           /* i >= quant -> fim */
            size_t fim = x_jcc(j, CC_GE);
            x_mov_r64_r64(j, RCX, RAX);
            x_imul_r64_mem(j, RCX, RBX, -24);          /* i * passo */
            x_add_r64_mem(j, RCX, RBX, -56);           /* + ini */
            x_add_r64_imm8(j, RAX, 1);
            x_mov_mem_r64(j, RBX, -8, RAX);            /* i + 1 */
            x_mov_mem32_imm(j, RBX, 0, V_INT);
            x_mov_mem_r64(j, RBX, 8, RCX);
            x_add_r64_imm8(j, RBX, 16);
            size_t segue = x_jmp(j);
            size_t aqui = j->n;
            j->b[fim] = (uint8_t)(aqui - (fim + 4)); j->b[fim + 1] = (uint8_t)((aqui - (fim + 4)) >> 8);
            j->b[fim + 2] = (uint8_t)((aqui - (fim + 4)) >> 16); j->b[fim + 3] = (uint8_t)((aqui - (fim + 4)) >> 24);
            jg_topo(g, -64);
            jg_salta(g, alvo);
            size_t aqui2 = j->n;
            j->b[segue] = (uint8_t)(aqui2 - (segue + 4)); j->b[segue + 1] = (uint8_t)((aqui2 - (segue + 4)) >> 8);
            j->b[segue + 2] = (uint8_t)((aqui2 - (segue + 4)) >> 16); j->b[segue + 3] = (uint8_t)((aqui2 - (segue + 4)) >> 24);
            break;
        }
        default:
            /* sem gabarito: o interpretador faz esta e o nativo retoma na seguinte */
            jg_ilha_sempre(g, i);
            break;
    }
}

/* Compila o proto inteiro. 0 = compilou (p->nativo pronto); -1 = não (e
 * p->jit_estado = 2: não tenta de novo). Nunca aborta o programa. */
static int jit_compila_proto(VM *vm, Proto *p)
{
    p->jit_estado = 2;
    if (!jit_ligado() || !p->code || p->ncode < 2) return -1;
    JitGen g;
    memset(&g, 0, sizeof(g));
    g.ninstr = p->ncode / 2;
    g.pos  = malloc(sizeof(int32_t) * (size_t)g.ninstr);
    g.ilha = calloc((size_t)g.ninstr, 1);
    if (!g.pos || !g.ilha) { free(g.pos); free(g.ilha); return -1; }
    JitBuf *j = &g.j;

    /* prólogo: salva os preservados, carrega o estado do frame */
    x_push(j, RBX); x_push(j, RBP); x_push(j, R12); x_push(j, R13); x_push(j, R14); x_push(j, R15);
    x_mov_r64_r64(j, R12, RDI);
    x_mov_r64_r64(j, R15, RSI);
    x_mov_r64_mem(j, R13, R12, (int32_t)offsetof(VM, stack));
    x_mov_r64_mem(j, R14, R12, (int32_t)offsetof(VM, locals));
    x_mov_r32_mem(j, RAX, R15, (int32_t)offsetof(JitCtx, lbase));
    x_shl_r64(j, RAX, 4);
    x_add_r64_r64(j, R14, RAX);
    x_mov_r32_mem(j, RAX, R15, (int32_t)offsetof(JitCtx, sp));
    x_shl_r64(j, RAX, 4);
    x_mov_r64_r64(j, RBX, R13);
    x_add_r64_r64(j, RBX, RAX);
    x_jmp_r64(j, RDX);                                  /* retoma onde o interpretador mandou */

    for (int32_t i = 0; i < g.ninstr && !j->falhou; i++) jg_instr(&g, p, i);
    x_ud2(j);                                           /* cair do fim é impossível: RETURN/HALT são ilhas */

    /* saída: publica o topo da pilha e devolve */
    size_t sai = j->n;
    x_mov_r64_r64(j, RAX, RBX);
    x_sub_r64_r64(j, RAX, R13);
    x_shr_r64(j, RAX, 4);
    x_mov_mem_r32(j, R15, (int32_t)offsetof(JitCtx, sp), RAX);
    x_pop(j, R15); x_pop(j, R14); x_pop(j, R13); x_pop(j, R12); x_pop(j, RBP); x_pop(j, RBX);
    x_ret(j);

    /* as ilhas: anota o ip (já com o +2, como o interpretador conta) e sai */
    int32_t *ilha_pos = malloc(sizeof(int32_t) * (size_t)g.ninstr);
    if (!ilha_pos) j->falhou = 1;
    for (int32_t i = 0; i < g.ninstr && !j->falhou; i++) {
        ilha_pos[i] = -1;
        if (!g.ilha[i]) continue;
        ilha_pos[i] = (int32_t)j->n;
        x_mov_mem32_imm(j, R15, (int32_t)offsetof(JitCtx, ip), i * 2 + 2);
        jg_fix(&g, x_jmp(j), FIX_SAI, 0);
    }
    /* resolve os saltos */
    for (int k = 0; k < g.nfx && !j->falhou; k++) {
        size_t campo = g.fx[k].campo;
        int32_t alvo_off = g.fx[k].tipo == FIX_INSTR ? g.pos[g.fx[k].alvo]
                         : g.fx[k].tipo == FIX_ILHA  ? ilha_pos[g.fx[k].alvo]
                         : (int32_t)sai;
        if (alvo_off < 0) { j->falhou = 1; break; }
        int32_t rel = alvo_off - (int32_t)(campo + 4);
        j->b[campo] = (uint8_t)rel; j->b[campo + 1] = (uint8_t)(rel >> 8);
        j->b[campo + 2] = (uint8_t)(rel >> 16); j->b[campo + 3] = (uint8_t)(rel >> 24);
    }
    free(ilha_pos);
    free(g.ilha);
    free(g.fx);
    if (j->falhou) { free(j->b); free(g.pos); return -1; }

    /* pra memória executável: RW, copia, RX. Recusa = segue interpretado. */
    size_t tam = (j->n + 4095) & ~(size_t)4095;
    void *mem = g_jit_falha ? MAP_FAILED
              : mmap(NULL, tam, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) { free(j->b); free(g.pos); return -1; }
    memcpy(mem, j->b, j->n);
    if (mprotect(mem, tam, PROT_READ | PROT_EXEC) != 0) { munmap(mem, tam); free(j->b); free(g.pos); return -1; }
    JitTrecho *t = malloc(sizeof(*t));
    if (!t) { munmap(mem, tam); free(j->b); free(g.pos); return -1; }
    t->mem = mem; t->tam = tam; t->prox = vm->jit_trechos; vm->jit_trechos = t;
    if (g_jit_log)
        fprintf(stderr, "[jit] %s: %d instrucoes, %zu bytes\n",
                (p->nome && p->nome[0]) ? p->nome : "<module>", g.ninstr, j->n);
    free(j->b);
    p->nativo = mem;
    p->nativo_ip = g.pos;
    p->jit_estado = 1;
    return 0;
}

static void jit_solta(VM *vm)
{
    JitTrecho *t = vm->jit_trechos;
    while (t) { JitTrecho *px = t->prox; munmap(t->mem, t->tam); free(t); t = px; }
    vm->jit_trechos = NULL;
}

#endif /* PS_JIT_X64_H */
