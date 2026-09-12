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

struct Unidade {
    /* Função que ENVOLVE esta. NULL no módulo e nas actions de topo — é o que
     * limita a captura: nome não achado aqui nem no pai vira global. */
    Unidade *pai;
    /* Slots que viram CÉLULA porque alguma action aninhada os usa. */
    unsigned char celula[256];
    /* Célula criada pelo `marca_celulas` que ainda não recebeu nada: o slot
     * EXISTE mas o nome ainda não vale nada. Sem separar os dois, o `for each`
     * achava que a variável dele já existia e tentava salvar o valor "de
     * fora", lendo uma célula vazia. */
    unsigned char celula_virgem[256];
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
    unsigned char certo[256];
    /* Tipo DECLARADO de cada slot (`str s = ...`): 0 = sem tipo, senao o
     * TIPO_* da VM + 1. E o que faz a tipagem ser estatica: toda escrita no
     * slot passa pelo OP_COERCE_DECL, nao so a declaracao. */
    unsigned char tipo_decl[256];
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
            if (i < 256) u->certo[i] = 0;
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
static int liga_o_nome(PSNode *n, const char *alvo)
{
    if (!n) return 0;
    switch (n->kind) {
        case N_ASSIGNMENT: case N_VAR_DECL: case N_FOR_EACH_STMT:
        case N_UNPACK_TARGET: case N_ACTION_DECL: case N_ENTITY_DECL:
        case N_MODEL_DECL: case N_ENUM_DECL: case N_IMPORT_STMT:
            if (n->texto && !strcmp(n->texto, alvo)) return 1;
            for (int32_t i = 0; i < n->lista2.n; i++) {
                PSNode *x = n->lista2.itens[i];
                if (x && x->texto && !strcmp(x->texto, alvo)) return 1;
            }
            for (int32_t i = 0; i < n->lista2_alias.n; i++) {
                PSNode *x = n->lista2_alias.itens[i];
                if (x && x->texto && !strcmp(x->texto, alvo)) return 1;
            }
            break;
        default: break;
    }
    if (liga_o_nome(n->a, alvo) || liga_o_nome(n->b, alvo)
        || liga_o_nome(n->c, alvo) || liga_o_nome(n->e, alvo)) return 1;
    for (int32_t i = 0; i < n->lista.n; i++)
        if (liga_o_nome(n->lista.itens[i], alvo)) return 1;
    for (int32_t i = 0; i < n->lista2.n; i++)
        if (liga_o_nome(n->lista2.itens[i], alvo)) return 1;
    return 0;
}

/* ── closure: captura de variável de fora ────────────────────────────────
 *
 * O modelo é o do CPython, não o do Lua: a variável capturada mora numa
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
        /* `i < 256` PRIMEIRO: o índice é do `celula[256]`, não do `locais`
         * (que é heap dimensionado por `nlocais`). Na ordem antiga o
         * `arrayIndexThenCheck` do cppcheck acusava, e ele estava certo em
         * apontar — quem lê não tem como saber qual dos dois vetores o 256
         * limita. Conserto é deixar o código óbvio pro detector, não silenciar
         * o detector. */
        if (i < 256 && u->pai->celula[i] && strcmp(u->pai->locais[i], nome) == 0)
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

/* Procura actions aninhadas dentro de `n` (sem entrar nelas duas vezes) e
 * coleta os nomes que elas citam. */
static void acha_aninhadas(C *c, PSNode *n, char ***v, int32_t *cnt, int32_t *cap)
{
    if (!n) return;
    if (n->kind == N_ACTION_DECL) { varre_nomes(c, n, v, cnt, cap); return; }
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
        if (slot < 0 || slot >= 256 || u->celula[slot]) continue;
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

/* A expressão de um decorador geral (`@obj.metodo(args)` / `@obj.prop`),
 * avaliada SEMPRE como chamada — é o mesmo protocolo em qualquer posição
 * (funct solta, em cima de classe, dentro de classe), num lugar só. O valor
 * fica no topo da pilha: é o registrar a quem se entrega a função. */
static void emite_decorador_chamada(C *c, Unidade *u, PSNode *dec)
{
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
        if (i < 256 && u->celula[i] && u->certo[i]) emite(c, u, OP_CELL_GET, i);
        else if (i < 256 && u->celula[i]) {
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, i, 0, NULL, 0));
            emite(c, u, OP_CELL_GET_NAME, idx_global(c, nome));
        }
        else if (i < 256 && u->certo[i]) emite(c, u, OP_LOAD_LOCAL, i);   /* param/tipada */
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
            return !(i < 256 && u->celula_virgem[i]);
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

/* codigo TIPO_* de um nome de tipo declaravel, ou -1 */
static int cod_tipo_decl(const char *t)
{
    static const struct { const char *nome; int cod; } T[] = {
        { "str", 0 }, { "int", 1 }, { "flo", 2 }, { "bool", 3 },
        { "list", 4 }, { "dict", 5 }, { "json", 5 }, { "tup", 6 },
        { "char", 8 }, { "Object", 10 }, { "object", 10 },
        /* 11 = TIPO_LONG: inteiro de qualquer tamanho, bignum inclusive */
        { "long", 11 }, { "Long", 11 },
        /* apelidos (ver eh_apelido_tipo no parser): a mesma regra do tipo */
        { "string", 0 }, { "String", 0 }, { "integer", 1 }, { "Integer", 1 },
        { "tuple", 6 }, { "Tuple", 6 }, { "dictionary", 5 }, { "Dictionary", 5 },
    };
    if (!t) return -1;
    for (size_t i = 0; i < sizeof(T) / sizeof(T[0]); i++)
        if (strcmp(T[i].nome, t) == 0) return T[i].cod;
    return -1;
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
        for (int32_t i = 0; i < q->nlocais && i < 256; i++)
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

static void guarda_nome_modo(C *c, Unidade *u, const char *nome, int certa)
{
    if (u->eh_modulo || eh_global_declarada(u, nome)) {
        emite_coerce_se_tipado(c, u, nome, tipo_topo_de(c, nome));
        if (u->eh_modulo) mod_criados_add(c, u, nome);  /* p/ escopo de bloco */
        emite(c, u, OP_STORE_GLOBAL, idx_global(c, nome));
        return;
    }
    /* Escrita num nome que vem de fora vai pra célula capturada — é o mesmo
     * critério que a linguagem já usava pra global ("se já existe lá fora,
     * escreve lá"), agora valendo também pro escopo da função que envolve. */
    {
        int32_t up = resolve_upval(c, u, nome);
        if (up >= 0) {
            emite_coerce_se_tipado(c, u, nome, tipo_de_upval(u, nome));
            emite(c, u, OP_STORE_UPVAL, up);
            return;
        }
    }
    int32_t i = idx_local(c, u, nome);
    if (certa && i < 256) u->certo[i] = 1;
    if (i < 256) u->celula_virgem[i] = 0;
    if (i < 256 && u->celula[i]) {
        emite_coerce_se_tipado(c, u, nome, u->tipo_decl[i]);
        if (u->certo[i]) { emite(c, u, OP_CELL_SET, i); return; }
        emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, i, 0, NULL, 0));
        emite(c, u, OP_CELL_SET_NAME, idx_global(c, nome));
        return;
    }
    if (i < 256 && u->certo[i]) {
        emite_coerce_se_tipado(c, u, nome, u->tipo_decl[i]);
        emite(c, u, OP_STORE_LOCAL, i);
        return;
    }
    /* STORE_NAME decide em runtime entre local novo e global existente: se o
     * nome e um global DECLARADO com tipo no topo do arquivo, e nele que a
     * escrita vai cair (write-through), entao confere pelo tipo dele. */
    emite_coerce_se_tipado(c, u, nome, i < 256 && u->tipo_decl[i] ? u->tipo_decl[i] : tipo_topo_de(c, nome));
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
/* Carimba linha/coluna em TODA a sub-árvore.
 *
 * O trecho `{...}` é re-lexado a partir de uma string isolada, então os nós
 * dele nascem na linha 1. O escopo do `expr` impede que esse 1 vaze pra fora,
 * mas DENTRO da interpolação ele continuaria errado: `post(f"{1 / x}")` na
 * linha 4 reportaria linha 1 na divisão por zero.
 *
 * É o mesmo ajuste que o CPython faz desde a PEP 498
 * (`fstring_fix_node_location`), e pelo mesmo motivo: ele também re-parseia o
 * interior. A coluna não é a exata dentro do trecho; a linha é a certa, e é
 * ela que o traceback mostra. */
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
 * O modelo de compilador é a posição ser ARGUMENTO, não estado pendurado (no
 * CPython o gerador usa a posição do nó, e é dela que sai o `co_linetable`).
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
                    /* I10 — `is` com LITERAL de um dos lados e erro.
                     *
                     * Aqui `is` e o operador de TIPO (`5 is int`), nao a
                     * identidade do Python — esta na doc e ha 77 usos no
                     * repositorio. O defeito era outro: quando o outro lado
                     * NAO era um tipo, ele caia em igualdade de valor SEM
                     * AVISAR. `x is 0` digitado no lugar de `x == 0` virava
                     * comparacao, dava o resultado "certo" e nunca reclamava.
                     *
                     * O CPython tambem nao deixa passar: emite
                     * `SyntaxWarning: "is" with 'int' literal. Did you mean
                     * "=="?` em tempo de compilacao. Aqui e ERRO — a linguagem
                     * nao tem canal de aviso, e silencio foi o que criou o
                     * problema.
                     *
                     * So o LITERAL: `x is y` com dois nomes continua valendo,
                     * porque `y` pode perfeitamente guardar um tipo. */
                    /* So o lado DIREITO: `5 is int` tem literal a esquerda
                     * e e o uso correto — o tipo e que vai a direita. Nome de
                     * tipo chega como N_TYPE_NAME, nunca N_LITERAL, entao a
                     * checagem separa os dois sozinha. */
                    /* `x is Null` FICA: e o idioma da linguagem pro teste
                     * de ausencia (o `x is None` do Python), e `type(null)` e
                     * literalmente "Null" — ali o literal ocupa a posicao de
                     * tipo com sentido. Os outros literais nao tem essa
                     * leitura. */
                    if (n->b && n->b->kind == N_LITERAL && n->b->lit != L_NULL) {
                        static const char *NOME_LIT[] = {
                            "int", "flo", "str", "bool", "Null", "str", "int"
                        };
                        const char *tn = (n->b->lit >= 0 && n->b->lit <= L_BIGINT)
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
                        cerro_sx(c, n, "argumento posicional depois de nomeado");
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
            if (sombreia) {
                snprintf(salvo, sizeof(salvo), "  lcv$%s", var);
                carrega_nome(c, u, var);
                guarda_nome_modo(c, u, salvo, 1);
            }
            int32_t M = escopo_marca(u);

            expr(c, u, n->a);
            emite(c, u, OP_LOAD_CONST, idx_const(c, u, K_INT, 0, 0, NULL, 0));
            int32_t topo = UP(c, u)->ncode;
            int32_t fim = emite(c, u, OP_ITER_NEXT, 0);
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
                guarda_nome_modo(c, u, var, 1);
            }
            carrega_nome(c, u, acc);
            return;
        }

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
            if (t < 0 && n->texto && strcmp(n->texto, "type") == 0) t = 7;
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
            int32_t nkw_b = 0;
            for (int32_t i = 0; i < n->lista.n; i++)
                if (n->lista.itens[i]->texto) nkw_b++;
            if (nkw_b == 0) {
                carrega_nome(c, u, pai);
                emite(c, u, OP_LOAD_SELF, 0);
                for (int32_t i = 0; i < n->lista.n; i++)
                    expr(c, u, n->lista.itens[i]->a);
                emite(c, u, OP_CALL_BASE, n->lista.n);
                return;
            }
            /* Com argumento NOMEADO (`base(x=5)`), em vez de repetir aqui toda
             * a resolução de nome/default do OP_CALL_KW, o `__init__` do pai é
             * carregado LIGADO ao self e a chamada segue o caminho normal. */
            for (int32_t i = 0, viu = 0; i < n->lista.n; i++) {
                if (n->lista.itens[i]->texto) viu = 1;
                else if (viu) { cerro_sx(c, n, "argumento posicional depois de nomeado"); return; }
            }
            carrega_nome(c, u, pai);
            emite(c, u, OP_LOAD_BASE_INIT, 0);
            for (int32_t i = 0; i < n->lista.n; i++)
                expr(c, u, n->lista.itens[i]->a);
            for (int32_t i = n->lista.n - nkw_b; i < n->lista.n; i++) {
                const char *nm = n->lista.itens[i]->texto;
                emite(c, u, OP_LOAD_CONST,
                      idx_const(c, u, K_STR, 0, 0, nm, (int32_t)strlen(nm)));
            }
            emite(c, u, OP_BUILD_TUPLE, nkw_b);
            emite(c, u, OP_CALL_KW, n->lista.n);
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
                if (si >= 0 && si < 256 && !u->celula[si]) u->certo[si] = 1;
            }
            int32_t idx = compila_action(c, n, u);
            if (CFALHOU(c)) return;
            emite_funcao(c, u, idx);
            if (u->eh_modulo && n->is_private) priv_global_add(c, n->texto);
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
                    if (sl < 256) u->tipo_decl[sl] = (unsigned char)(cod + 1);
                }
            }
            /* aponta o erro no INÍCIO do valor (RHS), não na sub-expressão
             * mais profunda que o expr() deixou em coluna_atual — é o
             * lugar EXATO do erro, igual ao node.value do interp. */
            if (n->a) {
                if (n->a->line) c->linha_atual  = n->a->line;
                if (n->a->col)  c->coluna_atual = n->a->col;
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
            if (sombreia) {
                snprintf(salvo, sizeof(salvo), "  fe$%s", var_laco);   /* nome não digitável */
                carrega_nome(c, u, var_laco);
                guarda_nome_modo(c, u, salvo, 1);
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
                    && !liga_o_nome(c->raiz, "range")) {
                usa_range = 1;
                for (int32_t k = 0; k < n->a->lista.n; k++)
                    if (n->a->lista.itens[k]->texto) { usa_range = 0; break; }  /* nomeado: não */
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
            else      guarda_nome_modo(c, u, n->texto ? n->texto : "", 1);
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
                /* `json` e os apelidos (`string`, `Integer`…) passam pela
                 * mesma tabela da declaracao; `char` e `Object` nao sao tipo
                 * de campo de model */
                if (t < 0) {
                    int a = cod_tipo_decl(f->texto2);
                    if (a >= 0 && a <= 6) t = a;
                }
                if (t < 0) { cerro_sx(c, f, "tipo desconhecido em model"); return; }
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
                const char *cls_nome = NULL, *met_nome = NULL;
                if (n->b->kind == N_BLOCK)
                    for (int32_t i = 0; i < n->b->lista.n; i++) {
                        PSNode *bi = n->b->lista.itens[i];
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

                /* 1) avalia SEMPRE a expressão do decorador como CHAMADA
                 * `obj.metodo(args)` — erro do decorador propaga, mesmo com 0
                 * args. É a MESMA emissão do decorador dentro da classe. */
                emite_decorador_chamada(c, u, dec);
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
                } else if (cls_nome && met_nome) {
                    /* handler de classe: instancia a classe e registra o método
                     * dela — $inst = Classe(); $reg.register($inst.metodo) */
                    carrega_nome(c, u, cls_nome);
                    emite(c, u, OP_CALL, 0);
                    guarda_nome_modo(c, u, "$inst", 1);
                    carrega_nome(c, u, "$reg");
                    emite(c, u, OP_GET_MEMBER, idx_const(c, u, K_STR, 0, 0, "register", 8));
                    carrega_nome(c, u, "$inst");
                    emite(c, u, OP_GET_MEMBER, idx_const(c, u, K_STR, 0, 0, met_nome, (int32_t)strlen(met_nome)));
                    emite(c, u, OP_CALL, 1);
                    emite(c, u, OP_POP_TOP, 0);
                }
                return;
            }
            /* `@qualquer` sozinho, sem action embaixo, é ignorado — o
             * interpretador aceita e segue. Recusar quebrava script válido. */
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
                    if (dec && dec->lista.n > 1 && ndec_pend < 8) dec_pend[ndec_pend++] = dec;
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
            for (int32_t i = 0; i < n->lista2.n; i++)
                carrega_nome(c, u, n->lista2.itens[i]->texto ? n->lista2.itens[i]->texto : "");
            emite(c, u, OP_MAKE_CLASS, ci);
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
            for (int k = 0; k < npares && !CFALHOU(c); k++) {
                PSNode *dec = par_dec[k];
                PSNode *met = par_met[k];
                const char *mn = met->texto ? met->texto : "";
                /* `@mapp.post(...)` com `mapp` sendo campo de INSTÂNCIA da
                 * própria classe: na hora em que o corpo é declarado não há
                 * instância, e o nome cairia num NameError apontando pra
                 * linha da classe — sem dizer que o que falta é `static`. */
                {
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
                }
                def = &c->out->classes[ci];
                int eh_est = met->is_static;
                for (int32_t j = 0; j < def->nmetodos && !eh_est; j++)
                    if (strcmp(def->met_nomes[j], mn) == 0)
                        eh_est = c->out->protos[def->met_protos[j]].eh_static;
                emite_decorador_chamada(c, u, dec);
                guarda_nome_modo(c, u, "$reg", 1);
                carrega_nome(c, u, "$reg");
                emite(c, u, OP_GET_MEMBER, idx_const(c, u, K_STR, 0, 0, "register", 8));
                if (eh_est) {
                    /* método estático: `Classe.metodo` é a própria funct */
                    carrega_nome(c, u, cls_nome_aqui);
                } else {
                    /* método comum precisa de instância — o mesmo protocolo
                     * do decorador em cima da classe: `$inst = Classe()` */
                    carrega_nome(c, u, cls_nome_aqui);
                    emite(c, u, OP_CALL, 0);
                    guarda_nome_modo(c, u, "$inst", 1);
                    carrega_nome(c, u, "$inst");
                }
                emite(c, u, OP_GET_MEMBER, idx_const(c, u, K_STR, 0, 0, mn, (int32_t)strlen(mn)));
                emite(c, u, OP_CALL, 1);
                emite(c, u, OP_POP_TOP, 0);
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
            char encoded[512]; int el = 0;
            /* `import 'x'` (i2 == -1): marcador \x01 + o literal como foi
             * escrito; o runtime decide se e caminho ou nome de modulo. O nome
             * ligado e o do arquivo, sem pasta e sem extensao. */
            char base_aspas[256]; base_aspas[0] = '\0';
            if (n->i2 == -1) {
                const char *spec = (n->lista.n > 0 && n->lista.itens[0]->texto) ? n->lista.itens[0]->texto : "";
                int pl = (int)strlen(spec); if (pl > 500) pl = 500;
                encoded[el++] = '\x01';
                memcpy(encoded + el, spec, (size_t)pl); el += pl;
                const char *b = strrchr(spec, '/'); b = b ? b + 1 : spec;
                snprintf(base_aspas, sizeof(base_aspas), "%s", b);
                char *ext = strrchr(base_aspas, '.');
                if (ext && ext != base_aspas && (strcmp(ext, ".ps") == 0 || strcmp(ext, ".psl") == 0 || strcmp(ext, ".p") == 0))
                    *ext = '\0';
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
            const char *mod = encoded;
            /* `import pacote.modulo` liga o ÚLTIMO segmento (`modulo`), como
             * a doc diz e o interpretador faz — antes a VM recusava com
             * NotImplementedError. Import RELATIVO (`import .x`) segue exigindo
             * `from`, porque aí não há nome óbvio pra ligar. */
            int simples = (n->i2 == -1) || (n->i2 == 0 && n->lista.n >= 1);
            const char *ultimo = (n->i2 == -1) ? base_aspas
                               : (n->lista.n > 0 && n->lista.itens[n->lista.n - 1]->texto
                                  ? n->lista.itens[n->lista.n - 1]->texto : mod);
            /* `import 'meu-mod.ps'` sem `as`: o nome do arquivo tem que servir
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
                    guarda_nome(c, u, n->texto2 ? n->texto2 : ultimo);
                    return;
                }
                for (int32_t i = 0; i < n->lista2.n; i++) {
                    const char *membro = n->lista2.itens[i]->texto;
                    const char *apelido = (i < n->lista2_alias.n && n->lista2_alias.itens[i])
                                        ? n->lista2_alias.itens[i]->texto : membro;
                    emite(c, u, OP_IMPORT_MOD, idx_const(c, u, K_STR, 0, 0, mod, (int32_t)strlen(mod)));
                    emite(c, u, OP_IMPORT_FROM, idx_const(c, u, K_STR, 0, 0, membro, (int32_t)strlen(membro)));
                    guarda_nome(c, u, apelido);
                }
                return;
            }

            if (strcmp(n->texto, "import") == 0) {
                if (!simples) { cerro_sx(c, n, "import de modulo pontuado/relativo precisa de 'from ... import ...'"); return; }
                emite(c, u, OP_IMPORT_MOD, idx_const(c, u, K_STR, 0, 0, mod, (int32_t)strlen(mod)));
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
    size_t n = strlen(nome);
    p->nome = malloc(n + 1);
    if (!p->nome) { cerro(c, "sem memoria", NULL); return -1; }
    memcpy(p->nome, nome, n + 1);
    return c->out->nprotos++;
}

/* Guarda o valor que está NO TOPO da pilha dentro de um alvo.
 *
 * O alvo pode ser nome, `o.campo`, `d[k]` ou um grupo aninhado — os mesmos
 * quatro do CPython. A ordem importa: aqui o valor JÁ está na pilha (o UNPACK
 * o pôs lá) e container/índice só são avaliados agora, depois do lado direito
 * inteiro e depois da checagem de quantidade — que é a ordem do CPython.
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

    /* slot 0 é o `self`; os campos vêm depois, na ordem de declaração */
    { int32_t si = idx_local(c, &u, "self"); if (si < 256) u.certo[si] = 1; }
    int32_t ndef = 0;
    for (int32_t i = 0; i < campos->n; i++) {
        const char *nome = campos->itens[i]->texto ? campos->itens[i]->texto : "";
        int32_t pi = idx_local(c, &u, nome);
        if (pi < 256) u.certo[pi] = 1;
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

    int32_t ndef = 0;
    for (int32_t i = 0; i < n->lista.n; i++) {
        PSNode *par = n->lista.itens[i];
        {
            int32_t pi = idx_local(c, &u, par->texto ? par->texto : "");
            if (pi < 256) u.certo[pi] = 1;
        }
        if (par->a) ndef++;
        else if (ndef > 0) {
            cerro_sx(c, n, "parametro sem valor padrao depois de um com padrao");
            break;
        }
    }
    c->out->protos[idx].nparams = n->lista.n;
    c->out->protos[idx].ndefaults = ndef;
    c->out->protos[idx].eh_async = n->is_async;
    c->out->protos[idx].eh_static = meu_static;
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
        /* Tipo declarado (`funct f(str nome)`) — o parser deixou em texto2.
         * O vetor só nasce se ALGUM parâmetro tiver tipo: função sem tipagem
         * não paga nada, nem memória nem checagem. */
        int32_t com_tipo = 0;
        for (int32_t i = 0; i < n->lista.n; i++)
            if (n->lista.itens[i]->texto2) com_tipo = 1;
        if (com_tipo) {
            char **tipos = calloc((size_t)n->lista.n, sizeof(char *));
            if (!tipos) { cerro(c, "sem memoria", n); }
            else {
                for (int32_t i = 0; i < n->lista.n; i++) {
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
        emite(c, &u, OP_STORE_LOCAL, i);
        if (pula >= 0) UP(c, (&u))->code[pula + 1] = UP(c, (&u))->ncode;
    }

    /* Células das variáveis que as actions aninhadas capturam. Vem depois do
     * prólogo (que testa o slot cru com JUMP_IF_SET) e antes do corpo. */
    marca_celulas(c, &u, n, n->b);

    /* Depois do prólogo: um default que avalie pra Null também é violação. */
    if (meu_nonnull) emite(c, &u, OP_CHECK_NONNULL, n->lista.n);

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
    c.raiz = programa;

    int32_t idx = novo_proto(&c, "<module>");
    if (idx < 0) return out;

    /* tipos declarados no topo do arquivo, antes de compilar qualquer action */
    coleta_tipos_topo(&c, programa);

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

    vardbg_fecha(&c, &u, 0);
    for (int32_t i = 0; i < u.nlocais; i++) free(u.locais[i]);
    free(u.locais);
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
    free(p);
}
