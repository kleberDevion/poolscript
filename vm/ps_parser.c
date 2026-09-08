/*
 * Parser recursivo-descendente da PoolScript em C puro.
 *
 * Recursive-descent sobre o subconjunto que a VM compila, incluindo a cadeia
 * de precedência inteira (do mais fraco pro mais forte):
 *
 *   or → and → not → comparação → | → ^ → & → << >> → + - → * / % → unário
 *   → posfixo (chamada, .membro, [índice]) → primário
 *
 * Regras herdadas que o teste diferencial cobre:
 *   - Palavra reservada não pode virar nome (variável, parâmetro, action).
 *   - `base` é reservada contextual: não vira nome, mas `base(...)` chama.
 *   - Bloco aceita `{ }` ou `:` + INDENT/DEDENT, e os dois convivem.
 *   - Chave de dict sem aspas (`{nome: 1}`) vira string.
 *
 * Nós fora do subconjunto param com erro explícito, nunca geram AST errada
 * em silêncio.
 */
#include "ps_parser.h"
#include "ps_pilha.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct {
    PSToken   *toks;
    int32_t    n;
    int32_t    pos;
    PSArena   *arena;
    PSParseResult *out;
    /* Dentro de Entity o decorador NÃO engole a action seguinte: os dois
     * viram entradas separadas do corpo (é o capture_action=False do parser
     * Python). Fora de Entity, `@NonNull action f()` captura. */
    int        dec_sem_captura;
    /* `for each x in <expr> {` — ali o `{` ABRE BLOCO, não interpola.
     * Sem isto, `for each c in "abc" {` lia `"abc" {` como interpolação e o
     * bloco do laço sumia, obrigando a passar a string por variável antes.
     * Só vale no nível 0: `grupo_depth` conta (), [] e {} atravessados,
     * porque dentro de um agrupamento não há bloco pra abrir. */
    int        chave_abre_bloco;
    int        grupo_depth;
    /* Profundidade da descida recursiva. Ver PS_PARSE_PROF_MAX. */
    int        prof;
} P;

/* TETO DE PROFUNDIDADE do parser.
 *
 * A descida é recursiva e não tinha limite: `post(((((…1…)))))` com 20 mil
 * parênteses esgotava a pilha do C e o processo morria de SIGSEGV, sem
 * mensagem. Medido: 10.000 níveis passavam, 20.000 matavam.
 *
 * Isso não é caso de laboratório — é o caminho de ENTRADA NÃO CONFIÁVEL por
 * definição: `--check` é o que o editor roda a cada tecla (e o LSP junto),
 * `psl install` compila pacote de terceiro, e a CI roda `--check` em arquivo
 * que veio de fora. Dentro do jinker é pior: a fibra tem pilha de 128 KB, ~64x
 * menor que os 8 MB do processo.
 *
 * QUEM DECIDE É A FOLGA DE PILHA, não a contagem. Contar nível não mede nada:
 * 2000 níveis de `(((...)))` gastam muito menos que 2000 níveis de expressão
 * com chamada, e o mesmo número que sobra folgado no `-O2` estoura no `-O0`,
 * onde o quadro é várias vezes maior. Número fixo ou aperta um build ou afrouxa
 * o outro, e o critério pra escolher acaba sendo "o valor que faz o teste
 * calar". `ps_pilha_apertada()` (ver `ps_pilha.h`) mede o que sobra de
 * verdade, e se adapta sozinha ao `ulimit -s` e à pilha pequena da fibra.
 *
 * O CONTADOR CONTINUA, com teto alto (100 mil), e por um motivo só: a medição
 * de folga depende de a base ter sido marcada, e um chamador que esqueça de
 * marcar deixaria a descida sem freio nenhum. O contador é o cinto de
 * segurança do freio — nunca é ele que dispara em uso real.
 *
 * É o que CPython faz com `PyOS_CheckStack`, e o SQLite no parser dele. */
#define PS_PARSE_PROF_MAX 100000

/* ── reservadas que também não podem virar nome ─────────────────────────── */
/* `=`, `+=`, `-=`, `*=`, `/=`, `%=` — os operadores que abrem atribuição. */
static int eh_op_atribuicao(const char *op)
{
    return strcmp(op, "=")  == 0 || strcmp(op, "+=") == 0 || strcmp(op, "-=") == 0
        || strcmp(op, "*=") == 0 || strcmp(op, "/=") == 0 || strcmp(op, "%=") == 0;
}

static int eh_contextual_reservada(const char *s)
{
    return s && strcmp(s, "base") == 0;
}

/* Copia o texto de um token PARA A ARENA.
 *
 * Obrigatório: os tokens são liberados assim que o parse termina, enquanto a
 * AST sobrevive até a compilação acabar. Guardar `tok->texto` direto no nó
 * deixava a AST cheia de ponteiros soltos — o serializador imprimia lixo
 * binário. A AST tem que ser autocontida; a arena garante isso sem custo de
 * gestão, porque some inteira num free só. */
static const char *dup_tok(P *p, PSToken *t)
{
    if (!t || !t->texto) return NULL;
    return ps_arena_strdup(p->arena, t->texto, t->texto_len);
}

static const char *dup_str(P *p, const char *s)
{
    if (!s) return NULL;
    return ps_arena_strdup(p->arena, s, (int)strlen(s));
}

/* ── erro ───────────────────────────────────────────────────────────────── */
static void perro(P *p, const char *msg, PSToken *t)
{
    if (!p->out->ok) return;              /* preserva o primeiro erro */
    p->out->ok = 0;
    snprintf(p->out->erro, sizeof(p->out->erro), "%s", msg);
    p->out->erro_linha = t ? t->line : 0;
    p->out->erro_col = t ? t->col : 0;
}

#define FALHOU(p) (!(p)->out->ok)

static PSToken *atual(P *p)
{
    return &p->toks[p->pos < p->n ? p->pos : p->n - 1];
}

static PSToken *espia(P *p, int off)
{
    int32_t i = p->pos + off;
    if (i >= p->n) i = p->n - 1;
    return &p->toks[i];
}

static int checa(P *p, PSTokType t)
{
    return atual(p)->type == t;
}

static int checa_kw(P *p, const char *kw)
{
    PSToken *t = atual(p);
    return t->type == T_KW && t->texto && strcmp(t->texto, kw) == 0;
}

static int checa_op(P *p, const char *op)
{
    PSToken *t = atual(p);
    return t->type == T_OP && t->texto && strcmp(t->texto, op) == 0;
}

/* `funct` e a palavra que declara funcao — a unica. `action` e `reaction`
 * SAIRAM da linguagem; o que sobrou delas e a recusa com nome, logo abaixo. */
static int eh_kw_funct(PSToken *t)
{
    return t->type == T_KW && t->texto && strcmp(t->texto, "funct") == 0;
}

/* `action` / `reaction` escritos onde `funct` deveria estar.
 *
 * Elas nao sao mais palavra reservada: o lexer devolve IDENT, e sem esta
 * checagem `action f() { ... }` viraria um nome solto seguido de outro, com
 * erro de sintaxe generico ou — pior — um `NameError: name 'action' is not
 * defined` em tempo de execucao. Recusar aqui, com a palavra certa na
 * mensagem, e o mesmo tratamento que o `def` ja tinha. */
static int eh_grafia_morta(PSToken *t)
{
    return t->texto && (t->type == T_IDENT || t->type == T_KW)
        && (strcmp(t->texto, "action") == 0 || strcmp(t->texto, "reaction") == 0);
}

/* `static` e `nonnull` COLADOS na declaracao: `static funct m(a)`,
 * `private nonnull int funct f(v)`. Valem por POSICAO — so na cabeca, antes
 * de `funct` — entao NAO viram palavra reservada: variavel chamada `static`
 * continua valendo. `NonNull` casa junto, que e como o decorador se escreve. */
static int eh_mod_funct(PSToken *t)
{
    if (!t->texto) return 0;
    if (t->type != T_KW && t->type != T_IDENT && t->type != T_IDENT_UPPER) return 0;
    return strcmp(t->texto, "static") == 0 || strcmp(t->texto, "nonnull") == 0
        || strcmp(t->texto, "NonNull") == 0;
}

static int eh_tipo_kw(PSToken *t);
static int eh_apelido_tipo(PSToken *t);

/* O tipo de RETORNO de uma funct, no nome canônico — ou NULL se este token não
 * é um tipo de retorno.
 *
 * UM lugar decide isso. Antes a resposta estava espalhada: o `eh_tipo_kw`
 * (usado também pra declaração de variável), a cabeça de funct e a validação
 * do statement, cada um com uma lista. O apelido não estava em nenhuma delas, e
 * `public static string funct main()` dentro de uma classe casava com a regra
 * de CAMPO: nascia um campo chamado `string`, do tipo `static`, a funct sumia
 * como membro e o programa rodava sem erro nenhum e sem fazer nada.
 *
 * Os apelidos resolvem pro canônico porque a doc diz que a linha do tipo
 * apelidado vale igual à do tipo apelidado: `string` É `str`. */
/* TODO tipo vale como retorno — os da linguagem e os que o usuário declara.
 * Não há lista branca: uma que aceitasse `dict` e recusasse `list` seria
 * arbitrária, e a linguagem é pra ser simples.
 *
 * O que decide é a POSIÇÃO: o que vem colado antes do `funct`, depois dos
 * modificadores, é o tipo de retorno. Nenhuma outra construção da linguagem
 * tem um nome seguido de `funct`, então não há ambiguidade a resolver.
 *
 * Palavra-chave precisa da lista porque nem toda palavra-chave é tipo:
 * `return funct` não pode virar "funct que devolve return". */
static int eh_tipo_de_retorno(PSToken *t)
{
    if (!t->texto) return 0;
    /* `static`/`nonnull` são IDENT e vêm na mesma posição. Sem esta linha eles
     * seriam lidos como TIPO — e `static funct m()` virava uma funct que
     * devolve `static`, sem a marca de estática: `C.m()` respondia "não tem
     * método estático 'm'" com o `static` escrito na tela. */
    if (eh_mod_funct(t)) return 0;
    /* Nome do usuário: classe, Entity, model, apelido de tipo. */
    if (t->type == T_IDENT || t->type == T_IDENT_UPPER) return 1;
    if (t->type != T_KW) return 0;
    static const char *const TIPOS[] = {
        "str", "int", "long", "flo", "bool", "char", "list",
        "dict", "tup", "json", "JSON", "Object", "object", NULL
    };
    for (int i = 0; TIPOS[i]; i++) if (strcmp(t->texto, TIPOS[i]) == 0) return 1;
    return 0;
}

/* O nome canônico de um apelido, ou NULL quando o token já é o nome canônico.
 * A doc diz que a linha do tipo apelidado vale igual: `string` É `str`, e a
 * árvore guarda `str` pra quem consome não precisar saber das duas grafias. */
static const char *canonico_de_apelido(PSToken *t)
{
    if (!t->texto) return NULL;
    const char *s = t->texto;
    if (strcmp(s, "string")     == 0 || strcmp(s, "String")     == 0) return "str";
    if (strcmp(s, "integer")    == 0 || strcmp(s, "Integer")    == 0) return "int";
    if (strcmp(s, "tuple")      == 0 || strcmp(s, "Tuple")      == 0) return "tup";
    if (strcmp(s, "dictionary") == 0 || strcmp(s, "Dictionary") == 0) return "dict";
    if (strcmp(s, "object")     == 0 || strcmp(s, "Object")     == 0) return "Object";
    if (strcmp(s, "json")       == 0 || strcmp(s, "JSON")       == 0) return "dict";
    return NULL;
}

/* O tipo de retorno deste token, pronto pra ir na árvore. */
static const char *tipo_retorno_dup(P *p, PSToken *t)
{
    const char *c = canonico_de_apelido(t);
    return c ? dup_str(p, c) : dup_tok(p, t);
}

#define MSG_TIPO_RETORNO \
    "antes de 'funct' vem o TIPO de retorno — um tipo da linguagem ou o nome " \
    "de uma classe"

/* A CABECA de uma declaracao de funcao, olhando a partir de `off`:
 *
 *     [public|private] {async | <tipo> | static|nonnull}* funct|action|reaction
 *
 * A ordem dos modificadores e do usuario. Devolve o indice do `funct` (>= off)
 * ou -1 se ali nao comeca uma funcao. Um so lugar sabe a forma da cabeca: os
 * quatro pontos que precisavam dela tinham copias a mao, e a que esquecia um
 * modificador deixava a declaracao virar statement solto, calada. */
static int cabeca_de_funct(P *p, int off)
{
    int j = off;
    for (;;) {
        PSToken *mk = espia(p, j);
        if ((mk->type == T_KW && mk->texto
             && (strcmp(mk->texto, "async") == 0
              || strcmp(mk->texto, "public") == 0 || strcmp(mk->texto, "private") == 0))
                || eh_tipo_de_retorno(mk) || eh_mod_funct(mk)) { j++; continue; }
        break;
    }
    return eh_kw_funct(espia(p, j)) ? j : -1;
}

static int aceita(P *p, PSTokType t)
{
    if (checa(p, t)) { p->pos++; return 1; }
    return 0;
}

static int aceita_kw(P *p, const char *kw)
{
    if (checa_kw(p, kw)) { p->pos++; return 1; }
    return 0;
}

static PSToken *exige(P *p, PSTokType t, const char *msg)
{
    if (checa(p, t)) return &p->toks[p->pos++];
    perro(p, msg, atual(p));
    return NULL;
}

/* Fechamento de estrutura aberta — '(' '[' '{'. Token inesperado em OUTRA
 * linha: a culpa é do ABRIDOR (o "faltou ')'" marca a chamada aberta, não o
 * `if` da linha de baixo). Na mesma linha, aponta o token estranho. Espelha
 * o fecha-com-origem. */
static PSToken *exige_fecha(P *p, PSTokType t, const char *msg, PSToken *abre)
{
    if (checa(p, t)) return &p->toks[p->pos++];
    PSToken *tk = atual(p);
    perro(p, msg, (tk->line != abre->line) ? abre : tk);
    return NULL;
}

static void pula_separadores(P *p)
{
    /* Dentro de um grupo (`{ }` de dicionário, `[ ]`, `( )`) a indentação não
     * significa nada: o lexer agora emite INDENT/DEDENT dentro de `{ }`
     * (porque `{` também abre BLOCO), então é aqui que eles são ignorados. */
    for (;;) {
        if (checa(p, T_NEWLINE) || checa(p, T_SEMI)) { p->pos++; continue; }
        if (p->grupo_depth > 0 && (checa(p, T_INDENT) || checa(p, T_DEDENT))) {
            p->pos++; continue;
        }
        /* DEDENT SOLTO entre statements — sempre, nao so dentro de grupo.
         *
         * Fechar o bloco na linha do ULTIMO comando (`i = i + 1 }`) era erro
         * de sintaxe, enquanto `{ i = i + 1 }` numa linha e o `}` sozinho na
         * linha de baixo funcionavam. A regra da chave mudava conforme a forma
         * do bloco, e nada na linguagem justifica isso.
         *
         * A causa: o lexer abre nivel de indentacao pro corpo e fecha na
         * quebra de linha SEGUINTE. Com o `}` sozinho, o DEDENT chega ANTES
         * dele e o laco do bloco o come; com o `}` colado, chega DEPOIS — o
         * bloco ja fechou, e o DEDENT vaza pro nivel de cima. Ali ele nao era
         * pulado, virava `primario()` e dava "expressao invalida" apontando a
         * linha do PROXIMO comando, porque o DEDENT carrega a posicao dela.
         *
         * Statement nenhum comeca com DEDENT: dentro de `{ }` a indentacao nao
         * significa nada (esta escrito em docs/linguagem/01, secao 1.3). O
         * INDENT continua guardado pelo `grupo_depth` porque o bloco `:` do
         * `if __name__ == "main":` — o unico que sobrou — depende dele. */
        if (checa(p, T_DEDENT)) { p->pos++; continue; }
        break;
    }
}

/* Igual ao `pula_separadores`, mas DEIXA o DEDENT.
 *
 * O bloco por `:` (o guard `if __name__ == "main":`) e o unico da linguagem
 * que ainda fecha por indentacao — ele TERMINA quando ve o DEDENT. Se o
 * pulador comer o DEDENT, o laco corre ate o EOF e da "bloco indentado nao foi
 * fechado corretamente". */
static void pula_separadores_com_dedent(P *p)
{
    for (;;) {
        if (checa(p, T_NEWLINE) || checa(p, T_SEMI)) { p->pos++; continue; }
        if (p->grupo_depth > 0 && (checa(p, T_INDENT) || checa(p, T_DEDENT))) {
            p->pos++; continue;
        }
        break;
    }
}

/* nome que vai ser LIGADO: recusa palavra reservada */
static const char *exige_nome(P *p, const char *contexto)
{
    PSToken *t = atual(p);
    if (t->type == T_KW || ((t->type == T_IDENT || t->type == T_IDENT_UPPER)
                            && eh_contextual_reservada(t->texto))) {
        char m[200];
        snprintf(m, sizeof(m),
                 "'%s' e palavra reservada da linguagem e nao pode ser usada como nome de %s",
                 t->texto ? t->texto : "?", contexto);
        perro(p, m, t);
        return NULL;
    }
    if (t->type != T_IDENT && t->type != T_IDENT_UPPER) {
        char m[160];
        snprintf(m, sizeof(m), "esperado nome de %s", contexto);
        perro(p, m, t);
        return NULL;
    }
    p->pos++;
    return dup_tok(p, t);
}

/* Um argumento pode seguir outro SEM vírgula: `post("a" b)` são dois
 * argumentos. É a justaposição da linguagem, e o parser Python a implementa
 * continuando o laço quando o token seguinte puder iniciar expressão. */
/* keywords que valem como NOME em expressão (post, self, list...) — espelho
 * reservadas que ABREM expressão. `if`/`return`/`while` etc. NÃO abrem
 * expressão: sem isso, `f(x` esquecido aberto engolia o `if` da linha de
 * baixo como argumento e o erro saía no lugar errado. */
static int kw_abre_expr(const char *s)
{
    static const char *NOMES[] = {
        "post", "input", "create", "clear", "space", "addEnd", "char", "list",
        "json", "dict", "tup", "JSON", "self",
        "upper", "lower", "replace", "split", "strip", "join", "startswith",
        "endswith", "find", "index", "format", "encode", "decode", "lstrip",
        "rstrip", "title", "capitalize", "not", "Not", NULL
    };
    if (!s) return 0;
    for (int i = 0; NOMES[i]; i++)
        if (strcmp(s, NOMES[i]) == 0) return 1;
    return 0;
}

static int pode_iniciar_expr(PSToken *t)
{
    switch (t->type) {
        case T_INT: case T_FLO: case T_STR: case T_FSTRING:
        case T_BOOL: case T_NULL: case T_COLOR:
        case T_IDENT: case T_IDENT_UPPER:
        case T_LPAREN: case T_LBRACK: case T_LBRACE:
            return 1;
        case T_KW:
            return kw_abre_expr(t->texto);
        case T_OP:
            /* unários que abrem expressão */
            return t->texto && (strcmp(t->texto, "-") == 0 || strcmp(t->texto, "+") == 0
                             || strcmp(t->texto, "!") == 0);
        default:
            return 0;
    }
}

static int eh_tipo_nome(const char *s)
{
    return s && (strcmp(s,"str")==0 || strcmp(s,"int")==0 || strcmp(s,"flo")==0
              || strcmp(s,"bool")==0 || strcmp(s,"list")==0 || strcmp(s,"json")==0
              || strcmp(s,"dict")==0 || strcmp(s,"tup")==0
              || strcmp(s,"type")==0);
}

/* tipos válidos como TypeName numa expressão (`x is int`) */
static int eh_tipo_kw_expr(PSToken *t)
{
    return t->type == T_KW && t->texto
        && (strcmp(t->texto, "str") == 0 || strcmp(t->texto, "int") == 0
         || strcmp(t->texto, "long") == 0
         || strcmp(t->texto, "flo") == 0 || strcmp(t->texto, "bool") == 0
         || strcmp(t->texto, "list") == 0 || strcmp(t->texto, "json") == 0
         || strcmp(t->texto, "dict") == 0 || strcmp(t->texto, "tup") == 0
         || strcmp(t->texto, "type") == 0);
}

/* ── protótipos ─────────────────────────────────────────────────────────── */
static PSNode *expressao(P *p);
static PSNode *e_ou(P *p);
static PSNode *unario(P *p);
static PSNode *potencia(P *p);
static PSNode *primario(P *p);
static PSNode *soma(P *p);
static int count_tipo_valor(P *p, const char **tipo, PSNode **valor);
static int parece_unpack(P *p);
static PSNode *alvos_unpack(P *p);
static int eh_tipo_nome(const char *s);
static int count_esquerda(P *p, PSNode *no, PSToken *tok, const char **tipo, PSNode **valor);
static PSNode *statement(P *p);
static PSNode *bloco(P *p);
static PSNode *bloco_entrada(P *p);

/* ── primário ───────────────────────────────────────────────────────────── */
/* O corpo da lambda, com o cursor no `funct`: `funct(params) { ... }`.
 * Um só lugar monta o nó, chamado dos dois caminhos — o `funct(` pelado e o
 * que vem atrás de modificadores. */
static PSNode *lambda_apos_kw(P *p)
{
    PSToken *t = atual(p);
    p->pos += 2;                               /* funct ( */
    PSNode *n = ps_node_novo(p->arena, N_LAMBDA_EXPR, t->line, t->col);
    if (!n) return NULL;
    while (!checa(p, T_RPAREN)) {
        PSToken *pt = atual(p);
        const char *pn = exige_nome(p, "parametro");
        if (FALHOU(p)) return NULL;
        PSNode *par = ps_node_novo(p->arena, N_NAME, pt->line, pt->col);
        if (!par) return NULL;
        par->texto = pn;
        if (ps_vec_push(p->arena, &n->lista, par) != 0) {
            perro(p, "sem memoria", pt); return NULL;
        }
        if (!aceita(p, T_COMMA)) break;
    }
    if (!exige(p, T_RPAREN, "faltou ')' na lambda")) return NULL;
    n->b = bloco(p);
    if (FALHOU(p)) return NULL;
    return n;
}

static PSNode *primario(P *p)
{
    PSToken *t = atual(p);

    /* LAMBDA COM MODIFICADORES — `async funct(x) { ... }`, `int funct(x) {...}`.
     *
     * Aqui é POSIÇÃO DE EXPRESSÃO: o que vem não é declaração de valor, é uma
     * função sem nome. Quem decide isso é a CABEÇA — a corrida de
     * modificadores terminando em `funct` seguida de `(`. Antes, só o `funct(`
     * pelado era lambda: qualquer modificador na frente jogava a linha no
     * caminho da declaração, que exige nome, e saía `esperado nome de funct`.
     *
     * A checagem é um lookahead que só dispara quando cai em `funct` COM `(`
     * logo depois. Se a corrida não terminar assim, este bloco não faz nada e
     * o resto do parser segue igual — por isso `int x = 1` e `str s = "a"` não
     * são tocados: ali a corrida termina num NOME, não em `funct`. */
    /* `x = action(y) { ... }` — a grafia morta em posição de lambda. Sem esta
     * recusa, `action` virava nome solto, `(y)` uma chamada e o `{` um literal
     * de dicionário: o erro saía como "faltou ':' no dicionario". */
    if (eh_grafia_morta(t) && espia(p, 1)->type == T_LPAREN) {
        char m[200];
        snprintf(m, sizeof(m),
                 "'%s' saiu da linguagem; a funcao sem nome se escreve 'funct(args) { ... }'",
                 t->texto);
        perro(p, m, t);
        return NULL;
    }

    {
        int ifun = cabeca_de_funct(p, 0);
        if (ifun > 0 && espia(p, ifun + 1)->type == T_LPAREN) {
            int is_async = 0, eh_static = 0, eh_nonnull = 0;
            const char *tipo = NULL;
            for (int k = 0; k < ifun; k++) {
                PSToken *m = espia(p, k);
                if (m->type == T_KW && m->texto && strcmp(m->texto, "async") == 0) is_async = 1;
                else if (eh_tipo_de_retorno(m)) tipo = tipo_retorno_dup(p, m);
                else if (eh_mod_funct(m)) {
                    if (strcmp(m->texto, "static") == 0) eh_static = 1; else eh_nonnull = 1;
                }
                /* `public`/`private` numa lambda não ligam nada: não há dono
                 * pra esconder. Consumidos e ignorados, sem erro. */
            }
            p->pos += ifun;                    /* consome os modificadores */
            PSNode *n = lambda_apos_kw(p);
            if (!n) return NULL;
            n->is_async = is_async;
            n->texto2 = tipo;
            n->is_static = eh_static;
            n->is_nonnull = eh_nonnull;
            return n;
        }
    }

    switch (t->type) {
        case T_INT: {
            p->pos++;
            PSNode *n = ps_node_novo(p->arena, N_LITERAL, t->line, t->col);
            if (!n) return NULL;
            /* Literal maior que int64 vira bignum: o lexer trava em INT64_MAX,
             * então re-checo o texto original com errno. */
            errno = 0;
            (void)strtoll(t->texto ? t->texto : "0", NULL, 10);
            if (errno == ERANGE && t->texto) { n->lit = L_BIGINT; n->texto = dup_tok(p, t); }
            else                             { n->lit = L_INT;    n->i = t->i; }
            return n;
        }
        case T_FLO: {
            p->pos++;
            PSNode *n = ps_node_novo(p->arena, N_LITERAL, t->line, t->col);
            if (!n) return NULL;
            n->lit = L_FLO; n->d = t->d;
            return n;
        }
        case T_STR: {
            p->pos++;
            PSNode *lit = ps_node_novo(p->arena, N_LITERAL, t->line, t->col);
            if (!lit) return NULL;
            lit->lit = L_STR;
            lit->texto = ps_arena_strdup(p->arena, t->texto ? t->texto : "", t->texto_len);
            lit->texto_len = t->texto_len;
            /* `"texto" {expr} "mais"` — interpolação por chaves */
            if (!checa(p, T_LBRACE)) return lit;
            if (p->chave_abre_bloco && p->grupo_depth == 0) return lit;

            PSNode *n = ps_node_novo(p->arena, N_INTERPOLATED_STRING, t->line, t->col);
            if (!n) return NULL;
            if (ps_vec_push(p->arena, &n->lista, lit) != 0) return NULL;
            while (aceita(p, T_LBRACE)) {
                PSNode *e = expressao(p);
                if (FALHOU(p)) return NULL;
                if (!exige(p, T_RBRACE, "faltou '}' na interpolacao")) return NULL;
                if (ps_vec_push(p->arena, &n->lista, e) != 0) return NULL;
                if (checa(p, T_STR)) {
                    PSToken *st = atual(p);
                    p->pos++;
                    PSNode *l2 = ps_node_novo(p->arena, N_LITERAL, st->line, st->col);
                    if (!l2) return NULL;
                    l2->lit = L_STR;
                    l2->texto = ps_arena_strdup(p->arena, st->texto ? st->texto : "", st->texto_len);
                    l2->texto_len = st->texto_len;
                    if (ps_vec_push(p->arena, &n->lista, l2) != 0) return NULL;
                }
            }
            return n;
        }
        case T_FSTRING: {
            /* f-string continua sendo Literal, com kind FSTRING: a
             * interpolação é resolvida em tempo de execução, não aqui —
             * é a regra. */
            p->pos++;
            PSNode *n = ps_node_novo(p->arena, N_LITERAL, t->line, t->col);
            if (!n) return NULL;
            n->lit = L_FSTRING;
            n->texto = ps_arena_strdup(p->arena, t->texto ? t->texto : "", t->texto_len);
            n->texto_len = t->texto_len;
            return n;
        }
        case T_BOOL: {
            p->pos++;
            PSNode *n = ps_node_novo(p->arena, N_LITERAL, t->line, t->col);
            if (!n) return NULL;
            n->lit = L_BOOL; n->i = t->i;
            return n;
        }
        case T_NULL: {
            p->pos++;
            PSNode *n = ps_node_novo(p->arena, N_LITERAL, t->line, t->col);
            if (!n) return NULL;
            n->lit = L_NULL;
            return n;
        }
        case T_IDENT:
        case T_IDENT_UPPER: {
            /* `base(...)` — chamada do __init__ do pai. `base` não é keyword
             * global (é contextual), por isso chega aqui como IDENT. */
            if (t->texto && strcmp(t->texto, "base") == 0
                    && espia(p, 1)->type == T_LPAREN) {
                p->pos += 2;
                PSNode *n = ps_node_novo(p->arena, N_BASE_CALL_NODE, t->line, t->col);
                if (!n) return NULL;
                if (!checa(p, T_RPAREN)) {
                    for (;;) {
                        PSToken *at = atual(p);
                        const char *nome_arg = NULL;
                        if ((at->type == T_IDENT || at->type == T_IDENT_UPPER)
                                && espia(p, 1)->type == T_OP && espia(p, 1)->texto
                                && strcmp(espia(p, 1)->texto, "=") == 0
                                && espia(p, 2)->type != T_COMMA
                                && espia(p, 2)->type != T_RPAREN) {
                            nome_arg = dup_tok(p, at);
                            p->pos += 2;
                        }
                        PSNode *v = expressao(p);
                        if (FALHOU(p)) return NULL;
                        PSNode *arg = ps_node_novo(p->arena, N_CALL_ARG, t->line, t->col);
                        if (!arg) return NULL;
                        arg->a = v; arg->texto = nome_arg;
                        if (ps_vec_push(p->arena, &n->lista, arg) != 0) return NULL;
                        if (!aceita(p, T_COMMA)) break;
                    }
                }
                if (!exige(p, T_RPAREN, "faltou ')' em base()")) return NULL;
                return n;
            }
            /* `base` sem `(` não é nome válido — mesmo erro do interp */
            if (t->texto && strcmp(t->texto, "base") == 0) {
                perro(p, "esperado '(' após 'base'", t);
                return NULL;
            }
            p->pos++;
            PSNode *n = ps_node_novo(p->arena, N_NAME, t->line, t->col);
            if (!n) return NULL;
            n->texto = dup_tok(p, t);
            return n;
        }
        case T_LPAREN: {
            p->pos++;
            p->grupo_depth++;
            /* `()` — tupla vazia */
            if (checa(p, T_RPAREN)) {
                p->pos++;
                p->grupo_depth--;
                PSNode *n = ps_node_novo(p->arena, N_TUPLE_LITERAL, t->line, t->col);
                return n;
            }
            PSNode *e = expressao(p);
            if (FALHOU(p)) { p->grupo_depth--; return NULL; }
            /* vírgula depois da primeira expressão => tupla, não agrupamento */
            if (aceita(p, T_COMMA)) {
                PSNode *n = ps_node_novo(p->arena, N_TUPLE_LITERAL, t->line, t->col);
                if (!n) return NULL;
                if (ps_vec_push(p->arena, &n->lista, e) != 0) return NULL;
                while (!checa(p, T_RPAREN)) {
                    PSNode *it = expressao(p);
                    if (FALHOU(p)) { p->grupo_depth--; return NULL; }
                    if (ps_vec_push(p->arena, &n->lista, it) != 0) { p->grupo_depth--; return NULL; }
                    if (!aceita(p, T_COMMA)) break;
                }
                p->grupo_depth--;
                if (!exige_fecha(p, T_RPAREN, "faltou ')' na tupla", t)) return NULL;
                return n;
            }
            p->grupo_depth--;
            if (!exige_fecha(p, T_RPAREN, "faltou ')'", t)) return NULL;
            return e;
        }
        case T_LBRACK: {
            p->pos++;
            p->grupo_depth++;
            PSNode *n = ps_node_novo(p->arena, N_LIST_LITERAL, t->line, t->col);
            if (!n) { p->grupo_depth--; return NULL; }
            pula_separadores(p);
            if (!checa(p, T_RBRACK)) {
                for (;;) {
                    pula_separadores(p);
                    PSNode *item = expressao(p);
                    if (FALHOU(p)) return NULL;
                    /* COMPREENSÃO: `[<expr> for each <n> in <it> (if <c>)?]`.
                     * Só cabe depois do PRIMEIRO item e no lugar da vírgula —
                     * `[a, b for each ...]` não é forma nenhuma. */
                    if (n->lista.n == 0 && checa_kw(p, "for")) {
                        p->pos++;
                        if (!aceita_kw(p, "each")) {
                            perro(p, "esperado 'each' depois de 'for' na compreensao", atual(p));
                            return NULL;
                        }
                        const char *var = exige_nome(p, "variavel da compreensao");
                        if (FALHOU(p)) return NULL;
                        if (!aceita_kw(p, "in")) {
                            perro(p, "esperado 'in' na compreensao de lista", atual(p));
                            return NULL;
                        }
                        PSNode *lc = ps_node_novo(p->arena, N_LIST_COMP, t->line, t->col);
                        if (!lc) return NULL;
                        lc->texto = var;
                        lc->b = item;
                        lc->a = expressao(p);
                        if (FALHOU(p)) return NULL;
                        if (checa_kw(p, "if")) {
                            p->pos++;
                            lc->c = expressao(p);
                            if (FALHOU(p)) return NULL;
                        }
                        pula_separadores(p);
                        p->grupo_depth--;
                        if (!exige_fecha(p, T_RBRACK, "faltou ']' na compreensao", t)) return NULL;
                        return lc;
                    }
                    if (ps_vec_push(p->arena, &n->lista, item) != 0) {
                        perro(p, "sem memoria", t); return NULL;
                    }
                    pula_separadores(p);
                    if (!aceita(p, T_COMMA)) break;
                    pula_separadores(p);
                    if (checa(p, T_RBRACK)) break;     /* vírgula final */
                }
            }
            pula_separadores(p);
            p->grupo_depth--;
            if (!exige_fecha(p, T_RBRACK, "faltou ']' na lista", t)) return NULL;
            return n;
        }
        case T_LBRACE: {
            p->pos++;
            p->grupo_depth++;
            PSNode *n = ps_node_novo(p->arena, N_DICT_LITERAL, t->line, t->col);
            if (!n) { p->grupo_depth--; return NULL; }
            pula_separadores(p);
            if (!checa(p, T_RBRACE)) {
                for (;;) {
                    pula_separadores(p);
                    PSToken *kt = atual(p);
                    PSNode *chave;
                    if (kt->type == T_STR) {
                        p->pos++;
                        chave = ps_node_novo(p->arena, N_LITERAL, kt->line, kt->col);
                        if (!chave) return NULL;
                        chave->lit = L_STR;
                        chave->texto = ps_arena_strdup(p->arena, kt->texto ? kt->texto : "",
                                                       kt->texto_len);
                    } else if (kt->type == T_IDENT || kt->type == T_IDENT_UPPER) {
                        /* Chave sem aspas (`{nome: 1}`) fica como Name, NÃO
                         * como string — quem converte
                         * pra string é o compilador. Emitir Literal aqui
                         * geraria uma AST diferente da de referência. */
                        p->pos++;
                        chave = ps_node_novo(p->arena, N_NAME, kt->line, kt->col);
                        if (!chave) return NULL;
                        chave->texto = dup_tok(p, kt);
                    } else {
                        /* Qualquer expressão serve de chave: `d[1] = "a"`
                         * sempre valeu, então `{1: "a"}` também tem que valer. */
                        chave = expressao(p);
                        if (FALHOU(p)) return NULL;
                    }
                    if (!exige(p, T_COLON, "faltou ':' no dicionario")) return NULL;
                    PSNode *valor = expressao(p);
                    if (FALHOU(p)) return NULL;

                    PSNode *ent = ps_node_novo(p->arena, N_DICT_ENTRY, kt->line, kt->col);
                    if (!ent) return NULL;
                    ent->a = chave; ent->b = valor;
                    if (ps_vec_push(p->arena, &n->lista, ent) != 0) {
                        perro(p, "sem memoria", kt); return NULL;
                    }
                    pula_separadores(p);
                    if (!aceita(p, T_COMMA)) break;
                    pula_separadores(p);
                    if (checa(p, T_RBRACE)) break;
                }
            }
            pula_separadores(p);
            p->grupo_depth--;
            if (!exige_fecha(p, T_RBRACE, "faltou '}' no dicionario", t)) return NULL;
            return n;
        }
        case T_COLOR: {
            /* <cor>"texto" — a cor decora a expressão string seguinte */
            p->pos++;
            PSNode *alvo = primario(p);
            if (FALHOU(p)) return NULL;
            PSNode *n = ps_node_novo(p->arena, N_COLOR_STR_EXPR, t->line, t->col);
            if (!n) return NULL;
            n->texto = dup_tok(p, t);
            n->a = alvo;
            return n;
        }
        case T_KW: {
            /* `to <tipo>` — açúcar do Parsing: vira a STRING com o nome do tipo
             * (ex: `Parsing.string(x, to int)` == `Parsing.string(x, "int")`).
             */
            if (strcmp(t->texto, "to") == 0) {
                PSToken *tt = espia(p, 1);
                static const char *TIPOS[] = {"int","float","str","flo","bool","json","list","tup","dict"};
                int ok = 0;
                if (tt->texto)
                    for (size_t i = 0; i < sizeof(TIPOS)/sizeof(TIPOS[0]); i++)
                        if (strcmp(tt->texto, TIPOS[i]) == 0) { ok = 1; break; }
                if (!ok) { perro(p, "esperado 'int', 'float' ou 'str' apos 'to'", tt); return NULL; }
                p->pos += 2;                       /* to <tipo> */
                PSNode *n = ps_node_novo(p->arena, N_LITERAL, t->line, t->col);
                if (!n) return NULL;
                n->lit = L_STR;
                n->texto = dup_tok(p, tt);         /* string com o nome do tipo */
                return n;
            }
            /* lambda: `funct(params) { ... }` como expressão */
            if (eh_kw_funct(t) && espia(p, 1)->type == T_LPAREN)
                return lambda_apos_kw(p);
            /* `count <tipo>[(<v>)] in <cont>` e `count each ...` como expressão */
            if (strcmp(t->texto, "count") == 0) {
                int eh_each = (espia(p, 1)->type == T_KW && espia(p, 1)->texto
                               && strcmp(espia(p, 1)->texto, "each") == 0);
                p->pos++;
                if (eh_each) p->pos++;
                const char *tipo; PSNode *valor;
                if (count_tipo_valor(p, &tipo, &valor) != 0) return NULL;
                if (!aceita_kw(p, "in")) {
                    perro(p, "esperado 'in' apos o tipo de 'count'", atual(p)); return NULL;
                }
                PSNode *cont = soma(p);          /* parse_add no Python */
                if (FALHOU(p)) return NULL;
                PSNode *n = ps_node_novo(p->arena,
                                         eh_each ? N_COUNT_EACH_EXPR : N_COUNT_EXPR,
                                         t->line, t->col);
                if (!n) return NULL;
                n->texto = tipo;
                n->b = valor;
                n->c = cont;
                if (!eh_each) n->texto2 = dup_str(p, valor ? "prefix_value" : "prefix_type");
                return n;
            }
            /* `await expr` */
            if (strcmp(t->texto, "await") == 0) {
                p->pos++;
                PSNode *v = unario(p);
                if (FALHOU(p)) return NULL;
                PSNode *n = ps_node_novo(p->arena, N_AWAIT_EXPR, t->line, t->col);
                if (!n) return NULL;
                n->a = v;
                return n;
            }
            /* keyword de TIPO isolada vira TypeName; seguida de '(' ou '.'
             * é chamada/módulo e continua sendo Name (`int(x)`, `json.parse`) */
            if (eh_tipo_kw_expr(t) && espia(p, 1)->type != T_LPAREN
                    && espia(p, 1)->type != T_DOT) {
                p->pos++;
                PSNode *n = ps_node_novo(p->arena, N_TYPE_NAME, t->line, t->col);
                if (!n) return NULL;
                n->texto = dup_tok(p, t);
                return n;
            }
            /* demais keywords válidas como expressão (post, input, self, base...) */
            p->pos++;
            PSNode *n = ps_node_novo(p->arena, N_NAME, t->line, t->col);
            if (!n) return NULL;
            n->texto = dup_tok(p, t);
            return n;
        }
        default:
            /* `// nota` no lugar de uma expressão: quase sempre é comentário
             * de código escrito antes do I11, quando `//` COMENTAVA. Hoje ele
             * é divisão inteira, e sem operando à esquerda dava só
             * "expressao invalida" — a mensagem certa pro parser e inútil pra
             * quem escreveu. Um arquivo inteiro pode cair por causa disto (e
             * caiu: uma lib instalada, com o erro aparecendo na linha do
             * `import` de quem a usava). */
            if (t->type == T_OP && t->texto && strcmp(t->texto, "//") == 0) {
                perro(p, "'//' e divisao inteira, nao comentario — comentario e '#' "
                         "(ou bloco entre tres aspas)", t);
                return NULL;
            }
            perro(p, "expressao invalida", t);
            return NULL;
    }
}

/* ── posfixo: chamada, .membro, [índice] ────────────────────────────────── */
static PSNode *posfixo(P *p)
{
    PSNode *no = primario(p);
    if (FALHOU(p)) return NULL;

    for (;;) {
        PSToken *t = atual(p);

        if (checa(p, T_LPAREN)) {
            p->pos++;
            p->grupo_depth++;
            PSNode *c = ps_node_novo(p->arena, N_CALL, t->line, t->col);
            if (!c) { p->grupo_depth--; return NULL; }
            c->a = no;
            pula_separadores(p);
            if (!checa(p, T_RPAREN)) {
                for (;;) {
                    pula_separadores(p);
                    PSToken *at = atual(p);
                    const char *nome_arg = NULL;
                    /* argumento nomeado: NOME '=' valor. O nome é só um
                     * rótulo, então keyword é aceita aqui (regex.sub(count=2)). */
                    if ((at->type == T_IDENT || at->type == T_IDENT_UPPER || at->type == T_KW)
                            && espia(p, 1)->type == T_OP
                            && espia(p, 1)->texto && strcmp(espia(p, 1)->texto, "=") == 0) {
                        nome_arg = dup_tok(p, at);
                        p->pos += 2;
                    }
                    PSNode *valor = expressao(p);
                    if (FALHOU(p)) return NULL;
                    /* COMPREENSÃO como argumento: `post(n * 2 for each n in l)`
                     * — a forma do Python, sem os colchetes. Só vale como
                     * argumento ÚNICO e sem nome, que é onde ela não é
                     * ambígua com uma lista de argumentos. */
                    if (c->lista.n == 0 && nome_arg == NULL && checa_kw(p, "for")) {
                        p->pos++;
                        if (!aceita_kw(p, "each")) {
                            perro(p, "esperado 'each' depois de 'for' na compreensao", atual(p));
                            return NULL;
                        }
                        const char *cvar = exige_nome(p, "variavel da compreensao");
                        if (FALHOU(p)) return NULL;
                        if (!aceita_kw(p, "in")) {
                            perro(p, "esperado 'in' na compreensao", atual(p));
                            return NULL;
                        }
                        PSNode *lc = ps_node_novo(p->arena, N_LIST_COMP, at->line, at->col);
                        if (!lc) return NULL;
                        lc->texto = cvar;
                        lc->b = valor;
                        lc->a = expressao(p);
                        if (FALHOU(p)) return NULL;
                        if (checa_kw(p, "if")) {
                            p->pos++;
                            lc->c = expressao(p);
                            if (FALHOU(p)) return NULL;
                        }
                        valor = lc;
                    }
                    /* `f(a, x for each x in l)` — ambíguo: não dá pra saber se
                     * a compreensão é um argumento ou se falta um `)`. Python
                     * também recusa; a mensagem diz o conserto. */
                    else if (checa_kw(p, "for")) {
                        perro(p, "compreensao so vale como argumento unico — ponha entre colchetes: [x for each ...]",
                              atual(p));
                        return NULL;
                    }
                    PSNode *arg = ps_node_novo(p->arena, N_CALL_ARG, at->line, at->col);
                    if (!arg) return NULL;
                    arg->a = valor;
                    arg->texto = nome_arg;
                    if (ps_vec_push(p->arena, &c->lista, arg) != 0) {
                        perro(p, "sem memoria", at); return NULL;
                    }
                    pula_separadores(p);
                    if (aceita(p, T_COMMA)) {
                        pula_separadores(p);
                        if (checa(p, T_RPAREN)) break;
                        continue;
                    }
                    if (checa(p, T_RPAREN)) break;
                    /* Um `{` (dicionário) NUNCA justapõe: sem vírgula antes
                     * dele é vírgula esquecida, não um novo argumento. Deixar
                     * justapor fazia o dict virar "argumento a mais" silencioso
                     * ("argumentos demais"), que não faz sentido. Erro claro
                     * apontando a vírgula que falta. */
                    if (checa(p, T_LBRACE)) {
                        perro(p, "faltou ',' antes do dicionario '{' — cada argumento precisa de virgula", atual(p));
                        return NULL;
                    }
                    /* sem vírgula: só continua se o próximo puder abrir
                     * expressão — é a justaposição `post("a" b)` */
                    if (pode_iniciar_expr(atual(p))) continue;
                    break;
                }
            }
            pula_separadores(p);
            p->grupo_depth--;
            if (!exige_fecha(p, T_RPAREN, "faltou ')' na chamada", t)) return NULL;
            no = c;
            continue;
        }

        if (checa(p, T_DOT)) {
            p->pos++;
            PSToken *mt = atual(p);
            if (mt->type != T_IDENT && mt->type != T_IDENT_UPPER && mt->type != T_KW) {
                perro(p, "esperado nome do membro apos '.'", mt);
                return NULL;
            }
            p->pos++;
            PSNode *m = ps_node_novo(p->arena, N_MEMBER_ACCESS, t->line, t->col);
            if (!m) return NULL;
            m->a = no;
            m->texto = dup_tok(p, mt);
            no = m;
            continue;
        }

        if (checa(p, T_LBRACK)) {
            p->pos++;
            /* Distingue índice de slice: `[i]` vs `[ini:fim:passo]`, com
             * qualquer parte podendo faltar (`[:2]`, `[::-1]`, `[1:]`). */
            PSNode *inicio = NULL, *fim = NULL, *passo = NULL;
            int eh_slice = 0;

            if (checa(p, T_COLON)) {
                eh_slice = 1;
                p->pos++;
                if (!checa(p, T_RBRACK) && !checa(p, T_COLON)) {
                    fim = expressao(p);
                    if (FALHOU(p)) return NULL;
                }
                if (aceita(p, T_COLON)) {
                    if (!checa(p, T_RBRACK)) {
                        passo = expressao(p);
                        if (FALHOU(p)) return NULL;
                    }
                }
            } else {
                PSNode *primeiro = expressao(p);
                if (FALHOU(p)) return NULL;
                if (checa(p, T_COLON)) {
                    eh_slice = 1;
                    inicio = primeiro;
                    p->pos++;
                    if (!checa(p, T_RBRACK) && !checa(p, T_COLON)) {
                        fim = expressao(p);
                        if (FALHOU(p)) return NULL;
                    }
                    if (aceita(p, T_COLON)) {
                        if (!checa(p, T_RBRACK)) {
                            passo = expressao(p);
                            if (FALHOU(p)) return NULL;
                        }
                    }
                } else {
                    inicio = primeiro;
                }
            }
            if (!exige_fecha(p, T_RBRACK, "faltou ']' no indice", t)) return NULL;

            if (eh_slice) {
                PSNode *sl = ps_node_novo(p->arena, N_SLICE_ACCESS, no->line, no->col);
                if (!sl) return NULL;
                sl->a = no; sl->b = inicio; sl->c = fim; sl->e = passo;
                no = sl;
            } else {
                PSNode *ix = ps_node_novo(p->arena, N_INDEX_ACCESS, t->line, t->col);
                if (!ix) return NULL;
                ix->a = no; ix->b = inicio;
                no = ix;
            }
            continue;
        }
        /* `x++` / `x--` — posfixo, só em variável */
        if (t->type == T_OP && t->texto
                && (strcmp(t->texto, "++") == 0 || strcmp(t->texto, "--") == 0)) {
            p->pos++;
            PSNode *n = ps_node_novo(p->arena, N_POSTFIX_OP, t->line, t->col);
            if (!n) return NULL;
            n->a = no;
            n->texto = dup_tok(p, t);
            no = n;
            continue;
        }
        break;
    }
    return no;
}

/* ── unário ─────────────────────────────────────────────────────────────── */
static PSNode *unario(P *p)
{
    PSToken *t = atual(p);
    if (t->type == T_OP && t->texto
            && (strcmp(t->texto, "-") == 0 || strcmp(t->texto, "+") == 0
                || strcmp(t->texto, "~") == 0)) {
        p->pos++;
        PSNode *operando = unario(p);
        if (FALHOU(p)) return NULL;
        PSNode *n = ps_node_novo(p->arena, N_UNARY_OP, t->line, t->col);
        if (!n) return NULL;
        n->texto = dup_tok(p, t);
        n->a = operando;
        return n;
    }
    return potencia(p);
}

/* `**` — I11. A precedencia dele nao cabe na cadeia normal, e a regra e a do
 * Python:
 *
 *   - liga mais FORTE que o unario a ESQUERDA:  -2 ** 2  ==  -(2 ** 2)  == -4
 *   - liga mais FRACO que o unario a DIREITA:    2 ** -1  ==  2 ** (-1)
 *   - e associa a DIREITA:                    2 ** 3 ** 2 == 2 ** (3 ** 2)
 *
 * Por isso ele fica ENTRE o unario e o posfixo, e o operando da direita volta
 * pelo `unario`: e isso que produz os tres comportamentos de uma vez.
 */
static PSNode *potencia(P *p)
{
    PSNode *base = posfixo(p);
    if (FALHOU(p)) return NULL;
    PSToken *t = atual(p);
    if (t->type == T_OP && t->texto && strcmp(t->texto, "**") == 0) {
        p->pos++;
        PSNode *expo = unario(p);          /* direita: pega o unario junto */
        if (FALHOU(p)) return NULL;
        PSNode *n = ps_node_novo(p->arena, N_BINARY_OP, t->line, t->col);
        if (!n) return NULL;
        n->texto = dup_tok(p, t);
        n->a = base; n->b = expo;
        return n;
    }
    return base;
}

/* ── cadeia binária, do mais forte pro mais fraco ───────────────────────── */
static PSNode *bin_no(P *p, PSToken *op, PSNode *e, PSNode *d)
{
    PSNode *n = ps_node_novo(p->arena, N_BINARY_OP, op->line, op->col);
    if (!n) return NULL;
    n->texto = dup_tok(p, op);
    n->a = e; n->b = d;
    return n;
}

#define NIVEL_BIN(nome, proximo, cond)                       \
    static PSNode *nome(P *p) {                              \
        PSNode *no = proximo(p);                             \
        if (FALHOU(p)) return NULL;                          \
        for (;;) {                                           \
            PSToken *t = atual(p);                           \
            if (!(cond)) break;                              \
            p->pos++;                                        \
            PSNode *d = proximo(p);                          \
            if (FALHOU(p)) return NULL;                      \
            no = bin_no(p, t, no, d);                        \
            if (!no) return NULL;                            \
        }                                                    \
        return no;                                           \
    }

static int op_eh(PSToken *t, const char *s)
{
    return t->type == T_OP && t->texto && strcmp(t->texto, s) == 0;
}

/* `//` (divisao inteira) tem a MESMA precedencia de `/` e `%`, como no
 * Python — I11. */
NIVEL_BIN(mul,    unario, op_eh(t, "*") || op_eh(t, "/") || op_eh(t, "%")
                       || op_eh(t, "//"))
NIVEL_BIN(soma,   mul,    op_eh(t, "+") || op_eh(t, "-"))
NIVEL_BIN(shift,  soma,   op_eh(t, "<<") || op_eh(t, ">>"))
NIVEL_BIN(bitand_, shift, op_eh(t, "&"))
NIVEL_BIN(bitxor_, bitand_, op_eh(t, "^"))
NIVEL_BIN(bitor_,  bitxor_, op_eh(t, "|"))

/* comparação: além dos operadores, aceita `is`, `in`, `not is`, `not in` */
static PSNode *comparacao(P *p)
{
    PSNode *no = bitor_(p);
    if (FALHOU(p)) return NULL;

    for (;;) {
        PSToken *t = atual(p);

        /* forma INFIXA: `int(7) count in nums` */
        if (checa_kw(p, "count") && espia(p, 1)->type == T_KW
                && espia(p, 1)->texto && strcmp(espia(p, 1)->texto, "in") == 0) {
            const char *tipo; PSNode *valor;
            if (count_esquerda(p, no, t, &tipo, &valor) != 0) return NULL;
            p->pos += 2;                       /* count in */
            PSNode *cont = soma(p);
            if (FALHOU(p)) return NULL;
            PSNode *n = ps_node_novo(p->arena, N_COUNT_EXPR, t->line, t->col);
            if (!n) return NULL;
            n->texto = tipo; n->texto2 = dup_str(p, "infix");
            n->b = valor; n->c = cont;
            no = n;
            continue;
        }

        /* `===` e `!==` SAÍRAM da linguagem (29/08).
         *
         * Eles nunca foram igualdade estrita: compilavam pro MESMO opcode do
         * `==`, então `false === Null` dava True junto com `false == Null`.
         * Quem escrevia `x === Null` acreditando estar protegido do valor
         * coagido estava rodando exatamente a comparação frouxa — nome de uma
         * coisa, comportamento de outra.
         *
         * O lexer ainda RECONHECE os dois, e é de propósito: sem isso,
         * `a === b` viraria `a == (= b)` e o erro sairia falando de outra
         * coisa. Reconhecer pra recusar dizendo o motivo é o que ensina. */
        if (t->type == T_OP && t->texto
                && (strcmp(t->texto, "===") == 0 || strcmp(t->texto, "!==") == 0)) {
            char m[96];
            snprintf(m, sizeof(m),
                     "`%s` nao existe nesta linguagem; use `%s`",
                     t->texto, t->texto[0] == '=' ? "==" : "!=");
            perro(p, m, t);
            return NULL;
        }

        if (t->type == T_OP && t->texto
                && (strcmp(t->texto, "==") == 0 || strcmp(t->texto, "!=") == 0
                 || strcmp(t->texto, "<") == 0 || strcmp(t->texto, ">") == 0
                 || strcmp(t->texto, "<=") == 0 || strcmp(t->texto, ">=") == 0)) {
            p->pos++;
            PSNode *d = bitor_(p);
            if (FALHOU(p)) return NULL;
            no = bin_no(p, t, no, d);
            if (!no) return NULL;
            continue;
        }

        if (checa_kw(p, "is") || checa_kw(p, "in")) {
            PSToken *op = t;
            p->pos++;
            const char *nome_op = dup_tok(p, op);
            if (strcmp(op->texto, "is") == 0 && (checa_kw(p, "not") || checa_kw(p, "Not"))) {
                p->pos++;
                nome_op = dup_str(p, "is not");
            }
            PSNode *d = bitor_(p);
            if (FALHOU(p)) return NULL;
            PSNode *n = ps_node_novo(p->arena, N_BINARY_OP, op->line, op->col);
            if (!n) return NULL;
            n->texto = nome_op; n->a = no; n->b = d;
            /* forma SUFIXA: `int in lista count` */
            if (strcmp(op->texto, "in") == 0 && checa_kw(p, "count")) {
                PSToken *ct = atual(p);
                const char *tipo; PSNode *valor;
                if (count_esquerda(p, n->a, ct, &tipo, &valor) != 0) return NULL;
                p->pos++;
                PSNode *ce = ps_node_novo(p->arena, N_COUNT_EXPR, ct->line, ct->col);
                if (!ce) return NULL;
                ce->texto = tipo; ce->texto2 = dup_str(p, "suffix");
                ce->b = valor; ce->c = n->b;
                no = ce;
                continue;
            }
            no = n;
            continue;
        }

        if ((checa_kw(p, "not") || checa_kw(p, "Not"))
                && (espia(p, 1)->type == T_KW && espia(p, 1)->texto
                    && (strcmp(espia(p, 1)->texto, "is") == 0
                     || strcmp(espia(p, 1)->texto, "in") == 0))) {
            PSToken *op = t;
            p->pos++;
            const char *seg = atual(p)->texto;
            p->pos++;
            PSNode *d = bitor_(p);
            if (FALHOU(p)) return NULL;
            PSNode *n = ps_node_novo(p->arena, N_BINARY_OP, op->line, op->col);
            if (!n) return NULL;
            n->texto = dup_str(p, (strcmp(seg, "is") == 0) ? "is not" : "not in");
            n->a = no; n->b = d;
            no = n;
            continue;
        }
        break;
    }
    return no;
}

static PSNode *nao(P *p)
{
    PSToken *t = atual(p);
    if (checa_kw(p, "not") || checa_kw(p, "Not") || op_eh(t, "!")) {
        p->pos++;
        PSNode *operando = nao(p);
        if (FALHOU(p)) return NULL;
        PSNode *n = ps_node_novo(p->arena, N_UNARY_OP, t->line, t->col);
        if (!n) return NULL;
        n->texto = dup_tok(p, t);
        n->a = operando;
        return n;
    }
    return comparacao(p);
}

static PSNode *e_logico(P *p)
{
    PSNode *no = nao(p);
    if (FALHOU(p)) return NULL;
    for (;;) {
        PSToken *t = atual(p);
        if (!(checa_kw(p, "and") || op_eh(t, "&&"))) break;
        p->pos++;
        PSNode *d = nao(p);
        if (FALHOU(p)) return NULL;
        no = bin_no(p, t, no, d);
        if (!no) return NULL;
    }
    return no;
}

static PSNode *e_ou(P *p)
{
    PSNode *no = e_logico(p);
    if (FALHOU(p)) return NULL;
    for (;;) {
        PSToken *t = atual(p);
        if (!(checa_kw(p, "or") || op_eh(t, "||"))) break;
        p->pos++;
        PSNode *d = e_logico(p);
        if (FALHOU(p)) return NULL;
        no = bin_no(p, t, no, d);
        if (!no) return NULL;
    }
    return no;
}

/* Topo da expressão: nível `or` + condicional inline (ternário Python)
 * `A if cond else B`. `if` só é ternário DEPOIS de uma expressão — no início
 * de statement ele já foi despachado como `if` statement, sem ambiguidade. */
/* Corpo real; a casca `expressao` conta a profundidade. */
static PSNode *expressao_no(P *p);

/* Casca que conta a descida. O corpo tem muitos `return`, e um contador
 * espalhado por todos eles é convite a esquecer um — aqui entrar e sair sempre
 * fecham. Ver PS_PARSE_PROF_MAX. */
static PSNode *expressao(P *p)
{
    if (p->prof >= PS_PARSE_PROF_MAX || ps_pilha_apertada()) {
        perro(p, "expressao aninhada demais", atual(p));
        return NULL;
    }
    p->prof++;
    PSNode *r = expressao_no(p);
    p->prof--;
    return r;
}

static PSNode *expressao_no(P *p)
{
    PSNode *no = e_ou(p);
    if (FALHOU(p)) return NULL;
    if (checa_kw(p, "if")) {
        PSToken *t = atual(p);
        int32_t salvo = p->pos;
        p->pos++;                       /* consome 'if' */
        PSNode *cond = e_ou(p);         /* cond = nível or (sem ternário aninhado) */
        if (FALHOU(p)) return NULL;
        /* O `else` desambigua: dentro de `{ }` não há NEWLINE entre statements,
         * então `x = A` seguido de `if cond { ... }` chega como `A if cond {`.
         * Só é ternário se vier `else`; senão o `if` inicia um statement e é
         * devolvido intacto (backtrack) pro parser de bloco. */
        if (checa_kw(p, "else")) {
            p->pos++;                   /* consome 'else' */
            PSNode *bfalso = expressao(p);  /* else à direita: permite encadear */
            if (FALHOU(p)) return NULL;
            PSNode *n = ps_node_novo(p->arena, N_CONDITIONAL, t->line, t->col);
            if (!n) return NULL;
            n->a = no; n->b = cond; n->c = bfalso;
            return n;
        }
        p->pos = salvo;                 /* não era ternário: é um `if` statement */
    }
    return no;
}

/* ── blocos ─────────────────────────────────────────────────────────────── */
/* Separadores + indentação solta: só o bloco de CHAVES usa, porque nele a
 * indentação não delimita nada. */
static void pula_indent_solto(P *p)
{
    while (checa(p, T_NEWLINE) || checa(p, T_SEMI)
           || checa(p, T_INDENT) || checa(p, T_DEDENT)) p->pos++;
}

/* Bloco do GUARD (`if __name__ == "main":`) — o ÚNICO lugar da linguagem onde
 * `:` + indentação ainda abre bloco. Todo o resto usa `{ }` (ver `bloco`).
 *
 * A exceção é deliberada: o guard é a última linha de quase todo programa e a
 * forma com dois-pontos é a que se escreve. As chaves continuam valendo aqui
 * também.
 *
 * Este comentário dizia `run_selfwith_("main"):`, que NÃO EXISTE MAIS na
 * linguagem — o próprio motor responde "run_selfwith_ nao existe mais — use:
 * if __name__ == \"main\"". A doc do parser apontava pra uma construção
 * removida. */
static PSNode *bloco_entrada(P *p)
{
    PSToken *t = atual(p);
    if (!checa(p, T_COLON)) return bloco(p);
    p->pos++;
    if (!exige(p, T_NEWLINE, "faltou quebra de linha apos ':'")) return NULL;
    while (checa(p, T_NEWLINE)) p->pos++;   /* comentário/linha vazia depois do ':' */
    if (!exige(p, T_INDENT, "faltou indentacao apos ':'")) return NULL;
    PSNode *b = ps_node_novo(p->arena, N_BLOCK, t->line, t->col);
    if (!b) return NULL;
    b->estilo = "colon";
    pula_separadores_com_dedent(p);
    while (!checa(p, T_DEDENT) && !checa(p, T_EOF)) {
        PSNode *st = statement(p);
        if (FALHOU(p)) return NULL;
        if (st && ps_vec_push(p->arena, &b->lista, st) != 0) {
            perro(p, "sem memoria", t); return NULL;
        }
        pula_separadores_com_dedent(p);
    }
    if (checa(p, T_EOF)) { perro(p, "bloco indentado nao foi fechado corretamente", t); return NULL; }
    p->pos++;   /* DEDENT */
    return b;
}

static PSNode *bloco_no(P *p);

/* Mesma casca do `expressao`: bloco dentro de bloco é o OUTRO caminho de
 * recursão do parser (`if { if { if { …`), e sem teto ele estoura a pilha do
 * mesmo jeito que o parêntese aninhado. */
static PSNode *bloco(P *p)
{
    if (p->prof >= PS_PARSE_PROF_MAX || ps_pilha_apertada()) {
        perro(p, "bloco aninhado demais", atual(p));
        return NULL;
    }
    p->prof++;
    PSNode *r = bloco_no(p);
    p->prof--;
    return r;
}

static PSNode *bloco_no(P *p)
{
    PSToken *t = atual(p);

    /* Estilo Allman — a chave na LINHA SEGUINTE:
     *
     *     if (x)
     *     {
     *         ...
     *     }
     *
     * `Entity`/`class`/`action` sempre aceitaram (o cabeçalho deles pula
     * separadores antes de procurar o `{`); `if`/`while`/`for` não, e a
     * diferença era acidental — o mesmo arquivo passava numa construção e
     * falhava na outra. Só pula os separadores quando o que vem depois deles
     * é MESMO um `{`: senão um bloco `:` perderia a quebra de linha que ele
     * exige. */
    if (checa(p, T_NEWLINE) || checa(p, T_INDENT)) {
        int32_t k = p->pos;
        while (k < p->n && (p->toks[k].type == T_NEWLINE
                            || p->toks[k].type == T_INDENT)) k++;
        if (k < p->n && p->toks[k].type == T_LBRACE) {
            p->pos = k;
            t = atual(p);
        }
    }

    if (aceita(p, T_LBRACE)) {
        PSNode *b = ps_node_novo(p->arena, N_BLOCK, t->line, t->col);
        if (!b) return NULL;
        b->estilo = "brace";
        /* Dentro de `{ }` a indentação é cosmética — o bloco acaba no `}`, não
         * num DEDENT. Os INDENT/DEDENT que sobram entre um statement e outro
         * são ignorados aqui; os que pertencem a um sub-bloco `:` são
         * consumidos pela chamada aninhada de bloco(). */
        pula_indent_solto(p);
        while (!checa(p, T_RBRACE) && !checa(p, T_EOF)) {
            PSNode *s = statement(p);
            if (FALHOU(p)) return NULL;
            if (s && ps_vec_push(p->arena, &b->lista, s) != 0) {
                perro(p, "sem memoria", t); return NULL;
            }
            pula_indent_solto(p);
        }
        if (checa(p, T_EOF)) { perro(p, "bloco com '{' nao foi fechado com '}'", t); return NULL; }
        /* A LINHA DO `}` — a extensão do bloco.
         *
         * O editor precisa dela: sem `linha_fim`, o escopo de uma action
         * acabava no último comando, e o cursor numa linha em branco antes do
         * fecha-chaves caía FORA dele. Parâmetro e variável local sumiam da
         * sugestão exatamente onde se está escrevendo. */
        b->linha_fim = atual(p)->line;
        p->pos++;   /* } */
        return b;
    }

    /* Bloco por `:` + indentação NÃO existe mais: o bloco da linguagem é
     * `{ }`, e só. Conviver com os dois custou caro — toda regressão de
     * parser desta linha do tempo saiu da interação entre indentação e chave
     * (INDENT dentro de `{}`, `match` com chave, chave na linha seguinte).
     * A mensagem diz o que fazer em vez de deixar "expressao invalida". */
    if (checa(p, T_COLON)) {
        perro(p, "bloco com ':' nao existe mais — use '{ }'", t);
        return NULL;
    }

    perro(p, "esperado inicio de bloco com '{'", t);
    return NULL;
}

/* ── padrões de match ───────────────────────────────────────────────────── */
/* MatchPattern usa: texto=kind, texto2=nome(capture), a=guard,
 * lista=itens/subpadrões, lista2=chaves(dict, como Name), lit/i/d=valor. */
static PSNode *padrao(P *p);

static PSNode *talvez_ou(P *p, PSNode *pat)
{
    /* `case 1 | 2 | 3` vira um único padrão kind="or" */
    if (!checa_op(p, "|")) return pat;
    PSNode *n = ps_node_novo(p->arena, N_MATCH_PATTERN, pat->line, pat->col);
    if (!n) return NULL;
    n->texto = dup_str(p, "or");
    if (ps_vec_push(p->arena, &n->lista, pat) != 0) return NULL;
    while (checa_op(p, "|")) {
        p->pos++;
        PSNode *outro = padrao(p);
        if (FALHOU(p)) return NULL;
        if (ps_vec_push(p->arena, &n->lista, outro) != 0) return NULL;
    }
    return n;
}

static PSNode *padrao(P *p)
{
    PSToken *t = atual(p);

    /* wildcard `_` */
    if (t->type == T_IDENT && t->texto && strcmp(t->texto, "_") == 0) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_MATCH_PATTERN, t->line, t->col);
        if (!n) return NULL;
        n->texto = dup_str(p, "wildcard");
        return talvez_ou(p, n);
    }
    /* lista de padrões */
    if (t->type == T_LBRACK) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_MATCH_PATTERN, t->line, t->col);
        if (!n) return NULL;
        n->texto = dup_str(p, "list");
        while (!checa(p, T_RBRACK)) {
            PSNode *it = padrao(p);
            if (FALHOU(p)) return NULL;
            if (ps_vec_push(p->arena, &n->lista, it) != 0) return NULL;
            if (!aceita(p, T_COMMA)) break;
        }
        if (!exige(p, T_RBRACK, "faltou ']' no padrao de lista")) return NULL;
        return talvez_ou(p, n);
    }
    /* dict de padrões: chave em lista2, padrão correspondente em lista */
    if (t->type == T_LBRACE) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_MATCH_PATTERN, t->line, t->col);
        if (!n) return NULL;
        n->texto = dup_str(p, "dict");
        while (!checa(p, T_RBRACE)) {
            PSToken *kt = atual(p);
            p->pos++;
            PSNode *chave = ps_node_novo(p->arena, N_NAME, kt->line, kt->col);
            if (!chave) return NULL;
            chave->texto = dup_tok(p, kt);
            if (!exige(p, T_COLON, "esperado ':' no padrao de dict")) return NULL;
            PSNode *vp = padrao(p);
            if (FALHOU(p)) return NULL;
            if (ps_vec_push(p->arena, &n->lista2, chave) != 0) return NULL;
            if (ps_vec_push(p->arena, &n->lista, vp) != 0) return NULL;
            if (!aceita(p, T_COMMA)) break;
        }
        if (!exige(p, T_RBRACE, "faltou '}' no padrao de dict")) return NULL;
        return talvez_ou(p, n);
    }
    /* literais */
    if (t->type == T_STR || t->type == T_INT || t->type == T_FLO
            || t->type == T_BOOL || t->type == T_NULL) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_MATCH_PATTERN, t->line, t->col);
        if (!n) return NULL;
        n->texto = dup_str(p, "value");
        n->b = ps_node_novo(p->arena, N_LITERAL, t->line, t->col);
        if (!n->b) return NULL;
        switch (t->type) {
            case T_STR:  n->b->lit = L_STR; n->b->texto = dup_tok(p, t);
                         n->b->texto_len = t->texto_len; break;
            case T_INT:  n->b->lit = L_INT; n->b->i = t->i; break;
            case T_FLO:  n->b->lit = L_FLO; n->b->d = t->d; break;
            case T_BOOL: n->b->lit = L_BOOL; n->b->i = t->i; break;
            default:     n->b->lit = L_NULL; break;
        }
        return talvez_ou(p, n);
    }
    /* captura por nome */
    if (t->type == T_IDENT) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_MATCH_PATTERN, t->line, t->col);
        if (!n) return NULL;
        n->texto = dup_str(p, "capture");
        n->texto2 = dup_tok(p, t);
        return talvez_ou(p, n);
    }
    /* número negativo: -10 */
    if (checa_op(p, "-")) {
        p->pos++;
        PSToken *num = atual(p);
        if (num->type != T_INT && num->type != T_FLO) {
            perro(p, "esperado numero apos '-' no padrao de case", num); return NULL;
        }
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_MATCH_PATTERN, t->line, t->col);
        if (!n) return NULL;
        n->texto = dup_str(p, "value");
        n->b = ps_node_novo(p->arena, N_LITERAL, num->line, num->col);
        if (!n->b) return NULL;
        if (num->type == T_INT) { n->b->lit = L_INT; n->b->i = -num->i; }
        else                    { n->b->lit = L_FLO; n->b->d = -num->d; }
        return talvez_ou(p, n);
    }

    perro(p, "padrao de case invalido", t);
    return NULL;
}

/* ── operador count ─────────────────────────────────────────────────────── */
/* Lê `<tipo>` ou `<tipo>(<valor>)`. `char` só é tipo válido aqui. */
static int count_tipo_valor(P *p, const char **tipo, PSNode **valor)
{
    PSToken *t = atual(p);
    if (t->type != T_KW || !t->texto
            || !(strcmp(t->texto,"str")==0 || strcmp(t->texto,"int")==0
              || strcmp(t->texto,"flo")==0 || strcmp(t->texto,"bool")==0
              || strcmp(t->texto,"list")==0 || strcmp(t->texto,"json")==0
              || strcmp(t->texto,"dict")==0 || strcmp(t->texto,"tup")==0
              || strcmp(t->texto,"char")==0)) {
        perro(p, "esperado tipo (str, int, flo, bool, list, json, char) apos 'count'", t);
        return -1;
    }
    p->pos++;
    *tipo = dup_tok(p, t);
    *valor = NULL;
    if (aceita(p, T_LPAREN)) {
        if (!checa(p, T_RPAREN)) {
            *valor = expressao(p);
            if (FALHOU(p)) return -1;
        }
        if (!exige(p, T_RPAREN, "faltou ')' no valor de 'count'")) return -1;
    }
    return 0;
}

/* Lado esquerdo do count infixo/sufixo: `int(7)` (Call), `int` (TypeName)
 * ou `int` (Name). Espelha _extract_count_left do parser Python. */
static int count_esquerda(P *p, PSNode *no, PSToken *tok,
                          const char **tipo, PSNode **valor)
{
    if (no && no->kind == N_CALL && no->a && no->a->kind == N_NAME
            && eh_tipo_nome(no->a->texto)) {
        for (int32_t i = 0; i < no->lista.n; i++) {
            if (no->lista.itens[i]->texto) {
                perro(p, "'count' nao aceita argumentos nomeados no valor", tok);
                return -1;
            }
        }
        if (no->lista.n > 1) {
            perro(p, "'count' aceita no maximo um valor entre parenteses", tok);
            return -1;
        }
        *tipo = no->a->texto;
        *valor = no->lista.n == 1 ? no->lista.itens[0]->a : NULL;
        return 0;
    }
    if (no && no->kind == N_TYPE_NAME) { *tipo = no->texto; *valor = NULL; return 0; }
    if (no && no->kind == N_NAME && eh_tipo_nome(no->texto)) {
        *tipo = no->texto; *valor = NULL; return 0;
    }
    perro(p, "lado esquerdo de 'count in' deve ser um tipo (ex: int(7) count in lista)", tok);
    return -1;
}

/* ── imports ────────────────────────────────────────────────────────────── */
/* Nome de módulo/membro aceita qualquer palavra, inclusive keyword
 * (`import json`, `from os import type`) — é a regra do parse_name_like. */
static const char *nome_livre(P *p, const char *msg)
{
    PSToken *t = atual(p);
    if (t->type != T_IDENT && t->type != T_IDENT_UPPER && t->type != T_KW) {
        perro(p, msg, t);
        return NULL;
    }
    p->pos++;
    return dup_tok(p, t);
}

/* caminho pontilhado: a.b.c — cada parte vira um Name na lista */
/* `import 'caminho/alvo.ps'` / `from 'nome' import x` / `PUSH 'x'`: o modulo
 * vem numa STRING, como no TypeScript. Fica um literal so em `lista`, e
 * `i2 = -1` marca a forma. Com `/` ou extensao da linguagem e caminho,
 * relativo ao arquivo que importa; sem isso e nome de modulo do motor ou de
 * lib instalada — quem decide e o runtime (acha_modulo_ps). Devolve 1 se
 * consumiu a string, 0 se nao havia string, -1 em erro. */
static int modulo_entre_aspas(P *p, PSNode *n)
{
    PSToken *t = atual(p);
    if (t->type != T_STR) return 0;
    if (t->texto_len == 0) {
        perro(p, "import entre aspas vazio: esperado um caminho ('../x.ps') ou o nome de um modulo ('json')", t);
        return -1;
    }
    p->pos++;
    PSNode *lit = ps_node_novo(p->arena, N_LITERAL, t->line, t->col);
    if (!lit) return -1;
    lit->lit = L_STR;
    lit->texto = ps_arena_strdup(p->arena, t->texto ? t->texto : "", t->texto_len);
    lit->texto_len = t->texto_len;
    if (ps_vec_push(p->arena, &n->lista, lit) != 0) return -1;
    n->i2 = -1;
    return 1;
}

static int caminho_modulo(P *p, PSNodeVec *v)
{
    PSToken *ponto = NULL;      /* o `.` que exigiu esta parte, se houve */
    for (;;) {
        PSToken *t = atual(p);
        /* `import jinker.` — o ponto sem nome depois. A mensagem genérica
         * ("esperado caminho de modulo") apontava pro `import`, no começo da
         * linha, e não dizia o que faltava: quem escreveu o ponto (muitas
         * vezes só pra chamar o completion do editor) ficava sem pista. */
        /* `nome_livre` aceita até palavra reservada (`@app.route`), então a
         * recusa aqui só vale pro que NÃO pode ser nome em hipótese nenhuma:
         * fim de linha, fim de arquivo, mudança de indentação. Exigir IDENT
         * quebrava todo decorador cujo membro é keyword. */
        if (ponto && (t->type == T_NEWLINE || t->type == T_EOF
                      || t->type == T_INDENT || t->type == T_DEDENT)) {
            perro(p, "faltou o nome do submodulo depois do '.'", ponto);
            return -1;
        }
        const char *n = nome_livre(p, "esperado nome de modulo depois de 'import'");
        if (FALHOU(p)) return -1;
        PSNode *no = ps_node_novo(p->arena, N_NAME, t->line, t->col);
        if (!no) return -1;
        no->texto = n;
        if (ps_vec_push(p->arena, v, no) != 0) return -1;
        ponto = atual(p);
        if (!aceita(p, T_DOT)) break;
    }
    return 0;
}

/* lista `a, b as c` — nomes em `nomes`, alias em `aliases` (Name ou nil
 * na mesma posição, pra o par ficar alinhado sem precisar de dict) */
static int lista_com_alias(P *p, PSNodeVec *nomes, PSNodeVec *aliases)
{
    for (;;) {
        PSToken *t = atual(p);
        const char *n = nome_livre(p, "esperado nome importado");
        if (FALHOU(p)) return -1;
        PSNode *no = ps_node_novo(p->arena, N_NAME, t->line, t->col);
        if (!no) return -1;
        no->texto = n;
        if (ps_vec_push(p->arena, nomes, no) != 0) return -1;

        PSNode *al = NULL;
        if (aceita_kw(p, "as")) {
            PSToken *at = atual(p);
            const char *a = nome_livre(p, "esperado nome apos 'as'");
            if (FALHOU(p)) return -1;
            al = ps_node_novo(p->arena, N_NAME, at->line, at->col);
            if (!al) return -1;
            al->texto = a;
        }
        if (ps_vec_push(p->arena, aliases, al) != 0) return -1;
        if (!aceita(p, T_COMMA)) break;
    }
    return 0;
}

/* ── desempacotamento ───────────────────────────────────────────────────── */
/* UnpackTarget: elementos em `lista` (Name ou UnpackTarget aninhado),
 * i2 = índice do alvo com `*` (-1 se não houver), is_async reaproveitado
 * como flag de vírgula final. */
static PSNode *alvos_unpack(P *p);

/* Um alvo: NOME, e depois quantos sufixos `.campo` e `[indice]` vierem.
 *
 * POR QUE O SUFIXO: `lista[c], lista[c + 1] = lista[c + 1], lista[c]` — a
 * troca do bubble sort — dava `SyntaxError: expressao invalida`. Aqui so se
 * lia NOME, entao `d["a"], d["b"] = 1, 2` e `o.x, o.y = 1, 2` caiam junto:
 * a linguagem tinha desempacotamento e tinha atribuicao indexada, e as duas
 * nao se encontravam. No CPython todo alvo de atribuicao serve de alvo de
 * desempacotamento, e e o que passa a valer aqui.
 *
 * Fatia (`l[1:2], x = ...`) NAO entra: `l[1:2] = ...` tambem nao existe na
 * linguagem, e aceitar so de um lado seria inventar meia feature. */
static PSNode *alvo_com_sufixos(P *p)
{
    PSToken *nt = atual(p);
    const char *nome = exige_nome(p, "variavel");
    if (FALHOU(p)) return NULL;
    PSNode *no = ps_node_novo(p->arena, N_NAME, nt->line, nt->col);
    if (!no) return NULL;
    no->texto = nome;

    for (;;) {
        if (checa(p, T_DOT)) {
            PSToken *dt = atual(p);
            p->pos++;
            PSToken *mt = atual(p);
            if (mt->type != T_IDENT && mt->type != T_IDENT_UPPER && mt->type != T_KW) {
                perro(p, "esperado nome do membro apos '.'", mt);
                return NULL;
            }
            p->pos++;
            PSNode *m = ps_node_novo(p->arena, N_MEMBER_ACCESS, dt->line, dt->col);
            if (!m) return NULL;
            m->a = no;
            m->texto = dup_tok(p, mt);
            no = m;
            continue;
        }
        if (checa(p, T_LBRACK)) {
            PSToken *bt = atual(p);
            p->pos++;
            PSNode *ix = expressao(p);
            if (FALHOU(p)) return NULL;
            if (!exige_fecha(p, T_RBRACK, "faltou ']' no indice", bt)) return NULL;
            PSNode *a = ps_node_novo(p->arena, N_INDEX_ACCESS, bt->line, bt->col);
            if (!a) return NULL;
            a->a = no; a->b = ix;
            no = a;
            continue;
        }
        break;
    }
    return no;
}

static int um_alvo(P *p, PSNode *alvo)
{
    PSToken *t = atual(p);
    if (checa_op(p, "*")) {
        if (alvo->i2 >= 0) {
            perro(p, "apenas um alvo com '*' e permitido por nivel de desempacotamento", t);
            return -1;
        }
        p->pos++;
        PSNode *no = alvo_com_sufixos(p);
        if (FALHOU(p) || !no) return -1;
        alvo->i2 = alvo->lista.n;
        return ps_vec_push(p->arena, &alvo->lista, no);
    }
    if (checa(p, T_LPAREN)) {
        p->pos++;
        PSNode *dentro = alvos_unpack(p);
        if (FALHOU(p)) return -1;
        if (!exige(p, T_RPAREN, "faltou ')' em alvo de desempacotamento")) return -1;
        /* grupo de 1 elemento simples colapsa, igual `(expr)` vira expr */
        if (dentro->lista.n == 1 && dentro->i2 < 0 && !dentro->is_async)
            return ps_vec_push(p->arena, &alvo->lista, dentro->lista.itens[0]);
        return ps_vec_push(p->arena, &alvo->lista, dentro);
    }
    PSNode *no = alvo_com_sufixos(p);
    if (FALHOU(p) || !no) return -1;
    return ps_vec_push(p->arena, &alvo->lista, no);
}

static PSNode *alvos_unpack(P *p)
{
    PSToken *t = atual(p);
    PSNode *alvo = ps_node_novo(p->arena, N_UNPACK_TARGET, t->line, t->col);
    if (!alvo) return NULL;
    alvo->i2 = -1;
    if (um_alvo(p, alvo) != 0) return NULL;
    while (aceita(p, T_COMMA)) {
        PSToken *cur = atual(p);
        if ((cur->type == T_OP && cur->texto && strcmp(cur->texto, "=") == 0)
                || cur->type == T_RPAREN) {
            alvo->is_async = 1;          /* vírgula final */
            break;
        }
        if (um_alvo(p, alvo) != 0) return NULL;
    }
    return alvo;
}

/* Lookahead puro: `a, b = ...` é desempacotamento? Não consome tokens.
 * Precisa recusar chamada, tupla-literal, comparação e atribuição simples —
 * todas começam parecido.
 *
 * `.campo` e `[indice]` SÃO alvo (`l[i], l[j] = l[j], l[i]`); só a chamada
 * `f(` nunca é. Enquanto isto recusava o colchete, `um_alvo` nem chegava a
 * ser chamado: a linha caía no parser de expressão e virava
 * "expressao invalida" na vírgula. */
static int parece_unpack(P *p)
{
    int32_t i = p->pos;
    int viu_virgula = 0, viu_estrela = 0;
    int prof = 0;

    while (i < p->n) {
        PSToken *t = &p->toks[i];
        if (t->type == T_OP && t->texto && strcmp(t->texto, "*") == 0) {
            viu_estrela = 1; i++; continue;
        }
        if (t->type == T_LPAREN) { prof++; i++; continue; }
        if (t->type == T_RPAREN) { if (prof == 0) return 0; prof--; i++; continue; }
        if (t->type == T_IDENT || t->type == T_IDENT_UPPER) {
            i++;
            for (;;) {                       /* cadeia de sufixos do alvo */
                if (i + 1 < p->n && p->toks[i].type == T_DOT
                        && (p->toks[i+1].type == T_IDENT
                         || p->toks[i+1].type == T_IDENT_UPPER
                         || p->toks[i+1].type == T_KW)) { i += 2; continue; }
                if (i < p->n && p->toks[i].type == T_LBRACK) {
                    int d = 0;               /* pula o índice inteiro, balanceado */
                    while (i < p->n) {
                        if (p->toks[i].type == T_LBRACK) d++;
                        else if (p->toks[i].type == T_RBRACK) d--;
                        else if (p->toks[i].type == T_EOF) return 0;
                        i++;
                        if (d == 0) break;
                    }
                    if (d != 0) return 0;
                    continue;
                }
                break;
            }
            /* `f(...)` é chamada, não alvo — nem depois de sufixo (`o.m(`) */
            if (i < p->n && p->toks[i].type == T_LPAREN) return 0;
            continue;
        }
        if (t->type == T_COMMA) { viu_virgula = 1; i++; continue; }
        if (t->type == T_OP && t->texto && strcmp(t->texto, "=") == 0)
            return prof == 0 && (viu_virgula || viu_estrela);
        return 0;
    }
    return 0;
}

/* ── statements ─────────────────────────────────────────────────────────── */
static PSNode *if_stmt(P *p)
{
    PSToken *t0 = atual(p);
    PSNode *n = ps_node_novo(p->arena, N_IF_STMT, t0->line, t0->col);
    if (!n) return NULL;
    p->pos++;    /* if */

    for (;;) {
        PSToken *tb = atual(p);
        PSNode *cond;
        {
            /* A condição vai até o `{`, comece com o que começar. Havia um
             * ramo especial pra `if (`: lia o grupo e EXIGIA `)` seguido do
             * bloco — então `if (a) or (b) {` quebrava com "esperado inicio de
             * bloco com '{'". Parêntese na condição é só precedência; o
             * `expressao` já lê `(a) or (b)` inteiro, e `if (x) {` continua
             * valendo por ser uma expressão como outra qualquer.
             *
             * `if x == "" {` — o `{` ali ABRE BLOCO, não interpola a string.
             * Sem isto, uma condição terminada em literal de texto engolia o
             * `{` do bloco como interpolação (`"txt" {x}`) e o `if` ficava sem
             * corpo. Mesmo tratamento que o `for each` já tinha. */
            int salvo = p->chave_abre_bloco;
            p->chave_abre_bloco = 1;
            cond = expressao(p);
            p->chave_abre_bloco = salvo;
            if (FALHOU(p)) return NULL;
        }
        PSNode *b = bloco(p);
        if (FALHOU(p)) return NULL;

        PSNode *ramo = ps_node_novo(p->arena, N_IF_BRANCH, tb->line, tb->col);
        if (!ramo) return NULL;
        ramo->a = cond; ramo->b = b;
        if (ps_vec_push(p->arena, &n->lista, ramo) != 0) { perro(p, "sem memoria", tb); return NULL; }

        /* separadores podem aparecer entre `}` e `elif`/`else` */
        int32_t salvo = p->pos;
        pula_separadores(p);
        if (checa_kw(p, "elif")) { p->pos++; continue; }
        if (checa_kw(p, "else")) {
            PSToken *te = atual(p);
            p->pos++;
            PSNode *eb = bloco(p);
            if (FALHOU(p)) return NULL;
            PSNode *r2 = ps_node_novo(p->arena, N_IF_BRANCH, te->line, te->col);
            if (!r2) return NULL;
            r2->a = NULL; r2->b = eb;
            if (ps_vec_push(p->arena, &n->lista, r2) != 0) { perro(p, "sem memoria", te); return NULL; }
            break;
        }
        p->pos = salvo;    /* não era elif/else: devolve os separadores */
        break;
    }
    return n;
}

static PSNode *while_stmt(P *p)
{
    PSToken *t = atual(p);
    p->pos++;
    PSNode *n = ps_node_novo(p->arena, N_WHILE_STMT, t->line, t->col);
    if (!n) return NULL;
    {
        /* idem `if`: a condição vai até o `{` (o `(` inicial é precedência,
         * não envoltório — `while (a) and (b) {` quebrava), e `while s != "" {`
         * abre bloco, não interpola */
        int salvo = p->chave_abre_bloco;
        p->chave_abre_bloco = 1;
        n->a = expressao(p);
        p->chave_abre_bloco = salvo;
        if (FALHOU(p)) return NULL;
    }
    n->b = bloco(p);
    if (FALHOU(p)) return NULL;
    return n;
}

static PSNode *for_stmt(P *p)
{
    PSToken *t = atual(p);
    p->pos++;                                  /* for */
    if (!aceita_kw(p, "each")) { perro(p, "esperado 'each' em 'for each'", atual(p)); return NULL; }

    /* `for each a, b in pares` — DESEMPACOTAMENTO, como o Python.
     *
     * Antes o parser lia UM nome e exigia `in`, entao a virgula dava
     * "esperado 'in' no loop for each" e quem itera lista de pares tinha que
     * abrir o item na mao dentro do corpo. A maquina ja existia inteira: o
     * `alvos_unpack` e o `compila_unpack_alvo` sao os mesmos do
     * `a, b = [1, 2]`, que ja funcionava. So o laco nao os chamava.
     *
     * O alvo do laco e o MESMO alvo da atribuicao: `for each l[0] in xs` e
     * `for each a, o.x in pares` valem, como no CPython. Sem isto o indexado
     * so seria alvo da segunda posicao em diante — a metade que a gente
     * lembrou de arrumar. */
    PSToken *nt = atual(p);
    PSNode *primeiro = alvo_com_sufixos(p);
    if (FALHOU(p) || !primeiro) return NULL;
    const char *item = primeiro->kind == N_NAME ? primeiro->texto : NULL;

    PSNode *desempacota = NULL;
    if (checa(p, T_COMMA)) {
        desempacota = ps_node_novo(p->arena, N_UNPACK_TARGET, nt->line, nt->col);
        if (!desempacota) return NULL;
        desempacota->i2 = -1;
        if (ps_vec_push(p->arena, &desempacota->lista, primeiro) != 0) {
            perro(p, "sem memoria", nt); return NULL;
        }
        while (aceita(p, T_COMMA)) {
            if (um_alvo(p, desempacota) != 0) return NULL;
        }
    } else if (!item) {
        desempacota = primeiro;      /* alvo unico que nao e nome: `for each l[0] in` */
    }

    if (!aceita_kw(p, "in")) { perro(p, "esperado 'in' no loop for each", atual(p)); return NULL; }
    PSNode *n = ps_node_novo(p->arena, N_FOR_EACH_STMT, t->line, t->col);
    if (!n) return NULL;
    n->texto = item;
    n->e = desempacota;      /* NULL quando e um nome so */
    {
        int salvo = p->chave_abre_bloco;
        p->chave_abre_bloco = 1;
        n->a = expressao(p);
        p->chave_abre_bloco = salvo;
    }
    if (FALHOU(p)) return NULL;
    n->b = bloco(p);
    if (FALHOU(p)) return NULL;
    return n;
}

static PSNode *action_decl(P *p, int is_async, const char *tipo_retorno)
{
    PSToken *t = atual(p);
    p->pos++;                                  /* funct / action / reaction */
    const char *nome = exige_nome(p, "funct");
    if (FALHOU(p)) return NULL;

    PSNode *n = ps_node_novo(p->arena, N_ACTION_DECL, t->line, t->col);
    if (!n) return NULL;
    n->texto = nome;
    n->texto2 = tipo_retorno;
    n->is_async = is_async;

    if (!exige(p, T_LPAREN, "faltou '(' na declaracao da funct")) return NULL;
    if (!checa(p, T_RPAREN)) {
        for (;;) {
            PSToken *pt = atual(p);
            const char *pn;
            if (pt->type == T_KW && pt->texto && strcmp(pt->texto, "self") == 0) {
                pn = dup_tok(p, pt);           /* `self` é o único KW aceito */
                p->pos++;
            } else {
                pn = exige_nome(p, "parametro");
                if (FALHOU(p)) return NULL;
            }
            PSNode *par = ps_node_novo(p->arena, N_NAME, pt->line, pt->col);
            if (!par) return NULL;
            par->texto = pn;
            /* valor padrão: `action f(a, b=1)`. Fica pendurado no próprio nó
             * do parâmetro (campo `a`), que é o que o serializador compara. */
            if (checa_op(p, "=")) {
                p->pos++;
                par->a = expressao(p);
                if (FALHOU(p)) return NULL;
            }
            if (ps_vec_push(p->arena, &n->lista, par) != 0) { perro(p, "sem memoria", pt); return NULL; }
            if (!aceita(p, T_COMMA)) break;
            if (checa(p, T_RPAREN)) break;   /* vírgula final */
        }
    }
    if (!exige(p, T_RPAREN, "faltou ')' na declaracao da funct")) return NULL;
    while (checa(p, T_NEWLINE)) p->pos++;      /* `{` pode vir na linha seguinte */
    n->b = bloco(p);
    if (FALHOU(p)) return NULL;
    return n;
}

static PSNode *return_stmt(P *p)
{
    PSToken *t = atual(p);
    p->pos++;
    PSNode *n = ps_node_novo(p->arena, N_RETURN_STMT, t->line, t->col);
    if (!n) return NULL;
    if (checa(p, T_NEWLINE) || checa(p, T_SEMI) || checa(p, T_RBRACE)
            || checa(p, T_DEDENT) || checa(p, T_EOF)) {
        n->a = NULL;
    } else {
        PSNode *e = expressao(p);
        if (FALHOU(p)) return NULL;
        /* `return jsonify({...}), 409` — empacota como tupla (valor, status) */
        if (aceita(p, T_COMMA)) {
            PSNode *status = expressao(p);
            if (FALHOU(p)) return NULL;
            PSNode *tup = ps_node_novo(p->arena, N_TUPLE_LITERAL, t->line, t->col);
            if (!tup) return NULL;
            if (ps_vec_push(p->arena, &tup->lista, e) != 0) return NULL;
            if (ps_vec_push(p->arena, &tup->lista, status) != 0) return NULL;
            n->a = tup;
        } else {
            n->a = e;
        }
    }
    return n;
}

/* Tipos aceitos em DECLARAÇÃO (`int x = 1`) e como tipo de retorno.
 *
 * Só str/int/flo/bool — `list` e `json` NÃO declaram variável: o parser
 * Python os deixa como TypeName solto, então `list nums = [...]` vira
 * `(TypeName list)` + `(Assignment nums ...)`, não um VarDecl. Incluí-los
 * aqui gerava uma AST diferente da de referência. */
static int eh_tipo_kw(PSToken *t)
{
    return t->type == T_KW && t->texto
        && (strcmp(t->texto, "str") == 0 || strcmp(t->texto, "int") == 0
         || strcmp(t->texto, "flo") == 0 || strcmp(t->texto, "bool") == 0);
}

/* Tipos que abrem uma DECLARAÇÃO (`char c = "a"`). É maior que o
 * `eh_tipo_kw`, que também guarda o tipo de RETORNO de action — e ali só
 * `int action`/`bool action` existem. */
/* Apelidos de tipo em declaracao: `string`/`String` = str, `integer`/`Integer`
 * = int, `tuple`/`Tuple` = tup, `dictionary`/`Dictionary` = dict. NAO sao
 * palavras reservadas — `string` continua podendo ser variavel ou nome de
 * parametro (`regex.sub(string=...)`): o apelido so vale onde um TIPO vale,
 * antes do nome numa declaracao. */
static int eh_apelido_tipo(PSToken *t)
{
    static const char *const A[] = { "string", "String", "integer", "Integer",
                                     "tuple", "Tuple", "dictionary", "Dictionary", NULL };
    if ((t->type != T_IDENT && t->type != T_IDENT_UPPER) || !t->texto) return 0;
    for (int i = 0; A[i]; i++) if (strcmp(t->texto, A[i]) == 0) return 1;
    return 0;
}

static int eh_tipo_kw_decl(PSToken *t)
{
    /* `list`/`dict`/`json`/`tup` entraram aqui com a tipagem estatica: `list
     * l = [1]` era lido como um TypeName SOLTO (statement vazio) seguido da
     * atribuicao comum `l = [1]` — a doc prometia "guarda o proprio tipo" e
     * nenhuma declaracao existia. */
    return eh_tipo_kw(t) || eh_apelido_tipo(t)
        || (t->type == T_KW && t->texto
            && (strcmp(t->texto, "char") == 0
                || strcmp(t->texto, "long") == 0
                || strcmp(t->texto, "list") == 0 || strcmp(t->texto, "dict") == 0
                || strcmp(t->texto, "json") == 0 || strcmp(t->texto, "tup") == 0
                || strcmp(t->texto, "Object") == 0 || strcmp(t->texto, "object") == 0));
}

static PSNode *statement(P *p)
{
    pula_separadores(p);
    PSToken *t = atual(p);

    if (t->type == T_EOF) return NULL;

    /* `def f(x) { ... }` — quem vem do Python escreve isto, e a linguagem
     * respondia "faltou ':' no dicionario" apontando pra DENTRO do corpo.
     *
     * O motivo: `def` nao e palavra da linguagem. Entao `def` vira um nome
     * solto, `f(x)` vira uma chamada, e o `{` que vinha depois era lido como
     * LITERAL DE DICIONARIO. O erro falava de dicionario porque, pro parser,
     * era um dicionario mesmo — e nada na mensagem sugeria `action`.
     *
     * A forma `def <nome> (` no comeco de um statement nao tem outra leitura
     * possivel nesta linguagem. Vale a pena dize-lo. */
    if ((t->type == T_IDENT || t->type == T_IDENT_UPPER)
            && t->texto && t->texto_len == 3 && strncmp(t->texto, "def", 3) == 0) {
        PSToken *n1 = espia(p, 1);
        PSToken *n2 = espia(p, 2);
        if ((n1->type == T_IDENT || n1->type == T_IDENT_UPPER)
                && n2->type == T_LPAREN) {
            perro(p, "'def' nao existe nesta linguagem; a funcao se declara com"
                     " 'funct': funct nome(args) { ... }", t);
            return NULL;
        }
    }

    /* `action nome(` / `reaction nome(` / `action(` — a grafia que saiu.
     *
     * Mesma armadilha do `def`: sem palavra reservada, `action f() {` vira
     * nome solto + chamada + literal de dicionario, e o erro fala de
     * dicionario. Com um `int` ou um `async` na frente, pior ainda: casava com
     * a regra de declaracao tipada e virava variavel. A recusa diz a palavra
     * nova e mostra a linha inteira consertada. */
    if (eh_grafia_morta(t)) {
        PSToken *n1 = espia(p, 1);
        PSToken *n2 = espia(p, 2);
        if (n1->type == T_LPAREN
                || ((n1->type == T_IDENT || n1->type == T_IDENT_UPPER) && n2->type == T_LPAREN)) {
            char m[200];
            snprintf(m, sizeof(m),
                     "'%s' saiu da linguagem; a funcao se declara com 'funct': funct %s(args) { ... }",
                     t->texto,
                     (n1->type == T_LPAREN) ? "" : (n1->texto ? n1->texto : "nome"));
            perro(p, m, t);
            return NULL;
        }
    }

    /* reservada seguida de '=' é tentativa de usá-la como variável */
    if (t->type == T_KW) {
        PSToken *nx = espia(p, 1);
        if (nx->type == T_OP && nx->texto && eh_op_atribuicao(nx->texto)) {
            char m[200];
            snprintf(m, sizeof(m),
                     "'%s' e palavra reservada da linguagem e nao pode ser usada como nome de variavel",
                     t->texto ? t->texto : "?");
            perro(p, m, t);
            return NULL;
        }
    }

    if (checa_kw(p, "continue")) {
        p->pos++;
        return ps_node_novo(p->arena, N_CONTINUE_STMT, t->line, t->col);
    }
    if (checa_kw(p, "break")) {
        p->pos++;
        return ps_node_novo(p->arena, N_BREAK_STMT, t->line, t->col);
    }
    /* `pass` — no-op, igual ao Python: vale em qualquer lugar onde caberia um
     * statement, e serve pra dar corpo a um bloco que não faz nada. */
    if (checa_kw(p, "pass")) {
        p->pos++;
        return ps_node_novo(p->arena, N_PASS_STMT, t->line, t->col);
    }
    if (checa_kw(p, "raise")) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_RAISE_STMT, t->line, t->col);
        if (!n) return NULL;
        /* raise Tipo  ou  raise Tipo("msg") — nome de tipo livre (IDENT_UPPER).
         * texto = nome do tipo; a = expressão da mensagem (ou NULL). */
        if (atual(p)->type == T_IDENT_UPPER) {
            n->texto = dup_tok(p, atual(p));
            p->pos++;
            if (checa(p, T_LPAREN)) {
                p->pos++;
                if (!checa(p, T_RPAREN)) {
                    n->a = expressao(p);
                    if (FALHOU(p)) return NULL;
                }
                if (!exige(p, T_RPAREN, "esperado ')' no raise")) return NULL;
            }
        } else {
            n->a = expressao(p);
            if (FALHOU(p)) return NULL;
        }
        return n;
    }
    if (checa_kw(p, "yield")) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_YIELD_STMT, t->line, t->col);
        if (!n) return NULL;
        if (!checa(p, T_RBRACE) && !checa(p, T_DEDENT)
                && !checa(p, T_NEWLINE) && !checa(p, T_EOF)) {
            n->a = expressao(p);
            if (FALHOU(p)) return NULL;
        }
        return n;
    }
    if (checa_kw(p, "global")) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_GLOBAL_STMT, t->line, t->col);
        if (!n) return NULL;
        for (;;) {
            PSToken *nt = atual(p);
            const char *nome = exige_nome(p, "variavel");
            if (FALHOU(p)) return NULL;
            PSNode *item = ps_node_novo(p->arena, N_NAME, nt->line, nt->col);
            if (!item) return NULL;
            item->texto = nome;
            if (ps_vec_push(p->arena, &n->lista, item) != 0) {
                perro(p, "sem memoria", nt); return NULL;
            }
            if (!aceita(p, T_COMMA)) break;
        }
        return n;
    }
    /* `run_selfwith_` SAIU: o ponto de entrada agora é `if __name__ == "main"`,
     * a forma do Python. A recusa diz o conserto em vez de virar
     * "variável não definida: run_selfwith_" lá na frente. */
    if (t->type == T_IDENT && t->texto && strcmp(t->texto, "run_selfwith_") == 0) {
        perro(p, "run_selfwith_ nao existe mais — use: if __name__ == \"main\"", t);
        return NULL;
    }

    /* `public class Nome()` / `private class Nome()` — modificador de visibilidade
     * na PRÓPRIA classe. `private` = não exportada no import (só usável no arquivo).
     * `is_private` não entra na serialização do AST, então o diff continua batendo. */
    int classe_priv = 0;
    if (t->type == T_KW && t->texto
            && (strcmp(t->texto, "private") == 0 || strcmp(t->texto, "public") == 0)) {
        PSToken *nx = espia(p, 1);
        if (nx && nx->type == T_KW && nx->texto
                && (strcmp(nx->texto, "Entity") == 0 || strcmp(nx->texto, "class") == 0
                    || strcmp(nx->texto, "Class") == 0)) {
            classe_priv = (strcmp(t->texto, "private") == 0);
            p->pos++;          /* consome private/public */
            t = atual(p);      /* agora aponta pro Entity/class/Class */
        }
    }
    /* Entity Nome(Pai) { action ... | campo: tipo | @decorador } */
    if (checa_kw(p, "Entity") || checa_kw(p, "class") || checa_kw(p, "Class")) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_ENTITY_DECL, t->line, t->col);
        if (!n) return NULL;
        n->is_private = classe_priv;
        n->texto = exige_nome(p, "Entity");
        if (FALHOU(p)) return NULL;
        if (!exige(p, T_LPAREN, "esperado '(' apos nome da Entity")) return NULL;
        while (checa(p, T_IDENT) || checa(p, T_IDENT_UPPER)) {
            PSToken *pt = atual(p);
            p->pos++;
            PSNode *pai = ps_node_novo(p->arena, N_NAME, pt->line, pt->col);
            if (!pai) return NULL;
            pai->texto = dup_tok(p, pt);
            if (ps_vec_push(p->arena, &n->lista2, pai) != 0) return NULL;
            if (!aceita(p, T_COMMA)) break;
        }
        if (!exige(p, T_RPAREN, "esperado ')' apos heranca da Entity")) return NULL;
        pula_separadores(p);

        /* corpo da Entity/class: só `{ }`, como todo bloco da linguagem.
         * O `pula_separadores` acima já deixou a chave na linha de baixo valer. */
        PSToken *abre_ent = atual(p);
        if (checa(p, T_COLON)) {
            perro(p, "bloco com ':' nao existe mais — use '{ }'", abre_ent);
            return NULL;
        }
        if (!exige(p, T_LBRACE, "esperado '{' para abrir o corpo da Entity")) return NULL;
        pula_indent_solto(p);
        while (!checa(p, T_RBRACE) && !checa(p, T_EOF)) {
            PSToken *mt = atual(p);
            /* modificador de visibilidade opcional antes de campo/método.
             * `is_private` não entra na serialização do AST (não é código), então
             * o diff de parser/bytecode continua batendo. */
            /* Modificadores do membro, em qualquer ordem: `public`, `private`
             * e `static`. O `static` em CAMPO é o que faz um atributo
             * pertencer à classe (avaliado uma vez, na declaração) em vez de
             * à instância — sem ele, `App.mapp` não existe e um método
             * estático não tem como enxergar o campo. */
            int membro_priv = 0, viu_visib = 0, campo_static = 0;
            for (;;) {
                if (mt->type == T_KW && mt->texto
                        && (strcmp(mt->texto, "private") == 0 || strcmp(mt->texto, "public") == 0)) {
                    membro_priv = (strcmp(mt->texto, "private") == 0);
                    viu_visib = 1;
                    p->pos++;
                    mt = atual(p);
                    continue;
                }
                if (mt->texto && strcmp(mt->texto, "static") == 0
                        && (mt->type == T_IDENT || mt->type == T_KW)
                        && espia(p, 1)->type != T_LPAREN) {
                    campo_static = 1;
                    p->pos++;
                    mt = atual(p);
                    continue;
                }
                break;
            }
            /* A grafia que saiu, escrita como método: aqui ela casaria com a
             * regra de campo (`<tipo> <nome>`) e nasceria um campo chamado
             * `action`, calado. A recusa vem antes, com a palavra certa. */
            if (eh_grafia_morta(mt)) {
                char m[200];
                snprintf(m, sizeof(m),
                         "'%s' saiu da linguagem; o metodo se declara com 'funct': funct %s(self) { ... }",
                         mt->texto, espia(p, 1)->texto ? espia(p, 1)->texto : "nome");
                perro(p, m, mt);
                return NULL;
            }
            /* `pass` sozinho: corpo vazio de classe, como no Python. Não vira
             * membro nenhum — só ocupa o lugar pra a Entity poder existir sem
             * campo nem método. */
            if (mt->type == T_KW && mt->texto && strcmp(mt->texto, "pass") == 0) {
                p->pos++;
                pula_indent_solto(p);
                continue;
            }
            if (checa(p, T_AT)) {
                int salvo_flag = p->dec_sem_captura;
                p->dec_sem_captura = 1;
                PSNode *d = statement(p);
                p->dec_sem_captura = salvo_flag;
                if (FALHOU(p)) return NULL;
                if (ps_vec_push(p->arena, &n->lista, d) != 0) return NULL;
            /* MÉTODO: a cabeça inteira, com os modificadores em qualquer ordem
             * (`int static funct r()`, `static funct s()`, `async funct t()`).
             *
             * Aqui havia uma cópia à mão da cabeça, e ela não conhecia os
             * modificadores COLADOS: em `public int static funct r(n)` o
             * `int static` casava com a regra de campo logo abaixo e virava um
             * CAMPO chamado `static`; a funct saía sem tipo, sem `public` e
             * sem a marca de estática, e `Tipo.r(...)` respondia "nao tem
             * metodo estatico 'r' — instancie primeiro", com o `static`
             * escrito na tela. Um só lugar decide o que é cabeça de funct. */
            } else if (cabeca_de_funct(p, 0) >= 0) {
                PSNode *a = statement(p);
                if (FALHOU(p)) return NULL;
                /* Só sobrescreve se a visibilidade veio ANTES do resto: em
                 * `static private funct m()` quem a leu foi a cabeça, e
                 * carimbar 0 aqui apagaria o `private` da pessoa. */
                if (a && viu_visib) a->is_private = membro_priv;
                /* o `static` lido pelo laço de modificadores acima é do método */
                if (a && campo_static) a->is_static = 1;
                if (ps_vec_push(p->arena, &n->lista, a) != 0) return NULL;
            } else if ((mt->type == T_IDENT || mt->type == T_IDENT_UPPER || mt->type == T_KW)
                       && (espia(p, 1)->type == T_IDENT || espia(p, 1)->type == T_IDENT_UPPER)) {
                /* `<tipo> <nome> [= valor]` — a MESMA declaração do §7.6 que
                 * vale dentro da action, escrita no corpo da classe. Equivale
                 * a `<nome>: <tipo> [= valor]`, e o `private`/`public` da
                 * frente já foi lido acima. */
                PSToken *tt = mt;
                p->pos++;                                  /* tipo */
                PSToken *nmt = atual(p);
                const char *nome = exige_nome(p, "campo");
                if (FALHOU(p)) return NULL;
                PSNode *f = ps_node_novo(p->arena, N_ENTITY_FIELD, nmt->line, nmt->col);
                if (!f) return NULL;
                f->texto = nome;
                f->texto2 = dup_tok(p, tt);
                if (checa_op(p, "=")) {
                    p->pos++;
                    f->a = expressao(p);
                    if (FALHOU(p)) return NULL;
                }
                f->is_private = membro_priv;
                f->is_static  = campo_static;
                if (ps_vec_push(p->arena, &n->lista2_alias, f) != 0) return NULL;
            } else if (mt->type == T_IDENT || mt->type == T_IDENT_UPPER) {
                /* Campo. Três grafias, todas o MESMO nó:
                 *   nome: tipo [= valor]     (a original)
                 *   tipo nome [= valor]      (o ramo acima)
                 *   nome = valor             (sem tipo — como uma variável)
                 * A terceira não existia: `conexao = ""` no corpo da classe
                 * era "so sao permitidas declaracoes 'action'...", e quem
                 * declara um campo como declara qualquer variável era
                 * repelido. Sem tipo, o campo é dinâmico, igual `x = 1`.
                 * O erro continua explícito pra qualquer outra coisa:
                 * devolver "nada" sem consumir token fazia o laço girar pra
                 * sempre no mesmo ponto. */
                p->pos++;                                  /* nome */
                PSNode *f = ps_node_novo(p->arena, N_ENTITY_FIELD, mt->line, mt->col);
                if (!f) return NULL;
                f->texto = dup_tok(p, mt);
                if (checa(p, T_COLON)) {
                    p->pos++;
                    PSToken *tt = atual(p);
                    if (tt->type != T_IDENT && tt->type != T_IDENT_UPPER && tt->type != T_KW) {
                        perro(p, "esperado tipo apos ':' no campo da Entity", tt); return NULL;
                    }
                    p->pos++;
                    f->texto2 = dup_tok(p, tt);
                    if (checa_op(p, "=")) {
                        p->pos++;
                        f->a = expressao(p);
                        if (FALHOU(p)) return NULL;
                    }
                } else if (checa_op(p, "=")) {
                    p->pos++;
                    f->a = expressao(p);
                    if (FALHOU(p)) return NULL;
                } else {
                    perro(p, "dentro de Entity entra 'funct', decorador ou campo: 'nome = valor', 'nome: tipo' ou 'tipo nome = valor'", mt);
                    return NULL;
                }
                f->is_private = membro_priv;
                f->is_static  = campo_static;
                if (ps_vec_push(p->arena, &n->lista2_alias, f) != 0) return NULL;
            } else {
                perro(p, "dentro de Entity entra 'funct', decorador ou campo: 'nome = valor', 'nome: tipo' ou 'tipo nome = valor'", mt);
                return NULL;
            }
            pula_indent_solto(p);
        }
        /* A LINHA DO `}` da Entity — mesma razão do `linha_fim` do bloco: sem
         * ela o corpo da classe "acaba" no último membro, e o cursor numa
         * linha nova antes do fecha-chaves cai FORA da Entity. O editor
         * deixava de oferecer o construtor, os campos e o `self` bem onde se
         * está escrevendo o próximo membro. */
        n->linha_fim = atual(p)->line;
        if (!exige_fecha(p, T_RBRACE, "corpo da Entity nao foi fechado", abre_ent)) return NULL;
        return n;
    }

    /* `obj.a.b = valor` — atribuição de membro (lookahead antes de expressão) */
    if (t->type == T_IDENT || t->type == T_IDENT_UPPER
            || (t->type == T_KW && t->texto && strcmp(t->texto, "self") == 0)) {
        int32_t k = p->pos + 1;
        int n_dots = 0;
        while (k + 1 < p->n && p->toks[k].type == T_DOT
               && (p->toks[k+1].type == T_IDENT || p->toks[k+1].type == T_IDENT_UPPER
                   || p->toks[k+1].type == T_KW)) {
            n_dots++; k += 2;
        }
        if (n_dots > 0 && k < p->n && p->toks[k].type == T_OP && p->toks[k].texto
                && (strcmp(p->toks[k].texto, "=")  == 0 || strcmp(p->toks[k].texto, "+=") == 0
                 || strcmp(p->toks[k].texto, "-=") == 0 || strcmp(p->toks[k].texto, "*=") == 0
                 || strcmp(p->toks[k].texto, "/=") == 0 || strcmp(p->toks[k].texto, "%=") == 0)) {
            const char *op_membro = dup_tok(p, &p->toks[k]);
            PSNode *base = ps_node_novo(p->arena, N_NAME, t->line, t->col);
            if (!base) return NULL;
            base->texto = dup_tok(p, t);
            int32_t j = p->pos + 1;
            for (int d = 0; d < n_dots - 1; d++) {
                PSNode *ma = ps_node_novo(p->arena, N_MEMBER_ACCESS, t->line, t->col);
                if (!ma) return NULL;
                ma->a = base;
                ma->texto = dup_tok(p, &p->toks[j + 1]);
                base = ma;
                j += 2;
            }
            const char *ultimo = dup_tok(p, &p->toks[j + 1]);
            p->pos = k + 1;
            PSNode *n = ps_node_novo(p->arena, N_MEMBER_ASSIGNMENT, t->line, t->col);
            if (!n) return NULL;
            n->a = base; n->texto = ultimo;
            n->texto2 = op_membro;          /* "=" ou aumentado (+=, -=, ...) */
            n->b = expressao(p);
            if (FALHOU(p)) return NULL;
            return n;
        }
    }

    /* import a.b [as c] | from [.]* a.b import x [as y], z | PUSH a [as b] [GET x, y] */
    if (checa_kw(p, "import") || checa_kw(p, "from") || checa_kw(p, "PUSH")) {
        PSNode *n = ps_node_novo(p->arena, N_IMPORT_STMT, t->line, t->col);
        if (!n) return NULL;

        if (aceita_kw(p, "import")) {
            n->texto = dup_str(p, "import");
            int aspas = modulo_entre_aspas(p, n);
            if (aspas < 0) return NULL;
            if (!aspas && caminho_modulo(p, &n->lista) != 0) return NULL;
            if (aceita_kw(p, "as")) {
                n->texto2 = nome_livre(p, "esperado nome apos 'as'");
                if (FALHOU(p)) return NULL;
            }
            return n;
        }
        if (aceita_kw(p, "from")) {
            n->texto = dup_str(p, "from");
            int aspas = modulo_entre_aspas(p, n);
            if (aspas < 0) return NULL;
            if (!aspas) {
                /* pontos iniciais = import relativo (`from .mod import x`) */
                while (checa(p, T_DOT)) { p->pos++; n->i2++; }
                if (!(n->i2 > 0 && checa_kw(p, "import"))) {
                    if (caminho_modulo(p, &n->lista) != 0) return NULL;
                }
            }
            if (!aceita_kw(p, "import")) {
                perro(p, "esperado 'import' apos o modulo", atual(p)); return NULL;
            }
            if (lista_com_alias(p, &n->lista2, &n->lista2_alias) != 0) return NULL;
            return n;
        }
        p->pos++;                                  /* PUSH */
        n->texto = dup_str(p, "push");
        int aspas_push = modulo_entre_aspas(p, n);
        if (aspas_push < 0) return NULL;
        if (!aspas_push && caminho_modulo(p, &n->lista) != 0) return NULL;
        if (aceita_kw(p, "as")) {
            n->texto2 = nome_livre(p, "esperado nome apos 'as'");
            if (FALHOU(p)) return NULL;
        }
        if (aceita_kw(p, "GET")) {
            if (lista_com_alias(p, &n->lista2, &n->lista2_alias) != 0) return NULL;
        }
        return n;
    }

    /* @decorador[.metodo](args) [bloco | action] */
    if (checa(p, T_AT)) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_DECORATOR_STMT, t->line, t->col);
        if (!n) return NULL;
        PSNode *dc = ps_node_novo(p->arena, N_DECORATOR_CALL, t->line, t->col);
        if (!dc) return NULL;
        if (caminho_modulo(p, &dc->lista) != 0) return NULL;
        if (aceita(p, T_LPAREN)) {
            dc->i2 = 1;   /* teve parênteses: é chamada (mesmo sem args) */
            pula_separadores(p);
            if (!checa(p, T_RPAREN)) {
                for (;;) {
                    pula_separadores(p);
                    PSToken *at = atual(p);
                    const char *nome_arg = NULL;
                    if ((at->type == T_IDENT || at->type == T_IDENT_UPPER || at->type == T_KW)
                            && espia(p, 1)->type == T_OP && espia(p, 1)->texto
                            && strcmp(espia(p, 1)->texto, "=") == 0) {
                        nome_arg = dup_tok(p, at);
                        p->pos += 2;
                    }
                    PSNode *v = expressao(p);
                    if (FALHOU(p)) return NULL;
                    PSNode *arg = ps_node_novo(p->arena, N_CALL_ARG, at->line, at->col);
                    if (!arg) return NULL;
                    arg->a = v; arg->texto = nome_arg;
                    if (ps_vec_push(p->arena, &dc->lista2, arg) != 0) return NULL;
                    pula_separadores(p);
                    if (!aceita(p, T_COMMA)) break;
                    pula_separadores(p);
                    if (checa(p, T_RPAREN)) break;
                }
            }
            pula_separadores(p);
            if (!exige(p, T_RPAREN, "faltou ')' no decorador")) return NULL;
        }
        n->a = dc;
        pula_separadores(p);

        if (checa(p, T_COLON) || checa(p, T_LBRACE)) {
            n->b = bloco(p);
            if (FALHOU(p)) return NULL;
        } else if (!p->dec_sem_captura) {
            /* `@NonNull action f()` — a action vira um bloco de um nó só.
             *
             * A cabeça da action é UMA unidade: `[public|private] {async|tipo}*
             * action|reaction`, em qualquer ordem dos modificadores — a mesma
             * que o parser de statement aceita. Aqui havia uma cópia à mão
             * que conhecia quatro formas (`action`, `tipo action`, `async
             * action`, `async tipo action`) e NÃO conhecia `tipo async
             * action`. O resultado era o pior possível: `@app.post(...)` em
             * cima de `int async action handler()` compilava limpo, a action
             * virava um statement solto SEM decorator, a rota nunca era
             * registrada, e o cliente recebia 404 — sem um aviso sequer. */
            PSToken *nt = atual(p);
            int eh_action = (cabeca_de_funct(p, 0) >= 0);
            /* @app.route(...) class Nome(): ... — handler baseado em classe.
             * O decorador captura a classe (com prefixo private/public opcional)
             * como bloco; o compilador enxerga a action dentro dela. */
            int eh_classe = (nt->type == T_KW && nt->texto
                             && (strcmp(nt->texto,"Entity")==0 || strcmp(nt->texto,"class")==0
                                 || strcmp(nt->texto,"Class")==0));
            if (!eh_classe && nt->type == T_KW && nt->texto
                    && (strcmp(nt->texto,"private")==0 || strcmp(nt->texto,"public")==0)) {
                PSToken *n2 = espia(p, 1);
                eh_classe = (n2->type == T_KW && n2->texto
                             && (strcmp(n2->texto,"Entity")==0 || strcmp(n2->texto,"class")==0
                                 || strcmp(n2->texto,"Class")==0));
            }
            if (eh_action || eh_classe) {
                PSNode *acao = statement(p);
                if (FALHOU(p)) return NULL;
                PSNode *b = ps_node_novo(p->arena, N_BLOCK, acao->line, acao->col);
                if (!b) return NULL;
                b->estilo = "brace";
                if (ps_vec_push(p->arena, &b->lista, acao) != 0) return NULL;
                n->b = b;
            }
        }
        return n;
    }

    /* model Nome() { campo: tipo(length=N) } */
    if (checa_kw(p, "model")) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_MODEL_DECL, t->line, t->col);
        if (!n) return NULL;
        n->texto = exige_nome(p, "model");
        if (FALHOU(p)) return NULL;
        if (!exige(p, T_LPAREN, "esperado '(' apos nome do model")) return NULL;
        if (!exige(p, T_RPAREN, "esperado ')' apos '('")) return NULL;
        pula_separadores(p);
        PSToken *abre_model = exige(p, T_LBRACE, "esperado '{' para abrir o model");
        if (!abre_model) return NULL;
        pula_indent_solto(p);
        while (!checa(p, T_RBRACE) && !checa(p, T_EOF)) {
            PSToken *ft = atual(p);
            const char *campo = exige_nome(p, "campo");
            if (FALHOU(p)) return NULL;
            if (!exige(p, T_COLON, "esperado ':' apos nome do campo")) return NULL;
            PSToken *tt = atual(p);
            /* os quatro tipos de campo, mais os apelidos de str e int
             * (`string`/`String`, `integer`/`Integer`) */
            int tipo_ok = tt->texto
                && ((tt->type == T_KW
                     && (strcmp(tt->texto,"str")==0 || strcmp(tt->texto,"int")==0
                         || strcmp(tt->texto,"flo")==0 || strcmp(tt->texto,"bool")==0))
                    || (eh_apelido_tipo(tt)
                        && (strcmp(tt->texto,"string")==0 || strcmp(tt->texto,"String")==0
                            || strcmp(tt->texto,"integer")==0 || strcmp(tt->texto,"Integer")==0)));
            if (!tipo_ok) {
                perro(p, "tipo do campo deve ser str, int, flo ou bool", tt); return NULL;
            }
            p->pos++;
            PSNode *f = ps_node_novo(p->arena, N_MODEL_FIELD, ft->line, ft->col);
            if (!f) return NULL;
            f->texto = campo;
            f->texto2 = dup_tok(p, tt);
            f->i2 = -1;                       /* -1 = sem length */
            if (aceita(p, T_LPAREN)) {
                if (!exige(p, T_IDENT, "esperado 'length'")) return NULL;
                if (!checa_op(p, "=")) { perro(p, "esperado '=' apos 'length'", atual(p)); return NULL; }
                p->pos++;
                PSToken *lt = atual(p);
                if (lt->type != T_INT) { perro(p, "esperado numero apos 'length='", lt); return NULL; }
                p->pos++;
                f->i2 = (int32_t)lt->i;
                if (!exige(p, T_RPAREN, "esperado ')' apos o valor de length")) return NULL;
            }
            if (ps_vec_push(p->arena, &n->lista, f) != 0) return NULL;
            pula_indent_solto(p);
        }
        pula_indent_solto(p);
        if (!exige_fecha(p, T_RBRACE, "faltou '}' no model", abre_model)) return NULL;
        return n;
    }

    /* enum Nome { A, B=v } — namespace de constantes; Nome.A devolve o valor.
     * Membro é NOME (auto) ou NOME = expr (explícito); vírgula opcional. */
    if (checa_kw(p, "enum")) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_ENUM_DECL, t->line, t->col);
        if (!n) return NULL;
        n->texto = exige_nome(p, "enum");
        if (FALHOU(p)) return NULL;
        pula_separadores(p);
        PSToken *abre_enum = exige(p, T_LBRACE, "esperado '{' para abrir o enum");
        if (!abre_enum) return NULL;
        pula_indent_solto(p);
        while (!checa(p, T_RBRACE) && !checa(p, T_EOF)) {
            PSToken *mt = atual(p);
            const char *nome_m = exige_nome(p, "membro");
            if (FALHOU(p)) return NULL;
            PSNode *m = ps_node_novo(p->arena, N_ENUM_MEMBER, mt->line, mt->col);
            if (!m) return NULL;
            m->texto = nome_m;
            if (checa_op(p, "=")) {
                p->pos++;
                m->a = expressao(p);          /* valor explícito */
                if (FALHOU(p)) return NULL;
            }
            if (ps_vec_push(p->arena, &n->lista, m) != 0) return NULL;
            aceita(p, T_COMMA);               /* vírgula opcional entre membros */
            pula_indent_solto(p);
        }
        pula_indent_solto(p);
        if (!exige_fecha(p, T_RBRACE, "faltou '}' no enum", abre_enum)) return NULL;
        return n;
    }

    /* `count each <tipo> in <cont> { ... }` — statement quando há bloco */
    if (checa_kw(p, "count") && espia(p, 1)->type == T_KW
            && espia(p, 1)->texto && strcmp(espia(p, 1)->texto, "each") == 0) {
        int32_t salvo = p->pos;
        p->pos += 2;
        const char *tipo; PSNode *valor;
        if (count_tipo_valor(p, &tipo, &valor) != 0) return NULL;
        if (!aceita_kw(p, "in")) {
            perro(p, "esperado 'in' apos o tipo de 'count each'", atual(p)); return NULL;
        }
        PSNode *cont = soma(p);
        if (FALHOU(p)) return NULL;
        if (checa(p, T_LBRACE) || checa(p, T_COLON)) {
            PSNode *n = ps_node_novo(p->arena, N_COUNT_EACH_STMT, t->line, t->col);
            if (!n) return NULL;
            n->texto = tipo; n->b = valor; n->c = cont;
            n->e = bloco(p);
            if (FALHOU(p)) return NULL;
            return n;
        }
        /* sem bloco: é expressão — refaz pelo caminho normal */
        p->pos = salvo;
    }

    /* match <expr> { case ... } | match <expr>: case ...: */
    /* `match` e palavra reservada: `match (x) {` e o statement com o sujeito
     * entre parenteses (precedencia, como em `if (a) or (b) {`). A exclusao
     * de `(` que havia aqui mandava `match (1 + 1) {` pro caminho de
     * expressao, e o `{` virava "faltou ':' no dicionario". */
    if (checa_kw(p, "match")) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_MATCH_STMT, t->line, t->col);
        if (!n) return NULL;
        {   /* `match s {` — o `{` abre bloco, não interpola o sujeito */
            int salvo = p->chave_abre_bloco;
            p->chave_abre_bloco = 1;
            n->a = expressao(p);
            p->chave_abre_bloco = salvo;
        }
        if (FALHOU(p)) return NULL;
        pula_separadores(p);

        PSToken *abre_match = atual(p);
        int estilo_chaves = aceita(p, T_LBRACE);
        if (!estilo_chaves) {
            if (!exige(p, T_COLON, "esperado ':' ou '{' apos expressao do match")) return NULL;
            pula_separadores(p);
            if (!exige(p, T_INDENT, "esperado indentacao apos 'match:'")) return NULL;
        }
        /* No estilo de chaves o corpo do match é `{ }`: a indentação dentro
         * dele é livre, então os INDENT/DEDENT soltos são ignorados aqui. */
        if (estilo_chaves) pula_indent_solto(p); else pula_separadores(p);

        while (checa_kw(p, "case")) {
            PSToken *ct = atual(p);
            p->pos++;
            PSNode *pat = padrao(p);
            if (FALHOU(p)) return NULL;
            if (checa_kw(p, "if")) {          /* guarda: case X if cond */
                p->pos++;
                /* `case x if m == 'GET' {` — sem isto o `{` do bloco era lido
                 * como interpolação da string que fecha a guarda, e o `case`
                 * ficava sem corpo. Mesmo tratamento do `if` e do `for each`. */
                int salvo = p->chave_abre_bloco;
                p->chave_abre_bloco = 1;
                pat->a = expressao(p);
                p->chave_abre_bloco = salvo;
                if (FALHOU(p)) return NULL;
            }
            PSNode *caso = ps_node_novo(p->arena, N_MATCH_CASE, ct->line, ct->col);
            if (!caso) return NULL;
            caso->a = pat;

            if (estilo_chaves) {
                pula_indent_solto(p);
                caso->b = bloco(p);
                if (FALHOU(p)) return NULL;
            } else {
                if (!exige(p, T_COLON, "esperado ':' apos padrao do case")) return NULL;
                pula_separadores(p);
                /* corpo vai até o próximo case, DEDENT ou EOF */
                PSNode *b = ps_node_novo(p->arena, N_BLOCK, ct->line, ct->col);
                if (!b) return NULL;
                b->estilo = "colon";
                if (checa(p, T_INDENT)) {
                    p->pos++;
                    pula_separadores(p);
                    while (!checa(p, T_DEDENT) && !checa(p, T_EOF) && !checa_kw(p, "case")) {
                        PSNode *st = statement(p);
                        if (FALHOU(p)) return NULL;
                        if (st && ps_vec_push(p->arena, &b->lista, st) != 0) return NULL;
                        pula_separadores(p);
                    }
                    if (checa(p, T_DEDENT)) p->pos++;
                } else {
                    PSNode *st = statement(p);
                    if (FALHOU(p)) return NULL;
                    if (st && ps_vec_push(p->arena, &b->lista, st) != 0) return NULL;
                }
                caso->b = b;
            }
            if (ps_vec_push(p->arena, &n->lista, caso) != 0) return NULL;
            if (estilo_chaves) pula_indent_solto(p); else pula_separadores(p);
        }

        if (estilo_chaves) { if (!exige_fecha(p, T_RBRACE, "faltou '}' no match", abre_match)) return NULL; }
        else               { if (!exige(p, T_DEDENT, "match indentado nao fechou")) return NULL; }
        return n;
    }

    /* try { } catch (e) { } [catch (Tipo n) { }] [finally { }] */
    if (checa_kw(p, "try")) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_TRY_CATCH_STMT, t->line, t->col);
        if (!n) return NULL;
        n->a = bloco(p);
        if (FALHOU(p)) return NULL;

        int32_t salvo = p->pos;
        pula_separadores(p);
        while (checa_kw(p, "catch")) {
            PSToken *ct = atual(p);
            p->pos++;
            if (!exige(p, T_LPAREN, "esperado '(' apos 'catch'")) return NULL;

            PSToken *tk = atual(p);
            const char *tipo = NULL;
            const char *nome = NULL;
            /* catch (Tipo [nome]) — o nome é OPCIONAL; catch (nome) — captura
             * tudo ligando a variável; catch () — captura tudo sem variável. */
            if (tk->type == T_IDENT_UPPER) {
                tipo = dup_tok(p, tk);
                p->pos++;
                if (atual(p)->type == T_IDENT || atual(p)->type == T_IDENT_UPPER) {
                    nome = exige_nome(p, "erro");
                    if (FALHOU(p)) return NULL;
                }
            } else if (atual(p)->type != T_RPAREN) {
                nome = exige_nome(p, "erro");
                if (FALHOU(p)) return NULL;
            }
            if (!exige(p, T_RPAREN, "esperado ')' apos catch")) return NULL;

            PSNode *cl = ps_node_novo(p->arena, N_CATCH_CLAUSE, tk->line, tk->col);
            if (!cl) return NULL;
            cl->texto = nome;
            cl->texto2 = tipo;
            cl->b = bloco(p);
            if (FALHOU(p)) return NULL;
            if (ps_vec_push(p->arena, &n->lista, cl) != 0) { perro(p, "sem memoria", ct); return NULL; }

            salvo = p->pos;
            pula_separadores(p);
        }
        /* `try { } finally { }` SEM catch nenhum é válido, e é o que o Python
         * faz: o finally roda e a exceção (se houver) propaga depois dele. O
         * parser exigia catch, então a forma "faça isto aconteça o que
         * acontecer, sem tratar o erro" — fechar arquivo, soltar trava,
         * derrubar servidor — não existia. Só é erro quando não há NEM catch
         * NEM finally: aí o `try` não pediria nada. */
        if (n->lista.n == 0 && !checa_kw(p, "finally")) {
            perro(p, "esperado 'catch' ou 'finally' apos bloco do try", atual(p));
            return NULL;
        }

        if (checa_kw(p, "finally")) {
            p->pos++;
            n->c = bloco(p);
            if (FALHOU(p)) return NULL;
        } else {
            p->pos = salvo;      /* não havia finally: devolve os separadores */
        }
        return n;
    }

    /* using <expr> as <nome> { } */
    if (checa_kw(p, "using")) {
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_USING_STMT, t->line, t->col);
        if (!n) return NULL;
        n->a = expressao(p);
        if (FALHOU(p)) return NULL;
        if (!aceita_kw(p, "as")) { perro(p, "esperado 'as' apos expressao de 'using'", atual(p)); return NULL; }
        PSToken *vt = atual(p);
        if (vt->type != T_IDENT && vt->type != T_IDENT_UPPER && vt->type != T_KW) {
            perro(p, "esperado nome de variavel apos 'as'", vt); return NULL;
        }
        p->pos++;
        n->texto = dup_tok(p, vt);
        n->b = bloco(p);
        if (FALHOU(p)) return NULL;
        return n;
    }

    /* Ponto de entrada: `if __name__ == "main":` — o bloco roda quando o
     * arquivo é executado direto e é PULADO quando ele é importado.
     *
     * É reconhecido pela FORMA, não avaliando a condição: `__name__` vale o
     * caminho do arquivo (é o que se passa pro `Jinker`), então compará-lo com
     * "main" nunca daria verdadeiro. O parser vê o desenho e emite o guard.
     *
     * É também o único lugar onde `:` ainda abre bloco (ver `bloco_entrada`);
     * `{ }` vale igual.
     */
    if (checa_kw(p, "if")) {
        int32_t k = p->pos + 1;
        int paren = 0;
        if (k < p->n && p->toks[k].type == T_LPAREN) { paren = 1; k++; }
        if (k + 2 < p->n
            && p->toks[k].type == T_IDENT && p->toks[k].texto
            && strcmp(p->toks[k].texto, "__name__") == 0
            && p->toks[k + 1].type == T_OP && p->toks[k + 1].texto
            && strcmp(p->toks[k + 1].texto, "==") == 0
            && p->toks[k + 2].type == T_STR) {
            PSToken *rot = &p->toks[k + 2];
            int32_t depois = k + 3;
            if (paren) {
                if (depois >= p->n || p->toks[depois].type != T_RPAREN) goto if_normal;
                depois++;
            }
            /* A chave pode vir na linha seguinte, como em todo bloco da
             * linguagem. Sem isto o guard com `{` embaixo caia no `if`
             * comum — que compara `__name__` com a string "main" de verdade,
             * da falso, e o bloco era pulado sem erro nenhum. */
            {
                int32_t q = depois;
                while (q < p->n && p->toks[q].type == T_NEWLINE) q++;
                if (q < p->n && p->toks[q].type == T_LBRACE) depois = q;
            }
            if (depois >= p->n
                || (p->toks[depois].type != T_COLON && p->toks[depois].type != T_LBRACE))
                goto if_normal;
            PSNode *n = ps_node_novo(p->arena, N_RUN_SELFWITH_STMT, t->line, t->col);
            if (!n) return NULL;
            n->texto = dup_tok(p, rot);
            p->pos = depois;
            n->b = bloco_entrada(p);
            if (FALHOU(p)) return NULL;
            return n;
        }
        if_normal:
        return if_stmt(p);
    }
    if (checa_kw(p, "while")) return while_stmt(p);
    if (checa_kw(p, "for"))   return for_stmt(p);
    if (checa_kw(p, "return")) return return_stmt(p);

    /* funct com modificadores em QUALQUER ordem — a ordem é do usuário:
     * {public|private|async|tipo|static|nonnull}* funct. Vale `int async funct`,
     * `public async funct`, `static funct`, `static private funct`,
     * `private nonnull int funct`, `int static funct`... */
    {
        int is_priv = -1;
        if (cabeca_de_funct(p, 0) >= 0) {
            int is_async = 0, eh_static = 0, eh_nonnull = 0; const char *tipo = NULL;
            PSToken *cur;
            for (;;) {
                cur = atual(p);
                if (cur->type == T_KW && cur->texto && strcmp(cur->texto, "async") == 0) is_async = 1;
                else if (cur->type == T_KW && cur->texto && strcmp(cur->texto, "public") == 0) is_priv = 0;
                else if (cur->type == T_KW && cur->texto && strcmp(cur->texto, "private") == 0) is_priv = 1;
                else if (eh_tipo_de_retorno(cur)) tipo = tipo_retorno_dup(p, cur);
                else if (eh_mod_funct(cur)) {
                    if (strcmp(cur->texto, "static") == 0) eh_static = 1; else eh_nonnull = 1;
                } else break;
                p->pos++;
            }
            PSNode *ad = action_decl(p, is_async, tipo);
            if (ad && is_priv >= 0) ad->is_private = is_priv;
            if (ad) { ad->is_static = eh_static; ad->is_nonnull = eh_nonnull; }
            return ad;
        }

        /* A cabeça `{async|tipo}+ NOME` SEM action/reaction é UMA unidade: uma
         * função com o `action` esquecido — nunca "uma variável chamada
         * async". Sem isto, `int async LoginHandler(data)` caía na declaração
         * tipada e o erro era "'async' e palavra reservada ... nome de
         * variavel" (o parser tinha lido `int async` como `int <nome>`);
         * `async f(x) {` virava chamada + literal de dict e o erro falava de
         * dicionário; `int f(x) {` dizia "exige '='". Três mensagens pro
         * mesmo esquecimento, e nenhuma com a palavra que faltava.
         *
         * `int x = 1` continua declaração tipada: só é função aqui se a cabeça
         * tem `async` (que nunca declara variável) ou se o nome vem seguido
         * de `(`, que declaração nenhuma tem. A mensagem devolve a linha
         * montada com os modificadores que a pessoa escreveu. */
        /* Daqui pra baixo NÃO é funct: é o campo com visibilidade
         * (`private str x = v`) e as mensagens de cabeça mal escrita. A
         * visibilidade é relida do token, e não herdada do ramo da funct
         * acima — que agora aceita `public` em qualquer posição e sai por
         * `return`, deixando `is_priv` em -1 aqui. Sem reler, `public str
         * name = nome` dentro do construtor virava campo PRIVATE e o acesso
         * de fora passava a ser negado. */
        PSToken *m0 = espia(p, 0);
        int tem_visib = (m0->type == T_KW && m0->texto
                         && (strcmp(m0->texto, "public") == 0 || strcmp(m0->texto, "private") == 0));
        if (tem_visib) is_priv = (strcmp(m0->texto, "private") == 0);
        int off = tem_visib ? 1 : 0;
        int j = off;
        PSToken *mk;
        while ((mk = espia(p, j))->type == T_KW && mk->texto
                && (strcmp(mk->texto, "async") == 0 || eh_tipo_kw(mk))) j++;
        PSToken *ap = espia(p, j);
        if (j > off && (ap->type == T_IDENT || ap->type == T_IDENT_UPPER)) {
            int tem_async = 0;
            for (int k = off; k < j; k++)
                if (strcmp(espia(p, k)->texto, "async") == 0) tem_async = 1;
            if (tem_async || espia(p, j + 1)->type == T_LPAREN) {
                char mods[64]; size_t w = 0; mods[0] = '\0';
                for (int k = off; k < j; k++) {
                    int n = snprintf(mods + w, sizeof(mods) - w, "%s ", espia(p, k)->texto);
                    if (n < 0 || (size_t)n >= sizeof(mods) - w) { mods[w] = '\0'; break; }
                    w += (size_t)n;
                }
                char m[240];
                const char *nome = ap->texto ? ap->texto : "?";
                /* `int async reaction h(a)`: o que está no lugar do nome é a
                 * grafia que saiu. Dizer "faltou funct antes de 'reaction'"
                 * mandaria escrever `funct reaction(...)`, que não é o
                 * conserto — o conserto é trocar a palavra. */
                if (eh_grafia_morta(ap))
                    snprintf(m, sizeof(m),
                             "'%s' saiu da linguagem; troque por 'funct': %sfunct %.40s(...)",
                             nome, mods, espia(p, j + 1)->texto ? espia(p, j + 1)->texto : "nome");
                else
                    snprintf(m, sizeof(m),
                             "faltou 'funct' antes de '%.40s': %sfunct %.40s(...)",
                             nome, mods, nome);
                perro(p, m, ap);
                return NULL;
            }
        }

        /* `private str name = nome` — CAMPO DO OBJETO declarado dentro da
         * action, com visibilidade. É a forma que ele pediu: o modificador
         * na frente da declaração tipada que a linguagem já tinha
         * (`str x = "a"`), e não a ordem do corpo da classe.
         *
         * O QUE HAVIA ANTES: nada disto era erro. `private` chegava no parser
         * de expressão, virava um NOME comum, e o programa compilava limpo
         * pra estourar `NameError: name 'private' is not defined` em tempo de
         * execução — dentro de `int action`, engolido pro 500 da seção 6.4,
         * ou seja, calado. Palavra reservada lida como variável é o pior dos
         * dois mundos: não faz o que diz e não avisa. */
        if (off) {
            PSToken *tt  = espia(p, 1);
            PSToken *nmt = espia(p, 2);

            /* `private nome: tipo = v` — a ordem do CORPO da Entity, escrita
             * dentro da action. Era "expressao invalida" apontando pro ':',
             * que não diz nada. A mensagem agora diz o conserto. */
            if ((tt->type == T_IDENT || tt->type == T_IDENT_UPPER)
                    && nmt->type == T_COLON) {
                char m[240];
                snprintf(m, sizeof(m),
                         "'%s nome: tipo' so vale no corpo da Entity — dentro de uma funct escreva "
                         "'%s <tipo> %s = <valor>'",
                         m0->texto, m0->texto, tt->texto ? tt->texto : "nome");
                perro(p, m, tt);
                return NULL;
            }

            /* `<tipo> <nome> = <valor>`. O tipo é o mesmo conjunto que o campo
             * `nome: tipo` aceita (qualquer nome de tipo, Entity inclusive);
             * a checagem em runtime só existe pros escalares, igual ao
             * `str x = "a"` sem modificador. */
            if ((tt->type == T_IDENT || tt->type == T_IDENT_UPPER || tt->type == T_KW)
                    && (nmt->type == T_IDENT || nmt->type == T_IDENT_UPPER)) {
                p->pos++;                                   /* private/public */
                const char *tipo = dup_tok(p, atual(p));
                p->pos++;                                   /* tipo */
                PSToken *nt2 = atual(p);
                const char *nome = exige_nome(p, "campo");
                if (FALHOU(p)) return NULL;
                if (!checa_op(p, "=")) {
                    perro(p, "campo declarado dentro de funct precisa de '=' e um valor", atual(p));
                    return NULL;
                }
                p->pos++;
                PSNode *n = ps_node_novo(p->arena, N_FIELD_DECL, nt2->line, nt2->col);
                if (!n) return NULL;
                n->texto = nome; n->texto2 = tipo;
                n->is_private = is_priv;
                n->a = expressao(p);
                if (FALHOU(p)) return NULL;
                return n;
            }

            /* Sobrou: `private` sem nada válido atrás. Os usos legítimos
             * (classe, action/reaction, campo) já retornaram acima. */
            char m[200];
            snprintf(m, sizeof(m),
                     "'%s' so vale antes de class/Entity, de funct ou de "
                     "'<tipo> <nome> = <valor>'", m0->texto);
            perro(p, m, m0);
            return NULL;
        }
    }

    /* tipo de retorno antes de action: `int action f()`; ou declaracao tipada
     * `int x = 1`. So entra aqui se o que vem depois do tipo e uma ACTION ou
     * um NOME — `list(x)`, `json.parse(s)`, `str(n)` como statement continuam
     * expressao (chamada/modulo), como o ramo de expressao ja tratava. */
    if (eh_tipo_kw_decl(t)
            && (eh_kw_funct(espia(p, 1))
                || espia(p, 1)->type == T_IDENT || espia(p, 1)->type == T_IDENT_UPPER)) {
        PSToken *nx = espia(p, 1);
        if (eh_kw_funct(nx)) {
            if (!eh_tipo_de_retorno(t)) {
                perro(p, MSG_TIPO_RETORNO, t);
                return NULL;
            }
            const char *tipo = tipo_retorno_dup(p, t);
            p->pos++;
            return action_decl(p, 0, tipo);
        }
        /* declaração tipada: `int x = 1` */
        const char *tipo = dup_tok(p, t);
        p->pos++;
        PSToken *nt = atual(p);
        const char *nome = exige_nome(p, "variavel");
        if (FALHOU(p)) return NULL;
        if (!(checa_op(p, "="))) { perro(p, "declaracao de variavel exige '='", atual(p)); return NULL; }
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_VAR_DECL, nt->line, nt->col);
        if (!n) return NULL;
        n->texto = nome; n->texto2 = tipo;
        n->a = expressao(p);
        if (FALHOU(p)) return NULL;
        return n;
    }

    /* desempacotamento: `a, b = 1, 2` / `a, *resto = l` / `(a, b), c = x` */
    if (parece_unpack(p)) {
        PSNode *n = ps_node_novo(p->arena, N_UNPACK_ASSIGNMENT, t->line, t->col);
        if (!n) return NULL;
        n->a = alvos_unpack(p);
        if (FALHOU(p)) return NULL;
        if (!checa_op(p, "=")) { perro(p, "desempacotamento exige '='", atual(p)); return NULL; }
        p->pos++;
        PSNode *v = expressao(p);
        if (FALHOU(p)) return NULL;
        if (aceita(p, T_COMMA)) {
            PSNode *tup = ps_node_novo(p->arena, N_TUPLE_LITERAL, t->line, t->col);
            if (!tup) return NULL;
            if (ps_vec_push(p->arena, &tup->lista, v) != 0) return NULL;
            while (!checa(p, T_NEWLINE) && !checa(p, T_SEMI) && !checa(p, T_EOF)
                   && !checa(p, T_RBRACE) && !checa(p, T_DEDENT)) {
                PSNode *it = expressao(p);
                if (FALHOU(p)) return NULL;
                if (ps_vec_push(p->arena, &tup->lista, it) != 0) return NULL;
                if (!aceita(p, T_COMMA)) break;
            }
            n->b = tup;
        } else {
            n->b = v;
        }
        return n;
    }

    /* atribuição simples: NOME <op>= expr */
    if ((t->type == T_IDENT || t->type == T_IDENT_UPPER)) {
        PSToken *nx = espia(p, 1);
        if (nx->type == T_OP && nx->texto && eh_op_atribuicao(nx->texto)) {
            if (eh_contextual_reservada(t->texto)) {
                char m[200];
                snprintf(m, sizeof(m),
                         "'%s' e palavra reservada da linguagem e nao pode ser usada como nome de variavel",
                         t->texto);
                perro(p, m, t);
                return NULL;
            }
            const char *alvo = dup_tok(p, t);
            const char *op = dup_tok(p, nx);
            p->pos += 2;
            PSNode *n = ps_node_novo(p->arena, N_ASSIGNMENT, t->line, t->col);
            if (!n) return NULL;
            n->texto = alvo; n->texto2 = op;
            n->a = expressao(p);
            if (FALHOU(p)) return NULL;
            return n;
        }
    }

    /* expressão solta */
    PSNode *e = expressao(p);
    if (FALHOU(p)) return NULL;

    /* `alvo[i] = v` e as formas aumentadas. Reconhecido DEPOIS de montar a
     * expressão, não por lookahead: o alvo pode ser qualquer coisa indexável
     * (`l[0][1]`, `obj.d["k"]`) e varrer colchetes balanceados à frente seria
     * refazer o trabalho que o parser de expressão já fez. */
    if (e->kind == N_INDEX_ACCESS && checa(p, T_OP) && atual(p)->texto
            && eh_op_atribuicao(atual(p)->texto)) {
        const char *op = dup_tok(p, atual(p));
        p->pos++;
        PSNode *n = ps_node_novo(p->arena, N_INDEX_ASSIGNMENT, e->line, e->col);
        if (!n) return NULL;
        n->a = e->a;          /* container */
        n->b = e->b;          /* índice    */
        n->texto = op;
        n->c = expressao(p);
        if (FALHOU(p)) return NULL;
        return n;
    }

    PSNode *n = ps_node_novo(p->arena, N_EXPRESSION_STMT, t->line, t->col);
    if (!n) return NULL;
    n->a = e;
    return n;
}

/* ── entrada ────────────────────────────────────────────────────────────── */
PSParseResult *ps_parse(PSToken *toks, int32_t n)
{
    PSParseResult *r = calloc(1, sizeof(PSParseResult));
    if (!r) return NULL;
    r->ok = 1;
    ps_arena_init(&r->arena);

    /* ZERADO de uma vez, e não campo a campo. Ao acrescentar `prof` eu esqueci
     * de inicializá-lo aqui: ele nasceu com lixo de pilha, o teto disparou de
     * cara e 3968 casos da suíte falharam. Campo novo em struct inicializada à
     * mão é convite a exatamente isso. */
    P p = {0};
    p.toks = toks; p.n = n;
    p.arena = &r->arena; p.out = r;

    PSNode *prog = ps_node_novo(&r->arena, N_PROGRAM, 1, 1);
    if (!prog) { r->ok = 0; snprintf(r->erro, sizeof(r->erro), "sem memoria"); return r; }

    pula_separadores(&p);
    while (!checa(&p, T_EOF) && r->ok) {
        PSNode *s = statement(&p);
        if (!r->ok) break;
        if (s && ps_vec_push(&r->arena, &prog->lista, s) != 0) {
            r->ok = 0; snprintf(r->erro, sizeof(r->erro), "sem memoria"); break;
        }
        pula_separadores(&p);
        if (s == NULL && checa(&p, T_EOF)) break;
    }
    r->programa = prog;
    return r;
}

void ps_parse_free(PSParseResult *r)
{
    if (!r) return;
    ps_arena_free(&r->arena);
    free(r);
}
