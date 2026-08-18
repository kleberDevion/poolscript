/*
 * Compilador AST → bytecode, em C puro.
 *
 * Porte de compiler.py + flatten.py. Duas coisas que ele resolve em tempo de
 * compilação e por isso somem do caminho quente:
 *
 *   - Nome vira ÍNDICE. `n` dentro de `fib` deixa de ser busca por string
 *     num dict de escopo e passa a ser `locals[0]`. É daí que veio a maior
 *     parte do ganho medido do tree-walker pra VM.
 *   - A tabela de protótipos já sai plana: `MAKE_FUNCTION` referencia o
 *     índice do protótipo, não uma constante que contém outro CodeObj.
 *
 * Nó fora do subconjunto compilável para com erro explícito — nunca gera
 * bytecode errado em silêncio.
 */
#include "ps_compiler.h"

#include "ps_lexer.h"
#include "ps_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── opcodes: precisam bater com opcodes.py e poolscript_vm.c ───────────── */
enum {
    OP_LOAD_CONST = 0, OP_LOAD_LOCAL = 1, OP_STORE_LOCAL = 2,
    OP_LOAD_GLOBAL = 3, OP_STORE_GLOBAL = 4,
    OP_ADD = 5, OP_SUB = 6, OP_MUL = 7, OP_DIV = 8, OP_MOD = 9, OP_NEG = 10,
    OP_LT = 11, OP_GT = 12, OP_LE = 13, OP_GE = 14, OP_EQ = 15, OP_NE = 16,
    OP_JUMP = 17, OP_JUMP_IF_FALSE = 18, OP_POP_TOP = 19,
    OP_CALL = 20, OP_RETURN = 21, OP_MAKE_FUNCTION = 22,
    OP_BIT_OR = 23, OP_BIT_XOR = 24, OP_BIT_AND = 25,
    OP_LSHIFT = 26, OP_RSHIFT = 27, OP_BIT_NOT = 28,
    OP_HALT = 29,
    OP_BUILD_LIST = 30, OP_BUILD_DICT = 31,
    OP_INDEX_GET = 32, OP_INDEX_SET = 33,
    OP_ITER_NEXT = 34, OP_DUP = 35,
    OP_BUILD_STR = 36, OP_BUILD_TUPLE = 37, OP_SLICE = 38,
    OP_JUMP_IF_SET = 39,
    OP_LOAD_NAME = 40, OP_STORE_NAME = 41, OP_CALL_KW = 42,
    OP_SETUP_TRY = 43, OP_POP_TRY = 44, OP_RAISE = 45, OP_PUSH_ERR_TYPE = 46,
    OP_JUMP_IF_TRUE = 47, OP_LEN = 48, OP_HAS_KEY = 49,
    OP_MAKE_CLASS = 50, OP_GET_MEMBER = 51, OP_SET_MEMBER = 52, OP_LOAD_SELF = 53, OP_CALL_BASE = 54, OP_DUP2 = 55, OP_IMPORT_MOD = 56,
    OP_NOT = 67, OP_TO_BOOL = 68, OP_COERCE_DECL = 69, OP_COERCE_RET = 70, OP_RERAISE = 71,
    OP_SKIP_IF_IMPORT = 72,   /* pula o bloco de run_selfwith_ quando importando */
    OP_IS = 57, OP_IN = 58, OP_LOAD_TIPO = 59,
    OP_COUNT = 60, OP_COUNT_PARES = 61, OP_CHECK_NONNULL = 62,
    OP_MAKE_MODEL = 63, OP_UNPACK = 64, OP_YIELD = 65, OP_CLOSE_SE_TEM = 66,
    OP_MAKE_ENUM = 73,  /* enum Nome { ... } — descritor em vm->enum_* */
    /* fim de bloco: apaga (V_UNSET) os locais/globais nascidos dentro do bloco,
     * pra variável de bloco não vazar pro escopo de fora (paridade com o interp) */
    OP_CLEAR_LOCAL = 74, OP_CLEAR_GLOBAL = 75
};

const char *ps_op_nome(int32_t op)
{
    switch (op) {
        case OP_LOAD_CONST: return "LOAD_CONST";
        case OP_LOAD_LOCAL: return "LOAD_LOCAL";
        case OP_STORE_LOCAL: return "STORE_LOCAL";
        case OP_LOAD_GLOBAL: return "LOAD_GLOBAL";
        case OP_STORE_GLOBAL: return "STORE_GLOBAL";
        case OP_ADD: return "ADD";  case OP_SUB: return "SUB";
        case OP_MUL: return "MUL";  case OP_DIV: return "DIV";
        case OP_MOD: return "MOD";  case OP_NEG: return "NEG";
        case OP_LT: return "LT";    case OP_GT: return "GT";
        case OP_LE: return "LE";    case OP_GE: return "GE";
        case OP_EQ: return "EQ";    case OP_NE: return "NE";
        case OP_JUMP: return "JUMP";
        case OP_JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case OP_POP_TOP: return "POP_TOP";
        case OP_CALL: return "CALL";
        case OP_RETURN: return "RETURN";
        case OP_MAKE_FUNCTION: return "MAKE_FUNCTION";
        case OP_BIT_OR: return "BIT_OR"; case OP_BIT_XOR: return "BIT_XOR";
        case OP_BIT_AND: return "BIT_AND";
        case OP_LSHIFT: return "LSHIFT"; case OP_RSHIFT: return "RSHIFT";
        case OP_BIT_NOT: return "BIT_NOT";
        case OP_HALT: return "HALT";
        case OP_BUILD_LIST: return "BUILD_LIST";
        case OP_BUILD_DICT: return "BUILD_DICT";
        case OP_INDEX_GET: return "INDEX_GET";
        case OP_INDEX_SET: return "INDEX_SET";
        case OP_ITER_NEXT: return "ITER_NEXT";
        case OP_DUP: return "DUP";
        case OP_BUILD_STR: return "BUILD_STR";
        case OP_BUILD_TUPLE: return "BUILD_TUPLE";
        case OP_SLICE: return "SLICE";
        case OP_JUMP_IF_SET: return "JUMP_IF_SET";
        case OP_LOAD_NAME: return "LOAD_NAME";
        case OP_STORE_NAME: return "STORE_NAME";
        case OP_CALL_KW: return "CALL_KW";
        case OP_SETUP_TRY: return "SETUP_TRY";
        case OP_POP_TRY: return "POP_TRY";
        case OP_RAISE: return "RAISE";
        case OP_PUSH_ERR_TYPE: return "PUSH_ERR_TYPE";
        case OP_JUMP_IF_TRUE: return "JUMP_IF_TRUE";
        case OP_LEN: return "LEN";
        case OP_HAS_KEY: return "HAS_KEY";
        case OP_MAKE_CLASS: return "MAKE_CLASS";
        case OP_GET_MEMBER: return "GET_MEMBER";
        case OP_SET_MEMBER: return "SET_MEMBER";
        case OP_LOAD_SELF: return "LOAD_SELF";
        case OP_CALL_BASE: return "CALL_BASE";
        case OP_DUP2: return "DUP2";
        case OP_IMPORT_MOD: return "IMPORT_MOD";
        case OP_IS: return "IS_OP";
        case OP_IN: return "IN_OP";
        case OP_LOAD_TIPO: return "LOAD_TIPO";
        case OP_NOT: return "NOT";
        case OP_COERCE_DECL: return "COERCE_DECL";
        case OP_COERCE_RET: return "COERCE_RET";
        case OP_RERAISE: return "RERAISE";
        case OP_SKIP_IF_IMPORT: return "SKIP_IF_IMPORT";
        case OP_TO_BOOL: return "TO_BOOL";
        case OP_COUNT: return "COUNT";
        case OP_COUNT_PARES: return "COUNT_PARES";
        case OP_CHECK_NONNULL: return "CHECK_NONNULL";
        case OP_MAKE_MODEL: return "MAKE_MODEL";
        case OP_MAKE_ENUM: return "MAKE_ENUM";
        case OP_UNPACK: return "UNPACK";
        case OP_YIELD: return "YIELD";
        case OP_CLOSE_SE_TEM: return "CLOSE_SE_TEM";
        case OP_CLEAR_LOCAL: return "CLEAR_LOCAL";
        case OP_CLEAR_GLOBAL: return "CLEAR_GLOBAL";
    }
    return "?";
}

/* ── estado ─────────────────────────────────────────────────────────────── */
/* Guarda o ÍNDICE do protótipo, não o ponteiro: uma action aninhada chama
 * `novo_proto`, que faz realloc do array — qualquer PSProto* guardado aqui
 * viraria ponteiro pendurado no meio da compilação. */
typedef struct {
    int32_t  idx;
    int32_t  cap_code;
    int32_t  cap_consts;
    char   **locais;       /* nome de cada slot local */
    int32_t  nlocais;
    int32_t  cap_locais;
    int      eh_modulo;
    /* Nomes marcados por `global x` dentro desta função: passam a resolver
     * na tabela de globais em vez de virar slot local, tanto na leitura
     * quanto na escrita. */
    char   **globais_decl;
    int32_t  nglobais_decl;
    int32_t  cap_globais_decl;
    /* Locais CERTOS: parâmetro ou declaração tipada (`int x = 1`). Só esses
     * usam LOAD_LOCAL/STORE_LOCAL direto. Nome atribuído sem tipo pode estar
     * modificando uma variável externa, então vira LOAD_NAME/STORE_NAME e a
     * decisão fica em runtime. */
    unsigned char certo[256];
    /* `int action` / `bool action`: 1 = int, 2 = bool, 0 = sem tipo. Faz o
     * RETURN converter Null e faz o corpo inteiro virar um `try` implícito —
     * é o contrato dessas duas declarações: nunca propagam erro. */
    int      tipo_ret;
    /* Módulo (eh_modulo): nomes de globais CRIADOS por atribuição, em ordem.
     * Um nome atribuído dentro de um bloco que ainda não existe vira "nascido
     * no bloco" e é apagado (OP_CLEAR_GLOBAL) no fim dele; reatribuir um nome
     * já criado é write-through (não entra de novo). Só o módulo usa isto —
     * função usa a marca sobre `locais`. */
    char   **mod_criados;
    int32_t  n_mod_criados;
    int32_t  cap_mod_criados;
} Unidade;

/* `break`/`continue` precisam saber o laço em que estão. O endereço do fim
 * do laço só é conhecido DEPOIS de compilar o corpo, então os saltos ficam
 * pendentes aqui e são corrigidos (backpatch) no fechamento. */
#define MAX_LACOS 32
#define MAX_SAIDAS 64

typedef struct {
    int32_t inicio;                  /* alvo do `continue` */
    int32_t saidas[MAX_SAIDAS];      /* saltos do `break`, a corrigir */
    int32_t nsaidas;
    int32_t continues[MAX_SAIDAS];   /* saltos do `continue`, a corrigir */
    int32_t ncontinues;
    /* Quantos slots o laço mantém na pilha durante o corpo. `for each`
     * carrega (container, indice) — 2 slots — e o ITER_NEXT só os descarta
     * quando a iteração TERMINA. Um `break` sai por fora, então precisa
     * limpá-los na mão; sem isso o lixo sobra e corrompe o laço externo. */
    int     slots_pilha;
    int32_t escopo_marca;       /* entrada do laço (inclui var do laço/self): o
                                 * break e a saída normal apagam tudo daí pra frente */
    int32_t escopo_marca_body;  /* início do CORPO (após self/_count etc.): o
                                 * continue e o fim de iteração resetam só daqui —
                                 * é o escopo por-iteração, sem tocar no que é do laço */
} Laco;

typedef struct {
    PSPrograma *out;
    int32_t     cap_protos;
    int32_t     cap_globais;
    Laco        lacos[MAX_LACOS];
    int         nlacos;
    int32_t     cap_classes;
    int32_t     cap_models;
    int32_t     cap_enums;
    /* `@NonNull` visto, esperando a action que ele decora. */
    int         pendente_nonnull;
    /* profundidade de `try` aberto — `yield` dentro de um é recusado */
    int         dentro_try;
    /* profundidade de `count each` aberto — muda o que `return;` devolve */
    int         dentro_count_each;
    /* Nome do primeiro pai da Entity cujos métodos estão sendo compilados —
     * é o que `base(...)` precisa pra buscar o __init__ no lugar certo. */
    const char *entity_pai;
    /* Estamos dentro de uma Entity (com ou sem pai)? `base()` sem pai é
     * no-op aqui e erro fora. */
    int dentro_entity;
    /* Linha do fonte do nó sendo compilado agora — o `emite()` grava por
     * instrução, pra o erro de runtime dizer ONDE aconteceu. */
    int32_t linha_atual;
    int32_t coluna_atual;
} C;

/* Resolve o protótipo da unidade AGORA — nunca cacheia o ponteiro. */
#define UP(c, u) (&(c)->out->protos[(u)->idx])

#define CFALHOU(c) (!(c)->out->ok)

static void cerro(C *c, const char *msg, PSNode *n)
{
    if (!c->out->ok) return;
    c->out->ok = 0;
    snprintf(c->out->erro, sizeof(c->out->erro), "%s", msg);
    c->out->erro_linha = n ? n->line : 0;
    c->out->erro_col = n ? n->col : 0;
}

/* ── emissão ────────────────────────────────────────────────────────────── */
static int32_t emite(C *c, Unidade *u, int32_t op, int32_t arg)
{
    if (UP(c, u)->ncode + 2 > u->cap_code) {
        int32_t novo = u->cap_code < 64 ? 64 : u->cap_code * 2;
        int32_t *nc = realloc(UP(c, u)->code, sizeof(int32_t) * (size_t)novo);
        if (!nc) { cerro(c, "sem memoria", NULL); return -1; }
        UP(c, u)->code = nc;
        /* tabelas de linha e coluna crescem junto com o code (mesma capacidade) */
        int32_t *nl = realloc(UP(c, u)->linhas, sizeof(int32_t) * (size_t)novo);
        if (!nl) { cerro(c, "sem memoria", NULL); return -1; }
        UP(c, u)->linhas = nl;
        int32_t *ncol = realloc(UP(c, u)->colunas, sizeof(int32_t) * (size_t)novo);
        if (!ncol) { cerro(c, "sem memoria", NULL); return -1; }
        UP(c, u)->colunas = ncol;
        u->cap_code = novo;
    }
    int32_t pos = UP(c, u)->ncode;
    UP(c, u)->linhas[pos]     = c->linha_atual;
    UP(c, u)->linhas[pos + 1] = c->linha_atual;
    UP(c, u)->colunas[pos]     = c->coluna_atual;
    UP(c, u)->colunas[pos + 1] = c->coluna_atual;
    UP(c, u)->code[UP(c, u)->ncode++] = op;
    UP(c, u)->code[UP(c, u)->ncode++] = arg;
    return pos;
}

/* Dedup por tipo E valor: `1` e `True` não podem colidir, senão o pool
 * devolveria o índice de um valor de outro tipo. */
static int32_t idx_const(C *c, Unidade *u, PSConstKind k,
                         int64_t i, double d, const char *s, int32_t slen)
{
    for (int32_t x = 0; x < UP(c, u)->nconsts; x++) {
        PSConst *e = &UP(c, u)->consts[x];
        if (e->kind != k) continue;
        if (k == K_STR || k == K_BIGINT) {
            if (e->slen == slen && memcmp(e->s, s, (size_t)slen) == 0) return x;
        } else if (k == K_FLO) {
            if (e->d == d) return x;
        } else if (k == K_NULL) {
            return x;
        } else {
            if (e->i == i) return x;
        }
    }
    if (UP(c, u)->nconsts + 1 > u->cap_consts) {
        int32_t novo = u->cap_consts < 16 ? 16 : u->cap_consts * 2;
        PSConst *nc = realloc(UP(c, u)->consts, sizeof(PSConst) * (size_t)novo);
        if (!nc) { cerro(c, "sem memoria", NULL); return -1; }
        UP(c, u)->consts = nc;
        u->cap_consts = novo;
    }
    PSConst *e = &UP(c, u)->consts[UP(c, u)->nconsts];
    memset(e, 0, sizeof(*e));
    e->kind = k; e->i = i; e->d = d;
    if (k == K_STR || k == K_BIGINT) {
        e->s = malloc((size_t)slen + 1);
        if (!e->s) { cerro(c, "sem memoria", NULL); return -1; }
        memcpy(e->s, s, (size_t)slen);
        e->s[slen] = '\0';
        e->slen = slen;
    }
    return UP(c, u)->nconsts++;
}

static int32_t idx_global(C *c, const char *nome)
{
    for (int32_t i = 0; i < c->out->nglobais; i++)
        if (strcmp(c->out->globais[i], nome) == 0) return i;
    if (c->out->nglobais + 1 > c->cap_globais) {
        int32_t novo = c->cap_globais < 16 ? 16 : c->cap_globais * 2;
        char **ng = realloc(c->out->globais, sizeof(char *) * (size_t)novo);
        if (!ng) { cerro(c, "sem memoria", NULL); return -1; }
        c->out->globais = ng;
        c->cap_globais = novo;
    }
    size_t n = strlen(nome);
    char *copia = malloc(n + 1);
    if (!copia) { cerro(c, "sem memoria", NULL); return -1; }
    memcpy(copia, nome, n + 1);
    c->out->globais[c->out->nglobais] = copia;
    return c->out->nglobais++;
}

static int32_t idx_local(C *c, Unidade *u, const char *nome)
{
    for (int32_t i = 0; i < u->nlocais; i++)
        if (strcmp(u->locais[i], nome) == 0) return i;
    if (u->nlocais + 1 > u->cap_locais) {
        int32_t novo = u->cap_locais < 8 ? 8 : u->cap_locais * 2;
        char **nl = realloc(u->locais, sizeof(char *) * (size_t)novo);
        if (!nl) { cerro(c, "sem memoria", NULL); return -1; }
        u->locais = nl;
        u->cap_locais = novo;
    }
    size_t n = strlen(nome);
    char *copia = malloc(n + 1);
    if (!copia) { cerro(c, "sem memoria", NULL); return -1; }
    memcpy(copia, nome, n + 1);
    u->locais[u->nlocais] = copia;
    /* nlocals = MARCA D'ÁGUA (maior slot já usado + 1), não o corrente: com
     * escopo de bloco os slots são reaproveitados (nlocais encolhe no fim do
     * bloco), e o frame precisa caber o pico, não o valor do momento. */
    if (u->nlocais + 1 > UP(c, u)->nlocals) UP(c, u)->nlocals = u->nlocais + 1;
    return u->nlocais++;
}

/* ── escopo de bloco ──────────────────────────────────────────────────────
 * Variável nascida dentro de um bloco (if/for/while/...) não vaza pro escopo
 * de fora — igual ao interpretador. A marca é o nº de nomes vivos na entrada
 * do bloco; no fim, os nomes dali pra frente são apagados em runtime
 * (OP_CLEAR_LOCAL/GLOBAL) e some da resolução de compilação. */
static int32_t escopo_marca(Unidade *u)
{
    return u->eh_modulo ? u->n_mod_criados : u->nlocais;
}

/* Emite os OP_CLEAR_* dos nomes nascidos desde `marca` — SEM mexer no estado
 * de compilação. Usado no fim de bloco (fall-through), no continue e no break. */
static void escopo_emite_clears(C *c, Unidade *u, int32_t marca)
{
    if (u->eh_modulo) {
        for (int32_t i = u->n_mod_criados - 1; i >= marca; i--)
            emite(c, u, OP_CLEAR_GLOBAL, idx_global(c, u->mod_criados[i]));
    } else {
        for (int32_t i = u->nlocais - 1; i >= marca; i--)
            emite(c, u, OP_CLEAR_LOCAL, i);
    }
}

/* Remove da resolução de compilação os nomes nascidos desde `marca` (sem
 * emitir nada). Depois disso o mesmo nome, se reusado, ganha slot/global novo. */
static void escopo_trunca(Unidade *u, int32_t marca)
{
    if (u->eh_modulo) {
        for (int32_t i = u->n_mod_criados - 1; i >= marca; i--)
            free(u->mod_criados[i]);
        if (u->n_mod_criados > marca) u->n_mod_criados = marca;
    } else {
        for (int32_t i = u->nlocais - 1; i >= marca; i--) {
            free(u->locais[i]);
            if (i < 256) u->certo[i] = 0;
        }
        if (u->nlocais > marca) u->nlocais = marca;
    }
}

/* Fecha um bloco comum (não-laço): apaga em runtime e some da compilação. */
static void escopo_fecha(C *c, Unidade *u, int32_t marca)
{
    escopo_emite_clears(c, u, marca);
    escopo_trunca(u, marca);
}

/* Módulo: registra um global CRIADO por atribuição (idempotente). Nome já
 * registrado (em qualquer escopo já aberto) é write-through — não reentra. */
static void mod_criados_add(C *c, Unidade *u, const char *nome)
{
    for (int32_t i = 0; i < u->n_mod_criados; i++)
        if (strcmp(u->mod_criados[i], nome) == 0) return;
    if (u->n_mod_criados + 1 > u->cap_mod_criados) {
        int32_t novo = u->cap_mod_criados < 8 ? 8 : u->cap_mod_criados * 2;
        char **nl = realloc(u->mod_criados, sizeof(char *) * (size_t)novo);
        if (!nl) { cerro(c, "sem memoria", NULL); return; }
        u->mod_criados = nl;
        u->cap_mod_criados = novo;
    }
    size_t n = strlen(nome);
    char *copia = malloc(n + 1);
    if (!copia) { cerro(c, "sem memoria", NULL); return; }
    memcpy(copia, nome, n + 1);
    u->mod_criados[u->n_mod_criados++] = copia;
}

/* ── operador do AST → opcode ───────────────────────────────────────────── */
static int32_t op_binario(const char *s)
{
    if (!s) return -1;
    if (!strcmp(s, "+"))  return OP_ADD;
    if (!strcmp(s, "-"))  return OP_SUB;
    if (!strcmp(s, "*"))  return OP_MUL;
    if (!strcmp(s, "/"))  return OP_DIV;
    if (!strcmp(s, "%"))  return OP_MOD;
    if (!strcmp(s, "<"))  return OP_LT;
    if (!strcmp(s, ">"))  return OP_GT;
    if (!strcmp(s, "<=")) return OP_LE;
    if (!strcmp(s, ">=")) return OP_GE;
    if (!strcmp(s, "==") || !strcmp(s, "===")) return OP_EQ;
    if (!strcmp(s, "!=") || !strcmp(s, "!==")) return OP_NE;
    if (!strcmp(s, "|"))  return OP_BIT_OR;
    if (!strcmp(s, "^"))  return OP_BIT_XOR;
    if (!strcmp(s, "&"))  return OP_BIT_AND;
    if (!strcmp(s, "<<")) return OP_LSHIFT;
    if (!strcmp(s, ">>")) return OP_RSHIFT;
    return -1;
}

/* ── protótipos internos ────────────────────────────────────────────────── */
static void expr(C *c, Unidade *u, PSNode *n);
static void compila_fstring(C *c, Unidade *u, PSNode *n);
static void stmt(C *c, Unidade *u, PSNode *n);
static int32_t compila_action(C *c, PSNode *n);

/* `__init__` sintetizado a partir dos campos tipados (`nome: str`).
 *
 * Não depende do `@dataentity`: o decorador é só um marcador, e no
 * interpretador a Entity ganha o init pelos campos com ou sem ele. Gerar
 * bytecode direto (em vez de montar uma AST) evita inventar nós sem posição
 * de origem, que estragariam a mensagem de erro. */
static int32_t sintetiza_init(C *c, PSNode *entidade);
static void compila_unpack_alvo(C *c, Unidade *u, PSNode *alvo);

static int eh_global_declarada(Unidade *u, const char *nome)
{
    for (int32_t i = 0; i < u->nglobais_decl; i++)
        if (strcmp(u->globais_decl[i], nome) == 0) return 1;
    return 0;
}

static void carrega_nome(C *c, Unidade *u, const char *nome)
{
    if (u->eh_modulo || eh_global_declarada(u, nome)) {
        emite(c, u, OP_LOAD_GLOBAL, idx_global(c, nome));
        return;
    }
    for (int32_t i = 0; i < u->nlocais; i++) {
        if (strcmp(u->locais[i], nome) != 0) continue;
        if (i < 256 && u->certo[i]) emite(c, u, OP_LOAD_LOCAL, i);   /* param/tipada */
        else {
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, i, 0, NULL, 0));
            emite(c, u, OP_LOAD_NAME, idx_global(c, nome));
        }
        return;
    }
    /* nome nunca visto nesta função: reserva slot e resolve em runtime */
    {
        int32_t i = idx_local(c, u, nome);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, i, 0, NULL, 0));
        emite(c, u, OP_LOAD_NAME, idx_global(c, nome));
    }
}

/* `certa` = o nome é local com certeza (parâmetro ou declaração tipada) */
static void guarda_nome_modo(C *c, Unidade *u, const char *nome, int certa)
{
    if (u->eh_modulo || eh_global_declarada(u, nome)) {
        if (u->eh_modulo) mod_criados_add(c, u, nome);  /* p/ escopo de bloco */
        emite(c, u, OP_STORE_GLOBAL, idx_global(c, nome));
        return;
    }
    int32_t i = idx_local(c, u, nome);
    if (certa && i < 256) u->certo[i] = 1;
    if (i < 256 && u->certo[i]) { emite(c, u, OP_STORE_LOCAL, i); return; }
    emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, i, 0, NULL, 0));
    emite(c, u, OP_STORE_NAME, idx_global(c, nome));
}

static void guarda_nome(C *c, Unidade *u, const char *nome)
{
    guarda_nome_modo(c, u, nome, 0);
}

/* ── f-string ───────────────────────────────────────────────────────────── */
/* `f"Ola, {nome}!"` — o template chega como um Literal cru; o corte em
 * pedaços acontece AQUI, em tempo de compilação.
 *
 * O interpretador resolve isso em runtime, reparseando o trecho a cada
 * avaliação. Compilar antes elimina esse custo do laço: o `{expr}` vira
 * bytecode normal, exatamente como se estivesse escrito fora da string.
 *
 * Regras herdadas: chaves aninhadas contam profundidade; `{{` e `}}`
 * escapam para chave literal.
 */
static void compila_fstring(C *c, Unidade *u, PSNode *n)
{
    const char *t = n->texto ? n->texto : "";
    int32_t len = (int32_t)strlen(t);
    int32_t partes = 0;

    char *buf = malloc((size_t)len + 1);
    if (!buf) { cerro(c, "sem memoria na f-string", n); return; }
    int32_t nb = 0;

    for (int32_t i = 0; i < len && !CFALHOU(c); ) {
        if (t[i] == '{' && i + 1 < len && t[i + 1] == '{') { buf[nb++] = '{'; i += 2; continue; }
        if (t[i] == '}' && i + 1 < len && t[i + 1] == '}') { buf[nb++] = '}'; i += 2; continue; }

        if (t[i] == '{') {
            /* fecha o pedaço literal acumulado */
            if (nb > 0) {
                emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_STR, 0, 0, buf, nb));
                partes++; nb = 0;
            }
            int prof = 1;
            int32_t j = i + 1;
            while (j < len && prof > 0) {
                if (t[j] == '{') prof++;
                else if (t[j] == '}') prof--;
                j++;
            }
            if (prof != 0) { free(buf); cerro(c, "chave nao fechada na f-string", n); return; }

            /* trecho entre as chaves: lexa, parseia e compila como expressão */
            int32_t elen = j - 1 - (i + 1);
            char *e = malloc((size_t)elen + 1);
            if (!e) { free(buf); cerro(c, "sem memoria na f-string", n); return; }
            memcpy(e, t + i + 1, (size_t)elen);
            e[elen] = '\0';

            PSTokenList *toks = ps_lexer_tokenize(e, (size_t)elen);
            if (!toks || !toks->ok) {
                if (toks) ps_lexer_free(toks);
                free(e); free(buf);
                cerro(c, "expressao invalida dentro da f-string", n);
                return;
            }
            PSParseResult *r = ps_parse(toks->tokens, toks->n);
            ps_lexer_free(toks);
            if (!r || !r->ok || !r->programa || r->programa->lista.n == 0) {
                if (r) ps_parse_free(r);
                free(e); free(buf);
                cerro(c, "expressao invalida dentro da f-string", n);
                return;
            }
            PSNode *st = r->programa->lista.itens[0];
            PSNode *alvo = (st->kind == N_EXPRESSION_STMT) ? st->a : st;
            /* Trecho que estoura sai como o texto original entre chaves —
             * `f"oi {nome}"` com `nome` indefinido imprime `oi {nome}`. É o
             * que o interpretador faz; sem isto o mesmo `.ps` roda num motor
             * e aborta no outro. Cada trecho tem seu `try` porque o resto da
             * f-string continua válido. */
            int32_t rede_f = emite(c, u, OP_SETUP_TRY, 0);
            expr(c, u, alvo);
            emite(c, u, OP_POP_TRY, 0);
            int32_t fim_f = emite(c, u, OP_JUMP, 0);
            if (rede_f >= 0) UP(c, u)->code[rede_f + 1] = UP(c, u)->ncode;
            emite(c, u, OP_POP_TOP, 0);              /* descarta a mensagem */
            {
                char *cru = malloc((size_t)elen + 3);
                if (!cru) { ps_parse_free(r); free(e); free(buf); cerro(c, "sem memoria na f-string", n); return; }
                cru[0] = '{';
                memcpy(cru + 1, e, (size_t)elen);
                cru[elen + 1] = '}';
                emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_STR, 0, 0, cru, elen + 2));
                free(cru);
            }
            if (fim_f >= 0) UP(c, u)->code[fim_f + 1] = UP(c, u)->ncode;
            /* A AST do trecho vive na arena do parse acima; o bytecode já foi
             * emitido e não guarda ponteiro pra ela, então liberar aqui é
             * seguro (as constantes string foram COPIADAS pro pool). */
            ps_parse_free(r);
            free(e);
            partes++;
            i = j;
            continue;
        }
        buf[nb++] = t[i++];
    }

    if (!CFALHOU(c) && nb > 0) {
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_STR, 0, 0, buf, nb));
        partes++;
    }
    free(buf);
    if (CFALHOU(c)) return;

    if (partes == 0) emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_STR, 0, 0, "", 0));
    else if (partes > 1) emite(c, u, OP_BUILD_STR, partes);
}


/* `count` codifica tipo e "tem valor" num argumento só. `char` (8) só existe
 * aqui; `json` e `dict` são o mesmo tipo. */
static int32_t arg_count(C *c, PSNode *n)
{
    static const char *nomes[] = { "str", "int", "flo", "bool",
                                   "list", "dict", "tup", "type", "char" };
    int32_t t = -1;
    for (int32_t k = 0; k < 9; k++)
        if (n->texto && strcmp(n->texto, nomes[k]) == 0) { t = k; break; }
    if (t < 0 && n->texto && strcmp(n->texto, "json") == 0) t = 5;
    if (t < 0) { cerro(c, "tipo desconhecido em count", n); return -1; }
    return t | ((n->b != NULL) << 8);
}

/* Empilha container e valor filtrado — o valor entra como Null quando não há
 * (o bit do argumento é quem diz se ele conta). */
static void count_operandos(C *c, Unidade *u, PSNode *n)
{
    expr(c, u, n->c);                       /* container */
    if (n->b) expr(c, u, n->b);
    else emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0));
}

/* ── expressões ─────────────────────────────────────────────────────────── */
static void expr(C *c, Unidade *u, PSNode *n)
{
    if (n && n->line) c->linha_atual = n->line;
    if (n && n->col)  c->coluna_atual = n->col;
    if (CFALHOU(c) || !n) return;

    switch (n->kind) {
        case N_LITERAL:
            switch (n->lit) {
                case L_INT:  emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, n->i, 0, NULL, 0)); break;
                case L_FLO:  emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_FLO, 0, n->d, NULL, 0)); break;
                case L_BOOL: emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, n->i, 0, NULL, 0)); break;
                case L_NULL: emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0)); break;
                case L_STR:
                    emite(c, u, OP_LOAD_CONST,
                          idx_const(c, u, K_STR, 0, 0, n->texto ? n->texto : "",
                                    n->texto ? (int32_t)strlen(n->texto) : 0));
                    break;
                case L_BIGINT:
                    emite(c, u, OP_LOAD_CONST,
                          idx_const(c, u, K_BIGINT, 0, 0, n->texto ? n->texto : "0",
                                    n->texto ? (int32_t)strlen(n->texto) : 1));
                    break;
                case L_FSTRING:
                    compila_fstring(c, u, n);
                    break;
            }
            return;

        case N_NAME:
            carrega_nome(c, u, n->texto ? n->texto : "");
            return;

        case N_CONDITIONAL: {
            /* ternário `A if cond else B` (a=A, b=cond, c=B): avalia só o ramo
             * escolhido; deixa UM valor na pilha (like o if statement, mas expr). */
            expr(c, u, n->b);                               /* cond */
            int32_t js = emite(c, u, OP_JUMP_IF_FALSE, 0);  /* pop cond; falso → else */
            expr(c, u, n->a);                               /* then */
            int32_t je = emite(c, u, OP_JUMP, 0);           /* pula o else */
            if (js >= 0) UP(c, u)->code[js + 1] = UP(c, u)->ncode;   /* else: */
            expr(c, u, n->c);                               /* else */
            if (je >= 0) UP(c, u)->code[je + 1] = UP(c, u)->ncode;   /* fim: */
            return;
        }

        case N_BINARY_OP: {
            /* `is`/`in` não estão na tabela de opcode binário: o argumento
             * carrega a negação, então cada um vira uma instrução só em vez
             * de operação + NOT. */
            if (n->texto) {
                if (!strcmp(n->texto, "is") || !strcmp(n->texto, "is not")
                        || !strcmp(n->texto, "not is")) {
                    expr(c, u, n->a);
                    expr(c, u, n->b);
                    emite(c, u, OP_IS, strcmp(n->texto, "is") != 0);
                    return;
                }
                if (!strcmp(n->texto, "in") || !strcmp(n->texto, "not in")) {
                    expr(c, u, n->a);
                    expr(c, u, n->b);
                    emite(c, u, OP_IN, strcmp(n->texto, "in") != 0);
                    return;
                }
            }
            /* `and`/`or` curto-circuitam: o lado direito só é avaliado se
             * precisar. Por isso não são opcode binário — viram salto.
             * O resultado é BOOL, não o operando: `0 or 5` é `True`. */
            if (n->texto && (!strcmp(n->texto, "and") || !strcmp(n->texto, "&&")
                          || !strcmp(n->texto, "or")  || !strcmp(n->texto, "||"))) {
                int eh_and = (n->texto[0] == 'a' || n->texto[0] == '&');
                expr(c, u, n->a);
                int32_t curto = emite(c, u, eh_and ? OP_JUMP_IF_FALSE : OP_JUMP_IF_TRUE, 0);
                expr(c, u, n->b);
                emite(c, u, OP_TO_BOOL, 0);
                int32_t pula_fim = emite(c, u, OP_JUMP, 0);
                if (curto >= 0) UP(c, u)->code[curto + 1] = UP(c, u)->ncode;
                emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, !eh_and, 0, NULL, 0));
                if (pula_fim >= 0) UP(c, u)->code[pula_fim + 1] = UP(c, u)->ncode;
                return;
            }
            int32_t op = op_binario(n->texto);
            if (op < 0) { cerro(c, "operador binario ainda nao compila na VM", n); return; }
            expr(c, u, n->a);
            expr(c, u, n->b);
            emite(c, u, op, 0);
            return;
        }

        case N_UNARY_OP:
            expr(c, u, n->a);
            if (!n->texto) return;
            if (!strcmp(n->texto, "-"))      emite(c, u, OP_NEG, 0);
            else if (!strcmp(n->texto, "~")) emite(c, u, OP_BIT_NOT, 0);
            else if (!strcmp(n->texto, "+")) { /* no-op */ }
            else if (!strcmp(n->texto, "not") || !strcmp(n->texto, "Not")
                     || !strcmp(n->texto, "!")) emite(c, u, OP_NOT, 0);
            else cerro(c, "operador unario ainda nao compila na VM", n);
            return;

        case N_CALL: {
            int32_t nkw = 0;
            for (int32_t i = 0; i < n->lista.n; i++)
                if (n->lista.itens[i]->texto) nkw++;
            /* nomeado só depois de posicional, como no Python */
            if (nkw > 0) {
                int viu_nome = 0;
                for (int32_t i = 0; i < n->lista.n; i++) {
                    if (n->lista.itens[i]->texto) viu_nome = 1;
                    else if (viu_nome) {
                        cerro(c, "argumento posicional depois de nomeado", n);
                        return;
                    }
                }
            }
            expr(c, u, n->a);
            for (int32_t i = 0; i < n->lista.n; i++)
                expr(c, u, n->lista.itens[i]->a);
            if (nkw == 0) { emite(c, u, OP_CALL, n->lista.n); return; }

            /* nomes dos kwargs entram como uma tupla de constantes */
            for (int32_t i = n->lista.n - nkw; i < n->lista.n; i++) {
                const char *nm = n->lista.itens[i]->texto;
                emite(c, u, OP_LOAD_CONST,
                      idx_const(c, u, K_STR, 0, 0, nm, (int32_t)strlen(nm)));
            }
            emite(c, u, OP_BUILD_TUPLE, nkw);
            emite(c, u, OP_CALL_KW, n->lista.n);
            return;
        }

        case N_LIST_LITERAL:
            for (int32_t i = 0; i < n->lista.n; i++) expr(c, u, n->lista.itens[i]);
            emite(c, u, OP_BUILD_LIST, n->lista.n);
            return;

        case N_DICT_LITERAL:
            for (int32_t i = 0; i < n->lista.n; i++) {
                PSNode *e = n->lista.itens[i];
                /* chave sem aspas (`{nome: 1}`) chega como Name e vira
                 * string aqui — é o parser que a mantém como Name */
                if (e->a && e->a->kind == N_NAME)
                    emite(c, u, OP_LOAD_CONST,
                          idx_const(c, u, K_STR, 0, 0, e->a->texto ? e->a->texto : "",
                                    e->a->texto ? (int32_t)strlen(e->a->texto) : 0));
                else
                    expr(c, u, e->a);
                expr(c, u, e->b);
            }
            emite(c, u, OP_BUILD_DICT, n->lista.n);
            return;

        case N_INDEX_ACCESS:
            expr(c, u, n->a);
            expr(c, u, n->b);
            emite(c, u, OP_INDEX_GET, 0);
            return;

        case N_TUPLE_LITERAL:
            for (int32_t i = 0; i < n->lista.n; i++) expr(c, u, n->lista.itens[i]);
            emite(c, u, OP_BUILD_TUPLE, n->lista.n);
            return;

        case N_INTERPOLATED_STRING:
            /* `"a" {x} "b"` — as partes já vêm separadas pelo parser; o
             * BUILD_STR concatena usando as MESMAS regras de texto do post. */
            for (int32_t i = 0; i < n->lista.n; i++) expr(c, u, n->lista.itens[i]);
            emite(c, u, OP_BUILD_STR, n->lista.n);
            return;

        case N_SLICE_ACCESS: {
            /* parte ausente vira Null — a VM normaliza como o Python */
            expr(c, u, n->a);
            if (n->b) expr(c, u, n->b);
            else emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0));
            if (n->c) expr(c, u, n->c);
            else emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0));
            if (n->e) expr(c, u, n->e);
            else emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0));
            emite(c, u, OP_SLICE, 0);
            return;
        }

        case N_COLOR_STR_EXPR: {
            /* `<red>"txt"` vira UMA constante já com os escapes ANSI: a cor é
             * conhecida em compilação, então não há o que decidir em runtime.
             * Hex de 4 ou 5 dígitos e nome desconhecido não são cor — o lexer
             * já teria voltado o `<` a ser "menor que", então aqui só chega
             * cor válida. */
            static const struct { const char *nome, *hex; } CORES[] = {
                {"red","FF3B30"}, {"green","34C759"}, {"blue","2196F3"},
                {"yellow","FFD60A"}, {"cyan","5AC8FA"}, {"magenta","FF2D55"},
                {"white","FFFFFF"}, {"black","000000"}, {"purple","AF52DE"},
                {"orange","FF9500"}, {"pink","FF2D55"}, {"gray","8E8E93"},
                {"grey","8E8E93"}, {"lime","30D158"}, {"teal","5AC8FA"},
            };
            const char *cor = n->texto ? n->texto : "";
            char hex[7] = {0};
            for (size_t k = 0; k < sizeof(CORES)/sizeof(CORES[0]); k++) {
                int igual = 1;
                for (const char *a = cor, *b = CORES[k].nome; ; a++, b++) {
                    char ca = (*a >= 'A' && *a <= 'Z') ? (char)(*a + 32) : *a;
                    if (ca != *b) { igual = 0; break; }
                    if (!*b) break;
                }
                if (igual) { memcpy(hex, CORES[k].hex, 6); break; }
            }
            if (!hex[0]) {
                size_t ln = strlen(cor);
                if (ln == 6) memcpy(hex, cor, 6);
                else if (ln == 3) { for (int k = 0; k < 3; k++) { hex[k*2] = cor[k]; hex[k*2+1] = cor[k]; } }
                else { cerro(c, "cor invalida", n); return; }
            }
            unsigned r = 0, g = 0, b = 0;
            if (sscanf(hex, "%2x%2x%2x", &r, &g, &b) != 3) { cerro(c, "cor invalida", n); return; }

            /* String SIMPLES (literal): a cor é conhecida em compilação, então
             * vira UMA constante já com os escapes ANSI — caminho rápido. */
            if (n->a && n->a->kind == N_LITERAL && n->a->lit == L_STR) {
                const char *txt = n->a->texto ? n->a->texto : "";
                char buf[1024];
                int len = snprintf(buf, sizeof(buf), "\033[38;2;%u;%u;%um%s\033[0m", r, g, b, txt);
                if (len < 0 || len >= (int)sizeof(buf)) { cerro(c, "string colorida longa demais", n); return; }
                emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_STR, 0, 0, buf, len));
                return;
            }

            /* Interno DINÂMICO (f-string, interpolação `"txt" {x}`, variável,
             * etc.): o valor só existe em runtime. Concatena PREFIXO + interno
             * + RESET. O interno passa por `str(...)` primeiro (via OP_LOAD_TIPO
             * str + OP_CALL, o MESMO que `str(x)` compila) — assim `<green>x`
             * colore QUALQUER tipo, igual o interp faz. Sem isso, x int/flo dava
             * "'+' entre tipos incompativeis" (o + interno da tag, que o usuário
             * nunca escreveu — ele só está imprimindo). */
            char pref[32];
            int pl = snprintf(pref, sizeof(pref), "\033[38;2;%u;%u;%um", r, g, b);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_STR, 0, 0, pref, pl));
            emite(c, u, OP_LOAD_TIPO, 0);     /* 0 = tipo str */
            expr(c, u, n->a);                 /* compila o interno DE VERDADE */
            emite(c, u, OP_CALL, 1);          /* str(interno) */
            emite(c, u, OP_ADD, 0);           /* prefixo + str(interno) */
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_STR, 0, 0, "\033[0m", 4));
            emite(c, u, OP_ADD, 0);           /* + reset */
            return;
        }

        case N_LAMBDA_EXPR: {
            /* Mesma máquina da action nomeada: um protótipo e um
             * MAKE_FUNCTION. A diferença é só não ter nome pra guardar. */
            int32_t pi = compila_action(c, n);
            if (CFALHOU(c)) return;
            emite(c, u, OP_MAKE_FUNCTION, pi);
            return;
        }

        case N_TYPE_NAME: {
            /* `json` e `dict` são o MESMO tipo — apelido, não dois. */
            static const char *nomes[] = { "str", "int", "flo", "bool",
                                           "list", "dict", "tup", "type" };
            int32_t t = -1;
            for (int32_t k = 0; k < 8; k++)
                if (n->texto && strcmp(n->texto, nomes[k]) == 0) { t = k; break; }
            if (t < 0 && n->texto && strcmp(n->texto, "json") == 0) t = 5;
            if (t < 0) { cerro(c, "tipo desconhecido", n); return; }
            emite(c, u, OP_LOAD_TIPO, t);
            return;
        }

        case N_COUNT_EXPR:
        case N_COUNT_EACH_EXPR: {
            int32_t a = arg_count(c, n);
            if (a < 0) return;
            count_operandos(c, u, n);
            emite(c, u, OP_COUNT, a);
            return;
        }

        case N_MEMBER_ACCESS:
            expr(c, u, n->a);
            emite(c, u, OP_GET_MEMBER,
                  idx_const(c, u, K_STR, 0, 0, n->texto ? n->texto : "",
                            n->texto ? (int32_t)strlen(n->texto) : 0));
            return;

        case N_BASE_CALL_NODE: {
            /* `base(v)` chama o __init__ do PRIMEIRO pai com o self atual.
             * Emite a classe pai explicitamente: buscar pela instância
             * acharia o override da filha e recursaria pra sempre. */
            const char *pai = n->texto2 ? n->texto2 : c->entity_pai;
            if (!pai) {
                /* Entity SEM herança chamando `base()`: não há o que
                 * inicializar — no-op que avalia pra Null, como no
                 * interpretador. Fora de Entity continua erro. */
                if (c->dentro_entity) {
                    for (int32_t i = 0; i < n->lista.n; i++)
                        if (!n->lista.itens[i]->texto) expr(c, u, n->lista.itens[i]->a);
                    for (int32_t i = 0; i < n->lista.n; i++)
                        emite(c, u, OP_POP_TOP, 0);
                    emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0));
                    return;
                }
                cerro(c, "base() fora de Entity com heranca", n);
                return;
            }
            carrega_nome(c, u, pai);
            emite(c, u, OP_LOAD_SELF, 0);
            for (int32_t i = 0; i < n->lista.n; i++) {
                if (n->lista.itens[i]->texto) { cerro(c, "base() nao aceita argumento nomeado", n); return; }
                expr(c, u, n->lista.itens[i]->a);
            }
            emite(c, u, OP_CALL_BASE, n->lista.n);
            return;
        }

        case N_POSTFIX_OP: {
            /* `x++` devolve o valor ANTIGO e guarda o novo — por isso o DUP
             * antes de somar (é o que o interpretador faz). */
            if (!n->a || n->a->kind != N_NAME) {
                cerro(c, "'++'/'--' so funcionam em variaveis", n);
                return;
            }
            const char *nome = n->a->texto ? n->a->texto : "";
            carrega_nome(c, u, nome);
            emite(c, u, OP_DUP, 0);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 1, 0, NULL, 0));
            emite(c, u, (n->texto && n->texto[0] == '+') ? OP_ADD : OP_SUB, 0);
            guarda_nome(c, u, nome);
            return;
        }

        default:
            cerro(c, "expressao ainda nao compila na VM", n);
            return;
    }
}

/* ── laços: pilha para backpatch de break/continue ──────────────────────── */
static void abre_laco(C *c, int32_t inicio, int slots_pilha)
{
    if (c->nlacos >= MAX_LACOS) return;      /* laço profundo demais: break/continue
                                              * caem no erro "fora de laco" */
    Laco *l = &c->lacos[c->nlacos++];
    l->inicio = inicio;
    l->nsaidas = 0;
    l->ncontinues = 0;
    l->slots_pilha = slots_pilha;
}

static void fecha_laco(C *c, Unidade *u, int32_t inicio)
{
    if (c->nlacos == 0) return;
    Laco *l = &c->lacos[--c->nlacos];
    int32_t fim = UP(c, u)->ncode;
    for (int32_t i = 0; i < l->nsaidas; i++)
        UP(c, u)->code[l->saidas[i] + 1] = fim;
    for (int32_t i = 0; i < l->ncontinues; i++)
        UP(c, u)->code[l->continues[i] + 1] = inicio;
}

/* ── padrões de match ───────────────────────────────────────────────────── */
/* Invariante: o valor a testar está no TOPO. O teste consome esse valor e
 * deixa um booleano no lugar. Manter isso uniforme é o que permite compor
 * padrões (lista dentro de lista, `|` entre quaisquer dois) sem caso
 * especial em cada combinação. */
static void padrao_testa(C *c, Unidade *u, PSNode *pat);

static void padrao_literal(C *c, Unidade *u, PSNode *lit)
{
    if (!lit) { emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0)); return; }
    switch (lit->lit) {
        case L_INT:  emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, lit->i, 0, NULL, 0)); break;
        case L_FLO:  emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_FLO, 0, lit->d, NULL, 0)); break;
        case L_BOOL: emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, lit->i, 0, NULL, 0)); break;
        case L_NULL: emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0)); break;
        case L_BIGINT: emite(c, u, OP_LOAD_CONST,
                  idx_const(c, u, K_BIGINT, 0, 0, lit->texto ? lit->texto : "0",
                            lit->texto ? (int32_t)strlen(lit->texto) : 1)); break;
        default:
            emite(c, u, OP_LOAD_CONST,
                  idx_const(c, u, K_STR, 0, 0, lit->texto ? lit->texto : "",
                            lit->texto ? (int32_t)strlen(lit->texto) : 0));
            break;
    }
}

static void padrao_testa(C *c, Unidade *u, PSNode *pat)
{
    if (CFALHOU(c) || !pat) return;
    const char *k = pat->texto ? pat->texto : "";

    if (!strcmp(k, "wildcard")) {
        emite(c, u, OP_POP_TOP, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 1, 0, NULL, 0));
        return;
    }
    if (!strcmp(k, "value")) {
        padrao_literal(c, u, pat->b);
        emite(c, u, OP_EQ, 0);
        return;
    }
    if (!strcmp(k, "capture")) {
        /* liga o valor ao nome e sempre casa */
        guarda_nome_modo(c, u, pat->texto2 ? pat->texto2 : "_", 1);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 1, 0, NULL, 0));
        return;
    }
    if (!strcmp(k, "or")) {
        /* Testa cada alternativa com uma CÓPIA e para no primeiro acerto.
         * A última não é duplicada: o booleano dela já é o resultado final,
         * então cai direto no fim. Os dois caminhos precisam terminar com
         * exatamente um booleano na pilha. */
        int32_t acertos[32];
        int nac = 0;
        int32_t fim_ok[32];
        int nfo = 0;
        for (int32_t i = 0; i < pat->lista.n && !CFALHOU(c); i++) {
            int ultimo = (i == pat->lista.n - 1);
            if (!ultimo) {
                emite(c, u, OP_DUP, 0);
                padrao_testa(c, u, pat->lista.itens[i]);
                if (nac < 32) acertos[nac++] = emite(c, u, OP_JUMP_IF_TRUE, 0);
            } else {
                padrao_testa(c, u, pat->lista.itens[i]);
                if (nfo < 32) fim_ok[nfo++] = emite(c, u, OP_JUMP, 0);
            }
        }
        /* alvo dos acertos: ainda resta a cópia original pra descartar */
        int32_t alvo_acerto = UP(c, u)->ncode;
        for (int i = 0; i < nac; i++) UP(c, u)->code[acertos[i] + 1] = alvo_acerto;
        emite(c, u, OP_POP_TOP, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 1, 0, NULL, 0));

        int32_t fim = UP(c, u)->ncode;
        for (int i = 0; i < nfo; i++) UP(c, u)->code[fim_ok[i] + 1] = fim;
        return;
    }
    if (!strcmp(k, "list")) {
        /* tamanho tem que bater, depois cada item */
        int32_t falhas[64];
        int nf = 0;
        emite(c, u, OP_DUP, 0);
        emite(c, u, OP_LEN, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, pat->lista.n, 0, NULL, 0));
        emite(c, u, OP_EQ, 0);
        if (nf < 64) falhas[nf++] = emite(c, u, OP_JUMP_IF_FALSE, 0);

        for (int32_t i = 0; i < pat->lista.n && !CFALHOU(c); i++) {
            emite(c, u, OP_DUP, 0);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, i, 0, NULL, 0));
            emite(c, u, OP_INDEX_GET, 0);
            padrao_testa(c, u, pat->lista.itens[i]);
            if (nf < 64) falhas[nf++] = emite(c, u, OP_JUMP_IF_FALSE, 0);
        }
        emite(c, u, OP_POP_TOP, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 1, 0, NULL, 0));
        int32_t fim = emite(c, u, OP_JUMP, 0);
        int32_t alvo_falha = UP(c, u)->ncode;
        for (int i = 0; i < nf; i++) UP(c, u)->code[falhas[i] + 1] = alvo_falha;
        emite(c, u, OP_POP_TOP, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 0, 0, NULL, 0));
        UP(c, u)->code[fim + 1] = UP(c, u)->ncode;
        return;
    }
    if (!strcmp(k, "dict")) {
        /* cada chave precisa existir E o valor casar */
        int32_t falhas[64];
        int nf = 0;
        for (int32_t i = 0; i < pat->lista2.n && !CFALHOU(c); i++) {
            const char *chave = pat->lista2.itens[i]->texto;
            int32_t kc = idx_const(c, u, K_STR, 0, 0, chave ? chave : "",
                                   chave ? (int32_t)strlen(chave) : 0);
            emite(c, u, OP_DUP, 0);
            emite(c, u, OP_LOAD_CONST, kc);
            emite(c, u, OP_HAS_KEY, 0);
            if (nf < 64) falhas[nf++] = emite(c, u, OP_JUMP_IF_FALSE, 0);
            emite(c, u, OP_DUP, 0);
            emite(c, u, OP_LOAD_CONST, kc);
            emite(c, u, OP_INDEX_GET, 0);
            padrao_testa(c, u, pat->lista.itens[i]);
            if (nf < 64) falhas[nf++] = emite(c, u, OP_JUMP_IF_FALSE, 0);
        }
        emite(c, u, OP_POP_TOP, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 1, 0, NULL, 0));
        int32_t fim = emite(c, u, OP_JUMP, 0);
        int32_t alvo_falha = UP(c, u)->ncode;
        for (int i = 0; i < nf; i++) UP(c, u)->code[falhas[i] + 1] = alvo_falha;
        emite(c, u, OP_POP_TOP, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 0, 0, NULL, 0));
        UP(c, u)->code[fim + 1] = UP(c, u)->ncode;
        return;
    }
    cerro(c, "padrao de match ainda nao compila na VM", pat);
}

/* ── statements ─────────────────────────────────────────────────────────── */
static void bloco_stmts(C *c, Unidade *u, PSNode *b)
{
    if (!b) return;
    for (int32_t i = 0; i < b->lista.n && !CFALHOU(c); i++)
        stmt(c, u, b->lista.itens[i]);
}

static void stmt(C *c, Unidade *u, PSNode *n)
{
    if (CFALHOU(c) || !n) return;
    if (n->line) c->linha_atual = n->line;
    if (n->col)  c->coluna_atual = n->col;

    switch (n->kind) {
        case N_ACTION_DECL: {
            int32_t idx = compila_action(c, n);
            if (CFALHOU(c)) return;
            emite(c, u, OP_MAKE_FUNCTION, idx);
            guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            return;
        }

        case N_VAR_DECL: {
            /* declaração tipada cria local, sempre — não sobe escopo */
            expr(c, u, n->a);
            /* Só os quatro escalares são checados: `list x = (1,2)` guarda a
             * tupla sem reclamar, no interpretador também. */
            static const char *ESC[] = { "str", "int", "flo", "bool" };
            for (int k = 0; k < 4; k++)
                if (n->texto2 && !strcmp(n->texto2, ESC[k])) {
                    /* Empacota nome+tipo num só operando: tipo nos 2 bits baixos
                     * (0-3), índice do nome (const string) no resto. O VM usa o
                     * nome pra dizer "variável X esperava T", igual ao interp. */
                    const char *vn = n->texto ? n->texto : "";
                    int32_t ni = idx_const(c, u, K_STR, 0, 0, vn, (int32_t)strlen(vn));
                    /* aponta o erro no INÍCIO do valor (RHS), não na sub-expressão
                     * mais profunda que o expr() deixou em coluna_atual — é o
                     * lugar EXATO do erro, igual ao node.value do interp. */
                    if (n->a) {
                        if (n->a->line) c->linha_atual  = n->a->line;
                        if (n->a->col)  c->coluna_atual = n->a->col;
                    }
                    emite(c, u, OP_COERCE_DECL, (ni << 2) | k);
                    break;
                }
            guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            return;
        }

        case N_ASSIGNMENT: {
            const char *op = n->texto2 ? n->texto2 : "=";
            if (strcmp(op, "=") != 0) {
                /* `x += v` → carrega x, calcula, guarda */
                char base[3] = { op[0], '\0', '\0' };
                int32_t opc = op_binario(base);
                if (opc < 0) { cerro(c, "operador de atribuicao invalido", n); return; }
                carrega_nome(c, u, n->texto ? n->texto : "");
                expr(c, u, n->a);
                emite(c, u, opc, 0);
            } else {
                expr(c, u, n->a);
            }
            guarda_nome(c, u, n->texto ? n->texto : "");
            return;
        }

        case N_EXPRESSION_STMT:
            expr(c, u, n->a);
            emite(c, u, OP_POP_TOP, 0);
            return;

        case N_RETURN_STMT:
            /* `return;` seco dentro de `count each` devolve o TOTAL contado,
             * não Null — é o que faz `count each ... { return; }` ser a forma
             * curta de "conte e me dê o número". `return <expr>` segue
             * normal. */
            if (!n->a && c->dentro_count_each > 0) carrega_nome(c, u, "_count");
            else if (n->a) expr(c, u, n->a);
            else      emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0));
            if (u->tipo_ret) emite(c, u, OP_COERCE_RET, u->tipo_ret);
            emite(c, u, OP_RETURN, 0);
            return;

        case N_BLOCK: {
            int32_t M = escopo_marca(u);
            bloco_stmts(c, u, n);
            escopo_fecha(c, u, M);   /* variáveis do bloco não vazam */
            return;
        }

        case N_IF_STMT: {
            int32_t fins[64];
            int nfins = 0;
            for (int32_t i = 0; i < n->lista.n && !CFALHOU(c); i++) {
                PSNode *ramo = n->lista.itens[i];
                if (!ramo->a) {                        /* else */
                    int32_t Me = escopo_marca(u);
                    bloco_stmts(c, u, ramo->b);
                    escopo_fecha(c, u, Me);
                    break;
                }
                expr(c, u, ramo->a);                   /* condição: fora do escopo do corpo */
                int32_t salto_falso = emite(c, u, OP_JUMP_IF_FALSE, 0);
                int32_t M = escopo_marca(u);
                bloco_stmts(c, u, ramo->b);
                escopo_fecha(c, u, M);
                if (nfins < 64) fins[nfins++] = emite(c, u, OP_JUMP, 0);
                if (salto_falso >= 0) UP(c, u)->code[salto_falso + 1] = UP(c, u)->ncode;
            }
            for (int k = 0; k < nfins; k++) UP(c, u)->code[fins[k] + 1] = UP(c, u)->ncode;
            return;
        }

        case N_WHILE_STMT: {
            int32_t M = escopo_marca(u);        /* sem var de laço: corpo == laço */
            int32_t topo = UP(c, u)->ncode;
            expr(c, u, n->a);
            int32_t sai = emite(c, u, OP_JUMP_IF_FALSE, 0);
            abre_laco(c, topo, 0);
            c->lacos[c->nlacos - 1].escopo_marca = M;
            c->lacos[c->nlacos - 1].escopo_marca_body = M;
            bloco_stmts(c, u, n->b);
            escopo_emite_clears(c, u, M);        /* reset por-iteração */
            emite(c, u, OP_JUMP, topo);
            if (sai >= 0) UP(c, u)->code[sai + 1] = UP(c, u)->ncode;
            fecha_laco(c, u, topo);              /* break/saída normal caem aqui */
            escopo_emite_clears(c, u, M);        /* limpa o que sobrou na saída */
            escopo_trunca(u, M);
            return;
        }

        case N_FOR_EACH_STMT: {
            /* Desaçucara em (container, índice) na pilha + ITER_NEXT.
             *   <iteravel>            ; container
             *   LOAD_CONST 0          ; indice
             * topo:
             *   ITER_NEXT fim         ; empurra item, ou limpa e salta
             *   STORE <var>
             *   <corpo>
             *   JUMP topo
             * fim: */
            int32_t M = escopo_marca(u);        /* marca ANTES da var do laço */
            expr(c, u, n->a);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 0, 0, NULL, 0));
            int32_t topo = UP(c, u)->ncode;
            int32_t fim = emite(c, u, OP_ITER_NEXT, 0);
            /* variável do laço é local desta função, como o parâmetro */
            guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            abre_laco(c, topo, 2);      /* container + indice */
            /* a var do laço é re-atribuída no topo a cada volta, então limpá-la
             * por-iteração é inofensivo — corpo e var compartilham a marca */
            c->lacos[c->nlacos - 1].escopo_marca = M;
            c->lacos[c->nlacos - 1].escopo_marca_body = M;
            bloco_stmts(c, u, n->b);
            escopo_emite_clears(c, u, M);        /* reset por-iteração */
            emite(c, u, OP_JUMP, topo);
            if (fim >= 0) UP(c, u)->code[fim + 1] = UP(c, u)->ncode;
            fecha_laco(c, u, topo);
            escopo_emite_clears(c, u, M);        /* saída: var do laço não vaza */
            escopo_trunca(u, M);
            return;
        }

        case N_UNPACK_ASSIGNMENT: {
            /* alvo aninhado `a, (b, c) = ...` desempacota de novo por alvo */
            PSNode *alvo = n->a;
            if (!alvo || alvo->kind != N_UNPACK_TARGET) { cerro(c, "alvo de desempacotamento invalido", n); return; }
            expr(c, u, n->b);
            compila_unpack_alvo(c, u, alvo);
            return;
        }

        case N_YIELD_STMT: {
            /* Marca a unidade atual: quem chama precisa saber que é gerador
             * ANTES de montar o frame. Como a marca é no protótipo e o
             * `yield` aparece durante o corpo, ela é escrita aqui. */
            UP(c, u)->eh_gerador = 1;
            if (n->a) expr(c, u, n->a);
            else emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0));
            emite(c, u, OP_YIELD, 0);
            return;
        }

        case N_MODEL_DECL: {
            /* O model vira um descritor no programa; MAKE_MODEL o instancia
             * em runtime e a validação acontece no `==` (dict contra model). */
            static const char *tipos_m[] = { "str", "int", "flo", "bool",
                                             "list", "dict", "tup", "type" };
            if (c->out->nmodels + 1 > c->cap_models) {
                int32_t novo = c->cap_models < 8 ? 8 : c->cap_models * 2;
                PSModelDef *nm = realloc(c->out->models, sizeof(PSModelDef) * (size_t)novo);
                if (!nm) { cerro(c, "sem memoria", n); return; }
                c->out->models = nm;
                c->cap_models = novo;
            }
            int32_t mi = c->out->nmodels++;
            PSModelDef *def = &c->out->models[mi];
            memset(def, 0, sizeof(*def));
            def->nome = strdup(n->texto ? n->texto : "?");
            def->ncampos = n->lista.n;
            def->campos = n->lista.n > 0
                        ? calloc((size_t)n->lista.n, sizeof(PSModelCampoDef)) : NULL;
            for (int32_t i = 0; i < n->lista.n; i++) {
                PSNode *f = n->lista.itens[i];
                def->campos[i].nome = strdup(f->texto ? f->texto : "?");
                def->campos[i].length = (f->i2 > 0) ? f->i2 : -1;
                int32_t t = -1;
                for (int32_t k = 0; k < 8; k++)
                    if (f->texto2 && strcmp(f->texto2, tipos_m[k]) == 0) { t = k; break; }
                if (t < 0 && f->texto2 && strcmp(f->texto2, "json") == 0) t = 5;
                if (t < 0) { cerro(c, "tipo desconhecido em model", f); return; }
                def->campos[i].tipo = t;
            }
            emite(c, u, OP_MAKE_MODEL, mi);
            guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            return;
        }

        case N_ENUM_DECL: {
            /* Descritor no programa (nome + nomes dos membros + flag auto);
             * os valores EXPLÍCITOS são expressões, empilhadas em ordem de
             * membro antes do MAKE_ENUM, que aplica a auto-numeração. */
            if (c->out->nenums + 1 > c->cap_enums) {
                int32_t novo = c->cap_enums < 8 ? 8 : c->cap_enums * 2;
                PSEnumDef *ne = realloc(c->out->enums, sizeof(PSEnumDef) * (size_t)novo);
                if (!ne) { cerro(c, "sem memoria", n); return; }
                c->out->enums = ne;
                c->cap_enums = novo;
            }
            int32_t ei = c->out->nenums++;
            PSEnumDef *def = &c->out->enums[ei];
            memset(def, 0, sizeof(*def));
            def->nome = strdup(n->texto ? n->texto : "?");
            def->nmembros = n->lista.n;
            def->membros = n->lista.n > 0
                         ? calloc((size_t)n->lista.n, sizeof(PSEnumMembroDef)) : NULL;
            for (int32_t i = 0; i < n->lista.n; i++) {
                PSNode *m = n->lista.itens[i];
                def->membros[i].nome = strdup(m->texto ? m->texto : "?");
                def->membros[i].tem_valor = (m->a != NULL);
            }
            /* empilha os valores explícitos, na ordem de declaração */
            for (int32_t i = 0; i < n->lista.n && !CFALHOU(c); i++) {
                PSNode *m = n->lista.itens[i];
                if (m->a) expr(c, u, m->a);
            }
            if (CFALHOU(c)) return;
            emite(c, u, OP_MAKE_ENUM, ei);
            guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            return;
        }

        case N_DECORATOR_STMT: {
            /* O decorador é resolvido em COMPILAÇÃO, não em runtime: os três
             * que a linguagem define mudam como a action é gerada, não o que
             * ela devolve. Decorador desconhecido não registra a action
             * nenhuma — é o que o interpretador faz, e por isso `f()` depois
             * dá "variável não definida" em vez de rodar sem o decorador. */
            PSNode *dec = n->a;
            const char *nome = (dec && dec->lista.n == 1) ? dec->lista.itens[0]->texto : NULL;
            if (nome && strcmp(nome, "dataentity") == 0) {
                if (n->b) bloco_stmts(c, u, n->b);
                return;
            }
            /* Decorador GERAL `@obj.metodo(args)` / `@obj.prop` (jinker):
             * avalia a expressão -> um registrar descartável; roda o bloco
             * (define a action); e chama `registrar.register(action)`. É o
             * mesmo protocolo do interpretador (registrar.register(handler)),
             * com o embrulho de request/retorno feito no lado do servidor. */
            if (dec && dec->lista.n > 1) {
                if (!n->b) return;
                const char *act = NULL;
                if (n->b->kind == N_BLOCK)
                    for (int32_t i = 0; i < n->b->lista.n; i++)
                        if (n->b->lista.itens[i]->kind == N_ACTION_DECL) { act = n->b->lista.itens[i]->texto; break; }

                /* 1) avalia SEMPRE a expressão do decorador como CHAMADA
                 * `obj.metodo(args)` — é o que o interpretador faz (erro do
                 * decorador propaga, mesmo com 0 args). */
                carrega_nome(c, u, dec->lista.itens[0]->texto);
                for (int32_t i = 1; i < dec->lista.n; i++)
                    emite(c, u, OP_GET_MEMBER,
                          idx_const(c, u, K_STR, 0, 0, dec->lista.itens[i]->texto,
                                    (int32_t)strlen(dec->lista.itens[i]->texto)));
                int32_t nkw = 0;
                for (int32_t i = 0; i < dec->lista2.n; i++)
                    if (dec->lista2.itens[i]->texto) nkw++;
                for (int32_t i = 0; i < dec->lista2.n; i++)
                    expr(c, u, dec->lista2.itens[i]->a);
                if (nkw == 0) emite(c, u, OP_CALL, dec->lista2.n);
                else {
                    for (int32_t i = dec->lista2.n - nkw; i < dec->lista2.n; i++) {
                        const char *nm = dec->lista2.itens[i]->texto;
                        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_STR, 0, 0, nm, (int32_t)strlen(nm)));
                    }
                    emite(c, u, OP_BUILD_TUPLE, nkw);
                    emite(c, u, OP_CALL_KW, dec->lista2.n);
                }
                /* 2) guarda o registrar (descartável) e roda o bloco */
                guarda_nome_modo(c, u, "$reg", 1);
                bloco_stmts(c, u, n->b);
                /* 3) registrar.register(action) — só se o bloco define action */
                if (act) {
                    carrega_nome(c, u, "$reg");
                    emite(c, u, OP_GET_MEMBER, idx_const(c, u, K_STR, 0, 0, "register", 8));
                    carrega_nome(c, u, act);
                    emite(c, u, OP_CALL, 1);
                    emite(c, u, OP_POP_TOP, 0);
                }
                return;
            }
            /* `@qualquer` sozinho, sem action embaixo, é ignorado — o
             * interpretador aceita e segue. Recusar quebrava script válido. */
            if (!n->b) return;
            if (nome && strcmp(nome, "static") == 0) {
                /* `@static` fora de Entity é só uma action comum */
                bloco_stmts(c, u, n->b);
                return;
            }
            if (nome && strcmp(nome, "NonNull") == 0) {
                c->pendente_nonnull = 1;
                bloco_stmts(c, u, n->b);
                c->pendente_nonnull = 0;
                return;
            }
            if (nome && strcmp(nome, "dataentity") == 0) {
                /* Marcador: quem gera o `__init__` são os campos tipados, com
                 * ou sem o decorador. Se vier bloco junto, compila o bloco. */
                if (n->b) bloco_stmts(c, u, n->b);
                return;
            }
            /* desconhecido: nada é registrado */
            return;
        }

        case N_RUN_SELFWITH_STMT: {
            /* Igual ao interp: o bloco é PULADO quando o arquivo está sendo
             * IMPORTADO (só roda quando é o principal). OP_SKIP_IF_IMPORT
             * salta o bloco em runtime se vm->importando > 0. */
            int32_t s = emite(c, u, OP_SKIP_IF_IMPORT, 0);
            int32_t Mr = escopo_marca(u);
            bloco_stmts(c, u, n->b ? n->b : n->a);
            escopo_fecha(c, u, Mr);            /* vars do run_selfwith_ não vazam */
            if (s >= 0) UP(c, u)->code[s + 1] = UP(c, u)->ncode;   /* alvo = pós-bloco */
            return;
        }

        case N_USING_STMT: {
            /* `using <expr> as f { ... }` — abre, roda, fecha. O fechamento é
             * emitido nos DOIS caminhos (fim normal e erro), como o `finally`,
             * porque sub-rotina exigiria opcode de chamada interna. */
            /* Abre e liga o `f` ANTES do try: se a própria aquisição falhar
             * (ex.: open() com arg inválido), o erro real tem que propagar —
             * senão a limpeza faria LOAD de um `f` nunca gravado e mascararia
             * tudo com "variavel nao definida". O try cobre só o corpo. */
            int32_t Mu = escopo_marca(u);      /* escopo da var do using (f) */
            expr(c, u, n->a);
            guarda_nome_modo(c, u, n->texto ? n->texto : "_", 1);
            int32_t Mub = escopo_marca(u);     /* escopo do corpo */
            int32_t setup = emite(c, u, OP_SETUP_TRY, 0);
            c->dentro_try++;
            bloco_stmts(c, u, n->b);
            escopo_fecha(c, u, Mub);           /* vars do corpo não vazam */
            c->dentro_try--;
            emite(c, u, OP_POP_TRY, 0);

            /* saída normal: fecha e pula o caminho de erro */
            carrega_nome(c, u, n->texto ? n->texto : "_");
            emite(c, u, OP_CLOSE_SE_TEM, 0);
            int32_t fim = emite(c, u, OP_JUMP, 0);

            /* saída por erro: fecha e relança */
            if (setup >= 0) UP(c, u)->code[setup + 1] = UP(c, u)->ncode;
            carrega_nome(c, u, n->texto ? n->texto : "_");
            emite(c, u, OP_CLOSE_SE_TEM, 0);
            emite(c, u, OP_RAISE, 0);          /* a mensagem já está na pilha */

            if (fim >= 0) UP(c, u)->code[fim + 1] = UP(c, u)->ncode;
            escopo_fecha(c, u, Mu);            /* a var do using (f) não vaza */
            return;
        }

        case N_COUNT_EACH_STMT: {
            /* Desaçucara em: total (pro `self`/`_count`) + laço sobre a lista
             * de pares. Os pares vêm prontos da VM porque quem sabe iterar
             * dict e string por caractere é ela, não o compilador.
             *
             * O container é avaliado UMA vez: `count each int in f()` não
             * pode chamar `f` duas vezes. Daí o DUP2 antes do COUNT. */
            int32_t a = arg_count(c, n);
            if (a < 0) return;
            int32_t M = escopo_marca(u);         /* laço: antes de self/_count */
            count_operandos(c, u, n);
            emite(c, u, OP_DUP2, 0);
            emite(c, u, OP_COUNT, a);
            emite(c, u, OP_DUP, 0);
            /* `self` dentro do bloco é o TOTAL, e não muda durante o laço */
            guarda_nome_modo(c, u, "self", 1);
            guarda_nome_modo(c, u, "_count", 1);
            int32_t Mb = escopo_marca(u);        /* corpo: self/_count sobrevivem ao laço */
            emite(c, u, OP_COUNT_PARES, a);

            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 0, 0, NULL, 0));
            int32_t topo = UP(c, u)->ncode;
            int32_t fim = emite(c, u, OP_ITER_NEXT, 0);
            /* o par (indice, item) vira `_index` e `_match` */
            emite(c, u, OP_DUP, 0);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 0, 0, NULL, 0));
            emite(c, u, OP_INDEX_GET, 0);
            guarda_nome_modo(c, u, "_index", 1);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 1, 0, NULL, 0));
            emite(c, u, OP_INDEX_GET, 0);
            guarda_nome_modo(c, u, "_match", 1);
            abre_laco(c, topo, 2);          /* lista + indice */
            c->lacos[c->nlacos - 1].escopo_marca = M;
            c->lacos[c->nlacos - 1].escopo_marca_body = Mb;
            c->dentro_count_each++;
            bloco_stmts(c, u, n->e);
            c->dentro_count_each--;
            escopo_emite_clears(c, u, Mb);       /* reset por-iteração (_index/_match + corpo) */
            emite(c, u, OP_JUMP, topo);
            if (fim >= 0) UP(c, u)->code[fim + 1] = UP(c, u)->ncode;
            fecha_laco(c, u, topo);
            escopo_emite_clears(c, u, M);        /* saída: self/_count e cia. somem */
            escopo_trunca(u, M);
            return;
        }

        case N_ENTITY_DECL: {
            /* Métodos viram protótipos normais; o descritor guarda só a
             * ligação nome→proto. Os PAIS são resolvidos em runtime (podem
             * ser declarados depois), então entram pela pilha. */
            if (c->out->nclasses + 1 > c->cap_classes) {
                int32_t novo = c->cap_classes < 8 ? 8 : c->cap_classes * 2;
                PSClassDef *nc = realloc(c->out->classes, sizeof(PSClassDef) * (size_t)novo);
                if (!nc) { cerro(c, "sem memoria", n); return; }
                c->out->classes = nc;
                c->cap_classes = novo;
            }
            int32_t ci = c->out->nclasses++;
            PSClassDef *def = &c->out->classes[ci];
            memset(def, 0, sizeof(*def));
            def->nome = strdup(n->texto ? n->texto : "?");
            def->npais = n->lista2.n;
            def->classe_privada = n->is_private;   /* `private class` = não exportada */

            /* nomes de membros `private` — métodos (n->lista) + campos
             * (n->lista2_alias). A VM usa isto p/ barrar acesso de fora. */
            int32_t npriv_max = n->lista.n + n->lista2_alias.n;
            if (npriv_max > 0) {
                def->priv_nomes = calloc((size_t)npriv_max, sizeof(char *));
                if (!def->priv_nomes) { cerro(c, "sem memoria", n); return; }
                for (int32_t i = 0; i < n->lista.n; i++) {
                    PSNode *m = n->lista.itens[i];
                    if (m->kind == N_ACTION_DECL && m->is_private && m->texto)
                        def->priv_nomes[def->npriv++] = strdup(m->texto);
                }
                for (int32_t i = 0; i < n->lista2_alias.n; i++) {
                    PSNode *f = n->lista2_alias.itens[i];
                    if (f->kind == N_ENTITY_FIELD && f->is_private && f->texto)
                        def->priv_nomes[def->npriv++] = strdup(f->texto);
                }
            }

            int32_t nm = 0;
            for (int32_t i = 0; i < n->lista.n; i++)
                if (n->lista.itens[i]->kind == N_ACTION_DECL) nm++;
            if (nm > 0) {
                def->met_nomes = calloc((size_t)nm, sizeof(char *));
                def->met_protos = calloc((size_t)nm, sizeof(int32_t));
                if (!def->met_nomes || !def->met_protos) { cerro(c, "sem memoria", n); return; }
            }
            const char *pai_salvo = c->entity_pai;
            int dentro_salvo = c->dentro_entity;
            c->entity_pai = (n->lista2.n > 0) ? n->lista2.itens[0]->texto : NULL;
            c->dentro_entity = 1;
            for (int32_t i = 0; i < n->lista.n && !CFALHOU(c); i++) {
                PSNode *m = n->lista.itens[i];
                if (m->kind != N_ACTION_DECL) continue;   /* decorador: ignorado por ora */
                int32_t pi = compila_action(c, m);
                if (CFALHOU(c)) return;
                def = &c->out->classes[ci];               /* realloc pode ter mexido */
                def->met_nomes[def->nmetodos] = strdup(m->texto ? m->texto : "?");
                def->met_protos[def->nmetodos] = pi;
                def->nmetodos++;
            }
            c->entity_pai = pai_salvo;
            c->dentro_entity = dentro_salvo;

            /* Campos tipados sem `__init__` escrito à mão: a Entity ganha um
             * gerado, com um parâmetro por campo na ordem de declaração. */
            if (n->lista2_alias.n > 0 && !CFALHOU(c)) {
                int tem_init = 0;
                for (int32_t i = 0; i < def->nmetodos; i++)
                    if (strcmp(def->met_nomes[i], "__init__") == 0) { tem_init = 1; break; }
                if (!tem_init) {
                    int32_t pi = sintetiza_init(c, n);
                    if (CFALHOU(c)) return;
                    def = &c->out->classes[ci];
                    char **mn = realloc(def->met_nomes, sizeof(char *) * (size_t)(def->nmetodos + 1));
                    int32_t *mp = realloc(def->met_protos, sizeof(int32_t) * (size_t)(def->nmetodos + 1));
                    if (!mn || !mp) { free(mn); free(mp); cerro(c, "sem memoria", n); return; }
                    def->met_nomes = mn; def->met_protos = mp;
                    def->met_nomes[def->nmetodos] = strdup("__init__");
                    def->met_protos[def->nmetodos] = pi;
                    def->nmetodos++;
                }
            }

            /* empilha os pais na ordem declarada */
            for (int32_t i = 0; i < n->lista2.n; i++)
                carrega_nome(c, u, n->lista2.itens[i]->texto ? n->lista2.itens[i]->texto : "");
            emite(c, u, OP_MAKE_CLASS, ci);
            guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            return;
        }

        case N_INDEX_ASSIGNMENT: {
            expr(c, u, n->a);                 /* container */
            expr(c, u, n->b);                 /* índice    */
            if (n->texto && strcmp(n->texto, "=") != 0) {
                /* aumentada: lê o valor atual sem re-avaliar container/índice */
                emite(c, u, OP_DUP2, 0);
                emite(c, u, OP_INDEX_GET, 0);
                expr(c, u, n->c);
                char bin[2] = { n->texto[0], 0 };
                int32_t op = op_binario(bin);
                if (op < 0) { cerro(c, "operador de atribuicao invalido", n); return; }
                emite(c, u, op, 0);
            } else {
                expr(c, u, n->c);
            }
            emite(c, u, OP_INDEX_SET, 0);
            return;
        }

        case N_IMPORT_STMT: {
            /* Nome do módulo codificado como o `module_name` do interpretador:
             * `n->i2` pontos de nível relativo, seguidos do caminho pontuado.
             * `from .a.b import x` -> ".a.b"; `from pkg.mod import x` -> "pkg.mod";
             * `import json` -> "json". O runtime (carrega_modulo_ps) resolve. */
            if (!n->texto) { cerro(c, "import mal formado", n); return; }
            if (n->i2 > 0 && n->lista.n == 0) {
                cerro(c, "import relativo precisa de um modulo depois dos pontos "
                         "(ex: from .modulo import x)", n);
                return;
            }
            char encoded[512]; int el = 0;
            for (int32_t i = 0; i < n->i2 && el < 500; i++) encoded[el++] = '.';
            for (int32_t i = 0; i < n->lista.n && el < 500; i++) {
                if (i) encoded[el++] = '.';
                const char *pt = n->lista.itens[i]->texto ? n->lista.itens[i]->texto : "";
                int pl = (int)strlen(pt);
                if (el + pl >= 500) pl = 500 - el;
                memcpy(encoded + el, pt, (size_t)pl); el += pl;
            }
            encoded[el] = '\0';
            const char *mod = encoded;
            /* ligar o MÓDULO inteiro a um nome (import x / import x as y) só
             * vale pra nome simples — pontuado/relativo exige `from ... import`,
             * como no interpretador (`import a.b` não liga `a`). */
            int simples = (n->i2 == 0 && n->lista.n == 1);

            /* `PUSH mod` é `import mod`; `PUSH mod GET a, b` é
             * `from mod import a, b`. Com GET, o módulo NÃO fica visível —
             * nem sob o `as`, que nesse caso não liga nada. É o que o
             * interpretador faz. */
            if (strcmp(n->texto, "push") == 0) {
                if (n->lista2.n == 0) {
                    if (!simples) { cerro(c, "import de modulo pontuado/relativo precisa de 'from ... import ...'", n); return; }
                    emite(c, u, OP_IMPORT_MOD, idx_const(c, u, K_STR, 0, 0, mod, (int32_t)strlen(mod)));
                    guarda_nome(c, u, n->texto2 ? n->texto2 : mod);
                    return;
                }
                for (int32_t i = 0; i < n->lista2.n; i++) {
                    const char *membro = n->lista2.itens[i]->texto;
                    const char *apelido = (i < n->lista2_alias.n && n->lista2_alias.itens[i])
                                        ? n->lista2_alias.itens[i]->texto : membro;
                    emite(c, u, OP_IMPORT_MOD, idx_const(c, u, K_STR, 0, 0, mod, (int32_t)strlen(mod)));
                    emite(c, u, OP_GET_MEMBER, idx_const(c, u, K_STR, 0, 0, membro, (int32_t)strlen(membro)));
                    guarda_nome(c, u, apelido);
                }
                return;
            }

            if (strcmp(n->texto, "import") == 0) {
                if (!simples) { cerro(c, "import de modulo pontuado/relativo precisa de 'from ... import ...'", n); return; }
                emite(c, u, OP_IMPORT_MOD, idx_const(c, u, K_STR, 0, 0, mod, (int32_t)strlen(mod)));
                guarda_nome(c, u, n->texto2 ? n->texto2 : mod);
                return;
            }
            /* `from mod import a, b as c` — um GET_MEMBER por nome pedido */
            for (int32_t i = 0; i < n->lista2.n; i++) {
                const char *membro = n->lista2.itens[i]->texto;
                const char *apelido = (i < n->lista2_alias.n && n->lista2_alias.itens[i])
                                    ? n->lista2_alias.itens[i]->texto : membro;
                emite(c, u, OP_IMPORT_MOD, idx_const(c, u, K_STR, 0, 0, mod, (int32_t)strlen(mod)));
                emite(c, u, OP_GET_MEMBER, idx_const(c, u, K_STR, 0, 0, membro, (int32_t)strlen(membro)));
                guarda_nome(c, u, apelido);
            }
            return;
        }

        case N_MEMBER_ASSIGNMENT: {
            int32_t mi = idx_const(c, u, K_STR, 0, 0, n->texto ? n->texto : "",
                                   n->texto ? (int32_t)strlen(n->texto) : 0);
            expr(c, u, n->a);                 /* objeto */
            if (n->texto2 && strcmp(n->texto2, "=") != 0) {
                /* aumentada `obj.x += v`: lê o atual sem re-avaliar o objeto */
                emite(c, u, OP_DUP, 0);
                emite(c, u, OP_GET_MEMBER, mi);
                expr(c, u, n->b);
                char bin[2] = { n->texto2[0], 0 };
                int32_t op = op_binario(bin);
                if (op < 0) { cerro(c, "operador de atribuicao invalido", n); return; }
                emite(c, u, op, 0);
            } else {
                expr(c, u, n->b);
            }
            emite(c, u, OP_SET_MEMBER, mi);
            return;
        }

        case N_MATCH_STMT: {
            expr(c, u, n->a);                  /* sujeito fica na pilha */
            int32_t fins[64];
            int nfins = 0;
            for (int32_t i = 0; i < n->lista.n && !CFALHOU(c); i++) {
                PSNode *caso = n->lista.itens[i];
                emite(c, u, OP_DUP, 0);        /* cada teste consome uma cópia */
                int32_t Mc = escopo_marca(u);  /* captura do padrão + corpo: escopo do case */
                padrao_testa(c, u, caso->a);
                int32_t falhou = emite(c, u, OP_JUMP_IF_FALSE, 0);
                /* guarda do case: `case v if v > 5` */
                int32_t falhou_guarda = -1;
                if (caso->a && caso->a->a) {
                    expr(c, u, caso->a->a);
                    falhou_guarda = emite(c, u, OP_JUMP_IF_FALSE, 0);
                }
                bloco_stmts(c, u, caso->b);
                escopo_fecha(c, u, Mc);        /* captura/vars do case não vazam */
                if (nfins < 64) fins[nfins++] = emite(c, u, OP_JUMP, 0);
                if (falhou >= 0) UP(c, u)->code[falhou + 1] = UP(c, u)->ncode;
                if (falhou_guarda >= 0) UP(c, u)->code[falhou_guarda + 1] = UP(c, u)->ncode;
            }
            int32_t fim = UP(c, u)->ncode;
            for (int k = 0; k < nfins; k++) UP(c, u)->code[fins[k] + 1] = fim;
            emite(c, u, OP_POP_TOP, 0);        /* descarta o sujeito */
            return;
        }

        case N_RAISE_STMT:
            if (n->texto) {                       /* raise Tipo[("msg")] — tipo livre */
                if (n->a) expr(c, u, n->a);        /* mensagem na pilha */
                else emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_STR, 0, 0, "", 0));
                emite(c, u, OP_LOAD_CONST,         /* nome do tipo por cima */
                      idx_const(c, u, K_STR, 0, 0, n->texto, (int32_t)strlen(n->texto)));
                emite(c, u, OP_RERAISE, 0);         /* consome (tipo, msg) */
            } else {
                expr(c, u, n->a);
                emite(c, u, OP_RAISE, 0);
            }
            return;

        case N_TRY_CATCH_STMT: {
            /* Layout:
             *   SETUP_TRY -> primeiro_catch
             *   <corpo do try>
             *   POP_TRY
             *   JUMP -> finally
             * primeiro_catch:            (a mensagem chega na pilha)
             *   <cadeia de catches>
             * finally:
             *   <bloco finally, se houver>
             *
             * O `finally` é emitido INLINE nos dois caminhos (saída normal e
             * saída por erro) em vez de virar sub-rotina: sem opcode de
             * chamada interna, duplicar o bloco é o jeito direto de garantir
             * que ele roda sempre. */
            int32_t setup = emite(c, u, OP_SETUP_TRY, 0);
            c->dentro_try++;
            int32_t Mt = escopo_marca(u);
            bloco_stmts(c, u, n->a);
            escopo_fecha(c, u, Mt);            /* vars do try não vazam (saída normal) */
            c->dentro_try--;
            emite(c, u, OP_POP_TRY, 0);
            int32_t pula_catches = emite(c, u, OP_JUMP, 0);

            if (setup >= 0) UP(c, u)->code[setup + 1] = UP(c, u)->ncode;

            /* Cada catch: se tem tipo, compara; senão captura tudo.
             * A mensagem já está no topo quando chegamos aqui. */
            int32_t fins[32];
            int nfins = 0;
            int32_t prox_falha = -1;
            for (int32_t i = 0; i < n->lista.n && !CFALHOU(c); i++) {
                PSNode *cl = n->lista.itens[i];
                if (prox_falha >= 0) {
                    UP(c, u)->code[prox_falha + 1] = UP(c, u)->ncode;
                    prox_falha = -1;
                }
                if (cl->texto2) {                    /* catch (Tipo nome) */
                    /* A mensagem fica na pilha o tempo todo: PUSH_ERR_TYPE
                     * empilha por cima e o EQ consome só os dois nomes de
                     * tipo. Quem "devolvia a mensagem" com um LOAD do nome
                     * do catch lia variável que ainda não existia. */
                    emite(c, u, OP_PUSH_ERR_TYPE, 0);
                    emite(c, u, OP_LOAD_CONST,
                          idx_const(c, u, K_STR, 0, 0, cl->texto2, (int32_t)strlen(cl->texto2)));
                    emite(c, u, OP_EQ, 0);
                    prox_falha = emite(c, u, OP_JUMP_IF_FALSE, 0);
                }
                /* liga a mensagem ao nome do catch e roda o bloco */
                int32_t Mcat = escopo_marca(u);
                guarda_nome_modo(c, u, cl->texto ? cl->texto : "e", 1);
                bloco_stmts(c, u, cl->b);
                escopo_fecha(c, u, Mcat);      /* var do catch (e) + corpo não vazam */
                if (nfins < 32) fins[nfins++] = emite(c, u, OP_JUMP, 0);
            }

            /* Nenhum catch casou: o erro CONTINUA. Engolir aqui fazia
             * `catch (KeyError e)` virar um catch-tudo silencioso — o `try`
             * de fora nunca via a divisão por zero. */
            if (prox_falha >= 0) {
                UP(c, u)->code[prox_falha + 1] = UP(c, u)->ncode;
                /* tipo junto da mensagem: o `finally` roda antes do RERAISE e
                 * pode ter trocado o erro corrente da VM */
                emite(c, u, OP_PUSH_ERR_TYPE, 0);
                if (n->c) { int32_t Mf = escopo_marca(u); bloco_stmts(c, u, n->c); escopo_fecha(c, u, Mf); }
                emite(c, u, OP_RERAISE, 0);
            }

            int32_t fim_catches = UP(c, u)->ncode;
            if (pula_catches >= 0) UP(c, u)->code[pula_catches + 1] = fim_catches;
            for (int k = 0; k < nfins; k++) UP(c, u)->code[fins[k] + 1] = fim_catches;

            if (n->c) { int32_t Mf = escopo_marca(u); bloco_stmts(c, u, n->c); escopo_fecha(c, u, Mf); }  /* finally */
            return;
        }

        case N_GLOBAL_STMT: {
            /* Só marca os nomes — não emite instrução. O efeito é na
             * resolução: daqui pra frente `x` vira LOAD/STORE_GLOBAL. */
            for (int32_t i = 0; i < n->lista.n; i++) {
                const char *nome = n->lista.itens[i]->texto;
                if (!nome || eh_global_declarada(u, nome)) continue;
                if (u->nglobais_decl + 1 > u->cap_globais_decl) {
                    int32_t novo = u->cap_globais_decl < 8 ? 8 : u->cap_globais_decl * 2;
                    char **ng = realloc(u->globais_decl, sizeof(char *) * (size_t)novo);
                    if (!ng) { cerro(c, "sem memoria", n); return; }
                    u->globais_decl = ng;
                    u->cap_globais_decl = novo;
                }
                size_t ln = strlen(nome);
                char *copia = malloc(ln + 1);
                if (!copia) { cerro(c, "sem memoria", n); return; }
                memcpy(copia, nome, ln + 1);
                u->globais_decl[u->nglobais_decl++] = copia;
                idx_global(c, nome);      /* garante o slot na tabela */
            }
            return;
        }

        case N_BREAK_STMT: {
            if (c->nlacos == 0) { cerro(c, "'break' fora de laco", n); return; }
            Laco *l = &c->lacos[c->nlacos - 1];
            if (l->nsaidas >= MAX_SAIDAS) { cerro(c, "'break' demais no mesmo laco", n); return; }
            /* saindo do laço: apaga TUDO nascido nele até aqui (var do laço +
             * corpo + blocos aninhados abertos), pra nada vazar pra fora */
            escopo_emite_clears(c, u, l->escopo_marca);
            /* limpa o estado do iterador antes de sair do laço */
            for (int k = 0; k < l->slots_pilha; k++) emite(c, u, OP_POP_TOP, 0);
            l->saidas[l->nsaidas++] = emite(c, u, OP_JUMP, 0);
            return;
        }

        case N_CONTINUE_STMT: {
            if (c->nlacos == 0) { cerro(c, "'continue' fora de laco", n); return; }
            Laco *l = &c->lacos[c->nlacos - 1];
            if (l->ncontinues >= MAX_SAIDAS) { cerro(c, "'continue' demais no mesmo laco", n); return; }
            /* próxima iteração começa limpa: apaga só o escopo do CORPO (o que é
             * do laço — var/ self/_count — é re-atribuído no topo ou persiste) */
            escopo_emite_clears(c, u, l->escopo_marca_body);
            l->continues[l->ncontinues++] = emite(c, u, OP_JUMP, 0);
            return;
        }

        default:
            cerro(c, "statement ainda nao compila na VM", n);
            return;
    }
}

/* ── action ─────────────────────────────────────────────────────────────── */
static int32_t novo_proto(C *c, const char *nome)
{
    if (c->out->nprotos + 1 > c->cap_protos) {
        int32_t novo = c->cap_protos < 8 ? 8 : c->cap_protos * 2;
        PSProto *np = realloc(c->out->protos, sizeof(PSProto) * (size_t)novo);
        if (!np) { cerro(c, "sem memoria", NULL); return -1; }
        c->out->protos = np;
        c->cap_protos = novo;
    }
    PSProto *p = &c->out->protos[c->out->nprotos];
    memset(p, 0, sizeof(*p));
    size_t n = strlen(nome);
    p->nome = malloc(n + 1);
    if (!p->nome) { cerro(c, "sem memoria", NULL); return -1; }
    memcpy(p->nome, nome, n + 1);
    return c->out->nprotos++;
}

/* Emite UNPACK + um STORE por alvo. Presume a sequência no topo da pilha. */
static void compila_unpack_alvo(C *c, Unidade *u, PSNode *alvo)
{
    int32_t n_alvos = alvo->lista.n;
    int32_t star = alvo->i2;                     /* -1 = sem `*` */
    if (n_alvos > 250) { cerro(c, "alvos demais no desempacotamento", alvo); return; }
    emite(c, u, OP_UNPACK, n_alvos | ((star + 1) << 8));
    for (int32_t i = 0; i < n_alvos && !CFALHOU(c); i++) {
        PSNode *e = alvo->lista.itens[i];
        if (e->kind == N_UNPACK_TARGET) {
            compila_unpack_alvo(c, u, e);        /* `a, (b, c) = ...` */
        } else if (e->kind == N_NAME) {
            guarda_nome(c, u, e->texto ? e->texto : "");
        } else {
            cerro(c, "alvo de desempacotamento precisa ser nome", e);
            return;
        }
    }
}

static int32_t sintetiza_init(C *c, PSNode *entidade)
{
    PSNodeVec *campos = &entidade->lista2_alias;
    int32_t idx = novo_proto(c, "__init__");
    if (idx < 0) return -1;

    Unidade u;
    memset(&u, 0, sizeof(u));
    u.idx = idx;
    u.eh_modulo = 0;

    /* slot 0 é o `self`; os campos vêm depois, na ordem de declaração */
    { int32_t si = idx_local(c, &u, "self"); if (si < 256) u.certo[si] = 1; }
    int32_t ndef = 0;
    for (int32_t i = 0; i < campos->n; i++) {
        const char *nome = campos->itens[i]->texto ? campos->itens[i]->texto : "";
        int32_t pi = idx_local(c, &u, nome);
        if (pi < 256) u.certo[pi] = 1;
        if (campos->itens[i]->a) ndef++;
    }
    c->out->protos[idx].nparams = campos->n + 1;
    c->out->protos[idx].ndefaults = ndef;
    {
        char **nomes = calloc((size_t)campos->n + 1, sizeof(char *));
        if (!nomes) { cerro(c, "sem memoria", entidade); return -1; }
        nomes[0] = strdup("self");
        for (int32_t i = 0; i < campos->n; i++)
            nomes[i + 1] = strdup(campos->itens[i]->texto ? campos->itens[i]->texto : "");
        c->out->protos[idx].param_nomes = nomes;
    }

    /* prólogo dos campos com valor padrão */
    for (int32_t i = 0; i < campos->n && !CFALHOU(c); i++) {
        if (!campos->itens[i]->a) continue;
        emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_INT, i + 1, 0, NULL, 0));
        int32_t pula = emite(c, &u, OP_JUMP_IF_SET, 0);
        expr(c, &u, campos->itens[i]->a);
        emite(c, &u, OP_STORE_LOCAL, i + 1);
        if (pula >= 0) UP(c, (&u))->code[pula + 1] = UP(c, (&u))->ncode;
    }

    /* corpo: self.<campo> = <parametro>, um por campo */
    for (int32_t i = 0; i < campos->n && !CFALHOU(c); i++) {
        const char *nome = campos->itens[i]->texto ? campos->itens[i]->texto : "";
        emite(c, &u, OP_LOAD_LOCAL, 0);
        emite(c, &u, OP_LOAD_LOCAL, i + 1);
        emite(c, &u, OP_SET_MEMBER, idx_const(c, &u, K_STR, 0, 0, nome, (int32_t)strlen(nome)));
    }
    emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_NULL, 0, 0, NULL, 0));
    emite(c, &u, OP_RETURN, 0);

    for (int32_t i = 0; i < u.nlocais; i++) free(u.locais[i]);
    free(u.locais);
    for (int32_t i = 0; i < u.n_mod_criados; i++) free(u.mod_criados[i]);
    free(u.mod_criados);
    for (int32_t i = 0; i < u.nglobais_decl; i++) free(u.globais_decl[i]);
    free(u.globais_decl);
    return idx;
}

static int32_t compila_action(C *c, PSNode *n)
{
    int32_t idx = novo_proto(c, n->texto ? n->texto : "<action>");
    if (idx < 0) return -1;

    Unidade u;
    memset(&u, 0, sizeof(u));
    u.idx = idx;
    u.eh_modulo = 0;
    if (n->texto2 && !strcmp(n->texto2, "int"))       u.tipo_ret = 1;
    else if (n->texto2 && !strcmp(n->texto2, "bool")) u.tipo_ret = 2;

    int32_t ndef = 0;
    for (int32_t i = 0; i < n->lista.n; i++) {
        PSNode *par = n->lista.itens[i];
        {
            int32_t pi = idx_local(c, &u, par->texto ? par->texto : "");
            if (pi < 256) u.certo[pi] = 1;
        }
        if (par->a) ndef++;
        else if (ndef > 0) {
            cerro(c, "parametro sem valor padrao depois de um com padrao", n);
            break;
        }
    }
    c->out->protos[idx].nparams = n->lista.n;
    c->out->protos[idx].ndefaults = ndef;
    if (n->lista.n > 0) {
        char **nomes = calloc((size_t)n->lista.n, sizeof(char *));
        if (!nomes) { cerro(c, "sem memoria", n); }
        else {
            for (int32_t i = 0; i < n->lista.n; i++) {
                const char *pn = n->lista.itens[i]->texto ? n->lista.itens[i]->texto : "";
                size_t ln = strlen(pn);
                nomes[i] = malloc(ln + 1);
                if (nomes[i]) memcpy(nomes[i], pn, ln + 1);
            }
            c->out->protos[idx].param_nomes = nomes;
        }
    }

    /* Prólogo: para cada parâmetro com default, avalia o default SÓ se o
     * argumento não veio na chamada. */
    for (int32_t i = 0; i < n->lista.n && !CFALHOU(c); i++) {
        PSNode *par = n->lista.itens[i];
        if (!par->a) continue;
        emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_INT, i, 0, NULL, 0));
        int32_t pula = emite(c, &u, OP_JUMP_IF_SET, 0);
        expr(c, &u, par->a);
        emite(c, &u, OP_STORE_LOCAL, i);
        if (pula >= 0) UP(c, (&u))->code[pula + 1] = UP(c, (&u))->ncode;
    }

    /* Depois do prólogo: um default que avalie pra Null também é violação. */
    if (c->pendente_nonnull) {
        c->pendente_nonnull = 0;
        emite(c, &u, OP_CHECK_NONNULL, n->lista.n);
    }

    /* `int action` e `bool action` não deixam erro escapar: devolvem 500 e
     * False. Sai mais barato emitir o `try` implícito aqui do que ensinar o
     * desenrolamento de erro da VM a olhar o tipo do frame. */
    int32_t rede = -1;
    if (u.tipo_ret) rede = emite(c, &u, OP_SETUP_TRY, 0);

    bloco_stmts(c, &u, n->b);
    if (u.tipo_ret) emite(c, &u, OP_POP_TRY, 0);
    /* action sem return explícito devolve Null */
    emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_NULL, 0, 0, NULL, 0));
    if (u.tipo_ret) emite(c, &u, OP_COERCE_RET, u.tipo_ret);
    emite(c, &u, OP_RETURN, 0);

    if (rede >= 0) {
        UP(c, (&u))->code[rede + 1] = UP(c, (&u))->ncode;
        emite(c, &u, OP_POP_TOP, 0);          /* descarta a mensagem do erro */
        if (u.tipo_ret == 1)
            emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_INT, 500, 0, NULL, 0));
        else
            emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_BOOL, 0, 0, NULL, 0));
        emite(c, &u, OP_RETURN, 0);
    }

    for (int32_t i = 0; i < u.nlocais; i++) free(u.locais[i]);
    free(u.locais);
    for (int32_t i = 0; i < u.n_mod_criados; i++) free(u.mod_criados[i]);
    free(u.mod_criados);
    for (int32_t i = 0; i < u.nglobais_decl; i++) free(u.globais_decl[i]);
    free(u.globais_decl);

    return idx;
}

/* ── entrada ────────────────────────────────────────────────────────────── */
PSPrograma *ps_compila(PSNode *programa)
{
    PSPrograma *out = calloc(1, sizeof(PSPrograma));
    if (!out) return NULL;
    out->ok = 1;

    C c;
    memset(&c, 0, sizeof(c));
    c.out = out;

    int32_t idx = novo_proto(&c, "<module>");
    if (idx < 0) return out;

    Unidade u;
    memset(&u, 0, sizeof(u));
    u.idx = idx;
    u.eh_modulo = 1;

    if (programa) {
        for (int32_t i = 0; i < programa->lista.n && out->ok; i++) {
            stmt(&c, &u, programa->lista.itens[i]);
        }
    }
    emite(&c, &u, OP_HALT, 0);

    for (int32_t i = 0; i < u.nlocais; i++) free(u.locais[i]);
    free(u.locais);
    for (int32_t i = 0; i < u.n_mod_criados; i++) free(u.mod_criados[i]);
    free(u.mod_criados);
    for (int32_t i = 0; i < u.nglobais_decl; i++) free(u.globais_decl[i]);
    free(u.globais_decl);
    return out;
}

void ps_compila_free(PSPrograma *p)
{
    if (!p) return;
    for (int32_t i = 0; i < p->nprotos; i++) {
        free(p->protos[i].nome);
        free(p->protos[i].code);
        free(p->protos[i].linhas);
        free(p->protos[i].colunas);
        if (p->protos[i].param_nomes) {
            for (int32_t k = 0; k < p->protos[i].nparams; k++)
                free(p->protos[i].param_nomes[k]);
            free(p->protos[i].param_nomes);
        }
        for (int32_t k = 0; k < p->protos[i].nconsts; k++)
            free(p->protos[i].consts[k].s);
        free(p->protos[i].consts);
    }
    free(p->protos);
    for (int32_t i = 0; i < p->nglobais; i++) free(p->globais[i]);
    free(p->globais);
    for (int32_t i = 0; i < p->nclasses; i++) {
        free(p->classes[i].nome);
        for (int32_t k = 0; k < p->classes[i].nmetodos; k++)
            free(p->classes[i].met_nomes[k]);
        free(p->classes[i].met_nomes);
        free(p->classes[i].met_protos);
        for (int32_t k = 0; k < p->classes[i].npriv; k++)
            free(p->classes[i].priv_nomes[k]);
        free(p->classes[i].priv_nomes);
    }
    free(p->classes);
    for (int32_t i = 0; i < p->nmodels; i++) {
        free(p->models[i].nome);
        for (int32_t k = 0; k < p->models[i].ncampos; k++)
            free(p->models[i].campos[k].nome);
        free(p->models[i].campos);
    }
    free(p->models);
    for (int32_t i = 0; i < p->nenums; i++) {
        free(p->enums[i].nome);
        for (int32_t k = 0; k < p->enums[i].nmembros; k++)
            free(p->enums[i].membros[k].nome);
        free(p->enums[i].membros);
    }
    free(p->enums);
    free(p);
}
