/*
 * Compilador AST → bytecode, em C puro.
 *
 * Duas coisas que o compilador resolve em tempo de
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
#include "ps_ext.h"
#include "ps_tipos.h"
#include "ps_regex.h"        /* `regex=` do campo de model: compilado aqui só pra validar */
#include "ps_retornos.h"
#include "ps_caminho.h"      /* nome do módulo a partir do caminho: `/` e, no Windows, `\` */

#include <stdarg.h>

#include "ps_lexer.h"
#include "ps_parser.h"
#include "ps_pilha.h"
#include "ps_assert.h"       /* invariante da pilha de valores medida por proto */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── opcodes ─────────────────────────────────────────────────────────────
 * A lista vive em `ps_opcodes.def` e é a MESMA que a VM inclui. Antes eram
 * dois enums escritos à mão, e nada no build conferia que batiam. */
enum {
#define PS_OP(nome, num, texto, tira, deixa) OP_##nome = (num),
#include "ps_opcodes.def"
#undef PS_OP
    OP__ULTIMO
};

/* Nome pro desmontador. O texto vem da terceira coluna do `.def` porque nem
 * todo opcode se imprime com o próprio sufixo: OP_IS sai como "IS_OP". */
const char *ps_op_nome(int32_t op)
{
    switch (op) {
#define PS_OP(nome, num, texto, tira, deixa) case OP_##nome: return texto;
#include "ps_opcodes.def"
#undef PS_OP
        default: break;
    }
    return "?";
}

/* ── desmontador: `jinga --bytecode arquivo.pr` ─────────────────────────
 *
 * O que cada laço VIRA é o que decide a velocidade, e sem ver o bytecode toda
 * otimização é palpite. Uma linha por instrução: `ip  NOME  arg  ; o que o
 * arg significa` — a constante, o nome do global, o alvo do salto, o proto.
 * É texto, não JSON: é pra gente ler e pra teste afirmar a sequência. */
static void desmonta_const(const PSConst *k, FILE *f)
{
    switch (k->kind) {
        case K_NULL:   fputs("Null", f); break;
        case K_BOOL:   fputs(k->i ? "True" : "False", f); break;
        case K_INT:    fprintf(f, "%lld", (long long)k->i); break;
        case K_FLO:    fprintf(f, "%g", k->d); break;
        case K_BIGINT: fprintf(f, "%s (bigint)", k->s ? k->s : ""); break;
        case K_BYTES:  fprintf(f, "bytes[%d]", (int)k->slen); break;
        case K_STR: {
            fputc('"', f);
            for (int32_t i = 0; i < k->slen && i < 40; i++) {
                unsigned char c = (unsigned char)k->s[i];
                if (c == '\n') fputs("\\n", f);
                else if (c == '"') fputs("\\\"", f);
                else if (c < 32) fprintf(f, "\\x%02x", c);
                else fputc(c, f);
            }
            if (k->slen > 40) fputs("...", f);
            fputc('"', f);
            break;
        }
    }
}

void ps_desmonta(const PSPrograma *prog, FILE *f)
{
    if (!prog) return;
    for (int32_t pi = 0; pi < prog->nprotos; pi++) {
        const PSProto *p = &prog->protos[pi];
        fprintf(f, "== proto %d %s nlocals=%d nparams=%d pilha=%d%s%s\n", pi,
                (p->nome && p->nome[0]) ? p->nome : "<modulo>", p->nlocals, p->nparams,
                p->pilha_max, p->eh_gerador ? " gerador" : "", p->eh_async ? " async" : "");
        for (int32_t ip = 0; ip + 1 < p->ncode; ip += 2) {
            int32_t op = p->code[ip], arg = p->code[ip + 1];
            fprintf(f, "%6d  %-16s %-8d", ip, ps_op_nome(op), arg);
            switch (op) {
                case OP_GET_MEMBER: case OP_LOAD_BASE_MEMBRO: {
                    /* o bit de "vai ser chamado" (PS_ARG_CHAMADO) */
                    int32_t k = arg & ~PS_ARG_CHAMADO;
                    if (k >= 0 && k < p->nconsts) { fputs("; ", f); desmonta_const(&p->consts[k], f); }
                    if (arg & PS_ARG_CHAMADO) fputs(" (chamado)", f);
                    break;
                }
                case OP_LOAD_CONST: case OP_SET_MEMBER:
                case OP_IMPORT_MOD: case OP_IMPORT_FROM: case OP_LOAD_METODO:
                case OP_SET_METODO: case OP_IMPORT_FROM_ESTRELA: case OP_CONFERE_TIPO:
                    if (arg >= 0 && arg < p->nconsts) { fputs("; ", f); desmonta_const(&p->consts[arg], f); }
                    break;
                case OP_COERCE_DECL:
                    if ((arg >> 4) >= 0 && (arg >> 4) < p->nconsts) {
                        fputs("; ", f); desmonta_const(&p->consts[arg >> 4], f);
                        fprintf(f, " tipo=%d", arg & 15);
                    }
                    break;
                case OP_LOAD_GLOBAL: case OP_STORE_GLOBAL: case OP_LOAD_NAME:
                case OP_STORE_NAME: case OP_CLEAR_GLOBAL: case OP_CELL_GET_NAME:
                case OP_CELL_SET_NAME:
                    if (arg >= 0 && arg < prog->nglobais && prog->globais[arg])
                        fprintf(f, "; %s", prog->globais[arg]);
                    break;
                case OP_JUMP: case OP_JUMP_IF_FALSE: case OP_JUMP_IF_TRUE:
                case OP_JUMP_IF_SET: case OP_JUMP_SE_UNSET: case OP_SKIP_IF_IMPORT:
                case OP_SETUP_TRY: case OP_ITER_NEXT: case OP_ITER_RANGE:
                case OP_JF_LT: case OP_JF_GT: case OP_JF_LE: case OP_JF_GE:
                case OP_JF_EQ: case OP_JF_NE:
                    fprintf(f, "; -> %d", arg);
                    break;
                case OP_INC_GLOBAL_INT: case OP_DEC_GLOBAL_INT:
                    if (arg >= 0 && arg < prog->nglobais && prog->globais[arg])
                        fprintf(f, "; %s", prog->globais[arg]);
                    break;
                case OP_MAKE_FUNCTION: case OP_MAKE_CLOSURE:
                    if (arg >= 0 && arg < prog->nprotos && prog->protos[arg].nome)
                        fprintf(f, "; %s", prog->protos[arg].nome);
                    break;
                default: break;
            }
            fputc('\n', f);
        }
    }
}

/* ── índice de nomes ─────────────────────────────────────────────────────── */
/* Nome -> posição numa lista, por hash. As listas de nomes do compilador (os
 * globais, os nomes colhidos do topo, os criados no módulo, os ligados no
 * módulo) eram procuradas com `strcmp` do começo ao fim — uma vez por
 * declaração, e cada declaração crescia a lista. Arquivo com N funções pagava
 * N²: 10 mil funções de uma linha levavam 0,8 s pra conferir e 40 mil, 32 s,
 * ao rodar também. A lista continua sendo a dona das strings; o índice só
 * aponta pra elas, e quem tira um nome da lista tira do índice ANTES de
 * liberar a string. */
typedef struct {
    const char **k;       /* NULL = vazio; IX_LAPIDE = removido */
    int32_t     *v;
    int32_t      cap;     /* potência de 2; 0 = índice ainda vazio */
    int32_t      usados;  /* ocupados + lápides: é o que decide crescer */
} IdxNomes;
#define IX_LAPIDE ((const char *)1)

static uint32_t ix_hash(const char *s)
{
    uint32_t h = 2166136261u;
    for (; *s; s++) { h ^= (unsigned char)*s; h *= 16777619u; }
    return h;
}

/* A posição guardada pro nome, ou -1. */
static int32_t ix_busca(const IdxNomes *ix, const char *nome)
{
    if (!ix->cap || !nome) return -1;
    uint32_t m = (uint32_t)ix->cap - 1, i = ix_hash(nome) & m;
    for (;;) {
        const char *k = ix->k[i];
        if (!k) return -1;
        if (k != IX_LAPIDE && strcmp(k, nome) == 0) return ix->v[i];
        i = (i + 1) & m;
    }
}

static int ix_poe(IdxNomes *ix, const char *nome, int32_t val);

static int ix_cresce(IdxNomes *ix)
{
    IdxNomes novo = {0};
    novo.cap = ix->cap ? ix->cap * 2 : 16;
    novo.k = calloc((size_t)novo.cap, sizeof(*novo.k));
    novo.v = calloc((size_t)novo.cap, sizeof(*novo.v));
    if (!novo.k || !novo.v) { free(novo.k); free(novo.v); return -1; }
    for (int32_t i = 0; i < ix->cap; i++)
        if (ix->k[i] && ix->k[i] != IX_LAPIDE) ix_poe(&novo, ix->k[i], ix->v[i]);
    free(ix->k); free(ix->v);
    *ix = novo;
    return 0;
}

/* Guarda (ou troca) a posição do nome. -1 = sem memória. */
static int ix_poe(IdxNomes *ix, const char *nome, int32_t val)
{
    if ((ix->usados + 1) * 10 > ix->cap * 7 && ix_cresce(ix) != 0) return -1;
    uint32_t m = (uint32_t)ix->cap - 1, i = ix_hash(nome) & m;
    int32_t lapide = -1;
    for (;;) {
        const char *k = ix->k[i];
        if (!k) break;
        if (k == IX_LAPIDE) { if (lapide < 0) lapide = (int32_t)i; }
        else if (strcmp(k, nome) == 0) { ix->v[i] = val; return 0; }
        i = (i + 1) & m;
    }
    if (lapide >= 0) i = (uint32_t)lapide;
    else ix->usados++;
    ix->k[i] = nome;
    ix->v[i] = val;
    return 0;
}

static void ix_tira(IdxNomes *ix, const char *nome)
{
    if (!ix->cap || !nome) return;
    uint32_t m = (uint32_t)ix->cap - 1, i = ix_hash(nome) & m;
    for (;;) {
        const char *k = ix->k[i];
        if (!k) return;
        if (k != IX_LAPIDE && strcmp(k, nome) == 0) { ix->k[i] = IX_LAPIDE; return; }
        i = (i + 1) & m;
    }
}

static void ix_solta(IdxNomes *ix)
{
    free(ix->k); free(ix->v);
    memset(ix, 0, sizeof(*ix));
}

/* ── estado ─────────────────────────────────────────────────────────────── */
/* Guarda o ÍNDICE do protótipo, não o ponteiro: uma funct aninhada chama
 * `novo_proto`, que faz realloc do array — qualquer PSProto* guardado aqui
 * viraria ponteiro pendurado no meio da compilação. */
typedef struct Unidade Unidade;
/* Um upvalue enquanto compila: além do descritor que vai pro proto, guarda o
 * NOME, que é a chave da busca no aninhamento. */
typedef struct { char *nome; int32_t em_local; int32_t idx; } UpvalC;

/* O que a tipagem ESTÁTICA sabe de um nome: o tipo e de onde ele veio. Um por
 * slot de função, um por nome de módulo vivo e um por global colhido antes de
 * compilar (ver "tipagem estatica", mais abaixo).
 *
 * `tipo` NULL com estado 1 = o primeiro valor veio de onde o tipo só se sabe
 * rodando (`json.parse`, `d["k"]`): a variável aceita qualquer valor, como uma
 * de tipo Object. Os textos de tipo são estáticos, da árvore ou do `pool` do
 * compilador — ninguém aqui é dono deles. */
typedef struct {
    const char *tipo;
    /* 0 = sem tipo ainda (nunca recebeu nada, ou só Null); 1 = fixado pela
     * primeira atribuição; 2 = declarado (`str s`, parâmetro tipado) */
    unsigned char estado;
    /* funct/lambda/Entity que o nome liga — é a assinatura que confere a
     * chamada. `decorado`: um decorador geral pode trocar a funct, e aí a
     * assinatura escrita não diz mais nada. */
    PSNode *decl;
    unsigned char decorado;
    /* `import os` liga `os` ao módulo nativo; `from os import getenv` liga
     * `getenv` ao membro dele */
    const char *mod_nativo;
    const char *membro_nativo;
    /* `import util` de um `.pr`: a AST e os exportados dele (do resolvedor), e
     * o nome como foi escrito, pra frase "module 'util' has no attribute".
     * `from util import f` liga `f` direto ao `decl` de dentro da AST. */
    const PSModuloAst *mod_pr;
    const char *mod_pr_nome;
    /* a declaração (class/model/enum/funct) que ligou o nome neste escopo: o
     * que ela é e a linha; NULL = o nome não nasceu de declaração */
    const char *decl_rotulo;
    int32_t decl_linha;
} SimInfo;

struct Unidade {
    /* Função que ENVOLVE esta. NULL no módulo e nas functs de topo — é o que
     * limita a captura: nome não achado aqui nem no pai vira global. */
    Unidade *pai;
    /* Na funct de topo (pai NULL): o módulo em que ela foi escrita. A
     * variável de BLOCO do topo que ela cita mora numa célula do módulo
     * (`celula_do_topo`), e é por aqui que a captura a acha. */
    Unidade *modulo;
    /* Os quatro vetores por SLOT abaixo (celula, celula_virgem, certo,
     * tipo_decl) crescem junto com `locais` (ver `idx_local`). Eram fixos em
     * 256: do slot 256 em diante o local perdia o tipo declarado (`int v299 =
     * 1` aceitava "texto"), a via rápida e a captura por closure — calado. */
    /* Slots que viram CÉLULA porque alguma funct aninhada os usa. */
    unsigned char *celula;
    /* Célula criada pelo `marca_celulas` que ainda não recebeu nada: o slot
     * EXISTE mas o nome ainda não vale nada. Sem separar os dois, o `for each`
     * achava que a variável dele já existia e tentava salvar o valor "de
     * fora", lendo uma célula vazia. */
    unsigned char *celula_virgem;
    UpvalC  *upvals;
    int32_t  nupvals;
    int32_t  cap_upvals;
    int32_t  idx;
    int32_t  cap_code;
    int32_t  cap_consts;
    /* Índice das constantes (posição + 1; 0 = vazio): a deduplicação varria
     * o vetor inteiro a cada constante nova, e o `<module>` de um arquivo com
     * milhares de declarações tem milhares delas. */
    int32_t *ix_consts;
    int32_t  cap_ix_consts, n_ix_consts;
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
    unsigned char *certo;
    /* Tipo DECLARADO de cada slot (`str s = ...`): 0 = sem tipo, senao o
     * TIPO_* da VM + 1. Toda escrita no slot passa pelo OP_COERCE_DECL, nao so
     * a declaracao. */
    unsigned char *tipo_decl;
    /* `int funct` / `bool funct`: 1 = int, 2 = bool, 0 = sem tipo. Faz o
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
    /* O que um bloco protegido (`try`) criou, pra apagar no alvo do handler
     * (ver `limpa_no_handler`): na funct, o PICO de slots locais que a
     * região alcançou — os blocos de dentro já devolveram os deles, mas o
     * valor continua no slot; no módulo, o índice global de cada nome criado,
     * em ordem, sem truncar (o `mod_criados` solta o nome quando o bloco
     * fecha). */
    int32_t  pico_locais;
    int32_t *mod_hist;
    int32_t  n_mod_hist;
    int32_t  cap_mod_hist;
    IdxNomes ix_mod;      /* nome -> posição em `mod_criados` */
    /* Tipagem estática: um SimInfo por slot (junto dos vetores de slot) e um
     * por nome de `mod_criados` (mesmo índice, mesma vida). */
    SimInfo *sim;
    SimInfo *mod_sim;
    int32_t  cap_mod_sim;
    /* Todo nome que ESTA funct liga (parâmetro, atribuição, `for each`,
     * `catch`, import...), em qualquer ponto do corpo — é o que diz, antes de
     * rodar, que um nome lido aqui existe. Vazio no módulo (lá vale o
     * conjunto do arquivo, em `C`). */
    char   **ligados;
    int32_t  nligados, cap_ligados;
    /* O tipo de retorno escrito (`str funct f()`), canônico, e o nome da
     * funct pra mensagem. NULL = sem tipo. */
    const char *tipo_ret_nome;
    const char *nome_funct;
};

/* `break`/`continue` precisam saber o laço em que estão. O endereço do fim
 * do laço só é conhecido DEPOIS de compilar o corpo, então os saltos ficam
 * pendentes aqui e são corrigidos (backpatch) no fechamento. */
#define MAX_LACOS 32

/* Saltos pendentes (backpatch), em vetor que CRESCE. Eram vetores fixos —
 * 64 no laço, 64 no if e no match, 32 no catch e nos padrões — e cada um
 * tratava o estouro do seu jeito: o laço recusava (65 `break` no mesmo laço
 * era "limite do compilador"), os outros DESCARTAVAM o salto calados e
 * deixavam um JUMP com alvo 0 no bytecode. */
typedef struct { int32_t *v; int32_t n, cap; } Saltos;

/* Um `finally` pendente: o bloco e em que laço/função o try foi aberto.
 *
 * `armados`: quantos handlers ESTE construto tem armados no ponto que está
 * compilando agora (1 no corpo do try; no catch, 1 se há finally — o try de
 * dentro que repropaga o erro do catch —, senão 0). A saída por `return`,
 * `break` ou `continue` desarma esses handlers (POP_TRY) antes de rodar o
 * finally: sem isso o `break` deixava o handler armado (o `raise` depois do
 * laço caía no catch velho), o `continue` empilhava um por volta (70 mil
 * voltas davam RecursionError) e o finally que levantava rodava duas vezes.
 *
 * `fecha`: o `using` não tem bloco de finally, tem o recurso a fechar — o
 * nome dele (`carrega_nome` + CLOSE_SE_TEM).
 *
 * `dono_laco`: o `for each` dono do gerador do cabeçalho (ITER_DONO), pelo
 * índice dele em `c->lacos` (-1 nos outros). A saída por `break` e `return`
 * emite DONO_SAI com a profundidade do gerador na pilha; o `continue` do
 * próprio laço não sai dele. */
struct FinPend { PSNode *bloco; int laco; const void *unidade; int armados; const char *fecha; int dono_laco; };

typedef struct {
    int32_t inicio;                  /* alvo do `continue` */
    Saltos  saidas;                  /* saltos do `break`, a corrigir */
    Saltos  continues;               /* saltos do `continue`, a corrigir */
    /* O `pilha_extra` do compilador quando o laço abriu. Um `break`/`continue`
     * dentro de um construto que guarda valores na pilha POR CIMA do estado
     * deste laço (o sujeito do `match`, a mensagem e o tipo da exceção numa
     * cópia do `finally`, o valor de um `return` em cima da qual o finally
     * roda) tem que descartar esses valores também: a diferença entre o
     * `pilha_extra` de agora e este. Sem isso o `continue` num `case` deixava
     * o sujeito em cima de [container, índice] e o ITER_NEXT lia lixo. */
    int     extra_base;
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

/* Uma classe com o corpo em compilação (ver `C.corpo`). Mora na pilha do C,
 * no `case N_ENTITY_DECL`. */
typedef struct CorpoClasse {
    PSNode  *no;                  /* a declaração */
    int32_t  ci;                  /* o descritor, em `out->classes` */
    /* declarada dentro de funct: o nome dela é local de lá, e o método chega
     * nela pela captura (o método dessa classe é closure da funct) */
    int      local;
    struct CorpoClasse *fora;
} CorpoClasse;

typedef struct {
    PSPrograma *out;
    int32_t     cap_protos;
    int32_t     cap_globais;
    IdxNomes    ix_globais;     /* nome -> posição em `out->globais` */
    Laco        lacos[MAX_LACOS];
    int         nlacos;
    /* Valores que os construtos abertos guardam na pilha durante o corpo
     * deles, por cima do estado dos laços (ver `Laco.extra_base`): +1 no
     * corpo de um `case` (o sujeito), +2 numa cópia do `finally` no caminho
     * da exceção (mensagem e tipo), +1 na cópia que um `return` emite (o
     * valor devolvido). */
    int         pilha_extra;
    /* contador de compreensões de lista, pra o nome do acumulador ser único
     * quando uma está dentro da outra */
    int         n_listcomp;
    int32_t     cap_classes;
    int32_t     cap_models;
    int32_t     cap_enums;
    /* `@NonNull` visto, esperando a funct que ele decora. */
    int         pendente_nonnull;
    /* `@static` visto, esperando a funct que ele decora — vira Proto.eh_static */
    int         pendente_static;
    /* profundidade de decorador EMPILHADO em compilação: cada nível guarda o
     * próprio decorador num temporário distinto (`$reg0`, `$reg1`…), senão
     * o de dentro sobrescrevia o de fora antes de ele ser usado */
    int         decor_prof;
    /* Profundidade da descida da árvore (compilação e checagem). O TETO
     * CONTADO é o que faz o MESMO programa dar o mesmo resultado em qualquer
     * `ulimit -s` (motor-1): a folga medida da pilha sozinha seguia o make 3.81
     * da Apple (64 MB) e o mesmo 8000 blocos passava lá e falhava nos 8 MB. */
    int         fundo;
    /* `finally` PENDENTES (try aninhado): o bloco é emitido inline nas saídas
     * do try, mas `return`, `break` e `continue` saltam por fora — sem isto o
     * finally simplesmente NÃO rodava nesses três caminhos. Cada entrada
     * guarda o bloco e em que laço/função ele estava, pra saber quais rodar. */
    struct FinPend *fin_pend;     /* vetor que cresce: o 33º try aninhado não
                                   * era registrado e o finally dele não rodava
                                   * em return/break */
    int         nfinally;
    int         cap_finally;
    /* profundidade de `try` aberto — `yield` dentro de um é recusado */
    int         dentro_try;
    /* quantos `try` em volta têm um `catch` que pega o `ImportError` (o tipo,
     * um pai dele ou o catch sem tipo): o módulo que não existe ali é o
     * import opcional, e o erro é do programa rodando, não do `--check` */
    int         import_protegido;
    /* profundidade de `count each` aberto — muda o que `return;` devolve */
    int         dentro_count_each;
    /* Nome do primeiro pai da Entity cujos métodos estão sendo compilados —
     * é o que `base(...)` precisa pra buscar o __init__ no lugar certo. */
    const char *entity_pai;
    /* Estamos dentro de uma Entity (com ou sem pai)? `base()` sem pai é
     * no-op aqui e erro fora. */
    int dentro_entity;
    /* O nó da Entity cujo corpo/métodos estão sendo compilados. É por ele que
     * um nome solto dentro da classe resolve pra campo `static` dela
     * (`mapp` -> `App.mapp`), no corpo (decoradores) e nos métodos. */
    PSNode *entity_no;
    /* As classes com o corpo em compilação, da de dentro pra de fora (classe
     * declarada num método de outra). É o "de dentro" do `private`: o proto
     * nasce com a classe da de dentro (`PSProto.classe`), e o checador
     * pergunta a mesma coisa à cadeia. NULL fora de classe. */
    struct CorpoClasse *corpo;
    /* A próxima funct compilada é método da classe de `corpo` (consumido no
     * `compila_funct_em`, como o `pendente_static`): o `self` dela é dessa
     * classe. Funct aninhada no método não é método. */
    int pendente_metodo;
    /* Linha do fonte do nó sendo compilado agora — o `emite()` grava por
     * instrução, pra o erro de runtime dizer ONDE aconteceu. */
    int32_t linha_atual;
    int32_t coluna_atual;
    /* `range` foi ligado em algum ponto do arquivo (inclusive depois do
     * laço)? A especialização do `for each ... in range` precisa saber. Antes
     * era uma varredura da árvore inteira a cada laço; a árvore inteira não
     * existe mais, e a resposta é colhida na primeira passada. */
    int range_ligado;

    /* ── compilação em DUAS PASSADAS sobre o fonte (ver ps_compila_fonte) ──
     *
     * A árvore do arquivo inteiro custava ~1,6 KB por linha e ficava na
     * memória até o fim da compilação: 1 milhão de linhas eram gigabytes, e a
     * máquina entrava na swap. Agora o arquivo é lido uma declaração de topo
     * por vez. Na primeira passada, o compilador guarda de cada declaração só
     * o que OUTRAS declarações precisam (a "poda": cabeçalhos, campos, e os
     * fatos dos corpos que o checador consulta) na arena `keep`; na segunda,
     * lê de novo, gera o código de cada declaração e a solta.
     *
     * `gemeos`: o nó podado que corresponde a um nó da segunda passada, pela
     * posição no fonte. É o que o `SimInfo.decl` guarda — a declaração da
     * segunda passada morre no fim do statement dela, o gêmeo vive até o fim. */
    int      fase;               /* 1 = colhendo, 2 = gerando código, 0 = árvore inteira */
    PSArena *keep;
    struct Gemeo { uint64_t chave; PSNode *no; } *gemeos;
    int32_t  ngemeos, cap_gemeos;
    /* Tipo declarado dos nomes de MODULO (`str s = ...` no topo do arquivo,
     * dentro ou fora de bloco), colhido numa passada antes de compilar: uma
     * funct compilada ANTES da declaracao ainda precisa saber o tipo pra
     * conferir o write-through (`s = 5` de dentro dela escreve no `s` do
     * modulo). 0 = sem tipo, senao TIPO_* + 1. */
    char   **tipos_topo_nomes;
    unsigned char *tipos_topo;
    int32_t  n_tipos_topo, cap_tipos_topo;
    /* `import *`: quem responde os nomes que um módulo exporta (NULL no
     * `--check`), e a resposta de cada `*` do topo do arquivo, colhida antes
     * de compilar qualquer statement — o `liga_o_nome` de um laço que vem
     * ANTES do import já precisa saber se o `*` liga `range`. */
    const PSResolvedor *resolve;
    /* `incompleto`: resolvido, mas um ciclo de `*` cortou a lista — o resto
     * dos nomes só existe rodando, e o checador não pode negar nome nenhum */
    struct { PSNode *no; char **nomes; int32_t n; int resolvido; int incompleto; } *estrelas;
    int32_t  nestrelas, cap_estrelas;
    /* o statement de topo do arquivo sendo compilado agora: `*` só vale nele */
    PSNode  *stmt_topo;
    /* nomes de MÓDULO gravados de dentro de uma funct com `global x` — o
     * arquivo os liga, então saem no import como os de topo */
    char   **globais_gravados;
    int32_t  nglobais_gravados;
    /* O que pode gravar um campo de objeto, colhido do arquivo inteiro na
     * primeira passada (ver `tp_campo_nunca_nasce`): os nomes gravados por
     * `obj.x = v` (qualquer receptor) e pelos campos declarados em funct (do
     * pool `tpool`), um `.__init__` chamado pelo nome (roda o construtor do
     * pai) e um módulo `.pr` importado (grava o que quiser). */
    const char **membros_gravados;
    int32_t  nmembros_gravados, cap_membros_gravados;
    int      chama_init_nome;
    int      importa_pr;

    /* ── tipagem estática ── */
    /* Textos de tipo montados aqui (a união `a|b`): o compilador é dono. */
    char   **tpool;
    int32_t  ntpool, cap_tpool;
    /* Os nomes do escopo do ARQUIVO que persistem (fora de bloco), com o tipo
     * da declaração ou da primeira atribuição, na ordem do fonte. Colhidos
     * antes de compilar: uma funct compilada antes de `x = 1` já precisa
     * saber que o `x` de fora é int (a escrita dela cai nele). */
    struct { const char *nome; SimInfo s; } *topo;
    /* Todo nome que o ARQUIVO liga no escopo dele, em qualquer bloco, mais o
     * que uma funct grava com `global x`. `nomes_incertos`: algum `import *`
     * não se resolveu, e aí não dá pra dizer que um nome não existe. */
    char   **ligados_mod;
    int32_t  nligados_mod, cap_ligados_mod;
    IdxNomes ix_ligados_mod;   /* nome -> posição em `ligados_mod` */
    /* Só os que o ARQUIVO liga fora de bloco (e os de `global x`): são os
     * globais que toda funct enxerga. O nome que só nasce dentro de um bloco
     * é do bloco — a funct escrita fora dele não o enxerga (ver
     * `tp_ligado_no_escopo`). */
    char   **ligados_mod_raso;
    int32_t  nligados_mod_raso, cap_ligados_mod_raso;
    IdxNomes ix_ligados_mod_raso;
    /* A última pergunta do `tp_ligado_no_escopo` achou o nome só num bloco
     * que não envolve a funct: a frase do NameError diz isso. */
    int      nome_oculto;
    int      nomes_incertos;
    /* o arquivo importa o jinker: o `request` é injetado quando o app sobe */
    int      usa_jinker;
    /* Profundidade de HANDLER de rota (motor-6): > 0 dentro do corpo de uma
     * funct decorada por `@app.route`/`@app.get`/... — é o único lugar onde o
     * `request` existe. Fora dele o nome é NameError, como rodando. */
    int      em_handler;
    int32_t  ntopo, cap_topo;
    IdxNomes ix_topo;          /* nome -> posição em `topo` */
    /* Toda Entity, model e enum do arquivo, em qualquer profundidade, por
     * nome — é o que diz se um nome de tipo existe e quem herda de quem. */
    PSNode **tipos_arq;
    int32_t  ntipos_arq, cap_tipos_arq;
    /* Os nomes que mais de uma declaração de tipo usa no arquivo (a classe
     * redeclarada, que é erro; o `if` e o `else` declarando cada um a sua; duas
     * functs com uma classe local de mesmo nome): o nome existe, mas não se
     * sabe QUAL classe um valor dele é, e conferir membro contra a primeira
     * daria veredito falso (o erro em cascata da redeclaração). */
    char   **tipos_ambiguos;
    int32_t  ntipos_ambiguos;
    /* O que se sabe do membro `static` que um nome solto alcança de dentro da
     * classe (tipo do campo, assinatura do método) — montado a cada consulta. */
    SimInfo  sim_estatico;
    /* métodos sem `self` que ganharam um sintetizado (tp_self_dos_metodos);
     * a AST volta ao que era no fim da compilação */
    struct SelfSint *self_sint;
    int32_t  n_self_sint, cap_self_sint;
    /* Os erros de tipo: NÃO param a compilação — o `--check` lista todos. */
    PSErroTipo *erros;
    int32_t  nerros, cap_erros;
    /* O valor que vai ser gravado agora, pro `guarda_nome_modo` conferir:
     * quem grava e sabe o valor preenche antes e limpa depois. `tem_valor`
     * 0 = gravação sem valor conhecido (desempacotamento, import...). */
    struct { int tem_valor; const char *tipo; PSNode *no; const char *declara; PSNode *decl;
             unsigned char decorado; const char *mod_nativo; const char *membro_nativo;
             const PSModuloAst *mod_pr; const char *mod_pr_nome;
             /* o nome passa a valer OUTRA coisa, de tipo desconhecido — o
              * resultado de um decorador geral sobre a funct */
             unsigned char redefine;
             /* a gravação é a de uma declaração (class/model/enum/funct, o nó
              * em `no`) ligando o nome dela */
             unsigned char declaracao; } grava;
    /* Na chamada de SAÍDA de uma funct tipada (`post` dentro de `int funct`):
     * o tipo que cada argumento posicional tem que ter, e o rótulo do erro.
     * `emite_args_e_chama` consome e limpa. */
    const char *saida_tipo;
    int         saida_indice;       /* -2 = todos os posicionais */
    char        saida_rotulo[300];
    /* O `x.m` que a chamada em curso vai chamar (o N_MEMBER_ACCESS do
     * N_CALL): o erro de membro ausente diz "method" nele e "attribute" numa
     * leitura, e o GET_MEMBER dele leva PS_ARG_CHAMADO pro runtime dizer o
     * mesmo. Por ponteiro: um valor que sobrou de outra chamada nunca é o nó
     * de uma leitura. */
    PSNode     *membro_chamado;
    /* O alvo (nome ou membro) da chamada que está sendo compilada, pela mesma
     * regra do ponteiro: é a chamada direta da saída, que a funct tipada
     * confere; a leitura como valor é outro nó (`tp_saida_como_valor`). */
    PSNode     *alvo_chamado;
} C;

/* "method" se `n` é o membro que uma chamada chama; "attribute" se é leitura */
static const char *tp_rotulo_membro(const C *c, const PSNode *n)
{
    return n && n == c->membro_chamado ? "method" : "attribute";
}

/* Resolve o protótipo da unidade AGORA — nunca cacheia o ponteiro. */
#define UP(c, u) (&(c)->out->protos[(u)->idx])

#define CFALHOU(c) (!(c)->out->ok)

/* Erro do COMPILADOR — o que NÃO é culpa de quem escreveu: nó que a VM ainda
 * não emite, limite do motor ou falta de memória. Sai como NotImplementedError
 * (rc 3). Erro do PROGRAMA vai em `cerro_sx` (SyntaxError, rc 2). Durante um
 * tempo os dois se misturaram e `break` fora de laço dizia "não implementado". */
static void cerro(C *c, const char *msg, PSNode *n)
{
    if (!c->out->ok) return;
    c->out->ok = 0;
    snprintf(c->out->erro, sizeof(c->out->erro), "%s", msg);
    c->out->erro_linha = n ? n->line : 0;
    c->out->erro_col = n ? n->col : 0;
    c->out->erro_do_programa = 0;
}

/* Erro do PROGRAMA: quem escreveu errou (SyntaxError), com formatação. */
static void cerro_sx(C *c, PSNode *n, const char *fmt, ...)
{
    if (!c->out->ok) return;
    c->out->ok = 0;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(c->out->erro, sizeof(c->out->erro), fmt, ap);
    va_end(ap);
    c->out->erro_linha = n ? n->line : 0;
    c->out->erro_col = n ? n->col : 0;
    c->out->erro_do_programa = 1;
}

static int fundo_demais(C *c, PSNode *n, int bloco);

/* ── emissão ────────────────────────────────────────────────────────────── */
static int32_t emite(C *c, Unidade *u, int32_t op, int32_t arg)
{
    if (UP(c, u)->ncode + 2 > u->cap_code) {
        /* começa pequeno: uma funct de uma linha usa ~12 palavras, e cada
         * proto nascia com 64 em três vetores (código, linha, coluna) —
         * 768 bytes por função, a maior parte vazia */
        int32_t novo = u->cap_code < 16 ? 16 : u->cap_code * 2;
        int32_t *nc = realloc(UP(c, u)->code, sizeof(int32_t) * (size_t)novo);
        if (!nc) { cerro(c, "sem memoria", NULL); return -1; }
        UP(c, u)->code = nc;
        /* tabelas de linha e coluna crescem junto com o code: UMA entrada por
         * instrução (duas palavras), metade do tamanho do code */
        int32_t *nl = realloc(UP(c, u)->linhas, sizeof(int32_t) * (size_t)(novo / 2));
        if (!nl) { cerro(c, "sem memoria", NULL); return -1; }
        UP(c, u)->linhas = nl;
        int32_t *ncol = realloc(UP(c, u)->colunas, sizeof(int32_t) * (size_t)(novo / 2));
        if (!ncol) { cerro(c, "sem memoria", NULL); return -1; }
        UP(c, u)->colunas = ncol;
        u->cap_code = novo;
    }
    int32_t pos = UP(c, u)->ncode;
    UP(c, u)->linhas[pos / 2]  = c->linha_atual;
    UP(c, u)->colunas[pos / 2] = c->coluna_atual;
    UP(c, u)->code[UP(c, u)->ncode++] = op;
    UP(c, u)->code[UP(c, u)->ncode++] = arg;
    return pos;
}

/* ── saltos pendentes (backpatch) ───────────────────────────────────────── */
static void saltos_push(C *c, Saltos *s, int32_t pos)
{
    if (s->n >= s->cap) {
        int32_t nc = s->cap ? s->cap * 2 : 8;
        int32_t *nv = realloc(s->v, sizeof(int32_t) * (size_t)nc);
        if (!nv) { cerro(c, "sem memoria", NULL); return; }
        s->v = nv; s->cap = nc;
    }
    s->v[s->n++] = pos;
}

/* Aponta todos os saltos pendentes pra `alvo`. */
static void saltos_corrige(C *c, Unidade *u, Saltos *s, int32_t alvo)
{
    for (int32_t i = 0; i < s->n; i++) UP(c, u)->code[s->v[i] + 1] = alvo;
}

static void saltos_solta(Saltos *s)
{
    free(s->v);
    s->v = NULL; s->n = s->cap = 0;
}

/* ── superinstruções: o que o laço quente vira ───────────────────────────
 *
 * `cond; JUMP_IF_FALSE alvo` em que `cond` acabou numa comparação vira UMA
 * instrução `JF_<cmp> alvo` (a comparação e o desvio, sem empilhar o bool).
 * Recebe o índice do JUMP_IF_FALSE recém-emitido e devolve o índice da
 * instrução que passa a carregar o alvo — quem chamou patcha `[idx + 1]`
 * como já fazia. Só reconhece o padrão que acabou de ser emitido; qualquer
 * outra forma de condição fica como está. */
static int32_t funde_salto_falso(C *c, Unidade *u, int32_t salto)
{
    if (salto < 2 || UP(c, u)->ncode != salto + 2) return salto;
    int32_t *code = UP(c, u)->code;
    int32_t jf;
    switch (code[salto - 2]) {
        case OP_LT: jf = OP_JF_LT; break;
        case OP_GT: jf = OP_JF_GT; break;
        case OP_LE: jf = OP_JF_LE; break;
        case OP_GE: jf = OP_JF_GE; break;
        case OP_EQ: jf = OP_JF_EQ; break;
        case OP_NE: jf = OP_JF_NE; break;
        default: return salto;
    }
    code[salto - 2] = jf;
    code[salto - 1] = 0;
    UP(c, u)->ncode = salto;
    return salto - 2;
}

/* `x++`/`x--` como statement, e `x += 1`/`x -= 1`, numa variável DECLARADA
 * `int` com global ou slot direto viram UMA instrução `INC/DEC_*_INT`. O
 * reconhecimento é pelo padrão que o compilador acabou de emitir:
 *   A: LOAD DUP LOAD_CONST(1) ADD|SUB COERCE_DECL(int) STORE POP_TOP
 *   B: LOAD LOAD_CONST(1) ADD|SUB COERCE_DECL(int) STORE
 * O COERCE_DECL de `int` só existe em variável declarada `int` — que por
 * isso é sempre V_INT em runtime, e é o que a instrução fundida assume. O
 * estouro de 64 bits dá o mesmo erro que o COERCE_DECL dava. Nenhum salto
 * aponta pra dentro do trecho: é o código de um statement só, e alvo que
 * apontava pro começo dele continua apontando pra instrução fundida. */
static void funde_incremento(C *c, Unidade *u)
{
    PSProto *p = UP(c, u);
    int32_t *code = p->code;
    int32_t n = p->ncode;
    int32_t ini, k, op, cd, st;
    if (n >= 14 && code[n - 2] == OP_POP_TOP && code[n - 12] == OP_DUP) {
        ini = n - 14; k = n - 10; op = n - 8; cd = n - 6; st = n - 4;
    } else if (n >= 10) {
        ini = n - 10; k = n - 8; op = n - 6; cd = n - 4; st = n - 2;
    } else return;
    if (code[k] != OP_LOAD_CONST) return;
    if (code[op] != OP_ADD && code[op] != OP_SUB) return;
    if (code[cd] != OP_COERCE_DECL || (code[cd + 1] & 15) != PS_TIPO_INT) return;
    int32_t kidx = code[k + 1];
    if (kidx < 0 || kidx >= p->nconsts || p->consts[kidx].kind != K_INT || p->consts[kidx].i != 1) return;
    if (code[ini + 1] != code[st + 1]) return;
    int32_t novo;
    if (code[ini] == OP_LOAD_GLOBAL && code[st] == OP_STORE_GLOBAL)
        novo = code[op] == OP_ADD ? OP_INC_GLOBAL_INT : OP_DEC_GLOBAL_INT;
    else if (code[ini] == OP_LOAD_LOCAL && code[st] == OP_STORE_LOCAL)
        novo = code[op] == OP_ADD ? OP_INC_LOCAL_INT : OP_DEC_LOCAL_INT;
    else return;
    code[ini] = novo;
    p->ncode = ini + 2;
}

/* A proto TERMINOU de ser compilada: os vetores dela encolhem ao tamanho
 * usado. Todas as protos do arquivo ficam vivas até a VM copiá-las, então a
 * folga de cada uma somava no pico — num arquivo de 200 mil funct, era a maior
 * parte da memória do compilador. */
static void proto_encolhe(C *c, Unidade *u)
{
    PSProto *p = UP(c, u);
    if (p->ncode > 0 && p->ncode < u->cap_code) {
        size_t t = sizeof(int32_t) * (size_t)p->ncode;
        size_t t2 = sizeof(int32_t) * (size_t)(p->ncode / 2);
        int32_t *a = realloc(p->code, t);     if (a) p->code = a;
        int32_t *b = realloc(p->linhas, t2);  if (b) p->linhas = b;
        int32_t *d = realloc(p->colunas, t2); if (d) p->colunas = d;
        if (a && b && d) u->cap_code = p->ncode;
    }
    if (p->nconsts > 0 && p->nconsts < u->cap_consts) {
        PSConst *k = realloc(p->consts, sizeof(PSConst) * (size_t)p->nconsts);
        if (k) { p->consts = k; u->cap_consts = p->nconsts; }
    }
    /* o índice das constantes só serve enquanto a unidade compila */
    free(u->ix_consts);
    u->ix_consts = NULL; u->cap_ix_consts = u->n_ix_consts = 0;
}

/* Dedup por tipo E valor: `1` e `True` não podem colidir, senão o pool
 * devolveria o índice de um valor de outro tipo. */
/* A MESMA igualdade que a deduplicação sempre usou: texto por bytes, flo por
 * `==` (então 0.0 e -0.0 são a mesma constante, e NaN nunca repete). */
static int const_igual(const PSConst *e, PSConstKind k, int64_t i, double d,
                       const char *s, int32_t slen)
{
    if (e->kind != k) return 0;
    if (k == K_STR || k == K_BIGINT || k == K_BYTES)
        return e->slen == slen && memcmp(e->s, s, (size_t)slen) == 0;
    if (k == K_FLO) return e->d == d;
    if (k == K_NULL) return 1;
    return e->i == i;
}

/* O hash respeita a igualdade acima: -0.0 cai no balde do 0.0. */
static uint32_t const_hash(PSConstKind k, int64_t i, double d, const char *s, int32_t slen)
{
    uint32_t h = 2166136261u ^ (uint32_t)k;
    const unsigned char *b; size_t n;
    if (k == K_STR || k == K_BIGINT || k == K_BYTES) { b = (const unsigned char *)s; n = (size_t)slen; }
    else if (k == K_FLO) { if (d == 0.0) d = 0.0; b = (const unsigned char *)&d; n = sizeof(d); }
    else if (k == K_NULL) { b = NULL; n = 0; }
    else { b = (const unsigned char *)&i; n = sizeof(i); }
    for (size_t q = 0; q < n; q++) { h ^= b[q]; h *= 16777619u; }
    return h;
}

/* Põe a constante `x` (já no vetor) no índice da unidade. -1 = sem memória. */
static int const_indexa(Unidade *u, const PSConst *consts, int32_t x)
{
    if ((u->n_ix_consts + 1) * 10 > u->cap_ix_consts * 7) {
        int32_t nc = u->cap_ix_consts ? u->cap_ix_consts * 2 : 8;
        int32_t *nv = calloc((size_t)nc, sizeof(int32_t));
        if (!nv) return -1;
        for (int32_t q = 0; q < u->cap_ix_consts; q++) {
            int32_t v = u->ix_consts[q];
            if (!v) continue;
            const PSConst *e = &consts[v - 1];
            uint32_t j = const_hash(e->kind, e->i, e->d, e->s, e->slen) & (uint32_t)(nc - 1);
            while (nv[j]) j = (j + 1) & (uint32_t)(nc - 1);
            nv[j] = v;
        }
        free(u->ix_consts);
        u->ix_consts = nv; u->cap_ix_consts = nc;
    }
    const PSConst *e = &consts[x];
    uint32_t j = const_hash(e->kind, e->i, e->d, e->s, e->slen) & (uint32_t)(u->cap_ix_consts - 1);
    while (u->ix_consts[j]) j = (j + 1) & (uint32_t)(u->cap_ix_consts - 1);
    u->ix_consts[j] = x + 1;
    u->n_ix_consts++;
    return 0;
}

static int32_t idx_const(C *c, Unidade *u, PSConstKind k,
                         int64_t i, double d, const char *s, int32_t slen)
{
    if (u->cap_ix_consts) {
        uint32_t m = (uint32_t)u->cap_ix_consts - 1;
        for (uint32_t j = const_hash(k, i, d, s, slen) & m; u->ix_consts[j]; j = (j + 1) & m)
            if (const_igual(&UP(c, u)->consts[u->ix_consts[j] - 1], k, i, d, s, slen))
                return u->ix_consts[j] - 1;
    }
    if (UP(c, u)->nconsts + 1 > u->cap_consts) {
        int32_t novo = u->cap_consts < 4 ? 4 : u->cap_consts * 2;
        PSConst *nc = realloc(UP(c, u)->consts, sizeof(PSConst) * (size_t)novo);
        if (!nc) { cerro(c, "sem memoria", NULL); return -1; }
        UP(c, u)->consts = nc;
        u->cap_consts = novo;
    }
    PSConst *e = &UP(c, u)->consts[UP(c, u)->nconsts];
    memset(e, 0, sizeof(*e));
    e->kind = k; e->i = i; e->d = d;
    if (k == K_STR || k == K_BIGINT || k == K_BYTES) {
        e->s = malloc((size_t)slen + 1);
        if (!e->s) { cerro(c, "sem memoria", NULL); return -1; }
        memcpy(e->s, s, (size_t)slen);
        e->s[slen] = '\0';
        e->slen = slen;
    }
    if (const_indexa(u, UP(c, u)->consts, UP(c, u)->nconsts) != 0) {
        cerro(c, "sem memoria", NULL); return -1;
    }
    return UP(c, u)->nconsts++;
}

static int32_t idx_global(C *c, const char *nome)
{
    int32_t ja = ix_busca(&c->ix_globais, nome);
    if (ja >= 0) return ja;
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
    if (ix_poe(&c->ix_globais, copia, c->out->nglobais) != 0) {
        free(copia); cerro(c, "sem memoria", NULL); return -1;
    }
    c->out->globais[c->out->nglobais] = copia;
    return c->out->nglobais++;
}

/* Abre a faixa de vida de um nome local, pro debugger saber que variável mora
 * em que slot em cada ponto do bytecode. Falha aqui não é erro de compilação:
 * sem a tabela o programa roda igual, só o painel de variáveis fica pobre. */
static void vardbg_abre(C *c, Unidade *u, const char *nome, int32_t slot)
{
    PSProto *p = UP(c, u);
    PSVarDbg *nv = realloc(p->vars, sizeof(PSVarDbg) * (size_t)(p->nvars + 1));
    if (!nv) return;
    p->vars = nv;
    char *copia = strdup(nome);
    if (!copia) return;
    p->vars[p->nvars].nome   = copia;
    p->vars[p->nvars].slot   = slot;
    p->vars[p->nvars].ip_ini = p->ncode;
    p->vars[p->nvars].ip_fim = -1;
    p->nvars++;
}

/* Fecha a faixa dos slots >= marca: eles morrem no fim do bloco e o mesmo slot
 * passa a ser outra variável. Só a entrada ABERTA mais recente de cada slot é
 * fechada — as anteriores já têm fim próprio. */
static void vardbg_fecha(C *c, Unidade *u, int32_t marca)
{
    PSProto *p = UP(c, u);
    for (int32_t i = p->nvars - 1; i >= 0; i--) {
        if (p->vars[i].ip_fim < 0 && p->vars[i].slot >= marca)
            p->vars[i].ip_fim = p->ncode;
    }
}

static int32_t slot_novo(C *c, Unidade *u, const char *nome);

/* O slot do nome, criando se não há. A busca vai do último pro primeiro: a
 * variável de um bloco de dentro tem slot próprio (`bloco_abre`) e sombreia a
 * de mesmo nome de fora enquanto o bloco dura. */
static int32_t idx_local(C *c, Unidade *u, const char *nome)
{
    for (int32_t i = u->nlocais - 1; i >= 0; i--)
        if (strcmp(u->locais[i], nome) == 0) return i;
    return slot_novo(c, u, nome);
}

/* Um slot NOVO pro nome, mesmo que outro slot já o tenha. */
static int32_t slot_novo(C *c, Unidade *u, const char *nome)
{
    if (u->nlocais + 1 > u->cap_locais) {
        int32_t novo = u->cap_locais < 8 ? 8 : u->cap_locais * 2;
        char **nl = realloc(u->locais, sizeof(char *) * (size_t)novo);
        if (!nl) { cerro(c, "sem memoria", NULL); return -1; }
        u->locais = nl;
        /* os vetores por slot crescem junto, com a parte nova zerada */
        unsigned char **vets[] = { &u->celula, &u->celula_virgem, &u->certo, &u->tipo_decl };
        for (size_t k = 0; k < sizeof(vets) / sizeof(vets[0]); k++) {
            unsigned char *nv = realloc(*vets[k], (size_t)novo);
            if (!nv) { cerro(c, "sem memoria", NULL); return -1; }
            memset(nv + u->cap_locais, 0, (size_t)(novo - u->cap_locais));
            *vets[k] = nv;
        }
        SimInfo *ns = realloc(u->sim, sizeof(SimInfo) * (size_t)novo);
        if (!ns) { cerro(c, "sem memoria", NULL); return -1; }
        memset(ns + u->cap_locais, 0, sizeof(SimInfo) * (size_t)(novo - u->cap_locais));
        u->sim = ns;
        u->cap_locais = novo;
    }
    size_t n = strlen(nome);
    char *copia = malloc(n + 1);
    if (!copia) { cerro(c, "sem memoria", NULL); return -1; }
    memcpy(copia, nome, n + 1);
    u->locais[u->nlocais] = copia;
    /* o slot reaproveitado (um bloco que fechou o devolveu) nasce limpo: a
     * célula de bloco que morou nele não passa pro nome novo */
    u->celula[u->nlocais] = 0;
    u->celula_virgem[u->nlocais] = 0;
    u->certo[u->nlocais] = 0;
    u->tipo_decl[u->nlocais] = 0;
    memset(&u->sim[u->nlocais], 0, sizeof(SimInfo));
    /* nlocals = MARCA D'ÁGUA (maior slot já usado + 1), não o corrente: com
     * escopo de bloco os slots são reaproveitados (nlocais encolhe no fim do
     * bloco), e o frame precisa caber o pico, não o valor do momento. */
    if (u->nlocais + 1 > UP(c, u)->nlocals) UP(c, u)->nlocals = u->nlocais + 1;
    vardbg_abre(c, u, nome, u->nlocais);
    if (u->nlocais + 1 > u->pico_locais) u->pico_locais = u->nlocais + 1;
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
static void escopo_trunca(C *c, Unidade *u, int32_t marca)
{
    if (u->eh_modulo) {
        for (int32_t i = u->n_mod_criados - 1; i >= marca; i--) {
            ix_tira(&u->ix_mod, u->mod_criados[i]);   /* antes do free: o índice aponta pra ela */
            free(u->mod_criados[i]);
        }
        if (u->n_mod_criados > marca) u->n_mod_criados = marca;
    } else {
        vardbg_fecha(c, u, marca);
        for (int32_t i = u->nlocais - 1; i >= marca; i--) {
            free(u->locais[i]);
            /* o slot volta LIMPO: sem zerar o tipo, o próximo nome que
             * caísse nele herdava o tipo declarado do bloco que já fechou */
            u->certo[i] = 0;
            u->tipo_decl[i] = 0;
            u->celula[i] = 0;
            u->celula_virgem[i] = 0;
            memset(&u->sim[i], 0, sizeof(SimInfo));
        }
        if (u->nlocais > marca) u->nlocais = marca;
    }
}

/* Fecha um bloco comum (não-laço): apaga em runtime e some da compilação. */
static void escopo_fecha(C *c, Unidade *u, int32_t marca)
{
    escopo_emite_clears(c, u, marca);
    escopo_trunca(c, u,marca);
}

/* Módulo: registra um global CRIADO por atribuição (idempotente). Nome já
 * registrado (em qualquer escopo já aberto) é write-through — não reentra. */
static void mod_criados_add(C *c, Unidade *u, const char *nome)
{
    if (ix_busca(&u->ix_mod, nome) >= 0) return;
    if (u->n_mod_criados + 1 > u->cap_mod_criados) {
        int32_t novo = u->cap_mod_criados < 8 ? 8 : u->cap_mod_criados * 2;
        char **nl = realloc(u->mod_criados, sizeof(char *) * (size_t)novo);
        if (!nl) { cerro(c, "sem memoria", NULL); return; }
        u->mod_criados = nl;
        u->cap_mod_criados = novo;
    }
    if (u->n_mod_criados + 1 > u->cap_mod_sim) {
        int32_t novo = u->cap_mod_sim < 8 ? 8 : u->cap_mod_sim * 2;
        SimInfo *ns = realloc(u->mod_sim, sizeof(SimInfo) * (size_t)novo);
        if (!ns) { cerro(c, "sem memoria", NULL); return; }
        u->mod_sim = ns;
        u->cap_mod_sim = novo;
    }
    size_t n = strlen(nome);
    char *copia = malloc(n + 1);
    if (!copia) { cerro(c, "sem memoria", NULL); return; }
    memcpy(copia, nome, n + 1);
    if (ix_poe(&u->ix_mod, copia, u->n_mod_criados) != 0) {
        free(copia); cerro(c, "sem memoria", NULL); return;
    }
    memset(&u->mod_sim[u->n_mod_criados], 0, sizeof(SimInfo));
    u->mod_criados[u->n_mod_criados++] = copia;
    /* o histórico, pro handler de um `try` saber o que o bloco criou */
    if (u->n_mod_hist + 1 > u->cap_mod_hist) {
        int32_t novo = u->cap_mod_hist < 16 ? 16 : u->cap_mod_hist * 2;
        int32_t *nh = realloc(u->mod_hist, sizeof(int32_t) * (size_t)novo);
        if (!nh) { cerro(c, "sem memoria", NULL); return; }
        u->mod_hist = nh;
        u->cap_mod_hist = novo;
    }
    u->mod_hist[u->n_mod_hist++] = idx_global(c, copia);
}

/* No alvo de um handler: apaga o que o bloco protegido criou. Os CLEAR do fim
 * do bloco só existem no caminho normal; o salto pro handler os pula, e o slot
 * que um nome de depois reaproveita (a variável do catch, a dica de uma global
 * lida) ainda guardava o valor do local do try — `return padrao` devolvia o
 * `sobrenome` do try, o `post` do catch chamava um valor velho, e no topo a
 * variável do try sobrevivia ao bloco. Funct: os slots de `marca` até o pico
 * que a região alcançou (inclusive os de blocos de dentro, já fechados).
 * Módulo: os nomes criados desde `hist0`. */
static void limpa_no_handler(C *c, Unidade *u, int32_t marca, int32_t hist0)
{
    if (u->eh_modulo) {
        for (int32_t k = u->n_mod_hist - 1; k >= hist0; k--)
            emite(c, u, OP_CLEAR_GLOBAL, u->mod_hist[k]);
    } else {
        for (int32_t i = u->pico_locais - 1; i >= marca; i--)
            emite(c, u, OP_CLEAR_LOCAL, i);
    }
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
    if (!strcmp(s, "**")) return OP_POW;
    if (!strcmp(s, "//")) return OP_FLOORDIV;
    if (!strcmp(s, "<"))  return OP_LT;
    if (!strcmp(s, ">"))  return OP_GT;
    if (!strcmp(s, "<=")) return OP_LE;
    if (!strcmp(s, ">=")) return OP_GE;
    if (!strcmp(s, "==")) return OP_EQ;
    if (!strcmp(s, "!=")) return OP_NE;
    if (!strcmp(s, "|"))  return OP_BIT_OR;
    if (!strcmp(s, "^"))  return OP_BIT_XOR;
    if (!strcmp(s, "&"))  return OP_BIT_AND;
    if (!strcmp(s, "<<")) return OP_LSHIFT;
    if (!strcmp(s, ">>")) return OP_RSHIFT;
    return -1;
}

/* ── protótipos internos ────────────────────────────────────────────────── */
/* tipagem estática (a seção "tipagem estatica", mais abaixo) */
enum { T_NAO = 0, T_SIM = 1, T_TALVEZ = 2 };
static const char *tp_de(C *c, Unidade *u, PSNode *n);
static int tp_decorador_embutido(PSNode *dec);
static SimInfo *tp_sim_mod_pr(C *c, Unidade *u, PSNode *n);
static PSNode *modulo_decl(const PSModuloAst *ma, const char *nome, int *decorado);
static int tp_classe_importada(C *c, PSNode *d);
static int mesmo_no(const PSNode *a, const PSNode *b);
static int tp_aceita(C *c, const char *d, const char *v, PSNode *no, int declarado);
static void tp_emite_confere(C *c, Unidade *u, const char *rotulo, const char *tipo, int declarado);
static void expr(C *c, Unidade *u, PSNode *n);
static void compila_fstring(C *c, Unidade *u, PSNode *n);
static void stmt(C *c, Unidade *u, PSNode *n);
static int32_t compila_funct(C *c, PSNode *n, Unidade *pai);

/* `__init__` sintetizado a partir dos campos tipados (`nome: str`).
 *
 * Não depende do `@dataentity`: o decorador é só um marcador, e no
 * interpretador a Entity ganha o init pelos campos com ou sem ele. Gerar
 * bytecode direto (em vez de montar uma AST) evita inventar nós sem posição
 * de origem, que estragariam a mensagem de erro. */
static int32_t sintetiza_init(C *c, PSNode *entidade);
static int32_t sintetiza_campos(C *c, PSNode *entidade, Unidade *pai);
static void compila_unpack_alvo(C *c, Unidade *u, PSNode *alvo);
static void guarda_em_alvo(C *c, Unidade *u, PSNode *e);

/* Os membros que a classe `cls` DECLARA, um por chamada de `f`:
 *   - os métodos (`cls->lista`);
 *   - os campos do corpo (`cls->lista2_alias`), com o tipo do `static`;
 *   - os campos declarados num método (`private str nome = n`, o
 *     N_FIELD_DECL), sem descer em classe declarada lá dentro, que declara os
 *     dela;
 *   - o `__init__` que o compilador gera dos campos de instância quando não
 *     há um escrito (`sintetiza_init`).
 * É a lista que vira a tabela da VM (`PSClassDef.membros`) e a que o checador
 * consulta: a mesma regra dos dois lados. Serve à árvore inteira e ao gêmeo
 * podado (a poda guarda o N_FIELD_DECL do corpo como fato). */
typedef void (*VisitaMembro)(void *ctx, const char *nome, int32_t marcas, const char *tipo);

static void membros_dos_metodos(PSNode *n, VisitaMembro f, void *ctx)
{
    if (!n) return;
    if (n->kind == N_ENTITY_DECL || n->kind == N_MODEL_DECL || n->kind == N_ENUM_DECL) return;
    if (n->kind == N_FIELD_DECL && n->texto) f(ctx, n->texto, n->is_private ? PS_MEMB_PRIVADO : 0, NULL);
    membros_dos_metodos(n->a, f, ctx);
    membros_dos_metodos(n->b, f, ctx);
    membros_dos_metodos(n->c, f, ctx);
    membros_dos_metodos(n->e, f, ctx);
    for (int32_t i = 0; i < n->lista.n; i++)  membros_dos_metodos(n->lista.itens[i], f, ctx);
    for (int32_t i = 0; i < n->lista2.n; i++) membros_dos_metodos(n->lista2.itens[i], f, ctx);
}

static void membros_declarados(PSNode *cls, VisitaMembro f, void *ctx)
{
    if (!cls || cls->kind != N_ENTITY_DECL) return;
    int tem_init = 0, n_inst = 0;
    for (int32_t i = 0; i < cls->lista.n; i++) {
        PSNode *m = cls->lista.itens[i];
        if (!m || m->kind != N_FUNCT_DECL || !m->texto) continue;
        f(ctx, m->texto, m->is_private ? PS_MEMB_PRIVADO : 0, NULL);
        if (strcmp(m->texto, "__init__") == 0) tem_init = 1;
    }
    for (int32_t i = 0; i < cls->lista2_alias.n; i++) {
        PSNode *fl = cls->lista2_alias.itens[i];
        if (!fl) continue;
        if (!fl->is_static) n_inst++;
        if (fl->kind != N_ENTITY_FIELD || !fl->texto) continue;
        f(ctx, fl->texto, (fl->is_private ? PS_MEMB_PRIVADO : 0) | (fl->is_static ? PS_MEMB_ESTATICO : 0)
                          | (!fl->is_static && !fl->a ? PS_MEMB_SEM_VALOR : 0),
          fl->is_static ? fl->texto2 : NULL);
    }
    for (int32_t i = 0; i < cls->lista.n; i++) {
        PSNode *m = cls->lista.itens[i];
        if (m && m->kind == N_FUNCT_DECL) membros_dos_metodos(m->b, f, ctx);
    }
    if (n_inst > 0 && !tem_init) f(ctx, "__init__", 0, NULL);
}

/* Membro declarado duas vezes no CORPO da classe (campo ou método, de
 * instância ou static): o 2º valia pela metade (campo repetido nascia Null e
 * quebrava o tipo; método repetido rodava o 1º). Erro no 2º, antes de rodar,
 * apontando o 1º. Os campos declarados dentro de um método não entram: lá
 * `private str nome = n` é a declaração do campo que o corpo não tem. */
static void terro(C *c, PSNode *n, const char *classe, const char *fmt, ...);

/* O NOME de uma declaração como nó, em `tmp`, pro erro que é do nome
 * (redeclarado, repetido) sublinhar o nome inteiro, não a declaração. Na
 * funct/class/model/enum o nó começa na palavra da declaração, e o parser
 * guarda a posição do nome em `i` (`linha << 32 | coluna`); no campo e no
 * membro de enum o nó já começa no nome. Sem a posição, o próprio nó. */
static PSNode *no_do_nome(PSNode *n, PSNode *tmp)
{
    if (!n || !n->texto) return n;
    int decl = n->kind == N_FUNCT_DECL || n->kind == N_ENTITY_DECL
            || n->kind == N_MODEL_DECL || n->kind == N_ENUM_DECL;
    if (decl && !n->i) return n;
    memset(tmp, 0, sizeof(*tmp));
    tmp->kind = n->kind;
    tmp->line = decl ? (int32_t)(n->i >> 32) : n->line;
    tmp->col = decl ? (int32_t)(uint32_t)n->i : n->col;
    int32_t nc = 0;
    for (const unsigned char *q = (const unsigned char *)n->texto; *q; q++)
        if ((*q & 0xC0) != 0x80) nc++;
    tmp->linha_fim = tmp->line;
    tmp->col_fim = tmp->col + nc;
    return tmp;
}

static void membros_repetidos(C *c, PSNode *cls)
{
    if (!cls || cls->kind != N_ENTITY_DECL) return;
    PSNodeVec *vs[2] = { &cls->lista, &cls->lista2_alias };
    for (int a = 0; a < 2; a++)
        for (int32_t i = 0; i < vs[a]->n; i++) {
            PSNode *m = vs[a]->itens[i];
            if (!m || !m->texto || (m->kind != N_FUNCT_DECL && m->kind != N_ENTITY_FIELD)) continue;
            PSNode *primeiro = NULL;
            for (int b = 0; b < 2; b++)
                for (int32_t k = 0; k < vs[b]->n; k++) {
                    PSNode *o = vs[b]->itens[k];
                    if (!o || o == m || !o->texto || (o->kind != N_FUNCT_DECL && o->kind != N_ENTITY_FIELD)
                            || strcmp(o->texto, m->texto) != 0)
                        continue;
                    if (o->line > m->line || (o->line == m->line && o->col >= m->col)) continue;
                    if (!primeiro || o->line < primeiro->line
                            || (o->line == primeiro->line && o->col < primeiro->col))
                        primeiro = o;
                }
            PSNode tmp;
            if (primeiro)
                terro(c, no_do_nome(m, &tmp), "SyntaxError",
                      "membro '%s' repetido na classe %s (o primeiro esta na linha %d)",
                      m->texto, cls->texto ? cls->texto : "?", primeiro->line);
        }
}

/* O mesmo nome declarado mais de uma vez na mesma classe (no corpo e num
 * método, já que no corpo repetido é erro) vale como UMA declaração: private
 * se alguma for, static só se todas forem campo static, e "sem valor" só se
 * nenhuma der valor (o campo declarado num método grava quando ele roda). */
static int32_t membro_junta_marcas(int32_t antes, int32_t agora)
{
    return ((antes | agora) & PS_MEMB_PRIVADO) | (antes & agora & (PS_MEMB_ESTATICO | PS_MEMB_SEM_VALOR));
}

typedef struct { C *c; PSClassDef *def; } TabMembros;

static void membro_tab_poe(void *ctx, const char *nome, int32_t marcas, const char *tipo)
{
    TabMembros *t = ctx;
    PSClassDef *def = t->def;
    for (int32_t k = 0; k < def->nmembros; k++)
        if (strcmp(def->membros[k].nome, nome) == 0) {
            def->membros[k].marcas = membro_junta_marcas(def->membros[k].marcas, marcas);
            return;
        }
    PSMembroDecl *nv = realloc(def->membros, sizeof(PSMembroDecl) * (size_t)(def->nmembros + 1));
    if (!nv) { cerro(t->c, "sem memoria", NULL); return; }
    def->membros = nv;
    char *nm = strdup(nome);
    char *tp = tipo ? strdup(tipo) : NULL;
    if (!nm || (tipo && !tp)) { free(nm); free(tp); cerro(t->c, "sem memoria", NULL); return; }
    def->membros[def->nmembros].nome = nm;
    def->membros[def->nmembros].marcas = marcas;
    def->membros[def->nmembros].tipo = tp;
    def->nmembros++;
}

/* `cls` (nó da classe) declara `nome`? 1 = sim, com as marcas em `*marcas`. */
typedef struct { const char *nome; int achou; int32_t marcas; } AchaMembro;

static void membro_acha(void *ctx, const char *nome, int32_t marcas, const char *tipo)
{
    (void)tipo;
    AchaMembro *a = ctx;
    if (strcmp(nome, a->nome) != 0) return;
    a->marcas = a->achou ? membro_junta_marcas(a->marcas, marcas) : marcas;
    a->achou = 1;
}

static int classe_declara_no(PSNode *cls, const char *nome, int32_t *marcas)
{
    AchaMembro a = { nome, 0, 0 };
    membros_declarados(cls, membro_acha, &a);
    if (a.achou && marcas) *marcas = a.marcas;
    return a.achou;
}

/* Um trecho do proto de `u` que é corpo da classe `ci` (ver
 * `PSProto.faixas_classe`). */
static void faixa_classe_poe(C *c, Unidade *u, int32_t ini, int32_t fim, int32_t ci)
{
    if (fim <= ini) return;
    PSProto *p = UP(c, u);
    PSFaixaClasse *nv = realloc(p->faixas_classe, sizeof(PSFaixaClasse) * (size_t)(p->nfaixas_classe + 1));
    if (!nv) { cerro(c, "sem memoria", NULL); return; }
    p->faixas_classe = nv;
    nv[p->nfaixas_classe].ip_ini = ini;
    nv[p->nfaixas_classe].ip_fim = fim;
    nv[p->nfaixas_classe].classe = ci;
    p->nfaixas_classe++;
}

/* Registra um campo tipado na classe (o primeiro de cada nome vale). */
static void classe_tip_add(C *c, PSClassDef *def, const char *nome, const char *tipo)
{
    if (!nome || !tipo) return;
    for (int32_t i = 0; i < def->ntip; i++)
        if (strcmp(def->tip_nomes[i], nome) == 0) return;
    char **nn = realloc(def->tip_nomes, sizeof(char *) * (size_t)(def->ntip + 1));
    if (!nn) { cerro(c, "sem memoria", NULL); return; }
    def->tip_nomes = nn;
    char **nt = realloc(def->tip_tipos, sizeof(char *) * (size_t)(def->ntip + 1));
    if (!nt) { cerro(c, "sem memoria", NULL); return; }
    def->tip_tipos = nt;
    char *cn = strdup(nome), *ct = strdup(tipo);
    if (!cn || !ct) { free(cn); free(ct); cerro(c, "sem memoria", NULL); return; }
    def->tip_nomes[def->ntip] = cn;
    def->tip_tipos[def->ntip] = ct;
    def->ntip++;
}

/* `private <tipo> <nome> = ...` dentro dos métodos: campo tipado também. */
static void varre_campos_decl_tipados(C *c, PSNode *n, PSClassDef *def)
{
    if (!n) return;
    if (n->kind == N_FIELD_DECL && n->texto && n->texto2) classe_tip_add(c, def, n->texto, n->texto2);
    varre_campos_decl_tipados(c, n->a, def);
    varre_campos_decl_tipados(c, n->b, def);
    varre_campos_decl_tipados(c, n->c, def);
    varre_campos_decl_tipados(c, n->e, def);
    for (int32_t i = 0; i < n->lista.n; i++)  varre_campos_decl_tipados(c, n->lista.itens[i], def);
    for (int32_t i = 0; i < n->lista2.n; i++) varre_campos_decl_tipados(c, n->lista2.itens[i], def);
}

static int eh_global_declarada(Unidade *u, const char *nome)
{
    for (int32_t i = 0; i < u->nglobais_decl; i++)
        if (strcmp(u->globais_decl[i], nome) == 0) return 1;
    return 0;
}


/* `range` é redefinível como qualquer nome (`range = 5`, `funct range()`,
 * `from x import range`). A especialização do `for each` só pode acontecer se
 * o programa NÃO liga esse nome em lugar nenhum — senão a VM rodaria o range
 * embutido no lugar do que o usuário escreveu, calada. */
/* O nome que `import m` / `PUSH m` (sem lista de nomes) liga quando não há
 * `as`: o ÚLTIMO segmento do caminho pontuado (`import pacote.modulo` liga
 * `modulo`), ou, entre aspas, o nome do arquivo sem pasta e sem extensão
 * (`import '../x/util.pr'` liga `util`). Um lugar só: o compilador liga com
 * isto e o `liga_o_nome` pergunta com isto. */
static void import_nome_do_arquivo(const PSNode *n, char *out, size_t cap)
{
    out[0] = '\0';
    if (n->i2 == -1) {
        const char *spec = (n->lista.n > 0 && n->lista.itens[0]->texto) ? n->lista.itens[0]->texto : "";
        const char *b = ps_ultimo_sep(spec); b = b ? b + 1 : spec;
        snprintf(out, cap, "%s", b);
        ps_tira_ext(out);
    } else if (n->lista.n > 0 && n->lista.itens[n->lista.n - 1]->texto) {
        snprintf(out, cap, "%s", n->lista.itens[n->lista.n - 1]->texto);
    }
}

/* O módulo de um import, codificado como o OP_IMPORT_MOD espera: `n->i2`
 * pontos de nível relativo seguidos do caminho pontuado (`from .a.b import x`
 * -> ".a.b"), ou, entre aspas (i2 == -1), o marcador \x01 + o literal como
 * foi escrito — o runtime decide se é caminho ou nome de módulo. `encoded`
 * tem 512 bytes. Um lugar só: a compilação do import e a expansão do `*`. */
static void import_modulo_codificado(const PSNode *n, char *encoded)
{
    int el = 0;
    if (n->i2 == -1) {
        const char *spec = (n->lista.n > 0 && n->lista.itens[0]->texto) ? n->lista.itens[0]->texto : "";
        int pl = (int)strlen(spec); if (pl > 500) pl = 500;
        encoded[el++] = '\x01';
        memcpy(encoded + el, spec, (size_t)pl); el += pl;
    } else {
        for (int32_t i = 0; i < n->i2 && el < 500; i++) encoded[el++] = '.';
        for (int32_t i = 0; i < n->lista.n && el < 500; i++) {
            if (i) encoded[el++] = '.';
            const char *pt = n->lista.itens[i]->texto ? n->lista.itens[i]->texto : "";
            int pl = (int)strlen(pt);
            if (el + pl >= 500) pl = 500 - el;
            memcpy(encoded + el, pt, (size_t)pl); el += pl;
        }
    }
    encoded[el] = '\0';
}

void ps_import_modulo_codificado(const PSNode *n, char *encoded)
{
    import_modulo_codificado(n, encoded);
}

static int nome_bate(const PSNode *x, const char *alvo)
{
    return x && x->texto && !strcmp(x->texto, alvo);
}

/* A primeira LEITURA de `alvo` na expressão `n`. Não entra em funct nem
 * lambda (o corpo delas roda depois, e a captura é válida) nem na
 * compreensão que liga o mesmo nome (lá dentro o nome é o dela). */
static PSNode *acha_leitura(C *c, PSNode *n, const char *alvo)
{
    if (!n || fundo_demais(c, n, 1)) return NULL;
    while (n->kind == N_BINARY_OP) {      /* cadeia `a + b + …` em laço (ver liga_o_nome) */
        PSNode *r = acha_leitura(c, n->b, alvo);
        if (r) return r;
        n = n->a;
        if (!n) return NULL;
    }
    if (n->kind == N_FUNCT_DECL || n->kind == N_LAMBDA_EXPR) return NULL;
    if (n->kind == N_LIST_COMP && nome_bate(n, alvo)) return NULL;
    if (n->kind == N_NAME && nome_bate(n, alvo)) return n;
    PSNode *r;
    if ((r = acha_leitura(c, n->a, alvo)) || (r = acha_leitura(c, n->b, alvo))
            || (r = acha_leitura(c, n->c, alvo)) || (r = acha_leitura(c, n->e, alvo))) return r;
    for (int32_t i = 0; i < n->lista.n; i++)  if ((r = acha_leitura(c, n->lista.itens[i], alvo))) return r;
    for (int32_t i = 0; i < n->lista2.n; i++) if ((r = acha_leitura(c, n->lista2.itens[i], alvo))) return r;
    return NULL;
}

/* Toda forma que LIGA um nome entra aqui. A lista antiga via atribuição,
 * declaração e import por lista, e deixava de fora parâmetro (`funct f(range)`),
 * `import json as range`, variável de `catch`, `using ... as`, captura de
 * `match`, alvo de desempacotamento e a variável da compreensão — em todos
 * esses o `for each i in range(2)` rodava o `range` embutido, calado.
 * Sobrar aqui só desliga o atalho; faltar troca o que o programa faz. */
static int liga_o_nome(C *c, PSNode *n, const char *alvo)
{
    if (!n || fundo_demais(c, n, 1)) return 0;
    /* `a + b + c + …` é uma cadeia pelo ->a, um nó por termo: todo caminhante
     * da árvore desce a cadeia em LAÇO (só o ->b de cada nó é recursão). Em
     * recursão pura, uma soma plana de 8000 termos (código gerado) eram 8000
     * quadros, e o freio da pilha disparava num programa correto. */
    while (n->kind == N_BINARY_OP) {
        if (liga_o_nome(c, n->b, alvo)) return 1;
        n = n->a;
        if (!n) return 0;
    }
    switch (n->kind) {
        case N_ASSIGNMENT: case N_VAR_DECL: case N_FOR_EACH_STMT: case N_LIST_COMP:
        case N_ENTITY_DECL: case N_MODEL_DECL: case N_ENUM_DECL:
        case N_USING_STMT: case N_CATCH_CLAUSE:
            if (nome_bate(n, alvo)) return 1;
            break;
        case N_MATCH_PATTERN:
            if (nome_bate(n, alvo) || (n->texto2 && !strcmp(n->texto2, alvo))) return 1;
            break;
        case N_FUNCT_DECL: case N_LAMBDA_EXPR: case N_UNPACK_TARGET:
            /* parâmetros e alvos de desempacotamento são Name na `lista` */
            if (nome_bate(n, alvo)) return 1;
            for (int32_t i = 0; i < n->lista.n; i++) {
                PSNode *x = n->lista.itens[i];
                if (x && x->kind == N_NAME && nome_bate(x, alvo)) return 1;
            }
            break;
        case N_IMPORT_STMT: {
            /* `*` liga os nomes que o módulo exporta (e não o do módulo) */
            if (n->texto3) {
                for (int32_t e = 0; e < c->nestrelas; e++) {
                    if (!mesmo_no(c->estrelas[e].no, n)) continue;
                    for (int32_t k = 0; k < c->estrelas[e].n; k++)
                        if (!strcmp(c->estrelas[e].nomes[k], alvo)) return 1;
                }
                return 0;
            }
            if (n->texto2 && !strcmp(n->texto2, alvo)) return 1;
            for (int32_t i = 0; i < n->lista2.n; i++)
                if (nome_bate(n->lista2.itens[i], alvo)) return 1;
            for (int32_t i = 0; i < n->lista2_alias.n; i++)
                if (nome_bate(n->lista2_alias.itens[i], alvo)) return 1;
            if (n->lista2.n == 0 && n->texto && strcmp(n->texto, "from") != 0) {
                char base[256];
                import_nome_do_arquivo(n, base, sizeof(base));
                if (!strcmp(base, alvo)) return 1;
            }
            return 0;   /* o caminho do módulo não liga nada */
        }
        default: break;
    }
    if (liga_o_nome(c, n->a, alvo) || liga_o_nome(c, n->b, alvo)
        || liga_o_nome(c, n->c, alvo) || liga_o_nome(c, n->e, alvo)) return 1;
    for (int32_t i = 0; i < n->lista.n; i++)
        if (liga_o_nome(c, n->lista.itens[i], alvo)) return 1;
    for (int32_t i = 0; i < n->lista2.n; i++)
        if (liga_o_nome(c, n->lista2.itens[i], alvo)) return 1;
    return 0;
}

/* ── closure: captura de variável de fora ────────────────────────────────
 *
 * A variável capturada mora numa
 * CÉLULA no heap, e tanto quem declara quanto quem captura mexem no valor de
 * dentro dela. Célula em vez de ponteiro pro slot porque a VM move locais —
 * gerador copia o frame pra dentro do objeto, fibra troca o array inteiro —
 * e um ponteiro pra `vm->locals` ficaria pendurado.
 *
 * Quais slots viram célula é decidido ANTES de compilar o corpo, varrendo as
 * functs aninhadas (`marca_celulas`): sem isso o `a = 1` já teria sido
 * emitido como slot cru quando o `funct dentro()` aparecesse depois. */

static int32_t add_upval(C *c, Unidade *u, const char *nome, int32_t em_local, int32_t idx)
{
    for (int32_t i = 0; i < u->nupvals; i++)
        if (strcmp(u->upvals[i].nome, nome) == 0) return i;
    if (u->nupvals + 1 > u->cap_upvals) {
        int32_t novo = u->cap_upvals < 4 ? 4 : u->cap_upvals * 2;
        UpvalC *nu = realloc(u->upvals, sizeof(UpvalC) * (size_t)novo);
        if (!nu) { cerro(c, "sem memoria", NULL); return -1; }
        u->upvals = nu; u->cap_upvals = novo;
    }
    size_t n = strlen(nome);
    char *copia = malloc(n + 1);
    if (!copia) { cerro(c, "sem memoria", NULL); return -1; }
    memcpy(copia, nome, n + 1);
    u->upvals[u->nupvals].nome = copia;
    u->upvals[u->nupvals].em_local = em_local;
    u->upvals[u->nupvals].idx = idx;
    return u->nupvals++;
}

static int32_t slot_existente(Unidade *u, const char *nome);

/* Índice do upvalue `nome` nesta unidade, criando a cadeia de captura pelos
 * pais se preciso. -1 = o nome não vem de nenhuma função de fora. */
static int32_t resolve_upval(C *c, Unidade *u, const char *nome)
{
    if (!u->pai || u->pai->eh_modulo) {
        /* Funct de topo: só a variável de BLOCO do topo que está numa célula
         * do módulo (`celula_do_topo`) é capturada; o resto é global. */
        Unidade *m = u->pai ? u->pai : u->modulo;
        if (!m) return -1;
        for (int32_t i = 0; i < u->nupvals; i++)
            if (strcmp(u->upvals[i].nome, nome) == 0) return i;
        int32_t s = slot_existente(m, nome);
        if (s >= 0 && m->celula[s]) return add_upval(c, u, nome, 1, s);
        return -1;
    }
    for (int32_t i = 0; i < u->nupvals; i++)
        if (strcmp(u->upvals[i].nome, nome) == 0) return i;
    /* do último slot pro primeiro: a célula do bloco aberto onde esta funct
     * foi escrita sombreia a de mesmo nome do nível de cima. A de um bloco
     * que não envolve esta funct não está viva aqui: não é capturada. */
    for (int32_t i = u->pai->nlocais - 1; i >= 0; i--)
        /* `celula` e `locais` têm a mesma capacidade (crescem juntos) */
        if (u->pai->celula[i] && strcmp(u->pai->locais[i], nome) == 0)
            return add_upval(c, u, nome, 1, i);
    int32_t k = resolve_upval(c, u->pai, nome);
    if (k >= 0) return add_upval(c, u, nome, 0, k);
    return -1;
}

/* Junta no vetor os nomes que APARECEM dentro de uma funct aninhada. É
 * conservador de propósito: se o nome bate com um local de fora, aquele slot
 * vira célula mesmo que a aninhada só use um homônimo dela. Marcar a mais
 * custa uma indireção; marcar a menos quebraria a captura. */
static void junta_nomes(C *c, char ***v, int32_t *n, int32_t *cap, const char *nome)
{
    if (!nome || !*nome) return;
    /* A lista do MÓDULO tem índice: ela junta um nome por declaração do
     * arquivo, e a varredura aqui era o N² de um arquivo com milhares de
     * funct. As de funct aninhada são curtas e seguem varridas. */
    IdxNomes *ix = (v == &c->ligados_mod) ? &c->ix_ligados_mod
                 : (v == &c->ligados_mod_raso) ? &c->ix_ligados_mod_raso : NULL;
    if (ix) { if (ix_busca(ix, nome) >= 0) return; }
    else for (int32_t i = 0; i < *n; i++) if (!strcmp((*v)[i], nome)) return;
    if (*n + 1 > *cap) {
        int32_t novo = *cap < 8 ? 8 : *cap * 2;
        char **nv = realloc(*v, sizeof(char *) * (size_t)novo);
        if (!nv) { cerro(c, "sem memoria", NULL); return; }
        *v = nv; *cap = novo;
    }
    size_t ln = strlen(nome);
    char *copia = malloc(ln + 1);
    if (!copia) { cerro(c, "sem memoria", NULL); return; }
    memcpy(copia, nome, ln + 1);
    if (ix && ix_poe(ix, copia, *n) != 0) { free(copia); cerro(c, "sem memoria", NULL); return; }
    (*v)[(*n)++] = copia;
}

/* Os nomes que um `import` liga: o apelido, os nomes do `from`, o do arquivo,
 * ou os que o módulo exporta no `*`. */
static void junta_nomes_do_import(C *c, PSNode *n, char ***v, int32_t *cnt, int32_t *cap)
{
    if (n->texto3) {                 /* `*`: os nomes que o módulo exporta */
        int achou = 0;
        for (int32_t e = 0; e < c->nestrelas; e++) {
            if (!mesmo_no(c->estrelas[e].no, n)) continue;
            achou = 1;
            if (!c->estrelas[e].resolvido || c->estrelas[e].incompleto) c->nomes_incertos = 1;
            for (int32_t k = 0; k < c->estrelas[e].n; k++)
                junta_nomes(c, v, cnt, cap, c->estrelas[e].nomes[k]);
        }
        if (!achou) c->nomes_incertos = 1;
        return;
    }
    char enc[512];
    if (!(n->i2 > 0 && n->lista.n == 0)) {
        import_modulo_codificado(n, enc);
        if (strcmp(enc, "jinker") == 0) c->usa_jinker = 1;
    }
    junta_nomes(c, v, cnt, cap, n->texto2);
    for (int32_t i = 0; i < n->lista2.n; i++) {
        const char *apelido = (i < n->lista2_alias.n && n->lista2_alias.itens[i])
                            ? n->lista2_alias.itens[i]->texto : n->lista2.itens[i]->texto;
        junta_nomes(c, v, cnt, cap, apelido);
    }
    if (n->lista2.n == 0 && !n->texto2 && n->texto && strcmp(n->texto, "from") != 0) {
        char base[256];
        import_nome_do_arquivo(n, base, sizeof(base));
        junta_nomes(c, v, cnt, cap, base);
    }
}

/* Os alvos de um desempacotamento (`a, (b, c) = ...`), em `*v`. */
static void junta_alvos(C *c, PSNode *alvo, char ***v, int32_t *cnt, int32_t *cap)
{
    if (!alvo) return;
    for (int32_t i = 0; i < alvo->lista.n; i++) {
        PSNode *x = alvo->lista.itens[i];
        if (!x) continue;
        if (x->kind == N_UNPACK_TARGET) junta_alvos(c, x, v, cnt, cap);
        else if (x->kind == N_NAME) junta_nomes(c, v, cnt, cap, x->texto);
    }
}

/* Os nomes que a instrução `s` liga NO BLOCO em que está escrita — não nos
 * blocos de dentro, que têm a entrada deles: a declaração, a funct, a classe,
 * o model, o enum e o import vão em `decl`; a atribuição simples e o
 * desempacotamento sem tipo vão em `atrib`, porque só criam variável quando o
 * nome ainda não existe (senão gravam no de fora, doc 4.6.2). O `using` e o
 * bloco que envolve a declaração decorada não abrem escopo: o que ligam é do
 * bloco de fora. */
static void liga_direto_stmt(C *c, PSNode *s, char ***decl, int32_t *nd, int32_t *cd,
                             char ***atrib, int32_t *na, int32_t *ca)
{
    if (!s || fundo_demais(c, s, 1)) return;
    switch (s->kind) {
        case N_VAR_DECL: case N_FUNCT_DECL: case N_ENTITY_DECL:
        case N_MODEL_DECL: case N_ENUM_DECL:
            junta_nomes(c, decl, nd, cd, s->texto);
            break;
        case N_ASSIGNMENT:
            junta_nomes(c, atrib, na, ca, s->texto);
            break;
        case N_UNPACK_ASSIGNMENT:
            junta_alvos(c, s->a, atrib, na, ca);
            break;
        case N_IMPORT_STMT:
            junta_nomes_do_import(c, s, decl, nd, cd);
            break;
        case N_USING_STMT:
            junta_nomes(c, decl, nd, cd, s->texto ? s->texto : "_");
            if (s->b)
                for (int32_t i = 0; i < s->b->lista.n; i++)
                    liga_direto_stmt(c, s->b->lista.itens[i], decl, nd, cd, atrib, na, ca);
            break;
        case N_DECORATOR_STMT:
            if (s->b && s->b->kind == N_BLOCK)
                for (int32_t i = 0; i < s->b->lista.n; i++)
                    liga_direto_stmt(c, s->b->lista.itens[i], decl, nd, cd, atrib, na, ca);
            break;
        default:
            break;
    }
}

static void liga_direto(C *c, PSNode *b, char ***decl, int32_t *nd, int32_t *cd,
                        char ***atrib, int32_t *na, int32_t *ca)
{
    if (!b) return;
    for (int32_t i = 0; i < b->lista.n; i++)
        liga_direto_stmt(c, b->lista.itens[i], decl, nd, cd, atrib, na, ca);
}

static void solta_nomes(char **v, int32_t n)
{
    for (int32_t i = 0; i < n; i++) free(v[i]);
    free(v);
}

static void varre_nomes(C *c, PSNode *n, char ***v, int32_t *cnt, int32_t *cap)
{
    if (!n || fundo_demais(c, n, 1)) return;
    while (n->kind == N_BINARY_OP) {      /* cadeia `a + b + …` em laço (ver liga_o_nome) */
        varre_nomes(c, n->b, v, cnt, cap);
        n = n->a;
        if (!n) return;
    }
    switch (n->kind) {
        case N_NAME: case N_ASSIGNMENT: case N_VAR_DECL: case N_FOR_EACH_STMT:
            junta_nomes(c, v, cnt, cap, n->texto);
            break;
        case N_FUNCT_DECL:
            junta_nomes(c, v, cnt, cap, n->texto);
            break;
        default: break;
    }
    varre_nomes(c, n->a, v, cnt, cap);
    varre_nomes(c, n->b, v, cnt, cap);
    varre_nomes(c, n->c, v, cnt, cap);
    varre_nomes(c, n->e, v, cnt, cap);
    for (int32_t i = 0; i < n->lista.n; i++)  varre_nomes(c, n->lista.itens[i], v, cnt, cap);
    for (int32_t i = 0; i < n->lista2.n; i++) varre_nomes(c, n->lista2.itens[i], v, cnt, cap);
}

/* Procura functs aninhadas dentro de `n` (sem entrar nelas duas vezes) e
 * coleta os nomes que elas citam.
 *
 * A LAMBDA conta igual à nomeada. Sem ela aqui, nome citado dentro de
 * `funct(b) { ... }` não entrava na conta e o slot de fora nunca virava
 * célula: `funct soma_de(a) { return funct(b) { return a + b } }` dava
 * "NameError: name 'a' is not defined" — a captura só funcionava com funct
 * nomeada. */
static void acha_aninhadas(C *c, PSNode *n, char ***v, int32_t *cnt, int32_t *cap)
{
    if (!n || fundo_demais(c, n, 1)) return;
    while (n->kind == N_BINARY_OP) {      /* cadeia `a + b + …` em laço (ver liga_o_nome) */
        acha_aninhadas(c, n->b, v, cnt, cap);
        n = n->a;
        if (!n) return;
    }
    if (n->kind == N_FUNCT_DECL || n->kind == N_LAMBDA_EXPR) { varre_nomes(c, n, v, cnt, cap); return; }
    /* O método de uma classe declarada aqui cita a própria classe sem
     * escrever o nome: o campo `static` solto é `Classe.x` (carrega_estatico).
     * E o valor padrão de campo de instância roda no proto de campos, que é
     * aninhado como o método (`sintetiza_campos`). */
    if (n->kind == N_ENTITY_DECL) {
        junta_nomes(c, v, cnt, cap, n->texto);
        for (int32_t i = 0; i < n->lista2_alias.n; i++) {
            PSNode *f = n->lista2_alias.itens[i];
            if (f && !f->is_static && f->a) varre_nomes(c, f->a, v, cnt, cap);
        }
    }
    acha_aninhadas(c, n->a, v, cnt, cap);
    acha_aninhadas(c, n->b, v, cnt, cap);
    acha_aninhadas(c, n->c, v, cnt, cap);
    acha_aninhadas(c, n->e, v, cnt, cap);
    for (int32_t i = 0; i < n->lista.n; i++)  acha_aninhadas(c, n->lista.itens[i], v, cnt, cap);
    for (int32_t i = 0; i < n->lista2.n; i++) acha_aninhadas(c, n->lista2.itens[i], v, cnt, cap);
}

static int tp_na_lista(char **v, int32_t n, const char *nome);
static int32_t celula_ativa_do_topo(Unidade *u, const char *nome);
static int nome_ja_existe(Unidade *u, const char *nome);

/* O módulo em que a funct `u` foi escrita: o da funct de topo da cadeia. */
static Unidade *modulo_de(Unidade *u)
{
    Unidade *k = u;
    while (k->pai && !k->pai->eh_modulo) k = k->pai;
    return (k->pai && k->pai->eh_modulo) ? k->pai : k->modulo;
}

/* A escrita de `nome` aqui iria pra variável de FORA desta funct: a célula
 * viva de uma funct que a envolve, ou a célula de bloco do topo — o que o
 * `resolve_upval` acharia, sem criar o upvalue. */
static int vem_de_fora(Unidade *u, const char *nome)
{
    for (int32_t i = 0; i < u->nupvals; i++)
        if (!strcmp(u->upvals[i].nome, nome)) return 1;
    for (Unidade *q = u->pai; q && !q->eh_modulo; q = q->pai)
        for (int32_t i = q->nlocais - 1; i >= 0; i--)
            if (q->celula[i] && !strcmp(q->locais[i], nome)) return 1;
    Unidade *m = modulo_de(u);
    return m && celula_ativa_do_topo(m, nome) >= 0;
}

/* A atribuição simples (`x = v`, `x += v`) a `nome` aqui grava FORA desta
 * funct (doc 4.6.2)? Na variável viva de uma funct de fora (`vem_de_fora`) ou
 * na global que se enxerga daqui — a do arquivo fora de bloco, ou a viva no
 * ponto em que a funct foi escrita (o STORE_NAME grava nela). O nome que só
 * existe num bloco que não envolve a funct não conta: daqui ele não existe. */
static int atrib_vai_pra_fora(C *c, Unidade *u, const char *nome)
{
    if (vem_de_fora(u, nome)) return 1;
    if (ix_busca(&c->ix_ligados_mod_raso, nome) >= 0) return 1;
    Unidade *m = modulo_de(u);
    return m && nome_ja_existe(m, nome);
}

/* Reserva o slot e emite a célula das variáveis capturadas: as que ESTA função
 * liga no nível de cima do corpo (ou os parâmetros) E que alguma funct
 * aninhada cita. A interseção é o que evita transformar uma GLOBAL lida lá
 * dentro em célula vazia. A variável que nasce num bloco do corpo é do bloco:
 * a célula dela nasce na entrada dele (`bloco_abre`), e a funct escrita fora
 * do bloco não a enxerga. Roda depois do prólogo dos defaults (que testa o
 * slot cru com JUMP_IF_SET) e antes do corpo — os acessos precisam já saber
 * que o slot virou célula. */
static void marca_celulas(C *c, Unidade *u, PSNode *params, PSNode *corpo)
{
    char **usados = NULL; int32_t nu = 0, cu = 0;
    char **ligados = NULL; int32_t nl = 0, cl = 0;
    char **atrib = NULL; int32_t na = 0, ca = 0;
    acha_aninhadas(c, corpo, &usados, &nu, &cu);
    liga_direto(c, corpo, &ligados, &nl, &cl, &atrib, &na, &ca);
    int32_t npar = params ? params->lista.n : 0;
    for (int32_t i = 0; i < npar; i++)
        junta_nomes(c, &ligados, &nl, &cl, params->lista.itens[i]->texto);
    for (int32_t i = 0; i < nu && !CFALHOU(c); i++) {
        int liga = tp_na_lista(ligados, nl, usados[i]);
        /* Só atribuído aqui (`total += 1`) e ligado fora: a escrita vai pro
         * de fora (STORE_UPVAL, STORE_NAME), e a funct aninhada lê o de fora
         * também. A célula local desviava as duas pra uma variável nova: a
         * global ficava 0 e a funct de dentro dava NameError. */
        if (!liga && tp_na_lista(atrib, na, usados[i]) && !atrib_vai_pra_fora(c, u, usados[i])) liga = 1;
        if (!liga) continue;
        if (eh_global_declarada(u, usados[i])) continue;   /* `global x` manda */
        int32_t slot = idx_local(c, u, usados[i]);
        if (slot < 0 || u->celula[slot]) continue;
        u->celula[slot] = 1;
        /* o parâmetro já tem valor (o da chamada); o resto nasce no corpo */
        u->celula_virgem[slot] = slot >= npar;
        emite(c, u, OP_MAKE_CELL, slot);
    }
    /* O que esta funct DECLARA no nível de cima (`funct g`, `class`, `import`)
     * é dela, mesmo com o nome de uma variável viva de uma funct de fora: o
     * slot nasce aqui, e a gravação da declaração não sobe pro upvalue (a
     * declaração tipada já reserva o slot dela). */
    for (int32_t i = 0; i < nl && !CFALHOU(c); i++)
        if (!eh_global_declarada(u, ligados[i]) && slot_existente(u, ligados[i]) < 0
                && vem_de_fora(u, ligados[i]))
            idx_local(c, u, ligados[i]);
    solta_nomes(usados, nu);
    solta_nomes(ligados, nl);
    solta_nomes(atrib, na);
}

/* As functs e lambdas escritas em `n` (sem entrar nelas), em `*v`. */
static void colhe_lambdas(C *c, PSNode *n, PSNode ***v, int32_t *k, int32_t *cap)
{
    if (!n || fundo_demais(c, n, 1)) return;
    while (n->kind == N_BINARY_OP) {      /* cadeia `a + b + …` em laço (ver liga_o_nome) */
        colhe_lambdas(c, n->b, v, k, cap);
        n = n->a;
        if (!n) return;
    }
    if (n->kind == N_FUNCT_DECL || n->kind == N_LAMBDA_EXPR) {
        if (*k + 1 > *cap) {
            int32_t novo = *cap < 4 ? 4 : *cap * 2;
            PSNode **nv = realloc(*v, sizeof(PSNode *) * (size_t)novo);
            if (!nv) { cerro(c, "sem memoria", NULL); return; }
            *v = nv; *cap = novo;
        }
        (*v)[(*k)++] = n;
        return;
    }
    colhe_lambdas(c, n->a, v, k, cap);
    colhe_lambdas(c, n->b, v, k, cap);
    colhe_lambdas(c, n->c, v, k, cap);
    colhe_lambdas(c, n->e, v, k, cap);
    for (int32_t i = 0; i < n->lista.n; i++)  colhe_lambdas(c, n->lista.itens[i], v, k, cap);
    for (int32_t i = 0; i < n->lista2.n; i++) colhe_lambdas(c, n->lista2.itens[i], v, k, cap);
}

/* O slot do nome, se esta unidade já tem um (sem criar). */
static int32_t slot_existente(Unidade *u, const char *nome)
{
    for (int32_t i = u->nlocais - 1; i >= 0; i--)
        if (u->locais[i] && !strcmp(u->locais[i], nome)) return i;
    return -1;
}

/* ── células de bloco ─────────────────────────────────────────────────────
 * A variável que NASCE num bloco (declarada nele, ou a do laço) e que uma
 * funct escrita no bloco cita mora numa CÉLULA DO BLOCO: nova a cada entrada
 * — no laço, a cada volta (doc 4.6.3) — e só dele. A funct escrita no bloco
 * a captura; a escrita fora dele não a enxerga (decisão 4 do Plano 6, escopo
 * de verdade, doc 4.6.1). Nome que já existe quando o bloco abre não nasce
 * nele: a escrita grava no de fora (doc 4.6.2).
 *
 * Na funct, a célula é um slot NOVO, depois da marca do bloco: a busca por
 * nome vai do último slot pro primeiro, então ele sombreia o de mesmo nome do
 * nível de cima, e o fim do bloco o apaga e devolve como a qualquer local do
 * bloco (`escopo_fecha`, o `break`, o `continue`, o alvo do handler).
 *
 * No topo do arquivo, onde o resto é global, é um slot local do módulo
 * enquanto o bloco dura. Global, o fim do bloco a apagava (CLEAR_GLOBAL), e a
 * funct de dentro dava NameError quando rodava depois; e as voltas do laço
 * dividiam uma só. Leitura e escrita no topo vão pela célula
 * (`carrega_nome_sem_estatico`, `guarda_nome_modo_no`), e a funct de topo a
 * captura pelo `modulo` (`resolve_upval`). A VM não muda: o frame do módulo
 * reserva os locais do proto, como o de qualquer funct. */
static int32_t celula_do_topo(C *c, Unidade *u, const char *nome)
{
    /* um slot que já saiu ou um novo — acima do de mesmo nome que está vivo
     * (o de um bloco de fora), que este sombreia enquanto dura */
    int32_t piso = slot_existente(u, nome);
    int32_t s = -1;
    for (int32_t i = piso + 1; i < u->nlocais && s < 0; i++)
        if (u->locais[i] && !u->locais[i][0]) {      /* slot de célula que já saiu */
            size_t n = strlen(nome);
            char *copia = malloc(n + 1);
            if (!copia) { cerro(c, "sem memoria", NULL); return -1; }
            memcpy(copia, nome, n + 1);
            free(u->locais[i]);
            u->locais[i] = copia;
            u->tipo_decl[i] = 0;
            memset(&u->sim[i], 0, sizeof(SimInfo));
            s = i;
        }
    if (s < 0) s = slot_novo(c, u, nome);
    if (s < 0) return -1;
    u->celula[s] = 1;
    u->celula_virgem[s] = 1;
    /* não "certo": a leitura vai pelo CELL_GET_NAME, que com a célula ainda
     * vazia (lida antes da declaração no bloco) cai na global ou no
     * NameError, como antes — o CELL_GET empurraria o vazio */
    u->certo[s] = 0;
    return s;
}

/* Funct: a célula de bloco num slot novo. */
static int32_t celula_de_bloco(C *c, Unidade *u, const char *nome)
{
    int32_t s = slot_novo(c, u, nome);
    if (s < 0) return -1;
    u->celula[s] = 1;
    u->celula_virgem[s] = 1;
    return s;
}

/* O fim do bloco: a célula sai do slot (CLEAR_LOCAL), o nome volta a ser
 * global (ou a não existir) e o slot fica livre. */
static void solta_celula_do_topo(C *c, Unidade *u, const char *nome)
{
    int32_t s = slot_existente(u, nome);
    if (s < 0 || !u->celula[s]) return;
    emite(c, u, OP_CLEAR_LOCAL, s);
    char *vazio = malloc(1);
    if (!vazio) { cerro(c, "sem memoria", NULL); return; }
    vazio[0] = '\0';
    free(u->locais[s]);
    u->locais[s] = vazio;
    u->celula[s] = 0;
    u->certo[s] = 0;
}

/* O nome é célula de bloco do topo agora? */
static int32_t celula_ativa_do_topo(Unidade *u, const char *nome)
{
    if (!u->eh_modulo) return -1;
    int32_t s = slot_existente(u, nome);
    return (s >= 0 && u->celula[s]) ? s : -1;
}

static void celula_nova_se_capturado(C *c, Unidade *u, const char *nome);
static int nome_ja_existe(Unidade *u, const char *nome);

/* `nome` nasce neste bloco? Só se ainda não existe aqui — o que já existe é a
 * mesma variável (a escrita grava nela, doc 4.6.2) — e, na atribuição simples
 * (`atrib`), se ela não grava fora desta funct (`atrib_vai_pra_fora`). A
 * declaração (`int x`, `funct`, `class`, `import`) cria a variável desta
 * funct mesmo com o nome de uma de fora, como no nível de cima dela.
 * Nascendo aqui, e citado por uma funct escrita no bloco (`cit`), vira célula
 * de bloco. Sem citação, ganha slot próprio só quando já há um de mesmo nome
 * ainda vazio — a célula de um nome do nível de cima que nasce DEPOIS deste
 * bloco —, pra a variável do bloco não cair nela. O que vira célula entra em
 * `ativ` (a célula nova da entrada, e o fechamento no topo). */
static void ativa_bloco(C *c, Unidade *u, const char *nome, int atrib, char **cit, int32_t nc,
                        char ***ativ, int32_t *na, int32_t *ca)
{
    if (!nome || !*nome || tp_na_lista(*ativ, *na, nome)) return;
    if (nome_ja_existe(u, nome) || eh_global_declarada(u, nome)) return;
    if (!u->eh_modulo && atrib && atrib_vai_pra_fora(c, u, nome)) return;
    int citado = tp_na_lista(cit, nc, nome);
    if (u->eh_modulo) {
        if (!citado && celula_ativa_do_topo(u, nome) < 0) return;
        if (celula_do_topo(c, u, nome) < 0) return;
    } else {
        if (!citado) {
            /* a declaração com o nome de uma variável viva de fora também:
             * sem o slot, a gravação dela iria pro upvalue */
            if (slot_existente(u, nome) >= 0 || (!atrib && vem_de_fora(u, nome))) slot_novo(c, u, nome);
            return;
        }
        if (celula_de_bloco(c, u, nome) < 0) return;
    }
    junta_nomes(c, ativ, na, ca, nome);
}

/* Entrada de um bloco (`bloco_stmts`): as variáveis que nascem DIRETAMENTE
 * nele (`liga_direto`; as de um bloco de dentro têm a entrada dele). A célula
 * nova sai aqui, a cada entrada — no corpo do laço, a cada volta. */
static void bloco_abre(C *c, Unidade *u, PSNode *b, char ***ativ, int32_t *na, int32_t *ca)
{
    if (!b) return;
    char **cit = NULL; int32_t nc = 0, cc = 0;
    char **decl = NULL; int32_t nd = 0, cd = 0;
    char **atrib = NULL; int32_t nt = 0, ct = 0;
    acha_aninhadas(c, b, &cit, &nc, &cc);
    liga_direto(c, b, &decl, &nd, &cd, &atrib, &nt, &ct);
    for (int32_t i = 0; i < nd; i++) ativa_bloco(c, u, decl[i], 0, cit, nc, ativ, na, ca);
    for (int32_t i = 0; i < nt; i++) ativa_bloco(c, u, atrib[i], 1, cit, nc, ativ, na, ca);
    for (int32_t i = 0; i < *na; i++) celula_nova_se_capturado(c, u, (*ativ)[i]);
    solta_nomes(cit, nc);
    solta_nomes(decl, nd);
    solta_nomes(atrib, nt);
}

/* `nome` aparece numa funct escrita em `corpo`? */
static int citado_em(C *c, PSNode *corpo, const char *nome)
{
    if (!nome || !*nome) return 0;
    char **cit = NULL; int32_t nc = 0, cc = 0;
    acha_aninhadas(c, corpo, &cit, &nc, &cc);
    int r = tp_na_lista(cit, nc, nome);
    solta_nomes(cit, nc);
    return r;
}

static SimInfo *tp_sim_upval(Unidade *u, const char *nome);
static int tipo_de_upval(Unidade *u, const char *nome);

/* O slot `s`, que a variável do laço ganhou só dela, herda o tipo DECLARADO
 * do nome de fora (`int i` antes do laço, o parâmetro tipado, a variável
 * tipada da funct que envolve): o laço confere o tipo como o laço que
 * sombreia (`tp_sombra_poe`) — `for each i in ["a"]` com `int i` lá fora é
 * erro com ou sem closure no corpo. `fora` = o slot de fora nesta funct, ou
 * -1 (o da funct que envolve, se houver). Sem tipo escrito, nada passa. */
static void laco_herda_tipo(Unidade *u, int32_t s, int32_t fora)
{
    const SimInfo *t;
    int cod;
    if (fora >= 0) { t = &u->sim[fora]; cod = u->tipo_decl[fora]; }
    else { t = tp_sim_upval(u, u->locais[s]); cod = tipo_de_upval(u, u->locais[s]); }
    if (!t || t->estado != 2) return;
    u->sim[s] = *t;
    u->tipo_decl[s] = (unsigned char)cod;
}

/* A variável do laço (`for each` e compreensão) é do laço: na funct, que uma
 * funct do corpo (`corpo`, `corpo2`) a cite ou que o nome venha de uma funct
 * de fora (`propria`), ela ganha slot próprio — célula, se citada, nova a
 * cada volta (`celulas_novas_da_volta`) — e a escrita não vai pra variável de
 * fora. No topo, a citada vira célula do laço e sombreia a global. Os alvos
 * do desempacotamento seguem a regra da atribuição (`ativa_bloco`): o que já
 * existe, aqui ou fora, é o de fora. */
static void laco_abre(C *c, Unidade *u, const char *var, int propria, PSNode *alvo,
                      PSNode *corpo, PSNode *corpo2, char ***ativ, int32_t *na, int32_t *ca)
{
    char **cit = NULL; int32_t nc = 0, cc = 0;
    acha_aninhadas(c, corpo, &cit, &nc, &cc);
    acha_aninhadas(c, corpo2, &cit, &nc, &cc);
    if (var && *var && !tp_na_lista(*ativ, *na, var)) {
        int32_t fora = u->eh_modulo ? -1 : slot_existente(u, var);
        int32_t s = -1;
        if (tp_na_lista(cit, nc, var)) {
            s = u->eh_modulo ? celula_do_topo(c, u, var) : celula_de_bloco(c, u, var);
            if (s >= 0) junta_nomes(c, ativ, na, ca, var);
        } else if (propria && !u->eh_modulo) s = slot_novo(c, u, var);
        if (s >= 0 && !u->eh_modulo) laco_herda_tipo(u, s, fora);
    }
    if (alvo) {
        char **alv = NULL; int32_t nv = 0, cv = 0;
        junta_alvos(c, alvo, &alv, &nv, &cv);
        for (int32_t i = 0; i < nv; i++) ativa_bloco(c, u, alv[i], 1, cit, nc, ativ, na, ca);
        solta_nomes(alv, nv);
    }
    solta_nomes(cit, nc);
}

/* O fim do bloco ou do laço: no topo, solta as células que a abertura
 * ativou; na funct, o slot delas já saiu com a marca do bloco
 * (`escopo_trunca`), e só a lista é solta. */
static void celulas_fecha(C *c, Unidade *u, char **ativ, int32_t na)
{
    for (int32_t i = 0; i < na; i++) {
        if (u->eh_modulo) solta_celula_do_topo(c, u, ativ[i]);
        free(ativ[i]);
    }
    free(ativ);
}

/* Célula nova no slot do nome capturado: a de antes fica com quem a
 * capturou (CLEAR_LOCAL tira a célula velha do slot; MAKE_CELL embrulha o
 * slot vazio). */
static void celula_nova_se_capturado(C *c, Unidade *u, const char *nome)
{
    if (!nome || !*nome) return;
    int32_t s = slot_existente(u, nome);
    if (s < 0 || !u->celula[s]) return;
    emite(c, u, OP_CLEAR_LOCAL, s);
    emite(c, u, OP_MAKE_CELL, s);
}

/* Closure criada numa volta do laço guarda a variável DAQUELA volta
 * (decisão 4 do Plano 6; doc 4.6.3: o corpo do laço é um escopo por volta).
 * A variável capturada mora numa célula, e a célula era uma só pro laço
 * inteiro: as closures de três voltas viam o último valor (3 3 3). Na
 * ENTRADA de cada volta, as células do laço (`ativ`, de `laco_abre`: a
 * variável do laço e os alvos que nascem nele) são trocadas por novas, como o
 * `let` do laço em JavaScript; as que o corpo declara ganham a delas na
 * entrada do corpo (`bloco_abre`), que é a entrada de cada volta. Variável
 * declarada FORA do laço segue uma só (as closures veem o último valor dela,
 * que é o certo). */
static void celulas_novas_da_volta(C *c, Unidade *u, char **ativ, int32_t na)
{
    for (int32_t i = 0; i < na; i++) celula_nova_se_capturado(c, u, ativ[i]);
}

static void carrega_nome(C *c, Unidade *u, const char *nome);
static void guarda_nome_modo(C *c, Unidade *u, const char *nome, int certa);

/* A sombra do laço (`for each i` e a compreensão, com `i` que já existe):
 * guarda o de fora num nome escondido e devolve na saída. Variável
 * capturada (célula) guarda a CÉLULA, não o valor: as voltas ganham células
 * novas (`celulas_novas_da_volta`), e a de fora volta inteira na saída —
 * quem a capturou continua vendo as escritas de depois. Devolvendo só o
 * valor, ele caía na célula da última volta, a da última closure. */
static int sombra_e_celula(Unidade *u, const char *var)
{
    int32_t s = u->eh_modulo ? -1 : slot_existente(u, var);
    return s >= 0 && u->celula[s];
}

static void sombra_guarda(C *c, Unidade *u, const char *var, const char *salvo)
{
    if (sombra_e_celula(u, var)) {
        emite(c, u, OP_LOAD_LOCAL, slot_existente(u, var));
        int32_t h = idx_local(c, u, salvo);
        if (h >= 0) emite(c, u, OP_STORE_LOCAL, h);
        return;
    }
    carrega_nome(c, u, var);
    guarda_nome_modo(c, u, salvo, 1);
}

/* O par do `sombra_guarda`: deixa o de fora na pilha (célula crua ou valor),
 * e `sombra_devolve_fim` grava. Entre os dois, o tipo de fora volta
 * (`tp_sombra_devolve`). */
static void sombra_devolve_carrega(C *c, Unidade *u, const char *var, const char *salvo)
{
    if (sombra_e_celula(u, var)) emite(c, u, OP_LOAD_LOCAL, slot_existente(u, salvo));
    else carrega_nome(c, u, salvo);
}

static void sombra_devolve_fim(C *c, Unidade *u, const char *var)
{
    if (sombra_e_celula(u, var)) emite(c, u, OP_STORE_LOCAL, slot_existente(u, var));
    else guarda_nome_modo(c, u, var, 1);
}

/* MAKE_CLOSURE só quando a funct realmente captura algo: sem upvalue ela
 * continua sendo o valor barato de sempre (só o índice do proto). */
static void emite_funcao(C *c, Unidade *u, int32_t proto)
{
    emite(c, u, c->out->protos[proto].nupvals > 0 ? OP_MAKE_CLOSURE : OP_MAKE_FUNCTION, proto);
}

static void expr(C *c, Unidade *u, PSNode *n);
static void carrega_nome(C *c, Unidade *u, const char *nome);

/* O nome é uma variável LOCAL desta unidade? Local ganha do campo `static`:
 * um parâmetro chamado `mapp` dentro de um método não pode virar `App.mapp`. */
static int nome_e_local(Unidade *u, const char *nome)
{
    for (int32_t i = 0; i < u->nlocais; i++)
        if (strcmp(u->locais[i], nome) == 0) return 1;
    return 0;
}

static PSNode *tp_tipo_arq(C *c, const char *nome);
static int tp_metodo_estatico(C *c, const char *classe, PSNode *met);
static int tp_metodo_decorado(C *c, const char *classe, PSNode *met);

/* O membro `static` (campo ou método) que o nome solto alcança de dentro da
 * classe `e` — dela ou de um pai conhecido no arquivo. `dono` recebe a classe
 * que o declara. O que `App.x` alcança de fora, `x` alcança de dentro: só o
 * campo entrava, e um `s()` de método static passava no `--check` e dava
 * NameError rodando — "não faz sentido não expor algo na classe só porque
 * é static". */
static PSNode *estatico_em(C *c, PSNode *e, const char *nome, int prof, PSNode **dono)
{
    if (!e || e->kind != N_ENTITY_DECL || prof > 32) return NULL;
    for (int32_t i = 0; i < e->lista2_alias.n; i++) {
        PSNode *f = e->lista2_alias.itens[i];
        if (f && f->kind == N_ENTITY_FIELD && f->is_static && f->texto
                && strcmp(f->texto, nome) == 0) { if (dono) *dono = e; return f; }
    }
    for (int32_t i = 0; i < e->lista.n; i++) {
        PSNode *m = e->lista.itens[i];
        if (m && m->kind == N_FUNCT_DECL && m->texto && strcmp(m->texto, nome) == 0
                && e->texto && tp_metodo_estatico(c, e->texto, m)) { if (dono) *dono = e; return m; }
    }
    for (int32_t i = 0; i < e->lista2.n; i++) {
        PSNode *p = e->lista2.itens[i];
        PSNode *r = (p && p->texto) ? estatico_em(c, tp_tipo_arq(c, p->texto), nome, prof + 1, dono) : NULL;
        if (r) return r;
    }
    return NULL;
}

static PSNode *estatico_da_classe(C *c, const char *nome, PSNode **dono)
{
    if (dono) *dono = NULL;
    if (!c->entity_no || !nome) return NULL;
    return estatico_em(c, c->entity_no, nome, 0, dono);
}

/* `nome` é um membro `static` da Entity que está sendo compilada (ou de um pai)? */
static int campo_estatico_da_classe(C *c, const char *nome)
{
    return estatico_da_classe(c, nome, NULL) != NULL;
}

/* `mapp` dentro da classe App -> `App.mapp` (a classe é um global). */
static void carrega_nome_sem_estatico(C *c, Unidade *u, const char *nome);
static void tp_confere_privado(C *c, PSNode *alvo, const char *nome, int exato, PSNode *onde);

static void carrega_estatico(C *c, Unidade *u, const char *nome)
{
    const char *cls = c->entity_no->texto ? c->entity_no->texto : "";
    /* o nome solto é `Classe.x`: o private do pai alcançado por ele é o
     * mesmo acesso negado, antes de rodar (o erro sai no nome) */
    tp_confere_privado(c, c->entity_no, nome, 1, NULL);
    /* Classe declarada dentro de funct: o nome dela é local de lá, e o
     * método (closure da funct) chega nela pela captura. Com LOAD_GLOBAL o
     * `n` solto dava "NameError: name 'Local' is not defined". */
    if (c->corpo && c->corpo->no == c->entity_no && c->corpo->local)
        carrega_nome_sem_estatico(c, u, cls);
    else
        emite(c, u, OP_LOAD_GLOBAL, idx_global(c, cls));
    emite(c, u, OP_GET_MEMBER, idx_const(c, u, K_STR, 0, 0, nome, (int32_t)strlen(nome)));
}

/* Os argumentos de uma chamada e a chamada em si, com o chamável JÁ na
 * pilha. É UMA função pros três lugares que chamam — `f(...)`, o decorador
 * `@d(...)` e `base(...)` por nome — porque eram três cópias da mesma regra,
 * e a regra cresceu: espalhamento.
 *
 * Sem estrela: `OP_CALL n` (só posicionais) ou `OP_CALL_KW n` com a tupla
 * de nomes por cima. Com `*x`/`**d` em algum argumento, os posicionais viram
 * UMA lista e os nomeados UM dict, montados NA ORDEM ESCRITA (`f(1, *a, 2)`
 * mantém o 2 depois dos itens de `a`; `f(k=1, **d)` deixa o `d` ganhar de
 * `k`, como o último nomeado sempre ganha), e o `OP_CALL_EX` espalha os dois
 * no alvo. A única ordem recusada é a que já era: posicional (ou `*x`)
 * depois de nomeado (ou `**d`). */
static void emite_args_e_chama(C *c, Unidade *u, PSNode *no, PSNodeVec *args)
{
    /* Chamada de SAÍDA numa funct tipada, com argumento que só se sabe
     * rodando (ver `tp_confere_saida`): a conferência vai logo depois de cada
     * um. Consumido aqui — os argumentos têm as chamadas deles. */
    const char *saida_t = c->saida_tipo;
    int saida_i = c->saida_indice;
    char saida_rot[300];
    if (saida_t) snprintf(saida_rot, sizeof(saida_rot), "%s", c->saida_rotulo);
    c->saida_tipo = NULL;
    int32_t n = args->n, nkw = 0, estrelas = 0;
    int viu_nome = 0;
    for (int32_t i = 0; i < n; i++) {
        PSNode *a = args->itens[i];
        int nomeado = (a->texto != NULL || a->i2 == 2);
        if (a->i2) estrelas++;
        if (a->texto) nkw++;
        if (nomeado) viu_nome = 1;
        else if (viu_nome) {
            cerro_sx(c, no, a->i2
                     ? "argumento `*x` depois de nomeado: mova-o pra antes dos nomeados"
                     : "argumento posicional depois de nomeado");
            return;
        }
    }
    if (estrelas == 0) {
        for (int32_t i = 0; i < n; i++) {
            expr(c, u, args->itens[i]->a);
            if (saida_t && !args->itens[i]->texto && (saida_i == -2 || saida_i == i)
                    && tp_aceita(c, saida_t, tp_de(c, u, args->itens[i]->a), args->itens[i]->a, 1) == T_TALVEZ)
                tp_emite_confere(c, u, saida_rot, saida_t, 1);
        }
        if (nkw == 0) { emite(c, u, OP_CALL, n); return; }
        /* nomes dos kwargs entram como uma tupla de constantes */
        for (int32_t i = n - nkw; i < n; i++) {
            const char *nm = args->itens[i]->texto;
            emite(c, u, OP_LOAD_CONST,
                  idx_const(c, u, K_STR, 0, 0, nm, (int32_t)strlen(nm)));
        }
        emite(c, u, OP_BUILD_TUPLE, nkw);
        emite(c, u, OP_CALL_KW, n);
        return;
    }
    /* Posicionais: os primeiros sem estrela numa BUILD_LIST só; daí em
     * diante cada `*x` estende e cada posicional solto entra como lista de
     * um item — é o que preserva a ordem escrita. */
    int32_t i = 0, k = 0;
    while (i < n && !args->itens[i]->texto && args->itens[i]->i2 == 0) {
        expr(c, u, args->itens[i]->a);
        i++; k++;
    }
    emite(c, u, OP_BUILD_LIST, k);
    for (; i < n; i++) {
        PSNode *a = args->itens[i];
        if (a->texto || a->i2 == 2) break;
        expr(c, u, a->a);
        if (a->i2 == 0) emite(c, u, OP_BUILD_LIST, 1);
        emite(c, u, OP_LIST_EXTEND, 0);
    }
    /* Nomeados: os primeiros `k=v` numa BUILD_DICT só; depois cada `**d`
     * funde e cada `k=v` solto entra como dict de um par. */
    int32_t m = 0;
    while (i < n && args->itens[i]->texto) {
        const char *nm = args->itens[i]->texto;
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_STR, 0, 0, nm, (int32_t)strlen(nm)));
        expr(c, u, args->itens[i]->a);
        i++; m++;
    }
    emite(c, u, OP_BUILD_DICT, m);
    for (; i < n; i++) {
        PSNode *a = args->itens[i];
        if (a->texto) {
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_STR, 0, 0, a->texto, (int32_t)strlen(a->texto)));
            expr(c, u, a->a);
            emite(c, u, OP_BUILD_DICT, 1);
        } else {
            expr(c, u, a->a);
        }
        emite(c, u, OP_DICT_MERGE, 0);
    }
    emite(c, u, OP_CALL_EX, 0);
}

/* A expressão de um decorador geral (`@obj.metodo(args)`, `@log()`, `@log`):
 * com parênteses é CHAMADA, sem parênteses é o VALOR (o parser marca em
 * `i2`). Ela vira a árvore de uma expressão comum: o nome, um acesso a membro
 * por `.` e, com parênteses, a chamada com os argumentos do decorador, cada
 * nó onde o parser põe o da expressão (o `.` e o `(`, ver PSNode.i). Pelo
 * `expr`, ela passa pelo checador como `obj.metodo(args)` em qualquer lugar:
 * o membro que não existe e a quantidade, o nome e o tipo dos argumentos saem
 * no `--check`, com a frase e a coluna de quando roda. Antes só o primeiro
 * nome era conferido, e `@app.rota("/x")` passava no `--check` e quebrava
 * rodando. É a mesma emissão em qualquer posição (funct solta, em cima de
 * classe, dentro de classe). O valor fica no topo da pilha: é o decorador a
 * quem o OP_DECORA entrega a funct. */
static int tp_conta_utf8(const char *s, int32_t n);
static void emite_decorador_expr(C *c, Unidade *u, PSNode *dec)
{
    int32_t n = dec->lista.n;
    if (n <= 0) return;
    PSNode *nos = calloc((size_t)n + 1, sizeof(PSNode));
    if (!nos) { cerro(c, "sem memoria", dec); return; }
    PSNode *e = NULL;
    for (int32_t i = 0; i < n; i++) {
        PSNode *parte = dec->lista.itens[i], *m = &nos[i];
        const char *nome = parte->texto ? parte->texto : "";
        m->kind = i ? N_MEMBER_ACCESS : N_NAME;
        m->line = i && parte->i ? (int32_t)(parte->i >> 32) : parte->line;
        m->col = i && parte->i ? (int32_t)(uint32_t)parte->i : parte->col;
        m->texto = nome;
        m->linha_fim = parte->line;
        m->col_fim = parte->col + tp_conta_utf8(nome, (int32_t)strlen(nome));
        m->a = e;
        e = m;
    }
    if (dec->i2) {                     /* `@log`: o valor, sem chamar */
        PSNode *ch = &nos[n];
        ch->kind = N_CALL;
        ch->line = dec->i ? (int32_t)(dec->i >> 32) : dec->line;
        ch->col = dec->i ? (int32_t)(uint32_t)dec->i : dec->col;
        ch->linha_fim = dec->linha_fim;
        ch->col_fim = dec->col_fim;
        ch->a = e;
        ch->lista = dec->lista2;
        e = ch;
    }
    expr(c, u, e);
    /* as marcas da chamada em curso não podem ficar apontando pra estes nós */
    for (int32_t i = 0; i <= n; i++) {
        if (c->membro_chamado == &nos[i]) c->membro_chamado = NULL;
        if (c->alvo_chamado == &nos[i]) c->alvo_chamado = NULL;
    }
    free(nos);
}

/* modo 0: [.., decorador, funct] -> OP_DECORA -> [.., resultado].
 * modo 1 (método): [.., classe, nome_metodo, decorador, funct] -> [.., resultado].
 * O nome juntado (`app.route`) vai como constante, pra mensagem de erro. */
static void emite_decora(C *c, Unidade *u, PSNode *dec, int32_t modo)
{
    char nome[256]; size_t j = 0;
    nome[0] = '\0';
    for (int32_t i = 0; i < dec->lista.n; i++) {
        const char *p = dec->lista.itens[i]->texto ? dec->lista.itens[i]->texto : "?";
        size_t t = strlen(p);
        if (j + t + 2 >= sizeof(nome)) break;
        if (i) nome[j++] = '.';
        memcpy(nome + j, p, t); j += t; nome[j] = '\0';
    }
    emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_STR, 0, 0, nome, (int32_t)j));
    emite(c, u, OP_DECORA, modo);
}

static void carrega_nome_sem_estatico(C *c, Unidade *u, const char *nome);

static void carrega_nome(C *c, Unidade *u, const char *nome)
{
    /* Campo `static` da Entity em compilação ganha do global e do "resolve em
     * runtime" — mas NÃO de um local: parâmetro com o mesmo nome continua
     * sendo o parâmetro. */
    if (c->entity_no && !nome_e_local(u, nome) && campo_estatico_da_classe(c, nome)) {
        carrega_estatico(c, u, nome);
        return;
    }
    carrega_nome_sem_estatico(c, u, nome);
}

/* O nome como variável: global, local, célula, captura ou "resolve rodando".
 * É por aqui que o `static` de uma classe declarada dentro de funct carrega a
 * própria classe (`carrega_estatico`), sem voltar à pergunta do static. */
static void carrega_nome_sem_estatico(C *c, Unidade *u, const char *nome)
{
    {
        int32_t s = celula_ativa_do_topo(u, nome);    /* variável de bloco capturada */
        if (s >= 0) {
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, s, 0, NULL, 0));
            emite(c, u, OP_CELL_GET_NAME, idx_global(c, nome));
            return;
        }
    }
    if (u->eh_modulo || eh_global_declarada(u, nome)) {
        emite(c, u, OP_LOAD_GLOBAL, idx_global(c, nome));
        return;
    }
    for (int32_t i = u->nlocais - 1; i >= 0; i--) {
        if (strcmp(u->locais[i], nome) != 0) continue;
        if (u->celula[i] && u->certo[i]) emite(c, u, OP_CELL_GET, i);
        else if (u->celula[i]) {
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, i, 0, NULL, 0));
            emite(c, u, OP_CELL_GET_NAME, idx_global(c, nome));
        }
        else if (u->certo[i]) emite(c, u, OP_LOAD_LOCAL, i);   /* param/tipada */
        else {
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, i, 0, NULL, 0));
            emite(c, u, OP_LOAD_NAME, idx_global(c, nome));
        }
        return;
    }
    /* nome de FORA: se é local de uma função que envolve esta, captura */
    {
        int32_t up = resolve_upval(c, u, nome);
        if (up >= 0) { emite(c, u, OP_LOAD_UPVAL, up); return; }
    }
    /* nome nunca visto nesta função: reserva slot e resolve em runtime */
    {
        int32_t i = idx_local(c, u, nome);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, i, 0, NULL, 0));
        emite(c, u, OP_LOAD_NAME, idx_global(c, nome));
    }
}

/* `certa` = o nome é local com certeza (parâmetro ou declaração tipada) */
/* O nome já existe no escopo atual? Serve pro `for each`: se existe, a
 * variável do laço tem que SOMBREAR (salvar e devolver depois) em vez de
 * sobrescrever — antes, `i = "x"` seguido de `for each i in [1,2]` deixava o
 * `i` valendo 2 pra sempre, e com uma FUNCT de mesmo nome a função sumia. */
static int nome_ja_existe(Unidade *u, const char *nome)
{
    if (!nome || !*nome) return 0;
    if (u->eh_modulo) return ix_busca(&u->ix_mod, nome) >= 0;
    for (int32_t i = u->nlocais - 1; i >= 0; i--)
        if (strcmp(u->locais[i], nome) == 0)
            return !u->celula_virgem[i];
    return 0;
}

/* ── tipagem estatica ────────────────────────────────────────────────────
 *
 * O tipo escrito numa declaracao (`str s = "a"`) e da VARIAVEL, nao so do
 * valor inicial. Antes a checagem valia uma vez, na criacao, e depois `s =
 * 42` passava — a doc chamava isso de "dinamica". Agora o compilador guarda o
 * tipo de cada nome declarado (slot da funcao, nome do modulo, upvalue) e
 * emite o OP_COERCE_DECL antes de TODA escrita nele: reatribuicao, `for
 * each`, desempacotamento, write-through de dentro de uma funct eclosure.
 * Nada muda na VM alem do proprio opcode conferir tambem list/dict/tup/Object.
 *
 * Nome sem tipo declarado continua como sempre: `x = 1` depois `x = "a"`. */

static void terro(C *c, PSNode *n, const char *classe, const char *fmt, ...);

/* `private <declaracao>` no topo do modulo: o nome entra na lista que o
 * `import` consulta, com a linha/coluna da declaracao (o traceback do acesso
 * negado mostra onde o nome foi declarado private). Antes o `private` de
 * funct compilava e nao fazia nada. */
static void priv_global_add(C *c, const char *nome, PSNode *no)
{
    if (!nome || !c->out) return;
    PSPrograma *o = c->out;
    int32_t n = o->npriv_globais;
    char **nv = realloc(o->priv_globais, sizeof(char *) * (size_t)(n + 1));
    if (!nv) { cerro(c, "sem memoria", NULL); return; }
    o->priv_globais = nv;
    int32_t *nl = realloc(o->priv_linhas, sizeof(int32_t) * (size_t)(n + 1));
    if (!nl) { cerro(c, "sem memoria", NULL); return; }
    o->priv_linhas = nl;
    int32_t *nc = realloc(o->priv_cols, sizeof(int32_t) * (size_t)(n + 1));
    if (!nc) { cerro(c, "sem memoria", NULL); return; }
    o->priv_cols = nc;
    o->priv_globais[n] = strdup(nome);
    if (!o->priv_globais[n]) { cerro(c, "sem memoria", NULL); return; }
    o->priv_linhas[n] = no ? no->line : 0;
    o->priv_cols[n]   = no ? no->col : 0;
    o->npriv_globais = n + 1;
}

/* O statement `n` esta no TOPO do arquivo (fora de funct, de bloco e de
 * Entity)? E onde `private`/`public` tem efeito no modulo. Declaracao dentro
 * de um bloco de decorador do topo (`@app.get` em cima da funct) conta como
 * topo — e como o `modulo_decl` a enxerga. */
static int decorador_contem(PSNode *s, PSNode *n)
{
    if (!s || s->kind != N_DECORATOR_STMT || !s->b || s->b->kind != N_BLOCK) return 0;
    for (int32_t k = 0; k < s->b->lista.n; k++) {
        PSNode *d = s->b->lista.itens[k];
        if (d == n || decorador_contem(d, n)) return 1;
    }
    return 0;
}

static int no_topo_do_modulo(C *c, Unidade *u, PSNode *n)
{
    if (!u->eh_modulo || c->dentro_entity || !c->stmt_topo) return 0;
    return c->stmt_topo == n || decorador_contem(c->stmt_topo, n);
}

/* `private` numa declaracao: no topo do modulo registra o nome; em qualquer
 * outro lugar (dentro de funct, de `if`/`for`/`try`, do guard `if __name__`)
 * nao teria efeito nenhum — e palavra que nao faz o que diz e erro, nao
 * silencio. `no_erro` e o no acusado (o da declaracao). */
static void priv_declara(C *c, Unidade *u, PSNode *decl, PSNode *no_erro, const char *nome)
{
    if (no_topo_do_modulo(c, u, decl)) { priv_global_add(c, nome, decl); return; }
    /* `private name = nome` dentro de um metodo de Entity: quem escreve isso
     * quer o CAMPO do objeto, e a forma dele leva o tipo */
    if (c->dentro_entity && !u->eh_modulo && decl->kind == N_ASSIGNMENT) {
        terro(c, no_erro, "SyntaxError",
              "'private %s = ...' dentro de uma funct de Entity declara campo do objeto: "
              "escreva 'private <tipo> %s = <valor>'", nome ? nome : "nome", nome ? nome : "nome");
        return;
    }
    terro(c, no_erro, "SyntaxError",
          "'private' aqui nao tem efeito: so vale no topo do arquivo (o que nao sai pelo import) "
          "ou no corpo da Entity");
}

/* Os NOMEs de um alvo de desempacotamento (`a, (b, *c) = …`), pra marca-los
 * private. Membro e indice (`o.x, l[i] = …`) nao declaram nome: passam. */
static void priv_alvos_unpack(C *c, Unidade *u, PSNode *decl, PSNode *alvo)
{
    if (!alvo) return;
    if (alvo->kind == N_NAME) { priv_declara(c, u, decl, decl, alvo->texto); return; }
    if (alvo->kind == N_UNPACK_TARGET)
        for (int32_t i = 0; i < alvo->lista.n; i++) priv_alvos_unpack(c, u, decl, alvo->lista.itens[i]);
}

/* Código TIPO_* de um nome de tipo (canônico ou apelido), ou -1. É o código da
 * tabela única `ps_tipos.def` — a mesma ordem do enum da VM e do bytecode. Aqui
 * havia uma cópia à mão dos números do enum, e ela já divergia do parser. */
static int cod_tipo_decl(const char *t)
{
    return ps_tipo_codigo(t);
}

static void tipo_topo_poe(C *c, const char *nome, int cod)
{
    if (!nome || cod < 0) return;
    for (int32_t i = 0; i < c->n_tipos_topo; i++)
        if (strcmp(c->tipos_topo_nomes[i], nome) == 0) { c->tipos_topo[i] = (unsigned char)(cod + 1); return; }
    if (c->n_tipos_topo + 1 > c->cap_tipos_topo) {
        int32_t novo = c->cap_tipos_topo < 8 ? 8 : c->cap_tipos_topo * 2;
        char **nn = realloc(c->tipos_topo_nomes, sizeof(char *) * (size_t)novo);
        unsigned char *nt = realloc(c->tipos_topo, (size_t)novo);
        if (!nn || !nt) { free(nn == c->tipos_topo_nomes ? NULL : nn); cerro(c, "sem memoria", NULL); return; }
        c->tipos_topo_nomes = nn; c->tipos_topo = nt; c->cap_tipos_topo = novo;
    }
    c->tipos_topo_nomes[c->n_tipos_topo] = strdup(nome);
    c->tipos_topo[c->n_tipos_topo] = (unsigned char)(cod + 1);
    c->n_tipos_topo++;
}

static int tipo_topo_de(C *c, const char *nome)
{
    if (!nome) return 0;
    for (int32_t i = 0; i < c->n_tipos_topo; i++)
        if (strcmp(c->tipos_topo_nomes[i], nome) == 0) return c->tipos_topo[i];
    return 0;
}

/* Passada previa: toda declaracao tipada no escopo de MODULO (fora de funct,
 * dentro ou fora de bloco). A primeira declaracao de cada nome vence. */
static void coleta_tipos_topo(C *c, PSNode *n)
{
    if (!n || fundo_demais(c, n, 1)) return;
    while (n->kind == N_BINARY_OP) {      /* cadeia `a + b + …` em laço (ver liga_o_nome) */
        coleta_tipos_topo(c, n->b);
        n = n->a;
        if (!n) return;
    }
    if (n->kind == N_FUNCT_DECL) return;       /* o corpo dela e outro escopo */
    if (n->kind == N_VAR_DECL && n->texto && !tipo_topo_de(c, n->texto))
        tipo_topo_poe(c, n->texto, cod_tipo_decl(n->texto2));
    coleta_tipos_topo(c, n->a);
    coleta_tipos_topo(c, n->b);
    coleta_tipos_topo(c, n->c);
    coleta_tipos_topo(c, n->e);
    for (int32_t i = 0; i < n->lista.n; i++)  coleta_tipos_topo(c, n->lista.itens[i]);
    for (int32_t i = 0; i < n->lista2.n; i++) coleta_tipos_topo(c, n->lista2.itens[i]);
}

/* tipo declarado de `nome` na funcao que ENVOLVE (a dona da celula), pra
 * escrita via upvalue */
static int tipo_de_upval(Unidade *u, const char *nome)
{
    for (Unidade *q = u->pai; q && !q->eh_modulo; q = q->pai)
        for (int32_t i = q->nlocais - 1; i >= 0; i--)
            if (strcmp(q->locais[i], nome) == 0) return q->tipo_decl[i];
    return 0;
}

/* Emite a conferencia do tipo declarado (se houver) sobre o valor no topo da
 * pilha — e chamado imediatamente antes do store. `cod1` = TIPO_* + 1. */
static void emite_coerce_se_tipado(C *c, Unidade *u, const char *nome, int cod1)
{
    if (cod1 <= 0) return;
    int32_t ni = idx_const(c, u, K_STR, 0, 0, nome, (int32_t)strlen(nome));
    emite(c, u, OP_COERCE_DECL, (ni << 4) | (cod1 - 1));
}

/* `global x` + escrita de dentro de uma funct: o nome é do arquivo, e sai no
 * import como os de topo (ver PSPrograma.exportados). */
static void global_gravado_add(C *c, const char *nome)
{
    for (int32_t i = 0; i < c->nglobais_gravados; i++)
        if (strcmp(c->globais_gravados[i], nome) == 0) return;
    char **nv = realloc(c->globais_gravados, sizeof(char *) * (size_t)(c->nglobais_gravados + 1));
    if (!nv) { cerro(c, "sem memoria", NULL); return; }
    c->globais_gravados = nv;
    char *copia = strdup(nome);
    if (!copia) { cerro(c, "sem memoria", NULL); return; }
    c->globais_gravados[c->nglobais_gravados++] = copia;
}

/* ══ tipagem estática: o que se sabe ANTES de rodar ══════════════════════════
 *
 * Tudo que tem tipo é conferido aqui, na compilação — o mesmo funil de rodar,
 * importar e `--check`, então o veredito é o mesmo nos três. Três respostas
 * possíveis pra "este valor serve neste lugar?":
 *
 *   T_SIM     sabido e certo: nada é emitido pra rodar (fica mais leve)
 *   T_NAO     sabido e errado: erro de compilação, e a compilação SEGUE pra
 *             achar os outros — o programa com erro não roda, e a lista sai
 *             inteira
 *   T_TALVEZ  o tipo só se sabe rodando (`json.parse`, `d["k"]`): a
 *             conferência é emitida no lugar (OP_COERCE_DECL/OP_CONFERE_TIPO)
 *
 * Os nomes se resolvem na MESMA ordem do `carrega_nome`/`guarda_nome_modo`
 * — campo static da classe, módulo, `global`, local, captura, global do
 * arquivo —, só que sem criar slot nem upvalue: aqui só se pergunta. Regra
 * de nome em dois lugares diverge; esta segue a de lá linha a linha.
 *
 * Variável sem tipo escrito tem o tipo fixado pela primeira atribuição: o
 * tipo que o compilador sabe do primeiro valor, e dali em diante só ele vale.
 * Valor de tipo desconhecido fixa "qualquer" (aceita tudo, como Object). Null
 * não fixa nem conflita: variável sem tipo escrito pode começar e voltar a
 * ficar vazia.
 * Declarado (`str s`, parâmetro tipado) não aceita Null — é a regra que já
 * valia rodando. */

/* Guarda um texto de tipo montado aqui (a união) — um só por texto. */
static const char *tp_guarda(C *c, const char *s)
{
    for (int32_t i = 0; i < c->ntpool; i++)
        if (strcmp(c->tpool[i], s) == 0) return c->tpool[i];
    if (c->ntpool + 1 > c->cap_tpool) {
        int32_t novo = c->cap_tpool < 16 ? 16 : c->cap_tpool * 2;
        char **nv = realloc(c->tpool, sizeof(char *) * (size_t)novo);
        if (!nv) { cerro(c, "sem memoria", NULL); return NULL; }
        c->tpool = nv; c->cap_tpool = novo;
    }
    char *d = strdup(s);
    if (!d) { cerro(c, "sem memoria", NULL); return NULL; }
    c->tpool[c->ntpool++] = d;
    return d;
}

/* Registra um erro da tipagem estática — sem parar a compilação. O mesmo
 * erro no mesmo lugar entra uma vez (um nó pode ser olhado duas vezes). */
static void terro(C *c, PSNode *n, const char *classe, const char *fmt, ...)
{
    PSErroTipo e;
    memset(&e, 0, sizeof(e));
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(e.msg, sizeof(e.msg), fmt, ap);
    va_end(ap);
    snprintf(e.classe, sizeof(e.classe), "%s", classe);
    e.linha = (n && n->line) ? n->line : c->linha_atual;
    e.col   = (n && n->col)  ? n->col  : c->coluna_atual;
    /* o trecho é o nó inteiro (a expressão, a chamada), não um caractere */
    if (n && n->col_fim) {
        e.linha_fim = n->linha_fim ? n->linha_fim : n->line;
        e.col_fim   = n->col_fim;
    }
    for (int32_t i = 0; i < c->nerros; i++)
        if (c->erros[i].linha == e.linha && c->erros[i].col == e.col && !strcmp(c->erros[i].msg, e.msg))
            return;
    if (c->nerros + 1 > c->cap_erros) {
        int32_t novo = c->cap_erros < 16 ? 16 : c->cap_erros * 2;
        PSErroTipo *nv = realloc(c->erros, sizeof(PSErroTipo) * (size_t)novo);
        if (!nv) { cerro(c, "sem memoria", NULL); return; }
        c->erros = nv; c->cap_erros = novo;
    }
    c->erros[c->nerros++] = e;
}

/* Nome de tipo escrito, na grafia canônica (`String` -> `str`); nome que não
 * é da tabela (classe, objeto nativo) fica como foi escrito. */
static const char *tp_canon(const char *t)
{
    if (!t) return NULL;
    const char *k = ps_tipo_canonico(t);
    return k ? k : t;
}

/* `tp_canon` pra um texto que vai ficar guardado além da declaração de onde
 * veio (o `SimInfo.tipo` de um nome de módulo): um tipo da tabela é estático;
 * um nome de classe é copiado pro pool do compilador, porque a árvore da
 * segunda passada morre no fim do statement. */
static const char *tp_canon_guarda(C *c, const char *t)
{
    if (!t) return NULL;
    const char *k = ps_tipo_canonico(t);
    if (k) return k;
    /* sem memória pro pool o `tp_guarda` já acusou (a compilação falha);
     * devolve o texto da árvore pra ninguém desreferenciar NULL até lá */
    k = tp_guarda(c, t);
    return k ? k : t;
}

/* ── gêmeos: o nó podado de um nó da segunda passada ────────────────────── */
static uint64_t gemeo_chave(const PSNode *n)
{
    return ((uint64_t)(uint32_t)n->line << 32) | ((uint64_t)(uint32_t)n->col << 8) | (uint64_t)(n->kind & 0xFF);
}

static void gemeo_poe(C *c, PSNode *podado)
{
    if (!podado) return;
    if ((c->ngemeos + 1) * 10 > c->cap_gemeos * 7) {
        int32_t nc = c->cap_gemeos ? c->cap_gemeos * 2 : 64;
        struct Gemeo *nv = calloc((size_t)nc, sizeof(*nv));
        if (!nv) { cerro(c, "sem memoria", NULL); return; }
        for (int32_t i = 0; i < c->cap_gemeos; i++) {
            if (!c->gemeos[i].no) continue;
            uint32_t m = (uint32_t)nc - 1, j = (uint32_t)(c->gemeos[i].chave * 11400714819323198485ull >> 20) & m;
            while (nv[j].no) j = (j + 1) & m;
            nv[j] = c->gemeos[i];
        }
        free(c->gemeos);
        c->gemeos = nv; c->cap_gemeos = nc;
    }
    uint64_t ch = gemeo_chave(podado);
    uint32_t m = (uint32_t)c->cap_gemeos - 1, j = (uint32_t)(ch * 11400714819323198485ull >> 20) & m;
    while (c->gemeos[j].no) {
        if (c->gemeos[j].chave == ch) { c->gemeos[j].no = podado; return; }
        j = (j + 1) & m;
    }
    c->gemeos[j].chave = ch; c->gemeos[j].no = podado;
    c->ngemeos++;
}

/* O gêmeo podado de `n`, ou o próprio `n` quando não há (compilação da
 * árvore inteira, ou declaração que só vive dentro do statement). */
static PSNode *gemeo_de(C *c, PSNode *n)
{
    if (!n || !c->cap_gemeos) return n;
    uint64_t ch = gemeo_chave(n);
    uint32_t m = (uint32_t)c->cap_gemeos - 1, j = (uint32_t)(ch * 11400714819323198485ull >> 20) & m;
    while (c->gemeos[j].no) {
        if (c->gemeos[j].chave == ch) return c->gemeos[j].no;
        j = (j + 1) & m;
    }
    return n;
}

/* O mesmo nó, seja o ponteiro ou o gêmeo dele na outra passada. */
static int mesmo_no(const PSNode *a, const PSNode *b)
{
    if (a == b) return 1;
    return a && b && a->kind == b->kind && a->line == b->line && a->col == b->col;
}

/* `lado` é um dos lados da união `u`? */
static int tp_tem_lado(const char *u, const char *lado)
{
    size_t n = strlen(lado);
    const char *q = u;
    while (q && *q) {
        const char *bar = strchr(q, '|');
        size_t len = bar ? (size_t)(bar - q) : strlen(q);
        if (len == n && memcmp(q, lado, n) == 0) return 1;
        q = bar ? bar + 1 : NULL;
    }
    return 0;
}

/* A união de dois tipos (desconhecido de um lado = desconhecido). */
static const char *tp_uniao(C *c, const char *a, const char *b)
{
    if (!a || !b) return NULL;
    if (strcmp(a, b) == 0) return a;
    char buf[256];
    size_t n = (size_t)snprintf(buf, sizeof(buf), "%s", a);
    const char *q = b;
    while (q && *q) {
        const char *bar = strchr(q, '|');
        size_t len = bar ? (size_t)(bar - q) : strlen(q);
        char lado[128];
        if (len >= sizeof(lado)) return NULL;
        memcpy(lado, q, len); lado[len] = '\0';
        if (!tp_tem_lado(buf, lado)) {
            if (n + len + 2 >= sizeof(buf)) return NULL;
            buf[n++] = '|';
            memcpy(buf + n, lado, len + 1);
            n += len;
        }
        q = bar ? bar + 1 : NULL;
    }
    return tp_guarda(c, buf);
}

/* A Entity/model/enum do arquivo com esse nome (a primeira), ou NULL: é o
 * que diz se o nome de tipo EXISTE. */
static PSNode *tp_tipo_decl_arq(C *c, const char *nome)
{
    if (!nome) return NULL;
    for (int32_t i = 0; i < c->ntipos_arq; i++)
        if (c->tipos_arq[i]->texto && strcmp(c->tipos_arq[i]->texto, nome) == 0) return c->tipos_arq[i];
    return NULL;
}

static int tp_tipo_ambiguo(C *c, const char *nome)
{
    for (int32_t i = 0; nome && i < c->ntipos_ambiguos; i++)
        if (strcmp(c->tipos_ambiguos[i], nome) == 0) return 1;
    return 0;
}

/* A declaração que um valor do tipo `nome` É — a que se confere membro,
 * herança e construtor —, ou NULL: nome que não é tipo do arquivo, ou que
 * mais de uma declaração usa (ver `C.tipos_ambiguos`). */
static PSNode *tp_tipo_arq(C *c, const char *nome)
{
    PSNode *d = tp_tipo_decl_arq(c, nome);
    return d && c->ntipos_ambiguos > 0 && tp_tipo_ambiguo(c, nome) ? NULL : d;
}

static void tp_coleta_tipos_arq(C *c, PSNode *n)
{
    if (!n || fundo_demais(c, n, 1)) return;
    while (n->kind == N_BINARY_OP) {      /* cadeia `a + b + …` em laço (ver liga_o_nome) */
        tp_coleta_tipos_arq(c, n->b);
        n = n->a;
        if (!n) return;
    }
    PSNode *ja = (n->kind == N_ENTITY_DECL || n->kind == N_MODEL_DECL || n->kind == N_ENUM_DECL) && n->texto
               ? tp_tipo_decl_arq(c, n->texto) : n;
    if (!ja) {
        if (c->ntipos_arq + 1 > c->cap_tipos_arq) {
            int32_t novo = c->cap_tipos_arq < 8 ? 8 : c->cap_tipos_arq * 2;
            PSNode **nv = realloc(c->tipos_arq, sizeof(PSNode *) * (size_t)novo);
            if (!nv) { cerro(c, "sem memoria", NULL); return; }
            c->tipos_arq = nv; c->cap_tipos_arq = novo;
        }
        c->tipos_arq[c->ntipos_arq++] = n;
    } else if (ja != n && !mesmo_no(ja, n) && !tp_tipo_ambiguo(c, n->texto)) {
        char **nv = realloc(c->tipos_ambiguos, sizeof(char *) * (size_t)(c->ntipos_ambiguos + 1));
        if (!nv) { cerro(c, "sem memoria", NULL); return; }
        c->tipos_ambiguos = nv;
        char *nm = strdup(n->texto);
        if (!nm) { cerro(c, "sem memoria", NULL); return; }
        c->tipos_ambiguos[c->ntipos_ambiguos++] = nm;
    }
    tp_coleta_tipos_arq(c, n->a);
    tp_coleta_tipos_arq(c, n->b);
    tp_coleta_tipos_arq(c, n->c);
    tp_coleta_tipos_arq(c, n->e);
    for (int32_t i = 0; i < n->lista.n; i++)  tp_coleta_tipos_arq(c, n->lista.itens[i]);
    for (int32_t i = 0; i < n->lista2.n; i++) tp_coleta_tipos_arq(c, n->lista2.itens[i]);
}

/* 1 = a classe `filha` é `mae` ou herda dela; 0 = não; -1 = a cadeia passa
 * por um pai que o arquivo não declara (importado): não dá pra saber. */
static int tp_herda(C *c, const char *filha, const char *mae, int prof)
{
    if (strcmp(filha, mae) == 0) return 1;
    if (prof > 32) return -1;
    PSNode *d = tp_tipo_arq(c, filha);
    if (!d || d->kind != N_ENTITY_DECL) return 0;
    int incerto = 0;
    for (int32_t i = 0; i < d->lista2.n; i++) {
        const char *p = d->lista2.itens[i]->texto;
        if (!p) continue;
        int r = tp_herda(c, p, mae, prof + 1);
        if (r == 1) return 1;
        if (r == -1 || !tp_tipo_arq(c, p)) incerto = 1;
    }
    return incerto ? -1 : 0;
}

/* Quantos caracteres (não bytes) tem o literal de texto. */
static int tp_conta_utf8(const char *s, int32_t n)
{
    int k = 0;
    for (int32_t i = 0; i < n; i++)
        if (((unsigned char)s[i] & 0xC0) != 0x80) k++;
    return k;
}

/* Um tipo `d` (sem união) aceita um valor de tipo `v` (sem união)?
 * `declarado`: 0 = a regra do tipo fixado pela primeira atribuição; 1 = a do
 * tipo ESCRITO; 2 = tipo escrito numa DECLARAÇÃO (`char c = 64` é o
 * caractere daquele codepoint — regra dele, só ali; em retorno, parâmetro e
 * campo `char` é um caractere de texto). `no` é o valor, quando se tem:
 * literal diz mais que o tipo (`int` promete 64 bits, `char` é um caractere). */
static int tp_aceita_um(C *c, const char *d, const char *v, PSNode *no, int declarado)
{
    /* Null só serve declarado onde o tipo É `Null` (o retorno da `Null funct`) */
    if (strcmp(v, "Null") == 0) return declarado && strcmp(d, "Null") != 0 ? T_NAO : T_SIM;
    /* `Object` é qualquer valor (o Null saiu acima): aceita todo tipo, e um
     * valor que só se sabe ser `Object` pode ser de qualquer tipo — quem
     * decide é a VM rodando (`aceita_obj`, a mesma regra) */
    if (strcmp(d, "Object") == 0) return T_SIM;
    if (strcmp(v, "Object") == 0) return T_TALVEZ;
    if (strcmp(d, v) == 0) {
        if (declarado && strcmp(d, "int") == 0) {
            if (no && no->kind == N_LITERAL && no->lit == L_INT) return T_SIM;
            return T_TALVEZ;          /* conta de int pode passar de 64 bits */
        }
        return T_SIM;
    }
    if (declarado) {
        if (strcmp(d, "int") == 0 && no && no->kind == N_LITERAL && no->lit == L_BIGINT) return T_NAO;
        if (strcmp(d, "long") == 0) return strcmp(v, "int") == 0 ? T_SIM : T_NAO;
        if (strcmp(d, "char") == 0) {
            if (strcmp(v, "str") == 0) {
                if (no && no->kind == N_LITERAL && no->lit == L_STR && no->texto)
                    return tp_conta_utf8(no->texto, no->texto_len > 0 ? no->texto_len : (int32_t)strlen(no->texto)) == 1
                           ? T_SIM : T_NAO;
                return T_TALVEZ;
            }
            /* na declaração, inteiro vira o caractere daquele codepoint — e
             * o intervalo só rodando (`char c = -1` é ConversionError) */
            if (strcmp(v, "int") == 0 || strcmp(v, "bool") == 0) return declarado == 2 ? T_TALVEZ : T_NAO;
            return T_NAO;
        }
        if (strcmp(d, "type") == 0 && strcmp(v, "Entity") == 0) return T_TALVEZ;
    }
    int h = tp_herda(c, v, d, 0);
    if (h == 1) return T_SIM;
    if (h == -1) return T_TALVEZ;
    return T_NAO;
}

/* `d` aceita `v`? Os dois podem ser união: cada lado do valor precisa de um
 * lado do tipo que o aceite. Valor desconhecido: só rodando. */
static int tp_aceita(C *c, const char *d, const char *v, PSNode *no, int declarado)
{
    if (!d) return T_SIM;
    if (!v) return T_TALVEZ;
    int todos_sim = 1, algum_sim = 0, algum_talvez = 0;
    const char *qv = v;
    while (qv && *qv) {
        const char *barv = strchr(qv, '|');
        size_t lv = barv ? (size_t)(barv - qv) : strlen(qv);
        char ladov[128];
        if (lv >= sizeof(ladov)) return T_TALVEZ;
        memcpy(ladov, qv, lv); ladov[lv] = '\0';
        int melhor = T_NAO;
        const char *qd = d;
        while (qd && *qd) {
            const char *bard = strchr(qd, '|');
            size_t ld = bard ? (size_t)(bard - qd) : strlen(qd);
            char ladod[128];
            if (ld >= sizeof(ladod)) return T_TALVEZ;
            memcpy(ladod, qd, ld); ladod[ld] = '\0';
            /* o nó do valor só vale quando o valor é de um lado só */
            int r = tp_aceita_um(c, ladod, ladov, barv || qv != v ? NULL : no, declarado);
            if (r == T_SIM) { melhor = T_SIM; break; }
            if (r == T_TALVEZ) melhor = T_TALVEZ;
            qd = bard ? bard + 1 : NULL;
        }
        if (melhor != T_SIM) todos_sim = 0;
        if (melhor == T_SIM) algum_sim = 1;
        if (melhor == T_TALVEZ) algum_talvez = 1;
        qv = barv ? barv + 1 : NULL;
    }
    if (todos_sim) return T_SIM;
    if (!algum_sim && !algum_talvez) return T_NAO;
    return T_TALVEZ;
}

/* O nome de tipo escrito existe? Tipo da linguagem, Entity/model/enum do
 * arquivo, tipo que um nativo devolve ou tem tabela, ou nome que um import
 * liga sem que se saiba o que é (a classe pode vir do módulo). Nome de
 * módulo não é tipo: `bytes d = b"x"` é "tipo bytes não existe" (o tipo é
 * `byte`). */
static SimInfo *tp_sim_de(C *c, Unidade *u, const char *nome);
static int tp_tipo_existe(C *c, Unidade *u, const char *t)
{
    if (!t) return 1;
    if (ps_tipo_info(t) || tp_tipo_decl_arq(c, t)) return 1;
    if (ps_retorno_tipo_existe(t) || ps_nativo_eh_tipo(t)) return 1;
    SimInfo *s = tp_sim_de(c, u, t);
    return s && s->estado == 1 && !s->tipo && !s->decl;
}

/* ── o que se sabe de um nome ── */
static SimInfo *tp_sim_topo(C *c, const char *nome)
{
    int32_t i = ix_busca(&c->ix_topo, nome);
    return i >= 0 ? &c->topo[i].s : NULL;
}

/* O tipo de cada item que o `for each` tira do iterável: texto dá texto,
 * bytes dá int; lista, tupla e dict guardam qualquer coisa. */
static const char *tp_elemento(const char *t)
{
    if (!t) return NULL;
    if (!strcmp(t, "str")) return "str";
    if (!strcmp(t, "byte")) return "int";
    return NULL;
}

static const char *tp_uniao(C *c, const char *a, const char *b);

/* O tipo de cada item que o laço (ou a compreensão) recebe de `it`. A lista
 * e a tupla ESCRITAS têm o tipo nos itens: a união deles (`["a", "b"]` dá
 * str, `[1, "a"]` dá int|str), e o `int i` do laço que recebe str é acusado
 * antes de rodar. Item de tipo que não se sabe deixa o laço sem tipo. */
static const char *tp_elemento_de(C *c, Unidade *u, PSNode *it)
{
    if (it && (it->kind == N_LIST_LITERAL || it->kind == N_TUPLE_LITERAL) && it->lista.n > 0) {
        const char *t = NULL;
        for (int32_t i = 0; i < it->lista.n; i++) {
            PSNode *item = it->lista.itens[i];
            if (!item) return NULL;
            const char *ti = tp_de(c, u, item);
            if (!ti) return NULL;
            t = t ? tp_uniao(c, t, ti) : ti;
            if (!t) return NULL;
        }
        return t;
    }
    return tp_elemento(tp_de(c, u, it));
}

static SimInfo *tp_sim_de(C *c, Unidade *u, const char *nome)
{
    if (!nome) return NULL;
    /* membro `static` da classe pelo nome solto: o tipo do campo e a
     * assinatura do método valem como em `App.x` — `s(1, 2)` num `static
     * funct s()` é acusado, `total + "a"` num `static int total` também */
    if (c->entity_no && !nome_e_local(u, nome)) {
        PSNode *dono = NULL;
        PSNode *st = estatico_da_classe(c, nome, &dono);
        if (st) {
            SimInfo *s = &c->sim_estatico;
            memset(s, 0, sizeof(*s));
            if (st->kind == N_FUNCT_DECL) {
                s->estado = 1; s->tipo = "funct"; s->decl = st;
                s->decorado = (unsigned char)(dono && dono->texto && tp_metodo_decorado(c, dono->texto, st));
            } else if (st->texto2) {
                s->estado = 2; s->tipo = tp_canon(st->texto2);
            } else {
                s->estado = 1;
            }
            return s;
        }
    }
    if (u->eh_modulo) {
        int32_t k = ix_busca(&u->ix_mod, nome);
        if (k >= 0) return &u->mod_sim[k];
        return tp_sim_topo(c, nome);
    }
    if (eh_global_declarada(u, nome)) return tp_sim_topo(c, nome);
    for (int32_t i = u->nlocais - 1; i >= 0; i--) {
        if (strcmp(u->locais[i], nome) != 0) continue;
        /* slot reservado por uma LEITURA de global (o `carrega_nome` reserva
         * e resolve rodando), ou célula ainda vazia (o CELL_GET_NAME lê o
         * global): enquanto nada foi gravado aqui, o que vale é o global */
        if (!u->certo[i] && u->sim[i].estado == 0) {
            SimInfo *t = tp_sim_topo(c, nome);
            if (t) return t;
        }
        return &u->sim[i];
    }
    for (Unidade *q = u->pai; q && !q->eh_modulo; q = q->pai)
        for (int32_t i = q->nlocais - 1; i >= 0; i--)
            if (strcmp(q->locais[i], nome) == 0) return &q->sim[i];
    return tp_sim_topo(c, nome);
}

/* O SimInfo é um dos globais colhidos antes de compilar? Esses valem pro
 * arquivo inteiro: não se põem de lado nem se restauram por laço. */
static int tp_eh_topo(C *c, const SimInfo *s)
{
    /* pelo endereço: o SimInfo de um global colhido mora DENTRO do vetor
     * `topo` — varrer o vetor a cada pergunta era mais um custo por nome */
    if (!c->ntopo || !s) return 0;
    const char *ini = (const char *)&c->topo[0].s;
    const char *fim = ini + (size_t)c->ntopo * sizeof(c->topo[0]);
    const char *q = (const char *)s;
    return q >= ini && q < fim && (size_t)(q - ini) % sizeof(c->topo[0]) == 0;
}

/* `for each x` e `[... for each x in ...]` com um `x` que já existe: sem tipo
 * escrito, a variável do laço SOMBREIA a de fora (é outra, e a de fora volta
 * no fim) — o tipo dela sai de cena aqui. Declarada (`str x`), é a MESMA
 * variável, e o tipo dela vale em cada volta. 1 = o de fora foi posto de
 * lado em `fora`, e volta com `tp_sombra_devolve`. */
static int tp_sombra_poe(C *c, Unidade *u, const char *nome, SimInfo *fora)
{
    memset(fora, 0, sizeof(*fora));
    SimInfo *s = tp_sim_de(c, u, nome);
    if (!s || tp_eh_topo(c, s) || s->estado == 2) return 0;
    *fora = *s;
    memset(s, 0, sizeof(*s));
    return 1;
}

/* Depois do laço: o tipo de fora volta, e a gravação que devolve o valor de
 * fora não confere nada (é o valor que já estava lá). */
static void tp_sombra_devolve(C *c, Unidade *u, const char *nome, int posto, const SimInfo *fora)
{
    memset(&c->grava, 0, sizeof(c->grava));
    if (!posto) return;
    SimInfo *s = tp_sim_de(c, u, nome);
    if (s && !tp_eh_topo(c, s)) *s = *fora;
    c->grava.tem_valor = 1;
    c->grava.tipo = fora->estado ? fora->tipo : "Null";
}

/* Os nomes que `n` liga NESTE escopo. O nome de uma funct aninhada conta; o
 * corpo dela e o da lambda são outro escopo. Cada forma que liga nome entra
 * — a mesma lista do `liga_o_nome`, mais o que o compilador liga sozinho
 * (`e` do catch sem nome, `self`/`_count`/`_index`/`_match` do `count each`). */
static void tp_junta_ligados(C *c, PSNode *n, char ***v, int32_t *cnt, int32_t *cap)
{
    if (!n || fundo_demais(c, n, 1)) return;
    while (n->kind == N_BINARY_OP) {      /* cadeia `a + b + …` em laço (ver liga_o_nome) */
        tp_junta_ligados(c, n->b, v, cnt, cap);
        n = n->a;
        if (!n) return;
    }
    switch (n->kind) {
        case N_ASSIGNMENT: case N_VAR_DECL: case N_FOR_EACH_STMT: case N_LIST_COMP:
        case N_USING_STMT:
            junta_nomes(c, v, cnt, cap, n->texto);
            break;
        case N_ENTITY_DECL: case N_MODEL_DECL: case N_ENUM_DECL:
            /* o NOME da classe é do escopo; os métodos e campos dela não —
             * `m()` solto num método com `funct m(self)` é NameError rodando,
             * e com os métodos na lista o `--check` dizia ok */
            junta_nomes(c, v, cnt, cap, n->texto);
            return;
        case N_CATCH_CLAUSE:
            junta_nomes(c, v, cnt, cap, n->texto ? n->texto : "e");
            break;
        case N_MATCH_PATTERN:
            junta_nomes(c, v, cnt, cap, n->texto);
            junta_nomes(c, v, cnt, cap, n->texto2);
            break;
        case N_COUNT_EACH_STMT:
            junta_nomes(c, v, cnt, cap, "self");
            junta_nomes(c, v, cnt, cap, "_count");
            junta_nomes(c, v, cnt, cap, "_index");
            junta_nomes(c, v, cnt, cap, "_match");
            break;
        case N_UNPACK_TARGET:
            for (int32_t i = 0; i < n->lista.n; i++)
                if (n->lista.itens[i] && n->lista.itens[i]->kind == N_NAME)
                    junta_nomes(c, v, cnt, cap, n->lista.itens[i]->texto);
            break;
        case N_FUNCT_DECL:
            junta_nomes(c, v, cnt, cap, n->texto);
            return;                          /* o corpo é outro escopo */
        case N_LAMBDA_EXPR:
            return;
        case N_IMPORT_STMT:
            junta_nomes_do_import(c, n, v, cnt, cap);
            return;
        default:
            break;
    }
    tp_junta_ligados(c, n->a, v, cnt, cap);
    tp_junta_ligados(c, n->b, v, cnt, cap);
    tp_junta_ligados(c, n->c, v, cnt, cap);
    tp_junta_ligados(c, n->e, v, cnt, cap);
    for (int32_t i = 0; i < n->lista.n; i++)  tp_junta_ligados(c, n->lista.itens[i], v, cnt, cap);
    for (int32_t i = 0; i < n->lista2.n; i++) tp_junta_ligados(c, n->lista2.itens[i], v, cnt, cap);
}

/* `global x` em qualquer funct do arquivo: a escrita cria o nome no arquivo. */
static void tp_junta_globais(C *c, PSNode *n)
{
    if (!n || fundo_demais(c, n, 1)) return;
    while (n->kind == N_BINARY_OP) {      /* cadeia `a + b + …` em laço (ver liga_o_nome) */
        tp_junta_globais(c, n->b);
        n = n->a;
        if (!n) return;
    }
    if (n->kind == N_GLOBAL_STMT)
        for (int32_t i = 0; i < n->lista.n; i++) {
            junta_nomes(c, &c->ligados_mod, &c->nligados_mod, &c->cap_ligados_mod, n->lista.itens[i]->texto);
            junta_nomes(c, &c->ligados_mod_raso, &c->nligados_mod_raso, &c->cap_ligados_mod_raso,
                        n->lista.itens[i]->texto);
        }
    tp_junta_globais(c, n->a);
    tp_junta_globais(c, n->b);
    tp_junta_globais(c, n->c);
    tp_junta_globais(c, n->e);
    for (int32_t i = 0; i < n->lista.n; i++)  tp_junta_globais(c, n->lista.itens[i]);
    for (int32_t i = 0; i < n->lista2.n; i++) tp_junta_globais(c, n->lista2.itens[i]);
}

static int tp_na_lista(char **v, int32_t n, const char *nome)
{
    for (int32_t i = 0; i < n; i++)
        if (strcmp(v[i], nome) == 0) return 1;
    return 0;
}

/* O programa liga o nome no escopo que se ENXERGA daqui? A própria funct (o
 * corpo inteiro), as que a envolvem, o arquivo (atribuição, funct, import) e
 * o campo `static` da classe em compilação. Da funct de fora (e do arquivo)
 * vale só o que está vivo no ponto em que esta foi escrita: o nível de cima
 * dela (parâmetros e células nascem antes do corpo) e os blocos abertos em
 * volta — o nome ligado lá só num bloco que não envolve esta funct é do
 * bloco, e daqui não se enxerga (decisão 4 do Plano 6; doc 4.6.1). A funct
 * nem o captura (`resolve_upval` só acha o vivo), e rodando ele seria
 * NameError ou, no arquivo, a global do bloco enquanto ele dura. Não conta o
 * que a VM liga sozinha. `c->nome_oculto` diz se o nome só existe assim. */
static int tp_ligado_no_escopo(C *c, Unidade *u, const char *nome)
{
    c->nome_oculto = 0;
    if (!u->eh_modulo) {
        if (tp_na_lista(u->ligados, u->nligados, nome)) return 1;
        Unidade *k = u;
        for (Unidade *q = u->pai; q && !q->eh_modulo; k = q, q = q->pai) {
            if (!tp_na_lista(q->ligados, q->nligados, nome)) continue;
            if (slot_existente(q, nome) >= 0) { c->nome_oculto = 0; return 1; }
            c->nome_oculto = 1;
        }
        Unidade *m = (k->pai && k->pai->eh_modulo) ? k->pai : k->modulo;
        if (ix_busca(&c->ix_ligados_mod, nome) >= 0) {
            if (!m || ix_busca(&c->ix_ligados_mod_raso, nome) >= 0
                    || celula_ativa_do_topo(m, nome) >= 0 || nome_ja_existe(m, nome)) {
                c->nome_oculto = 0;
                return 1;
            }
            c->nome_oculto = 1;
        }
    } else if (ix_busca(&c->ix_ligados_mod, nome) >= 0) return 1;
    if (c->entity_no && campo_estatico_da_classe(c, nome)) { c->nome_oculto = 0; return 1; }
    return 0;
}

/* O nome lido existe em algum lugar? O escopo (acima), o que a VM liga
 * sozinha e o que o jinker injeta. Nome que nada disso liga é `NameError`
 * certo rodando — e sai antes: uma `int funct` engolia esse erro e o programa
 * terminava calado, com `rc=0`, sem nunca ter feito o que devia. */
/* O nome não é ligado por nada: nem o escopo, nem a VM, nem o jinker. */
static int tp_nome_inexistente(C *c, Unidade *u, const char *nome)
{
    if (tp_ligado_no_escopo(c, u, nome)) return 0;
    /* Nome de TIPO só existe como nome se a VM o liga: o parser faz do tipo
     * sozinho uma carga de tipo (`post(JSON)` é o `dict`), mas seguido de `(`
     * ou `.` ele é nome comum, e rodando o apelido não existe (`JSON.parse`,
     * `String.upper`, `char(1)` davam NameError rodando e passavam aqui,
     * porque a pergunta era "está na tabela de tipos?"). */
    if (ps_nome_pre_ligado(nome)) return 0;
    /* O `request` da jinker só existe DENTRO de um handler de rota (motor-6):
     * a VM o injeta no slot global quando o servidor sobe, e a doc diz
     * "disponível automaticamente dentro de qualquer rota". Fora da rota o
     * `--check` aprovava e rodando dava `NameError: name 'request' is not
     * defined`. O `channel` saiu com o WebSocket da jinker no 19.0.0. */
    if (c->usa_jinker && c->em_handler > 0 && !strcmp(nome, "request")) return 0;
    return 1;
}

/* O NameError de `nome`, com o nome parecido que EXISTE aqui — do escopo, do
 * arquivo ou dos que a VM liga sozinha (`__name` -> `__name__`, `vlaor` ->
 * `valor`). Sem a dica a frase so dizia que nao existe, e o erro de digitacao
 * ficava por conta de quem le. Com `palavras`, as palavras da linguagem entram
 * entre os candidatos: e o nome solto no comeco do comando (`retrun x` ->
 * 'return', `whlie` -> 'while'), onde uma palavra-chave cabe. */
/* Lib do motor usada sem `import` (`Parsing.integer`, `json.parse`): a causa
 * é o import que falta, e a frase diz qual. */
static void tp_acusa_sem_import(C *c, PSNode *onde, const char *nome)
{
    terro(c, onde, "NameError", "name '%s' is not defined. Did you forget to import '%s'?", nome, nome);
}

/* O nome existe, mas só dentro de um bloco que não envolve a funct que o lê
 * (`c->nome_oculto`): a causa é essa, e a frase diz como enxergar. */
static void tp_acusa_oculto(C *c, PSNode *onde, const char *nome)
{
    terro(c, onde, "NameError", "name '%s' is not defined: '%s' só existe dentro de um bloco, "
          "e esta funct foi escrita fora dele (declare '%s' antes do bloco)", nome, nome, nome);
}

static void tp_junta_membros_classe(C *c, const char *classe, const char **v, int *cnt, int prof);

/* A classe do `self` que o código em `u` alcança: o método da classe em
 * compilação, ou a funct e a lambda escritas dentro dele. NULL fora de
 * método, e no método `static` (não tem `self`). */
static const char *tp_classe_do_self(C *c, Unidade *u)
{
    if (!c->entity_no || !c->entity_no->texto) return NULL;
    for (Unidade *q = u; q; q = q->pai)
        for (int32_t i = 0; i < q->nlocais; i++) {
            if (strcmp(q->locais[i], "self") != 0) continue;
            const char *t = q->sim ? q->sim[i].tipo : NULL;
            return (t && !strcmp(t, c->entity_no->texto)) ? t : NULL;
        }
    return NULL;
}

/* `nome` é membro de instância da classe (campo, método, `self.x` gravado,
 * os dos pais)? O membro `static` não chega aqui: o nome solto já o alcança. */
static int tp_membro_da_classe(C *c, const char *classe, const char *nome)
{
    const char *v[256];
    int cnt = 0;
    tp_junta_membros_classe(c, classe, v, &cnt, 0);
    for (int i = 0; i < cnt; i++)
        if (v[i] && !strcmp(v[i], nome)) return 1;
    return 0;
}

static void tp_acusa_nome(C *c, Unidade *u, const char *nome, PSNode *onde, int palavras)
{
    if (!tp_ligado_no_escopo(c, u, nome) && c->nome_oculto) { tp_acusa_oculto(c, onde, nome); return; }
    if (ps_nativo_eh_modulo(nome)) { tp_acusa_sem_import(c, onde, nome); return; }
    /* Membro de instância lido pelo nome solto, dentro do método: só se
     * alcança por `self`, e a dica é essa. Saía o nome parecido de outro
     * lugar (`saldo` -> 'False', `m` -> 'A'), que não conserta nada. */
    const char *cls = tp_classe_do_self(c, u);
    if (cls && tp_membro_da_classe(c, cls, nome)) {
        terro(c, onde, "NameError", "name '%s' is not defined. Did you mean: 'self.%s'?", nome, nome);
        return;
    }
    const char *dica = NULL;
    const char *const *kw = palavras ? ps_lexer_keywords() : NULL;
    const char *const *mp = palavras ? ps_parser_mods_de_posicao() : NULL;
    int32_t tot = c->nligados_mod + 1;
    for (Unidade *q = u; q; q = q->pai) tot += q->nligados;
    for (int i = 0; kw && kw[i]; i++) tot++;
    for (int i = 0; mp && mp[i]; i++) tot++;
    const char **cands = malloc(sizeof(char *) * (size_t)tot);
    if (cands) {
        int n = 0;
        for (Unidade *q = u; q; q = q->pai)
            for (int32_t i = 0; i < q->nligados; i++) cands[n++] = q->ligados[i];
        for (int32_t i = 0; i < c->nligados_mod; i++) cands[n++] = c->ligados_mod[i];
        const char *pre = ps_sugere_pre_ligado(nome);
        if (pre) cands[n++] = pre;
        for (int i = 0; kw && kw[i]; i++) cands[n++] = kw[i];
        for (int i = 0; mp && mp[i]; i++) cands[n++] = mp[i];
        dica = ps_sugere_nome(nome, cands, n);
        /* a dica aponta pra memoria de quem vive ate o fim da compilacao
         * (listas de nomes / tabelas fixas); o vetor em si morre aqui */
        free(cands);
    }
    if (dica)
        terro(c, onde, "NameError", "name '%s' is not defined. Did you mean: '%s'?", nome, dica);
    else
        terro(c, onde, "NameError", "name '%s' is not defined", nome);
}

/* `x = 10 // comentario`: nome que não existe à direita de um `//` é
 * comentário escrito do jeito antigo (antes do I11 o `//` comentava). A frase
 * é a do `//` no começo da linha, no trecho do `//` (doc 1.2: o erro diz isso
 * com todas as letras). Saía o NameError do nome, ou "dois comandos na mesma
 * linha" quando o comentário tinha mais de uma palavra. */
static void tp_acusa_barra2(C *c, PSNode *op)
{
    terro(c, op, "SyntaxError", "'//' e divisao inteira, nao comentario — comentario e '#' "
                                "(ou bloco entre tres aspas)");
}

/* O `//` da ponta direita de `v` cujo lado direito é o nome em (l, col) — o
 * nome solto de `1 // um comentario aqui` —, ou NULL. */
static PSNode *tp_barra2_com_direita(PSNode *v, int32_t l, int32_t col)
{
    while (v && v->kind == N_BINARY_OP) {
        if (v->texto && !strcmp(v->texto, "//") && v->b && v->b->kind == N_NAME
                && v->b->line == l && v->b->col == col) return v;
        v = v->b;
    }
    return NULL;
}

/* O lado direito do binário `op`, com a regra do `//` acima: o nome que não
 * existe sai com a frase do comentário, e o código fica inteiro (o load do
 * nome sai do mesmo jeito; quem não roda é o programa, pelo erro). */
static void expr_direito(C *c, Unidade *u, PSNode *op)
{
    PSNode *b = op->b;
    if (b && b->kind == N_NAME && b->texto && op->texto && !strcmp(op->texto, "//")
            && !c->nomes_incertos && tp_nome_inexistente(c, u, b->texto)) {
        tp_acusa_barra2(c, op);
        carrega_nome(c, u, b->texto);
        return;
    }
    expr(c, u, b);
}

static void tp_confere_nome(C *c, Unidade *u, const char *nome, PSNode *onde)
{
    if (!nome || !*nome || c->nomes_incertos) return;
    if (!tp_nome_inexistente(c, u, nome)) return;
    tp_acusa_nome(c, u, nome, onde, 0);
}

/* `retrun x`, `whlie i < 3 {`, `const int x = 1`, `Conta c`, `Conta c = new
 * Conta()`, `post x`: o primeiro comando da linha termina num nome solto e
 * outro começa logo depois. O parser marcou (ver `marca_nome_solto`: `a` = o
 * nome, `e` = onde começa o segundo, `i2` = 0 se o nome é o comando inteiro)
 * e pulou o resto da linha. A causa costuma estar no nome, e só aqui se sabe o
 * que ele é:
 *   - não existe: o NameError dele — no começo do comando, com as palavras da
 *     linguagem na dica (`retrun` -> 'return');
 *   - é o comando inteiro e é um tipo: a declaração sem `=` (`Conta c`, como
 *     `int x`);
 *   - existe: são mesmo dois comandos colados.
 * Sai erro SEMPRE: o resto da linha não foi lido, e o programa não pode rodar
 * sem ele. */
static void tp_nome_solto(C *c, Unidade *u, PSNode *n)
{
    const char *nome = n->a->texto;
    PSNode *seg = n->e;
    int no_comeco = (n->i2 == 0);
    if (nome && *nome && !c->nomes_incertos && tp_nome_inexistente(c, u, nome)) {
        PSNode *barra2 = n->b ? tp_barra2_com_direita(n->b->a, n->a->line, n->a->col) : NULL;
        if (barra2) { tp_acusa_barra2(c, barra2); return; }
        tp_acusa_nome(c, u, nome, n->a, no_comeco);
        return;
    }
    if (no_comeco && seg->i2 && nome && tp_tipo_existe(c, u, nome)) {
        cerro_sx(c, seg->a ? seg->a : seg, "declaracao de variavel exige '='");
        return;
    }
    cerro_sx(c, seg, "dois comandos na mesma linha: separe com ';' ou quebre a linha");
}

/* O programa liga `nome` a alguma coisa? Slot reservado só pela leitura
 * (o `carrega_nome` reserva um pra resolver rodando) não conta: sem nada
 * gravado, o nome é o builtin — `post` continua sendo o `post`. */
static int tp_nome_ligado(C *c, Unidade *u, const char *nome)
{
    SimInfo *s = tp_sim_de(c, u, nome);
    return s && (s->estado || s->decl || s->mod_nativo || s->mod_pr);
}

/* ── o tipo de uma expressão ── */
/* O primeiro `yield` do corpo, sem entrar nas functs de dentro: é o que faz
 * da funct um gerador. NULL = não tem. */
static PSNode *tp_acha_yield(PSNode *n)
{
    if (!n || ps_pilha_apertada()) return NULL;   /* fundo demais: quem acusa e o stmt_no */
    if (n->kind == N_YIELD_STMT) return n;
    if (n->kind == N_FUNCT_DECL || n->kind == N_LAMBDA_EXPR) return NULL;
    PSNode *y;
    if ((y = tp_acha_yield(n->a)) || (y = tp_acha_yield(n->b))
            || (y = tp_acha_yield(n->c)) || (y = tp_acha_yield(n->e))) return y;
    for (int32_t i = 0; i < n->lista.n; i++)  if ((y = tp_acha_yield(n->lista.itens[i]))) return y;
    for (int32_t i = 0; i < n->lista2.n; i++) if ((y = tp_acha_yield(n->lista2.itens[i]))) return y;
    return NULL;
}

static int tp_tem_yield(PSNode *n)
{
    return tp_acha_yield(n) != NULL;
}

/* O retorno que a funct DECLARA — e só quando a chamada devolve esse valor:
 * gerador devolve o gerador, `async` devolve o future. */
static const char *tp_ret_decl(PSNode *decl)
{
    if (!decl || decl->kind != N_FUNCT_DECL || !decl->texto2) return NULL;
    /* O que a CHAMADA devolve não é o tipo escrito (motor-17): a `async funct`
     * devolve `future` e a funct com `yield` devolve `generator`, e o
     * `--check` aceitava `int x = f()` e `str s = gen()` e o motor recusava
     * rodando. O tipo escrito é o do `await` e o do item do gerador. */
    if (decl->is_async) return "future";
    if (tp_tem_yield(decl->b)) return "generator";
    return tp_canon(decl->texto2);
}

/* O que a tabela MEDIDA diz do nativo; `*` (tipo do conteúdo) é desconhecido. */
static const char *tp_nativo(const char *dono, const char *membro)
{
    const char *r = ps_retorno_de(dono, membro);
    if (!r || strcmp(r, "*") == 0) return NULL;
    return r;
}

/* O método `nome` da Entity `classe` (ou de um pai), ou NULL. */
static PSNode *tp_metodo(C *c, const char *classe, const char *nome, int prof)
{
    PSNode *d = tp_tipo_arq(c, classe);
    if (!d || d->kind != N_ENTITY_DECL || prof > 32) return NULL;
    for (int32_t i = 0; i < d->lista.n; i++) {
        PSNode *m = d->lista.itens[i];
        if (m->kind == N_FUNCT_DECL && m->texto && strcmp(m->texto, nome) == 0) return m;
    }
    for (int32_t i = 0; i < d->lista2.n; i++) {
        PSNode *r = d->lista2.itens[i]->texto ? tp_metodo(c, d->lista2.itens[i]->texto, nome, prof + 1) : NULL;
        if (r) return r;
    }
    return NULL;
}

/* O método `nome` foi decorado por decorador geral (que pode trocá-lo)? */
static int tp_metodo_decorado(C *c, const char *classe, PSNode *met)
{
    PSNode *d = tp_tipo_arq(c, classe);
    if (!d || d->kind != N_ENTITY_DECL) return 0;
    for (int32_t i = 1; i < d->lista.n; i++) {
        if (!mesmo_no(d->lista.itens[i], met)) continue;
        PSNode *ant = d->lista.itens[i - 1];
        if (ant->kind != N_DECORATOR_STMT || !ant->a) return 0;
        const char *dn = ant->a->lista.n == 1 ? ant->a->lista.itens[0]->texto : NULL;
        return !(dn && (!strcmp(dn, "static") || !strcmp(dn, "NonNull") || !strcmp(dn, "dataentity")));
    }
    return 0;
}

/* `tp_metodo` que diz também a classe dona do método achado. */
static PSNode *tp_metodo_dono(C *c, const char *classe, const char *nome, int prof, const char **dono)
{
    PSNode *d = tp_tipo_arq(c, classe);
    if (!d || d->kind != N_ENTITY_DECL || prof > 32) return NULL;
    for (int32_t i = 0; i < d->lista.n; i++) {
        PSNode *m = d->lista.itens[i];
        if (m->kind == N_FUNCT_DECL && m->texto && strcmp(m->texto, nome) == 0) { *dono = d->texto; return m; }
    }
    for (int32_t i = 0; i < d->lista2.n; i++) {
        PSNode *r = d->lista2.itens[i]->texto
                  ? tp_metodo_dono(c, d->lista2.itens[i]->texto, nome, prof + 1, dono) : NULL;
        if (r) return r;
    }
    return NULL;
}

/* Sobrescrita que troca o tipo de retorno: o checador tipa `p.m()` pelo
 * retorno que o PAI declara quando `p` é declarado como o pai, e a filha que
 * devolve outra coisa (ou não declara o retorno) quebrava rodando, num ponto
 * que o `--check` aprovou. O método da filha com o nome de um método do pai
 * que declara o retorno tem que declarar um retorno que caiba nele (`Cao`
 * cabe em `Animal`). Erro na declaração da filha. Fica de fora o que não se
 * sabe: pai importado, método decorado (o decorador pode trocá-lo), `private`
 * do pai (a filha não o enxerga: o dela é outro método), `async`/gerador e
 * o `__init__`. O primeiro pai que tem o método decide, como na chamada. */
static void tp_confere_sobrescrita(C *c, PSNode *cls)
{
    if (!cls || cls->kind != N_ENTITY_DECL || !cls->texto) return;
    for (int32_t i = 0; i < cls->lista.n; i++) {
        PSNode *m = cls->lista.itens[i];
        if (!m || m->kind != N_FUNCT_DECL || !m->texto || strcmp(m->texto, "__init__") == 0) continue;
        if (m->is_async || tp_tem_yield(m->b) || tp_metodo_decorado(c, cls->texto, m)) continue;
        for (int32_t k = 0; k < cls->lista2.n; k++) {
            const char *pn = cls->lista2.itens[k]->texto;
            const char *dono = NULL;
            PSNode *pm = pn ? tp_metodo_dono(c, pn, m->texto, 0, &dono) : NULL;
            if (!pm) continue;
            const char *pt = tp_ret_decl(pm);
            if (!pt || pm->is_private || tp_metodo_decorado(c, dono, pm)) break;
            PSNode tmp;
            if (!m->texto2)
                terro(c, no_do_nome(m, &tmp), "TypeError",
                      "%s() de %s nao declara o retorno e sobrescreve %s() de %s, que devolve %s: declare %s funct %s",
                      m->texto, cls->texto, m->texto, dono, pt, pt, m->texto);
            else if (tp_aceita(c, pt, tp_canon(m->texto2), NULL, 1) == T_NAO)
                terro(c, no_do_nome(m, &tmp), "TypeError",
                      "%s() de %s devolve %s e sobrescreve %s() de %s, que devolve %s",
                      m->texto, cls->texto, tp_canon(m->texto2), m->texto, dono, pt);
            break;
        }
    }
}

/* Tipo escrito do campo `nome` da Entity (ou de um pai), ou NULL. */
static PSNode *tp_campo_decl_em(PSNode *n, const char *nome)
{
    if (!n) return NULL;
    if (n->kind == N_FIELD_DECL && n->texto && n->texto2 && strcmp(n->texto, nome) == 0) return n;
    if (n->kind == N_LAMBDA_EXPR) return NULL;
    PSNode *r;
    if ((r = tp_campo_decl_em(n->a, nome)) || (r = tp_campo_decl_em(n->b, nome))
            || (r = tp_campo_decl_em(n->c, nome)) || (r = tp_campo_decl_em(n->e, nome))) return r;
    for (int32_t i = 0; i < n->lista.n; i++)  if ((r = tp_campo_decl_em(n->lista.itens[i], nome))) return r;
    for (int32_t i = 0; i < n->lista2.n; i++) if ((r = tp_campo_decl_em(n->lista2.itens[i], nome))) return r;
    return NULL;
}

static const char *tp_campo_tipo(C *c, const char *classe, const char *nome, int prof)
{
    PSNode *d = tp_tipo_arq(c, classe);
    if (!d || d->kind != N_ENTITY_DECL || prof > 32) return NULL;
    for (int32_t i = 0; i < d->lista2_alias.n; i++) {
        PSNode *f = d->lista2_alias.itens[i];
        if (f->kind == N_ENTITY_FIELD && f->texto && f->texto2 && strcmp(f->texto, nome) == 0)
            return tp_canon(f->texto2);
    }
    for (int32_t i = 0; i < d->lista.n; i++) {
        PSNode *fd = tp_campo_decl_em(d->lista.itens[i], nome);
        if (fd) return tp_canon(fd->texto2);
    }
    for (int32_t i = 0; i < d->lista2.n; i++) {
        const char *t = d->lista2.itens[i]->texto ? tp_campo_tipo(c, d->lista2.itens[i]->texto, nome, prof + 1) : NULL;
        if (t) return t;
    }
    return NULL;
}

/* A Entity tem o membro? Método, campo do corpo, `private <tipo> x` e
 * `self.x = ...` de qualquer método, e os dos pais. 1 = tem; 0 = não tem;
 * -1 = um pai não é do arquivo, não dá pra saber. */
static int tp_self_grava(PSNode *n, const char *nome)
{
    if (!n) return 0;
    /* funct aninhada no método também grava no `self` (pela captura) */
    if (n->kind == N_MEMBER_ASSIGNMENT && n->texto && strcmp(n->texto, nome) == 0) return 1;
    /* alvo de desempacotamento `self.a, self.b = ...` chega como acesso */
    if (n->kind == N_UNPACK_TARGET) {
        for (int32_t i = 0; i < n->lista.n; i++) {
            PSNode *x = n->lista.itens[i];
            if (x && x->kind == N_MEMBER_ACCESS && x->texto && strcmp(x->texto, nome) == 0) return 1;
        }
    }
    if (tp_self_grava(n->a, nome) || tp_self_grava(n->b, nome) || tp_self_grava(n->c, nome)
            || tp_self_grava(n->e, nome)) return 1;
    for (int32_t i = 0; i < n->lista.n; i++)  if (tp_self_grava(n->lista.itens[i], nome)) return 1;
    for (int32_t i = 0; i < n->lista2.n; i++) if (tp_self_grava(n->lista2.itens[i], nome)) return 1;
    return 0;
}

static int tp_classe_tem(C *c, const char *classe, const char *nome, int prof)
{
    PSNode *d = tp_tipo_arq(c, classe);
    if (!d || d->kind != N_ENTITY_DECL || prof > 32) return -1;
    for (int32_t i = 0; i < d->lista2_alias.n; i++)
        if (d->lista2_alias.itens[i]->texto && strcmp(d->lista2_alias.itens[i]->texto, nome) == 0) return 1;
    for (int32_t i = 0; i < d->lista.n; i++) {
        PSNode *m = d->lista.itens[i];
        if (m->kind == N_FUNCT_DECL && m->texto && strcmp(m->texto, nome) == 0) return 1;
        if (m->kind == N_FUNCT_DECL && (tp_self_grava(m->b, nome) || tp_campo_decl_em(m->b, nome))) return 1;
    }
    int incerto = 0;
    for (int32_t i = 0; i < d->lista2.n; i++) {
        const char *p = d->lista2.itens[i]->texto;
        if (!p) continue;
        int r = tp_classe_tem(c, p, nome, prof + 1);
        if (r == 1) return 1;
        if (r == -1) incerto = 1;
    }
    return incerto ? -1 : 0;
}

/* ── `private` antes de rodar: a regra da VM (`priv_barra`) sobre a árvore ──
 *
 * A declaração que o nome ALCANÇA decide: a primeira classe, na ordem de
 * busca de membro (a própria, depois os pais da esquerda pra direita, em
 * profundidade), que declara o nome. Pública libera; private só libera o
 * código escrito numa classe que declara o nome (ou numa de fora dela, a
 * cadeia `c->corpo`) quando o valor descende dela. O checador só acusa o
 * que a VM barraria com certeza; o que depende de pai importado fica pra
 * rodando. */

/* A classe que declara `nome` a partir de `cls`. NULL com `*incerto` = 0:
 * ninguém declara; com 1: a busca passou por pai que o arquivo não declara. */
static PSNode *tp_classe_que_declara(C *c, PSNode *cls, const char *nome, int32_t *marcas,
                                     int *incerto, int prof)
{
    if (!cls || cls->kind != N_ENTITY_DECL || prof > 1000) { *incerto = 1; return NULL; }
    if (classe_declara_no(cls, nome, marcas)) return cls;
    for (int32_t i = 0; i < cls->lista2.n; i++) {
        const char *p = cls->lista2.itens[i] ? cls->lista2.itens[i]->texto : NULL;
        PSNode *r = tp_classe_que_declara(c, p ? tp_tipo_arq(c, p) : NULL, nome, marcas, incerto, prof + 1);
        if (r || *incerto) return r;
    }
    return NULL;
}

/* O código em compilação acessando `nome` num valor da classe `alvo`:
 * 0 = liberado, 1 = barrado (`*dona` = a classe que declara o private),
 * -1 = não dá pra saber. */
static int tp_priv_barra(C *c, PSNode *alvo, const char *nome, PSNode **dona)
{
    int32_t mk = 0;
    int incerto = 0;
    PSNode *d = tp_classe_que_declara(c, alvo, nome, &mk, &incerto, 0);
    if (incerto) return -1;
    if (!d || !(mk & PS_MEMB_PRIVADO)) return 0;
    for (CorpoClasse *k = c->corpo; k; k = k->fora) {
        if (!k->no || !k->no->texto || !classe_declara_no(k->no, nome, NULL)) continue;
        int h = (alvo->texto) ? tp_herda(c, alvo->texto, k->no->texto, 0) : -1;
        if (h == 1) return 0;
        if (h == -1) return -1;
    }
    *dona = d;
    return 1;
}

/* A expressão É uma classe (não instância dela)? A declaração, ou NULL: o
 * nome ligado à classe (inclusive o trazido por `from m import C`) e
 * `modulo.Classe` de um `.pr` importado. Classe decorada fica de fora (o
 * decorador pode ter trocado o valor). */
static PSNode *tp_classe_exata(C *c, Unidade *u, PSNode *e)
{
    if (!e) return NULL;
    if (e->kind == N_NAME && e->texto) {
        SimInfo *s = tp_sim_de(c, u, e->texto);
        return (s && s->decl && s->decl->kind == N_ENTITY_DECL && !s->decorado) ? s->decl : NULL;
    }
    if (e->kind == N_MEMBER_ACCESS && e->texto) {
        SimInfo *sm = tp_sim_mod_pr(c, u, e->a);
        if (!sm) return NULL;
        int deco = 0;
        PSNode *d = modulo_decl(sm->mod_pr, e->texto, &deco);
        return (d && d->kind == N_ENTITY_DECL && !deco) ? d : NULL;
    }
    return NULL;
}

/* Acusa o acesso a `nome` num valor de classe `alvo`. `exato`: o valor é a
 * própria classe (`Conta.x`, `base(Pai).x`); senão é instância de `alvo` ou
 * de uma classe que descende dela, e só se acusa quando TODA classe do
 * arquivo que o valor pode ser barra o acesso (uma filha que redeclara o
 * nome como público o libera rodando). `onde` é a posição do erro. */
static void tp_confere_privado(C *c, PSNode *alvo, const char *nome, int exato, PSNode *onde)
{
    if (!alvo || alvo->kind != N_ENTITY_DECL || !alvo->texto || !nome) return;
    PSNode *dona = NULL;
    if (tp_priv_barra(c, alvo, nome, &dona) != 1) return;
    if (!exato) {
        for (int32_t i = 0; i < c->ntipos_arq; i++) {
            PSNode *s = c->tipos_arq[i];
            if (s == alvo || s->kind != N_ENTITY_DECL || !s->texto) continue;
            int h = tp_herda(c, s->texto, alvo->texto, 0);
            if (h == 0) continue;
            PSNode *outra = NULL;
            if (h == -1 || tp_priv_barra(c, s, nome, &outra) != 1) return;
        }
    }
    terro(c, onde, "RuntimeError", "acesso negado: '%s' e private de %s (so acessivel de dentro da classe)",
          nome, dona->texto ? dona->texto : "?");
}

/* O módulo NATIVO que a expressão é — `os` ou `Parsing` (ligados por import)
 * ou `sys.stdout` — escrito em `buf`; NULL = não é módulo. */
static const char *tp_modulo_de(C *c, Unidade *u, PSNode *n, char *buf, size_t cap)
{
    if (!n) return NULL;
    if (n->kind == N_NAME && n->texto) {
        SimInfo *s = tp_nome_ligado(c, u, n->texto) ? tp_sim_de(c, u, n->texto) : NULL;
        if (s) {
            if (!s->mod_nativo || s->membro_nativo) return NULL;
            snprintf(buf, cap, "%s", s->mod_nativo);
            return buf;
        }
        return NULL;
    }
    if (n->kind == N_MEMBER_ACCESS && n->texto) {
        char b2[64];
        const char *m = tp_modulo_de(c, u, n->a, b2, sizeof(b2));
        if (m && strcmp(m, "sys") == 0
                && (!strcmp(n->texto, "stdout") || !strcmp(n->texto, "stderr") || !strcmp(n->texto, "stdin"))) {
            snprintf(buf, cap, "sys.%s", n->texto);
            return buf;
        }
    }
    return NULL;
}

/* O que `x.m(...)` devolve, com `x` do tipo `t` (união: a dos lados que não
 * são Null — chamar em Null é outro erro). */
static const char *tp_ret_metodo(C *c, const char *t, const char *m)
{
    if (!t || !m) return NULL;
    if (strchr(t, '|')) {
        const char *acc = NULL;
        int algum = 0;
        const char *q = t;
        while (q && *q) {
            const char *bar = strchr(q, '|');
            size_t len = bar ? (size_t)(bar - q) : strlen(q);
            char lado[128];
            if (len >= sizeof(lado)) return NULL;
            memcpy(lado, q, len); lado[len] = '\0';
            if (strcmp(lado, "Null") != 0) {
                const char *r = tp_ret_metodo(c, lado, m);
                if (!r) return NULL;
                acc = algum ? tp_uniao(c, acc, r) : r;
                algum = 1;
            }
            q = bar ? bar + 1 : NULL;
        }
        return acc;
    }
    if (tp_tipo_arq(c, t)) {
        PSNode *md = tp_metodo(c, t, m, 0);
        return (md && !tp_metodo_decorado(c, t, md)) ? tp_ret_decl(md) : NULL;
    }
    return ps_nativo_tem_membro(t, m) == 1 ? tp_nativo(t, m) : NULL;
}

static const char *tp_chamada(C *c, Unidade *u, PSNode *n)
{
    PSNode *f = n->a;
    if (!f) return NULL;
    if (f->kind == N_TYPE_NAME) {
        const char *k = tp_canon(f->texto);
        const char *r = k ? tp_nativo("builtins", k) : NULL;
        return r ? r : NULL;
    }
    if (f->kind == N_NAME && f->texto) {
        SimInfo *s = tp_nome_ligado(c, u, f->texto) ? tp_sim_de(c, u, f->texto) : NULL;
        if (s) {
            if (s->membro_nativo) return tp_nativo(s->mod_nativo, s->membro_nativo);
            if (s->decl && !s->decorado) {
                if (s->decl->kind == N_FUNCT_DECL) return tp_ret_decl(s->decl);
                if (s->decl->kind == N_ENTITY_DECL) return s->decl->texto;
            }
            return NULL;
        }
        return tp_nativo("builtins", f->texto);
    }
    if (f->kind == N_MEMBER_ACCESS && f->a && f->texto) {
        char buf[64];
        const char *mod = tp_modulo_de(c, u, f->a, buf, sizeof(buf));
        if (mod) return ps_nativo_tem_membro(mod, f->texto) == 1 ? tp_nativo(mod, f->texto) : NULL;
        {
            SimInfo *sm = tp_sim_mod_pr(c, u, f->a);
            if (sm) {
                int deco = 0;
                PSNode *d = modulo_decl(sm->mod_pr, f->texto, &deco);
                if (!d || deco) return NULL;
                if (d->kind == N_FUNCT_DECL) return tp_ret_decl(d);
                if (d->kind == N_ENTITY_DECL && tp_classe_importada(c, d)) return d->texto;
                return NULL;
            }
        }
        if (f->a->kind == N_NAME && f->a->texto) {
            SimInfo *s = tp_sim_de(c, u, f->a->texto);
            if (s && s->decl && s->decl->kind == N_ENTITY_DECL) {
                PSNode *md = tp_metodo(c, s->decl->texto, f->texto, 0);
                return (md && !tp_metodo_decorado(c, s->decl->texto, md)) ? tp_ret_decl(md) : NULL;
            }
        }
        return tp_ret_metodo(c, tp_de(c, u, f->a), f->texto);
    }
    return NULL;
}

static int tp_numerico(const char *t)
{
    return t && (!strcmp(t, "int") || !strcmp(t, "flo") || !strcmp(t, "bool"));
}

static int tp_sequencia(const char *t)
{
    return t && (!strcmp(t, "str") || !strcmp(t, "list") || !strcmp(t, "tup") || !strcmp(t, "byte"));
}

/* O tipo do resultado de `a op b`, pelas regras da seção 2.8. */
/* O nome que o `type()` da VM dá a um valor do tipo ESCRITO `t`, quando o
 * tipo é um dos que a VM conhece sozinha (número, texto, coleção, Null):
 * `long` é int e `char` é str rodando. NULL pra classe, união e desconhecido
 * — aí só rodando se sabe. */
static const char *tp_basico(const char *t)
{
    if (!t || strchr(t, '|')) return NULL;
    static const char *const B[] = { "int", "flo", "bool", "str", "byte", "list", "tup", "dict", "Null" };
    for (size_t i = 0; i < sizeof(B) / sizeof(B[0]); i++)
        if (!strcmp(t, B[i])) return B[i];
    if (!strcmp(t, "long")) return "int";
    if (!strcmp(t, "char")) return "str";
    return NULL;
}

/* `a op b` pelos tipos ESCRITOS, pela regra do laço da VM (os PS_CASE de
 * ADD a BIT_SHR e o CMP_CORPO): devolve o tipo do resultado, ou NULL quando
 * não dá pra saber. Quando os dois tipos são certos e a VM recusa a conta,
 * `erro` recebe a frase dela, letra por letra, e o checador acusa antes de
 * rodar. Tipo e recusa saem da mesma regra: antes o tipo vinha de uma tabela
 * à parte, que dizia `str` pra `"%d" % 5` e `dict` pra `d | e` — as duas
 * contas são TypeError rodando. */
static const char *tp_op(C *c, const char *op, const char *a, const char *b, char *erro, size_t cap)
{
    if (erro && cap) erro[0] = '\0';
    if (!op) return NULL;
    static const char *const LOGICOS[] = { "is", "is not", "not is", "in", "not in", "and", "&&",
                                           "or", "||", "==", "!=" };
    for (size_t i = 0; i < sizeof(LOGICOS) / sizeof(LOGICOS[0]); i++)
        if (strcmp(op, LOGICOS[i]) == 0) return "bool";
    /* `ba`/`bb`: o nome rodando (long é int, char é str); a recusa só sai
     * quando os DOIS são certos, e decide pelo nome rodando. O tipo do
     * resultado continua decidido pelo tipo escrito, como sempre foi. */
    const char *ba = tp_basico(a), *bb = tp_basico(b);
    int certos = ba && bb && erro && cap;
    int na = tp_numerico(a), nb = tp_numerico(b);
    int num_rodando = tp_numerico(ba) && tp_numerico(bb);
    int int_a = ba && (!strcmp(ba, "int") || !strcmp(ba, "bool"));
    int int_b = bb && (!strcmp(bb, "int") || !strcmp(bb, "bool"));
    int ordenaveis_iguais = ba && bb && !strcmp(ba, bb)
        && (!strcmp(ba, "str") || !strcmp(ba, "byte") || !strcmp(ba, "list") || !strcmp(ba, "tup"));
    if (!strcmp(op, "<") || !strcmp(op, ">") || !strcmp(op, "<=") || !strcmp(op, ">=")) {
        if (certos && !num_rodando && !ordenaveis_iguais)
            snprintf(erro, cap, "'%s' not supported between instances of '%s' and '%s'", op, ba, bb);
        return "bool";
    }
    if (!a || !b || strchr(a, '|') || strchr(b, '|')) return NULL;
    int flo = !strcmp(a, "flo") || !strcmp(b, "flo");
    int inteiro_b = !strcmp(b, "int") || !strcmp(b, "bool");
    int inteiro_a = !strcmp(a, "int") || !strcmp(a, "bool");
    int seq_a = ba && (!strcmp(ba, "str") || !strcmp(ba, "byte") || !strcmp(ba, "list") || !strcmp(ba, "tup"));
    int seq_b = bb && (!strcmp(bb, "str") || !strcmp(bb, "byte") || !strcmp(bb, "list") || !strcmp(bb, "tup"));
    if (!strcmp(op, "+")) {
        if (na && nb) return flo ? "flo" : "int";
        if (!strcmp(a, b) && tp_sequencia(a)) return a;
        if (certos && !num_rodando && !ordenaveis_iguais) {
            if (!strcmp(ba, "str") || !strcmp(ba, "list") || !strcmp(ba, "tup"))
                snprintf(erro, cap, "can only concatenate %s (not \"%s\") to %s", ba, bb, ba);
            else if (!strcmp(ba, "byte"))
                snprintf(erro, cap, "can't concat %s to bytes", bb);
            else
                snprintf(erro, cap, "unsupported operand type(s) for +: '%s' and '%s'", ba, bb);
        }
        return NULL;
    }
    if (!strcmp(op, "*")) {
        if (na && nb) return flo ? "flo" : "int";
        if (tp_sequencia(a) && inteiro_b) return a;
        if (tp_sequencia(b) && inteiro_a) return b;
        if (certos && !num_rodando && !(seq_a && int_b) && !(seq_b && int_a)) {
            if (seq_a)      snprintf(erro, cap, "can't multiply sequence by non-int of type '%s'", bb);
            else if (seq_b) snprintf(erro, cap, "can't multiply sequence by non-int of type '%s'", ba);
            else            snprintf(erro, cap, "unsupported operand type(s) for *: '%s' and '%s'", ba, bb);
        }
        return NULL;
    }
    if (!strcmp(op, "-") || !strcmp(op, "//") || !strcmp(op, "%") || !strcmp(op, "/") || !strcmp(op, "**")) {
        if (na && nb) {
            if (!strcmp(op, "/")) return "flo";
            if (!strcmp(op, "**")) return flo ? "flo" : tp_guarda(c, "int|flo");
            return flo ? "flo" : "int";
        }
        if (certos && !num_rodando)
            snprintf(erro, cap, "unsupported operand type(s) for %s: '%s' and '%s'",
                     !strcmp(op, "**") ? "** or pow()" : op, ba, bb);
        return NULL;
    }
    if (!strcmp(op, "&") || !strcmp(op, "|") || !strcmp(op, "^") || !strcmp(op, "<<") || !strcmp(op, ">>")) {
        if (inteiro_a && inteiro_b) return "int";
        if (certos && !(int_a && int_b))
            snprintf(erro, cap, "unsupported operand type(s) for %s: '%s' and '%s'", op, ba, bb);
        return NULL;
    }
    return NULL;
}

static const char *tp_binario(C *c, const char *op, const char *a, const char *b)
{
    return tp_op(c, op, a, b, NULL, 0);
}

/* ── base(): o pai que `base()` / `base(Pai)` designa ──────────────────────
 * `Pai` é ancestral de `classe`? 1 = sim; 0 = não; -1 = um pai não é do
 * arquivo, não dá pra saber (a VM confere rodando). */
static int tp_eh_ancestral(C *c, const char *classe, const char *pai, int prof)
{
    PSNode *d = tp_tipo_arq(c, classe);
    if (!d || d->kind != N_ENTITY_DECL || prof > 32) return -1;
    int incerto = 0;
    for (int32_t i = 0; i < d->lista2.n; i++) {
        const char *q = d->lista2.itens[i]->texto;
        if (!q) continue;
        if (strcmp(q, pai) == 0) return 1;
        int r = tp_eh_ancestral(c, q, pai, prof + 1);
        if (r == 1) return 1;
        if (r == -1) incerto = 1;
    }
    return incerto ? -1 : 0;
}

/* "base(A) ou base(B)" / "A, B" com os pais diretos da Entity. */
static void base_lista_pais(PSNode *ent, char *buf, size_t cap, int com_base)
{
    buf[0] = '\0';
    for (int32_t i = 0; i < ent->lista2.n; i++) {
        const char *q = ent->lista2.itens[i]->texto ? ent->lista2.itens[i]->texto : "?";
        size_t k = strlen(buf);
        if (k + strlen(q) + 12 >= cap) break;
        if (com_base) snprintf(buf + k, cap - k, "%sbase(%s)", i ? " ou " : "", q);
        else          snprintf(buf + k, cap - k, "%s%s", i ? ", " : "", q);
    }
}

/* O nome do pai SEM acusar nada (pra `tp_de`, que roda mais de uma vez por
 * nó): o escrito, ou o único pai da Entity atual. NULL = não dá pra saber. */
static const char *base_pai_nome(C *c, PSNode *n)
{
    if (n->texto2) return n->texto2;
    PSNode *ent = c->dentro_entity ? c->entity_no : NULL;
    if (!ent || ent->lista2.n != 1) return NULL;
    return ent->lista2.itens[0]->texto;
}

/* O pai que `base(...)` designa, acusando o que estiver errado: fora de
 * Entity, Entity sem pai, método sem `self`, `base()` com mais de um pai,
 * `base(X)` com X que não é ancestral. NULL = acusou. */
static const char *base_pai(C *c, Unidade *u, PSNode *n)
{
    PSNode *ent = c->dentro_entity ? c->entity_no : NULL;
    if (!ent) { cerro_sx(c, n, "base() fora de Entity com heranca"); return NULL; }
    if (ent->lista2.n == 0) {
        cerro_sx(c, n, "base() numa Entity sem heranca: nao ha pai pra inicializar");
        return NULL;
    }
    /* `base(...)` entrega ao pai o `self` do método (o OP_LOAD_SELF lê o
     * slot 0). Numa funct cujo 1º parâmetro não é `self` — `funct
     * __init__(*args)` — o slot 0 é outra coisa. Sem self não há objeto. */
    if (u->eh_modulo || u->nlocais == 0 || strcmp(u->locais[0], "self") != 0) {
        cerro_sx(c, n, "base() precisa do self: declare `funct %s(self, ...)` "
                       "(o self e o objeto que o pai inicializa)",
                 u->nome_funct ? u->nome_funct : "__init__");
        return NULL;
    }
    char lista[256];
    if (!n->texto2) {
        if (ent->lista2.n > 1) {
            base_lista_pais(ent, lista, sizeof(lista), 1);
            cerro_sx(c, n, "base() com mais de um pai: diga qual, %s", lista);
            return NULL;
        }
        return ent->lista2.itens[0]->texto;
    }
    for (int32_t i = 0; i < ent->lista2.n; i++)
        if (ent->lista2.itens[i]->texto && strcmp(ent->lista2.itens[i]->texto, n->texto2) == 0)
            return n->texto2;
    if (ent->texto && tp_eh_ancestral(c, ent->texto, n->texto2, 0) == 0) {
        base_lista_pais(ent, lista, sizeof(lista), 0);
        cerro_sx(c, n, "'%s' nao e pai de '%s' (pais: %s)", n->texto2, ent->texto, lista);
        return NULL;
    }
    return n->texto2;                 /* ancestral, ou pai importado: a VM confere */
}

static const char *tp_de(C *c, Unidade *u, PSNode *n)
{
    if (!n) return NULL;
    if (ps_pilha_apertada()) { cerro_sx(c, n, "expressao aninhada demais"); return NULL; }
    switch (n->kind) {
        /* `base(Pai)` VALE o pai pra quem confere o `.membro` em cima dele:
         * campo/método/aridade saem pelo mesmo caminho de `p.membro` */
        case N_BASE_CALL_NODE: return base_pai_nome(c, n);
        case N_LITERAL:
            switch (n->lit) {
                case L_INT: case L_BIGINT: return "int";
                case L_FLO:   return "flo";
                case L_STR: case L_FSTRING: return "str";
                case L_BOOL:  return "bool";
                case L_NULL:  return "Null";
                case L_BYTES: return "byte";
            }
            return NULL;
        case N_NAME: {
            if (tp_nome_ligado(c, u, n->texto)) {
                SimInfo *s = tp_sim_de(c, u, n->texto);
                return s->estado ? s->tipo : NULL;
            }
            if (n->texto && strcmp(n->texto, "__name__") == 0) return "str";
            /* O `request` da jinker dentro do handler: o tipo é o que o
             * `--metadata` publica, e é o que faz o checador conferir os
             * membros (motor-7). Sem isto o nome existia e o tipo não, e
             * `request.get_query("a")` passava e caía rodando com 500. */
            if (c->usa_jinker && c->em_handler > 0 && n->texto
                    && strcmp(n->texto, "request") == 0)
                return "RequestProxy";
            /* O nome de um builtin usado como VALOR é `funct` (motor-17):
             * `int x = len` passava e rodando dava `esperava int, recebeu
             * funct`. O `ps_nome_pre_ligado` diz que o nome existe; aqui o
             * que se sabe é o tipo dele. */
            if (n->texto && !tp_nome_ligado(c, u, n->texto)
                    && ps_nome_pre_ligado(n->texto))
                return "funct";
            return NULL;
        }
        case N_BINARY_OP: {
            /* cadeia à esquerda em laço (ver o mesmo em expr_no): 8000 termos
             * não podem ser 8000 quadros */
            int32_t fundo = 0;
            PSNode *q = n;
            while (q->kind == N_BINARY_OP) { fundo++; q = q->a; }
            if (fundo < 32) return tp_binario(c, n->texto, tp_de(c, u, n->a), tp_de(c, u, n->b));
            PSNode **cadeia = malloc(sizeof(PSNode *) * (size_t)fundo);
            if (!cadeia) return NULL;
            int32_t k = fundo;
            for (q = n; k > 0; q = q->a) cadeia[--k] = q;
            const char *t = tp_de(c, u, q);
            for (k = 0; k < fundo; k++) t = tp_binario(c, cadeia[k]->texto, t, tp_de(c, u, cadeia[k]->b));
            free(cadeia);
            return t;
        }
        case N_UNARY_OP: {
            if (!n->texto) return NULL;
            if (!strcmp(n->texto, "not") || !strcmp(n->texto, "Not") || !strcmp(n->texto, "!")) return "bool";
            const char *t = tp_de(c, u, n->a);
            if (!strcmp(n->texto, "~")) return (t && (!strcmp(t, "int") || !strcmp(t, "bool"))) ? "int" : NULL;
            if (t && !strcmp(t, "bool")) return "int";
            return tp_numerico(t) ? t : NULL;
        }
        case N_CONDITIONAL: return tp_uniao(c, tp_de(c, u, n->a), tp_de(c, u, n->c));
        case N_CALL: return tp_chamada(c, u, n);
        case N_LIST_LITERAL: case N_LIST_COMP: return "list";
        case N_DICT_LITERAL: return "dict";
        case N_TUPLE_LITERAL: return "tup";
        case N_INTERPOLATED_STRING: case N_COLOR_STR_EXPR: return "str";
        case N_LAMBDA_EXPR: return "funct";
        case N_TYPE_NAME: return "type";
        case N_COUNT_EXPR: return "int";
        case N_POSTFIX_OP: return n->a ? tp_de(c, u, n->a) : NULL;
        case N_INDEX_ACCESS: {
            const char *t = tp_de(c, u, n->a);
            if (t && !strcmp(t, "str")) return "str";
            if (t && !strcmp(t, "byte")) return "int";
            return NULL;
        }
        case N_SLICE_ACCESS: {
            const char *t = tp_de(c, u, n->a);
            return tp_sequencia(t) ? t : NULL;
        }
        case N_MEMBER_ACCESS: {
            if (!n->texto) return NULL;
            char buf[64];
            const char *mod = tp_modulo_de(c, u, n->a, buf, sizeof(buf));
            if (mod) {
                char b2[64];
                if (tp_modulo_de(c, u, n, b2, sizeof(b2))) return "module";
                int k = ps_nativo_tem_membro(mod, n->texto);
                return k == 2 ? tp_nativo(mod, n->texto) : (k == 1 ? "funct" : NULL);
            }
            {
                SimInfo *sm = tp_sim_mod_pr(c, u, n->a);
                if (sm) {
                    int deco = 0;
                    PSNode *d = modulo_decl(sm->mod_pr, n->texto, &deco);
                    return (d && d->kind == N_FUNCT_DECL && !deco) ? "funct" : NULL;
                }
            }
            /* `Enum.MEMBRO` vale o TIPO do enum (motor-17): o membro é um
             * valor daquele tipo, e o `--check` aceitava `Cor.RED.foo()` e o
             * motor recusava rodando com `'int' object has no method 'foo'`.
             * O tipo é o nome do enum, e não `int`: `Cor c = Cor.RED` passa e
             * `int x = Cor.RED` é recusado, como rodando. */
            {
                PSNode *en = tp_tipo_arq(c, n->a->kind == N_NAME ? n->a->texto : NULL);
                if (en && en->kind == N_ENUM_DECL) {
                    for (int32_t i = 0; i < en->lista.n; i++)
                        if (en->lista.itens[i]->texto
                                && strcmp(en->lista.itens[i]->texto, n->texto) == 0)
                            return en->texto;
                    return NULL;
                }
            }
            /* `Classe.x`: o campo `static` tem o tipo escrito na declaração
             * (`str s = K.n` com `static int n` é recusado antes de rodar) */
            {
                PSNode *ce = tp_classe_exata(c, u, n->a);
                if (ce) {
                    PSNode *f = estatico_em(c, ce, n->texto, 0, NULL);
                    if (f && f->kind == N_ENTITY_FIELD) return f->texto2 ? tp_canon(f->texto2) : NULL;
                    if (f && f->kind == N_FUNCT_DECL) return "funct";
                    return NULL;
                }
            }
            const char *t = tp_de(c, u, n->a);
            if (!t || strchr(t, '|')) return NULL;
            if (tp_tipo_arq(c, t)) {
                const char *ft = tp_campo_tipo(c, t, n->texto, 0);
                if (ft) return ft;
                return tp_metodo(c, t, n->texto, 0) ? "funct" : NULL;
            }
            int k = ps_nativo_tem_membro(t, n->texto);
            return k == 2 ? tp_nativo(t, n->texto) : (k == 1 ? "funct" : NULL);
        }
        default:
            return NULL;
    }
}

/* ── conferências ── */

/* O comando nunca deixa a execução seguir pro próximo? `return`, `raise`, um
 * bloco que tem um desses, `if` com `else` em que todo ramo termina, `try` em
 * que o corpo e todo `catch` terminam (ou o `finally` termina) e `while True`
 * sem `break`. É o que decide se uma funct tipada pode chegar ao fim. */
static int tp_tem_break(PSNode *n)
{
    if (!n || ps_pilha_apertada()) return 0;   /* fundo demais: quem acusa e o stmt_no */
    if (n->kind == N_BREAK_STMT) return 1;
    if (n->kind == N_WHILE_STMT || n->kind == N_FOR_EACH_STMT || n->kind == N_COUNT_EACH_STMT
            || n->kind == N_FUNCT_DECL || n->kind == N_LAMBDA_EXPR) return 0;
    if (tp_tem_break(n->a) || tp_tem_break(n->b) || tp_tem_break(n->c) || tp_tem_break(n->e)) return 1;
    for (int32_t i = 0; i < n->lista.n; i++)  if (tp_tem_break(n->lista.itens[i])) return 1;
    for (int32_t i = 0; i < n->lista2.n; i++) if (tp_tem_break(n->lista2.itens[i])) return 1;
    return 0;
}

static int tp_termina(PSNode *s)
{
    if (!s) return 0;
    switch (s->kind) {
        case N_RETURN_STMT:
        case N_RAISE_STMT:
            return 1;
        case N_BLOCK:
            for (int32_t i = 0; i < s->lista.n; i++)
                if (tp_termina(s->lista.itens[i])) return 1;
            return 0;
        case N_IF_STMT: {
            int tem_else = 0;
            for (int32_t i = 0; i < s->lista.n; i++) {
                PSNode *ramo = s->lista.itens[i];
                if (!ramo->a) tem_else = 1;
                if (!tp_termina(ramo->b)) return 0;
            }
            return tem_else;
        }
        case N_TRY_CATCH_STMT:
            if (s->c && tp_termina(s->c)) return 1;
            if (!tp_termina(s->a)) return 0;
            for (int32_t i = 0; i < s->lista.n; i++)
                if (!tp_termina(s->lista.itens[i]->b)) return 0;
            return 1;
        case N_WHILE_STMT:
            return s->a && s->a->kind == N_LITERAL && s->a->lit == L_BOOL && s->a->i && !tp_tem_break(s->b);
        case N_USING_STMT:
            return tp_termina(s->b);
        default:
            return 0;
    }
}

/* O que o import liga, pra próxima gravação: módulo nativo (`import os`), ou
 * o membro dele (`from os import getenv`). Arquivo `.pr` fica de tipo
 * desconhecido aqui. Membro que o nativo não tem é erro antes de rodar. */
/* ── módulo `.pr` importado (regra 5) ── */

/* O nome do módulo como o runtime o mostra ("module 'sub.pix' has no
 * attribute"): sem os pontos de nível e sem o marcador de literal; caminho
 * entre aspas vira só o nome do arquivo, sem pasta nem extensão. */
static void tp_mod_visivel(const char *enc, char *buf, size_t cap)
{
    if (enc[0] == '\x01') {
        const char *spec = enc + 1;
        const char *b = ps_ultimo_sep(spec);
        snprintf(buf, cap, "%s", b ? b + 1 : spec);
        size_t l = strlen(buf);
        if (l > 3 && !strcmp(buf + l - 3, ".pr")) buf[l - 3] = '\0';
        return;
    }
    while (*enc == '.') enc++;
    snprintf(buf, cap, "%s", enc);
}

/* A funct/classe/model/enum `nome` declarada no topo do módulo (ou dentro de
 * um bloco de decorador do topo — `decorado` diz se um decorador GERAL pode
 * tê-la trocado, e aí a assinatura escrita não vale). NULL = não é
 * declaração do topo (variável, ou nome que o módulo não tem). */
static PSNode *modulo_decl(const PSModuloAst *ma, const char *nome, int *decorado)
{
    if (decorado) *decorado = 0;
    if (!ma || !ma->programa || !nome) return NULL;
    for (int32_t i = 0; i < ma->programa->lista.n; i++) {
        PSNode *s = ma->programa->lista.itens[i];
        if (!s) continue;
        if ((s->kind == N_FUNCT_DECL || s->kind == N_ENTITY_DECL || s->kind == N_MODEL_DECL
                || s->kind == N_ENUM_DECL) && s->texto && !strcmp(s->texto, nome))
            return s;
        if (s->kind == N_DECORATOR_STMT && s->b && s->b->kind == N_BLOCK) {
            int deco = s->a && !tp_decorador_embutido(s->a);
            for (int32_t k = 0; k < s->b->lista.n; k++) {
                PSNode *d = s->b->lista.itens[k];
                if (d && (d->kind == N_FUNCT_DECL || d->kind == N_ENTITY_DECL) && d->texto
                        && !strcmp(d->texto, nome)) {
                    if (decorado) *decorado = deco;
                    return d;
                }
            }
        }
    }
    return NULL;
}

static int modulo_exporta(const PSModuloAst *ma, const char *nome)
{
    for (int32_t i = 0; i < ma->nexportados; i++)
        if (ma->exportados[i] && !strcmp(ma->exportados[i], nome)) return 1;
    return 0;
}

/* O nome `nome` esta declarado `private` no topo do modulo `ma` (funct, class,
 * model, enum, variavel, desempacotamento — inclusive dentro de bloco de
 * decorador do topo)? Devolve o no da declaracao, pra frase dizer onde. */
static PSNode *modulo_privado_em(PSNode *s, const char *nome)
{
    if (!s) return NULL;
    if (s->kind == N_DECORATOR_STMT && s->b && s->b->kind == N_BLOCK) {
        for (int32_t k = 0; k < s->b->lista.n; k++) {
            PSNode *d = modulo_privado_em(s->b->lista.itens[k], nome);
            if (d) return d;
        }
        return NULL;
    }
    if (!s->is_private) return NULL;
    switch (s->kind) {
        case N_FUNCT_DECL: case N_ENTITY_DECL: case N_MODEL_DECL: case N_ENUM_DECL:
        case N_ASSIGNMENT: case N_VAR_DECL: case N_FIELD_DECL:
            return (s->texto && !strcmp(s->texto, nome)) ? s : NULL;
        case N_UNPACK_ASSIGNMENT: {
            /* os NOMEs do alvo, em qualquer profundidade */
            PSNode *pilha[64]; int np = 0;
            if (s->a) pilha[np++] = s->a;
            while (np > 0) {
                PSNode *a = pilha[--np];
                if (!a) continue;
                if (a->kind == N_NAME && a->texto && !strcmp(a->texto, nome)) return s;
                if (a->kind == N_UNPACK_TARGET)
                    for (int32_t i = 0; i < a->lista.n && np < 64; i++) pilha[np++] = a->lista.itens[i];
            }
            return NULL;
        }
        default: return NULL;
    }
}

static PSNode *modulo_privado(const PSModuloAst *ma, const char *nome)
{
    if (!ma || !ma->programa || !nome) return NULL;
    for (int32_t i = 0; i < ma->programa->lista.n; i++) {
        PSNode *d = modulo_privado_em(ma->programa->lista.itens[i], nome);
        if (d) return d;
    }
    return NULL;
}

/* `m.x` / `from m import x` com `x` private em `m`: a frase do runtime
 * ("existe, mas é private"), antes de rodar, com o quadro da declaracao (o
 * arquivo do modulo, a linha e o nome — o cursor vai em cima do nome). */
static const char *base_do_caminho(const char *p)
{
    const char *b = p ? ps_ultimo_sep(p) : NULL;
    return b ? b + 1 : (p ? p : "?");
}

static void terro_privado(C *c, PSNode *n, const PSModuloAst *ma, const char *vis,
                          const char *nome, PSNode *decl)
{
    int32_t antes = c->nerros;
    terro(c, n, "AttributeError",
          "module '%s' has no attribute '%s' (existe, mas é private: declarada em %s, linha %d)",
          vis, nome, base_do_caminho(ma->caminho), decl->line);
    if (c->nerros > antes) {
        PSErroTipo *e = &c->erros[c->nerros - 1];
        snprintf(e->arquivo, sizeof(e->arquivo), "%s", ma->caminho ? ma->caminho : "");
        e->linha_arq = decl->line;
        e->col_arq = decl->col;
        snprintf(e->nome_arq, sizeof(e->nome_arq), "%s", nome);
    }
}

/* O nome `n` é um módulo `.pr` importado (`import util`, `import a.b as u`)? */
static SimInfo *tp_sim_mod_pr(C *c, Unidade *u, PSNode *n)
{
    if (!n || n->kind != N_NAME || !n->texto) return NULL;
    SimInfo *s = tp_nome_ligado(c, u, n->texto) ? tp_sim_de(c, u, n->texto) : NULL;
    return (s && s->mod_pr && !s->decl) ? s : NULL;
}

/* Registra a classe importada entre os tipos do arquivo, pra `tp_metodo`,
 * `tp_campo_tipo` e `tp_classe_tem` (que buscam por NOME) valerem pra ela.
 * 0 quando já existe OUTRA classe com esse nome aqui: aí não se confere
 * nada por esse nome — conferir contra a errada seria um veredito falso. */
static int tp_classe_importada(C *c, PSNode *d)
{
    if (!d || d->kind != N_ENTITY_DECL || !d->texto) return 0;
    PSNode *ja = tp_tipo_decl_arq(c, d->texto);
    if (ja) return ja == d && !tp_tipo_ambiguo(c, d->texto);
    tp_coleta_tipos_arq(c, d);
    return tp_tipo_arq(c, d->texto) == d;
}

/* O nó que o erro do import deve APONTAR: o nome do módulo (`n->lista` tem um
 * nó por parte do caminho, cada um com a coluna dele) ou o nome importado
 * (`n->lista2`). Antes o erro saía sempre na coluna do `import`/`from` — o
 * editor sublinhava a palavra-chave em vez do nome errado. */
static PSNode *tp_no_do_import(PSNode *n, const char *membro)
{
    if (!n) return n;
    if (membro)
        for (int32_t i = 0; i < n->lista2.n; i++)
            if (n->lista2.itens[i] && n->lista2.itens[i]->texto
                && strcmp(n->lista2.itens[i]->texto, membro) == 0)
                return n->lista2.itens[i];
    if (n->lista.n > 0 && n->lista.itens[0]) return n->lista.itens[0];
    return n;
}

/* Os opcodes do import apontam o trecho que o `--check` aponta: o NOME do
 * módulo (o IMPORT_MOD) e o do membro pedido (o IMPORT_FROM). O cursor do
 * erro rodando caía no `import`, e o do editor no nome. */
static int32_t emite_import(C *c, Unidade *u, PSNode *n, int32_t op, const char *nome, const char *membro)
{
    PSNode *onde = tp_no_do_import(n, membro);
    int32_t l0 = c->linha_atual, c0 = c->coluna_atual;
    if (onde && onde != n && onde->line > 0) { c->linha_atual = onde->line; c->coluna_atual = onde->col; }
    int32_t r = emite(c, u, op, idx_const(c, u, K_STR, 0, 0, nome, (int32_t)strlen(nome)));
    c->linha_atual = l0; c->coluna_atual = c0;
    return r;
}

static void tp_grava_import(C *c, PSNode *n, const char *mod, const char *membro)
{
    memset(&c->grava, 0, sizeof(c->grava));
    c->grava.no = n;
    if (!mod) return;
    if (n->i2 == 0 && ps_nativo_eh_modulo(mod)) {
        const char *m = tp_guarda(c, mod);
        if (!membro) {
            c->grava.tem_valor = 1;
            c->grava.tipo = "module";
            c->grava.mod_nativo = m;
            return;
        }
        int k = ps_nativo_tem_membro(mod, membro);
        if (k == 0) {
            terro(c, tp_no_do_import(n, membro), "ImportError",
                  "cannot import name '%s' from '%s' (unknown location)", membro, mod);
            return;
        }
        c->grava.tem_valor = 1;
        if (k == 1) {
            c->grava.tipo = "funct";
            c->grava.mod_nativo = m;
            c->grava.membro_nativo = tp_guarda(c, membro);
        } else {
            c->grava.tipo = tp_nativo(mod, membro);
            if (!c->grava.tipo) c->grava.tem_valor = 0;
        }
        return;
    }
    /* Módulo `.pr`: o resolvedor entrega a AST e os exportados (compilado
     * sem rodar, como na expansão do `*`). Sem resolvedor, ou módulo que não
     * se acha / não compila / em ciclo, o checador fica cego pra ele — o
     * runtime dá o erro de sempre na linha do import. */
    const PSModuloAst *ma = NULL;
    if (!c->resolve || !c->resolve->modulo_de) return;
    int r = c->resolve->modulo_de(c->resolve->ctx, mod, &ma);
    char vis[256];
    tp_mod_visivel(mod, vis, sizeof(vis));
    if (r == 3) {
        /* O módulo não existe. A frase é a MESMA que o import daria rodando —
         * antes o `--check` dizia `{"ok":true}` e um typo no import só
         * aparecia na execução. O `terro` não repete a mesma linha/coluna, e
         * por isso `from m import a, b, c` acusa uma vez só. Dentro de um
         * `try` que pega o `ImportError` é o import OPCIONAL: o programa
         * roda e o catch trata (doc 10.4), e o `--check` o reprovava. */
        if (!c->import_protegido)
            terro(c, tp_no_do_import(n, NULL), "ImportError", "No module named '%s'", vis);
        c->grava.tem_valor = 1;
        return;
    }
    if (r == 0 || !ma) return;
    if (r == 2) {
        /* o módulo não compila: a frase que o import daria rodando, na
         * linha do import — sem isto o erro só aparecia executando. O
         * arquivo e a linha de DENTRO do módulo vão junto, pro quadro. */
        int32_t antes = c->nerros;
        terro(c, n, ma->erro_classe, "%s: %s", vis, ma->erro_msg);
        if (c->nerros > antes) {
            PSErroTipo *e = &c->erros[c->nerros - 1];
            snprintf(e->arquivo, sizeof(e->arquivo), "%s", ma->erro_arquivo);
            e->linha_arq = ma->erro_linha;
            e->col_arq = ma->erro_col;
        }
        c->grava.tem_valor = 1;
        return;
    }
    if (!membro) {
        c->grava.tem_valor = 1;
        c->grava.tipo = "module";
        c->grava.mod_pr = ma;
        c->grava.mod_pr_nome = tp_guarda(c, vis);
        return;
    }
    int deco = 0;
    PSNode *priv = modulo_privado(ma, membro);
    if (priv) {
        /* `from m import x` com `x` private: a mesma frase que `m.x` — o
         * runtime tambem acusa AttributeError aqui, nao ImportError */
        terro_privado(c, tp_no_do_import(n, membro), ma, vis, membro, priv);
        c->grava.tem_valor = 1;
        return;
    }
    PSNode *d = modulo_decl(ma, membro, &deco);
    if (!d) {
        /* variável do módulo (tipo só rodando), ou nome que ele não tem —
         * a mesma frase do ImportError de runtime, antes de rodar */
        if (!ma->incompleto && !modulo_exporta(ma, membro))
            terro(c, tp_no_do_import(n, membro), "ImportError",
                  "cannot import name '%s' from '%s' (%s)", membro, vis, ma->caminho);
        c->grava.tem_valor = 1;
        return;
    }
    c->grava.tem_valor = 1;
    c->grava.decl = d;
    c->grava.decorado = (unsigned char)deco;
    if (d->kind == N_FUNCT_DECL) c->grava.tipo = "funct";
    else if (d->kind == N_ENTITY_DECL) {
        c->grava.tipo = "Entity";
        if (!tp_classe_importada(c, d)) c->grava.decl = NULL;
    }
}

/* Emite a conferência RODANDO do valor no topo da pilha contra `tipo`
 * (OP_CONFERE_TIPO). `rotulo` é o começo da mensagem ("variável x"). */
static void tp_emite_confere(C *c, Unidade *u, const char *rotulo, const char *tipo, int declarado)
{
    if (!tipo) return;
    char spec[512];
    int n = snprintf(spec, sizeof(spec), "%c%s\x1f%s", declarado ? 'D' : 'I', rotulo, tipo);
    if (n < 0 || n >= (int)sizeof(spec)) return;
    emite(c, u, OP_CONFERE_TIPO, idx_const(c, u, K_STR, 0, 0, spec, n));
}

/* Escrita do valor de `c->grava` no nome cujo SimInfo é `s`: confere o que dá
 * pra saber e atualiza o que se sabe do nome. Devolve T_SIM/T_NAO/T_TALVEZ. */
static int tp_escreve(C *c, SimInfo *s, const char *nome)
{
    if (!s) return T_TALVEZ;
    PSNode *onde = c->grava.no;
    if (c->grava.redefine) {
        /* o nome continua sendo o da declaração que o decorador embrulhou */
        const char *rot = s->decl_rotulo;
        int32_t lin = s->decl_linha;
        memset(s, 0, sizeof(*s));
        s->estado = 1;
        s->decorado = 1;
        s->decl_rotulo = rot; s->decl_linha = lin;
        return T_SIM;
    }
    /* Uma declaração num nome que outra declaração ainda liga no MESMO escopo,
     * com um dos dois sendo class/model/enum: a 2ª valia pela metade (o
     * checador seguia a 1ª e a VM a 2ª, e o erro caía num `self.y` certo).
     * O nome de um bloco que já fechou saiu do escopo e não conta (o `if` e o
     * `else` podem declarar cada um a sua); funct sobre funct fica como era.
     * Acusada a 2ª, ela passa a valer, sem o erro de tipo em cascata. */
    if (c->grava.declaracao && onde) {
        const char *rot = onde->kind == N_ENTITY_DECL ? "class" : onde->kind == N_MODEL_DECL ? "model"
                        : onde->kind == N_ENUM_DECL ? "enum" : "funct";
        if (s->decl_rotulo && (strcmp(rot, "funct") != 0 || strcmp(s->decl_rotulo, "funct") != 0)) {
            PSNode tmp;
            terro(c, no_do_nome(onde, &tmp), "SyntaxError",
                  "'%s' ja foi declarado na linha %d (%s): um nome, uma declaracao no mesmo escopo",
                  nome, s->decl_linha, s->decl_rotulo);
            memset(s, 0, sizeof(*s));
        }
        s->decl_rotulo = rot;
        s->decl_linha = onde->line;
    }
    if (c->grava.declara) {
        const char *T = tp_canon_guarda(c, c->grava.declara);
        if (s->estado == 2 && s->tipo && strcmp(s->tipo, T) != 0)
            terro(c, onde, "AttributedValueError",
                  "variável %s já foi declarada como %s e não pode ser redeclarada como %s", nome, s->tipo, T);
        else if (s->estado == 1 && s->tipo && strcmp(s->tipo, "Null") != 0
                 && tp_aceita(c, T, s->tipo, NULL, 1) == T_NAO)
            terro(c, onde, "AttributedValueError",
                  "variável %s já é %s (tipo fixado na primeira atribuição) e não pode ser redeclarada como %s",
                  nome, s->tipo, T);
        s->estado = 2;
        s->tipo = T;
        s->decl = NULL; s->decorado = 0;
        s->mod_nativo = s->membro_nativo = NULL;
        s->mod_pr = NULL; s->mod_pr_nome = NULL;
    }
    if (c->grava.decl) { s->decl = c->grava.decl; s->decorado = c->grava.decorado; }
    if (c->grava.mod_nativo) { s->mod_nativo = c->grava.mod_nativo; s->membro_nativo = c->grava.membro_nativo; }
    if (c->grava.mod_pr) { s->mod_pr = c->grava.mod_pr; s->mod_pr_nome = c->grava.mod_pr_nome; }
    if (!c->grava.tem_valor) {
        if (s->estado == 0) { s->estado = 1; s->tipo = NULL; }
        return (s->estado == 2 || s->tipo) ? T_TALVEZ : T_SIM;
    }
    const char *vt = c->grava.tipo;
    if (s->estado == 0) {
        if (vt && strcmp(vt, "Null") == 0) return T_SIM;
        s->estado = 1;
        /* guardado: o SimInfo de um nome de módulo vive até o fim da
         * compilação, e `vt` pode ser o nome de uma classe lido da árvore da
         * declaração atual, que morre com ela na segunda passada */
        s->tipo = tp_canon_guarda(c, vt);
        return T_SIM;
    }
    if (!s->tipo) return T_SIM;
    int v = tp_aceita(c, s->tipo, vt, c->grava.no, s->estado == 2 ? 2 : 0);
    if (v == T_NAO) {
        PSNode *lit = c->grava.no;
        if (s->estado == 2 && !strcmp(s->tipo, "char") && lit && lit->kind == N_LITERAL
                && lit->lit == L_STR && lit->texto)
            /* a frase do OP_COERCE_DECL: quantos caracteres vieram */
            terro(c, onde, "AttributedValueError", "variável %s esperava char (um caractere), recebeu %d", nome,
                  tp_conta_utf8(lit->texto, lit->texto_len > 0 ? lit->texto_len : (int32_t)strlen(lit->texto)));
        else if (s->estado == 2)
            terro(c, onde, "AttributedValueError", "variável %s esperava %s, recebeu %s", nome, s->tipo, vt);
        else
            terro(c, onde, "AttributedValueError",
                  "variável %s é %s (tipo fixado na primeira atribuição), recebeu %s", nome, s->tipo, vt);
    }
    return v;
}

/* Depois de `tp_escreve`: o que emitir pra conferir rodando. `cod1` é o tipo
 * da tabela (TIPO_* + 1) que o OP_COERCE_DECL já sabe conferir — e converter,
 * no caso do `char`. */
static void tp_emite_escrita(C *c, Unidade *u, const char *nome, SimInfo *s, int veredito, int cod1)
{
    if (veredito == T_SIM || veredito == T_NAO) return;
    if (cod1 > 0) { emite_coerce_se_tipado(c, u, nome, cod1); return; }
    if (!s || !s->tipo || s->estado == 0) return;
    char rot[300];
    snprintf(rot, sizeof(rot), "variável %s", nome);
    tp_emite_confere(c, u, rot, s->tipo, s->estado == 2);
}

/* O método é `static` (modificador colado ou `@static` na entrada de cima)? */
static int tp_metodo_estatico(C *c, const char *classe, PSNode *met)
{
    if (met->is_static) return 1;
    PSNode *d = tp_tipo_arq(c, classe);
    if (!d || d->kind != N_ENTITY_DECL) return 0;
    for (int32_t i = 1; i < d->lista.n; i++) {
        if (!mesmo_no(d->lista.itens[i], met)) continue;
        PSNode *ant = d->lista.itens[i - 1];
        const char *dn = (ant->kind == N_DECORATOR_STMT && ant->a && ant->a->lista.n == 1)
                       ? ant->a->lista.itens[0]->texto : NULL;
        return dn && !strcmp(dn, "static");
    }
    return 0;
}

/* Método sem `self`: o erro é da DECLARAÇÃO e sai ali, antes de rodar.
 *
 * A regra (7.3) é que o primeiro parâmetro de um método é `self`, a instância
 * — a não ser que ele seja `static`, ou que comece por `*args` (a instância
 * entra na tup, 7.2). Sem isto o que saía era o SINTOMA, longe da causa:
 * `NameError: name 'self' is not defined` no primeiro `self` do corpo, mais um
 * `TypeError: m() takes 0 positional arguments but 1 was given` em cada
 * chamada — e nada na linha do `funct`, que é onde o defeito está.
 *
 * Depois de acusar, o método ganha um `self` sintetizado na frente dos
 * parâmetros e o resto da conferência segue como se ele tivesse sido escrito:
 * um erro só, o da causa, sem a cascata. O programa não roda (erro de tipo),
 * então o parâmetro a mais nunca executa; e a AST volta ao que era no fim da
 * compilação (`self_sint`, desfeito em ps_compila_com). */
struct SelfSint { PSNode *met; PSNodeVec orig; PSNode *no; };

static void tp_prepoe_self(C *c, PSNode *m)
{
    PSNode  *no    = calloc(1, sizeof(PSNode));
    PSNode **itens = malloc(sizeof(PSNode *) * (size_t)(m->lista.n + 1));
    if (!no || !itens) { free(no); free(itens); cerro(c, "sem memoria", m); return; }
    if (c->n_self_sint + 1 > c->cap_self_sint) {
        int32_t novo = c->cap_self_sint < 8 ? 8 : c->cap_self_sint * 2;
        struct SelfSint *nv = realloc(c->self_sint, sizeof(*nv) * (size_t)novo);
        if (!nv) { free(no); free(itens); cerro(c, "sem memoria", m); return; }
        c->self_sint = nv; c->cap_self_sint = novo;
    }
    no->kind = N_NAME; no->line = m->line; no->col = m->col; no->texto = "self";
    itens[0] = no;
    if (m->lista.n > 0) memcpy(itens + 1, m->lista.itens, sizeof(PSNode *) * (size_t)m->lista.n);
    struct SelfSint *s = &c->self_sint[c->n_self_sint++];
    s->met = m; s->orig = m->lista; s->no = no;
    m->lista.itens = itens;
    m->lista.n += 1;
    m->lista.cap = m->lista.n;
    m->self_faltava = 1;
}

/* Corre os métodos de UMA classe/Entity. Idempotente (`self_faltava`): a
 * pré-passada chama pras classes do arquivo, antes de qualquer chamada ser
 * conferida; a compilação da classe chama de novo, pelas que a pré-passada
 * não vê (classe dentro de funct). O `@static` é a entrada anterior da lista,
 * como na compilação da classe. */
static void tp_self_dos_metodos(C *c, PSNode *cls)
{
    if (!cls || cls->kind != N_ENTITY_DECL) return;
    int estatico = 0;
    for (int32_t i = 0; i < cls->lista.n && !CFALHOU(c); i++) {
        PSNode *m = cls->lista.itens[i];
        if (!m) continue;
        if (m->kind == N_DECORATOR_STMT) {
            PSNode *dec = m->a;
            const char *dn = (dec && dec->lista.n == 1) ? dec->lista.itens[0]->texto : NULL;
            if (dn && !strcmp(dn, "static")) estatico = 1;
            continue;
        }
        if (m->kind != N_FUNCT_DECL) continue;
        int meu = estatico || m->is_static;
        estatico = 0;
        if (meu || m->self_faltava) continue;
        PSNode *p0 = m->lista.n > 0 ? m->lista.itens[0] : NULL;
        if (p0 && p0->i2 == 1) continue;                               /* `*args`: 7.2 */
        if (p0 && p0->i2 == 0 && p0->texto && !strcmp(p0->texto, "self")) continue;
        const char *nome = m->texto ? m->texto : "?";
        if (p0 && p0->i2 == 0 && p0->texto)
            terro(c, m, "TypeError", "método %s(%s) sem self: o primeiro parâmetro de um método é self, "
                                     "não '%s' (ou marque static)", nome, p0->texto, p0->texto);
        else if (p0 && p0->i2 == 2 && p0->texto)
            terro(c, m, "TypeError", "método %s(**%s) sem self: o primeiro parâmetro de um método é self "
                                     "(ou marque static)", nome, p0->texto);
        else
            terro(c, m, "TypeError", "método %s() sem self: o primeiro parâmetro de um método é self "
                                     "(ou marque static)", nome);
        tp_prepoe_self(c, m);
    }
}

/* Os nomes que uma Entity do arquivo tem — campos declarados, métodos,
 * `self.x` gravados nos métodos e os dos pais — pro "Did you mean" da frase
 * de membro ausente antes de rodar (a mesma régua da VM). */
static void tp_junta_self_gravados(PSNode *n, const char **v, int *cnt)
{
    if (!n || *cnt >= 256) return;
    if (n->kind == N_LAMBDA_EXPR) return;
    if (n->kind == N_MEMBER_ASSIGNMENT && n->texto && n->a && n->a->kind == N_NAME
            && n->a->texto && !strcmp(n->a->texto, "self")) v[(*cnt)++] = n->texto;
    if (n->kind == N_FIELD_DECL && n->texto) v[(*cnt)++] = n->texto;
    tp_junta_self_gravados(n->a, v, cnt);
    tp_junta_self_gravados(n->b, v, cnt);
    tp_junta_self_gravados(n->c, v, cnt);
    tp_junta_self_gravados(n->e, v, cnt);
    for (int32_t i = 0; i < n->lista.n; i++)  tp_junta_self_gravados(n->lista.itens[i], v, cnt);
    for (int32_t i = 0; i < n->lista2.n; i++) tp_junta_self_gravados(n->lista2.itens[i], v, cnt);
}
static void tp_junta_membros_classe(C *c, const char *classe, const char **v, int *cnt, int prof)
{
    PSNode *d = tp_tipo_arq(c, classe);
    if (!d || d->kind != N_ENTITY_DECL || prof > 32 || *cnt >= 256) return;
    for (int32_t i = 0; i < d->lista2_alias.n && *cnt < 256; i++)
        if (d->lista2_alias.itens[i] && d->lista2_alias.itens[i]->texto) v[(*cnt)++] = d->lista2_alias.itens[i]->texto;
    for (int32_t i = 0; i < d->lista.n && *cnt < 256; i++) {
        PSNode *m = d->lista.itens[i];
        if (!m) continue;
        if (m->kind == N_FUNCT_DECL && m->texto) v[(*cnt)++] = m->texto;
        if (m->kind == N_FUNCT_DECL) tp_junta_self_gravados(m->b, v, cnt);
    }
    for (int32_t i = 0; i < d->lista2.n; i++)
        if (d->lista2.itens[i] && d->lista2.itens[i]->texto)
            tp_junta_membros_classe(c, d->lista2.itens[i]->texto, v, cnt, prof + 1);
}
static const char *tp_sugere_membro(C *c, const char *tipo, const char *nome)
{
    if (!tipo || !nome || strchr(tipo, '|')) return NULL;
    PSNode *d = tp_tipo_arq(c, tipo);
    if (d && d->kind == N_ENTITY_DECL) {
        const char *v[256];
        int cnt = 0;
        tp_junta_membros_classe(c, tipo, v, &cnt, 0);
        return ps_sugere_nome(nome, v, cnt);
    }
    if (d) return NULL;
    return ps_nativo_sugestao(tipo, nome);
}
/* ". Did you mean: 'x'?" ou "" — o mesmo sufixo que a VM põe rodando */
static const char *tp_dica_membro(C *c, const char *tipo, const char *nome, char *buf, size_t cap)
{
    const char *s = tp_sugere_membro(c, tipo, nome);
    if (!s) return "";
    snprintf(buf, cap, ". Did you mean: '%s'?", s);
    return buf;
}

static PSNode *tp_campo_nunca_nasce(C *c, PSNode *cls, const char *nome);

/* `alvo.nome` existe? Módulo nativo, tipo nativo (pelas tabelas da VM),
 * Entity do arquivo (campo, método, `self.x` gravado, pais) e enum. O que não
 * se sabe — tipo desconhecido, pai importado — passa. */
static void tp_confere_membro(C *c, Unidade *u, PSNode *n)
{
    if (!n->texto || !n->a) return;
    /* `json.parse` sem `import json`: `json` também é nome de TIPO (o dict),
     * e por isso a conferência do nome solto o deixava passar. Receptor de
     * `.` é o MÓDULO, e rodando dava NameError: o módulo que pede import, sem
     * o import, é NameError aqui também — a frase do `os.getcwd()` sem import. */
    if (n->a->kind == N_NAME && n->a->texto && !c->nomes_incertos
            && ps_tipo_info(n->a->texto) && ps_nativo_eh_modulo(n->a->texto)
            && !tp_ligado_no_escopo(c, u, n->a->texto)) {
        if (c->nome_oculto) tp_acusa_oculto(c, n->a, n->a->texto);
        else tp_acusa_sem_import(c, n->a, n->a->texto);
        return;
    }
    char buf[64];
    const char *mod = tp_modulo_de(c, u, n->a, buf, sizeof(buf));
    if (mod) {
        char b2[64];
        if (tp_modulo_de(c, u, n, b2, sizeof(b2))) return;       /* sys.stdout */
        if (ps_nativo_tem_membro(mod, n->texto) == 0) {
            const char *dica = ps_nativo_sugestao(mod, n->texto);
            if (dica)
                terro(c, n, "AttributeError", "module '%s' has no attribute '%s'. Did you mean: '%s'?",
                      mod, n->texto, dica);
            else
                terro(c, n, "AttributeError", "module '%s' has no attribute '%s'", mod, n->texto);
        }
        return;
    }
    {
        /* módulo `.pr` importado: o que ele não exporta não existe — a
         * mesma frase do runtime, antes de rodar (ciclo de `*`: sem veredito) */
        SimInfo *sm = tp_sim_mod_pr(c, u, n->a);
        if (sm) {
            const PSModuloAst *ma = sm->mod_pr;
            PSNode *priv = modulo_privado(ma, n->texto);
            if (priv)
                terro_privado(c, n, ma, sm->mod_pr_nome ? sm->mod_pr_nome : "?", n->texto, priv);
            else if (!ma->incompleto && !modulo_decl(ma, n->texto, NULL) && !modulo_exporta(ma, n->texto))
                terro(c, n, "AttributeError", "module '%s' has no attribute '%s'",
                      sm->mod_pr_nome ? sm->mod_pr_nome : "?", n->texto);
            return;
        }
    }
    if (n->a->kind == N_BASE_CALL_NODE) {
        /* `base(Pai).x`: o valor é o próprio pai (exato), e quem acessa é a
         * classe do método — sem `base_pai`, que já acusa o `base()` mal
         * escrito na compilação */
        PSNode *ent = c->dentro_entity ? c->entity_no : NULL;
        const char *pai = n->a->texto2 ? n->a->texto2
                        : (ent && ent->lista2.n == 1 ? ent->lista2.itens[0]->texto : NULL);
        if (pai) tp_confere_privado(c, tp_tipo_arq(c, pai), n->texto, 1, n);
    }
    if (n->a->kind == N_MEMBER_ACCESS) {
        /* `banco.Conta.x`: a classe de um `.pr` importado, pelo módulo */
        PSNode *ce = tp_classe_exata(c, u, n->a);
        if (ce) { tp_confere_privado(c, ce, n->texto, 1, n); return; }
    }
    if (n->a->kind == N_NAME && n->a->texto) {
        SimInfo *s = tp_sim_de(c, u, n->a->texto);
        if (s && s->decl && s->decl->kind == N_ENTITY_DECL && !s->decorado) {
            if (tp_classe_tem(c, s->decl->texto, n->texto, 0) == 0) {
                char dica[160];
                terro(c, n, "AttributeError", "'%s' object has no %s '%s'%s", s->decl->texto,
                      tp_rotulo_membro(c, n), n->texto,
                      tp_dica_membro(c, s->decl->texto, n->texto, dica, sizeof(dica)));
                return;
            }
            /* `Conta.segredo` de fora: o private vale pelo nome da classe
             * também, como pela instância (a VM confere a mesma coisa) */
            int nerros_antes = c->nerros;
            tp_confere_privado(c, s->decl, n->texto, 1, n);
            if (c->nerros > nerros_antes) return;
            /* Campo declarado SEM `static` lido pela Entity: existe, mas só na
             * instância. Antes o `--check` calava e a VM mandava procurar erro
             * de digitação num nome escrito certo. A frase é a mesma do método
             * não-estático, e a causa é a mesma. */
            if (tp_campo_tipo(c, s->decl->texto, n->texto, 0)
                    && !estatico_em(c, s->decl, n->texto, 0, NULL))
                terro(c, n, "RuntimeError",
                      "Entity '%s' não tem campo estático '%s' — instancie primeiro",
                      s->decl->texto ? s->decl->texto : "?", n->texto);
            return;
        }
        if (s && s->decl && s->decl->kind == N_ENUM_DECL) {
            for (int32_t i = 0; i < s->decl->lista.n; i++)
                if (s->decl->lista.itens[i]->texto && !strcmp(s->decl->lista.itens[i]->texto, n->texto)) return;
            terro(c, n, "AttributeError", "type object '%s' has no attribute '%s'",
                  s->decl->texto ? s->decl->texto : "?", n->texto);
            return;
        }
    }
    const char *t = tp_de(c, u, n->a);
    if (!t) return;
    /* instância de uma classe do arquivo, com o membro existindo: o private,
     * e o campo declarado sem valor que nada grava (a frase da VM) */
    if (!strchr(t, '|')) {
        PSNode *d = tp_tipo_arq(c, t);
        if (d && d->kind == N_ENTITY_DECL && tp_classe_tem(c, t, n->texto, 0) == 1) {
            int nerros_antes = c->nerros;
            tp_confere_privado(c, d, n->texto, 0, n);
            PSNode *dona = c->nerros == nerros_antes ? tp_campo_nunca_nasce(c, d, n->texto) : NULL;
            if (dona) {
                char causa[PS_FRASE_SEM_VALOR_CAP];
                snprintf(causa, sizeof(causa), PS_FRASE_SEM_VALOR, n->texto, dona->texto, n->texto);
                terro(c, n, "AttributeError", "'%s' object has no %s '%s'%s", t, tp_rotulo_membro(c, n),
                      n->texto, causa);
            }
            return;
        }
    }
    /* união: só é erro se NENHUM lado (fora o Null) tem o membro. O Null
     * sozinho não tem membro nenhum: `Null.foo()` é erro, como rodando. */
    int lados = 0;
    const char *q = t;
    while (q && *q) {
        const char *bar = strchr(q, '|');
        size_t len = bar ? (size_t)(bar - q) : strlen(q);
        char lado[128];
        if (len >= sizeof(lado)) return;
        memcpy(lado, q, len); lado[len] = '\0';
        q = bar ? bar + 1 : NULL;
        if (!strcmp(lado, "Null") && strchr(t, '|')) continue;
        PSNode *d = tp_tipo_arq(c, lado);
        int r = (d && d->kind == N_ENTITY_DECL) ? tp_classe_tem(c, lado, n->texto, 0)
              : d ? -1 : ps_nativo_tem_membro(lado, n->texto);
        if (r != 0) return;
        lados++;
    }
    if (lados > 0) {
        char dica[160];
        terro(c, n, "AttributeError", "'%s' object has no %s '%s'%s", t, tp_rotulo_membro(c, n), n->texto,
              tp_dica_membro(c, t, n->texto, dica, sizeof(dica)));
    }
}

/* ── qual construtor roda pra `Classe(...)` ────────────────────────────────
 * A regra da VM (`acha_metodo`: a classe antes dos pais, da esquerda pra
 * direita, em profundidade) com a síntese do compilador (sem `__init__`
 * escrito, a classe com campo de instância ganha o gerado dos campos dela).
 * Era um `tp_metodo(__init__)` que passava reto pelo gerado da filha e
 * conferia `Filha()` contra o `__init__` escrito do pai. `ma`: a classe é de
 * um `.pr` importado, e os pais se procuram nele. */
enum { CONS_INCERTO = -1, CONS_NENHUM = 0, CONS_ESCRITO = 1, CONS_GERADO = 2 };

static int tp_decorador_geral_antes(PSNode *cls, int32_t i)
{
    if (i <= 0) return 0;
    PSNode *ant = cls->lista.itens[i - 1];
    if (!ant || ant->kind != N_DECORATOR_STMT || !ant->a) return 0;
    const char *dn = ant->a->lista.n == 1 ? ant->a->lista.itens[0]->texto : NULL;
    return !(dn && (!strcmp(dn, "static") || !strcmp(dn, "NonNull") || !strcmp(dn, "dataentity")));
}

/* CONS_ESCRITO: `*no` é o `__init__`; CONS_GERADO: `*no` é a classe dona dos
 * campos; CONS_NENHUM; CONS_INCERTO (pai que não se acha, `__init__`
 * decorado: fica pra VM). */
static int tp_construtor(C *c, PSNode *cls, const PSModuloAst *ma, PSNode **no, int prof)
{
    if (!cls || cls->kind != N_ENTITY_DECL || prof > 1000) return CONS_INCERTO;
    for (int32_t i = 0; i < cls->lista.n; i++) {
        PSNode *m = cls->lista.itens[i];
        if (!m || m->kind != N_FUNCT_DECL || !m->texto || strcmp(m->texto, "__init__") != 0) continue;
        if (tp_decorador_geral_antes(cls, i)) return CONS_INCERTO;
        *no = m;
        return CONS_ESCRITO;
    }
    for (int32_t i = 0; i < cls->lista2_alias.n; i++)
        if (cls->lista2_alias.itens[i] && !cls->lista2_alias.itens[i]->is_static) { *no = cls; return CONS_GERADO; }
    for (int32_t i = 0; i < cls->lista2.n; i++) {
        const char *pn = cls->lista2.itens[i] ? cls->lista2.itens[i]->texto : NULL;
        int deco = 0;
        PSNode *pd = !pn ? NULL : ma ? modulo_decl(ma, pn, &deco) : tp_tipo_arq(c, pn);
        if (!pd || deco) return CONS_INCERTO;
        int r = tp_construtor(c, pd, ma, no, prof + 1);
        if (r != CONS_NENHUM) return r;
    }
    return CONS_NENHUM;
}

/* As declarações de `nome` na cadeia de `cls` (a classe e todos os pais do
 * arquivo): `*sem` = a primeira classe, na ordem de busca de membro, que o
 * declara como campo do corpo SEM valor; `*outro` = alguma declaração dá
 * valor a ele (campo com padrão, `static`) ou o faz método. -1 = um pai não é
 * do arquivo (não dá pra saber). */
static int tp_campo_na_cadeia(C *c, PSNode *cls, const char *nome, int prof, PSNode **sem, int *outro)
{
    if (!cls || cls->kind != N_ENTITY_DECL || prof > 32) return -1;
    for (int32_t i = 0; i < cls->lista2_alias.n; i++) {
        PSNode *f = cls->lista2_alias.itens[i];
        if (!f || !f->texto || strcmp(f->texto, nome) != 0) continue;
        if (f->kind == N_ENTITY_FIELD && !f->is_static && !f->a) { if (!*sem) *sem = cls; }
        else *outro = 1;
    }
    for (int32_t i = 0; i < cls->lista.n; i++) {
        PSNode *m = cls->lista.itens[i];
        if (m && m->kind == N_FUNCT_DECL && m->texto && strcmp(m->texto, nome) == 0) *outro = 1;
    }
    for (int32_t i = 0; i < cls->lista2.n; i++) {
        const char *pn = cls->lista2.itens[i] ? cls->lista2.itens[i]->texto : NULL;
        if (!pn || tp_campo_na_cadeia(c, tp_tipo_arq(c, pn), nome, prof + 1, sem, outro) < 0) return -1;
    }
    return 0;
}

/* `obj.x` lido com `obj` da classe `cls` do arquivo, e `x` campo do corpo
 * declarado SEM valor: rodando é AttributeError quando nada o gravou
 * (decisão 2 do Plano 6: o padrão nasce sempre, e o campo sem padrão só o
 * construtor gerado da classe que o declara grava — o da filha recebe só os
 * dela). É certo antes de rodar quando nada PODE gravá-lo em nenhuma classe
 * que o valor pode ser (`cls` e as filhas dela no arquivo):
 *   - nenhuma declaração de `x` na cadeia dá valor a ele ou o faz método;
 *   - o construtor de cada uma não é o gerado de uma classe que declara `x`,
 *     nem incerto (pai importado, `__init__` decorado);
 *   - o arquivo não grava `.x = v` com receptor nenhum, não declara campo
 *     `x` em funct, não chama `__init__` pelo nome (`base(Pai).__init__(v)`
 *     roda o gerado do pai) e não importa módulo `.pr`.
 * Devolve a classe que declara `x` sem valor (a frase a cita), ou NULL. */
static PSNode *tp_campo_nunca_nasce(C *c, PSNode *cls, const char *nome)
{
    /* nome de tipo usado por mais de uma declaração: não se sabe que classes
     * o valor pode ser */
    if (!cls || !cls->texto || c->importa_pr || c->chama_init_nome || c->ntipos_ambiguos > 0) return NULL;
    for (int32_t i = 0; i < c->nmembros_gravados; i++)
        if (strcmp(c->membros_gravados[i], nome) == 0) return NULL;
    PSNode *dona = NULL;
    for (int32_t k = 0; k < c->ntipos_arq; k++) {
        PSNode *d = c->tipos_arq[k];
        if (!d || d->kind != N_ENTITY_DECL || !d->texto || tp_herda(c, d->texto, cls->texto, 0) != 1) continue;
        PSNode *sem = NULL;
        int outro = 0;
        if (tp_campo_na_cadeia(c, d, nome, 0, &sem, &outro) < 0 || outro || !sem) return NULL;
        PSNode *no = NULL;
        int r = tp_construtor(c, d, NULL, &no, 0);
        if (r == CONS_INCERTO) return NULL;
        if (r == CONS_GERADO && no)
            for (int32_t i = 0; i < no->lista2_alias.n; i++) {
                PSNode *f = no->lista2_alias.itens[i];
                if (f && f->texto && !f->is_static && strcmp(f->texto, nome) == 0) return NULL;
            }
        if (d == cls || !dona) dona = sem;
    }
    return dona;
}

static void tp_confere_args(C *c, Unidade *u, PSNode *call, const char *nome, PSNodeVec *params,
                            int recebe, int oculto, const char *classe);

/* Confere a chamada `call` contra o construtor que `tp_construtor` achou.
 * `classe` é a classe instanciada: é ela que a frase do campo cita (a da VM,
 * `campo x de Filha`, mesmo com o gerado sendo do pai). */
static void tp_confere_construtor(C *c, Unidade *u, PSNode *call, const char *classe, int r, PSNode *no)
{
    if (r == CONS_ESCRITO) { tp_confere_args(c, u, call, "__init__", &no->lista, 1, 0, NULL); return; }
    if (r == CONS_GERADO) { tp_confere_args(c, u, call, "__init__", &no->lista2_alias, 1, 0, classe); return; }
    if (r != CONS_NENHUM || call->lista.n == 0) return;
    int npos = 0, nomeado = 0;
    for (int32_t i = 0; i < call->lista.n; i++) {
        PSNode *a = call->lista.itens[i];
        if (a->i2) return;                       /* espalhamento: só rodando */
        if (a->texto) nomeado = 1; else npos++;
    }
    /* as frases da VM (OP_CALL e OP_CALL_KW) */
    if (nomeado) terro(c, call, "TypeError", "%s() takes no arguments", classe);
    else terro(c, call, "TypeError", "%s() takes no arguments (%d given)", classe, npos);
}

/* A frase de "faltou argumento" da VM (`lista_faltantes`): 'a', 'b' and 'c'. */
static void tp_lista_faltantes(char *buf, size_t cap, PSNode **fix, const int *marcado, int nfix)
{
    buf[0] = '\0';
    int quantos = 0, escritos = 0;
    for (int k = 0; k < nfix; k++) if (!marcado[k] && !fix[k]->a) quantos++;
    for (int k = 0; k < nfix; k++) {
        if (marcado[k] || fix[k]->a) continue;
        const char *sep = escritos == 0 ? "" : (escritos == quantos - 1 ? (quantos == 2 ? " and " : ", and ") : ", ");
        size_t u = strlen(buf);
        if (u + 8 >= cap) break;
        snprintf(buf + u, cap - u, "%s'%s'", sep, fix[k]->texto ? fix[k]->texto : "?");
        escritos++;
    }
}

/* Os argumentos de `call` contra os parâmetros `params`: quantos, quais
 * nomes, quais faltam e o tipo de cada um — as mesmas regras e as mesmas
 * frases do binding da VM (`liga_args`), antes de rodar.
 *   `recebe`: a chamada põe o receptor no slot 0 (método de instância,
 *             construtor) — ele ocupa o 1º parâmetro fixo, ou cai no `*args`
 *             se o 1º parâmetro não for fixo;
 *   `oculto`: `Classe.metodo()` de um `@static` cujo 1º parâmetro é `self` —
 *             o buraco ocupa o `self` e some da frase de quantidade;
 *   `classe`: construtor GERADO dos campos (`params` são os campos, o `self`
 *             vem antes deles, e o tipo errado sai com a frase do campo). */
static void tp_confere_args(C *c, Unidade *u, PSNode *call, const char *nome, PSNodeVec *params,
                            int recebe, int oculto, const char *classe)
{
    PSNode *fix[PS_MAX_PARAMS];
    int nfix = 0, tem_var = 0, tem_kw = 0, ndef = 0;
    if (classe) fix[nfix++] = NULL;                /* o `self` do __init__ gerado */
    for (int32_t i = 0; i < params->n && nfix < PS_MAX_PARAMS; i++) {
        PSNode *p = params->itens[i];
        if (classe && p->is_static) continue;
        if (!classe && p->i2 == 1) { tem_var = 1; continue; }
        if (!classe && p->i2 == 2) { tem_kw = 1; continue; }
        fix[nfix++] = p;
        if (p->a) ndef++;
    }
    int desloca = recebe || oculto;
    int primeiro_fixo = classe || (params->n > 0 && params->itens[0]->i2 == 0);
    int base = (desloca && primeiro_fixo) ? 1 : 0;   /* 1º fixo que a chamada preenche */
    int npos = 0;
    for (int32_t i = 0; i < call->lista.n; i++) {
        PSNode *a = call->lista.itens[i];
        if (a->i2) return;                       /* espalhamento: só rodando */
        if (!a->texto) npos++;
    }
    int dado = desloca + npos;
    if (dado > nfix && !tem_var) {
        int maxpos = nfix - oculto, dados = dado - oculto;
        if (ndef > 0)
            terro(c, call, "TypeError", "%s() takes from %d to %d positional arguments but %d %s given",
                  nome, maxpos - ndef, maxpos, dados, dados == 1 ? "was" : "were");
        else
            terro(c, call, "TypeError", "%s() takes %d positional argument%s but %d %s given",
                  nome, maxpos, maxpos == 1 ? "" : "s", dados, dados == 1 ? "was" : "were");
        return;
    }
    int marcado[PS_MAX_PARAMS];
    PSNode *valor[PS_MAX_PARAMS];
    memset(marcado, 0, sizeof(marcado));
    memset(valor, 0, sizeof(valor));
    for (int q = 0; q < base; q++) marcado[q] = 1;       /* o receptor */
    int k = base;
    for (int32_t i = 0; i < call->lista.n; i++) {
        PSNode *a = call->lista.itens[i];
        if (a->texto) continue;
        if (k < nfix) { marcado[k] = 1; valor[k] = a->a; }
        k++;
    }
    int kw_errado = 0;
    for (int32_t i = 0; i < call->lista.n; i++) {
        PSNode *a = call->lista.itens[i];
        if (!a->texto) continue;
        int achou = -1;
        for (int q = base; q < nfix; q++)
            if (fix[q] && fix[q]->texto && !strcmp(fix[q]->texto, a->texto)) { achou = q; break; }
        if (achou >= 0) {
            /* o parâmetro já ligado (por posição, ou o self) — a frase da VM
             * (liga_args), na mesma ordem dos nomeados */
            if (marcado[achou]) {
                terro(c, a, "TypeError", "%s() got multiple values for argument '%s'", nome, a->texto);
                kw_errado = 1;
                continue;
            }
            marcado[achou] = 1; valor[achou] = a->a; continue;
        }
        if (!tem_kw) {
            terro(c, a, "TypeError", "%s() got an unexpected keyword argument '%s'", nome, a->texto);
            kw_errado = 1;
        }
    }
    /* a VM confere o nomeado errado ANTES do que falta e para nele: a mesma
     * ordem aqui, senão a primeira frase da lista difere da que sai rodando */
    int faltam = 0;
    for (int q = base; q < nfix; q++) if (!marcado[q] && !fix[q]->a) faltam++;
    if (faltam > 0 && !kw_errado) {
        char lista[256];
        tp_lista_faltantes(lista, sizeof(lista), fix + base, marcado + base, nfix - base);
        terro(c, call, "TypeError", "%s() missing %d required positional argument%s: %s",
              nome, faltam, faltam == 1 ? "" : "s", lista);
    }
    for (int q = base; q < nfix; q++) {
        if (!marcado[q] || !fix[q]->texto2 || !valor[q]) continue;
        const char *T = tp_canon(fix[q]->texto2);
        const char *vt = tp_de(c, u, valor[q]);
        if (tp_aceita(c, T, vt, valor[q], 1) != T_NAO) continue;
        if (classe)
            terro(c, valor[q], "AttributedValueError", "campo %s de %s esperava %s, recebeu %s",
                  fix[q]->texto ? fix[q]->texto : "?", classe, T, vt);
        else
            terro(c, valor[q], "AttributedValueError", "parâmetro %s de %s() esperava %s, recebeu %s",
                  fix[q]->texto ? fix[q]->texto : "?", nome, T, vt);
    }
}

/* A chamada `n` tem assinatura conhecida? Funct do arquivo, construtor de
 * Entity do arquivo (o `__init__` escrito ou o gerado dos campos), método de
 * instância e `Classe.metodo_static()`. Decorado por decorador geral não: o
 * decorador pode ter trocado a funct. */
/* A chamada a um nativo MEDIDO (scripts/mede_aridades.pr, a tabela de
 * ps_retornos.c), conferida antes de rodar pela regra que a VM aplica
 * rodando, com a frase dela: os nomeados pela lista de nomes do nativo (a do
 * `liga_nomeados_nat`), a quantidade, e — com um argumento só — o tipo dele.
 * Argumento espalhado (`*x`, `**d`) só rodando se sabe. `metodo` é o tipo do
 * dono quando é método: a frase de "sem nome" leva ele
 * (`str.upper() takes no keyword arguments`). */
static void tp_confere_nativo(C *c, PSNode *n, const char *dono, const char *membro,
                              const char *metodo, const char *t_unico)
{
    PSAridade ar;
    if (!ps_aridade_de(dono, membro, &ar)) return;
    const PSAridade *a = &ar;
    int npos = 0, nkw = 0;
    for (int32_t i = 0; i < n->lista.n; i++) {
        PSNode *arg = n->lista.itens[i];
        if (arg->i2) return;
        if (arg->texto) nkw++; else npos++;
    }
    int dados = npos;
    if (nkw > 0) {
        if (!a->params) {
            if (metodo) terro(c, n, "TypeError", "%s.%s() takes no keyword arguments", metodo, membro);
            else        terro(c, n, "TypeError", "%s() takes no keyword arguments", membro);
            return;
        }
        int veio[16];
        for (int k = 0; k < 16; k++) veio[k] = k < npos;
        for (int32_t i = 0; i < n->lista.n; i++) {
            PSNode *arg = n->lista.itens[i];
            if (!arg->texto) continue;
            int idx = 0, achou = -1;
            for (const char *q = a->params; *q; idx++) {
                const char *fim = strchr(q, ',');
                size_t tam = fim ? (size_t)(fim - q) : strlen(q);
                const char *ig = memchr(q, '=', tam);
                size_t tn = ig ? (size_t)(ig - q) : tam;
                if (strlen(arg->texto) == tn && !strncmp(q, arg->texto, tn)) { achou = idx; break; }
                q = fim ? fim + 1 : q + tam;
            }
            if (achou < 0 || achou >= 16) {
                terro(c, arg, "TypeError", "'%s' is an invalid keyword argument for %s()", arg->texto, membro);
                return;
            }
            if (veio[achou]) {
                terro(c, arg, "TypeError", "%s() got multiple values for argument '%s'", membro, arg->texto);
                return;
            }
            veio[achou] = 1;
            if (achou + 1 > dados) dados = achou + 1;
        }
        int idx = 0;
        for (const char *q = a->params; *q && idx < dados; idx++) {
            const char *fim = strchr(q, ',');
            size_t tam = fim ? (size_t)(fim - q) : strlen(q);
            if (!veio[idx] && !memchr(q, '=', tam)) {
                terro(c, n, "TypeError", "%s() missing required argument '%.*s'", membro, (int)tam, q);
                return;
            }
            q = fim ? fim + 1 : q + tam;
        }
    }
    if (dados < a->minimo || (a->maximo >= 0 && dados > a->maximo)) {
        char frase[160];
        ps_frase_aridade(frase, sizeof(frase), a->nome, a->minimo, a->maximo, dados);
        terro(c, n, "TypeError", "%s", frase);
        return;
    }
    if (npos == 1 && nkw == 0) {
        int k = ps_aridade_tipo(tp_basico(t_unico));
        if (k >= 0 && a->recusa[k]) terro(c, n->lista.itens[0], "TypeError", "%s", a->recusa[k]);
    }
}

static void tp_confere_chamada(C *c, Unidade *u, PSNode *n)
{
    PSNode *f = n->a;
    if (!f) return;
    /* o tipo do argumento único, pra recusa de tipo do nativo */
    const char *t_unico = (n->lista.n == 1 && n->lista.itens[0]->a && !n->lista.itens[0]->texto)
                          ? tp_de(c, u, n->lista.itens[0]->a) : NULL;
    if (f->kind == N_NAME && f->texto) {
        SimInfo *s = tp_sim_de(c, u, f->texto);
        if (!s || !s->decl || s->decorado) {
            /* builtin que o programa não sombreou: `len(5)`, `chr("a")`; e o
             * membro de módulo trazido pelo nome (`from os import getenv`) */
            if (!tp_nome_ligado(c, u, f->texto) && ps_nome_pre_ligado(f->texto))
                tp_confere_nativo(c, n, "builtins", f->texto, NULL, t_unico);
            else if (s && s->mod_nativo && s->membro_nativo)
                tp_confere_nativo(c, n, s->mod_nativo, s->membro_nativo, NULL, t_unico);
            return;
        }
        if (s->decl->kind == N_FUNCT_DECL) {
            tp_confere_args(c, u, n, s->decl->texto ? s->decl->texto : "?", &s->decl->lista, 0, 0, NULL);
            return;
        }
        if (s->decl->kind != N_ENTITY_DECL || !s->decl->texto) return;
        const char *cls = s->decl->texto;
        /* construir é chamar o `__init__`: `private funct __init__` só deixa
         * construir de dentro da classe (a fábrica, o singleton) */
        tp_confere_privado(c, s->decl, "__init__", 1, f);
        /* o construtor que roda: o escrito, o gerado de quem tem campo, ou
         * nenhum (`Z(1)` com Z sem construtor) */
        PSNode *no = NULL;
        int r = tp_construtor(c, s->decl, NULL, &no, 0);
        tp_confere_construtor(c, u, n, cls, r, no);
        return;
    }
    if (f->kind != N_MEMBER_ACCESS || !f->texto || !f->a) return;
    if (f->a->kind == N_BASE_CALL_NODE) {
        /* `base(Pai).__init__(...)` / `base(Pai).metodo(...)`: o método do
         * pai com o self na frente, conferido como `A(...)` e `p.m(...)` */
        const char *pai = base_pai_nome(c, f->a);
        PSNode *d = pai ? tp_tipo_arq(c, pai) : NULL;
        if (!d || d->kind != N_ENTITY_DECL) return;
        if (strcmp(f->texto, "__init__") == 0) {
            /* o construtor do pai pela mesma regra de `Pai(...)`; o campo do
             * gerado é gravado no `self`, da classe deste método */
            PSNode *no = NULL;
            int r = tp_construtor(c, d, NULL, &no, 0);
            if (r == CONS_NENHUM)
                terro(c, f, "TypeError", "base(): a Entity pai '%s' nao tem __init__", pai);
            else
                tp_confere_construtor(c, u, n, c->entity_no && c->entity_no->texto ? c->entity_no->texto : pai,
                                      r, no);
            return;
        }
        PSNode *m = tp_metodo(c, pai, f->texto, 0);
        if (!m) {
            if (tp_classe_tem(c, pai, f->texto, 0) == 0) {
                char dica[160];
                /* aqui é sempre `base(Pai).m(...)`: uma chamada */
                terro(c, f, "AttributeError", "'%s' object has no method '%s'%s", pai, f->texto,
                      tp_dica_membro(c, pai, f->texto, dica, sizeof(dica)));
            }
            return;
        }
        if (tp_metodo_decorado(c, pai, m) || tp_campo_tipo(c, pai, f->texto, 0)) return;
        tp_confere_args(c, u, n, m->texto ? m->texto : "?", &m->lista,
                        tp_metodo_estatico(c, pai, m) ? 0 : 1, 0, NULL);
        return;
    }
    {
        /* `json.parse(5)`, `sys.exit(1, 2)`: função de módulo nativo */
        char mb[64];
        const char *mod = tp_modulo_de(c, u, f->a, mb, sizeof(mb));
        if (mod) {
            tp_confere_nativo(c, n, mod, f->texto, NULL, t_unico);
            return;
        }
    }
    {
        /* `util.soma(...)` / `util.Conta(...)` de um `.pr` importado: a
         * assinatura vem da AST do módulo (funct decorada por decorador
         * geral não vale — pode ter sido trocada) */
        SimInfo *sm = tp_sim_mod_pr(c, u, f->a);
        if (sm) {
            int deco = 0;
            PSNode *d = modulo_decl(sm->mod_pr, f->texto, &deco);
            if (!d || deco) return;
            if (d->kind == N_FUNCT_DECL) {
                tp_confere_args(c, u, n, d->texto ? d->texto : "?", &d->lista, 0, 0, NULL);
                return;
            }
            if (d->kind == N_ENTITY_DECL && tp_classe_importada(c, d)) {
                tp_confere_privado(c, d, "__init__", 1, f);
                /* o construtor pela regra de `Classe(...)`, com os pais
                 * procurados no próprio módulo */
                PSNode *no = NULL;
                int r = tp_construtor(c, d, sm->mod_pr, &no, 0);
                tp_confere_construtor(c, u, n, d->texto ? d->texto : "?", r, no);
            }
            return;
        }
    }
    if (f->a->kind == N_NAME && f->a->texto) {
        SimInfo *s = tp_sim_de(c, u, f->a->texto);
        if (s && s->decl && s->decl->kind == N_ENTITY_DECL && s->decl->texto && !s->decorado) {
            const char *cls = s->decl->texto;
            PSNode *m = tp_metodo(c, cls, f->texto, 0);
            if (!m || tp_metodo_decorado(c, cls, m) || tp_campo_tipo(c, cls, f->texto, 0)) return;
            if (!tp_metodo_estatico(c, cls, m)) {
                terro(c, f, "RuntimeError", "Entity '%s' não tem método estático '%s' — instancie primeiro",
                      cls, f->texto);
                return;
            }
            int self = m->lista.n > 0 && m->lista.itens[0]->texto && !strcmp(m->lista.itens[0]->texto, "self");
            tp_confere_args(c, u, n, m->texto ? m->texto : "?", &m->lista, 0, self, NULL);
            return;
        }
    }
    const char *t = tp_de(c, u, f->a);
    /* O valor de um ENUM não tem método nenhum (motor-17): `Cor.RED.foo()`
     * passava e rodando dava `'int' object has no method 'foo'`. O enum é um
     * tipo do arquivo, e por isso caía fora da conferência de nativo abaixo. */
    if (t && !strchr(t, '|')) {
        PSNode *en = tp_tipo_arq(c, t);
        if (en && en->kind == N_ENUM_DECL) {
            terro(c, f, "AttributeError", "'int' object has no method '%s'", f->texto);
            return;
        }
    }
    if (t && !strchr(t, '|') && !tp_tipo_arq(c, t)) {
        /* método de valor nativo: `"a".upper(1)`, `[1].index(1, 2, 3, 4)`.
         * `char` e `long` rodam como str e int */
        const char *dono = tp_basico(t) ? tp_basico(t) : t;
        /* A PROPRIEDADE chamada como método (motor-12): `f.name()` com
         * `name` valendo `str` dava `PoolFile.name() takes no keyword
         * arguments`, como se fosse método sem parâmetro, e rodando dava
         * `'str' object is not callable`. O `ps_nativo_tem_membro` devolve 2
         * para valor e 1 para método; o 2 aqui é a chamada do que não se
         * chama. */
        /* O dict fica de fora: `d.chave` é acesso à chave, e `d.type()` é a
         * chave `type` chamada — o erro é o KeyError da chave que não existe,
         * não "não chamável" (o caso ".type() num dict e a chave type"). */
        if (strcmp(dono, "dict") != 0 && ps_nativo_tem_membro(dono, f->texto) == 2) {
            const char *vt = tp_nativo(dono, f->texto);
            terro(c, f, "TypeError", "'%s' object is not callable",
                  vt ? vt : "valor");
            return;
        }
        if (ps_aridade_de(dono, f->texto, NULL)) tp_confere_nativo(c, n, dono, f->texto, dono, t_unico);
        return;
    }
    if (!t || strchr(t, '|')) return;
    PSNode *m = tp_metodo(c, t, f->texto, 0);
    if (!m || tp_metodo_decorado(c, t, m) || tp_metodo_estatico(c, t, m)) return;
    if (tp_campo_tipo(c, t, f->texto, 0)) return;          /* campo ganha de método */
    tp_confere_args(c, u, n, m->texto ? m->texto : "?", &m->lista, 1, 0, NULL);
}

/* A chamada é SAÍDA (`post`, `sys.stdout.write`, `f.write` num PoolFile,
 * `os.writeFile`, `os.warn`)? Devolve o índice do posicional que sai (-2 =
 * todos), ou -1 se não é saída. É o que a funct tipada confere: ela só dá
 * saída do tipo dela. */
static int tp_saida_do_alvo(C *c, Unidade *u, PSNode *f)
{
    if (!f) return -1;
    if (f->kind == N_NAME && f->texto)
        return (!strcmp(f->texto, "post") && !tp_nome_ligado(c, u, "post")) ? -2 : -1;
    if (f->kind != N_MEMBER_ACCESS || !f->texto || !f->a) return -1;
    char buf[64];
    const char *mod = tp_modulo_de(c, u, f->a, buf, sizeof(buf));
    if (mod) {
        if ((!strcmp(mod, "sys.stdout") || !strcmp(mod, "sys.stderr"))
                && (!strcmp(f->texto, "write") || !strcmp(f->texto, "writeln"))) return -2;
        if (!strcmp(mod, "os") && !strcmp(f->texto, "writeFile")) return 1;
        if (!strcmp(mod, "os") && !strcmp(f->texto, "warn")) return 0;
        return -1;
    }
    const char *t = tp_de(c, u, f->a);
    if (t && !strcmp(t, "PoolFile") && !strcmp(f->texto, "write")) return 0;
    return -1;
}

static int tp_arg_de_saida(C *c, Unidade *u, PSNode *call)
{
    return tp_saida_do_alvo(c, u, call->a);
}

/* A saída como VALOR (`Object p = post`, `map(l, post)`) escaparia da regra
 * da funct tipada: quem escreve depois não é a chamada que a passada confere.
 * Conferir rodando pedia ao laço da VM dizer ao nativo em que funct ele está,
 * e custou +4% a +6% (medido no lote 6). Decisão dele (2026-10-08): erro
 * antes de rodar, na funct que devolve valor. `n` não é o alvo da chamada em
 * curso (`c->alvo_chamado`): esse é a chamada direta, conferida. */
static void tp_saida_como_valor(C *c, Unidade *u, PSNode *n)
{
    if (!u->tipo_ret_nome || !strcmp(u->tipo_ret_nome, "Null") || n == c->alvo_chamado) return;
    if (tp_saida_do_alvo(c, u, n) == -1) return;
    /* o nome como foi escrito: `sys.stdout.write`, `os.warn`, `f.write` */
    const PSNode *partes[8];
    int np = 0;
    for (const PSNode *q = n; q && np < 8; q = q->a) {
        partes[np++] = q;
        if (q->kind != N_MEMBER_ACCESS) break;
    }
    char nome[160];
    size_t t = 0;
    nome[0] = '\0';
    for (int i = np - 1; i >= 0 && t < sizeof(nome); i--) {
        int k = snprintf(nome + t, sizeof(nome) - t, "%s%s", i == np - 1 ? "" : ".",
                         partes[i]->texto ? partes[i]->texto : "?");
        if (k < 0) break;
        t += (size_t)k;
    }
    terro(c, n, "AttributedValueError",
          "%s como valor em %s(), que devolve %s: a saída dele não seria conferida (chame %s direto)",
          nome, u->nome_funct ? u->nome_funct : "?", u->tipo_ret_nome, nome);
}

/* Funct tipada: os argumentos de saída da chamada `n`, antes de rodar. O que
 * só se sabe rodando fica marcado em `c->saida_*` pro `emite_args_e_chama`
 * emitir a conferência logo depois de avaliar cada argumento. */
static void tp_confere_saida(C *c, Unidade *u, PSNode *n)
{
    c->saida_tipo = NULL;
    /* `Null funct` não devolve valor: a regra da saída é de funct que devolve
     * (decisão dele, 2026-10-05: `post` de qualquer coisa vale nela) */
    if (!u->tipo_ret_nome || !strcmp(u->tipo_ret_nome, "Null")) return;
    int idx = tp_arg_de_saida(c, u, n);
    if (idx == -1) return;
    int pos = 0, talvez = 0;
    for (int32_t i = 0; i < n->lista.n; i++) {
        PSNode *a = n->lista.itens[i];
        if (a->texto || a->i2) continue;
        if (idx == -2 || idx == pos) {
            const char *vt = tp_de(c, u, a->a);
            int v = tp_aceita(c, u->tipo_ret_nome, vt, a->a, 1);
            if (v == T_NAO)
                terro(c, a->a, "AttributedValueError", "saída de %s() esperava %s, recebeu %s",
                      u->nome_funct ? u->nome_funct : "?", u->tipo_ret_nome, vt);
            else if (v == T_TALVEZ) talvez = 1;
        }
        pos++;
    }
    if (!talvez) return;
    c->saida_tipo = u->tipo_ret_nome;
    c->saida_indice = idx;
    snprintf(c->saida_rotulo, sizeof(c->saida_rotulo), "saída de %s()", u->nome_funct ? u->nome_funct : "?");
}

/* O código da tabela (TIPO_* + 1) pro OP_COERCE_DECL de um nome DECLARADO;
 * 0 pra nome sem tipo escrito ou com tipo fora da tabela (classe, objeto
 * nativo), que vão pelo OP_CONFERE_TIPO. Sem o que se sabe estaticamente,
 * vale o código que o chamador já tinha. */
static int tp_cod1(SimInfo *s, int cod_antigo)
{
    if (!s) return cod_antigo;
    if (s->estado != 2 || !s->tipo) return 0;
    int k = ps_tipo_codigo(s->tipo);
    return k >= 0 ? k + 1 : 0;
}

/* O SimInfo do slot da função que ENVOLVE (a dona da célula capturada). */
static SimInfo *tp_sim_upval(Unidade *u, const char *nome)
{
    for (Unidade *q = u->pai; q && !q->eh_modulo; q = q->pai)
        for (int32_t i = q->nlocais - 1; i >= 0; i--)
            if (strcmp(q->locais[i], nome) == 0) return &q->sim[i];
    return NULL;
}

static void guarda_nome_modo_no(C *c, Unidade *u, const char *nome, int certa)
{
    if (u->eh_modulo) {
        mod_criados_add(c, u, nome);  /* p/ escopo de bloco */
        int32_t km = ix_busca(&u->ix_mod, nome);
        SimInfo *s = km >= 0 ? &u->mod_sim[km] : NULL;
        int v = tp_escreve(c, s, nome);
        tp_emite_escrita(c, u, nome, s, v, tp_cod1(s, tipo_topo_de(c, nome)));
        /* variável de bloco capturada: grava na célula (a conferência de tipo
         * acima é a mesma da global) */
        int32_t sc = celula_ativa_do_topo(u, nome);
        if (sc >= 0) emite(c, u, OP_CELL_SET, sc);
        else emite(c, u, OP_STORE_GLOBAL, idx_global(c, nome));
        return;
    }
    if (eh_global_declarada(u, nome)) {
        SimInfo *s = tp_sim_topo(c, nome);
        int v = tp_escreve(c, s, nome);
        tp_emite_escrita(c, u, nome, s, v, tp_cod1(s, tipo_topo_de(c, nome)));
        global_gravado_add(c, nome);
        emite(c, u, OP_STORE_GLOBAL, idx_global(c, nome));
        return;
    }
    /* Escrita num nome que vem de fora vai pra célula capturada — é o mesmo
     * critério que a linguagem já usava pra global ("se já existe lá fora,
     * escreve lá"), agora valendo também pro escopo da função que envolve.
     * Mas o nome que ESTA funct já tem (o parâmetro, a variável dela, a
     * célula do bloco ou do laço) é o dela, como na leitura: a escrita no
     * parâmetro de mesmo nome de uma variável capturada lá fora ia pra ela. */
    if (slot_existente(u, nome) < 0) {
        int32_t up = resolve_upval(c, u, nome);
        if (up >= 0) {
            SimInfo *s = tp_sim_upval(u, nome);
            int v = tp_escreve(c, s, nome);
            tp_emite_escrita(c, u, nome, s, v, tp_cod1(s, tipo_de_upval(u, nome)));
            emite(c, u, OP_STORE_UPVAL, up);
            return;
        }
    }
    int32_t i = idx_local(c, u, nome);
    if (i < 0) return;
    if (certa) u->certo[i] = 1;
    u->celula_virgem[i] = 0;
    /* Nome sem tipo que o ARQUIVO também liga: o STORE_NAME grava lá
     * (write-through), então é o tipo de lá que vale. Célula não: o
     * CELL_SET_NAME grava sempre nela — é local desta função. */
    SimInfo *s = &u->sim[i];
    if (!u->certo[i] && !u->celula[i]) {
        SimInfo *t = tp_sim_topo(c, nome);
        if (t) s = t;
    }
    int v = tp_escreve(c, s, nome);
    if (u->celula[i]) {
        tp_emite_escrita(c, u, nome, s, v, tp_cod1(s, u->tipo_decl[i]));
        if (u->certo[i]) { emite(c, u, OP_CELL_SET, i); return; }
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, i, 0, NULL, 0));
        emite(c, u, OP_CELL_SET_NAME, idx_global(c, nome));
        return;
    }
    if (u->certo[i]) {
        tp_emite_escrita(c, u, nome, s, v, tp_cod1(s, u->tipo_decl[i]));
        emite(c, u, OP_STORE_LOCAL, i);
        return;
    }
    /* STORE_NAME decide em runtime entre local novo e global existente: se o
     * nome e um global DECLARADO com tipo no topo do arquivo, e nele que a
     * escrita vai cair (write-through), entao confere pelo tipo dele. */
    tp_emite_escrita(c, u, nome, s, v,
                     tp_cod1(s, u->tipo_decl[i] ? u->tipo_decl[i] : tipo_topo_de(c, nome)));
    emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, i, 0, NULL, 0));
    emite(c, u, OP_STORE_NAME, idx_global(c, nome));
}

/* O valor a gravar em `c->grava` vale pra UMA gravação: esta. */
static void guarda_nome_modo(C *c, Unidade *u, const char *nome, int certa)
{
    guarda_nome_modo_no(c, u, nome, certa);
    memset(&c->grava, 0, sizeof(c->grava));
}

/* ── parâmetros do campo de `model` (regex, in, not_in, min, max, optional, of) ──
 * Tudo aqui é constante do model, decidida na compilação: o valor tem que ser
 * literal (número com sinal, texto, true/false, lista de literais) e caber no
 * tipo do campo. O que não cabe é erro na linha do parâmetro — antes de
 * rodar, como o resto da tipagem. */
static int mlit_numero(PSNode *v, double *out, int *inteiro)
{
    if (!v) return 0;
    int neg = 0;
    if (v->kind == N_UNARY_OP && v->texto && !strcmp(v->texto, "-") && v->a) { neg = 1; v = v->a; }
    if (v->kind != N_LITERAL) return 0;
    if (v->lit == L_INT) { *out = neg ? -(double)v->i : (double)v->i; *inteiro = 1; return 1; }
    if (v->lit == L_FLO) { *out = neg ? -v->d : v->d; *inteiro = 0; return 1; }
    return 0;
}

static int mlit_de(PSNode *v, int32_t tipo, PSModelLit *out)
{
    memset(out, 0, sizeof(*out));
    out->tipo = tipo;
    double d = 0; int inteiro = 0;
    switch (tipo) {
        case PS_TIPO_STR:
            if (v && v->kind == N_LITERAL && v->lit == L_STR && v->texto) { out->s = strdup(v->texto); return out->s != NULL; }
            return 0;
        case PS_TIPO_BOOL:
            if (v && v->kind == N_LITERAL && v->lit == L_BOOL) { out->i = v->i; return 1; }
            return 0;
        case PS_TIPO_INT:
            if (mlit_numero(v, &d, &inteiro) && inteiro) { out->i = (int64_t)d; return 1; }
            return 0;
        case PS_TIPO_FLO:
            if (mlit_numero(v, &d, &inteiro)) { out->d = d; return 1; }
            return 0;
        default:
            return 0;
    }
}

/* `nome` é um model — deste arquivo (em qualquer ponto dele) ou importado? */
static int modelo_conhecido(C *c, Unidade *u, const char *nome)
{
    PSNode *d = tp_tipo_decl_arq(c, nome);
    if (d && d->kind == N_MODEL_DECL) return 1;
    SimInfo *s = tp_nome_ligado(c, u, nome) ? tp_sim_de(c, u, nome) : NULL;
    return s && s->decl && s->decl->kind == N_MODEL_DECL;
}

static void mlits_solta(PSModelLit *v, int32_t n)
{
    for (int32_t i = 0; i < n; i++) free(v[i].s);
    free(v);
}

/* Os parâmetros escritos no campo `f` (`lista`: nome em `texto`, valor em `a`)
 * vão pro descritor `cd`. -1 = erro, já acusado. */
static int compila_params_model(C *c, Unidade *u, PSNode *f, PSModelCampoDef *cd)
{
    const char *nc = f->texto ? f->texto : "?";
    int32_t t = cd->tipo;
    if (cd->length >= 0 && t != PS_TIPO_STR && t != PS_TIPO_INT && t != PS_TIPO_LIST) {
        cerro_sx(c, f, "parametro 'length' so vale em str, int e list (campo %s)", nc); return -1;
    }
    for (int32_t k = 0; k < f->lista.n; k++) {
        PSNode *par = f->lista.itens[k];
        const char *pn = par->texto ? par->texto : "?";
        PSNode *v = par->a;
        if (!strcmp(pn, "regex")) {
            if (t != PS_TIPO_STR) { cerro_sx(c, par, "parametro 'regex' so vale em str (campo %s)", nc); return -1; }
            if (!v || v->kind != N_LITERAL || v->lit != L_STR || !v->texto) {
                cerro_sx(c, par, "parametro 'regex' do campo %s: so literal de texto", nc); return -1;
            }
            char e[256] = {0};
            int len = v->texto_len > 0 ? v->texto_len : (int)strlen(v->texto);
            PSRegex *r = ps_regex_compila(v->texto, len, e, sizeof(e));
            if (!r) { cerro_sx(c, par, "regex invalida no campo %s: %s", nc, e[0] ? e : "padrao invalido"); return -1; }
            ps_regex_free(r);
            cd->regex = strdup(v->texto);
            if (!cd->regex) { cerro(c, "sem memoria", par); return -1; }
        } else if (!strcmp(pn, "in") || !strcmp(pn, "not_in")) {
            if (t != PS_TIPO_STR && t != PS_TIPO_INT && t != PS_TIPO_FLO && t != PS_TIPO_BOOL) {
                cerro_sx(c, par, "parametro '%s' so vale em str, int, flo e bool (campo %s)", pn, nc); return -1;
            }
            if (!v || v->kind != N_LIST_LITERAL || v->lista.n == 0) {
                cerro_sx(c, par, "parametro '%s' do campo %s: so lista de literais, com pelo menos um", pn, nc); return -1;
            }
            PSModelLit *ls = calloc((size_t)v->lista.n, sizeof(PSModelLit));
            if (!ls) { cerro(c, "sem memoria", par); return -1; }
            for (int32_t i = 0; i < v->lista.n; i++) {
                if (mlit_de(v->lista.itens[i], t, &ls[i])) continue;
                cerro_sx(c, v->lista.itens[i] ? v->lista.itens[i] : par,
                         "parametro '%s' do campo %s: cada valor tem que ser um literal %s", pn, nc,
                         f->texto2 ? f->texto2 : "?");
                mlits_solta(ls, v->lista.n);
                return -1;
            }
            if (pn[0] == 'i') { cd->in = ls; cd->n_in = v->lista.n; }
            else              { cd->not_in = ls; cd->n_not_in = v->lista.n; }
        } else if (!strcmp(pn, "min") || !strcmp(pn, "max")) {
            if (t != PS_TIPO_INT && t != PS_TIPO_FLO) {
                cerro_sx(c, par, "parametro '%s' so vale em int e flo (campo %s)", pn, nc); return -1;
            }
            double d = 0; int inteiro = 0;
            if (!mlit_numero(v, &d, &inteiro)) { cerro_sx(c, par, "parametro '%s' do campo %s: so numero literal", pn, nc); return -1; }
            if (pn[1] == 'i') { cd->tem_min = 1; cd->min = d; } else { cd->tem_max = 1; cd->max = d; }
        } else if (!strcmp(pn, "optional")) {
            if (!v || v->kind != N_LITERAL || v->lit != L_BOOL) {
                cerro_sx(c, par, "parametro 'optional' do campo %s: so True ou False", nc); return -1;
            }
            cd->optional = v->i != 0;
        } else if (!strcmp(pn, "of")) {
            if (t != PS_TIPO_LIST) { cerro_sx(c, par, "parametro 'of' so vale em list (campo %s)", nc); return -1; }
            const char *tn = (v && v->kind == N_NAME) ? v->texto : NULL;
            const PSTipoInfo *oti = tn ? ps_tipo_info(tn) : NULL;
            if (oti && oti->model && oti->cod != PS_TIPO_LIST) cd->of_tipo = oti->cod;
            else if (tn && !oti && modelo_conhecido(c, u, tn)) {
                cd->of_tipo = -2;
                cd->of_model = strdup(tn);
                if (!cd->of_model) { cerro(c, "sem memoria", par); return -1; }
            } else {
                cerro_sx(c, par, "parametro 'of' do campo %s: tipo desconhecido %s", nc, tn ? tn : "?"); return -1;
            }
        }
    }
    if (cd->tem_min && cd->tem_max && cd->min > cd->max) {
        cerro_sx(c, f, "campo %s: min maior que max", nc); return -1;
    }
    return 0;
}

/* Prepara a próxima gravação com o valor `no` (o tipo é o que se sabe dele
 * AGORA, antes de a escrita mudar o que se sabe do nome). */
static void grava_valor(C *c, Unidade *u, PSNode *no)
{
    memset(&c->grava, 0, sizeof(c->grava));
    c->grava.tem_valor = 1;
    c->grava.no = no;
    c->grava.tipo = tp_de(c, u, no);
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
/* Carimba linha/coluna em TODA a sub-árvore.
 *
 * O trecho `{...}` é re-lexado a partir de uma string isolada, então os nós
 * dele nascem na linha 1. O escopo do `expr` impede que esse 1 vaze pra fora,
 * mas DENTRO da interpolação ele continuaria errado: `post(f"{1 / x}")` na
 * linha 4 reportaria linha 1 na divisão por zero.
 *
 * Por isso a sub-árvore recebe a posição do nó da f-string. A coluna não é a
 * exata dentro do trecho; a linha é a certa, e é ela que o traceback mostra. */
static void carimba_pos(PSNode *n, int32_t linha, int32_t col)
{
    if (!n) return;
    n->line = linha;
    n->col  = col;
    carimba_pos(n->a, linha, col);
    carimba_pos(n->b, linha, col);
    carimba_pos(n->c, linha, col);
    carimba_pos(n->e, linha, col);
    for (int32_t i = 0; i < n->lista.n; i++)        carimba_pos(n->lista.itens[i], linha, col);
    for (int32_t i = 0; i < n->lista2.n; i++)       carimba_pos(n->lista2.itens[i], linha, col);
    for (int32_t i = 0; i < n->lista2_alias.n; i++) carimba_pos(n->lista2_alias.itens[i], linha, col);
}

/* Quantas interpolações a varredura acha no texto — a MESMA regra do laço
 * abaixo, só contando. Serve pra conferir se os filhos que o parser já trouxe
 * correspondem, um a um, ao que este laço vai cortar. */
static int32_t conta_interp(const char *t, int32_t len)
{
    int32_t q = 0;
    for (int32_t i = 0; i < len; ) {
        if (t[i] == '{' && i + 1 < len && t[i + 1] == '{') { i += 2; continue; }
        if (t[i] == '}' && i + 1 < len && t[i + 1] == '}') { i += 2; continue; }
        if (t[i] == '{') {
            int prof = 1;
            int32_t j = i + 1;
            while (j < len && prof > 0) {
                if (t[j] == '{') prof++;
                else if (t[j] == '}') prof--;
                j++;
            }
            if (prof != 0) return -1;
            q++; i = j; continue;
        }
        i++;
    }
    return q;
}

static void compila_fstring(C *c, Unidade *u, PSNode *n)
{
    const char *t = n->texto ? n->texto : "";
    /* `texto_len`, não strlen: um `\x00` no meio da f-string é dado, não fim
     * de string — com strlen o resto do texto sumia calado. */
    int32_t len = n->texto_len > 0 ? n->texto_len : (int32_t)strlen(t);
    int32_t partes = 0;

    /* O PARSER já parseou cada `{...}`, a partir do fonte cru, com a linha e a
     * coluna de verdade: os nós estão em `n->lista`. Quando a contagem bate,
     * é essa sub-árvore que vale — sem re-lexar, e o traceback de `f"{1/x}"`
     * aponta a divisão em vez do começo da string.
     *
     * A contagem é o portão: string tripla não registra interpolação, e um
     * parse feito sem a lista do lexer (`ps_parse`) não traz filho nenhum. Aí
     * o caminho de baixo, que corta na mão, continua valendo inteiro. */
    int32_t k_filho = 0;
    int usa_filhos = n->lista.n > 0 && conta_interp(t, len) == n->lista.n;

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
            if (prof != 0) { free(buf); cerro_sx(c, n, "chave nao fechada na f-string"); return; }

            if (usa_filhos) {
                expr(c, u, n->lista.itens[k_filho++]);
                partes++;
                i = j;
                continue;
            }

            /* trecho entre as chaves: lexa, parseia e compila como expressão */
            int32_t elen = j - 1 - (i + 1);
            char *e = malloc((size_t)elen + 1);
            if (!e) { free(buf); cerro(c, "sem memoria na f-string", n); return; }
            memcpy(e, t + i + 1, (size_t)elen);
            e[elen] = '\0';

            /* `{x:SPEC}`: o mesmo `"{:SPEC}".format(x)` que o parser monta
             * (fstring_com_formato), aqui escrito em texto e parseado pelo
             * mesmo parser. Campo dentro do formato faz dele uma f-string. */
            int32_t dp = ps_fstring_formato(e, elen);
            if (dp >= 0) {
                const char *spec = e + dp + 1;
                int campo = strchr(spec, '{') != NULL;
                char aspa = !strchr(spec, '"') ? '"' : !strchr(spec, '\'') ? '\'' : 0;
                if (!aspa) {
                    free(e); free(buf);
                    cerro_sx(c, n, "formato da f-string com aspas simples e duplas: use format()");
                    return;
                }
                size_t cap = (size_t)elen + 32;
                char *src = malloc(cap);
                if (!src) { free(e); free(buf); cerro(c, "sem memoria na f-string", n); return; }
                int ns = snprintf(src, cap, "(%s%c%s:%s%s%c).format(%.*s)",
                                  campo ? "f" : "", aspa, campo ? "{{" : "{", spec,
                                  campo ? "}}" : "}", aspa, (int)dp, e);
                free(e);
                e = src;
                elen = ns;
            }

            PSTokenList *toks = ps_lexer_tokenize(e, (size_t)elen);
            if (!toks || !toks->ok) {
                if (toks) ps_lexer_free(toks);
                free(e); free(buf);
                cerro_sx(c, n, "expressao invalida dentro da f-string");
                return;
            }
            PSParseResult *r = ps_parse(toks->tokens, toks->n);
            ps_lexer_free(toks);
            if (!r || !r->ok || !r->programa || r->programa->lista.n == 0) {
                if (r) ps_parse_free(r);
                free(e); free(buf);
                cerro_sx(c, n, "expressao invalida dentro da f-string");
                return;
            }
            PSNode *st = r->programa->lista.itens[0];
            /* UMA expressão, inteira: o que sobrar depois dela (`{nome
             * sobrenome}`, `{x = }`) e o que não é expressão (`{x = 1}`)
             * eram descartados calados — este caminho usava só o primeiro
             * statement. O parser já acusa com a posição exata quando lê o
             * arquivo inteiro; aqui é a rede de baixo. */
            if (r->programa->lista.n > 1 || st->kind != N_EXPRESSION_STMT || st->e) {
                ps_parse_free(r); free(e); free(buf);
                cerro_sx(c, n, "dentro de {} da f-string vai uma expressao so (sobrou texto, ou e um comando)");
                return;
            }
            PSNode *alvo = st->a;
            carimba_pos(alvo, n->line, n->col);
            /* O trecho é compilado como expressão NORMAL: se estourar (nome
             * fora de escopo, método inexistente...), o erro SOBE. Antes cada
             * trecho tinha um `try` que devolvia o texto cru — `f"oi {nome}"`
             * com `nome` indefinido imprimia `oi {nome}` e o bug do usuário
             * sumia. Erro engolido é pior que erro barulhento. */
            expr(c, u, alvo);
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
    /* Com UMA parte só o BUILD_STR era pulado "por otimização" — e aí
     * `f"{l}"` devolvia a PRÓPRIA lista em vez de texto: `type()` dava `list`,
     * dava pra chamar `.append` no resultado e isso MUTAVA o original.
     * f-string é sempre string: converte também no caso de uma parte. */
    else emite(c, u, OP_BUILD_STR, partes);
}


/* `count` codifica tipo e "tem valor" num argumento só. O tipo sai da tabela
 * única (coluna `count`): aqui havia mais uma lista à mão, e ela não conhecia
 * `JSON`, que o parser já aceitava. */
static int32_t arg_count(C *c, PSNode *n)
{
    const PSTipoInfo *ti = ps_tipo_info(n->texto);
    int32_t t = (ti && ti->count) ? ti->cod : -1;
    if (t < 0) { cerro_sx(c, n, "tipo desconhecido em count"); return -1; }
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
/* A posição do fonte é ESCOPADA, não global.
 *
 * `c->linha_atual` é o que o `emite` grava na tabela de linhas. Entrar num nó
 * escrevia nela e ninguém devolvia — então compilar qualquer coisa ANINHADA
 * deixava a linha dela no lugar, e tudo que fosse emitido depois no mesmo
 * statement herdava. `raise Boom(f"erro: {e}")` na linha 3 era gravado como
 * linha 1: o interior da f-string é re-parseado a partir de uma string
 * isolada, e ali tudo é linha 1.
 *
 * O modelo de compilador é a posição ser ARGUMENTO, não estado pendurado: a
 * tabela de linhas sai da posição do nó que está sendo emitido.
 * Salvar e restaurar em volta de cada nó dá o mesmo efeito sem passar a
 * posição nas ~1000 chamadas de `emite`: nada aninhado alcança quem o contém. */
static void expr_no(C *c, Unidade *u, PSNode *n);

static void expr(C *c, Unidade *u, PSNode *n)
{
    int32_t l = c->linha_atual, co = c->coluna_atual;
    expr_no(c, u, n);
    c->linha_atual = l; c->coluna_atual = co;
}

/* Teto CONTADO da descida da árvore (motor-1), irmão do PS_PARSE_PROF_MAX e do
 * PS_PARSE_BLOCO_PROF_MAX do parser. A contagem é o que faz o MESMO programa
 * dar o mesmo resultado em qualquer `ulimit -s`: a folga medida da pilha
 * sozinha seguia o limite do processo, e o make 3.81 da Apple dá 64 MB aos
 * filhos — os 8000 blocos aninhados passavam lá e falhavam nos 8 MB do shell.
 * Um nível acima do teto é erro em qualquer pilha.
 *
 * O valor do bloco é o MESMO do parser (3000): o compilador desce a árvore
 * que o parser aceitou, e o teto tem que falar no mesmo nível, senão o
 * `--check` recusaria no compilador o que o parser aprovou. Subiu o valor,
 * sobe o `fundo` do caso em teste/casos_crash.c junto. */
#define PS_COMPILA_PROF_MAX 100000
#define PS_COMPILA_BLOCO_PROF_MAX 1500

/* FREIO da descida pela árvore. O parser tem o dele (`expressao`/`bloco`),
 * mas 4000 blocos aninhados ou 5000 decoradores passam por ele e derrubavam
 * o compilador e o checador, que descem a mesma árvore com quadros maiores e
 * sem freio nenhum. Dois freios, como no parser: o TETO CONTADO (o mesmo
 * resultado em qualquer pilha, motor-1) e a folga medida da pilha do C (o
 * quadro do binário de verdade, que o ASan aumenta). */
static int fundo_demais(C *c, PSNode *n, int bloco)
{
    /* O do bloco tem o dobro da margem do da expressão (ver `ps_pilha.h`), pra
     * 4000 `if` aninhados acusarem o bloco e não a condição de dentro, em
     * qualquer binário */
    if (c->fundo < (bloco ? PS_COMPILA_BLOCO_PROF_MAX : PS_COMPILA_PROF_MAX)
            && !(bloco ? ps_pilha_apertada_bloco() : ps_pilha_apertada()))
        return 0;
    cerro_sx(c, n, "%s", bloco ? "bloco aninhado demais" : "expressao aninhada demais");
    return 1;
}

/* Literal de lista, tupla ou dict em LOTES. Até LITERAL_LOTE itens, é a forma
 * de sempre: todos na pilha e um BUILD_* os recolhe. Acima disso, a
 * sequência nasce vazia (BUILD_LIST 0 / BUILD_DICT 0) e recebe um lote por
 * vez (BUILD_LIST k + LIST_EXTEND; BUILD_DICT k + DICT_MERGE); a tupla nasce
 * lista e vira tupla no fim (TUP_DE_LISTA). Um literal de 65537 itens
 * empilhava 65537 valores de uma vez — mais que a pilha inteira — e escrevia
 * fora do vetor; e a cota de cada frame é a profundidade máxima que o código
 * dele alcança (`pilha_max`), que assim fica pequena mesmo com o literal
 * grande, no topo, numa funct ou numa fibra. A ordem de avaliação dos itens é
 * a mesma (da esquerda pra direita), e chave repetida no dict ganha da
 * anterior nas duas formas. */
#define LITERAL_LOTE 256
static void emite_literal_seq(C *c, Unidade *u, PSNode *n, int tupla)
{
    int32_t total = n->lista.n;
    if (total <= LITERAL_LOTE) {
        for (int32_t i = 0; i < total; i++) expr(c, u, n->lista.itens[i]);
        emite(c, u, tupla ? OP_BUILD_TUPLE : OP_BUILD_LIST, total);
        return;
    }
    emite(c, u, OP_BUILD_LIST, 0);
    for (int32_t i = 0; i < total; ) {
        int32_t k = 0;
        for (; k < LITERAL_LOTE && i + k < total; k++) expr(c, u, n->lista.itens[i + k]);
        emite(c, u, OP_BUILD_LIST, k);
        emite(c, u, OP_LIST_EXTEND, 0);
        i += k;
    }
    if (tupla) emite(c, u, OP_TUP_DE_LISTA, 0);
}

static void emite_literal_dict(C *c, Unidade *u, PSNode *n)
{
    int32_t total = n->lista.n;
    if (total <= LITERAL_LOTE) {
        for (int32_t i = 0; i < total; i++) {
            PSNode *e = n->lista.itens[i];
            expr(c, u, e->a);
            expr(c, u, e->b);
        }
        emite(c, u, OP_BUILD_DICT, total);
        return;
    }
    emite(c, u, OP_BUILD_DICT, 0);
    for (int32_t i = 0; i < total; ) {
        int32_t k = 0;
        for (; k < LITERAL_LOTE && i + k < total; k++) {
            PSNode *e = n->lista.itens[i + k];
            expr(c, u, e->a);
            expr(c, u, e->b);
        }
        emite(c, u, OP_BUILD_DICT, k);
        emite(c, u, OP_DICT_MERGE, 0);
        i += k;
    }
}

static void expr_no_corpo(C *c, Unidade *u, PSNode *n);

static void expr_no(C *c, Unidade *u, PSNode *n)
{
    if (n && n->line) c->linha_atual = n->line;
    if (n && n->col)  c->coluna_atual = n->col;
    if (CFALHOU(c) || !n) return;
    if (fundo_demais(c, n, 0)) return;
    /* o teto contado da descida (motor-1): a casca garante o decremento nos
     * 44 `return` do corpo, como o `bloco` do parser */
    c->fundo++;
    expr_no_corpo(c, u, n);
    c->fundo--;
}

static void expr_no_corpo(C *c, Unidade *u, PSNode *n)
{
    switch (n->kind) {
        case N_LITERAL:
            switch (n->lit) {
                case L_INT:  emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, n->i, 0, NULL, 0)); break;
                case L_FLO:  emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_FLO, 0, n->d, NULL, 0)); break;
                case L_BOOL: emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, n->i, 0, NULL, 0)); break;
                case L_NULL: emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0)); break;
                case L_STR:
                    /* `texto_len` e não strlen: `"a\x00b"` tem 3 bytes e o
                     * NUL do meio não pode cortar a constante. */
                    emite(c, u, OP_LOAD_CONST,
                          idx_const(c, u, K_STR, 0, 0, n->texto ? n->texto : "",
                                    n->texto ? (n->texto_len > 0 ? n->texto_len
                                                                 : (int32_t)strlen(n->texto))
                                             : 0));
                    break;
                case L_BIGINT:
                    emite(c, u, OP_LOAD_CONST,
                          idx_const(c, u, K_BIGINT, 0, 0, n->texto ? n->texto : "0",
                                    n->texto ? (int32_t)strlen(n->texto) : 1));
                    break;
                case L_BYTES:
                    /* `b"..."`: texto_len sempre (NUL dentro; `b""` é 0) */
                    emite(c, u, OP_LOAD_CONST,
                          idx_const(c, u, K_BYTES, 0, 0, n->texto ? n->texto : "",
                                    n->texto ? n->texto_len : 0));
                    break;
                case L_FSTRING:
                    compila_fstring(c, u, n);
                    break;
            }
            return;

        case N_NAME:
            tp_confere_nome(c, u, n->texto, n);
            tp_saida_como_valor(c, u, n);
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
                    /* I10 — `is` com LITERAL de um dos lados e erro.
                     *
                     * Aqui `is` e o operador de TIPO (`5 is int`), nao
                     * identidade de objeto — esta na doc e ha 77 usos no
                     * repositorio. O defeito era outro: quando o outro lado
                     * NAO era um tipo, ele caia em igualdade de valor SEM
                     * AVISAR. `x is 0` digitado no lugar de `x == 0` virava
                     * comparacao, dava o resultado "certo" e nunca reclamava.
                     *
                     * Aqui e ERRO em tempo de compilacao — a linguagem nao tem
                     * canal de aviso, e silencio foi o que criou o problema.
                     *
                     * So o LITERAL: `x is y` com dois nomes continua valendo,
                     * porque `y` pode perfeitamente guardar um tipo. */
                    /* So o lado DIREITO: `5 is int` tem literal a esquerda
                     * e e o uso correto — o tipo e que vai a direita. Nome de
                     * tipo chega como N_TYPE_NAME, nunca N_LITERAL, entao a
                     * checagem separa os dois sozinha. */
                    /* `x is Null` FICA: e o idioma da linguagem pro teste
                     * de ausencia, e `type(Null)` e
                     * literalmente "Null" — ali o literal ocupa a posicao de
                     * tipo com sentido. Os outros literais nao tem essa
                     * leitura. */
                    if (n->b && n->b->kind == N_LITERAL && n->b->lit != L_NULL) {
                        static const char *NOME_LIT[] = {
                            "int", "flo", "str", "bool", "Null", "str", "int", "byte"
                        };
                        const char *tn = n->b->lit <= L_BYTES ? NOME_LIT[n->b->lit] : "?";
                        cerro_sx(c, n,
                                 "'is' com literal '%s' a direita — 'is' compara"
                                 " TIPO (`x is int`); para comparar valor use '=='",
                                 tn);
                        return;
                    }
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
                if (eh_and) curto = funde_salto_falso(c, u, curto);
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
            /* CADEIA À ESQUERDA SEM RECURSÃO. `0 + 1 + 1 + ...` com 8000
             * termos é uma árvore de 8000 níveis pelo `->a` (associatividade
             * à esquerda), que o parser monta em laço; descer um quadro por
             * termo aqui derrubava o processo. Os nós da cadeia são
             * recolhidos em laço e emitidos da esquerda pra direita; só o que
             * não é cadeia (o termo da ponta e os lados direitos) desce. */
            {
                int32_t fundo = 0;
                PSNode *q = n;
                while (q->kind == N_BINARY_OP && q->texto && op_binario(q->texto) >= 0) { fundo++; q = q->a; }
                PSNode **cadeia = malloc(sizeof(PSNode *) * (size_t)fundo);
                if (!cadeia) { cerro(c, "sem memoria", n); return; }
                int32_t k = fundo;
                for (q = n; k > 0; q = q->a) cadeia[--k] = q;
                expr(c, u, q);                           /* o termo mais à esquerda */
                /* a conta que a VM recusaria com os tipos escritos (`"a" + 1`)
                 * é acusada aqui, com a frase dela (tp_op) */
                const char *t_esq = tp_de(c, u, q);
                for (k = 0; k < fundo && !CFALHOU(c); k++) {
                    expr_direito(c, u, cadeia[k]);
                    char recusa[200];
                    t_esq = tp_op(c, cadeia[k]->texto, t_esq, tp_de(c, u, cadeia[k]->b),
                                  recusa, sizeof(recusa));
                    if (recusa[0]) terro(c, cadeia[k], "TypeError", "%s", recusa);
                    if (cadeia[k]->line) c->linha_atual = cadeia[k]->line;
                    if (cadeia[k]->col)  c->coluna_atual = cadeia[k]->col;
                    emite(c, u, op_binario(cadeia[k]->texto), 0);
                }
                free(cadeia);
            }
            return;
        }

        case N_UNARY_OP:
            expr(c, u, n->a);
            if (!n->texto) return;
            /* a recusa que a VM daria com o tipo escrito certo (`-"a"`,
             * `+[1]`, `~1.5`, `~True`), com a frase dela, antes de rodar */
            {
                const char *ta = tp_basico(tp_de(c, u, n->a));
                if (ta && (!strcmp(n->texto, "-") || !strcmp(n->texto, "+")) && !tp_numerico(ta))
                    terro(c, n, "TypeError", "bad operand type for unary %s: '%s'", n->texto, ta);
                else if (ta && !strcmp(n->texto, "~") && strcmp(ta, "int"))
                    terro(c, n, "TypeError", "bad operand type for unary ~: '%s'", ta);
            }
            if (!strcmp(n->texto, "-"))      emite(c, u, OP_NEG, 0);
            else if (!strcmp(n->texto, "~")) emite(c, u, OP_BIT_NOT, 0);
            else if (!strcmp(n->texto, "+")) {
                /* `+"abc"` passava calado (não emitia nada). Negar duas
                 * vezes devolve o número (bool vira int) e recusa o resto,
                 * com a frase do `+` (o NEG 1 na VM). Com o operando de
                 * tipo certo `int`/`flo`/`long` pelo que está escrito, o `+`
                 * não tem o que fazer e não emite nada: os dois NEG custavam
                 * +13,9% de instruções num laço de `+i` (medido no 17.0.17) */
                const char *tb = tp_basico(tp_de(c, u, n->a));
                int numero_certo = tb && (!strcmp(tb, "int") || !strcmp(tb, "flo") || !strcmp(tb, "long"));
                if (!numero_certo) {
                    emite(c, u, OP_NEG, 1);
                    emite(c, u, OP_NEG, 0);
                }
            }
            else if (!strcmp(n->texto, "not") || !strcmp(n->texto, "Not")
                     || !strcmp(n->texto, "!")) emite(c, u, OP_NOT, 0);
            else cerro(c, "operador unario ainda nao compila na VM", n);
            return;

        case N_CALL:
            if (n->a && n->a->kind == N_MEMBER_ACCESS) c->membro_chamado = n->a;
            c->alvo_chamado = n->a;
            tp_confere_chamada(c, u, n);
            if (n->a && n->a->kind == N_MEMBER_ACCESS && n->a->a
                    && n->a->a->kind == N_BASE_CALL_NODE) {
                /* `base(Pai).membro(args)`: a classe pai é carregada pelo
                 * NOME — buscar pela instância acharia o override da filha e
                 * recursaria pra sempre. `__init__` só com posicionais vai
                 * pelo CALL_BASE (sem criar bound); o resto liga o método
                 * ao self (LOAD_BASE_MEMBRO) e chama como qualquer bound,
                 * inclusive com nomeado e espalhado. */
                PSNode *f = n->a;
                const char *pai = base_pai(c, u, f->a);
                if (!pai) return;
                int32_t nkw_b = 0;
                for (int32_t i = 0; i < n->lista.n; i++)
                    if (n->lista.itens[i]->texto || n->lista.itens[i]->i2) nkw_b++;
                carrega_nome(c, u, pai);
                if (f->texto && strcmp(f->texto, "__init__") == 0 && nkw_b == 0) {
                    emite(c, u, OP_LOAD_SELF, 0);
                    for (int32_t i = 0; i < n->lista.n; i++)
                        expr(c, u, n->lista.itens[i]->a);
                    emite(c, u, OP_CALL_BASE, n->lista.n);
                    return;
                }
                emite(c, u, OP_LOAD_BASE_MEMBRO,
                      idx_const(c, u, K_STR, 0, 0, f->texto ? f->texto : "",
                                f->texto ? (int32_t)strlen(f->texto) : 0) | PS_ARG_CHAMADO);
                tp_confere_saida(c, u, n);
                emite_args_e_chama(c, u, n, &n->lista);
                return;
            }
            expr(c, u, n->a);
            /* depois do chamado: ele pode ter chamadas dentro, e a marca de
             * saída é só desta */
            tp_confere_saida(c, u, n);
            emite_args_e_chama(c, u, n, &n->lista);
            return;

        case N_LIST_LITERAL:
            emite_literal_seq(c, u, n, 0);
            return;

        case N_LIST_COMP: {
            /* `[<expr> for each v in <it> (if <c>)?]`
             *
             * O acumulador NÃO pode ficar na pilha: o estado do iterador
             * (container + índice) fica por cima dele, e não há opcode que
             * anexe a uma lista N posições abaixo. Então ele mora num nome
             * escondido — não digitável, e numerado pra compreensão aninhada
             * não pisar na de fora.
             *
             *   BUILD_LIST 0 ; STORE acc
             *   <it> ; LOAD_CONST 0
             * topo:
             *   ITER_NEXT fim ; STORE v
             *   (<c> ; JUMP_IF_FALSE prox)
             *   LOAD acc ; GET_MEMBER append ; <expr> ; CALL 1 ; POP_TOP
             * prox:
             *   JUMP topo
             * fim:
             *   LOAD acc                       ; o valor da expressão
             */
            char acc[64];
            snprintf(acc, sizeof(acc), "  lc$%d", c->n_listcomp++);
            const char *var = n->texto ? n->texto : "";

            emite(c, u, OP_BUILD_LIST, 0);
            guarda_nome_modo(c, u, acc, 1);

            /* o nome da var da compreensão SOMBREIA, como no `for each`; na
             * funct, a capturada pelo item ou pela condição, ou com o nome de
             * uma variável da funct de fora, tem slot próprio (`laco_abre`), e
             * o de fora fica onde está, sem ser guardado */
            char salvo[128];
            int propria = !u->eh_modulo
                       && (citado_em(c, n->b, var) || citado_em(c, n->c, var) || vem_de_fora(u, var));
            int sombreia = !propria && nome_ja_existe(u, var);
            SimInfo sim_fora;
            int sim_posto = 0;
            if (sombreia) {
                snprintf(salvo, sizeof(salvo), "  lcv$%s", var);
                sombra_guarda(c, u, var, salvo);
                sim_posto = tp_sombra_poe(c, u, var, &sim_fora);
            }
            int32_t M = escopo_marca(u);

            expr(c, u, n->a);
            /* a var capturada pelo item ou pela condição vira célula do laço */
            char **ativ_lc = NULL; int32_t na_lc = 0, ca_lc = 0;
            laco_abre(c, u, var, propria, NULL, n->b, n->c, &ativ_lc, &na_lc, &ca_lc);
            /* sombra de variável capturada fora: célula nova por volta (ver o `for each`) */
            if (sombreia && sombra_e_celula(u, var)) junta_nomes(c, &ativ_lc, &na_lc, &ca_lc, var);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 0, 0, NULL, 0));
            int32_t topo = UP(c, u)->ncode;
            int32_t fim = emite(c, u, OP_ITER_NEXT, 0);
            /* a compreensão também é laço: a closure de cada item guarda a
             * variável daquela volta */
            celulas_novas_da_volta(c, u, ativ_lc, na_lc);
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.tem_valor = 1;
            c->grava.tipo = tp_elemento_de(c, u, n->a);
            c->grava.no = n;
            guarda_nome_modo(c, u, var, 1);

            int32_t pula_item = -1;
            if (n->c) {
                expr(c, u, n->c);
                pula_item = emite(c, u, OP_JUMP_IF_FALSE, 0);
            }
            carrega_nome(c, u, acc);
            emite(c, u, OP_GET_MEMBER, idx_const(c, u, K_STR, 0, 0, "append", 6));
            expr(c, u, n->b);
            emite(c, u, OP_CALL, 1);
            emite(c, u, OP_POP_TOP, 0);
            if (pula_item >= 0) UP(c, u)->code[pula_item + 1] = UP(c, u)->ncode;

            escopo_emite_clears(c, u, M);
            emite(c, u, OP_JUMP, topo);
            UP(c, u)->code[fim + 1] = UP(c, u)->ncode;
            escopo_emite_clears(c, u, M);
            escopo_trunca(c, u,M);
            celulas_fecha(c, u, ativ_lc, na_lc);
            if (sombreia) {
                sombra_devolve_carrega(c, u, var, salvo);
                tp_sombra_devolve(c, u, var, sim_posto, &sim_fora);
                sombra_devolve_fim(c, u, var);
            }
            carrega_nome(c, u, acc);
            return;
        }

        case N_DICT_LITERAL:
            /* A chave é expressão (parser): `{k: 1}` carrega a variável `k`,
             * e nome que o arquivo não liga é NameError antes de rodar, como
             * em qualquer expressão. Antes o Name virava a string "k" aqui. */
            emite_literal_dict(c, u, n);
            return;

        case N_INDEX_ACCESS:
            expr(c, u, n->a);
            expr(c, u, n->b);
            emite(c, u, OP_INDEX_GET, 0);
            return;

        case N_TUPLE_LITERAL:
            emite_literal_seq(c, u, n, 1);
            return;

        case N_INTERPOLATED_STRING:
            /* `"a" {x} "b"` — as partes já vêm separadas pelo parser; o
             * BUILD_STR concatena usando as MESMAS regras de texto do post. */
            for (int32_t i = 0; i < n->lista.n; i++) expr(c, u, n->lista.itens[i]);
            emite(c, u, OP_BUILD_STR, n->lista.n);
            return;

        case N_SLICE_ACCESS: {
            /* parte ausente vira Null — a VM normaliza pro padrão da fatia */
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
                else { cerro_sx(c, n, "cor invalida"); return; }
            }
            unsigned r = 0, g = 0, b = 0;
            if (sscanf(hex, "%2x%2x%2x", &r, &g, &b) != 3) { cerro_sx(c, n, "cor invalida"); return; }

            /* String SIMPLES (literal): a cor é conhecida em compilação, então
             * vira UMA constante já com os escapes ANSI — caminho rápido. */
            if (n->a && n->a->kind == N_LITERAL && n->a->lit == L_STR) {
                const char *txt = n->a->texto ? n->a->texto : "";
                char buf[1024];
                int len = snprintf(buf, sizeof(buf), "\033[38;2;%u;%u;%um%s\033[0m", r, g, b, txt);
                if (len < 0 || len >= (int)sizeof(buf)) { cerro_sx(c, n, "string colorida longa demais"); return; }
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
            /* Mesma máquina da funct nomeada: um protótipo e um
             * MAKE_FUNCTION. A diferença é só não ter nome pra guardar. */
            int32_t pi = compila_funct(c, n, u);
            if (CFALHOU(c)) return;
            emite_funcao(c, u, pi);
            return;
        }

        case N_TYPE_NAME: {
            /* A MESMA tabela da declaração (`cod_tipo_decl`): aqui havia uma
             * segunda lista, com oito nomes escritos à mão, e por isso `long`
             * valia em `long x = 1` e não valia como valor em `long(x)` —
             * duas listas pro mesmo conceito discordando uma da outra.
             * `json` e `dict` são o MESMO tipo, e o apelido sai de lá. */
            int t = cod_tipo_decl(n->texto);
            if (t < 0) { cerro_sx(c, n, "tipo desconhecido"); return; }
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

        case N_MEMBER_ACCESS: {
            tp_confere_membro(c, u, n);
            /* lido ANTES de descer: uma chamada dentro do alvo (`f().m()`)
             * troca o `membro_chamado` e o `alvo_chamado` */
            tp_saida_como_valor(c, u, n);
            int32_t chamado = n == c->membro_chamado ? PS_ARG_CHAMADO : 0;
            if (n->a && n->a->kind == N_BASE_CALL_NODE) {
                /* `base(Pai).campo` / `base(Pai).metodo` (sem chamar) */
                const char *pai = base_pai(c, u, n->a);
                if (!pai) return;
                carrega_nome(c, u, pai);
                emite(c, u, OP_LOAD_BASE_MEMBRO,
                      idx_const(c, u, K_STR, 0, 0, n->texto ? n->texto : "",
                                n->texto ? (int32_t)strlen(n->texto) : 0) | chamado);
                return;
            }
            expr(c, u, n->a);
            emite(c, u, OP_GET_MEMBER,
                  idx_const(c, u, K_STR, 0, 0, n->texto ? n->texto : "",
                            n->texto ? (int32_t)strlen(n->texto) : 0) | chamado);
            return;
        }

        case N_AWAIT_EXPR:
            /* `await expr` — avalia a expr (que dá um future) e resolve. */
            expr(c, u, n->a);
            emite(c, u, OP_AWAIT, 0);
            return;

        case N_BASE_CALL_NODE:
            /* `base(Pai)` só existe embaixo de um `.membro` (o parser exige);
             * solto, é o que a frase diz. */
            cerro_sx(c, n, "base(Pai) precisa de um membro: base(Pai).__init__(...) ou base(Pai).metodo(...)");
            return;

        case N_POSTFIX_OP: {
            /* `x++` devolve o valor ANTIGO e guarda o novo — por isso o DUP
             * antes de somar (é o que o interpretador faz). */
            if (!n->a || n->a->kind != N_NAME) {
                cerro_sx(c, n, "'++'/'--' so funcionam em variaveis");
                return;
            }
            const char *nome = n->a->texto ? n->a->texto : "";
            tp_confere_nome(c, u, nome, n->a);
            carrega_nome(c, u, nome);
            emite(c, u, OP_DUP, 0);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 1, 0, NULL, 0));
            emite(c, u, (n->texto && n->texto[0] == '+') ? OP_ADD : OP_SUB, 0);
            {
                SimInfo *sx = tp_sim_de(c, u, nome);
                memset(&c->grava, 0, sizeof(c->grava));
                c->grava.tem_valor = 1;
                c->grava.tipo = tp_binario(c, "+", (sx && sx->estado) ? sx->tipo : NULL, "int");
                c->grava.no = n;
            }
            guarda_nome(c, u, nome);
            return;
        }

        default:
            cerro(c, "expressao ainda nao compila na VM", n);
            return;
    }
}

/* ── laços: pilha para backpatch de break/continue ──────────────────────── */
/* Devolve -1 no estouro de MAX_LACOS, com o erro já posto. Antes voltava em
 * silêncio e o chamador escrevia em `c->lacos[c->nlacos - 1]` — o laço de
 * FORA — enquanto `break`/`continue` do de dentro saltavam pro lugar errado;
 * o comentário prometia um erro "fora de laco" que nunca acontecia. */
static int abre_laco(C *c, PSNode *n, int32_t inicio, int slots_pilha)
{
    if (c->nlacos >= MAX_LACOS) { cerro(c, "limite do compilador: lacos aninhados demais", n); return -1; }
    Laco *l = &c->lacos[c->nlacos++];
    l->inicio = inicio;
    memset(&l->saidas, 0, sizeof(l->saidas));
    memset(&l->continues, 0, sizeof(l->continues));
    l->slots_pilha = slots_pilha;
    l->extra_base = c->pilha_extra;
    return 0;
}

static void fecha_laco(C *c, Unidade *u, int32_t inicio)
{
    if (c->nlacos == 0) return;
    Laco *l = &c->lacos[--c->nlacos];
    int32_t fim = UP(c, u)->ncode;
    saltos_corrige(c, u, &l->saidas, fim);
    saltos_corrige(c, u, &l->continues, inicio);
    saltos_solta(&l->saidas);
    saltos_solta(&l->continues);
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
        case L_BYTES: emite(c, u, OP_LOAD_CONST,          /* `case b"x":` casa bytes */
                  idx_const(c, u, K_BYTES, 0, 0, lit->texto ? lit->texto : "",
                            lit->texto ? lit->texto_len : 0)); break;
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
        Saltos acertos = {0}, fim_ok = {0};
        for (int32_t i = 0; i < pat->lista.n && !CFALHOU(c); i++) {
            int ultimo = (i == pat->lista.n - 1);
            if (!ultimo) {
                emite(c, u, OP_DUP, 0);
                padrao_testa(c, u, pat->lista.itens[i]);
                saltos_push(c, &acertos, emite(c, u, OP_JUMP_IF_TRUE, 0));
            } else {
                padrao_testa(c, u, pat->lista.itens[i]);
                saltos_push(c, &fim_ok, emite(c, u, OP_JUMP, 0));
            }
        }
        /* alvo dos acertos: ainda resta a cópia original pra descartar */
        saltos_corrige(c, u, &acertos, UP(c, u)->ncode);
        emite(c, u, OP_POP_TOP, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 1, 0, NULL, 0));

        saltos_corrige(c, u, &fim_ok, UP(c, u)->ncode);
        saltos_solta(&acertos);
        saltos_solta(&fim_ok);
        return;
    }
    if (!strcmp(k, "list")) {
        /* tamanho tem que bater, depois cada item */
        Saltos falhas = {0};
        emite(c, u, OP_DUP, 0);
        emite(c, u, OP_LEN, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, pat->lista.n, 0, NULL, 0));
        emite(c, u, OP_EQ, 0);
        saltos_push(c, &falhas, emite(c, u, OP_JUMP_IF_FALSE, 0));

        for (int32_t i = 0; i < pat->lista.n && !CFALHOU(c); i++) {
            emite(c, u, OP_DUP, 0);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, i, 0, NULL, 0));
            emite(c, u, OP_INDEX_GET, 0);
            padrao_testa(c, u, pat->lista.itens[i]);
            saltos_push(c, &falhas, emite(c, u, OP_JUMP_IF_FALSE, 0));
        }
        emite(c, u, OP_POP_TOP, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 1, 0, NULL, 0));
        int32_t fim = emite(c, u, OP_JUMP, 0);
        saltos_corrige(c, u, &falhas, UP(c, u)->ncode);
        saltos_solta(&falhas);
        emite(c, u, OP_POP_TOP, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 0, 0, NULL, 0));
        UP(c, u)->code[fim + 1] = UP(c, u)->ncode;
        return;
    }
    if (!strcmp(k, "dict")) {
        /* cada chave precisa existir E o valor casar */
        Saltos falhas = {0};
        for (int32_t i = 0; i < pat->lista2.n && !CFALHOU(c); i++) {
            const char *chave = pat->lista2.itens[i]->texto;
            int32_t kc = idx_const(c, u, K_STR, 0, 0, chave ? chave : "",
                                   chave ? (int32_t)strlen(chave) : 0);
            emite(c, u, OP_DUP, 0);
            emite(c, u, OP_LOAD_CONST, kc);
            emite(c, u, OP_HAS_KEY, 0);
            saltos_push(c, &falhas, emite(c, u, OP_JUMP_IF_FALSE, 0));
            emite(c, u, OP_DUP, 0);
            emite(c, u, OP_LOAD_CONST, kc);
            emite(c, u, OP_INDEX_GET, 0);
            padrao_testa(c, u, pat->lista.itens[i]);
            saltos_push(c, &falhas, emite(c, u, OP_JUMP_IF_FALSE, 0));
        }
        emite(c, u, OP_POP_TOP, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 1, 0, NULL, 0));
        int32_t fim = emite(c, u, OP_JUMP, 0);
        saltos_corrige(c, u, &falhas, UP(c, u)->ncode);
        saltos_solta(&falhas);
        emite(c, u, OP_POP_TOP, 0);
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 0, 0, NULL, 0));
        UP(c, u)->code[fim + 1] = UP(c, u)->ncode;
        return;
    }
    cerro(c, "padrao de match ainda nao compila na VM", pat);
}

/* ── statements ─────────────────────────────────────────────────────────── */
/* As instruções de `b` sem abrir escopo de células: o corpo da funct (as
 * células do nível de cima dela vêm do `marca_celulas`), o do `using` (que
 * não abre escopo: o que liga é do bloco de fora) e o bloco que o parser monta
 * em volta da declaração decorada (`@x funct f`), que não é bloco do usuário:
 * o nome declarado nele vale fora, e o decorador o rebinda logo depois. */
static void stmts_de(C *c, Unidade *u, PSNode *b)
{
    if (!b) return;
    for (int32_t i = 0; i < b->lista.n && !CFALHOU(c); i++)
        stmt(c, u, b->lista.itens[i]);
}

/* Um bloco do usuário: a variável que nasce nele e que uma funct escrita
 * nele cita vira célula do bloco (`bloco_abre`). Na funct, o slot dela some
 * com a marca que quem chamou tomou antes do bloco; no topo, o fechamento a
 * solta aqui. */
static void bloco_stmts(C *c, Unidade *u, PSNode *b)
{
    if (!b) return;
    char **ativ = NULL; int32_t na = 0, ca = 0;
    bloco_abre(c, u, b, &ativ, &na, &ca);
    stmts_de(c, u, b);
    celulas_fecha(c, u, ativ, na);
}


/* Emite os blocos `finally` pendentes, do mais interno pro mais externo, que
 * a saída atual está atravessando. `ate_laco` < 0 = todos da função (return);
 * >= 0 = só os que estão DENTRO do laço indicado (break/continue). */
/* `extra` = valores que quem chama deixou na pilha por cima de tudo e que
 * ficam lá enquanto as cópias rodam: 1 no `return` (o valor devolvido), 0 no
 * `break`/`continue`. Um `break` dentro da cópia descarta esses também (ver
 * `Laco.extra_base`). */
static void emite_finallys(C *c, Unidade *u, int ate_laco, int extra, int continua)
{
    c->pilha_extra += extra;
    for (int i = c->nfinally - 1; i >= 0; i--) {
        if (c->fin_pend[i].unidade != (const void *)u) break;   /* outra função */
        if (ate_laco >= 0 && c->fin_pend[i].laco < ate_laco) break;
        int li = c->fin_pend[i].dono_laco;
        if (li >= 0) {
            /* o laço dono: fecha o gerador dele, que está embaixo do que os
             * construtos de dentro empilharam e do estado dos laços de
             * dentro. O `continue` dele não sai. */
            if (continua && li == c->nlacos - 1) continue;
            int32_t prof = (c->pilha_extra - c->lacos[li].extra_base) + 2;
            for (int k = li + 1; k < c->nlacos; k++) prof += c->lacos[k].slots_pilha;
            emite(c, u, OP_DONO_SAI, prof);
            continue;
        }
        /* primeiro desarma o que este construto tem armado: o finally (e o
         * fechamento) rodam fora da proteção dele, como na saída normal */
        for (int k = 0; k < c->fin_pend[i].armados; k++) emite(c, u, OP_POP_TRY, 0);
        PSNode *bloco = c->fin_pend[i].bloco;
        const char *fecha = c->fin_pend[i].fecha;
        if (!bloco && !fecha) continue;
        /* Enquanto ESTA entrada compila, ela e as de dentro dela saem de
         * pendente: um `return` (ou break/continue) dentro do próprio finally
         * achava o finally pendente e o recompilava sem fim
         * (`stmt_no <- emite_finallys <- stmt_no`, até derrubar o processo).
         * Um finally nunca envolve a si mesmo. */
        int salvo = c->nfinally;
        c->nfinally = i;
        if (fecha) {
            carrega_nome(c, u, fecha);
            emite(c, u, OP_CLOSE_SE_TEM, 0);
        } else {
            int32_t M = escopo_marca(u);
            bloco_stmts(c, u, bloco);
            escopo_fecha(c, u, M);
        }
        c->nfinally = salvo;
    }
    c->pilha_extra -= extra;
}

static void stmt_no(C *c, Unidade *u, PSNode *n);

/* mesma regra do `expr` */
static void stmt(C *c, Unidade *u, PSNode *n)
{
    int32_t l = c->linha_atual, co = c->coluna_atual;
    stmt_no(c, u, n);
    c->linha_atual = l; c->coluna_atual = co;
}

static void stmt_no_corpo(C *c, Unidade *u, PSNode *n);

static void stmt_no(C *c, Unidade *u, PSNode *n)
{
    if (CFALHOU(c) || !n) return;
    if (n->line) c->linha_atual = n->line;
    if (n->col)  c->coluna_atual = n->col;
    if (fundo_demais(c, n, 1)) return;
    /* o teto contado da descida (motor-1), como no `expr_no`: a casca garante
     * o decremento em qualquer `return` do corpo (o `bloco` do parser) */
    c->fundo++;
    stmt_no_corpo(c, u, n);
    c->fundo--;
}

static void stmt_no_corpo(C *c, Unidade *u, PSNode *n)
{
    switch (n->kind) {
        case N_FUNCT_DECL: {
            /* O nome nasce ANTES do corpo compilar: sem isto uma funct
             * aninhada não conseguiria chamar a si mesma (o nome dela ainda
             * não seria local da função de fora na hora da captura). */
            if (!u->eh_modulo && n->texto) {
                int32_t si = idx_local(c, u, n->texto);
                if (si >= 0 && !u->celula[si]) u->certo[si] = 1;
            }
            int32_t idx = compila_funct(c, n, u);
            if (CFALHOU(c)) return;
            emite_funcao(c, u, idx);
            if (n->is_private) priv_declara(c, u, n, n, n->texto);
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.tem_valor = 1;
            c->grava.tipo = "funct";
            c->grava.no = n;
            c->grava.declaracao = 1;
            /* o gêmeo podado, não o nó desta passada: o nó morre com o
             * statement, e a assinatura tem que valer até o fim (só aqui, nos
             * nós DESTE arquivo — a declaração de um módulo importado vem de
             * outra árvore e podia colidir por posição) */
            c->grava.decl = gemeo_de(c, n);
            guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            return;
        }

        case N_VAR_DECL: {
            /* declaração tipada cria local, sempre — não sobe escopo */
            expr(c, u, n->a);
            /* O tipo fica REGISTRADO na variavel (slot da funcao ou nome do
             * modulo); quem emite a conferencia e o `guarda_nome_modo`, em
             * toda escrita — esta e so a primeira. O codigo e o TIPO_* da VM;
             * `char` e 8, `Object` e 10, por isso e tabela e nao indice. */
            const char *vn = n->texto ? n->texto : "";
            int cod = cod_tipo_decl(n->texto2);
            if (cod >= 0) {
                if (u->eh_modulo || eh_global_declarada(u, vn)) tipo_topo_poe(c, vn, cod);
                else {
                    int32_t sl = idx_local(c, u, vn);
                    if (sl >= 0) u->tipo_decl[sl] = (unsigned char)(cod + 1);
                }
            }
            /* aponta o erro no INÍCIO do valor (RHS), não na sub-expressão
             * mais profunda que o expr() deixou em coluna_atual — é o
             * lugar EXATO do erro, igual ao node.value do interp. */
            if (n->a) {
                if (n->a->line) c->linha_atual  = n->a->line;
                if (n->a->col)  c->coluna_atual = n->a->col;
            }
            if (!tp_tipo_existe(c, u, n->texto2)) {
                /* o erro é o nome do tipo; conferir o valor contra um tipo
                 * que não existe só repetiria o mesmo erro com outra frase */
                terro(c, n, "AttributedValueError", "tipo %s não existe (variável %s)",
                      n->texto2 ? n->texto2 : "?", vn);
                memset(&c->grava, 0, sizeof(c->grava));
            } else {
                grava_valor(c, u, n->a);
                c->grava.declara = n->texto2;
            }
            guarda_nome_modo(c, u, vn, 1);
            /* `private int n = 1` no topo (veio de N_FIELD_DECL, ver
             * `topo_campo_vira_var`): o nome não sai pelo import */
            if (n->is_private && !CFALHOU(c)) priv_declara(c, u, n, n, vn);
            return;
        }

        case N_FIELD_DECL: {
            /* `private str name = nome` dentro da funct: é CAMPO DO OBJETO,
             * não local. Escreve em `self` pelo mesmo caminho de
             * `self.name = nome` — um só lugar decide dict×instância e o
             * corte de private. A visibilidade em si é registrada na CLASSE,
             * no N_ENTITY_DECL, senão o `private` compilaria sem barrar nada. */
            if (!c->dentro_entity || u->eh_modulo || !nome_ja_existe(u, "self")) {
                /* No TOPO do arquivo esta grafia e a variavel tipada do modulo
                 * com visibilidade — e ja chegou aqui como N_VAR_DECL (ver
                 * `topo_campo_vira_var`). O que sobra e o `private` fora do
                 * lugar. */
                if (u->eh_modulo) {
                    /* dentro de um bloco do modulo (`if`, `for`, o guard):
                     * nem campo (nao tem self) nem variavel de modulo (nao e
                     * o topo) */
                    cerro_sx(c, n, "'%s' aqui nao tem efeito: so vale no topo do arquivo "
                                   "(o que nao sai pelo import) ou no corpo da Entity",
                             n->is_private ? "private" : "public");
                    return;
                }
                cerro_sx(c, n, "'%s %s %s = ...' declara campo do objeto: so vale dentro de uma "
                               "funct de Entity que recebe 'self'",
                         n->is_private ? "private" : "public",
                         n->texto2 ? n->texto2 : "tipo", n->texto ? n->texto : "nome");
                return;
            }
            const char *nome = n->texto ? n->texto : "";
            int32_t mi = idx_const(c, u, K_STR, 0, 0, nome, (int32_t)strlen(nome));
            if (n->texto2) {
                const char *cls = c->entity_no && c->entity_no->texto ? c->entity_no->texto : "?";
                if (!tp_tipo_existe(c, u, n->texto2))
                    terro(c, n, "AttributedValueError", "tipo %s não existe (campo %s de %s)", n->texto2, nome, cls);
                else {
                    /* declaração: o OP_COERCE_DECL logo abaixo faz o `char` */
                    const char *T = tp_canon(n->texto2);
                    const char *vt = tp_de(c, u, n->a);
                    if (tp_aceita(c, T, vt, n->a, 2) == T_NAO)
                        terro(c, n->a, "AttributedValueError", "campo %s de %s esperava %s, recebeu %s",
                              nome, cls, T, vt);
                }
            }
            carrega_nome(c, u, "self");
            expr(c, u, n->a);
            /* MESMA tabela do N_VAR_DECL: só escalar é conferido, e `list`/
             * `json` guardam sem reclamar — a regra do tipo não muda por ter
             * ganhado um modificador na frente. */
            {
                int codf = cod_tipo_decl(n->texto2);
                if (codf >= 0) {
                    int32_t ni = idx_const(c, u, K_STR, 0, 0, nome, (int32_t)strlen(nome));
                    if (n->a) {
                        if (n->a->line) c->linha_atual  = n->a->line;
                        if (n->a->col)  c->coluna_atual = n->a->col;
                    }
                    emite(c, u, OP_COERCE_DECL, (ni << 4) | codf);
                }
            }
            emite(c, u, OP_SET_MEMBER, mi);
            return;
        }

        case N_ASSIGNMENT: {
            const char *op = n->texto2 ? n->texto2 : "=";
            if (strcmp(op, "=") != 0) {
                /* `x += v` → carrega x, calcula, guarda */
                char base[3] = { op[0], '\0', '\0' };
                int32_t opc = op_binario(base);
                if (opc < 0) { cerro_sx(c, n, "operador de atribuicao invalido"); return; }
                tp_confere_nome(c, u, n->texto, n);
                carrega_nome(c, u, n->texto ? n->texto : "");
                expr(c, u, n->a);
                emite(c, u, opc, 0);
                SimInfo *sx = tp_sim_de(c, u, n->texto);
                char recusa[200];
                const char *tv = tp_op(c, base, (sx && sx->estado) ? sx->tipo : NULL, tp_de(c, u, n->a),
                                       recusa, sizeof(recusa));
                if (recusa[0]) terro(c, n, "TypeError", "%s", recusa);
                memset(&c->grava, 0, sizeof(c->grava));
                c->grava.tem_valor = 1;
                c->grava.tipo = tv;
                c->grava.no = n;
            } else {
                expr(c, u, n->a);
                grava_valor(c, u, n->a);
            }
            guarda_nome(c, u, n->texto ? n->texto : "");
            if (strcmp(op, "=") != 0) funde_incremento(c, u);   /* `x += 1` em int declarado */
            if (n->is_private && !CFALHOU(c)) priv_declara(c, u, n, n, n->texto);
            return;
        }

        case N_EXPRESSION_STMT:
            if (n->e) { tp_nome_solto(c, u, n); return; }
            expr(c, u, n->a);
            emite(c, u, OP_POP_TOP, 0);
            funde_incremento(c, u);          /* `x++` em int declarado */
            return;

        case N_RETURN_STMT:
            /* `return` só existe dentro de funct (lambda e método contam). No
             * topo do arquivo, ou num `if`/laço do topo, ele encerrava o
             * programa calado, com rc 0 e o `--check` dizendo ok. */
            if (u->eh_modulo) { cerro_sx(c, n, "'return' fora de funct"); return; }
            /* `return;` seco dentro de `count each` devolve o TOTAL contado,
             * não Null — é o que faz `count each ... { return; }` ser a forma
             * curta de "conte e me dê o número". `return <expr>` segue
             * normal. */
            if (!n->a && c->dentro_count_each > 0) carrega_nome(c, u, "_count");
            else if (n->a) expr(c, u, n->a);
            else      emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0));
            /* funct tipada: todo `return` devolve o tipo dela */
            if (u->tipo_ret_nome && !(!n->a && c->dentro_count_each > 0)) {
                if (!n->a) {
                    /* na `Null funct` o `return` seco é o jeito de sair */
                    if (!u->tipo_ret && strcmp(u->tipo_ret_nome, "Null") != 0)
                        terro(c, n, "AttributedValueError",
                              "%s() devolve %s, e `return` sem valor devolveria Null",
                              u->nome_funct ? u->nome_funct : "?", u->tipo_ret_nome);
                } else if (!(u->tipo_ret && n->a->kind == N_LITERAL && n->a->lit == L_NULL)) {
                    /* `return Null` numa `int`/`bool funct` é o `return` seco:
                     * o sentinela (0 / True) */
                    const char *vt = tp_de(c, u, n->a);
                    int v = tp_aceita(c, u->tipo_ret_nome, vt, n->a, 1);
                    if (v == T_NAO)
                        terro(c, n->a, "AttributedValueError", "retorno de %s() esperava %s, recebeu %s",
                              u->nome_funct ? u->nome_funct : "?", u->tipo_ret_nome, vt);
                    else if (v == T_TALVEZ && !u->tipo_ret) {
                        char rot[300];
                        snprintf(rot, sizeof(rot), "retorno de %s()", u->nome_funct ? u->nome_funct : "?");
                        tp_emite_confere(c, u, rot, u->tipo_ret_nome, 1);
                    }
                }
            }
            if (u->tipo_ret) emite(c, u, OP_COERCE_RET, u->tipo_ret);
            emite_finallys(c, u, -1, 1, 0);   /* todos os finally abertos nesta função; o valor fica em cima */
            emite(c, u, OP_RETURN, 0);
            return;

        case N_BLOCK: {
            int32_t M = escopo_marca(u);
            bloco_stmts(c, u, n);
            escopo_fecha(c, u, M);   /* variáveis do bloco não vazam */
            return;
        }

        case N_IF_STMT: {
            Saltos fins = {0};
            for (int32_t i = 0; i < n->lista.n && !CFALHOU(c); i++) {
                PSNode *ramo = n->lista.itens[i];
                if (!ramo->a) {                        /* else */
                    int32_t Me = escopo_marca(u);
                    bloco_stmts(c, u, ramo->b);
                    escopo_fecha(c, u, Me);
                    break;
                }
                expr(c, u, ramo->a);                   /* condição: fora do escopo do corpo */
                int32_t salto_falso = funde_salto_falso(c, u, emite(c, u, OP_JUMP_IF_FALSE, 0));
                int32_t M = escopo_marca(u);
                bloco_stmts(c, u, ramo->b);
                escopo_fecha(c, u, M);
                saltos_push(c, &fins, emite(c, u, OP_JUMP, 0));
                if (salto_falso >= 0) UP(c, u)->code[salto_falso + 1] = UP(c, u)->ncode;
            }
            saltos_corrige(c, u, &fins, UP(c, u)->ncode);
            saltos_solta(&fins);
            return;
        }

        case N_WHILE_STMT: {
            int32_t M = escopo_marca(u);        /* sem var de laço: corpo == laço */
            int32_t topo = UP(c, u)->ncode;
            expr(c, u, n->a);
            int32_t sai = funde_salto_falso(c, u, emite(c, u, OP_JUMP_IF_FALSE, 0));
            if (abre_laco(c, n, topo, 0) != 0) return;
            c->lacos[c->nlacos - 1].escopo_marca = M;
            c->lacos[c->nlacos - 1].escopo_marca_body = M;
            bloco_stmts(c, u, n->b);
            escopo_emite_clears(c, u, M);        /* reset por-iteração */
            emite(c, u, OP_JUMP, topo);
            if (sai >= 0) UP(c, u)->code[sai + 1] = UP(c, u)->ncode;
            fecha_laco(c, u, topo);              /* break/saída normal caem aqui */
            escopo_emite_clears(c, u, M);        /* limpa o que sobrou na saída */
            escopo_trunca(c, u,M);
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
            /* Se o nome do laço já existe, guarda o valor de fora num nome
             * escondido e devolve na saída: o laço SOMBREIA, não destrói. */
            const char *var_laco = n->texto ? n->texto : "";
            char salvo[128];
            /* Com desempacotamento (`for each a, b in ...`) o primeiro nome
             * ainda esta em `n->texto`, mas os outros vivem em `n->e`; a
             * sombra so cobre o primeiro, entao aqui ela sai de cena e as
             * variaveis do laco seguem a regra normal de escopo (limpas na
             * marca M, logo abaixo). Sombrear um nome de tres seria pior que
             * nao sombrear nenhum. Na funct, a var que uma funct do corpo
             * captura, ou com o nome de uma variável da funct de fora, tem
             * slot próprio (`laco_abre`): sombreia sem guardar o de fora, que
             * fica onde está. */
            int propria = !n->e && !u->eh_modulo
                       && (citado_em(c, n->b, var_laco) || vem_de_fora(u, var_laco));
            int sombreia = !n->e && !propria && nome_ja_existe(u, var_laco);
            SimInfo sim_fora;
            int sim_posto = 0;
            if (sombreia) {
                snprintf(salvo, sizeof(salvo), "  fe$%s", var_laco);   /* nome não digitável */
                sombra_guarda(c, u, var_laco, salvo);
                sim_posto = tp_sombra_poe(c, u, var_laco, &sim_fora);
            }
            int32_t M = escopo_marca(u);        /* marca ANTES da var do laço */
            char **ativ_fe = NULL; int32_t na_fe = 0, ca_fe = 0;
            /* `for each i in range(a, b, p)`: em vez de materializar a lista
             * inteira (20 milhões de itens = 320 MB), empilha ini/fim/passo e
             * conta. Só entra quando o nome `range` não é ligado em lugar
             * nenhum do programa — senão o `range` do usuário é que vale. */
            int usa_range = 0;
            if (n->a && n->a->kind == N_CALL && n->a->a
                    && n->a->a->kind == N_NAME && n->a->a->texto
                    && !strcmp(n->a->a->texto, "range")
                    && n->a->lista.n >= 1 && n->a->lista.n <= 3
                    && !c->range_ligado) {
                usa_range = 1;
                /* nomeado ou espalhado (`range(*l)`) não: o atalho lê cada
                 * argumento como um limite, e a lista inteira virava o fim */
                for (int32_t k = 0; k < n->a->lista.n; k++)
                    if (n->a->lista.itens[k]->texto || n->a->lista.itens[k]->i2) { usa_range = 0; break; }
            }
            int32_t topo, fim;
            /* O laço é DONO do gerador que o cabeçalho cria (`for each x in
             * gen()`, decisão dele): fecha-o quando sai antes do fim (break e
             * return pelo DONO_SAI; erro pelo `erro_runtime`), e o `finally`/
             * `using` de dentro dele roda. Só o cabeçalho que é chamada e cujo
             * tipo o checador não sabe que não é gerador: o laço sobre a lista
             * de um `split()` não paga nada. */
            int fin_dono = -1;
            const char *t_it = (!usa_range && n->a && n->a->kind == N_CALL) ? tp_de(c, u, n->a) : "";
            int dono = !usa_range && n->a && n->a->kind == N_CALL
                       && (!t_it || !strcmp(t_it, "generator") || !strcmp(t_it, "Object"));
            if (usa_range) {
                int32_t na = n->a->lista.n;
                if (na == 1) {                       /* range(fim) */
                    emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 0, 0, NULL, 0));
                    expr(c, u, n->a->lista.itens[0]->a);
                } else {                             /* range(ini, fim[, passo]) */
                    expr(c, u, n->a->lista.itens[0]->a);
                    expr(c, u, n->a->lista.itens[1]->a);
                }
                if (na == 3) expr(c, u, n->a->lista.itens[2]->a);
                else         emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 1, 0, NULL, 0));
                emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 0, 0, NULL, 0));  /* contador */
                emite(c, u, OP_RANGE_PREPARA, 0);    /* decodifica os três uma vez */
                topo = UP(c, u)->ncode;
                fim = emite(c, u, OP_ITER_RANGE, 0);
            } else {
            expr(c, u, n->a);
            if (dono) emite(c, u, OP_ITER_DONO, 0);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 0, 0, NULL, 0));
            topo = UP(c, u)->ncode;
            fim = emite(c, u, OP_ITER_NEXT, 0);
            }
            /* variável do laço é local desta função, como o parâmetro.
             * Com `for each a, b in ...`, o item que o ITER_NEXT deixou no
             * topo e desempacotado pelo MESMO emissor do `a, b = [1, 2]` —
             * mesma checagem de quantidade, mesma mensagem de erro. A var
             * (ou o alvo) que uma funct do corpo captura vira célula do laço,
             * nova a cada volta — depois do iterável, que lê o de fora. */
            laco_abre(c, u, n->e ? NULL : var_laco, propria, n->e, n->b, NULL, &ativ_fe, &na_fe, &ca_fe);
            /* A sombra de uma variável que uma funct de fora do laço captura
             * (célula): o laço grava no slot dela, então cada volta ganha
             * célula nova, e a de fora, guardada pela sombra, não é tocada. */
            if (sombreia && sombra_e_celula(u, var_laco)) junta_nomes(c, &ativ_fe, &na_fe, &ca_fe, var_laco);
            celulas_novas_da_volta(c, u, ativ_fe, na_fe);
            if (n->e) guarda_em_alvo(c, u, n->e);
            else {
                memset(&c->grava, 0, sizeof(c->grava));
                c->grava.tem_valor = 1;
                c->grava.tipo = usa_range ? "int" : tp_elemento_de(c, u, n->a);
                c->grava.no = n;
                guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            }
            if (abre_laco(c, n, topo, usa_range ? 4 : 2) != 0) return;   /* estado do laço na pilha */
            /* a var do laço é re-atribuída no topo a cada volta, então limpá-la
             * por-iteração é inofensivo — corpo e var compartilham a marca */
            c->lacos[c->nlacos - 1].escopo_marca = M;
            c->lacos[c->nlacos - 1].escopo_marca_body = M;
            if (dono) {
                /* pendente como um finally: `break` e `return` de dentro o
                 * emitem (DONO_SAI) antes de saltar */
                if (c->nfinally >= c->cap_finally) {
                    int nc = c->cap_finally ? c->cap_finally * 2 : 16;
                    struct FinPend *nv = realloc(c->fin_pend, sizeof(struct FinPend) * (size_t)nc);
                    if (!nv) { cerro(c, "sem memoria", n); return; }
                    c->fin_pend = nv; c->cap_finally = nc;
                }
                fin_dono = c->nfinally++;
                c->fin_pend[fin_dono].bloco     = NULL;
                c->fin_pend[fin_dono].laco      = c->nlacos;
                c->fin_pend[fin_dono].unidade   = (const void *)u;
                c->fin_pend[fin_dono].armados   = 0;
                c->fin_pend[fin_dono].fecha     = NULL;
                c->fin_pend[fin_dono].dono_laco = c->nlacos - 1;
            }
            bloco_stmts(c, u, n->b);
            if (fin_dono >= 0) c->nfinally = fin_dono;   /* sai de pendente */
            escopo_emite_clears(c, u, M);        /* reset por-iteração */
            emite(c, u, OP_JUMP, topo);
            if (fim >= 0) UP(c, u)->code[fim + 1] = UP(c, u)->ncode;
            fecha_laco(c, u, topo);
            escopo_emite_clears(c, u, M);        /* saída: var do laço não vaza */
            escopo_trunca(c, u,M);
            celulas_fecha(c, u, ativ_fe, na_fe);
            if (sombreia) {                      /* devolve o de fora */
                sombra_devolve_carrega(c, u, var_laco, salvo);
                tp_sombra_devolve(c, u, var_laco, sim_posto, &sim_fora);
                sombra_devolve_fim(c, u, var_laco);
            }
            return;
        }

        case N_UNPACK_ASSIGNMENT: {
            /* alvo aninhado `a, (b, c) = ...` desempacota de novo por alvo */
            PSNode *alvo = n->a;
            if (!alvo || alvo->kind != N_UNPACK_TARGET) { cerro_sx(c, n, "alvo de desempacotamento invalido"); return; }
            expr(c, u, n->b);
            compila_unpack_alvo(c, u, alvo);
            if (n->is_private && !CFALHOU(c)) priv_alvos_unpack(c, u, n, alvo);
            return;
        }

        case N_YIELD_STMT: {
            /* `yield` fora de funct só quebrava rodando ("yield fora de
             * gerador"), e o `--check` dizia ok: o erro sai aqui, como o do
             * `return`. */
            if (u->eh_modulo) { cerro_sx(c, n, "'yield' fora de funct"); return; }
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
            /* campo repetido: o 2º valia pela metade, como no enum e na class */
            for (int32_t i = 0; i < n->lista.n; i++) {
                PSNode *f = n->lista.itens[i];
                if (!f->texto) continue;
                for (int32_t k = 0; k < i; k++) {
                    PSNode *o = n->lista.itens[k];
                    if (o->texto && strcmp(o->texto, f->texto) == 0) {
                        PSNode tmp;
                        terro(c, no_do_nome(f, &tmp), "SyntaxError",
                              "campo '%s' repetido no model %s (o primeiro esta na linha %d)",
                              f->texto, n->texto ? n->texto : "?", o->line);
                        break;
                    }
                }
            }
            for (int32_t i = 0; i < n->lista.n; i++) {
                PSNode *f = n->lista.itens[i];
                PSModelCampoDef *cd = &def->campos[i];
                cd->nome = strdup(f->texto ? f->texto : "?");
                cd->length = (f->i2 > 0) ? f->i2 : -1;
                cd->of_tipo = -1;
                /* a coluna `model` da tabela única decide o tipo; o que não
                 * está nela tem que ser OUTRO model (estrutura aninhada),
                 * deste arquivo ou importado */
                const PSTipoInfo *ti = ps_tipo_info(f->texto2);
                if (ti && ti->model) cd->tipo = ti->cod;
                else if (f->texto2 && modelo_conhecido(c, u, f->texto2)) {
                    cd->tipo = -1;
                    cd->tipo_model = strdup(f->texto2);
                    if (!cd->tipo_model) { cerro(c, "sem memoria", f); return; }
                } else {
                    cerro_sx(c, f, "tipo desconhecido em model: %s", f->texto2 ? f->texto2 : "?"); return;
                }
                if (compila_params_model(c, u, f, cd) != 0) return;
            }
            emite(c, u, OP_MAKE_MODEL, mi);
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.no = n;
            c->grava.declaracao = 1;
            /* o gêmeo podado, não o nó desta passada: o nó morre com o
             * statement, e a assinatura tem que valer até o fim (só aqui, nos
             * nós DESTE arquivo — a declaração de um módulo importado vem de
             * outra árvore e podia colidir por posição) */
            c->grava.decl = gemeo_de(c, n);
            guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            if (n->is_private) priv_declara(c, u, n, n, n->texto);
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
            /* Antes de rodar: membro repetido (o 2º sumia calado, `E.A` dava
             * o primeiro), e membro sem valor depois de um valor LITERAL que
             * não é inteiro (a contagem não tem de onde continuar; o valor
             * que só se sabe rodando é conferido no MAKE_ENUM). */
            for (int32_t i = 0; i < n->lista.n; i++) {
                PSNode *m = n->lista.itens[i];
                if (!m->texto) continue;
                for (int32_t k = 0; k < i; k++) {
                    PSNode *o = n->lista.itens[k];
                    if (o->texto && strcmp(o->texto, m->texto) == 0) {
                        PSNode tmp;
                        terro(c, no_do_nome(m, &tmp), "SyntaxError",
                              "membro '%s' repetido no enum %s (o primeiro esta na linha %d)",
                              m->texto, n->texto ? n->texto : "?", o->line);
                        break;
                    }
                }
                if (m->a || i == 0) continue;
                PSNode *ant = n->lista.itens[i - 1];
                PSNode tmp;
                if (ant->a && ant->a->kind == N_LITERAL && ant->a->lit != L_INT && ant->a->lit != L_BIGINT)
                    terro(c, no_do_nome(m, &tmp), "TypeError",
                          "enum %s: o membro %s nao tem valor e o anterior (%s) nao e inteiro "
                          "— so um inteiro continua a contagem: escreva o valor de %s",
                          n->texto ? n->texto : "?", m->texto, ant->texto ? ant->texto : "?", m->texto);
            }
            /* empilha os valores explícitos, na ordem de declaração */
            for (int32_t i = 0; i < n->lista.n && !CFALHOU(c); i++) {
                PSNode *m = n->lista.itens[i];
                if (m->a) expr(c, u, m->a);
            }
            if (CFALHOU(c)) return;
            emite(c, u, OP_MAKE_ENUM, ei);
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.no = n;
            c->grava.declaracao = 1;
            /* o gêmeo podado, não o nó desta passada: o nó morre com o
             * statement, e a assinatura tem que valer até o fim (só aqui, nos
             * nós DESTE arquivo — a declaração de um módulo importado vem de
             * outra árvore e podia colidir por posição) */
            c->grava.decl = gemeo_de(c, n);
            guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            if (n->is_private) priv_declara(c, u, n, n, n->texto);
            return;
        }

        case N_DECORATOR_STMT: {
            /* Três decoradores são resolvidos em COMPILAÇÃO (`static`,
             * `NonNull`, `dataentity`): mudam como a funct é gerada. Todo
             * OUTRO decorador é o protocolo geral, em runtime (OP_DECORA):
             * `@obj.metodo(args)`, `@log`, `@log()` — inclusive nome que não
             * existe, que dá NameError na linha do `@` em vez de sumir calado
             * com a funct embaixo (era o que acontecia). */
            PSNode *dec = n->a;
            const char *nome = (dec && dec->lista.n == 1) ? dec->lista.itens[0]->texto : NULL;
            int embutido = nome && (strcmp(nome, "dataentity") == 0
                                    || strcmp(nome, "static") == 0
                                    || strcmp(nome, "NonNull") == 0);
            /* `@app.route(...)`, `@app.get(...)`, `@app.post(...)`: o corpo da
             * funct decorada é HANDLER de rota, e é o único lugar onde o
             * `request` existe (motor-6). O nome do decorador é o caminho
             * `obj.metodo`; o que importa é o ÚLTIMO membro. */
            int eh_rota = 0;
            if (dec && dec->lista.n >= 2) {
                const char *ult = dec->lista.itens[dec->lista.n - 1]->texto;
                if (ult && (!strcmp(ult, "route") || !strcmp(ult, "get")
                            || !strcmp(ult, "post") || !strcmp(ult, "put")
                            || !strcmp(ult, "delete") || !strcmp(ult, "patch")
                            /* o middleware também recebe o `request`
                             * (docs/jinker/middleware/middleware.md) */
                            || !strcmp(ult, "middleware")))
                    eh_rota = 1;
            }
            if (nome && strcmp(nome, "dataentity") == 0) {
                if (n->b) stmts_de(c, u, n->b);
                return;
            }
            /* Decorador GERAL: avalia a expressão -> o decorador (guardado);
             * roda o bloco (define a funct); e entrega a funct ao OP_DECORA,
             * que registra (`.register`) ou envolve (chamável). Na funct solta
             * o nome passa a valer o resultado; em cima de classe, o 1º método
             * da classe passa a valer o resultado. */
            if (dec && !embutido) {
                if (!n->b) return;
                const char *act = NULL;
                const char *cls_nome = NULL, *met_nome = NULL;
                PSNode *corpo = n->b;
                /* decorador EMPILHADO: o bloco é outro N_DECORATOR_STMT; o de
                 * dentro aplica primeiro e rebinda o nome, e este carrega o
                 * nome já envolvido — a funct fica no fundo da pilha de @ */
                while (corpo && corpo->kind == N_BLOCK && corpo->lista.n == 1
                       && corpo->lista.itens[0]->kind == N_DECORATOR_STMT)
                    corpo = corpo->lista.itens[0]->b;
                if (corpo && corpo->kind == N_BLOCK)
                    for (int32_t i = 0; i < corpo->lista.n; i++) {
                        PSNode *bi = corpo->lista.itens[i];
                        if (bi->kind == N_FUNCT_DECL) { act = bi->texto; break; }
                        /* handler baseado em CLASSE: acha a funct DENTRO da
                         * classe (a primeira fora de __init__), nome qualquer */
                        if (bi->kind == N_ENTITY_DECL) {
                            cls_nome = bi->texto;
                            for (int32_t j = 0; j < bi->lista.n; j++) {
                                PSNode *m = bi->lista.itens[j];
                                if (m->kind == N_FUNCT_DECL && m->texto
                                        && strcmp(m->texto, "__init__") != 0) { met_nome = m->texto; break; }
                            }
                            break;
                        }
                    }

                if (cls_nome && !met_nome) {
                    cerro_sx(c, dec, "decorador em cima de Entity '%s' sem metodo: nao ha o que registrar", cls_nome);
                    return;
                }
                static const char *const REG[] = { "$reg0", "$reg1", "$reg2", "$reg3",
                                                   "$reg4", "$reg5", "$reg6", "$reg7" };
                if (c->decor_prof >= 8) { cerro(c, "limite do compilador: 8 decoradores empilhados", n); return; }
                const char *reg = REG[c->decor_prof];
                /* 1) avalia a expressão do decorador (chamada se teve
                 * parênteses) — erro do decorador propaga */
                emite_decorador_expr(c, u, dec);
                /* 2) guarda o decorador e roda o bloco (define a funct; um
                 * decorador empilhado ali dentro usa o próprio temporário) */
                guarda_nome_modo(c, u, reg, 1);
                c->decor_prof++;
                /* o corpo da funct decorada por rota é HANDLER: é onde o
                 * `request` existe (motor-6) */
                if (eh_rota) c->em_handler++;
                stmts_de(c, u, n->b);
                if (eh_rota) c->em_handler--;
                c->decor_prof--;
                if (CFALHOU(c)) return;
                /* 3) OP_DECORA: funct solta -> o nome passa a valer o resultado */
                if (act) {
                    carrega_nome(c, u, reg);
                    carrega_nome(c, u, act);
                    emite_decora(c, u, dec, 0);
                    /* o nome passa a valer o que o decorador devolveu */
                    memset(&c->grava, 0, sizeof(c->grava));
                    c->grava.redefine = 1;
                    guarda_nome_modo(c, u, act, 1);
                } else if (cls_nome && met_nome) {
                    /* decorador em cima da classe: vale pro 1º método dela, com
                     * o mesmo protocolo do decorador escrito em cima do método —
                     * o valor atual do método entra, o resultado volta pra
                     * tabela da classe. Empilhados em cima da classe compõem:
                     * o de dentro já gravou quando este lê. */
                    int32_t kmet = idx_const(c, u, K_STR, 0, 0, met_nome, (int32_t)strlen(met_nome));
                    carrega_nome(c, u, cls_nome);
                    emite(c, u, OP_LOAD_CONST, kmet);
                    carrega_nome(c, u, reg);
                    carrega_nome(c, u, cls_nome);
                    emite(c, u, OP_LOAD_METODO, kmet);
                    emite_decora(c, u, dec, 1);
                    guarda_nome_modo(c, u, "$dmval", 1);
                    carrega_nome(c, u, cls_nome);
                    carrega_nome(c, u, "$dmval");
                    emite(c, u, OP_SET_METODO, kmet);
                }
                return;
            }
            if (!n->b) return;
            if (nome && strcmp(nome, "static") == 0) {
                /* Marca a funct decorada como estática (chamável na Entity
                 * sem instância). Fora de Entity, a marca é inofensiva: só
                 * vale na resolução de `Classe.metodo`. */
                c->pendente_static = 1;
                stmts_de(c, u, n->b);
                c->pendente_static = 0;
                return;
            }
            if (nome && strcmp(nome, "NonNull") == 0) {
                c->pendente_nonnull = 1;
                stmts_de(c, u, n->b);
                c->pendente_nonnull = 0;
                return;
            }
            return;
        }

        case N_RUN_SELFWITH_STMT: {
            /* Igual ao interp: o bloco é PULADO quando o arquivo está sendo
             * IMPORTADO (só roda quando é o principal). OP_SKIP_IF_IMPORT
             * salta o bloco em runtime se vm->importando > 0. */
            int32_t s = emite(c, u, OP_SKIP_IF_IMPORT, 0);
            int32_t Mr = escopo_marca(u);
            bloco_stmts(c, u, n->b ? n->b : n->a);
            escopo_fecha(c, u, Mr);            /* vars do guard não vazam */
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
            /* NÃO abre escopo de bloco: no interp, a var do `using` e as
             * variáveis atribuídas no corpo SOBREVIVEM depois do bloco (é
             * deliberado). Manter igual pra não divergir. */
            const char *recurso = n->texto ? n->texto : "_";
            expr(c, u, n->a);
            grava_valor(c, u, n->a);
            guarda_nome_modo(c, u, recurso, 1);
            int32_t setup = emite(c, u, OP_SETUP_TRY, 0);
            c->dentro_try++;
            /* O fechamento fica PENDENTE como um `finally`: `return`, `break`
             * e `continue` de dentro do corpo o emitem antes de saltar. Antes
             * saíam sem fechar, e o arquivo escrito ficava vazio. */
            int fin_meu = -1;
            if (c->nfinally >= c->cap_finally) {
                int nc = c->cap_finally ? c->cap_finally * 2 : 16;
                struct FinPend *nv = realloc(c->fin_pend, sizeof(struct FinPend) * (size_t)nc);
                if (!nv) { cerro(c, "sem memoria", n); return; }
                c->fin_pend = nv; c->cap_finally = nc;
            }
            fin_meu = c->nfinally++;
            c->fin_pend[fin_meu].bloco   = NULL;
            c->fin_pend[fin_meu].laco    = c->nlacos;
            c->fin_pend[fin_meu].unidade = (const void *)u;
            c->fin_pend[fin_meu].armados = 1;
            c->fin_pend[fin_meu].fecha   = recurso;
            c->fin_pend[fin_meu].dono_laco = -1;
            stmts_de(c, u, n->b);   /* o que ele liga é do bloco de fora (`liga_direto`) */
            c->nfinally = fin_meu;  /* sai de pendente */
            c->dentro_try--;
            emite(c, u, OP_POP_TRY, 0);

            /* saída normal: fecha e pula o caminho de erro */
            carrega_nome(c, u, recurso);
            emite(c, u, OP_CLOSE_SE_TEM, 0);
            int32_t fim = emite(c, u, OP_JUMP, 0);

            /* saída por erro: fecha e relança o MESMO erro, com o tipo junto
             * da mensagem (como o try sem catch). O RAISE só com a mensagem
             * fazia de todo erro um RuntimeError — `catch (ZeroDivisionError
             * e)` não o pegava — e o `sys.exit` de dentro saía com código 1 */
            if (setup >= 0) UP(c, u)->code[setup + 1] = UP(c, u)->ncode;
            emite(c, u, OP_PUSH_ERR_TYPE, 0);
            c->pilha_extra += 2;               /* [mensagem, tipo] por baixo */
            carrega_nome(c, u, recurso);
            emite(c, u, OP_CLOSE_SE_TEM, 0);
            c->pilha_extra -= 2;
            emite(c, u, OP_RERAISE, 0);

            if (fim >= 0) UP(c, u)->code[fim + 1] = UP(c, u)->ncode;
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
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.tem_valor = 1; c->grava.tipo = "int"; c->grava.no = n;
            guarda_nome_modo(c, u, "self", 1);
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.tem_valor = 1; c->grava.tipo = "int"; c->grava.no = n;
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
            if (abre_laco(c, n, topo, 2) != 0) return;          /* lista + indice */
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
            escopo_trunca(c, u,M);
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
            def->fora = c->corpo ? c->corpo->ci : -1;
            def->campos_proto = -1;

            /* Os membros que a classe declara, com as marcas private e static:
             * os métodos, os campos do corpo e os campos declarados DENTRO das
             * functs (`private str name = nome` no __init__). Estes últimos
             * moram no corpo do método, mas a visibilidade é da CLASSE: sem
             * eles aqui o `private` compilaria e não barraria nada — pior que
             * não ter encapsulamento, porque parece que tem. Antes de compilar
             * os métodos: uma classe declarada lá dentro realoca o vetor. */
            {
                TabMembros tm = { c, def };
                membros_declarados(n, membro_tab_poe, &tm);
                if (CFALHOU(c)) return;
            }
            membros_repetidos(c, n);
            tp_confere_sobrescrita(c, n);

            /* campos de INSTÂNCIA com tipo escrito: a VM confere toda escrita
             * neles. O `static` pertence à classe, não entra. */
            for (int32_t i = 0; i < n->lista2_alias.n; i++) {
                PSNode *f = n->lista2_alias.itens[i];
                if (f->kind != N_ENTITY_FIELD || !f->texto || !f->texto2) continue;
                if (!tp_tipo_existe(c, u, f->texto2)) {
                    terro(c, f, "AttributedValueError", "tipo %s não existe (campo %s de %s)",
                          f->texto2, f->texto, n->texto ? n->texto : "?");
                    continue;
                }
                if (f->a) {
                    const char *T = tp_canon(f->texto2);
                    const char *vt = tp_de(c, u, f->a);
                    if (tp_aceita(c, T, vt, f->a, 1) == T_NAO)
                        terro(c, f->a, "AttributedValueError", "campo %s de %s esperava %s, recebeu %s",
                              f->texto, n->texto ? n->texto : "?", T, vt);
                }
                if (!f->is_static) classe_tip_add(c, def, f->texto, f->texto2);
            }
            for (int32_t i = 0; i < n->lista.n; i++)
                varre_campos_decl_tipados(c, n->lista.itens[i], def);

            int32_t nm = 0;
            for (int32_t i = 0; i < n->lista.n; i++)
                if (n->lista.itens[i]->kind == N_FUNCT_DECL) nm++;
            if (nm > 0) {
                def->met_nomes = calloc((size_t)nm, sizeof(char *));
                def->met_protos = calloc((size_t)nm, sizeof(int32_t));
                if (!def->met_nomes || !def->met_protos) { cerro(c, "sem memoria", n); return; }
            }
            const char *pai_salvo = c->entity_pai;
            int dentro_salvo = c->dentro_entity;
            PSNode *no_salvo = c->entity_no;
            c->entity_pai = (n->lista2.n > 0) ? n->lista2.itens[0]->texto : NULL;
            c->dentro_entity = 1;
            c->entity_no = n;   /* nome solto -> campo `static` desta classe */
            /* Daqui até o fim do corpo, todo proto que nascer (método, lambda,
             * funct aninhada, o `__init__` gerado) é desta classe. Toda saída
             * deste case passa pelo `fim_classe`, que desempilha. */
            CorpoClasse corpo_aqui = { n, ci, !u->eh_modulo, c->corpo };
            c->corpo = &corpo_aqui;
            tp_self_dos_metodos(c, n);   /* classe que a pré-passada não viu (aninhada) */
            /* Dentro de Entity o decorador é uma entrada SEPARADA do corpo
             * (dec_sem_captura no parser): ele não embrulha a funct. Então o
             * `@static` visto aqui vale pra PRÓXIMA funct da lista — é assim
             * que a marca chega no Proto (Proto.eh_static). */
            int static_pendente = 0, nonnull_pendente = 0;
            /* Decorador GERAL (`@mapp.post("/x")`) em cima de um método: era
             * DESCARTADO aqui — o `continue` abaixo pulava o nó, o método
             * compilava sem registro nenhum e a rota nunca existia, calada.
             * Os pares (decorador, método) ficam guardados e são emitidos
             * DEPOIS de a classe existir, porque a expressão do decorador pode
             * ler um campo `static` que só nasce então. O vetor dos pares
             * cresce: com 64 fixos, o 65º par era largado calado. */
            PSNode *dec_pend[8];  int ndec_pend = 0;
            PSNode **par_dec = NULL, **par_met = NULL;  int npares = 0, cap_pares = 0;
            int32_t ini_corpo = -1;          /* o trecho do corpo que roda aqui */
            for (int32_t i = 0; i < n->lista.n && !CFALHOU(c); i++) {
                PSNode *m = n->lista.itens[i];
                if (m->kind == N_DECORATOR_STMT) {
                    PSNode *dec = m->a;
                    const char *dn = (dec && dec->lista.n == 1) ? dec->lista.itens[0]->texto : NULL;
                    if (dn && strcmp(dn, "static") == 0) static_pendente = 1;
                    /* `@NonNull` dentro de Entity era DESCARTADO junto com o
                     * nó do decorador: o método rodava sem checagem nenhuma
                     * enquanto o interpretador recusava o Null. */
                    if (dn && strcmp(dn, "NonNull") == 0) nonnull_pendente = 1;
                    /* todo decorador que não é um dos três embutidos vai pro
                     * protocolo geral — `@log` num método era ignorado calado */
                    int emb = dn && (strcmp(dn, "static") == 0 || strcmp(dn, "NonNull") == 0
                                     || strcmp(dn, "dataentity") == 0);
                    if (dec && !emb) {
                        if (ndec_pend >= 8) { cerro(c, "limite do compilador: 8 decoradores num metodo", m); goto fim_classe; }
                        dec_pend[ndec_pend++] = dec;
                    }
                    continue;
                }
                if (m->kind != N_FUNCT_DECL) continue;
                for (int k = 0; k < ndec_pend; k++) {
                    if (npares == cap_pares) {
                        int novo = cap_pares < 8 ? 8 : cap_pares * 2;
                        PSNode **nd = realloc(par_dec, sizeof(PSNode *) * (size_t)novo);
                        if (!nd) { cerro(c, "sem memoria", m); goto fim_classe; }
                        par_dec = nd;
                        PSNode **nm2 = realloc(par_met, sizeof(PSNode *) * (size_t)novo);
                        if (!nm2) { cerro(c, "sem memoria", m); goto fim_classe; }
                        par_met = nm2;
                        cap_pares = novo;
                    }
                    par_dec[npares] = dec_pend[k];
                    par_met[npares] = m;
                    npares++;
                }
                ndec_pend = 0;
                c->pendente_static = static_pendente;
                c->pendente_nonnull = nonnull_pendente;
                c->pendente_metodo = 1;
                /* O pai é a unidade onde a classe foi declarada: na funct, o
                 * método é closure dela (escopo léxico, doc 4.6) e alcança o
                 * que ela declara, a própria classe inclusive. No topo do
                 * arquivo não muda nada: unidade de módulo não é pai. */
                int32_t pi = compila_funct(c, m, u);
                c->pendente_static = 0;
                c->pendente_nonnull = 0;
                c->pendente_metodo = 0;
                static_pendente = 0;
                nonnull_pendente = 0;
                if (CFALHOU(c)) goto fim_classe;
                def = &c->out->classes[ci];               /* realloc pode ter mexido */
                def->met_nomes[def->nmetodos] = strdup(m->texto ? m->texto : "?");
                def->met_protos[def->nmetodos] = pi;
                def->nmetodos++;
            }
            c->entity_pai = pai_salvo;
            c->dentro_entity = dentro_salvo;

            /* Campos tipados sem `__init__` escrito à mão: a Entity ganha um
             * gerado, com um parâmetro por campo na ordem de declaração. */
            /* Campo `static` NÃO é parâmetro do `__init__`: pertence à classe,
             * não à instância. O `sintetiza_init` recebe só os de instância. */
            int32_t n_inst = 0;
            for (int32_t i = 0; i < n->lista2_alias.n; i++)
                if (!n->lista2_alias.itens[i]->is_static) n_inst++;
            if (n_inst > 0 && !CFALHOU(c)) {
                int tem_init = 0;
                for (int32_t i = 0; i < def->nmetodos; i++)
                    if (strcmp(def->met_nomes[i], "__init__") == 0) { tem_init = 1; break; }
                if (!tem_init) {
                    PSNodeVec todos = n->lista2_alias;
                    PSNode **so_inst = calloc((size_t)n_inst, sizeof(PSNode *));
                    if (!so_inst) { cerro(c, "sem memoria", n); goto fim_classe; }
                    int32_t k = 0;
                    for (int32_t i = 0; i < todos.n; i++)
                        if (!todos.itens[i]->is_static) so_inst[k++] = todos.itens[i];
                    n->lista2_alias.itens = so_inst;
                    n->lista2_alias.n = n_inst;
                    int32_t pi = sintetiza_init(c, n);
                    n->lista2_alias = todos;
                    free(so_inst);
                    if (CFALHOU(c)) goto fim_classe;
                    def = &c->out->classes[ci];
                    /* Cada campo é PUBLICADO assim que o realloc dele dá certo.
                     * Guardar os dois pra publicar no fim parecia mais limpo e
                     * era liberação dupla: o realloc que dá certo já soltou o
                     * bloco antigo, então `free(mn)` no erro do SEGUNDO deixava
                     * `def->met_nomes` apontando pra memória morta — e o
                     * destrutor da classe passa lá liberando de novo. */
                    char **mn = realloc(def->met_nomes, sizeof(char *) * (size_t)(def->nmetodos + 1));
                    if (!mn) { cerro(c, "sem memoria", n); goto fim_classe; }
                    def->met_nomes = mn;
                    int32_t *mp = realloc(def->met_protos, sizeof(int32_t) * (size_t)(def->nmetodos + 1));
                    if (!mp) { cerro(c, "sem memoria", n); goto fim_classe; }
                    def->met_protos = mp;
                    def->met_nomes[def->nmetodos] = strdup("__init__");
                    def->met_protos[def->nmetodos] = pi;
                    def->nmetodos++;
                }
            }

            /* os campos com valor padrão: o proto que a instanciação roda antes
             * do `__init__` (o descritor já pode ter sido realocado) */
            if (!CFALHOU(c)) {
                int32_t pc = sintetiza_campos(c, n, u);
                if (CFALHOU(c)) goto fim_classe;
                c->out->classes[ci].campos_proto = pc;
                def = &c->out->classes[ci];
            }

            /* empilha os pais na ordem declarada */
            for (int32_t i = 0; i < n->lista2.n; i++) {
                tp_confere_nome(c, u, n->lista2.itens[i]->texto, n->lista2.itens[i]);
                carrega_nome(c, u, n->lista2.itens[i]->texto ? n->lista2.itens[i]->texto : "");
            }
            emite(c, u, OP_MAKE_CLASS, ci);
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.tem_valor = 1;
            c->grava.tipo = "Entity";
            c->grava.no = n;
            c->grava.declaracao = 1;
            /* o gêmeo podado, não o nó desta passada: o nó morre com o
             * statement, e a assinatura tem que valer até o fim (só aqui, nos
             * nós DESTE arquivo — a declaração de um módulo importado vem de
             * outra árvore e podia colidir por posição) */
            c->grava.decl = gemeo_de(c, n);
            guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            /* `private class`: alem de `classe_privada`, entra na tabela dos
             * nomes private do modulo — e ela que diz ONDE foi declarada */
            if (n->is_private) priv_declara(c, u, n, n, n->texto);

            /* A classe existe. Agora, nesta ordem:
             *   1) campos `static` com valor — `Classe.x = <inicializador>`
             *      (o sem valor já nasceu Null no MAKE_CLASS, pela tabela de
             *      membros: um `Classe.x = Null` aqui seria recusado pelo tipo
             *      do campo);
             *   2) decoradores dos métodos — a expressão do decorador pode
             *      ler esses campos (`@mapp.post(...)`), por isso vem depois.
             * Tudo com `entity_no` ainda apontando pra esta classe: é o que
             * faz `mapp` virar `App.mapp` dentro dessas expressões. E é corpo
             * da classe rodando aqui fora: a faixa diz isso ao `private`. A
             * classe se carrega pelo nome como variável, nunca como o campo
             * `static` homônimo. */
            ini_corpo = UP(c, u)->ncode;
            const char *cls_nome_aqui = n->texto ? n->texto : "";
            for (int32_t i = 0; i < n->lista2_alias.n && !CFALHOU(c); i++) {
                PSNode *f = n->lista2_alias.itens[i];
                if (!f->is_static || !f->texto || !f->a) continue;
                carrega_nome_sem_estatico(c, u, cls_nome_aqui);
                expr(c, u, f->a);
                emite(c, u, OP_SET_MEMBER,
                      idx_const(c, u, K_STR, 0, 0, f->texto, (int32_t)strlen(f->texto)));
            }
            /* Os decoradores de CADA método, na mesma regra da funct solta: as
             * expressões avaliadas de cima pra baixo, a aplicação de baixo pra
             * cima (`@a @b m` = `a(b(m))`), cada uma recebendo o valor que a de
             * baixo deixou. O valor sai da tabela de métodos da classe
             * (LOAD_METODO) e o resultado final volta pra ela (SET_METODO) — é
             * ele que `inst.m` liga ao receptor. Os pares vêm agrupados por
             * método, na ordem do fonte. */
            static const char *const DM[] = { "$dm0", "$dm1", "$dm2", "$dm3",
                                               "$dm4", "$dm5", "$dm6", "$dm7" };
            for (int k = 0; k < npares && !CFALHOU(c); ) {
                PSNode *met = par_met[k];
                const char *mn = met->texto ? met->texto : "";
                int ini = k, fim = k;
                while (fim < npares && par_met[fim] == met) fim++;
                k = fim;
                if (fim - ini > 8) { cerro(c, "limite do compilador: 8 decoradores num metodo", met); goto fim_classe; }
                for (int q = ini; q < fim; q++) {
                    PSNode *dec = par_dec[q];
                    /* `@mapp.post(...)` com `mapp` sendo campo de INSTÂNCIA da
                     * própria classe: na hora em que o corpo é declarado não há
                     * instância, e o nome cairia num NameError apontando pra
                     * linha da classe — sem dizer que o que falta é `static`. */
                    const char *raiz = dec->lista.itens[0]->texto;
                    for (int32_t i = 0; raiz && i < n->lista2_alias.n; i++) {
                        PSNode *f = n->lista2_alias.itens[i];
                        if (f->kind == N_ENTITY_FIELD && !f->is_static && f->texto
                                && strcmp(f->texto, raiz) == 0) {
                            cerro_sx(c, dec, "'%s' e campo de instancia — o decorador no corpo da classe "
                                     "roda antes de existir instancia; declare-o `static`: "
                                     "static %s = ...", raiz, raiz);
                            goto fim_classe;
                        }
                    }
                    emite_decorador_expr(c, u, dec);
                    guarda_nome_modo(c, u, DM[q - ini], 1);
                }
                int32_t kmet = idx_const(c, u, K_STR, 0, 0, mn, (int32_t)strlen(mn));
                carrega_nome_sem_estatico(c, u, cls_nome_aqui);
                emite(c, u, OP_LOAD_METODO, kmet);
                guarda_nome_modo(c, u, "$dmval", 1);
                for (int q = fim - 1; q >= ini && !CFALHOU(c); q--) {
                    carrega_nome_sem_estatico(c, u, cls_nome_aqui);
                    emite(c, u, OP_LOAD_CONST, kmet);
                    carrega_nome(c, u, DM[q - ini]);
                    carrega_nome(c, u, "$dmval");
                    emite_decora(c, u, par_dec[q], 1);
                    guarda_nome_modo(c, u, "$dmval", 1);
                }
                carrega_nome_sem_estatico(c, u, cls_nome_aqui);
                carrega_nome(c, u, "$dmval");
                emite(c, u, OP_SET_METODO, kmet);
            }
            if (!CFALHOU(c)) faixa_classe_poe(c, u, ini_corpo, UP(c, u)->ncode, ci);
        fim_classe:
            free(par_dec);
            free(par_met);
            c->corpo = corpo_aqui.fora;
            c->entity_no = no_salvo;
            c->entity_pai = pai_salvo;
            c->dentro_entity = dentro_salvo;
            return;
        }

        case N_INDEX_ASSIGNMENT: {
            /* `s[0] = "x"` em str, tup ou byte: a VM recusa no INDEX_SET; com
             * o tipo escrito certo, a recusa sai antes de rodar, com a frase
             * dela */
            {
                const char *tc = tp_basico(tp_de(c, u, n->a));
                if (tc && (!strcmp(tc, "str") || !strcmp(tc, "tup") || !strcmp(tc, "byte")))
                    terro(c, n, "TypeError", "'%s' object does not support item assignment", tc);
            }
            expr(c, u, n->a);                 /* container */
            expr(c, u, n->b);                 /* índice    */
            if (n->texto && strcmp(n->texto, "=") != 0) {
                /* aumentada: lê o valor atual sem re-avaliar container/índice */
                emite(c, u, OP_DUP2, 0);
                emite(c, u, OP_INDEX_GET, 0);
                expr(c, u, n->c);
                char bin[2] = { n->texto[0], 0 };
                int32_t op = op_binario(bin);
                if (op < 0) { cerro_sx(c, n, "operador de atribuicao invalido"); return; }
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
            if (!n->texto) { cerro_sx(c, n, "import mal formado"); return; }
            if (n->i2 > 0 && n->lista.n == 0) {
                cerro_sx(c, n, "import relativo precisa de um modulo depois dos pontos "
                               "(ex: from .modulo import x)");
                return;
            }
            char encoded[512];
            import_modulo_codificado(n, encoded);
            /* O nome ligado sem `as` é o do arquivo, sem pasta e sem extensão. */
            char base_aspas[256];
            import_nome_do_arquivo(n, base_aspas, sizeof(base_aspas));
            const char *mod = encoded;

            /* `from m import *` / `import m *` / `PUSH m GET *`: os nomes já
             * vieram do resolvedor (pré-passada em ps_compila_com); aqui vira
             * a lista explícita. Só no topo do arquivo: dentro de funct os
             * slots são decididos na compilação, e o fim de um bloco apaga os
             * nomes que nasceram nele por nome conhecido — nome que só o
             * módulo sabe não teria como ser apagado. */
            if (n->texto3) {
                if (!u->eh_modulo || n != c->stmt_topo) {
                    if (n->i2 == -1)
                        cerro_sx(c, n, "`*` do import so vale no topo do arquivo; dentro de funct ou bloco "
                                       "nomeie o que usa: from '%s' import a, b", mod + 1);
                    else
                        cerro_sx(c, n, "`*` do import so vale no topo do arquivo; dentro de funct ou bloco "
                                       "nomeie o que usa: from %s import a, b", mod);
                    return;
                }
                int32_t e = -1;
                for (int32_t k = 0; k < c->nestrelas; k++) if (mesmo_no(c->estrelas[k].no, n)) { e = k; break; }
                emite_import(c, u, n, OP_IMPORT_MOD, mod, NULL);
                if ((e < 0 || !c->estrelas[e].resolvido) && c->resolve && c->resolve->nomes_de)
                    c->out->estrela_incompleta = 1;
                if (e < 0 || !c->estrelas[e].resolvido) {
                    /* Não resolveu na compilação: o IMPORT_MOD dá o erro de
                     * sempre (módulo ausente, que não compila). Se o módulo
                     * só aparecer em runtime, o "*" diz por que não há nomes. */
                    emite(c, u, OP_IMPORT_FROM_ESTRELA, idx_const(c, u, K_STR, 0, 0, "*", 1));
                    emite(c, u, OP_POP_TOP, 0);
                    return;
                }
                for (int32_t k = 0; k < c->estrelas[e].n && !CFALHOU(c); k++) {
                    const char *nm = c->estrelas[e].nomes[k];
                    emite(c, u, OP_DUP, 0);
                    emite(c, u, OP_IMPORT_FROM_ESTRELA, idx_const(c, u, K_STR, 0, 0, nm, (int32_t)strlen(nm)));
                    int32_t pula = emite(c, u, OP_JUMP_SE_UNSET, 0);
                    guarda_nome(c, u, nm);
                    if (pula >= 0) UP(c, u)->code[pula + 1] = UP(c, u)->ncode;
                }
                emite(c, u, OP_POP_TOP, 0);
                return;
            }
            /* `import pacote.modulo` liga o ÚLTIMO segmento (`modulo`), como
             * a doc diz e o interpretador faz — antes a VM recusava com
             * NotImplementedError. Import RELATIVO (`import .x`) segue exigindo
             * `from`, porque aí não há nome óbvio pra ligar. */
            int simples = (n->i2 == -1) || (n->i2 == 0 && n->lista.n >= 1);
            const char *ultimo = (n->i2 == -1 || base_aspas[0]) ? base_aspas : mod;
            /* `import 'meu-mod.pr'` sem `as`: o nome do arquivo tem que servir
             * de nome de variavel, senao nao ha o que ligar. */
            if (n->i2 == -1 && !n->texto2 && n->lista2.n == 0) {
                int ok_nome = ultimo[0] != '\0';
                for (const char *q = ultimo; ok_nome && *q; q++) {
                    int letra = (*q >= 'a' && *q <= 'z') || (*q >= 'A' && *q <= 'Z') || *q == '_';
                    int digito = (*q >= '0' && *q <= '9');
                    if (!(letra || (digito && q != ultimo))) ok_nome = 0;
                }
                if (!ok_nome) {
                    cerro_sx(c, n, "'%s' nao serve de nome de variavel: ligue com `as` (import '%s' as nome)",
                             ultimo, mod + 1);
                    return;
                }
            }

            /* `PUSH mod` é `import mod`; `PUSH mod GET a, b` é
             * `from mod import a, b`. Com GET, o módulo NÃO fica visível —
             * nem sob o `as`, que nesse caso não liga nada. É o que o
             * interpretador faz. */
            if (strcmp(n->texto, "push") == 0) {
                if (n->lista2.n == 0) {
                    if (!simples) { cerro_sx(c, n, "import de modulo pontuado/relativo precisa de 'from ... import ...'"); return; }
                    emite_import(c, u, n, OP_IMPORT_MOD, mod, NULL);
                    tp_grava_import(c, n, mod, NULL);
                    guarda_nome(c, u, n->texto2 ? n->texto2 : ultimo);
                    return;
                }
                for (int32_t i = 0; i < n->lista2.n; i++) {
                    const char *membro = n->lista2.itens[i]->texto;
                    const char *apelido = (i < n->lista2_alias.n && n->lista2_alias.itens[i])
                                        ? n->lista2_alias.itens[i]->texto : membro;
                    emite_import(c, u, n, OP_IMPORT_MOD, mod, NULL);
                    emite_import(c, u, n, OP_IMPORT_FROM, membro, membro);
                    tp_grava_import(c, n, mod, membro);
                    guarda_nome(c, u, apelido);
                }
                return;
            }

            if (strcmp(n->texto, "import") == 0) {
                if (!simples) { cerro_sx(c, n, "import de modulo pontuado/relativo precisa de 'from ... import ...'"); return; }
                emite_import(c, u, n, OP_IMPORT_MOD, mod, NULL);
                tp_grava_import(c, n, mod, NULL);
                guarda_nome(c, u, n->texto2 ? n->texto2 : ultimo);
                return;
            }
            /* `from mod import a, b as c` — um GET_MEMBER por nome pedido */
            for (int32_t i = 0; i < n->lista2.n; i++) {
                const char *membro = n->lista2.itens[i]->texto;
                const char *apelido = (i < n->lista2_alias.n && n->lista2_alias.itens[i])
                                    ? n->lista2_alias.itens[i]->texto : membro;
                emite_import(c, u, n, OP_IMPORT_MOD, mod, NULL);
                emite_import(c, u, n, OP_IMPORT_FROM, membro, membro);
                tp_grava_import(c, n, mod, membro);
                guarda_nome(c, u, apelido);
            }
            return;
        }

        case N_MEMBER_ASSIGNMENT: {
            /* `base(Pai).x = v`: o campo mora no MESMO objeto que `self.x`;
             * escrever por `base` só esconderia isso. */
            if (n->a && n->a->kind == N_BASE_CALL_NODE) {
                cerro_sx(c, n, "atribua pelo self: self.%s = ... (base(Pai).%s le o campo, nao grava)",
                         n->texto ? n->texto : "x", n->texto ? n->texto : "x");
                return;
            }
            /* O erro do membro aponta o `.` dele, como na leitura (o nó fica
             * no começo do comando: `c.saldo = 999` apontava o `c`). */
            PSNode no_membro;
            memset(&no_membro, 0, sizeof(no_membro));
            no_membro.line = n->i ? (int32_t)(n->i >> 32) : n->line;
            no_membro.col  = n->i ? (int32_t)(uint32_t)(n->i & 0xffffffff) : n->col;
            /* Entity do arquivo: o campo tem que existir nela (declarado no
             * corpo, `private <tipo> x` ou gravado em `self.x` por um método),
             * e campo tipado recebe o tipo dele. A VM confere o tipo de novo
             * rodando — é o que pega o `obj` que só se sabe rodando. */
            {
                const char *t = tp_de(c, u, n->a);
                PSNode *d = (t && !strchr(t, '|')) ? tp_tipo_arq(c, t) : NULL;
                if (d && d->kind == N_ENTITY_DECL && n->texto) {
                    int nerros_antes = c->nerros;
                    if (tp_classe_tem(c, t, n->texto, 0) == 0) {
                        char dica[160];
                        terro(c, n, "AttributeError", "'%s' object has no attribute '%s'%s", t, n->texto,
                              tp_dica_membro(c, t, n->texto, dica, sizeof(dica)));
                    } else
                        tp_confere_privado(c, d, n->texto, 0, &no_membro);
                    const char *ft = c->nerros == nerros_antes ? tp_campo_tipo(c, t, n->texto, 0) : NULL;
                    if (ft && (!n->texto2 || !strcmp(n->texto2, "="))) {
                        const char *vt = tp_de(c, u, n->b);
                        if (tp_aceita(c, ft, vt, n->b, 1) == T_NAO)
                            terro(c, n->b, "AttributedValueError", "campo %s de %s esperava %s, recebeu %s",
                                  n->texto, t, ft, vt);
                    }
                }
                /* `Classe.x = v`: o private pelo nome da classe, e o campo
                 * `static` tipado recebe o tipo dele (a frase é a do campo de
                 * instância, com a classe que o declara). Acesso negado é a
                 * causa: o tipo de um valor que nem pode entrar não se
                 * confere. */
                /* `Cor.RED = 99`: membro de enum é constante (a frase da VM) */
                {
                    SimInfo *se = (n->a && n->a->kind == N_NAME && n->a->texto)
                                ? tp_sim_de(c, u, n->a->texto) : NULL;
                    if (se && se->decl && se->decl->kind == N_ENUM_DECL && n->texto) {
                        int membro = 0;
                        for (int32_t i = 0; i < se->decl->lista.n && !membro; i++)
                            membro = se->decl->lista.itens[i]->texto
                                  && !strcmp(se->decl->lista.itens[i]->texto, n->texto);
                        if (membro)
                            terro(c, &no_membro, "AttributeError",
                                  "membro '%s' do enum %s e constante: nao se reatribui",
                                  n->texto, se->decl->texto ? se->decl->texto : "?");
                        else
                            terro(c, &no_membro, "AttributeError",
                                  "enum %s nao ganha membro novo ('%s'): os membros sao os da declaracao",
                                  se->decl->texto ? se->decl->texto : "?", n->texto);
                    }
                }
                PSNode *ce = tp_classe_exata(c, u, n->a);
                int nerros_antes = c->nerros;
                if (ce && n->texto) tp_confere_privado(c, ce, n->texto, 1, &no_membro);
                if (ce && n->texto && c->nerros == nerros_antes) {
                    PSNode *dono = NULL;
                    PSNode *f = estatico_em(c, ce, n->texto, 0, &dono);
                    if (f && f->kind == N_ENTITY_FIELD && f->texto2 && (!n->texto2 || !strcmp(n->texto2, "="))) {
                        const char *ft = tp_canon(f->texto2);
                        const char *vt = tp_de(c, u, n->b);
                        if (tp_aceita(c, ft, vt, n->b, 1) == T_NAO)
                            terro(c, n->b, "AttributedValueError", "campo %s de %s esperava %s, recebeu %s",
                                  n->texto, dono && dono->texto ? dono->texto : "?", ft, vt);
                    }
                }
            }
            int32_t mi = idx_const(c, u, K_STR, 0, 0, n->texto ? n->texto : "",
                                   n->texto ? (int32_t)strlen(n->texto) : 0);
            expr(c, u, n->a);                 /* objeto */
            int32_t l0 = c->linha_atual, c0 = c->coluna_atual;
            if (n->texto2 && strcmp(n->texto2, "=") != 0) {
                /* aumentada `obj.x += v`: lê o atual sem re-avaliar o objeto */
                emite(c, u, OP_DUP, 0);
                c->linha_atual = no_membro.line; c->coluna_atual = no_membro.col;
                emite(c, u, OP_GET_MEMBER, mi);
                c->linha_atual = l0; c->coluna_atual = c0;
                expr(c, u, n->b);
                char bin[2] = { n->texto2[0], 0 };
                int32_t op = op_binario(bin);
                if (op < 0) { cerro_sx(c, n, "operador de atribuicao invalido"); return; }
                emite(c, u, op, 0);
            } else {
                expr(c, u, n->b);
            }
            c->linha_atual = no_membro.line; c->coluna_atual = no_membro.col;
            emite(c, u, OP_SET_MEMBER, mi);
            c->linha_atual = l0; c->coluna_atual = c0;
            return;
        }

        case N_MATCH_STMT: {
            expr(c, u, n->a);                  /* sujeito fica na pilha */
            Saltos fins = {0};
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
                c->pilha_extra++;              /* o sujeito fica em cima durante o corpo */
                bloco_stmts(c, u, caso->b);
                c->pilha_extra--;
                /* Os TRES caminhos (casou e rodou o corpo; padrão falhou;
                 * guarda falhou) convergem num ponto só, e a limpeza das
                 * variáveis do case (`escopo_fecha`) roda ali, pra todos.
                 *
                 * Antes a limpeza só ficava no caminho de sucesso. Numa
                 * funct, `case v if v < 50` com a guarda falsa deixava o
                 * slot de `v` preenchido; o slot era devolvido ao pool e o
                 * próximo nome sem declaração (`post`, resolvido por
                 * LOAD_NAME) caía nele — e o 999 do sujeito era "chamado":
                 * "'int' object is not callable". Só dentro de function, só
                 * com guarda falsa, e "consertava" se o corpo seguinte
                 * atribuísse algo antes — exatamente o que se mediu. */
                emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 1, 0, NULL, 0));
                int32_t casou = emite(c, u, OP_JUMP, 0);
                if (falhou >= 0) UP(c, u)->code[falhou + 1] = UP(c, u)->ncode;
                if (falhou_guarda >= 0) UP(c, u)->code[falhou_guarda + 1] = UP(c, u)->ncode;
                emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_BOOL, 0, 0, NULL, 0));
                UP(c, u)->code[casou + 1] = UP(c, u)->ncode;
                escopo_fecha(c, u, Mc);        /* captura/vars do case não vazam — nos 3 caminhos */
                saltos_push(c, &fins, emite(c, u, OP_JUMP_IF_TRUE, 0));
            }
            int32_t fim = UP(c, u)->ncode;
            saltos_corrige(c, u, &fins, fim);
            saltos_solta(&fins);
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
            /* enquanto o corpo (e os catches) compilam, este `finally` fica
             * PENDENTE: `return`/`break`/`continue` lá dentro emitem o bloco
             * antes de saltar, senão ele não roda nesses caminhos. */
            int fin_meu = -1;
            if (c->nfinally >= c->cap_finally) {
                int nc = c->cap_finally ? c->cap_finally * 2 : 16;
                struct FinPend *nv = realloc(c->fin_pend, sizeof(struct FinPend) * (size_t)nc);
                if (!nv) { cerro(c, "sem memoria", n); return; }
                c->fin_pend = nv; c->cap_finally = nc;
            }
            fin_meu = c->nfinally;
            c->fin_pend[fin_meu].bloco   = n->c;      /* pode ser NULL (sem finally) */
            c->fin_pend[fin_meu].laco    = c->nlacos;
            c->fin_pend[fin_meu].unidade = (const void *)u;
            c->fin_pend[fin_meu].armados = 1;         /* o SETUP_TRY do corpo */
            c->fin_pend[fin_meu].fecha   = NULL;
            c->fin_pend[fin_meu].dono_laco = -1;
            c->nfinally++;
            int32_t Mt = escopo_marca(u);
            /* o que o corpo (e depois os catches) criar é apagado no alvo de
             * cada handler: os CLEAR do fim do bloco só rodam no caminho normal */
            int32_t pico_fora = u->pico_locais, hist0 = u->n_mod_hist;
            u->pico_locais = u->nlocais;
            int pega_import = 0;
            for (int32_t i = 0; i < n->lista.n; i++) {
                PSNode *cl = n->lista.itens[i];
                if (cl && (!cl->texto2 || ps_excecao_eh("ImportError", cl->texto2))) pega_import = 1;
            }
            c->import_protegido += pega_import;
            bloco_stmts(c, u, n->a);
            c->import_protegido -= pega_import;
            escopo_fecha(c, u, Mt);            /* vars do try não vazam (saída normal) */
            c->dentro_try--;
            emite(c, u, OP_POP_TRY, 0);
            int32_t pula_catches = emite(c, u, OP_JUMP, 0);

            if (setup >= 0) UP(c, u)->code[setup + 1] = UP(c, u)->ncode;
            limpa_no_handler(c, u, Mt, hist0);

            /* Erro levantado DENTRO de um catch também tem que passar pelo
             * finally. O bloco inline só cobre a saída normal e o "nenhum
             * catch casou"; um `raise` no corpo do catch desenrolava por cima
             * dele. Um try interno segura esse caso e repropaga. */
            int32_t setup_cat = -1;
            if (n->c) setup_cat = emite(c, u, OP_SETUP_TRY, 0);
            /* nos catches o handler do corpo já saiu (o erro o consumiu); fica
             * armado só o de cima, quando há finally */
            c->fin_pend[fin_meu].armados = setup_cat >= 0 ? 1 : 0;

            /* Cada catch: se tem tipo, compara; senão captura tudo.
             * A mensagem já está no topo quando chegamos aqui. */
            Saltos fins = {0};
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
                    /* EXC_CASA, nao EQ: o catch pergunta "este tipo E do tipo
                     * pedido?", e a resposta vem da tabela de excecoes, entao
                     * o pai pega o filho. Com EQ, `catch (Exception e)` nao
                     * pegava nada. */
                    emite(c, u, OP_EXC_CASA, 0);
                    prox_falha = emite(c, u, OP_JUMP_IF_FALSE, 0);
                } else {
                    /* O catch sem tipo pega tudo MENOS o `sys.exit`: ele
                     * atravessa, roda os finally e sai (ps_retornos.h). Sem
                     * o teste, `catch (e)` engolia o exit e o programa
                     * seguia. Só roda quando um erro chegou aqui. */
                    emite(c, u, OP_PUSH_ERR_TYPE, 0);
                    emite(c, u, OP_LOAD_CONST,
                          idx_const(c, u, K_STR, 0, 0, PS_TIPO_SAIDA, (int32_t)strlen(PS_TIPO_SAIDA)));
                    emite(c, u, OP_EXC_CASA, 0);
                    prox_falha = emite(c, u, OP_JUMP_IF_TRUE, 0);
                }
                /* liga a mensagem ao nome do catch e roda o bloco */
                int32_t Mcat = escopo_marca(u);
                memset(&c->grava, 0, sizeof(c->grava));
                c->grava.tem_valor = 1;
                c->grava.tipo = "str";          /* é a mensagem do erro */
                c->grava.no = cl;
                guarda_nome_modo(c, u, cl->texto ? cl->texto : "e", 1);
                bloco_stmts(c, u, cl->b);
                escopo_fecha(c, u, Mcat);      /* var do catch (e) + corpo não vazam */
                saltos_push(c, &fins, emite(c, u, OP_JUMP, 0));
            }

            /* Nenhum catch casou: o erro CONTINUA. Engolir aqui fazia
             * `catch (KeyError e)` virar um catch-tudo silencioso — o `try`
             * de fora nunca via a divisão por zero.
             *
             * `n->lista.n == 0` é o `try { } finally { }` SEM catch, que o
             * parser passou a aceitar. Sem esta condição o handler não emitia
             * RERAISE nenhum e a exceção sumia CALADA: o finally rodava, o
             * programa saía com rc=0 e o erro nunca aparecia. Trocar um
             * `SyntaxError` por um erro engolido teria sido piorar. */
            /* Daqui em diante só se emitem cópias do PRÓPRIO finally: ele sai
             * de pendente ANTES delas. A primeira cópia (abaixo) vinha antes
             * desta linha, e um `return` dentro do finally a recompilava sem
             * fim. */
            if (fin_meu >= 0) c->nfinally = fin_meu;   /* sai de pendente */

            if (prox_falha >= 0 || n->lista.n == 0) {
                if (prox_falha >= 0) UP(c, u)->code[prox_falha + 1] = UP(c, u)->ncode;
                /* tipo junto da mensagem: o `finally` roda antes do RERAISE e
                 * pode ter trocado o erro corrente da VM */
                if (setup_cat >= 0) emite(c, u, OP_POP_TRY, 0);   /* não caia no próprio handler */
                emite(c, u, OP_PUSH_ERR_TYPE, 0);
                /* a cópia roda com [mensagem, tipo] em cima (ver `pilha_extra`) */
                c->pilha_extra += 2;
                if (n->c) { int32_t Mf = escopo_marca(u); bloco_stmts(c, u, n->c); escopo_fecha(c, u, Mf); }
                c->pilha_extra -= 2;
                emite(c, u, OP_RERAISE, 0);
            }

            int32_t pula_handler = -1;
            if (setup_cat >= 0) {
                /* fim normal de um catch: fecha o try interno e segue */
                int32_t pos_pop = UP(c, u)->ncode;
                emite(c, u, OP_POP_TRY, 0);
                pula_handler = emite(c, u, OP_JUMP, 0);
                saltos_corrige(c, u, &fins, pos_pop);
                fins.n = 0;
                /* handler: erro veio de dentro de um catch -> finally + repropaga */
                UP(c, u)->code[setup_cat + 1] = UP(c, u)->ncode;
                limpa_no_handler(c, u, Mt, hist0);     /* o erro saiu de um catch: o `e` e o corpo dele */
                emite(c, u, OP_PUSH_ERR_TYPE, 0);
                /* este handler foi armado com a mensagem do erro original já
                 * na pilha: a cópia roda com [mensagem, mensagem2, tipo] em cima */
                c->pilha_extra += 3;
                { int32_t Mf = escopo_marca(u); bloco_stmts(c, u, n->c); escopo_fecha(c, u, Mf); }
                c->pilha_extra -= 3;
                emite(c, u, OP_RERAISE, 0);
            }

            int32_t fim_catches = UP(c, u)->ncode;
            if (pula_catches >= 0) UP(c, u)->code[pula_catches + 1] = fim_catches;
            if (pula_handler >= 0) UP(c, u)->code[pula_handler + 1] = fim_catches;
            saltos_corrige(c, u, &fins, fim_catches);
            saltos_solta(&fins);

            if (n->c) { int32_t Mf = escopo_marca(u); bloco_stmts(c, u, n->c); escopo_fecha(c, u, Mf); }  /* finally */
            /* o pico volta a valer pra região de fora (um try que envolve este) */
            if (pico_fora > u->pico_locais) u->pico_locais = pico_fora;
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
            if (c->nlacos == 0) { cerro_sx(c, n, "'break' fora de laco"); return; }
            Laco *l = &c->lacos[c->nlacos - 1];
            /* saindo do laço: apaga TUDO nascido nele até aqui (var do laço +
             * corpo + blocos aninhados abertos), pra nada vazar pra fora */
            emite_finallys(c, u, c->nlacos, 0, 0);   /* finally aberto DENTRO deste laço */
            escopo_emite_clears(c, u, l->escopo_marca);
            /* limpa o estado do iterador antes de sair do laço — e o que os
             * construtos abertos desde o laço guardam por cima dele (ver
             * `Laco.extra_base`) */
            for (int k = 0; k < l->slots_pilha + (c->pilha_extra - l->extra_base); k++)
                emite(c, u, OP_POP_TOP, 0);
            saltos_push(c, &l->saidas, emite(c, u, OP_JUMP, 0));
            return;
        }

        /* `pass` não emite nada: é o statement que existe só pra ocupar lugar.
         * Sem bytecode, sem efeito, sem custo. */
        case N_PASS_STMT:
            return;

        case N_CONTINUE_STMT: {
            if (c->nlacos == 0) { cerro_sx(c, n, "'continue' fora de laco"); return; }
            Laco *l = &c->lacos[c->nlacos - 1];
            /* próxima iteração começa limpa: apaga só o escopo do CORPO (o que é
             * do laço — var/ self/_count — é re-atribuído no topo ou persiste) */
            emite_finallys(c, u, c->nlacos, 0, 1);   /* finally aberto DENTRO deste laço */
            escopo_emite_clears(c, u, l->escopo_marca_body);
            /* o topo do laço espera só o estado dele: o que os construtos
             * abertos desde o laço guardam por cima sai antes do salto */
            for (int k = 0; k < c->pilha_extra - l->extra_base; k++) emite(c, u, OP_POP_TOP, 0);
            saltos_push(c, &l->continues, emite(c, u, OP_JUMP, 0));
            return;
        }

        default:
            cerro(c, "statement ainda nao compila na VM", n);
            return;
    }
}

/* ── funct─────────────────────────────────────────────────────────────── */
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
    p->slot_vararg = -1;
    p->slot_kwarg = -1;
    /* o único lugar onde proto nasce: o método, o `__init__` gerado, e a
     * lambda e a funct aninhada dentro deles ficam todos com a classe */
    p->classe = c->corpo ? c->corpo->ci : -1;
    size_t n = strlen(nome);
    p->nome = malloc(n + 1);
    if (!p->nome) { cerro(c, "sem memoria", NULL); return -1; }
    memcpy(p->nome, nome, n + 1);
    return c->out->nprotos++;
}

/* Guarda o valor que está NO TOPO da pilha dentro de um alvo.
 *
 * O alvo pode ser nome, `o.campo`, `d[k]` ou um grupo aninhado.
 * A ordem importa: aqui o valor JÁ está na pilha (o UNPACK
 * o pôs lá) e container/índice só são avaliados agora, depois do lado direito
 * inteiro e depois da checagem de quantidade.
 * Como `INDEX_SET`/`SET_MEMBER` querem o valor por ÚLTIMO, o giro (ROT3/SWAP)
 * acerta a pilha e a semântica de escrita continua sendo a MESMA de
 * `l[i] = v` e `o.x = v`: um só lugar decide lista×dict, private e erro. */
static void guarda_em_alvo(C *c, Unidade *u, PSNode *e)
{
    switch (e->kind) {
        case N_UNPACK_TARGET:
            compila_unpack_alvo(c, u, e);            /* `a, (b, c) = ...` */
            return;
        case N_NAME:
            guarda_nome(c, u, e->texto ? e->texto : "");
            return;
        case N_INDEX_ACCESS:                         /* `l[i], x = ...` */
            expr(c, u, e->a);                        /* container */
            expr(c, u, e->b);                        /* índice    */
            emite(c, u, OP_ROT3, 0);                 /* [v,c,i] -> [c,i,v] */
            emite(c, u, OP_INDEX_SET, 0);
            return;
        case N_MEMBER_ACCESS: {                      /* `o.x, y = ...` */
            if (e->a && e->a->kind == N_BASE_CALL_NODE) {
                cerro_sx(c, e, "atribua pelo self: self.%s = ... (base(Pai).%s le o campo, nao grava)",
                         e->texto ? e->texto : "x", e->texto ? e->texto : "x");
                return;
            }
            int32_t mi = idx_const(c, u, K_STR, 0, 0, e->texto ? e->texto : "",
                                   e->texto ? (int32_t)strlen(e->texto) : 0);
            expr(c, u, e->a);                        /* objeto */
            emite(c, u, OP_SWAP, 0);                 /* [v,o] -> [o,v] */
            emite(c, u, OP_SET_MEMBER, mi);
            return;
        }
        default:
            cerro_sx(c, e, "alvo de desempacotamento precisa ser nome, membro ou indice");
            return;
    }
}

/* Emite UNPACK + um STORE por alvo. Presume a sequência no topo da pilha. */
static void compila_unpack_alvo(C *c, Unidade *u, PSNode *alvo)
{
    int32_t n_alvos = alvo->lista.n;
    int32_t star = alvo->i2;                     /* -1 = sem `*` */
    if (n_alvos > 250) { cerro(c, "limite do compilador: alvos demais no desempacotamento", alvo); return; }
    emite(c, u, OP_UNPACK, n_alvos | ((star + 1) << 8));
    for (int32_t i = 0; i < n_alvos && !CFALHOU(c); i++)
        guarda_em_alvo(c, u, alvo->lista.itens[i]);
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

    /* o `__init__` gerado recebe self + um parâmetro por campo: o limite de
     * parâmetros vira limite de campos, e quem declarou os campos é que
     * precisa ler isso — não quem instancia */
    if (campos->n + 1 > PS_MAX_PARAMS) {
        cerro_sx(c, entidade, "Entity %s: o __init__ gerado dos campos tem parametros demais "
                              "(maximo %d: self + %d campos)",
                 entidade->texto ? entidade->texto : "?", PS_MAX_PARAMS, PS_MAX_PARAMS - 1);
        return -1;
    }
    /* slot 0 é o `self`; os campos vêm depois, na ordem de declaração */
    { int32_t si = idx_local(c, &u, "self"); if (si >= 0) u.certo[si] = 1; }
    int32_t ndef = 0;
    for (int32_t i = 0; i < campos->n; i++) {
        const char *nome = campos->itens[i]->texto ? campos->itens[i]->texto : "";
        int32_t pi = idx_local(c, &u, nome);
        if (pi >= 0) u.certo[pi] = 1;
        if (campos->itens[i]->a) ndef++;
        else if (ndef > 0) {
            /* Mesma regra da funct: com `a, b=2, c` o `ndefaults` (que conta
             * só o SUFIXO) mentia, e a chamada `P(1)` acusava o parâmetro
             * errado ('b', que tem padrão) em vez do 'c' que faltou. */
            cerro_sx(c, campos->itens[i], "campo '%s' sem valor padrao vem depois de um com padrao", nome);
            break;
        }
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

    /* corpo: self.<campo> = <parametro>, um por campo. O campo com valor
     * padrão já nasceu com ele antes do `__init__` (o proto de campos, ver
     * `sintetiza_campos`): aqui ele só é gravado se o argumento veio — o
     * padrão não é avaliado de novo (`int id = proximo()` rodaria duas
     * vezes). Cada escrita leva a posição do campo: o erro do valor de tipo
     * errado aponta o campo, não a linha da classe. */
    int32_t l0 = c->linha_atual, c0 = c->coluna_atual;
    for (int32_t i = 0; i < campos->n && !CFALHOU(c); i++) {
        PSNode *campo = campos->itens[i];
        const char *nome = campo->texto ? campo->texto : "";
        if (campo->line) { c->linha_atual = campo->line; c->coluna_atual = campo->col; }
        int32_t veio = -1, pula = -1;
        if (campo->a) {
            emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_INT, i + 1, 0, NULL, 0));
            veio = emite(c, &u, OP_JUMP_IF_SET, 0);
            pula = emite(c, &u, OP_JUMP, 0);
            if (veio >= 0) UP(c, (&u))->code[veio + 1] = UP(c, (&u))->ncode;
        }
        emite(c, &u, OP_LOAD_LOCAL, 0);
        emite(c, &u, OP_LOAD_LOCAL, i + 1);
        emite(c, &u, OP_SET_MEMBER, idx_const(c, &u, K_STR, 0, 0, nome, (int32_t)strlen(nome)));
        if (pula >= 0) UP(c, (&u))->code[pula + 1] = UP(c, (&u))->ncode;
    }
    c->linha_atual = l0; c->coluna_atual = c0;
    emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_NULL, 0, 0, NULL, 0));
    emite(c, &u, OP_RETURN, 0);
    proto_encolhe(c, &u);

    vardbg_fecha(c, &u, 0);   /* o que chegou vivo ao fim vale até a última palavra */
    for (int32_t i = 0; i < u.nlocais; i++) free(u.locais[i]);
    free(u.locais);
    free(u.celula); free(u.celula_virgem); free(u.certo); free(u.tipo_decl);
    free(u.sim); free(u.mod_sim);
    for (int32_t i = 0; i < u.nligados; i++) free(u.ligados[i]);
    free(u.ligados);
    for (int32_t i = 0; i < u.n_mod_criados; i++) free(u.mod_criados[i]);
    free(u.mod_criados);
    free(u.mod_hist);
    ix_solta(&u.ix_mod);
    for (int32_t i = 0; i < u.nglobais_decl; i++) free(u.globais_decl[i]);
    free(u.globais_decl);
    return idx;
}

/* As capturas de uma unidade de funct viram a tabela do proto (é o que o
 * MAKE_CLOSURE lê pra montar as células), e a unidade é solta. */
static void fecha_unidade_funct(C *c, Unidade *u, int32_t idx, PSNode *n)
{
    if (u->nupvals > 0) {
        PSUpval *uv = calloc((size_t)u->nupvals, sizeof(PSUpval));
        if (!uv) cerro(c, "sem memoria", n);
        else {
            char **nms = calloc((size_t)u->nupvals, sizeof(char *));
            for (int32_t i = 0; i < u->nupvals; i++) {
                uv[i].em_local = u->upvals[i].em_local;
                uv[i].idx      = u->upvals[i].idx;
                if (nms) nms[i] = strdup(u->upvals[i].nome);
            }
            c->out->protos[idx].upvals  = uv;
            c->out->protos[idx].nupvals = u->nupvals;
            c->out->protos[idx].upval_nomes = nms;
        }
    }
    for (int32_t i = 0; i < u->nupvals; i++) free(u->upvals[i].nome);
    free(u->upvals);
    proto_encolhe(c, u);

    vardbg_fecha(c, u, 0);
    for (int32_t i = 0; i < u->nlocais; i++) free(u->locais[i]);
    free(u->locais);
    free(u->celula); free(u->celula_virgem); free(u->certo); free(u->tipo_decl);
    free(u->sim); free(u->mod_sim);
    for (int32_t i = 0; i < u->nligados; i++) free(u->ligados[i]);
    free(u->ligados);
    for (int32_t i = 0; i < u->n_mod_criados; i++) free(u->mod_criados[i]);
    free(u->mod_criados);
    free(u->mod_hist);
    ix_solta(&u->ix_mod);
    for (int32_t i = 0; i < u->nglobais_decl; i++) free(u->globais_decl[i]);
    free(u->globais_decl);
}

/* O proto dos campos com valor padrão da classe: `self.campo = padrão`, na
 * ordem de declaração. A VM o roda em TODA instância, antes do `__init__`
 * (gerado ou escrito), pela cadeia de classes, pais antes: o campo com valor
 * existe sempre, e o `__init__` escrito não apaga mais o padrão (o padrão só
 * entrava no `__init__` gerado da própria classe). `pai`: a unidade onde a
 * classe foi declarada — na funct, o proto é closure dela, como o método.
 * -1 = a classe não tem campo de instância com valor. */
static int32_t sintetiza_campos(C *c, PSNode *entidade, Unidade *pai)
{
    PSNodeVec *campos = &entidade->lista2_alias;
    int tem = 0;
    for (int32_t i = 0; i < campos->n && !tem; i++)
        tem = campos->itens[i] && !campos->itens[i]->is_static && campos->itens[i]->a;
    if (!tem) return -1;
    int32_t idx = novo_proto(c, "<campos>");
    if (idx < 0) return -1;

    Unidade u;
    memset(&u, 0, sizeof(u));
    u.pai = (pai && !pai->eh_modulo) ? pai : NULL;
    u.modulo = (pai && pai->eh_modulo) ? pai : NULL;   /* a célula de bloco do topo, como no método */
    u.idx = idx;
    u.eh_modulo = 0;
    u.nome_funct = "<campos>";
    { int32_t si = idx_local(c, &u, "self"); if (si >= 0) u.certo[si] = 1; }
    c->out->protos[idx].nparams = 1;
    {
        char **nomes = calloc(1, sizeof(char *));
        if (!nomes) { cerro(c, "sem memoria", entidade); return -1; }
        nomes[0] = strdup("self");
        c->out->protos[idx].param_nomes = nomes;
    }
    int32_t l0 = c->linha_atual, c0 = c->coluna_atual;
    for (int32_t i = 0; i < campos->n && !CFALHOU(c); i++) {
        PSNode *campo = campos->itens[i];
        if (!campo || campo->is_static || !campo->a || !campo->texto) continue;
        if (campo->line) { c->linha_atual = campo->line; c->coluna_atual = campo->col; }
        emite(c, &u, OP_LOAD_LOCAL, 0);
        expr(c, &u, campo->a);
        if (campo->line) { c->linha_atual = campo->line; c->coluna_atual = campo->col; }
        emite(c, &u, OP_SET_MEMBER, idx_const(c, &u, K_STR, 0, 0, campo->texto, (int32_t)strlen(campo->texto)));
    }
    c->linha_atual = l0; c->coluna_atual = c0;
    emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_NULL, 0, 0, NULL, 0));
    emite(c, &u, OP_RETURN, 0);
    fecha_unidade_funct(c, &u, idx, entidade);
    return idx;
}

static int32_t compila_funct_em(C *c, PSNode *n, Unidade *pai);

/* O corpo da funct roda quando ela é CHAMADA, fora do `try` em volta da
 * declaração: o import opcional daquele `try` não vale aqui dentro. */
static int32_t compila_funct(C *c, PSNode *n, Unidade *pai)
{
    int protegido = c->import_protegido;
    c->import_protegido = 0;
    int32_t r = compila_funct_em(c, n, pai);
    c->import_protegido = protegido;
    return r;
}

static int32_t compila_funct_em(C *c, PSNode *n, Unidade *pai)
{
    int32_t idx = novo_proto(c, n->texto ? n->texto : "<funct>");
    if (idx < 0) return -1;

    /* Modificadores COLADOS na cabeça (`static funct m()`, `nonnull funct f()`)
     * valem exatamente o que os decoradores antigos `@static` e `@NonNull`, que
     * chegam aqui pelas pendências. Um OU: escrever os dois não é erro.
     *
     * E as pendências são CONSUMIDAS aqui: valem para ESTA funct, não para as
     * declaradas dentro do corpo dela. `static` vazava — uma funct aninhada
     * herdava a marca, e se o primeiro parâmetro dela se chamasse `self` ele
     * era descartado: `inner() takes 0 positional arguments but 1 was given`,
     * sem nenhuma relação visível com o `static` de fora. */
    int meu_static  = c->pendente_static  || n->is_static;
    int meu_nonnull = c->pendente_nonnull || n->is_nonnull;
    int eh_metodo   = c->pendente_metodo;
    c->pendente_static  = 0;
    c->pendente_nonnull = 0;
    c->pendente_metodo  = 0;

    /* `static funct f(self)` FORA de Entity: não existe instância nunca, o
     * `self` ali é engano — e a frase tem que dizer isso na DECLARAÇÃO, não
     * "missing 1 required positional argument: 'self'" em cada chamada
     * (ele, 2026-09-26: "static não recebe self"). Dentro de Entity o self
     * num método static é o da doc (14-decoradores): dropado na chamada
     * pela classe, ligado na chamada pela instância. */
    if (meu_static && !c->dentro_entity && n->lista.n > 0 && n->lista.itens[0]
            && n->lista.itens[0]->i2 == 0 && n->lista.itens[0]->texto
            && strcmp(n->lista.itens[0]->texto, "self") == 0) {
        terro(c, n->lista.itens[0], "TypeError",
              "funct static %s(self, ...) nao recebe self: static nao tem instancia, tire o self",
              n->texto ? n->texto : "?");
    }

    Unidade u;
    memset(&u, 0, sizeof(u));
    u.pai = (pai && !pai->eh_modulo) ? pai : NULL;
    u.modulo = (pai && pai->eh_modulo) ? pai : NULL;
    u.idx = idx;
    u.eh_modulo = 0;
    if (n->texto2 && !strcmp(n->texto2, "int"))       u.tipo_ret = 1;
    else if (n->texto2 && !strcmp(n->texto2, "bool")) u.tipo_ret = 2;
    const char *nome_f = n->texto ? n->texto : "<funct>";
    u.nome_funct = nome_f;
    /* `async funct` com `yield` virava gerador comum, calado: o corpo não
     * começava na chamada, e o `await` e o `gather` devolviam o próprio
     * gerador. Uma chamada é tarefa ou gerador, não as duas (decisão dele no
     * Plano 6): erro antes de rodar, no `yield`. */
    if (n->is_async) {
        PSNode *y = tp_acha_yield(n->b);
        if (y) cerro_sx(c, y, "'yield' em async funct: a funct e tarefa (async) ou gerador (yield), nao as duas");
    }
    if (n->kind == N_FUNCT_DECL && n->texto2) {
        u.tipo_ret_nome = tp_canon(n->texto2);
        if (!tp_tipo_existe(c, &u, n->texto2))
            terro(c, n, "AttributedValueError", "tipo %s não existe (retorno de %s())", n->texto2, nome_f);
        /* gerador devolve o gerador: o tipo escrito não é o do `return` */
        if (tp_tem_yield(n->b)) u.tipo_ret_nome = NULL;
    }
    /* método de Entity: o `self` é da classe — é o que confere `self.campo`
     * e `self.metodo()` antes de rodar. Pela marca, não pela falta de `pai`:
     * o método de classe declarada dentro de funct tem a funct como pai. */
    int self_da_classe = eh_metodo && c->dentro_entity && c->entity_no && c->entity_no->texto;
    /* os nomes que esta funct liga: parâmetros e o corpo inteiro */
    for (int32_t i = 0; i < n->lista.n; i++)
        junta_nomes(c, &u.ligados, &u.nligados, &u.cap_ligados, n->lista.itens[i]->texto);
    tp_junta_ligados(c, n->b, &u.ligados, &u.nligados, &u.cap_ligados);

    /* Os parâmetros ocupam os primeiros slots, na ordem escrita. O parser
     * garante a ordem comuns → `*args` → `**kwarg`, então os FIXOS são os
     * `nfix` primeiros da lista: é só deles que `nparams`, `param_nomes` e
     * `param_tipos` falam. A estrela ganha o slot dela e nada mais — a tup e
     * o dict nascem no binding da chamada, não têm default nem tipo. */
    int32_t ndef = 0, nfix = 0, slot_vararg = -1, slot_kwarg = -1;
    for (int32_t i = 0; i < n->lista.n; i++) {
        PSNode *par = n->lista.itens[i];
        int32_t pi = idx_local(c, &u, par->texto ? par->texto : "");
        if (pi < 0) break;
        u.certo[pi] = 1;
        u.sim[pi].estado = 1;          /* valor da chamada: desconhecido */
        if (par->i2 == 1) { slot_vararg = pi; u.sim[pi].tipo = "tup"; continue; }
        if (par->i2 == 2) { slot_kwarg = pi; u.sim[pi].tipo = "dict"; continue; }
        if (i == 0 && self_da_classe && par->texto && !strcmp(par->texto, "self") && !par->texto2)
            u.sim[pi].tipo = c->entity_no->texto;
        /* O tipo do parâmetro vale pra TODA escrita nele, não só pro binding
         * da chamada: `funct f(int n) { n = "x" }` passava calado. */
        {
            int cod = cod_tipo_decl(par->texto2);
            if (cod >= 0) u.tipo_decl[pi] = (unsigned char)(cod + 1);
        }
        if (par->texto2) {
            u.sim[pi].estado = 2;
            u.sim[pi].tipo = tp_canon(par->texto2);
            if (!tp_tipo_existe(c, &u, par->texto2))
                terro(c, par, "AttributedValueError", "tipo %s não existe (parâmetro %s de %s())",
                      par->texto2, par->texto ? par->texto : "?", nome_f);
        }
        nfix++;
        if (par->a) ndef++;
        else if (ndef > 0) {
            cerro_sx(c, n, "parametro sem valor padrao depois de um com padrao");
            break;
        }
    }
    /* Passava no `--check` e quebrava em TODA chamada, inclusive a
     * posicional, com a linha apontando a chamada e não a declaração. */
    if (nfix > PS_MAX_PARAMS)
        cerro_sx(c, n, "%s() tem parametros demais (maximo %d)",
                 n->texto ? n->texto : "<funct>", PS_MAX_PARAMS);
    c->out->protos[idx].nparams = nfix;
    c->out->protos[idx].ndefaults = ndef;
    c->out->protos[idx].slot_vararg = slot_vararg;
    c->out->protos[idx].slot_kwarg = slot_kwarg;
    c->out->protos[idx].eh_async = n->is_async;
    c->out->protos[idx].eh_static = meu_static;
    c->out->protos[idx].eh_nonnull = meu_nonnull;
    if (nfix > 0) {
        char **nomes = calloc((size_t)nfix, sizeof(char *));
        if (!nomes) { cerro(c, "sem memoria", n); }
        else {
            for (int32_t i = 0; i < nfix; i++) {
                const char *pn = n->lista.itens[i]->texto ? n->lista.itens[i]->texto : "";
                size_t ln = strlen(pn);
                nomes[i] = malloc(ln + 1);
                if (nomes[i]) memcpy(nomes[i], pn, ln + 1);
            }
            c->out->protos[idx].param_nomes = nomes;
        }
        /* Tipo declarado (`funct f(str nome)`) — o parser deixou em texto2.
         * O vetor só nasce se ALGUM parâmetro tiver tipo: função sem tipagem
         * não paga nada, nem memória nem checagem. */
        int32_t com_tipo = 0;
        for (int32_t i = 0; i < nfix; i++)
            if (n->lista.itens[i]->texto2) com_tipo = 1;
        if (com_tipo) {
            char **tipos = calloc((size_t)nfix, sizeof(char *));
            if (!tipos) { cerro(c, "sem memoria", n); }
            else {
                for (int32_t i = 0; i < nfix; i++) {
                    const char *pt = n->lista.itens[i]->texto2;
                    if (!pt) continue;
                    size_t lt = strlen(pt);
                    tipos[i] = malloc(lt + 1);
                    if (tipos[i]) memcpy(tipos[i], pt, lt + 1);
                }
                c->out->protos[idx].param_tipos = tipos;
            }
        }
    }

    /* Prólogo: para cada parâmetro com default, avalia o default SÓ se o
     * argumento não veio na chamada. */
    for (int32_t i = 0; i < n->lista.n && !CFALHOU(c); i++) {
        PSNode *par = n->lista.itens[i];
        if (!par->a) continue;
        /* O padrão roda na entrada, na ordem dos parâmetros: o próprio e os
         * de depois ainda não têm valor (o padrão lia o slot vazio e virava
         * Null, calado). As estrelas valem: nascem antes, no binding. */
        for (int32_t j = i; j < n->lista.n; j++) {
            PSNode *pj = n->lista.itens[j];
            if (!pj || pj->i2 != 0 || !pj->texto) continue;
            PSNode *lido = acha_leitura(c, par->a, pj->texto);
            if (!lido) continue;
            const char *pn = par->texto ? par->texto : "?";
            if (j == i)
                terro(c, lido, "NameError", "o padrão de '%s' lê o próprio '%s', que ainda não tem valor: "
                      "no padrão só valem os parâmetros de antes", pn, pj->texto);
            else
                terro(c, lido, "NameError", "o padrão de '%s' lê '%s', um parâmetro que vem depois e ainda "
                      "não tem valor: no padrão só valem os parâmetros de antes", pn, pj->texto);
            break;
        }
        /* Lambda no padrão que CAPTURA parâmetro (`funct f(int b = 1, a =
         * funct(int x) { return x + b })`): a célula do parâmetro tem que
         * existir quando a lambda nasce, aqui no prólogo — as células da
         * funct só nasciam depois dele, e a lambda lia `b` como global
         * (NameError rodando, com o `--check` ok). O parâmetro de antes ganha
         * a célula agora (o slot dele já tem valor); o próprio e os de depois
         * seguem a regra de cima. Na `nonnull funct`, os de antes são
         * conferidos antes de virar célula (o CHECK_NONNULL lê o slot cru). */
        {
            PSNode **lams = NULL; int32_t nl = 0, cl = 0;
            colhe_lambdas(c, par->a, &lams, &nl, &cl);
            int conferiu = 0;
            for (int32_t q = 0; q < nl && !CFALHOU(c); q++) {
                char **cit = NULL; int32_t nc = 0, cc = 0;
                varre_nomes(c, lams[q], &cit, &nc, &cc);
                for (int32_t j = 0; j < n->lista.n; j++) {
                    PSNode *pj = n->lista.itens[j];
                    if (!pj || pj->i2 != 0 || !pj->texto || !tp_na_lista(cit, nc, pj->texto)) continue;
                    if (liga_o_nome(c, lams[q], pj->texto)) continue;   /* o nome é dela */
                    if (j >= i) {
                        terro(c, lams[q], "NameError", "a lambda no padrão de '%s' captura '%s', que ainda não "
                              "tem valor: no padrão só valem os parâmetros de antes",
                              par->texto ? par->texto : "?", pj->texto);
                        continue;
                    }
                    if (u.celula[j]) continue;
                    if (meu_nonnull && !conferiu) { emite(c, &u, OP_CHECK_NONNULL, i); conferiu = 1; }
                    u.celula[j] = 1;
                    u.celula_virgem[j] = 0;          /* o parâmetro de antes já tem valor */
                    emite(c, &u, OP_MAKE_CELL, j);
                }
                for (int32_t t = 0; t < nc; t++) free(cit[t]);
                free(cit);
            }
            free(lams);
        }
        emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_INT, i, 0, NULL, 0));
        int32_t pula = emite(c, &u, OP_JUMP_IF_SET, 0);
        expr(c, &u, par->a);
        /* o padrão também respeita o tipo: `funct f(str s = 10)` devolvia 10 */
        if (par->texto2) {
            const char *vt = tp_de(c, &u, par->a);
            const char *T = tp_canon(par->texto2);
            int v = tp_aceita(c, T, vt, par->a, 1);
            if (v == T_NAO)
                terro(c, par->a, "AttributedValueError", "parâmetro %s de %s() esperava %s, recebeu %s",
                      par->texto ? par->texto : "?", nome_f, T, vt);
            else if (v == T_TALVEZ) {
                if (u.tipo_decl[i]) emite_coerce_se_tipado(c, &u, par->texto ? par->texto : "", u.tipo_decl[i]);
                else {
                    char rot[300];
                    snprintf(rot, sizeof(rot), "parâmetro %s de %s()", par->texto ? par->texto : "?", nome_f);
                    tp_emite_confere(c, &u, rot, T, 1);
                }
            }
        }
        emite(c, &u, OP_STORE_LOCAL, i);
        if (pula >= 0) UP(c, (&u))->code[pula + 1] = UP(c, (&u))->ncode;
    }

    /* Depois do prólogo: um default que avalie pra Null também é violação.
     * Só os fixos: a tup e o dict das estrelas nunca são Null. ANTES das
     * células: o CHECK_NONNULL lê o slot cru, e o parâmetro capturado por
     * uma funct de dentro virava célula antes — a célula não é Null, e
     * `nonnull funct f(v)` com `v` capturado deixava `f(Null)` passar. */
    if (meu_nonnull) emite(c, &u, OP_CHECK_NONNULL, nfix);

    /* Células das variáveis que as functs aninhadas capturam. Vem depois do
     * prólogo (que testa o slot cru com JUMP_IF_SET) e antes do corpo. */
    marca_celulas(c, &u, n, n->b);

    /* `int funct` e `bool funct` não deixam erro escapar: devolvem 500 e
     * False. Sai mais barato emitir o `try` implícito aqui do que ensinar o
     * desenrolamento de erro da VM a olhar o tipo do frame. */
    int32_t rede = -1;
    if (u.tipo_ret) rede = emite(c, &u, OP_SETUP_TRY, 0);

    stmts_de(c, &u, n->b);
    /* Chegar ao fim devolve Null, e Null não é valor de nenhum tipo escrito
     * — a não ser `int`/`bool`, cujo fim é o sentinela 0/True, e a própria
     * `Null funct`, que não devolve valor. Gerador devolve o gerador. */
    if (u.tipo_ret_nome && !u.tipo_ret && strcmp(u.tipo_ret_nome, "Null") != 0
            && !tp_tem_yield(n->b) && !tp_termina(n->b))
        terro(c, n, "AttributedValueError",
              "%s() devolve %s e pode chegar ao fim sem return (o fim devolveria Null)",
              nome_f, u.tipo_ret_nome);
    if (u.tipo_ret) emite(c, &u, OP_POP_TRY, 0);
    /* funct sem return explícito devolve Null */
    emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_NULL, 0, 0, NULL, 0));
    if (u.tipo_ret) emite(c, &u, OP_COERCE_RET, u.tipo_ret);
    emite(c, &u, OP_RETURN, 0);

    if (rede >= 0) {
        UP(c, (&u))->code[rede + 1] = UP(c, (&u))->ncode;
        /* o erro sai no stderr JUNTO com o sentinela — descartá-lo calado
         * mostrava "500" como se fosse resultado */
        emite(c, &u, OP_ENGOLE, u.tipo_ret == 1 ? 1 : 2);
        if (u.tipo_ret == 1)
            emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_INT, 500, 0, NULL, 0));
        else
            emite(c, &u, OP_LOAD_CONST, idx_const(c, &u, K_BOOL, 0, 0, NULL, 0));
        emite(c, &u, OP_RETURN, 0);
    }

    /* Os upvalues descobertos durante o corpo viram tabela no proto: é o que
     * o MAKE_CLOSURE lê pra montar as células. */
    fecha_unidade_funct(c, &u, idx, n);
    return idx;
}

/* ── tipagem estática: a passada antes de compilar ──────────────────────────
 *
 * Os nomes que o ARQUIVO liga fora de bloco, com o tipo, na ordem do fonte:
 * uma funct compilada antes de `total = 0` já precisa saber que o `total`
 * de fora é int (a escrita dela cai nele), e uma chamada `f()` no topo
 * precisa da assinatura de `f`. O que nasce dentro de bloco morre com ele, e
 * não entra. A conferência é a mesma da compilação (`tp_escreve`), com o
 * mesmo nó de posição: o erro que ela ache sai uma vez só na lista. */
static SimInfo *tp_topo_poe(C *c, const char *nome)
{
    if (!nome) return NULL;            /* `tp_guarda` sem memória: o erro já saiu */
    SimInfo *s = tp_sim_topo(c, nome);
    if (s) return s;
    if (c->ntopo + 1 > c->cap_topo) {
        int32_t novo = c->cap_topo < 16 ? 16 : c->cap_topo * 2;
        void *nv = realloc(c->topo, sizeof(c->topo[0]) * (size_t)novo);
        if (!nv) { cerro(c, "sem memoria", NULL); return NULL; }
        c->topo = nv;
        c->cap_topo = novo;
    }
    if (ix_poe(&c->ix_topo, nome, c->ntopo) != 0) { cerro(c, "sem memoria", NULL); return NULL; }
    c->topo[c->ntopo].nome = nome;
    memset(&c->topo[c->ntopo].s, 0, sizeof(SimInfo));
    return &c->topo[c->ntopo++].s;
}

static int tp_decorador_embutido(PSNode *dec)
{
    const char *dn = (dec && dec->lista.n == 1) ? dec->lista.itens[0]->texto : NULL;
    return dn && (!strcmp(dn, "static") || !strcmp(dn, "NonNull") || !strcmp(dn, "dataentity"));
}

static void tp_pre_stmt(C *c, Unidade *mod, PSNode *s, int decorado)
{
    if (!s || CFALHOU(c)) return;
    switch (s->kind) {
        case N_VAR_DECL:
            if (!s->texto) return;
            grava_valor(c, mod, s->a);
            /* o tipo escrito que não existe é o erro da declaração, na
             * segunda passada ("tipo X não existe"): aqui ele não vira o tipo
             * do nome, senão o valor seria conferido contra ele e o mesmo
             * erro sairia de novo, com outra frase */
            c->grava.declara = tp_tipo_existe(c, mod, s->texto2) ? s->texto2 : NULL;
            tp_escreve(c, tp_topo_poe(c, s->texto), s->texto);
            break;
        case N_ASSIGNMENT:
            if (!s->texto || (s->texto2 && strcmp(s->texto2, "=") != 0)) return;
            grava_valor(c, mod, s->a);
            tp_escreve(c, tp_topo_poe(c, s->texto), s->texto);
            break;
        case N_FUNCT_DECL:
            if (!s->texto) return;
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.tem_valor = 1; c->grava.tipo = "funct"; c->grava.no = s;
            c->grava.decl = s; c->grava.decorado = (unsigned char)decorado;
            c->grava.declaracao = 1;
            tp_escreve(c, tp_topo_poe(c, s->texto), s->texto);
            break;
        case N_ENTITY_DECL:
            if (!s->texto) return;
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.tem_valor = 1; c->grava.tipo = "Entity"; c->grava.no = s; c->grava.decl = s;
            c->grava.declaracao = 1;
            tp_escreve(c, tp_topo_poe(c, s->texto), s->texto);
            break;
        case N_MODEL_DECL:
        case N_ENUM_DECL:
            if (!s->texto) return;
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.decl = s; c->grava.no = s;
            c->grava.declaracao = 1;
            tp_escreve(c, tp_topo_poe(c, s->texto), s->texto);
            break;
        case N_IMPORT_STMT: {
            if (!s->texto || s->texto3 || (s->i2 > 0 && s->lista.n == 0)) return;
            char enc[512], base[256];
            import_modulo_codificado(s, enc);
            import_nome_do_arquivo(s, base, sizeof(base));
            int eh_from = strcmp(s->texto, "from") == 0 || (strcmp(s->texto, "push") == 0 && s->lista2.n > 0);
            if (!eh_from) {
                const char *nome = s->texto2 ? s->texto2 : (s->i2 == -1 || base[0] ? base : enc);
                tp_grava_import(c, s, enc, NULL);
                tp_escreve(c, tp_topo_poe(c, tp_guarda(c, nome)), nome);
            } else {
                for (int32_t i = 0; i < s->lista2.n; i++) {
                    const char *membro = s->lista2.itens[i]->texto;
                    const char *apelido = (i < s->lista2_alias.n && s->lista2_alias.itens[i])
                                        ? s->lista2_alias.itens[i]->texto : membro;
                    if (!membro || !apelido) continue;
                    tp_grava_import(c, s, enc, membro);
                    tp_escreve(c, tp_topo_poe(c, apelido), apelido);
                }
            }
            break;
        }
        case N_DECORATOR_STMT: {
            int deco = decorado || (s->a && !tp_decorador_embutido(s->a));
            if (s->b && s->b->kind == N_BLOCK)
                for (int32_t i = 0; i < s->b->lista.n; i++) tp_pre_stmt(c, mod, s->b->lista.itens[i], deco);
            break;
        }
        default:
            break;
    }
    memset(&c->grava, 0, sizeof(c->grava));
}

static int tp_erro_cmp(const void *a, const void *b)
{
    const PSErroTipo *x = a, *y = b;
    if (x->linha != y->linha) return x->linha < y->linha ? -1 : 1;
    if (x->col != y->col) return x->col < y->col ? -1 : 1;
    return 0;
}

/* ── entrada ────────────────────────────────────────────────────────────── */
static int eh_identificador(const char *s)
{
    if (!s || !*s) return 0;
    for (const char *q = s; *q; q++) {
        int letra = (*q >= 'a' && *q <= 'z') || (*q >= 'A' && *q <= 'Z') || *q == '_'
                 || ((unsigned char)*q >= 0x80);                 /* UTF-8 */
        int digito = (*q >= '0' && *q <= '9');
        if (!(letra || (digito && q != s))) return 0;
    }
    return 1;
}

/* `PSPrograma.exportados`: o que sobrou no escopo do ARQUIVO depois de
 * compilado (`mod_criados` — o fim de cada bloco já tirou dali o que nasceu
 * nele) mais o que funct grava com `global x`. Fica de fora `private` (funct
 * de módulo e class) e nome interno (`$reg0`, `  fe$x`), que não é
 * identificador. */
static void calcula_exportados(C *c, Unidade *u)
{
    PSPrograma *out = c->out;
    int32_t cap = u->n_mod_criados + c->nglobais_gravados;
    if (cap == 0) return;
    out->exportados = calloc((size_t)cap, sizeof(char *));
    if (!out->exportados) { cerro(c, "sem memoria", NULL); return; }
    /* O que NÃO sai do módulo (os `private`) e o que já saiu, indexados: cada
     * nome era conferido contra todos os anteriores — N² num arquivo com
     * milhares de funct, e isto roda também ao executar. */
    IdxNomes fora_ix = {0}, ja_ix = {0};
    for (int32_t k = 0; k < out->npriv_globais; k++)
        if (ix_poe(&fora_ix, out->priv_globais[k], k) != 0) goto sem_memoria;
    for (int32_t k = 0; k < out->nclasses; k++)
        if (out->classes[k].classe_privada && out->classes[k].nome
                && ix_poe(&fora_ix, out->classes[k].nome, k) != 0) goto sem_memoria;
    for (int passo = 0; passo < 2; passo++) {
        char  **fonte = passo == 0 ? u->mod_criados : c->globais_gravados;
        int32_t nf    = passo == 0 ? u->n_mod_criados : c->nglobais_gravados;
        for (int32_t i = 0; i < nf; i++) {
            const char *nm = fonte[i];
            if (!eh_identificador(nm)) continue;
            if (ix_busca(&fora_ix, nm) >= 0 || ix_busca(&ja_ix, nm) >= 0) continue;
            char *copia = strdup(nm);
            if (!copia) goto sem_memoria;
            if (ix_poe(&ja_ix, copia, out->nexportados) != 0) { free(copia); goto sem_memoria; }
            out->exportados[out->nexportados++] = copia;
        }
    }
    ix_solta(&fora_ix); ix_solta(&ja_ix);
    return;
sem_memoria:
    ix_solta(&fora_ix); ix_solta(&ja_ix);
    cerro(c, "sem memoria", NULL);
}

/* Um `*` do topo do arquivo, resolvido ANTES de compilar o primeiro
 * statement (ver o campo `estrelas` do C). */
static void resolve_estrela(C *c, PSNode *s)
{
    if (!s || s->kind != N_IMPORT_STMT || !s->texto3) return;
    if (c->nestrelas + 1 > c->cap_estrelas) {
        int32_t nc = c->cap_estrelas < 4 ? 4 : c->cap_estrelas * 2;
        void *nv = realloc(c->estrelas, sizeof(*c->estrelas) * (size_t)nc);
        if (!nv) { cerro(c, "sem memoria", NULL); return; }
        c->estrelas = nv; c->cap_estrelas = nc;
    }
    int32_t e = c->nestrelas++;
    memset(&c->estrelas[e], 0, sizeof(c->estrelas[e]));
    c->estrelas[e].no = s;
    if (!c->resolve || !c->resolve->nomes_de) return;
    if (s->i2 > 0 && s->lista.n == 0) return;    /* o compilador recusa */
    char enc[512];
    import_modulo_codificado(s, enc);
    int r = c->resolve->nomes_de(c->resolve->ctx, enc, &c->estrelas[e].nomes, &c->estrelas[e].n);
    if (r == 3) {
        /* `from m import *` de módulo que não existe: mesma frase do
         * import comum, na linha do import */
        char vis[256];
        tp_mod_visivel(enc, vis, sizeof(vis));
        terro(c, tp_no_do_import(s, NULL), "ImportError", "No module named '%s'", vis);
    }
    c->estrelas[e].resolvido = (r == 1 || r == 2);
    c->estrelas[e].incompleto = r == 2;
}

/* ── a PODA: o que fica de uma declaração depois da primeira passada ──────
 *
 * Copia pra arena `keep` só o que outra declaração pode consultar. De uma
 * funct fica o cabeçalho (nome, parâmetros com padrão, tipo de retorno,
 * marcas); do corpo ficam só os FATOS que os leitores de corpo do checador
 * procuram — e a poda os deixa na forma que cada leitor já sabe ler, então
 * nenhum leitor muda:
 *   - `yield` direto no corpo (tp_tem_yield): um N_YIELD_STMT;
 *   - `obj.x = ...` e `self.a, self.b = ...` (tp_self_grava,
 *     tp_junta_self_gravados): o N_MEMBER_ASSIGNMENT / N_UNPACK_TARGET só com
 *     o nome;
 *   - `private T x = ...` no corpo (tp_campo_decl_em): o N_FIELD_DECL;
 *   - `global x` (tp_junta_globais);
 *   - Entity/model/enum declarados lá dentro (tp_coleta_tipos_arq), podados
 *     por sua vez;
 *   - funct e lambda aninhadas: o cabeçalho, com o corpo podado do mesmo
 *     jeito — os leitores que param nelas continuam parando, os que descem
 *     continuam descendo.
 * Entity, model e enum ficam inteiros, com os métodos podados como functs. O
 * resto (import, atribuição, `if`, o bloco do `main`...) é copiado inteiro,
 * com as functs de dentro podadas. */
static PSNode *poda(C *c, PSNode *n);
static PSNode *poda_funct(C *c, PSNode *n);

static PSNode *copia_raso(C *c, const PSNode *n)
{
    PSNode *k = ps_node_novo(c->keep, n->kind, n->line, n->col);
    if (!k) { cerro(c, "sem memoria", NULL); return NULL; }
    k->linha_fim = n->linha_fim; k->col_fim = n->col_fim;
    k->lit = n->lit; k->i = n->i; k->d = n->d; k->texto_len = n->texto_len; k->i2 = n->i2;
    k->estilo = n->estilo;
    k->is_async = n->is_async; k->is_private = n->is_private;
    k->is_static = n->is_static; k->is_nonnull = n->is_nonnull;
    if (n->texto) {
        int len = n->texto_len > 0 ? n->texto_len : (int)strlen(n->texto);
        k->texto = ps_arena_strdup(c->keep, n->texto, len);
    }
    if (n->texto2) k->texto2 = ps_arena_strdup(c->keep, n->texto2, (int)strlen(n->texto2));
    if (n->texto3) k->texto3 = ps_arena_strdup(c->keep, n->texto3, (int)strlen(n->texto3));
    if ((n->texto && !k->texto) || (n->texto2 && !k->texto2) || (n->texto3 && !k->texto3)) {
        cerro(c, "sem memoria", NULL); return NULL;
    }
    return k;
}

static void poda_vec(C *c, const PSNodeVec *de, PSNodeVec *para)
{
    if (de->n == 0) return;
    para->itens = ps_arena_alloc(c->keep, sizeof(PSNode *) * (size_t)de->n);
    if (!para->itens) { cerro(c, "sem memoria", NULL); return; }
    para->n = para->cap = de->n;
    for (int32_t i = 0; i < de->n; i++) para->itens[i] = poda(c, de->itens[i]);
}

static int poda_poe(C *c, PSNode *bloco, PSNode *k)
{
    if (!k) return -1;
    if (ps_vec_push(c->keep, &bloco->lista, k) != 0) { cerro(c, "sem memoria", NULL); return -1; }
    return 0;
}

/* Os fatos de um corpo, na lista de `bloco`. Não entra em funct/lambda
 * aninhada: elas viram nós podados próprios. */
static void colhe_fatos(C *c, PSNode *n, PSNode *bloco, int *viu_yield)
{
    if (!n || CFALHOU(c) || fundo_demais(c, n, 1)) return;
    while (n->kind == N_BINARY_OP) {      /* cadeia `a + b + …` em laço (ver liga_o_nome) */
        colhe_fatos(c, n->b, bloco, viu_yield);
        n = n->a;
        if (!n) return;
    }
    switch (n->kind) {
        case N_FUNCT_DECL: case N_LAMBDA_EXPR:
            poda_poe(c, bloco, poda_funct(c, n));
            return;
        case N_ENTITY_DECL: case N_MODEL_DECL: case N_ENUM_DECL:
            poda_poe(c, bloco, poda(c, n));
            return;
        case N_YIELD_STMT:
            if (!*viu_yield) { *viu_yield = 1; poda_poe(c, bloco, copia_raso(c, n)); }
            break;
        case N_MEMBER_ASSIGNMENT: {
            PSNode *k = copia_raso(c, n);
            if (k && n->a && n->a->kind == N_NAME) k->a = copia_raso(c, n->a);
            poda_poe(c, bloco, k);
            break;
        }
        case N_UNPACK_TARGET: {
            PSNode *k = copia_raso(c, n);
            if (!k) break;
            for (int32_t i = 0; i < n->lista.n; i++) {
                PSNode *x = n->lista.itens[i];
                if (x && x->kind == N_MEMBER_ACCESS) poda_poe(c, k, copia_raso(c, x));
            }
            poda_poe(c, bloco, k);
            break;
        }
        case N_FIELD_DECL:
            poda_poe(c, bloco, copia_raso(c, n));
            break;
        case N_GLOBAL_STMT: {
            PSNode *k = copia_raso(c, n);
            if (!k) break;
            for (int32_t i = 0; i < n->lista.n; i++)
                if (n->lista.itens[i]) poda_poe(c, k, copia_raso(c, n->lista.itens[i]));
            poda_poe(c, bloco, k);
            break;
        }
        default:
            break;
    }
    colhe_fatos(c, n->a, bloco, viu_yield);
    colhe_fatos(c, n->b, bloco, viu_yield);
    colhe_fatos(c, n->c, bloco, viu_yield);
    colhe_fatos(c, n->e, bloco, viu_yield);
    for (int32_t i = 0; i < n->lista.n; i++)        colhe_fatos(c, n->lista.itens[i], bloco, viu_yield);
    for (int32_t i = 0; i < n->lista2.n; i++)       colhe_fatos(c, n->lista2.itens[i], bloco, viu_yield);
    for (int32_t i = 0; i < n->lista2_alias.n; i++) colhe_fatos(c, n->lista2_alias.itens[i], bloco, viu_yield);
}

/* Corpo sem fato nenhum (a maioria) fica NULL: todo leitor de corpo aceita
 * NULL, e é um nó a menos por funct. Os fatos são colhidos num vetor da
 * pilha do C primeiro, porque a arena não devolve o que alocou. */
static PSNode *poda_corpo(C *c, PSNode *b)
{
    if (!b) return NULL;
    PSNode tmp;
    memset(&tmp, 0, sizeof(tmp));
    int viu_yield = 0;
    colhe_fatos(c, b, &tmp, &viu_yield);
    if (tmp.lista.n == 0) return NULL;
    PSNode *k = ps_node_novo(c->keep, N_BLOCK, b->line, b->col);
    if (!k) { cerro(c, "sem memoria", NULL); return NULL; }
    k->linha_fim = b->linha_fim; k->col_fim = b->col_fim; k->estilo = b->estilo;
    k->lista = tmp.lista;
    return k;
}

static PSNode *poda_funct(C *c, PSNode *n)
{
    PSNode *k = copia_raso(c, n);
    if (!k) return NULL;
    gemeo_poe(c, k);
    k->a = poda(c, n->a);
    k->b = poda_corpo(c, n->b);
    k->c = poda(c, n->c);
    k->e = poda(c, n->e);
    poda_vec(c, &n->lista, &k->lista);
    poda_vec(c, &n->lista2, &k->lista2);
    poda_vec(c, &n->lista2_alias, &k->lista2_alias);
    return k;
}

static PSNode *poda(C *c, PSNode *n)
{
    if (!n || CFALHOU(c) || fundo_demais(c, n, 1)) return NULL;
    if (n->kind == N_BINARY_OP) {
        /* cadeia `a + b + …` em laço (ver liga_o_nome): copia nó a nó pelo
         * ->a, e o ->a do último recebe a cópia do termo da ponta */
        PSNode *raiz = NULL, **ponta = &raiz;
        while (n && n->kind == N_BINARY_OP) {
            if (CFALHOU(c)) return NULL;
            PSNode *k = copia_raso(c, n);
            if (!k) return NULL;
            k->b = poda(c, n->b);
            k->c = poda(c, n->c);
            k->e = poda(c, n->e);
            poda_vec(c, &n->lista, &k->lista);
            poda_vec(c, &n->lista2, &k->lista2);
            poda_vec(c, &n->lista2_alias, &k->lista2_alias);
            *ponta = k;
            ponta = &k->a;
            n = n->a;
        }
        *ponta = poda(c, n);
        return raiz;
    }
    if (n->kind == N_FUNCT_DECL || n->kind == N_LAMBDA_EXPR) return poda_funct(c, n);
    PSNode *k = copia_raso(c, n);
    if (!k) return NULL;
    if (n->kind == N_ENTITY_DECL || n->kind == N_MODEL_DECL || n->kind == N_ENUM_DECL) gemeo_poe(c, k);
    k->a = poda(c, n->a);
    k->b = poda(c, n->b);
    k->c = poda(c, n->c);
    k->e = poda(c, n->e);
    poda_vec(c, &n->lista, &k->lista);
    poda_vec(c, &n->lista2, &k->lista2);
    poda_vec(c, &n->lista2_alias, &k->lista2_alias);
    return k;
}

/* ── as duas passadas ───────────────────────────────────────────────────── */
static void compila_prepara(C *c, PSPrograma *out, const PSResolvedor *resolve)
{
    memset(c, 0, sizeof(*c));
    c->out = out;
    c->resolve = resolve;
}

/* Primeira passada, uma declaração de topo: o que se colhe da declaração
 * INTEIRA (`cheia`) e o que se registra da PODADA (`podada`, que é a que
 * vive até o fim). Na compilação da árvore inteira as duas são o mesmo nó. */
/* `private int n = 1` no TOPO do arquivo: o parser entrega N_FIELD_DECL (ele
 * não sabe se está num método de Entity, onde a mesma grafia é campo do
 * objeto). No topo é a variável tipada do módulo com visibilidade — o mesmo
 * N_VAR_DECL de `int n = 1`. A troca é feita aqui, na entrada de cada
 * passada, pra TODA a coleta (nomes ligados, tipo do topo) já vê-la como
 * variável. */
static void topo_campo_vira_var(PSNode *s)
{
    if (s && s->kind == N_FIELD_DECL) s->kind = N_VAR_DECL;
}

static void membro_gravado_poe(C *c, const char *nome)
{
    if (!nome) return;
    for (int32_t i = 0; i < c->nmembros_gravados; i++)
        if (strcmp(c->membros_gravados[i], nome) == 0) return;
    if (c->nmembros_gravados + 1 > c->cap_membros_gravados) {
        int32_t novo = c->cap_membros_gravados < 16 ? 16 : c->cap_membros_gravados * 2;
        const char **nv = realloc(c->membros_gravados, sizeof(char *) * (size_t)novo);
        if (!nv) { cerro(c, "sem memoria", NULL); return; }
        c->membros_gravados = nv; c->cap_membros_gravados = novo;
    }
    char *d = strdup(nome);
    if (!d) { cerro(c, "sem memoria", NULL); return; }
    c->membros_gravados[c->nmembros_gravados++] = d;
}

/* Do statement INTEIRO (antes da poda, com os corpos das functs), o que pode
 * gravar um campo de objeto — ver `C.membros_gravados`. */
static void tp_colhe_gravacoes(C *c, PSNode *n)
{
    if (!n || CFALHOU(c) || fundo_demais(c, n, 1)) return;
    while (n->kind == N_BINARY_OP) {      /* cadeia `a + b + …` em laço (ver liga_o_nome) */
        tp_colhe_gravacoes(c, n->b);
        n = n->a;
        if (!n) return;
    }
    switch (n->kind) {
        case N_MEMBER_ASSIGNMENT:
            if (!n->texto2 || !strcmp(n->texto2, "=")) membro_gravado_poe(c, n->texto);
            break;
        case N_FIELD_DECL:
            membro_gravado_poe(c, n->texto);
            break;
        case N_UNPACK_TARGET:
            for (int32_t i = 0; i < n->lista.n; i++)
                if (n->lista.itens[i] && n->lista.itens[i]->kind == N_MEMBER_ACCESS)
                    membro_gravado_poe(c, n->lista.itens[i]->texto);
            break;
        case N_MEMBER_ACCESS:
            if (n->texto && !strcmp(n->texto, "__init__")) c->chama_init_nome = 1;
            break;
        case N_IMPORT_STMT: {
            char enc[512];
            import_modulo_codificado(n, enc);
            if (!ps_nativo_eh_modulo(enc)) c->importa_pr = 1;
            break;
        }
        default:
            break;
    }
    tp_colhe_gravacoes(c, n->a);
    tp_colhe_gravacoes(c, n->b);
    tp_colhe_gravacoes(c, n->c);
    tp_colhe_gravacoes(c, n->e);
    for (int32_t i = 0; i < n->lista.n; i++)        tp_colhe_gravacoes(c, n->lista.itens[i]);
    for (int32_t i = 0; i < n->lista2.n; i++)       tp_colhe_gravacoes(c, n->lista2.itens[i]);
    for (int32_t i = 0; i < n->lista2_alias.n; i++) tp_colhe_gravacoes(c, n->lista2_alias.itens[i]);
}

static void passada1_stmt(C *c, PSNode *cheia, PSNode *podada)
{
    if (CFALHOU(c)) return;
    topo_campo_vira_var(cheia);
    topo_campo_vira_var(podada);
    tp_colhe_gravacoes(c, cheia);
    coleta_tipos_topo(c, cheia);
    resolve_estrela(c, podada);
    tp_coleta_tipos_arq(c, podada);
    tp_junta_ligados(c, cheia, &c->ligados_mod, &c->nligados_mod, &c->cap_ligados_mod);
    liga_direto_stmt(c, cheia, &c->ligados_mod_raso, &c->nligados_mod_raso, &c->cap_ligados_mod_raso,
                     &c->ligados_mod_raso, &c->nligados_mod_raso, &c->cap_ligados_mod_raso);
    tp_junta_globais(c, cheia);
    if (!c->range_ligado && liga_o_nome(c, cheia, "range")) c->range_ligado = 1;
}

/* Fim da primeira passada: o que precisa de TODAS as declarações colhidas. */
static void passada1_fim(C *c, PSNode *programa)
{
    if (!programa || CFALHOU(c)) return;
    /* método sem `self`: acusa e sintetiza ANTES de qualquer chamada ser conferida */
    for (int32_t i = 0; i < c->ntipos_arq && !CFALHOU(c); i++)
        tp_self_dos_metodos(c, c->tipos_arq[i]);
    Unidade mod;
    memset(&mod, 0, sizeof(mod));
    mod.eh_modulo = 1;
    for (int32_t i = 0; i < programa->lista.n && !CFALHOU(c); i++)
        tp_pre_stmt(c, &mod, programa->lista.itens[i], 0);
}

/* Segunda passada, uma declaração de topo: o código dela. O `self`
 * sintetizado num método desta declaração é desfeito aqui mesmo, porque o nó
 * dela morre com o statement (na árvore inteira, o nó vive; desfazer cedo é
 * igual). */
static void passada2_stmt(C *c, Unidade *u, PSNode *s)
{
    if (CFALHOU(c)) return;
    int32_t marca = c->n_self_sint;
    topo_campo_vira_var(s);
    c->stmt_topo = s;
    stmt(c, u, s);
    c->stmt_topo = NULL;
    for (int32_t i = marca; i < c->n_self_sint; i++) {
        struct SelfSint *x = &c->self_sint[i];
        free(x->met->lista.itens);
        x->met->lista = x->orig;
        x->met->self_faltava = 0;
        free(x->no);
    }
    c->n_self_sint = marca;
}

static void compila_termina(C *c, Unidade *u);

PSPrograma *ps_compila(PSNode *programa)
{
    return ps_compila_com(programa, NULL);
}

PSPrograma *ps_compila_com(PSNode *programa, const PSResolvedor *resolve)
{
    PSPrograma *out = calloc(1, sizeof(PSPrograma));
    if (!out) return NULL;
    out->ok = 1;

    C c;
    compila_prepara(&c, out, resolve);

    int32_t idx = novo_proto(&c, "<module>");
    if (idx < 0) return out;

    /* tipos declarados no topo do arquivo, antes de compilar qualquer funct*/
    if (programa)
        for (int32_t i = 0; i < programa->lista.n && out->ok; i++)
            passada1_stmt(&c, programa->lista.itens[i], programa->lista.itens[i]);
    passada1_fim(&c, programa);

    Unidade u;
    memset(&u, 0, sizeof(u));
    u.idx = idx;
    u.eh_modulo = 1;

    if (programa)
        for (int32_t i = 0; i < programa->lista.n && out->ok; i++)
            passada2_stmt(&c, &u, programa->lista.itens[i]);
    compila_termina(&c, &u);
    return out;
}

PSPrograma *ps_compila_fonte(const char *fonte, size_t len, int recupera,
                             const PSResolvedor *resolve, PSFonteCompilado *fc)
{
    fc->lexer = NULL; fc->parse = NULL;
    PSPrograma *out = calloc(1, sizeof(PSPrograma));
    PSParseResult *keep = calloc(1, sizeof(PSParseResult));
    PSParserInc *pi = ps_parse_inc_abre(fonte, len, recupera);
    if (!out || !keep || !pi) {
        free(out); free(keep); ps_parse_inc_fecha(pi, NULL);
        return NULL;
    }
    out->ok = 1;
    keep->ok = 1;
    ps_arena_init(&keep->arena);
    fc->parse = keep;

    C c;
    compila_prepara(&c, out, resolve);
    c.keep = &keep->arena;
    c.fase = 1;

    PSNode *programa = ps_node_novo(c.keep, N_PROGRAM, 1, 1);
    int32_t idx = programa ? novo_proto(&c, "<module>") : -1;
    if (idx < 0) {
        ps_parse_inc_fecha(pi, &fc->lexer);
        ps_compila_free(out);
        return NULL;
    }
    keep->programa = programa;

    /* PRIMEIRA PASSADA: lê, colhe, poda, solta. Com erro de sintaxe o
     * compilador para de colher (o arquivo não compila), mas o parser segue
     * até o fim no modo de recuperação, pra lista de erros ficar inteira. */
    for (;;) {
        PSNode *s = ps_parse_inc_proxima(pi);
        if (!s) break;
        PSParseResult *rp = ps_parse_inc_resultado(pi);
        if (!out->ok || !rp->ok || rp->nerros > 0 || !ps_parse_inc_lexer(pi)->ok) continue;
        PSNode *k = poda(&c, s);
        if (!k || ps_vec_push(c.keep, &programa->lista, k) != 0) { cerro(&c, "sem memoria", NULL); continue; }
        passada1_stmt(&c, s, k);
    }
    {
        PSParseResult *rp = ps_parse_inc_resultado(pi);
        keep->ok = rp->ok;
        snprintf(keep->erro, sizeof(keep->erro), "%s", rp->erro);
        keep->erro_linha = rp->erro_linha; keep->erro_col = rp->erro_col;
        keep->erros = rp->erros; keep->nerros = rp->nerros; keep->cap_erros = rp->cap_erros;
        rp->erros = NULL; rp->nerros = rp->cap_erros = 0;
    }
    ps_parse_inc_fecha(pi, &fc->lexer);
    if (!fc->lexer || !fc->lexer->ok || !keep->ok) {
        /* sintaxe: quem chamou lê o erro em `fc`; não há programa */
        Unidade u0;
        memset(&u0, 0, sizeof(u0));
        u0.idx = idx; u0.eh_modulo = 1;
        compila_termina(&c, &u0);
        ps_compila_free(out);
        return NULL;
    }
    passada1_fim(&c, programa);

    /* SEGUNDA PASSADA: lê de novo, gera o código de cada declaração e a
     * solta. A sintaxe já foi aprovada: o mesmo fonte, o mesmo parser. */
    c.fase = 2;
    Unidade u;
    memset(&u, 0, sizeof(u));
    u.idx = idx;
    u.eh_modulo = 1;
    pi = ps_parse_inc_abre(fonte, len, 0);
    if (!pi) cerro(&c, "sem memoria", NULL);
    while (pi && out->ok) {
        PSNode *s = ps_parse_inc_proxima(pi);
        if (!s) {
            if (!ps_parse_inc_resultado(pi)->ok) cerro(&c, "sem memoria", NULL);
            break;
        }
        passada2_stmt(&c, &u, s);
    }
    ps_parse_inc_fecha(pi, NULL);
    compila_termina(&c, &u);
    return out;
}

/* ── a cota de pilha de cada proto ──────────────────────────────────────── */
/* Quantos valores MAKE_CLASS tira da pilha: os pais da classe `arg`. */
static int32_t pilha_npais(const PSPrograma *prog, int32_t arg)
{
    return (arg >= 0 && arg < prog->nclasses) ? prog->classes[arg].npais : 0;
}

/* Quantos valores MAKE_ENUM tira: os membros com valor explícito. */
static int32_t pilha_nenum(const PSPrograma *prog, int32_t arg)
{
    if (arg < 0 || arg >= prog->nenums) return 0;
    int32_t n = 0;
    for (int32_t i = 0; i < prog->enums[arg].nmembros; i++)
        if (prog->enums[arg].membros[i].tem_valor) n++;
    return n;
}

/* O efeito de um opcode sobre a pilha no caminho que SEGUE: a coluna
 * tira/deixa de `ps_opcodes.def`, que é onde ela mora de propósito. */
static void efeito_pilha(const PSPrograma *prog, int32_t op, int32_t arg,
                         int32_t *tira, int32_t *deixa)
{
    (void)prog;
    switch (op) {
#define PS_OP(nome, num, texto, t, d) case OP_##nome: *tira = (t); *deixa = (d); return;
#include "ps_opcodes.def"
#undef PS_OP
        default: *tira = 0; *deixa = 0; return;
    }
}

/* Marca a profundidade `d` na instrução `i` e a põe na fila se é a primeira
 * vez. Duas chegadas com profundidades diferentes são o compilador
 * desbalanceado (um ramo deixa um valor que o outro não deixa): invariante
 * no binário de debug; no normal a cota só cresce (vale a maior, e ela é
 * repropagada, com um teto de passadas pra não girar). */
static void pilha_chega(int32_t *prof, int32_t *fila, int32_t *nf, int32_t n,
                        int32_t i, int32_t d, int32_t *maximo, int32_t *maximo_i, int32_t *repro)
{
    if (i < 0 || i >= n || d < 0) return;
    if (prof[i] < 0) { prof[i] = d; fila[(*nf)++] = i; return; }
    if (prof[i] == d) return;
    PS_ASSERT_MSG(prof[i] == d, "pilha desbalanceada na instrucao %d: %d e %d", (int)i, (int)prof[i], (int)d);
    if (d > prof[i] && *repro < 8 * n) { prof[i] = d; fila[(*nf)++] = i; (*repro)++; }
    if (d > *maximo) { *maximo = d; *maximo_i = i; }
}

/* Profundidade máxima da pilha de valores que `code` alcança, medida do
 * início do frame: um passo sobre o código pronto, com o efeito de cada
 * opcode e fila de trabalho pelos alvos de salto. Cada instrução entra na
 * fila uma vez (e o `pilha_chega` explica a exceção), então custa o tamanho
 * do código. É a cota EXATA que a VM confere em toda entrada de frame, no
 * lugar da antiga `ncode/2` ("cada instrução empilha no máximo um valor"),
 * que valia só pra chamada de funct e recusava funct grande sem motivo. */
static int32_t ps_pilha_max_do_codigo(const PSPrograma *prog, const int32_t *code, int32_t ncode,
                                      int32_t *ip_max)
{
    int32_t n = ncode / 2;
    *ip_max = 0;
    if (n <= 0 || !code) return 0;
    int32_t *prof = malloc(sizeof(int32_t) * (size_t)n);
    int32_t *fila = malloc(sizeof(int32_t) * (size_t)(9 * (size_t)n + 1));
    if (!prof || !fila) { free(prof); free(fila); return n + 8; }   /* sem memória: a cota antiga, folgada */
    for (int32_t i = 0; i < n; i++) prof[i] = -1;
    int32_t nf = 0, maximo = 0, maximo_i = 0, repro = 0;
    prof[0] = 0; fila[nf++] = 0;
    while (nf > 0) {
        int32_t i = fila[--nf];
        int32_t d = prof[i], op = code[2 * i], arg = code[2 * i + 1];
        /* o máximo é medido na ENTRADA da instrução: a que o alcança é a que
         * consome a pilha cheia (o CALL, o BUILD_*), e é nela que o erro de
         * cota aponta — a chamada, não o último argumento */
        if (d > maximo) { maximo = d; maximo_i = i; }
        int32_t tira = 0, deixa = 0;
        efeito_pilha(prog, op, arg, &tira, &deixa);
        PS_ASSERT_MSG(d - tira >= 0, "pilha vazia demais na instrucao %d (%s): tem %d, tira %d",
                      (int)i, ps_op_nome(op), (int)d, (int)tira);
        int32_t d2 = d - tira + deixa;
        if (d2 < 0) d2 = 0;
        switch (op) {
            case OP_JUMP:
                pilha_chega(prof, fila, &nf, n, arg / 2, d2, &maximo, &maximo_i, &repro);
                break;
            case OP_JUMP_IF_FALSE: case OP_JUMP_IF_TRUE: case OP_JUMP_IF_SET:
            case OP_JF_LT: case OP_JF_GT: case OP_JF_LE: case OP_JF_GE: case OP_JF_EQ: case OP_JF_NE:
            case OP_SKIP_IF_IMPORT:
                pilha_chega(prof, fila, &nf, n, i + 1, d2, &maximo, &maximo_i, &repro);
                pilha_chega(prof, fila, &nf, n, arg / 2, d2, &maximo, &maximo_i, &repro);
                break;
            case OP_ITER_NEXT:        /* saída do laço: tira [cont, idx] */
                pilha_chega(prof, fila, &nf, n, i + 1, d2, &maximo, &maximo_i, &repro);
                pilha_chega(prof, fila, &nf, n, arg / 2, d - 2, &maximo, &maximo_i, &repro);
                break;
            case OP_ITER_RANGE:       /* saída do laço: tira [ini, quant, passo, i] */
                pilha_chega(prof, fila, &nf, n, i + 1, d2, &maximo, &maximo_i, &repro);
                pilha_chega(prof, fila, &nf, n, arg / 2, d - 4, &maximo, &maximo_i, &repro);
                break;
            case OP_JUMP_SE_UNSET:    /* quando salta, tira o UNSET */
                pilha_chega(prof, fila, &nf, n, i + 1, d2, &maximo, &maximo_i, &repro);
                pilha_chega(prof, fila, &nf, n, arg / 2, d - 1, &maximo, &maximo_i, &repro);
                break;
            case OP_SETUP_TRY:        /* o handler entra com a mensagem no topo */
                pilha_chega(prof, fila, &nf, n, i + 1, d2, &maximo, &maximo_i, &repro);
                pilha_chega(prof, fila, &nf, n, arg / 2, d + 1, &maximo, &maximo_i, &repro);
                break;
            case OP_RETURN: case OP_HALT: case OP_RAISE: case OP_RERAISE:
                break;                /* sem caminho que segue */
            default:
                pilha_chega(prof, fila, &nf, n, i + 1, d2, &maximo, &maximo_i, &repro);
                break;
        }
    }
    free(prof);
    free(fila);
    *ip_max = maximo_i;
    return maximo;
}

static void compila_termina(C *c, Unidade *u)
{
    PSPrograma *out = c->out;
    emite(c, u, OP_HALT, 0);
    if (out->ok) calcula_exportados(c, u);

    /* Erro de tipo: o programa não roda, e a lista vai INTEIRA, na ordem do
     * fonte. Erro de sintaxe achado no caminho manda (a compilação parou ali
     * e a lista estaria pela metade). */
    if (c->nerros > 0 && out->ok) {
        qsort(c->erros, (size_t)c->nerros, sizeof(PSErroTipo), tp_erro_cmp);
        out->ok = 0;
        snprintf(out->erro, sizeof(out->erro), "%s", c->erros[0].msg);
        out->erro_linha = c->erros[0].linha;
        out->erro_col = c->erros[0].col;
        out->erro_do_programa = 2;
        out->erros_tipo = c->erros;
        out->nerros_tipo = c->nerros;
        c->erros = NULL;
    }
    free(c->erros);
    /* a AST volta ao que era: o `self` sintetizado só existiu nesta compilação */
    for (int32_t i = 0; i < c->n_self_sint; i++) {
        struct SelfSint *s = &c->self_sint[i];
        free(s->met->lista.itens);
        s->met->lista = s->orig;
        s->met->self_faltava = 0;
        free(s->no);
    }
    free(c->self_sint);
    for (int32_t i = 0; i < c->ntpool; i++) free(c->tpool[i]);
    free(c->tpool);
    free(c->topo);
    free(c->fin_pend);
    /* laço que ficou aberto (a compilação parou no meio dele) ainda tem os
     * vetores de salto; o fechamento normal é `fecha_laco` */
    for (int32_t i = 0; i < c->nlacos; i++) {
        saltos_solta(&c->lacos[i].saidas);
        saltos_solta(&c->lacos[i].continues);
    }
    c->nlacos = 0;
    ix_solta(&c->ix_topo);
    for (int32_t i = 0; i < c->nligados_mod; i++) free(c->ligados_mod[i]);
    free(c->ligados_mod);
    ix_solta(&c->ix_ligados_mod);
    for (int32_t i = 0; i < c->nligados_mod_raso; i++) free(c->ligados_mod_raso[i]);
    free(c->ligados_mod_raso);
    ix_solta(&c->ix_ligados_mod_raso);
    ix_solta(&c->ix_globais);        /* as chaves são do programa de saída: só o índice sai */
    free(c->tipos_arq);
    free(c->gemeos);

    for (int32_t e = 0; e < c->nestrelas; e++) {
        for (int32_t k = 0; k < c->estrelas[e].n; k++) free(c->estrelas[e].nomes[k]);
        free(c->estrelas[e].nomes);
    }
    free(c->estrelas);
    for (int32_t i = 0; i < c->nglobais_gravados; i++) free(c->globais_gravados[i]);
    free(c->globais_gravados);
    for (int32_t i = 0; i < c->nmembros_gravados; i++) free((char *)c->membros_gravados[i]);
    free(c->membros_gravados);
    for (int32_t i = 0; i < c->ntipos_ambiguos; i++) free(c->tipos_ambiguos[i]);
    free(c->tipos_ambiguos);
    proto_encolhe(c, u);

    /* a cota de pilha de CADA proto, com o código pronto (ver `pilha_max`):
     * aqui, e não por unidade, pra nenhum proto ficar sem ela. Só com a
     * compilação inteira: código de compilação que parou no meio é
     * desbalanceado por natureza, e não vai rodar. */
    if (out->ok) for (int32_t i = 0; i < out->nprotos; i++)
        out->protos[i].pilha_max = ps_pilha_max_do_codigo(out, out->protos[i].code, out->protos[i].ncode,
                                                          &out->protos[i].pilha_max_ip);

    vardbg_fecha(c, u, 0);
    for (int32_t i = 0; i < u->nlocais; i++) free(u->locais[i]);
    free(u->locais);
    free(u->celula); free(u->celula_virgem); free(u->certo); free(u->tipo_decl);
    free(u->sim); free(u->mod_sim);
    for (int32_t i = 0; i < u->nligados; i++) free(u->ligados[i]);
    free(u->ligados);
    for (int32_t i = 0; i < u->n_mod_criados; i++) free(u->mod_criados[i]);
    free(u->mod_criados);
    free(u->mod_hist);
    ix_solta(&u->ix_mod);
    for (int32_t i = 0; i < u->nglobais_decl; i++) free(u->globais_decl[i]);
    free(u->globais_decl);
    for (int32_t i = 0; i < c->n_tipos_topo; i++) free(c->tipos_topo_nomes[i]);
    free(c->tipos_topo_nomes);
    free(c->tipos_topo);
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
        if (p->protos[i].param_tipos) {
            for (int32_t k = 0; k < p->protos[i].nparams; k++)
                free(p->protos[i].param_tipos[k]);
            free(p->protos[i].param_tipos);
        }
        for (int32_t k = 0; k < p->protos[i].nconsts; k++)
            free(p->protos[i].consts[k].s);
        free(p->protos[i].consts);
        free(p->protos[i].upvals);
        if (p->protos[i].upval_nomes) {
            for (int32_t k = 0; k < p->protos[i].nupvals; k++)
                free(p->protos[i].upval_nomes[k]);
            free(p->protos[i].upval_nomes);
        }
        for (int32_t k = 0; k < p->protos[i].nvars; k++)
            free(p->protos[i].vars[k].nome);
        free(p->protos[i].vars);
        free(p->protos[i].faixas_classe);
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
        for (int32_t k = 0; k < p->classes[i].nmembros; k++) {
            free(p->classes[i].membros[k].nome);
            free(p->classes[i].membros[k].tipo);
        }
        free(p->classes[i].membros);
        for (int32_t k = 0; k < p->classes[i].ntip; k++) {
            free(p->classes[i].tip_nomes[k]);
            free(p->classes[i].tip_tipos[k]);
        }
        free(p->classes[i].tip_nomes);
        free(p->classes[i].tip_tipos);
    }
    free(p->classes);
    for (int32_t i = 0; i < p->nmodels; i++) {
        free(p->models[i].nome);
        for (int32_t k = 0; k < p->models[i].ncampos; k++) {
            PSModelCampoDef *cd = &p->models[i].campos[k];
            free(cd->nome); free(cd->tipo_model); free(cd->regex); free(cd->of_model);
            mlits_solta(cd->in, cd->n_in);
            mlits_solta(cd->not_in, cd->n_not_in);
        }
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
    for (int32_t i = 0; i < p->npriv_globais; i++) free(p->priv_globais[i]);
    free(p->priv_globais);
    free(p->priv_linhas);
    free(p->priv_cols);
    for (int32_t i = 0; i < p->nexportados; i++) free(p->exportados[i]);
    free(p->exportados);
    free(p->erros_tipo);
    free(p);
}
