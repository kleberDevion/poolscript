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
#include "ps_retornos.h"

#include <stdarg.h>

#include "ps_lexer.h"
#include "ps_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── opcodes ─────────────────────────────────────────────────────────────
 * A lista vive em `ps_opcodes.def` e é a MESMA que a VM inclui. Antes eram
 * dois enums escritos à mão, e nada no build conferia que batiam. */
enum {
#define PS_OP(nome, num, texto) OP_##nome = (num),
#include "ps_opcodes.def"
#undef PS_OP
    OP__ULTIMO
};

/* Nome pro desmontador. O texto vem da terceira coluna do `.def` porque nem
 * todo opcode se imprime com o próprio sufixo: OP_IS sai como "IS_OP". */
const char *ps_op_nome(int32_t op)
{
    switch (op) {
#define PS_OP(nome, num, texto) case OP_##nome: return texto;
#include "ps_opcodes.def"
#undef PS_OP
        default: break;
    }
    return "?";
}

/* ── estado ─────────────────────────────────────────────────────────────── */
/* Guarda o ÍNDICE do protótipo, não o ponteiro: uma action aninhada chama
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
} SimInfo;

struct Unidade {
    /* Função que ENVOLVE esta. NULL no módulo e nas actions de topo — é o que
     * limita a captura: nome não achado aqui nem no pai vira global. */
    Unidade *pai;
    /* Os quatro vetores por SLOT abaixo (celula, celula_virgem, certo,
     * tipo_decl) crescem junto com `locais` (ver `idx_local`). Eram fixos em
     * 256: do slot 256 em diante o local perdia o tipo declarado (`int v299 =
     * 1` aceitava "texto"), a via rápida e a captura por closure — calado. */
    /* Slots que viram CÉLULA porque alguma action aninhada os usa. */
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
    /* contador de compreensões de lista, pra o nome do acumulador ser único
     * quando uma está dentro da outra */
    int         n_listcomp;
    int32_t     cap_classes;
    int32_t     cap_models;
    int32_t     cap_enums;
    /* `@NonNull` visto, esperando a action que ele decora. */
    int         pendente_nonnull;
    /* `@static` visto, esperando a action que ele decora — vira Proto.eh_static */
    int         pendente_static;
    /* profundidade de decorador EMPILHADO em compilação: cada nível guarda o
     * próprio decorador num temporário distinto (`$reg0`, `$reg1`…), senão
     * o de dentro sobrescrevia o de fora antes de ele ser usado */
    int         decor_prof;
    /* `finally` PENDENTES (try aninhado): o bloco é emitido inline nas saídas
     * do try, mas `return`, `break` e `continue` saltam por fora — sem isto o
     * finally simplesmente NÃO rodava nesses três caminhos. Cada entrada
     * guarda o bloco e em que laço/função ele estava, pra saber quais rodar. */
    PSNode     *fin_bloco[32];
    int         fin_laco[32];     /* c->nlacos no momento em que entrou */
    const void *fin_unidade[32];  /* qual Unidade (função) abriu o try */
    int         nfinally;
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
    /* O nó da Entity cujo corpo/métodos estão sendo compilados. É por ele que
     * um nome solto dentro da classe resolve pra campo `static` dela
     * (`mapp` -> `App.mapp`), no corpo (decoradores) e nos métodos. */
    PSNode *entity_no;
    /* Linha do fonte do nó sendo compilado agora — o `emite()` grava por
     * instrução, pra o erro de runtime dizer ONDE aconteceu. */
    int32_t linha_atual;
    int32_t coluna_atual;
    /* Raiz da AST do programa — a especialização do `for each ... in range`
     * precisa varrer o arquivo INTEIRO pra saber se `range` foi redefinido
     * em algum ponto (inclusive depois do laço). */
    PSNode *raiz;
    /* Tipo declarado dos nomes de MODULO (`str s = ...` no topo do arquivo,
     * dentro ou fora de bloco), colhido numa passada antes de compilar: uma
     * action compilada ANTES da declaracao ainda precisa saber o tipo pra
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
    int32_t  nestrelas;
    /* o statement de topo do arquivo sendo compilado agora: `*` só vale nele */
    PSNode  *stmt_topo;
    /* nomes de MÓDULO gravados de dentro de uma funct com `global x` — o
     * arquivo os liga, então saem no import como os de topo */
    char   **globais_gravados;
    int32_t  nglobais_gravados;

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
    int      nomes_incertos;
    /* o arquivo importa o jinker: `request` e `channel` são injetados quando
     * o app sobe */
    int      usa_jinker;
    int32_t  ntopo, cap_topo;
    /* Toda Entity, model e enum do arquivo, em qualquer profundidade, por
     * nome — é o que diz se um nome de tipo existe e quem herda de quem. */
    PSNode **tipos_arq;
    int32_t  ntipos_arq, cap_tipos_arq;
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
             /* o nome passa a valer OUTRA coisa, de tipo desconhecido — o
              * resultado de um decorador geral sobre a funct */
             unsigned char redefine; } grava;
    /* Na chamada de SAÍDA de uma funct tipada (`post` dentro de `int funct`):
     * o tipo que cada argumento posicional tem que ter, e o rótulo do erro.
     * `emite_args_e_chama` consome e limpa. */
    const char *saida_tipo;
    int         saida_indice;       /* -2 = todos os posicionais */
    char        saida_rotulo[300];
} C;

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
        if (k == K_STR || k == K_BIGINT || k == K_BYTES) {
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
    if (k == K_STR || k == K_BIGINT || k == K_BYTES) {
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

static int32_t idx_local(C *c, Unidade *u, const char *nome)
{
    for (int32_t i = 0; i < u->nlocais; i++)
        if (strcmp(u->locais[i], nome) == 0) return i;
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
    /* nlocals = MARCA D'ÁGUA (maior slot já usado + 1), não o corrente: com
     * escopo de bloco os slots são reaproveitados (nlocais encolhe no fim do
     * bloco), e o frame precisa caber o pico, não o valor do momento. */
    if (u->nlocais + 1 > UP(c, u)->nlocals) UP(c, u)->nlocals = u->nlocais + 1;
    vardbg_abre(c, u, nome, u->nlocais);
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
        for (int32_t i = u->n_mod_criados - 1; i >= marca; i--)
            free(u->mod_criados[i]);
        if (u->n_mod_criados > marca) u->n_mod_criados = marca;
    } else {
        vardbg_fecha(c, u, marca);
        for (int32_t i = u->nlocais - 1; i >= marca; i--) {
            free(u->locais[i]);
            /* o slot volta LIMPO: sem zerar o tipo, o próximo nome que
             * caísse nele herdava o tipo declarado do bloco que já fechou */
            u->certo[i] = 0;
            u->tipo_decl[i] = 0;
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
    for (int32_t i = 0; i < u->n_mod_criados; i++)
        if (strcmp(u->mod_criados[i], nome) == 0) return;
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
    memset(&u->mod_sim[u->n_mod_criados], 0, sizeof(SimInfo));
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
static int tp_aceita(C *c, const char *d, const char *v, PSNode *no, int declarado);
static void tp_emite_confere(C *c, Unidade *u, const char *rotulo, const char *tipo, int declarado);
static void expr(C *c, Unidade *u, PSNode *n);
static void compila_fstring(C *c, Unidade *u, PSNode *n);
static void stmt(C *c, Unidade *u, PSNode *n);
static int32_t compila_action(C *c, PSNode *n, Unidade *pai);

/* `__init__` sintetizado a partir dos campos tipados (`nome: str`).
 *
 * Não depende do `@dataentity`: o decorador é só um marcador, e no
 * interpretador a Entity ganha o init pelos campos com ou sem ele. Gerar
 * bytecode direto (em vez de montar uma AST) evita inventar nós sem posição
 * de origem, que estragariam a mensagem de erro. */
static int32_t sintetiza_init(C *c, PSNode *entidade);
static void compila_unpack_alvo(C *c, Unidade *u, PSNode *alvo);
static void guarda_em_alvo(C *c, Unidade *u, PSNode *e);

/* Nomes de `private <tipo> <nome> = ...` na subárvore. `nomes == NULL` só
 * conta (pra dimensionar o vetor); `achados` é o índice onde começar a
 * escrever, então a varredura soma aos nomes que os campos do corpo da
 * classe já puseram. */
static int32_t varre_campos_priv(PSNode *n, char **nomes, int32_t achados)
{
    if (!n) return achados;
    if (n->kind == N_FIELD_DECL && n->is_private && n->texto) {
        if (nomes) nomes[achados] = strdup(n->texto);
        achados++;
    }
    achados = varre_campos_priv(n->a, nomes, achados);
    achados = varre_campos_priv(n->b, nomes, achados);
    achados = varre_campos_priv(n->c, nomes, achados);
    achados = varre_campos_priv(n->e, nomes, achados);
    for (int32_t i = 0; i < n->lista.n; i++)
        achados = varre_campos_priv(n->lista.itens[i], nomes, achados);
    for (int32_t i = 0; i < n->lista2.n; i++)
        achados = varre_campos_priv(n->lista2.itens[i], nomes, achados);
    return achados;
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


/* `range` é redefinível como qualquer nome (`range = 5`, `action range()`,
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
        const char *b = strrchr(spec, '/'); b = b ? b + 1 : spec;
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

static int nome_bate(const PSNode *x, const char *alvo)
{
    return x && x->texto && !strcmp(x->texto, alvo);
}

/* Toda forma que LIGA um nome entra aqui. A lista antiga via atribuição,
 * declaração e import por lista, e deixava de fora parâmetro (`funct f(range)`),
 * `import json as range`, variável de `catch`, `using ... as`, captura de
 * `match`, alvo de desempacotamento e a variável da compreensão — em todos
 * esses o `for each i in range(2)` rodava o `range` embutido, calado.
 * Sobrar aqui só desliga o atalho; faltar troca o que o programa faz. */
static int liga_o_nome(C *c, PSNode *n, const char *alvo)
{
    if (!n) return 0;
    switch (n->kind) {
        case N_ASSIGNMENT: case N_VAR_DECL: case N_FOR_EACH_STMT: case N_LIST_COMP:
        case N_ENTITY_DECL: case N_MODEL_DECL: case N_ENUM_DECL:
        case N_USING_STMT: case N_CATCH_CLAUSE:
            if (nome_bate(n, alvo)) return 1;
            break;
        case N_MATCH_PATTERN:
            if (nome_bate(n, alvo) || (n->texto2 && !strcmp(n->texto2, alvo))) return 1;
            break;
        case N_ACTION_DECL: case N_LAMBDA_EXPR: case N_UNPACK_TARGET:
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
                    if (c->estrelas[e].no != n) continue;
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
 * actions aninhadas (`marca_celulas`): sem isso o `a = 1` já teria sido
 * emitido como slot cru quando o `action dentro()` aparecesse depois. */

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

/* Índice do upvalue `nome` nesta unidade, criando a cadeia de captura pelos
 * pais se preciso. -1 = o nome não vem de nenhuma função de fora. */
static int32_t resolve_upval(C *c, Unidade *u, const char *nome)
{
    if (!u->pai || u->pai->eh_modulo) return -1;
    for (int32_t i = 0; i < u->nupvals; i++)
        if (strcmp(u->upvals[i].nome, nome) == 0) return i;
    for (int32_t i = 0; i < u->pai->nlocais; i++)
        /* `celula` e `locais` têm a mesma capacidade (crescem juntos) */
        if (u->pai->celula[i] && strcmp(u->pai->locais[i], nome) == 0)
            return add_upval(c, u, nome, 1, i);
    int32_t k = resolve_upval(c, u->pai, nome);
    if (k >= 0) return add_upval(c, u, nome, 0, k);
    return -1;
}

/* Junta no vetor os nomes que APARECEM dentro de uma action aninhada. É
 * conservador de propósito: se o nome bate com um local de fora, aquele slot
 * vira célula mesmo que a aninhada só use um homônimo dela. Marcar a mais
 * custa uma indireção; marcar a menos quebraria a captura. */
static void junta_nomes(C *c, char ***v, int32_t *n, int32_t *cap, const char *nome)
{
    if (!nome || !*nome) return;
    for (int32_t i = 0; i < *n; i++) if (!strcmp((*v)[i], nome)) return;
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
    (*v)[(*n)++] = copia;
}

static void varre_nomes(C *c, PSNode *n, char ***v, int32_t *cnt, int32_t *cap)
{
    if (!n) return;
    switch (n->kind) {
        case N_NAME: case N_ASSIGNMENT: case N_VAR_DECL: case N_FOR_EACH_STMT:
            junta_nomes(c, v, cnt, cap, n->texto);
            break;
        case N_ACTION_DECL:
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
    if (!n) return;
    if (n->kind == N_ACTION_DECL || n->kind == N_LAMBDA_EXPR) { varre_nomes(c, n, v, cnt, cap); return; }
    acha_aninhadas(c, n->a, v, cnt, cap);
    acha_aninhadas(c, n->b, v, cnt, cap);
    acha_aninhadas(c, n->c, v, cnt, cap);
    acha_aninhadas(c, n->e, v, cnt, cap);
    for (int32_t i = 0; i < n->lista.n; i++)  acha_aninhadas(c, n->lista.itens[i], v, cnt, cap);
    for (int32_t i = 0; i < n->lista2.n; i++) acha_aninhadas(c, n->lista2.itens[i], v, cnt, cap);
}

/* Nomes que ESTA função liga: parâmetro, atribuição, declaração tipada,
 * variável de for-each, action aninhada e alvo de desempacotamento. Não entra
 * nas actions aninhadas — o que elas ligam é escopo delas. */
static void binda_nomes(C *c, PSNode *n, char ***v, int32_t *cnt, int32_t *cap)
{
    if (!n) return;
    switch (n->kind) {
        case N_ASSIGNMENT: case N_VAR_DECL: case N_FOR_EACH_STMT:
        case N_UNPACK_TARGET:
            junta_nomes(c, v, cnt, cap, n->texto);
            break;
        case N_ACTION_DECL:
            junta_nomes(c, v, cnt, cap, n->texto);
            return;                 /* não desce: o corpo dela é outro escopo */
        case N_LAMBDA_EXPR:
            return;                 /* não liga nome nenhum aqui, e o corpo é dela */
        default: break;
    }
    binda_nomes(c, n->a, v, cnt, cap);
    binda_nomes(c, n->b, v, cnt, cap);
    binda_nomes(c, n->c, v, cnt, cap);
    binda_nomes(c, n->e, v, cnt, cap);
    for (int32_t i = 0; i < n->lista.n; i++)  binda_nomes(c, n->lista.itens[i], v, cnt, cap);
    for (int32_t i = 0; i < n->lista2.n; i++) binda_nomes(c, n->lista2.itens[i], v, cnt, cap);
}

/* Reserva o slot e emite a célula das variáveis capturadas: as que ESTA função
 * liga E que alguma action aninhada cita. A interseção é o que evita
 * transformar uma GLOBAL lida lá dentro em célula vazia. Roda depois do
 * prólogo dos defaults (que testa o slot cru com JUMP_IF_SET) e antes do
 * corpo — os acessos precisam já saber que o slot virou célula. */
static void marca_celulas(C *c, Unidade *u, PSNode *params, PSNode *corpo)
{
    char **usados = NULL; int32_t nu = 0, cu = 0;
    char **ligados = NULL; int32_t nl = 0, cl = 0;
    acha_aninhadas(c, corpo, &usados, &nu, &cu);
    binda_nomes(c, corpo, &ligados, &nl, &cl);
    if (params)
        for (int32_t i = 0; i < params->lista.n; i++)
            junta_nomes(c, &ligados, &nl, &cl, params->lista.itens[i]->texto);
    for (int32_t i = 0; i < nu && !CFALHOU(c); i++) {
        int liga = 0;
        for (int32_t k = 0; k < nl; k++) if (!strcmp(ligados[k], usados[i])) { liga = 1; break; }
        if (!liga) continue;
        if (eh_global_declarada(u, usados[i])) continue;   /* `global x` manda */
        int32_t slot = idx_local(c, u, usados[i]);
        if (slot < 0 || u->celula[slot]) continue;
        u->celula[slot] = 1;
        u->celula_virgem[slot] = 1;
        emite(c, u, OP_MAKE_CELL, slot);
    }
    for (int32_t i = 0; i < nu; i++) free(usados[i]);
    free(usados);
    for (int32_t i = 0; i < nl; i++) free(ligados[i]);
    free(ligados);
}

/* MAKE_CLOSURE só quando a action realmente captura algo: sem upvalue ela
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

/* `nome` é um campo `static` da Entity que está sendo compilada? */
static int campo_estatico_da_classe(C *c, const char *nome)
{
    PSNode *e = c->entity_no;
    if (!e || !nome) return 0;
    for (int32_t i = 0; i < e->lista2_alias.n; i++) {
        PSNode *f = e->lista2_alias.itens[i];
        if (f && f->kind == N_ENTITY_FIELD && f->is_static && f->texto
                && strcmp(f->texto, nome) == 0) return 1;
    }
    return 0;
}

/* `mapp` dentro da classe App -> `App.mapp` (a classe é um global). */
static void carrega_estatico(C *c, Unidade *u, const char *nome)
{
    const char *cls = c->entity_no->texto ? c->entity_no->texto : "";
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
 * `i2`). É a mesma emissão em qualquer posição (funct solta, em cima de
 * classe, dentro de classe). O valor fica no topo da pilha: é o decorador a
 * quem o OP_DECORA entrega a funct. */
static void tp_confere_nome(C *c, Unidade *u, const char *nome, PSNode *onde);
static void emite_decorador_expr(C *c, Unidade *u, PSNode *dec)
{
    tp_confere_nome(c, u, dec->lista.itens[0]->texto, dec);
    carrega_nome(c, u, dec->lista.itens[0]->texto);
    for (int32_t i = 1; i < dec->lista.n; i++)
        emite(c, u, OP_GET_MEMBER,
              idx_const(c, u, K_STR, 0, 0, dec->lista.itens[i]->texto,
                        (int32_t)strlen(dec->lista.itens[i]->texto)));
    if (!dec->i2) return;              /* `@log`: o valor, sem chamar */
    emite_args_e_chama(c, u, dec, &dec->lista2);
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

static void carrega_nome(C *c, Unidade *u, const char *nome)
{
    /* Campo `static` da Entity em compilação ganha do global e do "resolve em
     * runtime" — mas NÃO de um local: parâmetro com o mesmo nome continua
     * sendo o parâmetro. */
    if (c->entity_no && !nome_e_local(u, nome) && campo_estatico_da_classe(c, nome)) {
        carrega_estatico(c, u, nome);
        return;
    }
    if (u->eh_modulo || eh_global_declarada(u, nome)) {
        emite(c, u, OP_LOAD_GLOBAL, idx_global(c, nome));
        return;
    }
    for (int32_t i = 0; i < u->nlocais; i++) {
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
 * `i` valendo 2 pra sempre, e com uma ACTION de mesmo nome a função sumia. */
static int nome_ja_existe(Unidade *u, const char *nome)
{
    if (!nome || !*nome) return 0;
    if (u->eh_modulo) {
        for (int32_t i = 0; i < u->n_mod_criados; i++)
            if (strcmp(u->mod_criados[i], nome) == 0) return 1;
        return 0;
    }
    for (int32_t i = 0; i < u->nlocais; i++)
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
 * each`, desempacotamento, write-through de dentro de uma action e closure.
 * Nada muda na VM alem do proprio opcode conferir tambem list/dict/tup/Object.
 *
 * Nome sem tipo declarado continua como sempre: `x = 1` depois `x = "a"`. */

/* `private action f()` no nivel do modulo: o nome entra na lista que o
 * `import` consulta. Antes o `private` compilava e nao fazia nada. */
static void priv_global_add(C *c, const char *nome)
{
    if (!nome || !c->out) return;
    char **nv = realloc(c->out->priv_globais, sizeof(char *) * (size_t)(c->out->npriv_globais + 1));
    if (!nv) { cerro(c, "sem memoria", NULL); return; }
    c->out->priv_globais = nv;
    c->out->priv_globais[c->out->npriv_globais] = strdup(nome);
    if (c->out->priv_globais[c->out->npriv_globais]) c->out->npriv_globais++;
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

/* Passada previa: toda declaracao tipada no escopo de MODULO (fora de action,
 * dentro ou fora de bloco). A primeira declaracao de cada nome vence. */
static void coleta_tipos_topo(C *c, PSNode *n)
{
    if (!n) return;
    if (n->kind == N_ACTION_DECL) return;       /* o corpo dela e outro escopo */
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
        for (int32_t i = 0; i < q->nlocais; i++)
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
 * Variável sem tipo escrito tem o tipo fixado pela primeira atribuição, como
 * o `var` do Java: o tipo que o compilador sabe do primeiro valor. Valor de
 * tipo desconhecido fixa "qualquer" (aceita tudo, como Object). Null não fixa
 * nem conflita: variável sem tipo escrito pode começar e voltar a ficar vazia.
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

/* A Entity/model/enum do arquivo com esse nome (a primeira), ou NULL. */
static PSNode *tp_tipo_arq(C *c, const char *nome)
{
    if (!nome) return NULL;
    for (int32_t i = 0; i < c->ntipos_arq; i++)
        if (c->tipos_arq[i]->texto && strcmp(c->tipos_arq[i]->texto, nome) == 0) return c->tipos_arq[i];
    return NULL;
}

static void tp_coleta_tipos_arq(C *c, PSNode *n)
{
    if (!n) return;
    if ((n->kind == N_ENTITY_DECL || n->kind == N_MODEL_DECL || n->kind == N_ENUM_DECL) && n->texto
            && !tp_tipo_arq(c, n->texto)) {
        if (c->ntipos_arq + 1 > c->cap_tipos_arq) {
            int32_t novo = c->cap_tipos_arq < 8 ? 8 : c->cap_tipos_arq * 2;
            PSNode **nv = realloc(c->tipos_arq, sizeof(PSNode *) * (size_t)novo);
            if (!nv) { cerro(c, "sem memoria", NULL); return; }
            c->tipos_arq = nv; c->cap_tipos_arq = novo;
        }
        c->tipos_arq[c->ntipos_arq++] = n;
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
    if (strcmp(v, "Null") == 0) return declarado ? T_NAO : T_SIM;
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
        if (strcmp(d, "Object") == 0) {
            static const char *const NAO_OBJ[] = { "str", "list", "dict", "tup", "byte", "int", "flo", "bool" };
            for (size_t i = 0; i < sizeof(NAO_OBJ) / sizeof(NAO_OBJ[0]); i++)
                if (strcmp(v, NAO_OBJ[i]) == 0) return T_NAO;
            return strcmp(v, "type") == 0 ? T_TALVEZ : T_SIM;
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
 * liga sem que se saiba o que é (a classe pode vir do módulo). */
static SimInfo *tp_sim_de(C *c, Unidade *u, const char *nome);
static int tp_tipo_existe(C *c, Unidade *u, const char *t)
{
    if (!t) return 1;
    if (ps_tipo_info(t) || tp_tipo_arq(c, t)) return 1;
    if (ps_retorno_tipo_existe(t) || ps_nativo_tem_membro(t, "type") != -1) return 1;
    SimInfo *s = tp_sim_de(c, u, t);
    return s && s->estado == 1 && !s->tipo && !s->decl;
}

/* ── o que se sabe de um nome ── */
static SimInfo *tp_sim_topo(C *c, const char *nome)
{
    for (int32_t i = 0; i < c->ntopo; i++)
        if (strcmp(c->topo[i].nome, nome) == 0) return &c->topo[i].s;
    return NULL;
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

static SimInfo *tp_sim_de(C *c, Unidade *u, const char *nome)
{
    if (!nome) return NULL;
    /* campo `static` da classe: não tem tipo estático aqui */
    if (c->entity_no && !nome_e_local(u, nome) && campo_estatico_da_classe(c, nome)) return NULL;
    if (u->eh_modulo) {
        for (int32_t i = u->n_mod_criados - 1; i >= 0; i--)
            if (strcmp(u->mod_criados[i], nome) == 0) return &u->mod_sim[i];
        return tp_sim_topo(c, nome);
    }
    if (eh_global_declarada(u, nome)) return tp_sim_topo(c, nome);
    for (int32_t i = 0; i < u->nlocais; i++) {
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
        for (int32_t i = 0; i < q->nlocais; i++)
            if (strcmp(q->locais[i], nome) == 0) return &q->sim[i];
    return tp_sim_topo(c, nome);
}

/* O SimInfo é um dos globais colhidos antes de compilar? Esses valem pro
 * arquivo inteiro: não se põem de lado nem se restauram por laço. */
static int tp_eh_topo(C *c, const SimInfo *s)
{
    for (int32_t i = 0; i < c->ntopo; i++)
        if (&c->topo[i].s == s) return 1;
    return 0;
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
    if (!n) return;
    switch (n->kind) {
        case N_ASSIGNMENT: case N_VAR_DECL: case N_FOR_EACH_STMT: case N_LIST_COMP:
        case N_ENTITY_DECL: case N_MODEL_DECL: case N_ENUM_DECL: case N_USING_STMT:
            junta_nomes(c, v, cnt, cap, n->texto);
            break;
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
        case N_ACTION_DECL:
            junta_nomes(c, v, cnt, cap, n->texto);
            return;                          /* o corpo é outro escopo */
        case N_LAMBDA_EXPR:
            return;
        case N_IMPORT_STMT: {
            if (n->texto3) {                 /* `*`: os nomes que o módulo exporta */
                int achou = 0;
                for (int32_t e = 0; e < c->nestrelas; e++) {
                    if (c->estrelas[e].no != n) continue;
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
            return;
        }
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
    if (!n) return;
    if (n->kind == N_GLOBAL_STMT)
        for (int32_t i = 0; i < n->lista.n; i++)
            junta_nomes(c, &c->ligados_mod, &c->nligados_mod, &c->cap_ligados_mod, n->lista.itens[i]->texto);
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

/* O nome lido existe em algum lugar? A funct e as que a envolvem, o arquivo,
 * o campo `static` da classe em compilação, o que a VM liga sozinha e o que o
 * jinker injeta. Nome que nada disso liga é `NameError` certo rodando — e
 * sai antes: uma `int funct` engolia esse erro e o programa terminava calado,
 * com `rc=0`, sem nunca ter feito o que devia. */
static void tp_confere_nome(C *c, Unidade *u, const char *nome, PSNode *onde)
{
    if (!nome || !*nome || c->nomes_incertos) return;
    for (Unidade *q = u; q; q = q->pai)
        if (tp_na_lista(q->ligados, q->nligados, nome)) return;
    if (tp_na_lista(c->ligados_mod, c->nligados_mod, nome)) return;
    if (c->entity_no && campo_estatico_da_classe(c, nome)) return;
    if (ps_nome_pre_ligado(nome) || ps_tipo_info(nome)) return;
    if (c->usa_jinker && (!strcmp(nome, "request") || !strcmp(nome, "channel"))) return;
    terro(c, onde, "NameError", "name '%s' is not defined", nome);
}

/* O programa liga `nome` a alguma coisa? Slot reservado só pela leitura
 * (o `carrega_nome` reserva um pra resolver rodando) não conta: sem nada
 * gravado, o nome é o builtin — `post` continua sendo o `post`. */
static int tp_nome_ligado(C *c, Unidade *u, const char *nome)
{
    SimInfo *s = tp_sim_de(c, u, nome);
    return s && (s->estado || s->decl || s->mod_nativo);
}

/* ── o tipo de uma expressão ── */
static int tp_tem_yield(PSNode *n)
{
    if (!n) return 0;
    if (n->kind == N_YIELD_STMT) return 1;
    if (n->kind == N_ACTION_DECL || n->kind == N_LAMBDA_EXPR) return 0;
    if (tp_tem_yield(n->a) || tp_tem_yield(n->b) || tp_tem_yield(n->c) || tp_tem_yield(n->e)) return 1;
    for (int32_t i = 0; i < n->lista.n; i++)  if (tp_tem_yield(n->lista.itens[i])) return 1;
    for (int32_t i = 0; i < n->lista2.n; i++) if (tp_tem_yield(n->lista2.itens[i])) return 1;
    return 0;
}

/* O retorno que a funct DECLARA — e só quando a chamada devolve esse valor:
 * gerador devolve o gerador, `async` devolve o future. */
static const char *tp_ret_decl(PSNode *decl)
{
    if (!decl || decl->kind != N_ACTION_DECL || !decl->texto2) return NULL;
    if (decl->is_async || tp_tem_yield(decl->b)) return NULL;
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
        if (m->kind == N_ACTION_DECL && m->texto && strcmp(m->texto, nome) == 0) return m;
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
        if (d->lista.itens[i] != met) continue;
        PSNode *ant = d->lista.itens[i - 1];
        if (ant->kind != N_DECORATOR_STMT || !ant->a) return 0;
        const char *dn = ant->a->lista.n == 1 ? ant->a->lista.itens[0]->texto : NULL;
        return !(dn && (!strcmp(dn, "static") || !strcmp(dn, "NonNull") || !strcmp(dn, "dataentity")));
    }
    return 0;
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
    if (strcmp(nome, "type") == 0) return 1;
    for (int32_t i = 0; i < d->lista2_alias.n; i++)
        if (d->lista2_alias.itens[i]->texto && strcmp(d->lista2_alias.itens[i]->texto, nome) == 0) return 1;
    for (int32_t i = 0; i < d->lista.n; i++) {
        PSNode *m = d->lista.itens[i];
        if (m->kind == N_ACTION_DECL && m->texto && strcmp(m->texto, nome) == 0) return 1;
        if (m->kind == N_ACTION_DECL && (tp_self_grava(m->b, nome) || tp_campo_decl_em(m->b, nome))) return 1;
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

/* O módulo NATIVO que a expressão é — `os` (ligado por import), `Parsing`
 * (nasce ligado) ou `sys.stdout` — escrito em `buf`; NULL = não é módulo. */
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
        if (strcmp(n->texto, "Parsing") == 0) { snprintf(buf, cap, "Parsing"); return buf; }
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
                if (s->decl->kind == N_ACTION_DECL) return tp_ret_decl(s->decl);
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
static const char *tp_binario(C *c, const char *op, const char *a, const char *b)
{
    if (!op) return NULL;
    static const char *const LOGICOS[] = { "is", "is not", "not is", "in", "not in", "and", "&&",
                                           "or", "||", "==", "!=", "<", ">", "<=", ">=" };
    for (size_t i = 0; i < sizeof(LOGICOS) / sizeof(LOGICOS[0]); i++)
        if (strcmp(op, LOGICOS[i]) == 0) return "bool";
    if (!a || !b || strchr(a, '|') || strchr(b, '|')) return NULL;
    int na = tp_numerico(a), nb = tp_numerico(b);
    int flo = !strcmp(a, "flo") || !strcmp(b, "flo");
    int inteiro_b = !strcmp(b, "int") || !strcmp(b, "bool");
    int inteiro_a = !strcmp(a, "int") || !strcmp(a, "bool");
    if (!strcmp(op, "+")) {
        if (na && nb) return flo ? "flo" : "int";
        if (!strcmp(a, b) && tp_sequencia(a)) return a;
        return NULL;
    }
    if (!strcmp(op, "-") || !strcmp(op, "//")) return (na && nb) ? (flo ? "flo" : "int") : NULL;
    if (!strcmp(op, "%")) {
        if (na && nb) return flo ? "flo" : "int";
        return !strcmp(a, "str") ? "str" : NULL;
    }
    if (!strcmp(op, "*")) {
        if (na && nb) return flo ? "flo" : "int";
        if (tp_sequencia(a) && inteiro_b) return a;
        if (tp_sequencia(b) && inteiro_a) return b;
        return NULL;
    }
    if (!strcmp(op, "/")) return (na && nb) ? "flo" : NULL;
    if (!strcmp(op, "**")) return (na && nb) ? (flo ? "flo" : tp_guarda(c, "int|flo")) : NULL;
    if (!strcmp(op, "&") || !strcmp(op, "|") || !strcmp(op, "^") || !strcmp(op, "<<") || !strcmp(op, ">>")) {
        if (inteiro_a && inteiro_b) return "int";
        if (!strcmp(op, "|") && !strcmp(a, "dict") && !strcmp(b, "dict")) return "dict";
        return NULL;
    }
    return NULL;
}

static const char *tp_de(C *c, Unidade *u, PSNode *n)
{
    if (!n) return NULL;
    switch (n->kind) {
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
            return NULL;
        }
        case N_BINARY_OP: return tp_binario(c, n->texto, tp_de(c, u, n->a), tp_de(c, u, n->b));
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
    if (!n) return 0;
    if (n->kind == N_BREAK_STMT) return 1;
    if (n->kind == N_WHILE_STMT || n->kind == N_FOR_EACH_STMT || n->kind == N_COUNT_EACH_STMT
            || n->kind == N_ACTION_DECL || n->kind == N_LAMBDA_EXPR) return 0;
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
static void tp_grava_import(C *c, PSNode *n, const char *mod, const char *membro)
{
    memset(&c->grava, 0, sizeof(c->grava));
    c->grava.no = n;
    if (!mod || n->i2 != 0 || !ps_nativo_eh_modulo(mod)) return;
    const char *m = tp_guarda(c, mod);
    if (!membro) {
        c->grava.tem_valor = 1;
        c->grava.tipo = "module";
        c->grava.mod_nativo = m;
        return;
    }
    int k = ps_nativo_tem_membro(mod, membro);
    if (k == 0) {
        terro(c, n, "ImportError", "cannot import name '%s' from '%s' (unknown location)", membro, mod);
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
        memset(s, 0, sizeof(*s));
        s->estado = 1;
        s->decorado = 1;
        return T_SIM;
    }
    if (c->grava.declara) {
        const char *T = tp_canon(c->grava.declara);
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
    }
    if (c->grava.decl) { s->decl = c->grava.decl; s->decorado = c->grava.decorado; }
    if (c->grava.mod_nativo) { s->mod_nativo = c->grava.mod_nativo; s->membro_nativo = c->grava.membro_nativo; }
    if (!c->grava.tem_valor) {
        if (s->estado == 0) { s->estado = 1; s->tipo = NULL; }
        return (s->estado == 2 || s->tipo) ? T_TALVEZ : T_SIM;
    }
    const char *vt = c->grava.tipo;
    if (s->estado == 0) {
        if (vt && strcmp(vt, "Null") == 0) return T_SIM;
        s->estado = 1;
        s->tipo = vt;
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
        if (d->lista.itens[i] != met) continue;
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
        if (m->kind != N_ACTION_DECL) continue;
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

/* `alvo.nome` existe? Módulo nativo, tipo nativo (pelas tabelas da VM),
 * Entity do arquivo (campo, método, `self.x` gravado, pais) e enum. O que não
 * se sabe — tipo desconhecido, pai importado — passa. */
static void tp_confere_membro(C *c, Unidade *u, PSNode *n)
{
    if (!n->texto || !n->a) return;
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
    if (n->a->kind == N_NAME && n->a->texto) {
        SimInfo *s = tp_sim_de(c, u, n->a->texto);
        if (s && s->decl && s->decl->kind == N_ENTITY_DECL && !s->decorado) {
            if (tp_classe_tem(c, s->decl->texto, n->texto, 0) == 0)
                terro(c, n, "AttributeError", "'%s' object has no attribute '%s'", s->decl->texto, n->texto);
            return;
        }
        if (s && s->decl && s->decl->kind == N_ENUM_DECL) {
            if (!strcmp(n->texto, "type")) return;
            for (int32_t i = 0; i < s->decl->lista.n; i++)
                if (s->decl->lista.itens[i]->texto && !strcmp(s->decl->lista.itens[i]->texto, n->texto)) return;
            terro(c, n, "AttributeError", "type object '%s' has no attribute '%s'",
                  s->decl->texto ? s->decl->texto : "?", n->texto);
            return;
        }
    }
    const char *t = tp_de(c, u, n->a);
    if (!t) return;
    /* união: só é erro se NENHUM lado (fora o Null) tem o membro */
    int lados = 0;
    const char *q = t;
    while (q && *q) {
        const char *bar = strchr(q, '|');
        size_t len = bar ? (size_t)(bar - q) : strlen(q);
        char lado[128];
        if (len >= sizeof(lado)) return;
        memcpy(lado, q, len); lado[len] = '\0';
        q = bar ? bar + 1 : NULL;
        if (!strcmp(lado, "Null")) continue;
        PSNode *d = tp_tipo_arq(c, lado);
        int r = (d && d->kind == N_ENTITY_DECL) ? tp_classe_tem(c, lado, n->texto, 0)
              : d ? -1 : ps_nativo_tem_membro(lado, n->texto);
        if (r != 0) return;
        lados++;
    }
    if (lados > 0)
        terro(c, n, "AttributeError", "'%s' object has no attribute '%s'", t, n->texto);
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
        if (achou >= 0) { marcado[achou] = 1; valor[achou] = a->a; continue; }
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
static void tp_confere_chamada(C *c, Unidade *u, PSNode *n)
{
    PSNode *f = n->a;
    if (!f) return;
    if (f->kind == N_NAME && f->texto) {
        SimInfo *s = tp_sim_de(c, u, f->texto);
        if (!s || !s->decl || s->decorado) return;
        if (s->decl->kind == N_ACTION_DECL) {
            tp_confere_args(c, u, n, s->decl->texto ? s->decl->texto : "?", &s->decl->lista, 0, 0, NULL);
            return;
        }
        if (s->decl->kind != N_ENTITY_DECL || !s->decl->texto) return;
        const char *cls = s->decl->texto;
        PSNode *init = tp_metodo(c, cls, "__init__", 0);
        if (init) {
            if (tp_metodo_decorado(c, cls, init)) return;
            tp_confere_args(c, u, n, "__init__", &init->lista, 1, 0, NULL);
            return;
        }
        /* `__init__` gerado: um parâmetro por campo de instância, da própria
         * classe — com pai, os campos dele entram por outro caminho */
        if (s->decl->lista2.n == 0 && s->decl->lista2_alias.n > 0)
            tp_confere_args(c, u, n, "__init__", &s->decl->lista2_alias, 1, 0, cls);
        return;
    }
    if (f->kind != N_MEMBER_ACCESS || !f->texto || !f->a) return;
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
    if (!t || strchr(t, '|') || !tp_tipo_arq(c, t)) return;
    PSNode *m = tp_metodo(c, t, f->texto, 0);
    if (!m || tp_metodo_decorado(c, t, m) || tp_metodo_estatico(c, t, m)) return;
    if (tp_campo_tipo(c, t, f->texto, 0)) return;          /* campo ganha de método */
    tp_confere_args(c, u, n, m->texto ? m->texto : "?", &m->lista, 1, 0, NULL);
}

/* A chamada é SAÍDA (`post`, `sys.stdout.write`, `f.write` num PoolFile,
 * `os.writeFile`, `os.warn`)? Devolve o índice do posicional que sai (-2 =
 * todos), ou -1 se não é saída. É o que a funct tipada confere: ela só dá
 * saída do tipo dela. */
static int tp_arg_de_saida(C *c, Unidade *u, PSNode *call)
{
    PSNode *f = call->a;
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

/* Funct tipada: os argumentos de saída da chamada `n`, antes de rodar. O que
 * só se sabe rodando fica marcado em `c->saida_*` pro `emite_args_e_chama`
 * emitir a conferência logo depois de avaliar cada argumento. */
static void tp_confere_saida(C *c, Unidade *u, PSNode *n)
{
    c->saida_tipo = NULL;
    if (!u->tipo_ret_nome) return;
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
        for (int32_t i = 0; i < q->nlocais; i++)
            if (strcmp(q->locais[i], nome) == 0) return &q->sim[i];
    return NULL;
}

static void guarda_nome_modo_no(C *c, Unidade *u, const char *nome, int certa)
{
    if (u->eh_modulo) {
        mod_criados_add(c, u, nome);  /* p/ escopo de bloco */
        SimInfo *s = NULL;
        for (int32_t k = u->n_mod_criados - 1; k >= 0; k--)
            if (strcmp(u->mod_criados[k], nome) == 0) { s = &u->mod_sim[k]; break; }
        int v = tp_escreve(c, s, nome);
        tp_emite_escrita(c, u, nome, s, v, tp_cod1(s, tipo_topo_de(c, nome)));
        emite(c, u, OP_STORE_GLOBAL, idx_global(c, nome));
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
     * escreve lá"), agora valendo também pro escopo da função que envolve. */
    {
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

static void compila_fstring(C *c, Unidade *u, PSNode *n)
{
    const char *t = n->texto ? n->texto : "";
    /* `texto_len`, não strlen: um `\x00` no meio da f-string é dado, não fim
     * de string — com strlen o resto do texto sumia calado. */
    int32_t len = n->texto_len > 0 ? n->texto_len : (int32_t)strlen(t);
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
            if (prof != 0) { free(buf); cerro_sx(c, n, "chave nao fechada na f-string"); return; }

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
            PSNode *alvo = (st->kind == N_EXPRESSION_STMT) ? st->a : st;
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

static void expr_no(C *c, Unidade *u, PSNode *n)
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
                     * de ausencia, e `type(null)` e
                     * literalmente "Null" — ali o literal ocupa a posicao de
                     * tipo com sentido. Os outros literais nao tem essa
                     * leitura. */
                    if (n->b && n->b->kind == N_LITERAL && n->b->lit != L_NULL) {
                        static const char *NOME_LIT[] = {
                            "int", "flo", "str", "bool", "Null", "str", "int", "byte"
                        };
                        const char *tn = (n->b->lit >= 0 && n->b->lit <= L_BYTES)
                                         ? NOME_LIT[n->b->lit] : "?";
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

        case N_CALL:
            tp_confere_chamada(c, u, n);
            expr(c, u, n->a);
            /* depois do chamado: ele pode ter chamadas dentro, e a marca de
             * saída é só desta */
            tp_confere_saida(c, u, n);
            emite_args_e_chama(c, u, n, &n->lista);
            return;

        case N_LIST_LITERAL:
            for (int32_t i = 0; i < n->lista.n; i++) expr(c, u, n->lista.itens[i]);
            emite(c, u, OP_BUILD_LIST, n->lista.n);
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

            /* o nome da var da compreensão SOMBREIA, como no `for each` */
            char salvo[128];
            int sombreia = nome_ja_existe(u, var);
            SimInfo sim_fora;
            int sim_posto = 0;
            if (sombreia) {
                snprintf(salvo, sizeof(salvo), "  lcv$%s", var);
                carrega_nome(c, u, var);
                guarda_nome_modo(c, u, salvo, 1);
                sim_posto = tp_sombra_poe(c, u, var, &sim_fora);
            }
            int32_t M = escopo_marca(u);

            expr(c, u, n->a);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 0, 0, NULL, 0));
            int32_t topo = UP(c, u)->ncode;
            int32_t fim = emite(c, u, OP_ITER_NEXT, 0);
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.tem_valor = 1;
            c->grava.tipo = tp_elemento(tp_de(c, u, n->a));
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
            if (sombreia) {
                carrega_nome(c, u, salvo);
                tp_sombra_devolve(c, u, var, sim_posto, &sim_fora);
                guarda_nome_modo(c, u, var, 1);
            }
            carrega_nome(c, u, acc);
            return;
        }

        case N_DICT_LITERAL:
            /* A chave é expressão (parser): `{k: 1}` carrega a variável `k`,
             * e nome que o arquivo não liga é NameError antes de rodar, como
             * em qualquer expressão. Antes o Name virava a string "k" aqui. */
            for (int32_t i = 0; i < n->lista.n; i++) {
                PSNode *e = n->lista.itens[i];
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
            /* Mesma máquina da action nomeada: um protótipo e um
             * MAKE_FUNCTION. A diferença é só não ter nome pra guardar. */
            int32_t pi = compila_action(c, n, u);
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

        case N_MEMBER_ACCESS:
            tp_confere_membro(c, u, n);
            expr(c, u, n->a);
            emite(c, u, OP_GET_MEMBER,
                  idx_const(c, u, K_STR, 0, 0, n->texto ? n->texto : "",
                            n->texto ? (int32_t)strlen(n->texto) : 0));
            return;

        case N_AWAIT_EXPR:
            /* `await expr` — avalia a expr (que dá um future) e resolve. */
            expr(c, u, n->a);
            emite(c, u, OP_AWAIT, 0);
            return;

        case N_BASE_CALL_NODE: {
            /* `base(v)` chama o __init__ do PRIMEIRO pai com o self atual.
             * Emite a classe pai explicitamente: buscar pela instância
             * acharia o override da filha e recursaria pra sempre. */
            const char *pai = n->texto2 ? n->texto2 : c->entity_pai;
            if (!pai) {
                /* `base()` numa Entity SEM herança era um no-op calado: quem
                 * escreveu isso errou (não há pai pra inicializar) e o erro
                 * ficava escondido. Agora fala. */
                cerro_sx(c, n, "%s", c->dentro_entity
                             ? "base() numa Entity sem heranca: nao ha pai pra inicializar"
                             : "base() fora de Entity com heranca");
                return;
            }
            /* `base(...)` passa o `self` do frame ao `__init__` do pai (o
             * OP_LOAD_SELF lê o slot 0). Numa funct cujo 1º parâmetro não é
             * `self` — `funct __init__(*args)` — o slot 0 é outra coisa, e o
             * pai inicializava a tup: `self.a = a` dava "'tup' object has no
             * attribute 'a'". Sem self não há objeto pra entregar. */
            if (u->eh_modulo || u->nlocais == 0 || strcmp(u->locais[0], "self") != 0) {
                cerro_sx(c, n, "base() precisa do self: declare `funct __init__(self, ...)` "
                               "(o self e o objeto que o pai inicializa)");
                return;
            }
            int32_t nkw_b = 0;
            for (int32_t i = 0; i < n->lista.n; i++)
                if (n->lista.itens[i]->texto || n->lista.itens[i]->i2) nkw_b++;
            if (nkw_b == 0) {
                carrega_nome(c, u, pai);
                emite(c, u, OP_LOAD_SELF, 0);
                for (int32_t i = 0; i < n->lista.n; i++)
                    expr(c, u, n->lista.itens[i]->a);
                emite(c, u, OP_CALL_BASE, n->lista.n);
                return;
            }
            /* Com argumento NOMEADO ou espalhado (`base(x=5)`, `base(**kw)`),
             * em vez de repetir aqui toda a resolução de nome/default do
             * OP_CALL_KW, o `__init__` do pai é carregado LIGADO ao self e a
             * chamada segue o caminho normal. */
            carrega_nome(c, u, pai);
            emite(c, u, OP_LOAD_BASE_INIT, 0);
            emite_args_e_chama(c, u, n, &n->lista);
            return;
        }

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
    l->nsaidas = 0;
    l->ncontinues = 0;
    l->slots_pilha = slots_pilha;
    return 0;
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


/* Emite os blocos `finally` pendentes, do mais interno pro mais externo, que
 * a saída atual está atravessando. `ate_laco` < 0 = todos da função (return);
 * >= 0 = só os que estão DENTRO do laço indicado (break/continue). */
static void emite_finallys(C *c, Unidade *u, int ate_laco)
{
    for (int i = c->nfinally - 1; i >= 0; i--) {
        if (c->fin_unidade[i] != (const void *)u) break;   /* outra função */
        if (ate_laco >= 0 && c->fin_laco[i] < ate_laco) break;
        PSNode *bloco = c->fin_bloco[i];
        if (!bloco) continue;
        int32_t M = escopo_marca(u);
        bloco_stmts(c, u, bloco);
        escopo_fecha(c, u, M);
    }
}

static void stmt_no(C *c, Unidade *u, PSNode *n);

/* mesma regra do `expr` */
static void stmt(C *c, Unidade *u, PSNode *n)
{
    int32_t l = c->linha_atual, co = c->coluna_atual;
    stmt_no(c, u, n);
    c->linha_atual = l; c->coluna_atual = co;
}

static void stmt_no(C *c, Unidade *u, PSNode *n)
{
    if (CFALHOU(c) || !n) return;
    if (n->line) c->linha_atual = n->line;
    if (n->col)  c->coluna_atual = n->col;

    switch (n->kind) {
        case N_ACTION_DECL: {
            /* O nome nasce ANTES do corpo compilar: sem isto uma action
             * aninhada não conseguiria chamar a si mesma (o nome dela ainda
             * não seria local da função de fora na hora da captura). */
            if (!u->eh_modulo && n->texto) {
                int32_t si = idx_local(c, u, n->texto);
                if (si >= 0 && !u->celula[si]) u->certo[si] = 1;
            }
            int32_t idx = compila_action(c, n, u);
            if (CFALHOU(c)) return;
            emite_funcao(c, u, idx);
            if (u->eh_modulo && n->is_private) priv_global_add(c, n->texto);
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.tem_valor = 1;
            c->grava.tipo = "funct";
            c->grava.no = n;
            c->grava.decl = n;
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
            return;
        }

        case N_FIELD_DECL: {
            /* `private str name = nome` dentro da action: é CAMPO DO OBJETO,
             * não local. Escreve em `self` pelo mesmo caminho de
             * `self.name = nome` — um só lugar decide dict×instância e o
             * corte de private. A visibilidade em si é registrada na CLASSE,
             * no N_ENTITY_DECL, senão o `private` compilaria sem barrar nada. */
            if (!c->dentro_entity || u->eh_modulo || !nome_ja_existe(u, "self")) {
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
                const char *tv = tp_binario(c, base, (sx && sx->estado) ? sx->tipo : NULL, tp_de(c, u, n->a));
                memset(&c->grava, 0, sizeof(c->grava));
                c->grava.tem_valor = 1;
                c->grava.tipo = tv;
                c->grava.no = n;
            } else {
                expr(c, u, n->a);
                grava_valor(c, u, n->a);
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
            /* funct tipada: todo `return` devolve o tipo dela */
            if (u->tipo_ret_nome && !(!n->a && c->dentro_count_each > 0)) {
                if (!n->a) {
                    if (!u->tipo_ret)
                        terro(c, n, "AttributedValueError",
                              "%s() devolve %s, e `return` sem valor devolveria Null",
                              u->nome_funct ? u->nome_funct : "?", u->tipo_ret_nome);
                } else if (!(u->tipo_ret && n->a->kind == N_LITERAL && n->a->lit == L_NULL)) {
                    /* `return null` numa `int`/`bool funct` é o `return` seco:
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
            emite_finallys(c, u, -1);   /* todos os finally abertos nesta função */
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
             * nao sombrear nenhum. */
            int sombreia = !n->e && nome_ja_existe(u, var_laco);
            SimInfo sim_fora;
            int sim_posto = 0;
            if (sombreia) {
                snprintf(salvo, sizeof(salvo), "  fe$%s", var_laco);   /* nome não digitável */
                carrega_nome(c, u, var_laco);
                guarda_nome_modo(c, u, salvo, 1);
                sim_posto = tp_sombra_poe(c, u, var_laco, &sim_fora);
            }
            int32_t M = escopo_marca(u);        /* marca ANTES da var do laço */
            /* `for each i in range(a, b, p)`: em vez de materializar a lista
             * inteira (20 milhões de itens = 320 MB), empilha ini/fim/passo e
             * conta. Só entra quando o nome `range` não é ligado em lugar
             * nenhum do programa — senão o `range` do usuário é que vale. */
            int usa_range = 0;
            if (n->a && n->a->kind == N_CALL && n->a->a
                    && n->a->a->kind == N_NAME && n->a->a->texto
                    && !strcmp(n->a->a->texto, "range")
                    && n->a->lista.n >= 1 && n->a->lista.n <= 3
                    && !liga_o_nome(c, c->raiz, "range")) {
                usa_range = 1;
                /* nomeado ou espalhado (`range(*l)`) não: o atalho lê cada
                 * argumento como um limite, e a lista inteira virava o fim */
                for (int32_t k = 0; k < n->a->lista.n; k++)
                    if (n->a->lista.itens[k]->texto || n->a->lista.itens[k]->i2) { usa_range = 0; break; }
            }
            int32_t topo, fim;
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
                topo = UP(c, u)->ncode;
                fim = emite(c, u, OP_ITER_RANGE, 0);
            } else {
            expr(c, u, n->a);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 0, 0, NULL, 0));
            topo = UP(c, u)->ncode;
            fim = emite(c, u, OP_ITER_NEXT, 0);
            }
            /* variável do laço é local desta função, como o parâmetro.
             * Com `for each a, b in ...`, o item que o ITER_NEXT deixou no
             * topo e desempacotado pelo MESMO emissor do `a, b = [1, 2]` —
             * mesma checagem de quantidade, mesma mensagem de erro. */
            if (n->e) guarda_em_alvo(c, u, n->e);
            else {
                memset(&c->grava, 0, sizeof(c->grava));
                c->grava.tem_valor = 1;
                c->grava.tipo = usa_range ? "int" : tp_elemento(tp_de(c, u, n->a));
                c->grava.no = n;
                guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            }
            if (abre_laco(c, n, topo, usa_range ? 4 : 2) != 0) return;   /* estado do laço na pilha */
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
            escopo_trunca(c, u,M);
            if (sombreia) {                      /* devolve o valor de fora */
                carrega_nome(c, u, salvo);
                tp_sombra_devolve(c, u, var_laco, sim_posto, &sim_fora);
                guarda_nome_modo(c, u, var_laco, 1);
            }
            return;
        }

        case N_UNPACK_ASSIGNMENT: {
            /* alvo aninhado `a, (b, c) = ...` desempacota de novo por alvo */
            PSNode *alvo = n->a;
            if (!alvo || alvo->kind != N_UNPACK_TARGET) { cerro_sx(c, n, "alvo de desempacotamento invalido"); return; }
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
                /* a coluna `model` da tabela única decide; o parser já barrou
                 * o resto com a frase do tipo de campo */
                const PSTipoInfo *ti = ps_tipo_info(f->texto2);
                int32_t t = (ti && ti->model) ? ti->cod : -1;
                if (t < 0) { cerro_sx(c, f, "tipo desconhecido em model"); return; }
                def->campos[i].tipo = t;
            }
            emite(c, u, OP_MAKE_MODEL, mi);
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.decl = n;
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
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.decl = n;
            guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
            return;
        }

        case N_DECORATOR_STMT: {
            /* Três decoradores são resolvidos em COMPILAÇÃO (`static`,
             * `NonNull`, `dataentity`): mudam como a action é gerada. Todo
             * OUTRO decorador é o protocolo geral, em runtime (OP_DECORA):
             * `@obj.metodo(args)`, `@log`, `@log()` — inclusive nome que não
             * existe, que dá NameError na linha do `@` em vez de sumir calado
             * com a funct embaixo (era o que acontecia). */
            PSNode *dec = n->a;
            const char *nome = (dec && dec->lista.n == 1) ? dec->lista.itens[0]->texto : NULL;
            int embutido = nome && (strcmp(nome, "dataentity") == 0
                                    || strcmp(nome, "static") == 0
                                    || strcmp(nome, "NonNull") == 0);
            if (nome && strcmp(nome, "dataentity") == 0) {
                if (n->b) bloco_stmts(c, u, n->b);
                return;
            }
            /* Decorador GERAL: avalia a expressão -> o decorador (guardado);
             * roda o bloco (define a action); e entrega a action ao OP_DECORA,
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
                 * nome já envolvido — a action fica no fundo da pilha de @ */
                while (corpo && corpo->kind == N_BLOCK && corpo->lista.n == 1
                       && corpo->lista.itens[0]->kind == N_DECORATOR_STMT)
                    corpo = corpo->lista.itens[0]->b;
                if (corpo && corpo->kind == N_BLOCK)
                    for (int32_t i = 0; i < corpo->lista.n; i++) {
                        PSNode *bi = corpo->lista.itens[i];
                        if (bi->kind == N_ACTION_DECL) { act = bi->texto; break; }
                        /* handler baseado em CLASSE: acha a action DENTRO da
                         * classe (a primeira fora de __init__), nome qualquer */
                        if (bi->kind == N_ENTITY_DECL) {
                            cls_nome = bi->texto;
                            for (int32_t j = 0; j < bi->lista.n; j++) {
                                PSNode *m = bi->lista.itens[j];
                                if (m->kind == N_ACTION_DECL && m->texto
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
                /* 2) guarda o decorador e roda o bloco (define a action; um
                 * decorador empilhado ali dentro usa o próprio temporário) */
                guarda_nome_modo(c, u, reg, 1);
                c->decor_prof++;
                bloco_stmts(c, u, n->b);
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
                /* Marca a action decorada como estática (chamável na Entity
                 * sem instância). Fora de Entity, a marca é inofensiva: só
                 * vale na resolução de `Classe.metodo`. */
                c->pendente_static = 1;
                bloco_stmts(c, u, n->b);
                c->pendente_static = 0;
                return;
            }
            if (nome && strcmp(nome, "NonNull") == 0) {
                c->pendente_nonnull = 1;
                bloco_stmts(c, u, n->b);
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
            expr(c, u, n->a);
            grava_valor(c, u, n->a);
            guarda_nome_modo(c, u, n->texto ? n->texto : "_", 1);
            int32_t setup = emite(c, u, OP_SETUP_TRY, 0);
            c->dentro_try++;
            bloco_stmts(c, u, n->b);
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

            /* nomes de membros `private` — métodos (n->lista) + campos
             * (n->lista2_alias) + os campos declarados DENTRO das actions
             * (`private str name = nome` no __init__). Estes últimos moram no
             * corpo do método, mas a visibilidade é da CLASSE: sem varrer por
             * eles aqui, o `private` compilaria e não barraria nada — pior que
             * não ter encapsulamento, porque parece que tem. */
            int32_t npriv_corpo = 0;
            for (int32_t i = 0; i < n->lista.n; i++)
                npriv_corpo = varre_campos_priv(n->lista.itens[i], NULL, npriv_corpo);
            int32_t npriv_max = n->lista.n + n->lista2_alias.n + npriv_corpo;
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
                for (int32_t i = 0; i < n->lista.n; i++)
                    def->npriv = varre_campos_priv(n->lista.itens[i], def->priv_nomes, def->npriv);
            }

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
                if (n->lista.itens[i]->kind == N_ACTION_DECL) nm++;
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
            tp_self_dos_metodos(c, n);   /* classe que a pré-passada não viu (aninhada) */
            /* Dentro de Entity o decorador é uma entrada SEPARADA do corpo
             * (dec_sem_captura no parser): ele não embrulha a action. Então o
             * `@static` visto aqui vale pra PRÓXIMA action da lista — é assim
             * que a marca chega no Proto (Proto.eh_static). */
            int static_pendente = 0, nonnull_pendente = 0;
            /* Decorador GERAL (`@mapp.post("/x")`) em cima de um método: era
             * DESCARTADO aqui — o `continue` abaixo pulava o nó, o método
             * compilava sem registro nenhum e a rota nunca existia, calada.
             * Os pares (decorador, método) ficam guardados e são emitidos
             * DEPOIS de a classe existir, porque a expressão do decorador pode
             * ler um campo `static` que só nasce então. */
            PSNode *dec_pend[8];  int ndec_pend = 0;
            PSNode *par_dec[64];  PSNode *par_met[64];  int npares = 0;
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
                        if (ndec_pend >= 8) { cerro(c, "limite do compilador: 8 decoradores num metodo", m); return; }
                        dec_pend[ndec_pend++] = dec;
                    }
                    continue;
                }
                if (m->kind != N_ACTION_DECL) continue;
                for (int k = 0; k < ndec_pend && npares < 64; k++) {
                    par_dec[npares] = dec_pend[k];
                    par_met[npares] = m;
                    npares++;
                }
                ndec_pend = 0;
                c->pendente_static = static_pendente;
                c->pendente_nonnull = nonnull_pendente;
                int32_t pi = compila_action(c, m, NULL);
                c->pendente_static = 0;
                c->pendente_nonnull = 0;
                static_pendente = 0;
                nonnull_pendente = 0;
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
                    if (!so_inst) { cerro(c, "sem memoria", n); return; }
                    int32_t k = 0;
                    for (int32_t i = 0; i < todos.n; i++)
                        if (!todos.itens[i]->is_static) so_inst[k++] = todos.itens[i];
                    n->lista2_alias.itens = so_inst;
                    n->lista2_alias.n = n_inst;
                    int32_t pi = sintetiza_init(c, n);
                    n->lista2_alias = todos;
                    free(so_inst);
                    if (CFALHOU(c)) return;
                    def = &c->out->classes[ci];
                    /* Cada campo é PUBLICADO assim que o realloc dele dá certo.
                     * Guardar os dois pra publicar no fim parecia mais limpo e
                     * era liberação dupla: o realloc que dá certo já soltou o
                     * bloco antigo, então `free(mn)` no erro do SEGUNDO deixava
                     * `def->met_nomes` apontando pra memória morta — e o
                     * destrutor da classe passa lá liberando de novo. */
                    char **mn = realloc(def->met_nomes, sizeof(char *) * (size_t)(def->nmetodos + 1));
                    if (!mn) { cerro(c, "sem memoria", n); return; }
                    def->met_nomes = mn;
                    int32_t *mp = realloc(def->met_protos, sizeof(int32_t) * (size_t)(def->nmetodos + 1));
                    if (!mp) { cerro(c, "sem memoria", n); return; }
                    def->met_protos = mp;
                    def->met_nomes[def->nmetodos] = strdup("__init__");
                    def->met_protos[def->nmetodos] = pi;
                    def->nmetodos++;
                }
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
            c->grava.decl = n;
            guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);

            /* A classe existe. Agora, nesta ordem:
             *   1) campos `static` — `Classe.x = <inicializador>` (ou Null);
             *   2) decoradores dos métodos — a expressão do decorador pode
             *      ler esses campos (`@mapp.post(...)`), por isso vem depois.
             * Tudo com `entity_no` ainda apontando pra esta classe: é o que
             * faz `mapp` virar `App.mapp` dentro dessas expressões. */
            const char *cls_nome_aqui = n->texto ? n->texto : "";
            for (int32_t i = 0; i < n->lista2_alias.n && !CFALHOU(c); i++) {
                PSNode *f = n->lista2_alias.itens[i];
                if (!f->is_static || !f->texto) continue;
                carrega_nome(c, u, cls_nome_aqui);
                if (f->a) expr(c, u, f->a);
                else emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_NULL, 0, 0, NULL, 0));
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
                if (fim - ini > 8) { cerro(c, "limite do compilador: 8 decoradores num metodo", met); return; }
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
                            return;
                        }
                    }
                    emite_decorador_expr(c, u, dec);
                    guarda_nome_modo(c, u, DM[q - ini], 1);
                }
                int32_t kmet = idx_const(c, u, K_STR, 0, 0, mn, (int32_t)strlen(mn));
                carrega_nome(c, u, cls_nome_aqui);
                emite(c, u, OP_LOAD_METODO, kmet);
                guarda_nome_modo(c, u, "$dmval", 1);
                for (int q = fim - 1; q >= ini && !CFALHOU(c); q--) {
                    carrega_nome(c, u, cls_nome_aqui);
                    emite(c, u, OP_LOAD_CONST, kmet);
                    carrega_nome(c, u, DM[q - ini]);
                    carrega_nome(c, u, "$dmval");
                    emite_decora(c, u, par_dec[q], 1);
                    guarda_nome_modo(c, u, "$dmval", 1);
                }
                carrega_nome(c, u, cls_nome_aqui);
                carrega_nome(c, u, "$dmval");
                emite(c, u, OP_SET_METODO, kmet);
            }
            c->entity_no = no_salvo;
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
                for (int32_t k = 0; k < c->nestrelas; k++) if (c->estrelas[k].no == n) { e = k; break; }
                int32_t cmod = idx_const(c, u, K_STR, 0, 0, mod, (int32_t)strlen(mod));
                emite(c, u, OP_IMPORT_MOD, cmod);
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
                    emite(c, u, OP_IMPORT_MOD, idx_const(c, u, K_STR, 0, 0, mod, (int32_t)strlen(mod)));
                    tp_grava_import(c, n, mod, NULL);
                    guarda_nome(c, u, n->texto2 ? n->texto2 : ultimo);
                    return;
                }
                for (int32_t i = 0; i < n->lista2.n; i++) {
                    const char *membro = n->lista2.itens[i]->texto;
                    const char *apelido = (i < n->lista2_alias.n && n->lista2_alias.itens[i])
                                        ? n->lista2_alias.itens[i]->texto : membro;
                    emite(c, u, OP_IMPORT_MOD, idx_const(c, u, K_STR, 0, 0, mod, (int32_t)strlen(mod)));
                    emite(c, u, OP_IMPORT_FROM, idx_const(c, u, K_STR, 0, 0, membro, (int32_t)strlen(membro)));
                    tp_grava_import(c, n, mod, membro);
                    guarda_nome(c, u, apelido);
                }
                return;
            }

            if (strcmp(n->texto, "import") == 0) {
                if (!simples) { cerro_sx(c, n, "import de modulo pontuado/relativo precisa de 'from ... import ...'"); return; }
                emite(c, u, OP_IMPORT_MOD, idx_const(c, u, K_STR, 0, 0, mod, (int32_t)strlen(mod)));
                tp_grava_import(c, n, mod, NULL);
                guarda_nome(c, u, n->texto2 ? n->texto2 : ultimo);
                return;
            }
            /* `from mod import a, b as c` — um GET_MEMBER por nome pedido */
            for (int32_t i = 0; i < n->lista2.n; i++) {
                const char *membro = n->lista2.itens[i]->texto;
                const char *apelido = (i < n->lista2_alias.n && n->lista2_alias.itens[i])
                                    ? n->lista2_alias.itens[i]->texto : membro;
                emite(c, u, OP_IMPORT_MOD, idx_const(c, u, K_STR, 0, 0, mod, (int32_t)strlen(mod)));
                emite(c, u, OP_IMPORT_FROM, idx_const(c, u, K_STR, 0, 0, membro, (int32_t)strlen(membro)));
                tp_grava_import(c, n, mod, membro);
                guarda_nome(c, u, apelido);
            }
            return;
        }

        case N_MEMBER_ASSIGNMENT: {
            /* Entity do arquivo: o campo tem que existir nela (declarado no
             * corpo, `private <tipo> x` ou gravado em `self.x` por um método),
             * e campo tipado recebe o tipo dele. A VM confere o tipo de novo
             * rodando — é o que pega o `obj` que só se sabe rodando. */
            {
                const char *t = tp_de(c, u, n->a);
                PSNode *d = (t && !strchr(t, '|')) ? tp_tipo_arq(c, t) : NULL;
                if (d && d->kind == N_ENTITY_DECL && n->texto) {
                    if (tp_classe_tem(c, t, n->texto, 0) == 0)
                        terro(c, n, "AttributeError", "'%s' object has no attribute '%s'", t, n->texto);
                    const char *ft = tp_campo_tipo(c, t, n->texto, 0);
                    if (ft && (!n->texto2 || !strcmp(n->texto2, "="))) {
                        const char *vt = tp_de(c, u, n->b);
                        if (tp_aceita(c, ft, vt, n->b, 1) == T_NAO)
                            terro(c, n->b, "AttributedValueError", "campo %s de %s esperava %s, recebeu %s",
                                  n->texto, t, ft, vt);
                    }
                }
            }
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
                if (op < 0) { cerro_sx(c, n, "operador de atribuicao invalido"); return; }
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
                /* Os TRES caminhos (casou e rodou o corpo; padrão falhou;
                 * guarda falhou) convergem num ponto só, e a limpeza das
                 * variáveis do case (`escopo_fecha`) roda ali, pra todos.
                 *
                 * Antes a limpeza só ficava no caminho de sucesso. Numa
                 * action, `case v if v < 50` com a guarda falsa deixava o
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
                if (nfins < 64) fins[nfins++] = emite(c, u, OP_JUMP_IF_TRUE, 0);
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
            /* enquanto o corpo (e os catches) compilam, este `finally` fica
             * PENDENTE: `return`/`break`/`continue` lá dentro emitem o bloco
             * antes de saltar, senão ele não roda nesses caminhos. */
            int fin_meu = -1;
            if (c->nfinally < 32) {
                fin_meu = c->nfinally;
                c->fin_bloco[fin_meu]   = n->c;      /* pode ser NULL (sem finally) */
                c->fin_laco[fin_meu]    = c->nlacos;
                c->fin_unidade[fin_meu] = (const void *)u;
                c->nfinally++;
            }
            int32_t Mt = escopo_marca(u);
            bloco_stmts(c, u, n->a);
            escopo_fecha(c, u, Mt);            /* vars do try não vazam (saída normal) */
            c->dentro_try--;
            emite(c, u, OP_POP_TRY, 0);
            int32_t pula_catches = emite(c, u, OP_JUMP, 0);

            if (setup >= 0) UP(c, u)->code[setup + 1] = UP(c, u)->ncode;

            /* Erro levantado DENTRO de um catch também tem que passar pelo
             * finally. O bloco inline só cobre a saída normal e o "nenhum
             * catch casou"; um `raise` no corpo do catch desenrolava por cima
             * dele. Um try interno segura esse caso e repropaga. */
            int32_t setup_cat = -1;
            if (n->c) setup_cat = emite(c, u, OP_SETUP_TRY, 0);

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
                    /* EXC_CASA, nao EQ: o catch pergunta "este tipo E do tipo
                     * pedido?", e a resposta vem da tabela de excecoes, entao
                     * o pai pega o filho. Com EQ, `catch (Exception e)` nao
                     * pegava nada. */
                    emite(c, u, OP_EXC_CASA, 0);
                    prox_falha = emite(c, u, OP_JUMP_IF_FALSE, 0);
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
                if (nfins < 32) fins[nfins++] = emite(c, u, OP_JUMP, 0);
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
            if (prox_falha >= 0 || n->lista.n == 0) {
                if (prox_falha >= 0) UP(c, u)->code[prox_falha + 1] = UP(c, u)->ncode;
                /* tipo junto da mensagem: o `finally` roda antes do RERAISE e
                 * pode ter trocado o erro corrente da VM */
                if (setup_cat >= 0) emite(c, u, OP_POP_TRY, 0);   /* não caia no próprio handler */
                emite(c, u, OP_PUSH_ERR_TYPE, 0);
                if (n->c) { int32_t Mf = escopo_marca(u); bloco_stmts(c, u, n->c); escopo_fecha(c, u, Mf); }
                emite(c, u, OP_RERAISE, 0);
            }

            if (fin_meu >= 0) c->nfinally = fin_meu;   /* sai de pendente */

            int32_t pula_handler = -1;
            if (setup_cat >= 0) {
                /* fim normal de um catch: fecha o try interno e segue */
                int32_t pos_pop = UP(c, u)->ncode;
                emite(c, u, OP_POP_TRY, 0);
                pula_handler = emite(c, u, OP_JUMP, 0);
                for (int k = 0; k < nfins; k++) UP(c, u)->code[fins[k] + 1] = pos_pop;
                nfins = 0;
                /* handler: erro veio de dentro de um catch -> finally + repropaga */
                UP(c, u)->code[setup_cat + 1] = UP(c, u)->ncode;
                emite(c, u, OP_PUSH_ERR_TYPE, 0);
                { int32_t Mf = escopo_marca(u); bloco_stmts(c, u, n->c); escopo_fecha(c, u, Mf); }
                emite(c, u, OP_RERAISE, 0);
            }

            int32_t fim_catches = UP(c, u)->ncode;
            if (pula_catches >= 0) UP(c, u)->code[pula_catches + 1] = fim_catches;
            if (pula_handler >= 0) UP(c, u)->code[pula_handler + 1] = fim_catches;
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
            if (c->nlacos == 0) { cerro_sx(c, n, "'break' fora de laco"); return; }
            Laco *l = &c->lacos[c->nlacos - 1];
            if (l->nsaidas >= MAX_SAIDAS) { cerro(c, "limite do compilador: 'break' demais no mesmo laco", n); return; }
            /* saindo do laço: apaga TUDO nascido nele até aqui (var do laço +
             * corpo + blocos aninhados abertos), pra nada vazar pra fora */
            emite_finallys(c, u, c->nlacos);   /* finally aberto DENTRO deste laço */
            escopo_emite_clears(c, u, l->escopo_marca);
            /* limpa o estado do iterador antes de sair do laço */
            for (int k = 0; k < l->slots_pilha; k++) emite(c, u, OP_POP_TOP, 0);
            l->saidas[l->nsaidas++] = emite(c, u, OP_JUMP, 0);
            return;
        }

        /* `pass` não emite nada: é o statement que existe só pra ocupar lugar.
         * Sem bytecode, sem efeito, sem custo. */
        case N_PASS_STMT:
            return;

        case N_CONTINUE_STMT: {
            if (c->nlacos == 0) { cerro_sx(c, n, "'continue' fora de laco"); return; }
            Laco *l = &c->lacos[c->nlacos - 1];
            if (l->ncontinues >= MAX_SAIDAS) { cerro(c, "limite do compilador: 'continue' demais no mesmo laco", n); return; }
            /* próxima iteração começa limpa: apaga só o escopo do CORPO (o que é
             * do laço — var/ self/_count — é re-atribuído no topo ou persiste) */
            emite_finallys(c, u, c->nlacos);   /* finally aberto DENTRO deste laço */
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
    p->slot_vararg = -1;
    p->slot_kwarg = -1;
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
            /* Mesma regra da action: com `a, b=2, c` o `ndefaults` (que conta
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

    vardbg_fecha(c, &u, 0);   /* o que chegou vivo ao fim vale até a última palavra */
    for (int32_t i = 0; i < u.nlocais; i++) free(u.locais[i]);
    free(u.locais);
    free(u.celula); free(u.celula_virgem); free(u.certo); free(u.tipo_decl);
    free(u.sim); free(u.mod_sim);
    for (int32_t i = 0; i < u.nligados; i++) free(u.ligados[i]);
    free(u.ligados);
    for (int32_t i = 0; i < u.n_mod_criados; i++) free(u.mod_criados[i]);
    free(u.mod_criados);
    for (int32_t i = 0; i < u.nglobais_decl; i++) free(u.globais_decl[i]);
    free(u.globais_decl);
    return idx;
}

static int32_t compila_action(C *c, PSNode *n, Unidade *pai)
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
    c->pendente_static  = 0;
    c->pendente_nonnull = 0;

    Unidade u;
    memset(&u, 0, sizeof(u));
    u.pai = (pai && !pai->eh_modulo) ? pai : NULL;
    u.idx = idx;
    u.eh_modulo = 0;
    if (n->texto2 && !strcmp(n->texto2, "int"))       u.tipo_ret = 1;
    else if (n->texto2 && !strcmp(n->texto2, "bool")) u.tipo_ret = 2;
    const char *nome_f = n->texto ? n->texto : "<funct>";
    u.nome_funct = nome_f;
    if (n->kind == N_ACTION_DECL && n->texto2) {
        u.tipo_ret_nome = tp_canon(n->texto2);
        if (!tp_tipo_existe(c, &u, n->texto2))
            terro(c, n, "AttributedValueError", "tipo %s não existe (retorno de %s())", n->texto2, nome_f);
        /* gerador devolve o gerador: o tipo escrito não é o do `return` */
        if (tp_tem_yield(n->b)) u.tipo_ret_nome = NULL;
    }
    /* método de Entity: o `self` é da classe — é o que confere `self.campo`
     * e `self.metodo()` antes de rodar */
    int self_da_classe = !pai && c->dentro_entity && c->entity_no && c->entity_no->texto;
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

    /* Células das variáveis que as actions aninhadas capturam. Vem depois do
     * prólogo (que testa o slot cru com JUMP_IF_SET) e antes do corpo. */
    marca_celulas(c, &u, n, n->b);

    /* Depois do prólogo: um default que avalie pra Null também é violação.
     * Só os fixos: a tup e o dict das estrelas nunca são Null. */
    if (meu_nonnull) emite(c, &u, OP_CHECK_NONNULL, nfix);

    /* `int action` e `bool action` não deixam erro escapar: devolvem 500 e
     * False. Sai mais barato emitir o `try` implícito aqui do que ensinar o
     * desenrolamento de erro da VM a olhar o tipo do frame. */
    int32_t rede = -1;
    if (u.tipo_ret) rede = emite(c, &u, OP_SETUP_TRY, 0);

    bloco_stmts(c, &u, n->b);
    /* Chegar ao fim devolve Null, e Null não é valor de nenhum tipo escrito
     * — a não ser `int`/`bool`, cujo fim é o sentinela 0/True. Gerador
     * devolve o gerador. */
    if (u.tipo_ret_nome && !u.tipo_ret && !tp_tem_yield(n->b) && !tp_termina(n->b))
        terro(c, n, "AttributedValueError",
              "%s() devolve %s e pode chegar ao fim sem return (o fim devolveria Null)",
              nome_f, u.tipo_ret_nome);
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

    /* Os upvalues descobertos durante o corpo viram tabela no proto: é o que
     * o MAKE_CLOSURE lê pra montar as células. */
    if (u.nupvals > 0) {
        PSUpval *uv = calloc((size_t)u.nupvals, sizeof(PSUpval));
        if (!uv) cerro(c, "sem memoria", n);
        else {
            char **nms = calloc((size_t)u.nupvals, sizeof(char *));
            for (int32_t i = 0; i < u.nupvals; i++) {
                uv[i].em_local = u.upvals[i].em_local;
                uv[i].idx      = u.upvals[i].idx;
                if (nms) nms[i] = strdup(u.upvals[i].nome);
            }
            c->out->protos[idx].upvals  = uv;
            c->out->protos[idx].nupvals = u.nupvals;
            c->out->protos[idx].upval_nomes = nms;
        }
    }
    for (int32_t i = 0; i < u.nupvals; i++) free(u.upvals[i].nome);
    free(u.upvals);

    vardbg_fecha(c, &u, 0);
    for (int32_t i = 0; i < u.nlocais; i++) free(u.locais[i]);
    free(u.locais);
    free(u.celula); free(u.celula_virgem); free(u.certo); free(u.tipo_decl);
    free(u.sim); free(u.mod_sim);
    for (int32_t i = 0; i < u.nligados; i++) free(u.ligados[i]);
    free(u.ligados);
    for (int32_t i = 0; i < u.n_mod_criados; i++) free(u.mod_criados[i]);
    free(u.mod_criados);
    for (int32_t i = 0; i < u.nglobais_decl; i++) free(u.globais_decl[i]);
    free(u.globais_decl);

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
    SimInfo *s = tp_sim_topo(c, nome);
    if (s) return s;
    if (c->ntopo + 1 > c->cap_topo) {
        int32_t novo = c->cap_topo < 16 ? 16 : c->cap_topo * 2;
        void *nv = realloc(c->topo, sizeof(c->topo[0]) * (size_t)novo);
        if (!nv) { cerro(c, "sem memoria", NULL); return NULL; }
        c->topo = nv;
        c->cap_topo = novo;
    }
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
            c->grava.declara = s->texto2;
            tp_escreve(c, tp_topo_poe(c, s->texto), s->texto);
            break;
        case N_ASSIGNMENT:
            if (!s->texto || (s->texto2 && strcmp(s->texto2, "=") != 0)) return;
            grava_valor(c, mod, s->a);
            tp_escreve(c, tp_topo_poe(c, s->texto), s->texto);
            break;
        case N_ACTION_DECL:
            if (!s->texto) return;
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.tem_valor = 1; c->grava.tipo = "funct"; c->grava.no = s;
            c->grava.decl = s; c->grava.decorado = (unsigned char)decorado;
            tp_escreve(c, tp_topo_poe(c, s->texto), s->texto);
            break;
        case N_ENTITY_DECL:
            if (!s->texto) return;
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.tem_valor = 1; c->grava.tipo = "Entity"; c->grava.no = s; c->grava.decl = s;
            tp_escreve(c, tp_topo_poe(c, s->texto), s->texto);
            break;
        case N_MODEL_DECL:
        case N_ENUM_DECL:
            if (!s->texto) return;
            memset(&c->grava, 0, sizeof(c->grava));
            c->grava.decl = s; c->grava.no = s;
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

static void tp_pre_passada(C *c, PSNode *programa)
{
    if (!programa) return;
    tp_coleta_tipos_arq(c, programa);
    /* método sem `self`: acusa e sintetiza ANTES de qualquer chamada ser conferida */
    for (int32_t i = 0; i < c->ntipos_arq && !CFALHOU(c); i++)
        tp_self_dos_metodos(c, c->tipos_arq[i]);
    tp_junta_ligados(c, programa, &c->ligados_mod, &c->nligados_mod, &c->cap_ligados_mod);
    tp_junta_globais(c, programa);
    Unidade mod;
    memset(&mod, 0, sizeof(mod));
    mod.eh_modulo = 1;
    for (int32_t i = 0; i < programa->lista.n && !CFALHOU(c); i++)
        tp_pre_stmt(c, &mod, programa->lista.itens[i], 0);
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
    for (int passo = 0; passo < 2; passo++) {
        char  **fonte = passo == 0 ? u->mod_criados : c->globais_gravados;
        int32_t nf    = passo == 0 ? u->n_mod_criados : c->nglobais_gravados;
        for (int32_t i = 0; i < nf; i++) {
            const char *nm = fonte[i];
            if (!eh_identificador(nm)) continue;
            int fora = 0;
            for (int32_t k = 0; k < out->npriv_globais && !fora; k++)
                if (!strcmp(out->priv_globais[k], nm)) fora = 1;
            for (int32_t k = 0; k < out->nclasses && !fora; k++)
                if (out->classes[k].classe_privada && out->classes[k].nome && !strcmp(out->classes[k].nome, nm))
                    fora = 1;
            for (int32_t k = 0; k < out->nexportados && !fora; k++)
                if (!strcmp(out->exportados[k], nm)) fora = 1;
            if (fora) continue;
            char *copia = strdup(nm);
            if (!copia) { cerro(c, "sem memoria", NULL); return; }
            out->exportados[out->nexportados++] = copia;
        }
    }
}

/* Os `*` do topo do arquivo, resolvidos ANTES de compilar o primeiro
 * statement (ver o campo `estrelas` do C). */
static void resolve_estrelas(C *c, PSNode *programa)
{
    if (!programa) return;
    int32_t total = 0;
    for (int32_t i = 0; i < programa->lista.n; i++) {
        PSNode *s = programa->lista.itens[i];
        if (s && s->kind == N_IMPORT_STMT && s->texto3) total++;
    }
    if (total == 0) return;
    c->estrelas = calloc((size_t)total, sizeof(*c->estrelas));
    if (!c->estrelas) { cerro(c, "sem memoria", NULL); return; }
    for (int32_t i = 0; i < programa->lista.n; i++) {
        PSNode *s = programa->lista.itens[i];
        if (!s || s->kind != N_IMPORT_STMT || !s->texto3) continue;
        int32_t e = c->nestrelas++;
        c->estrelas[e].no = s;
        if (!c->resolve || !c->resolve->nomes_de) continue;
        if (s->i2 > 0 && s->lista.n == 0) continue;    /* o compilador recusa */
        char enc[512];
        import_modulo_codificado(s, enc);
        int r = c->resolve->nomes_de(c->resolve->ctx, enc, &c->estrelas[e].nomes, &c->estrelas[e].n);
        c->estrelas[e].resolvido = r >= 1;
        c->estrelas[e].incompleto = r == 2;
    }
}

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
    memset(&c, 0, sizeof(c));
    c.out = out;
    c.raiz = programa;
    c.resolve = resolve;

    int32_t idx = novo_proto(&c, "<module>");
    if (idx < 0) return out;

    /* tipos declarados no topo do arquivo, antes de compilar qualquer action */
    coleta_tipos_topo(&c, programa);
    resolve_estrelas(&c, programa);
    tp_pre_passada(&c, programa);

    Unidade u;
    memset(&u, 0, sizeof(u));
    u.idx = idx;
    u.eh_modulo = 1;

    if (programa) {
        for (int32_t i = 0; i < programa->lista.n && out->ok; i++) {
            c.stmt_topo = programa->lista.itens[i];
            stmt(&c, &u, programa->lista.itens[i]);
        }
        c.stmt_topo = NULL;
    }
    emite(&c, &u, OP_HALT, 0);
    if (out->ok) calcula_exportados(&c, &u);

    /* Erro de tipo: o programa não roda, e a lista vai INTEIRA, na ordem do
     * fonte. Erro de sintaxe achado no caminho manda (a compilação parou ali
     * e a lista estaria pela metade). */
    if (c.nerros > 0 && out->ok) {
        qsort(c.erros, (size_t)c.nerros, sizeof(PSErroTipo), tp_erro_cmp);
        out->ok = 0;
        snprintf(out->erro, sizeof(out->erro), "%s", c.erros[0].msg);
        out->erro_linha = c.erros[0].linha;
        out->erro_col = c.erros[0].col;
        out->erro_do_programa = 2;
        out->erros_tipo = c.erros;
        out->nerros_tipo = c.nerros;
        c.erros = NULL;
    }
    free(c.erros);
    /* a AST volta ao que era: o `self` sintetizado só existiu nesta compilação */
    for (int32_t i = 0; i < c.n_self_sint; i++) {
        struct SelfSint *s = &c.self_sint[i];
        free(s->met->lista.itens);
        s->met->lista = s->orig;
        s->met->self_faltava = 0;
        free(s->no);
    }
    free(c.self_sint);
    for (int32_t i = 0; i < c.ntpool; i++) free(c.tpool[i]);
    free(c.tpool);
    free(c.topo);
    for (int32_t i = 0; i < c.nligados_mod; i++) free(c.ligados_mod[i]);
    free(c.ligados_mod);
    free(c.tipos_arq);

    for (int32_t e = 0; e < c.nestrelas; e++) {
        for (int32_t k = 0; k < c.estrelas[e].n; k++) free(c.estrelas[e].nomes[k]);
        free(c.estrelas[e].nomes);
    }
    free(c.estrelas);
    for (int32_t i = 0; i < c.nglobais_gravados; i++) free(c.globais_gravados[i]);
    free(c.globais_gravados);

    vardbg_fecha(&c, &u, 0);
    for (int32_t i = 0; i < u.nlocais; i++) free(u.locais[i]);
    free(u.locais);
    free(u.celula); free(u.celula_virgem); free(u.certo); free(u.tipo_decl);
    free(u.sim); free(u.mod_sim);
    for (int32_t i = 0; i < u.nligados; i++) free(u.ligados[i]);
    free(u.ligados);
    for (int32_t i = 0; i < u.n_mod_criados; i++) free(u.mod_criados[i]);
    free(u.mod_criados);
    for (int32_t i = 0; i < u.nglobais_decl; i++) free(u.globais_decl[i]);
    free(u.globais_decl);
    for (int32_t i = 0; i < c.n_tipos_topo; i++) free(c.tipos_topo_nomes[i]);
    free(c.tipos_topo_nomes);
    free(c.tipos_topo);
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
    for (int32_t i = 0; i < p->npriv_globais; i++) free(p->priv_globais[i]);
    free(p->priv_globais);
    for (int32_t i = 0; i < p->nexportados; i++) free(p->exportados[i]);
    free(p->exportados);
    free(p->erros_tipo);
    free(p);
}
