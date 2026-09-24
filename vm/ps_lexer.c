/*
 * Lexer da Jinga em C puro.
 *
 * As regras que não são óbvias, e que os testes diferenciais cobrem:
 *
 *   - Indentação ESTRITA: só espaços (TAB é erro), múltiplo de 4, e sobe
 *     no máximo um nível por vez.
 *   - Dentro de ( [ { a indentação é ignorada e NEWLINE não é emitido.
 *   - Linha que começa com `.membro` é continuação da anterior (method
 *     chaining multi-linha) — não emite NEWLINE nem mexe na pilha de indent.
 *   - `"""` é comentário de bloco, NÃO string. String multi-linha usa `'''`.
 *   - `<hex>` / `<nome>` vira COLOR só se o hex tiver exatamente 3 ou 6
 *     dígitos ou o nome estiver na tabela; senão `<` volta a ser operador.
 *
 * Sem dependência de fora: só a libc.
 */
#include "ps_lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── tabelas da linguagem ───────────────────────────────────────────────── */

static const char *KEYWORDS[] = {
    "if", "else", "elif", "while", "for", "each", "in", "is",
    "and", "or", "not", "Not",
    "funct", "return", "continue", "break", "pass", "model", "enum", "async", "await",
    "try", "catch", "as", "with", "of", "using",
    "import", "from", "PUSH", "GET",
    /* `long`: inteiro de qualquer tamanho. `int` promete caber em 64 bits e
     * por isso recusa bignum; `long` nao promete, e aceita os dois. */
    "str", "int", "long", "flo", "bool",
    /* `post`, `input` e `addEnd` sao BUILTINS (estao na tabela BUILTINS[] da
     * VM). `listen`, `route`, `create`, `clear` e `space` SAIRAM daqui: nao
     * eram construcao da linguagem nem builtin — `route` e `listen` sao METODOS
     * (`app.route(...)`, `sock.listen()`), e acesso a membro nao precisa que o
     * nome seja reservado. Enquanto estavam nesta tabela, `route = 1` era
     * "palavra reservada da linguagem e nao pode ser usada como nome de
     * variavel", e o editor pintava `route` de palavra-chave em qualquer
     * lugar — inclusive como nome de variavel de quem escreve. */
    "post", "input", "addEnd", "char", "list",
    "Class", "class", "type",
    "Entity", "self",
    "private", "public",
    "match", "case",
    "yield",
    "raise", "finally",
    "to",
    "count",
    "global",
    "POST", "PUT", "DELETE", "JSON", "json",
    /* `dict` é apelido de `json`; `tup` nomeia a tupla */
    "dict", "tup",
    /* `Object`/`object`: o tipo de qualquer objeto que nao e str/list/dict/
     * tup/bytes — instancia de classe, servidor, conexao, arquivo. E o que
     * se declara pra receber o que uma lib devolve: `Object app = Jinker()`. */
    "Object", "object",
    NULL
};

/* A lista de palavras-chave, pra quem precisa dela FORA do lexer — o
 * `--metadata` a publica e o editor sugere `funct`, `if`, `for each`… a
 * partir daí, em vez de manter uma cópia digitada que envelhece. */
const char *const *ps_lexer_keywords(void)
{
    return KEYWORDS;
}
/*as cores devem funcionar no hexadecimal tbm*/
static const char *CORES[] = {
    "red", "green", "blue", "yellow", "cyan", "magenta", "white", "black",
    "purple", "orange", "pink", "gray", "grey", "lime", "teal", NULL
};

/* mais longos primeiro — a ordem decide `<=` vs `<` */
static const char *MULTI_OPS[] = {
    "===", "!==", "==", "!=", "<=", ">=", "&&", "||", "<<", ">>",
    /* `**` ANTES de `*=` e de `*`: mais longo primeiro, senao `2 ** 3` seria
     * lido como `2 * (*3)`. Potencia — I11. E `//` (divisao inteira), que
     * ate hoje era comentario de linha. */
    "**", "//",
    "+=", "-=", "*=", "/=", "%=", "++", "--", NULL
};

static const char SINGLE_OPS[] = "+-*/%=<>!.,@|^&~";

#define INDENT_UNIT 4

/* ── estado ─────────────────────────────────────────────────────────────── */
typedef struct {
    const char *src;
    size_t      len;
    size_t      pos;
    int32_t     linha;
    int32_t     col;

    int32_t     indent[128];
    int         nindent;

    int         paren_depth;   /* só ( e [ — ver trata_newline */
    /* `{` abertos. Dentro de um bloco de chaves a indentação é LIVRE (1
     * espaço, 2, tab): quem delimita é o `}`. Os INDENT/DEDENT continuam
     * sendo emitidos, porque um sub-bloco `:` lá dentro precisa deles — o que
     * cai é só a validação de "múltiplo de 4" e "avançou exatamente 4". */
    int         chave_depth;
    /* Que TIPO de `{` é cada um dos abertos: 1 = dicionário, 0 = bloco.
     *
     * Num dicionário a indentação não significa nada e não pode nem tocar a
     * pilha de indentação — `{"a": 1,\n     "b": 2}` empurrava um nível que
     * ninguém tirava, e a linha seguinte vinha com um DEDENT órfão
     * ("expressao invalida"). Num BLOCO ela conta, porque pode haver um
     * sub-bloco `:` dentro. O lexer decide pelo token ANTERIOR ao `{`. */
    unsigned char chave_dict[64];

    /* 1 = guarda os comentários como T_COMMENT, pro realce do editor. O
     * caminho do compilador roda com 0 e continua descartando. */
    int         marca_comentarios;
    /* 1 = SEGUE depois do erro (modo dos comandos de editor): o trecho ruim
     * vira um token T_ERRO, o erro entra na lista e a análise continua. */
    int         recupera;

    /* A PILHA DOS GRUPOS ABERTOS — só no modo de recuperação.
     *
     * Os contadores acima (`paren_depth`, `chave_depth`) bastam pra decidir
     * se a quebra de linha conta, e o modo normal continua só com eles. Mas
     * contador não sabe QUAL grupo está aberto nem ONDE ele abriu: um `)`
     * fechava um `[`, e um `(` esquecido engolia o resto do arquivo sem que
     * ninguém dissesse onde ele estava. Com a pilha, o fechador que não casa e
     * o grupo aberto no fim do arquivo viram erro no ABRIDOR. */
    struct { unsigned char tipo; int32_t linha, col; int32_t indent; } grupos[256];
    int         ngrupos;
    /* 1 = segunda passada: a linha que só pode começar declaração, ou que
     * volta à indentação da linha do abridor, fecha o grupo de expressão
     * aberto (ver `ps_lexer_tokenize_modo`) — SÓ se ele é um dos `alvos`: os
     * grupos que a PRIMEIRA passada achou sem par (posição do abridor). Um
     * grupo que fecha direito nunca é tocado pelo palpite. */
    int         sincroniza;
    struct PSFechado *alvos;
    int32_t     nalvos;

    /* Onde começou a volta anterior do laço (ver `fecha_span`). Moram aqui, e
     * não em variáveis do laço, porque no modo fluxo o laço para e volta
     * quando o parser pede mais token. */
    int32_t     span_col0, span_lin0, span_n0;

    PSTokenList *out;
} Lexer;

/* ── armazenamento dos tokens no MODO FLUXO ────────────────────────────── */
#define PS_TOK_BLOCO 4096

/* Texto dos tokens de um bloco, em pedaços: um `malloc` por pedaço, não por
 * token. Some junto com o bloco. */
typedef struct TextoPedaco {
    struct TextoPedaco *prox;
    size_t cap, usado;
    char   dados[];
} TextoPedaco;

struct PSTokBloco {
    PSToken      toks[PS_TOK_BLOCO];
    TextoPedaco *textos;
};

/* O token `i` já nascido, nas duas formas de lista. */
static PSToken *tok_de(PSTokenList *o, int32_t i)
{
    if (!o->fluxo) return &o->tokens[i];
    return &o->blocos[i / PS_TOK_BLOCO]->toks[i % PS_TOK_BLOCO];
}

static void bloco_solta(struct PSTokBloco *b)
{
    if (!b) return;
    TextoPedaco *t = b->textos;
    while (t) { TextoPedaco *prox = t->prox; free(t); t = prox; }
    free(b);
}

enum { G_PAREN = 1, G_BRACK, G_DICT, G_BLOCO };

static int esta_na_lista(const char *s, int n, const char **lista)
{
    for (int i = 0; lista[i]; i++) {
        if ((int)strlen(lista[i]) == n && strncmp(lista[i], s, (size_t)n) == 0)
            return 1;
    }
    return 0;
}

static int eh_digito(char c) { return c >= '0' && c <= '9'; }
static int eh_alpha(char c)  { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
static int eh_hex(char c)    { return eh_digito(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }

static char espia(Lexer *lx, int off)
{
    size_t p = lx->pos + (size_t)off;
    return p < lx->len ? lx->src[p] : '\0';
}

/* Avança 1 byte, mas só conta coluna em INÍCIO de caractere.
 *
 * A coluna existe pra apontar o erro no código-fonte, e o editor conta
 * caracteres, não bytes. Byte de continuação de UTF-8 é 10xxxxxx: contá-lo
 * faria a coluna derivar em toda linha com acento — foi exatamente o que o
 * teste diferencial pegou em `post("Clima: " {grau} "°")`, onde o `°` (2
 * bytes) empurrava a coluna 1 casa à frente do lexer do interpretador. */
static void avanca1(Lexer *lx)
{
    if (((unsigned char)lx->src[lx->pos] & 0xC0) != 0x80) lx->col++;
    lx->pos++;
}

/* Guarda o erro na LISTA (modo de recuperação). Sem memória, só não guarda —
 * perder a linha do segundo erro é melhor que derrubar a análise. */
static void erro_na_lista(Lexer *lx, const char *msg, int32_t l, int32_t c, int32_t l2, int32_t c2)
{
    PSTokenList *o = lx->out;
    if (o->nerros >= 64) return;             /* arquivo ruim não vira enxurrada */
    if (o->nerros >= o->cap_erros) {
        int32_t nc = o->cap_erros ? o->cap_erros * 2 : 8;
        PSAviso *nv = realloc(o->erros, sizeof(PSAviso) * (size_t)nc);
        if (!nv) return;
        o->erros = nv; o->cap_erros = nc;
    }
    snprintf(o->erros[o->nerros].msg, sizeof(o->erros[o->nerros].msg), "%s", msg);
    o->erros[o->nerros].linha = l;
    o->erros[o->nerros].col = c;
    o->erros[o->nerros].linha_fim = l2;
    o->erros[o->nerros].col_fim = c2;
    o->nerros++;
}

/* Erro no caractere da posição atual: o trecho é ele. */
static void erro(Lexer *lx, const char *msg)
{
    erro_na_lista(lx, msg, lx->linha, lx->col, lx->linha, lx->col + 1);
    if (!lx->out->ok) return;          /* preserva o primeiro erro */
    lx->out->ok = 0;
    snprintf(lx->out->erro, sizeof(lx->out->erro), "%s", msg);
    lx->out->erro_linha = lx->linha;
    lx->out->erro_col = lx->col;
}

/* Registra um AVISO na posição dada. Não mexe em `ok`: o programa compila e
 * roda; quem apresenta é quem chamou o lexer. Ver o campo `avisos` em
 * ps_lexer.h. Sem memória, o aviso é descartado em silêncio — perder um aviso
 * é melhor que derrubar a compilação por causa dele. */
static void aviso_em(Lexer *lx, const char *msg, int32_t l, int32_t c)
{
    PSTokenList *o = lx->out;
    if (o->navisos >= 64) return;            /* um arquivo ruim não vira enxurrada */
    if (o->navisos >= o->cap_avisos) {
        int32_t nc = o->cap_avisos ? o->cap_avisos * 2 : 8;
        PSAviso *nv = realloc(o->avisos, sizeof(PSAviso) * (size_t)nc);
        if (!nv) return;
        o->avisos = nv; o->cap_avisos = nc;
    }
    snprintf(o->avisos[o->navisos].msg, sizeof(o->avisos[o->navisos].msg), "%s", msg);
    o->avisos[o->navisos].linha = l;
    o->avisos[o->navisos].col = c;
    o->avisos[o->navisos].linha_fim = 0;
    o->avisos[o->navisos].col_fim = 0;
    o->navisos++;
}

/* Erro que começa em (l, c) e termina em (l2, c2). */
static void erro_em_ate(Lexer *lx, const char *msg, int32_t l, int32_t c, int32_t l2, int32_t c2)
{
    erro_na_lista(lx, msg, l, c, l2, c2);
    if (!lx->out->ok) return;
    lx->out->ok = 0;
    snprintf(lx->out->erro, sizeof(lx->out->erro), "%s", msg);
    lx->out->erro_linha = l;
    lx->out->erro_col = c;
}
/* Erro que começa em (l, c) e vai até onde o lexer está agora: a string ou o
 * comentário de bloco sem fechar, o número sem dígito. */
static void erro_em(Lexer *lx, const char *msg, int32_t l, int32_t c)
{
    erro_em_ate(lx, msg, l, c, lx->linha, lx->col);
}

static PSToken *novo_token(Lexer *lx, PSTokType t, int32_t linha, int32_t col)
{
    PSTokenList *o = lx->out;
    PSToken *tk;
    if (o->fluxo) {
        int32_t b = o->n / PS_TOK_BLOCO;
        if (b >= o->nblocos) {
            if (o->nblocos >= o->cap_blocos) {
                int32_t nc = o->cap_blocos < 16 ? 16 : o->cap_blocos * 2;
                struct PSTokBloco **nv = realloc(o->blocos, sizeof(*nv) * (size_t)nc);
                if (!nv) { erro(lx, "sem memoria"); return NULL; }
                o->blocos = nv; o->cap_blocos = nc;
            }
            struct PSTokBloco *nb = malloc(sizeof(*nb));
            if (!nb) { erro(lx, "sem memoria"); return NULL; }
            nb->textos = NULL;
            o->blocos[o->nblocos++] = nb;
        }
        tk = &o->blocos[b]->toks[o->n % PS_TOK_BLOCO];
        o->n++;
    } else {
        if (o->n + 1 > o->cap) {
            int32_t novo = o->cap < 64 ? 64 : o->cap * 2;
            PSToken *p = realloc(o->tokens, sizeof(PSToken) * (size_t)novo);
            if (!p) { erro(lx, "sem memoria"); return NULL; }
            o->tokens = p;
            o->cap = novo;
        }
        tk = &o->tokens[o->n++];
    }
    memset(tk, 0, sizeof(*tk));
    tk->type = t;
    tk->line = linha;
    tk->col = col;
    return tk;
}

/* O texto do token que ACABOU de nascer (todo chamador cria o token e guarda o
 * texto em seguida). No modo fluxo ele vai pro pedaço de texto do bloco do
 * token, e morre com o bloco.
 *
 * Sem memória pro texto, o token é DESFEITO (`o->n--`): no modo fluxo o
 * parser lê os tokens enquanto o lexer os produz, e um token de palavra-chave
 * sem texto era um `strcmp(NULL)` no parser (achado pelo `make oom`). Na
 * lista inteira ninguém parseia depois de `ok = 0`, mas a regra é uma só. */
static int guarda_texto(Lexer *lx, PSToken *tk, const char *s, int n)
{
    PSTokenList *o = lx->out;
    if (o->fluxo) {
        struct PSTokBloco *b = o->blocos[(o->n - 1) / PS_TOK_BLOCO];
        size_t precisa = (size_t)n + 1;
        TextoPedaco *t = b->textos;
        if (!t || t->usado + precisa > t->cap) {
            size_t cap = precisa > 16384 ? precisa : 16384;
            TextoPedaco *nt = malloc(sizeof(TextoPedaco) + cap);
            if (!nt) { o->n--; erro(lx, "sem memoria"); return -1; }
            nt->prox = t; nt->cap = cap; nt->usado = 0;
            b->textos = t = nt;
        }
        tk->texto = t->dados + t->usado;
        t->usado += precisa;
    } else {
        tk->texto = malloc((size_t)n + 1);
        if (!tk->texto) { o->n--; erro(lx, "sem memoria"); return -1; }
    }
    memcpy(tk->texto, s, (size_t)n);
    tk->texto[n] = '\0';
    tk->texto_len = n;
    return 0;
}

/* Guarda o trecho do comentário como T_COMMENT. Só no modo do editor: o
 * compilador nunca vê este token. */
static void marca_comentario(Lexer *lx, int32_t l0, int32_t c0, size_t p0)
{
    PSToken *tk = novo_token(lx, T_COMMENT, l0, c0);
    if (!tk) return;
    guarda_texto(lx, tk, lx->src + p0, (int)(lx->pos - p0));
}

/* buffer dinâmico para montar strings com escapes */
typedef struct { char *b; int n; int cap; } Buf;

static int buf_push(Buf *bf, char c)
{
    if (bf->n + 1 > bf->cap) {
        int novo = bf->cap < 32 ? 32 : bf->cap * 2;
        char *p = realloc(bf->b, (size_t)novo);
        if (!p) return -1;
        bf->b = p; bf->cap = novo;
    }
    bf->b[bf->n++] = c;
    return 0;
}

/* ── grupos abertos (só no modo de recuperação) ─────────────────────────── */
static const char *grupo_msg(unsigned char tipo)
{
    if (tipo == G_PAREN) return "parentese '(' aberto nao foi fechado";
    if (tipo == G_BRACK) return "colchete '[' aberto nao foi fechado";
    return "chave '{' aberta nao foi fechada";
}

/* Anota o trecho de um grupo fechado à força (ver `fechados` em ps_lexer.h).
 * Sem memória, só não anota: o parser acusaria um sintoma a mais, nada pior. */
static void grupo_anota(Lexer *lx, unsigned char tipo, int32_t l1, int32_t c1, int32_t l2, int32_t c2)
{
    PSTokenList *o = lx->out;
    o->grupos_forcados++;
    if (o->nfechados >= o->cap_fechados) {
        int32_t nc = o->cap_fechados ? o->cap_fechados * 2 : 8;
        struct PSFechado *nv = realloc(o->fechados, sizeof(*nv) * (size_t)nc);
        if (!nv) return;
        o->fechados = nv; o->cap_fechados = nc;
    }
    o->fechados[o->nfechados].l1 = l1;
    o->fechados[o->nfechados].c1 = c1;
    o->fechados[o->nfechados].l2 = l2;
    o->fechados[o->nfechados].c2 = c2;
    o->fechados[o->nfechados].tipo = tipo;
    o->nfechados++;
}

/* Indentação (espaços/tabs iniciais) da linha em que `pos` está. Só a
 * segunda passada precisa: é a régua da regra "voltou à indentação do
 * abridor = o grupo acabou". */
static int32_t lx_indent_da_linha(const Lexer *lx, size_t pos)
{
    size_t i = pos;
    while (i > 0 && lx->src[i - 1] != '\n') i--;
    int32_t n = 0;
    while (i < lx->len && (lx->src[i] == ' ' || lx->src[i] == '\t')) { n++; i++; }
    return n;
}

/* O grupo aberto em (l, c) é um dos que a primeira passada achou sem par? */
static int lx_eh_alvo(const Lexer *lx, int32_t l, int32_t c)
{
    for (int32_t k = 0; k < lx->nalvos; k++)
        if (lx->alvos[k].l1 == l && lx->alvos[k].c1 == c) return 1;
    return 0;
}

static void grupo_abre(Lexer *lx, unsigned char tipo, int32_t l, int32_t c)
{
    if (!lx->recupera) return;
    /* aninhamento acima do teto não entra na pilha: aquele trecho perde a
     * recuperação fina, mas a pilha nunca mente sobre o que está abaixo */
    if (lx->ngrupos >= (int)(sizeof(lx->grupos) / sizeof(lx->grupos[0]))) return;
    lx->grupos[lx->ngrupos].tipo = tipo;
    lx->grupos[lx->ngrupos].indent = lx->sincroniza ? lx_indent_da_linha(lx, lx->pos) : 0;
    lx->grupos[lx->ngrupos].linha = l;
    lx->grupos[lx->ngrupos].col = c;
    lx->ngrupos++;
}

/* Fecha À FORÇA os grupos do índice `ate` pra cima e marca o ponto com um
 * T_SINC em (l, c). O erro sai UMA vez, no abridor do mais de fora (`ate`):
 * os abertos dentro dele são consequência do mesmo esquecimento — `funct f( {`
 * é um `(` sem par, não dois erros. Os contadores do modo normal são
 * acertados junto, pra quebra de linha voltar a contar. */
static void grupo_forca(Lexer *lx, int ate, int32_t l, int32_t c)
{
    if (ate < 0 || ate >= lx->ngrupos) return;
    unsigned char t0 = lx->grupos[ate].tipo;
    int32_t la = lx->grupos[ate].linha, ca = lx->grupos[ate].col;
    while (lx->ngrupos > ate) {
        unsigned char t = lx->grupos[--lx->ngrupos].tipo;
        if (t == G_PAREN || t == G_BRACK) { if (lx->paren_depth > 0) lx->paren_depth--; }
        else if (lx->chave_depth > 0) lx->chave_depth--;
    }
    erro_em_ate(lx, grupo_msg(t0), la, ca, la, ca + 1);   /* o trecho é o abridor */
    grupo_anota(lx, t0, la, ca, l, c);
    novo_token(lx, T_SINC, l, c);
}

/* Um fechador chegou: casa com o grupo aberto mais de dentro do tipo dele. Os
 * que estão ACIMA foram abertos depois e nunca fechados — fecham à força antes,
 * com o erro no abridor. É regra exata, não palpite: código válido sempre
 * aninha, então isto só acontece em código quebrado. Chamado ANTES do token do
 * fechador nascer, pro T_SINC ficar antes dele. */
static void grupo_fecha(Lexer *lx, char c)
{
    if (!lx->recupera) return;
    int k = lx->ngrupos - 1;
    for (; k >= 0; k--) {
        unsigned char t = lx->grupos[k].tipo;
        if ((c == ')' && t == G_PAREN) || (c == ']' && t == G_BRACK)
                || (c == '}' && (t == G_DICT || t == G_BLOCO))) break;
    }
    if (k < 0) return;                  /* fechador sobrando: quem reclama é o parser */
    if (k < lx->ngrupos - 1) grupo_forca(lx, k + 1, lx->linha, lx->col);
    lx->ngrupos = k;                    /* o que casou; o contador dele fica com o chamador */
}

/* Fim do arquivo com grupo aberto. O `{` de BLOCO quem acusa é o parser
 * ("bloco com '{' nao foi fechado"), com a frase certa pro caso; aqui vai cada
 * trecho de `(`, `[` e `{` de dicionário sem par — um erro por trecho, no
 * abridor do mais de fora dele. */
static void grupo_fim(Lexer *lx)
{
    if (!lx->recupera || lx->ngrupos == 0) return;
    int acusou = 0;
    int k = lx->ngrupos - 1;
    while (k >= 0) {
        if (lx->grupos[k].tipo == G_BLOCO) { k--; continue; }
        int base = k;
        while (base - 1 >= 0 && lx->grupos[base - 1].tipo != G_BLOCO) base--;
        erro_em_ate(lx, grupo_msg(lx->grupos[base].tipo), lx->grupos[base].linha, lx->grupos[base].col,
                    lx->grupos[base].linha, lx->grupos[base].col + 1);
        grupo_anota(lx, lx->grupos[base].tipo, lx->grupos[base].linha, lx->grupos[base].col, lx->linha, lx->col + 1);
        acusou = 1;
        k = base - 1;
    }
    lx->ngrupos = 0;
    if (acusou) novo_token(lx, T_SINC, lx->linha, lx->col);
}

/* A linha que começa em `p` SÓ pode ser começo de declaração? É o ponto de
 * sincronia da segunda passada: dentro de `(`, `[` ou `{` de dicionário,
 * nenhuma destas formas continua uma expressão —
 *   `funct nome`, `<qualquer palavra>... funct nome` (tipo e modificadores:
 *   `int funct depois`, `public static Pessoa funct criar`), `class Nome`,
 *   `Class`/`Entity`/`model`/`enum` + nome, `@decorador`, `import x`, `from x`.
 * `funct(` sem nome é lambda e continua valendo; palavra seguida de `=` é
 * argumento nomeado (`class=1`, `from=2`) e também. */
static size_t lx_palavra(const char *s, size_t n, size_t i, size_t *fim)
{
    if (i >= n || !(eh_alpha(s[i]) || s[i] == '_')) { *fim = i; return 0; }
    size_t j = i;
    while (j < n && (eh_alpha(s[j]) || eh_digito(s[j]) || s[j] == '_')) j++;
    *fim = j;
    return j - i;
}

static int lx_so_declaracao(const Lexer *lx, size_t p)
{
    const char *s = lx->src; size_t n = lx->len;
    while (p < n && (s[p] == ' ' || s[p] == '\t')) p++;
    if (p >= n) return 0;
    if (s[p] == '@') return p + 1 < n && (eh_alpha(s[p + 1]) || s[p + 1] == '_');
    size_t fim;
    size_t tam = lx_palavra(s, n, p, &fim);
    if (!tam) return 0;
    const char *w = s + p;
#define LX_EH(lit) (tam == sizeof(lit) - 1 && memcmp(w, lit, tam) == 0)
    size_t q = fim;
    while (q < n && (s[q] == ' ' || s[q] == '\t')) q++;
    int seguido_de_nome = q > fim && q < n && (eh_alpha(s[q]) || s[q] == '_');
    if (LX_EH("class") || LX_EH("Class") || LX_EH("Entity") || LX_EH("model") || LX_EH("enum"))
        return seguido_de_nome;
    if (LX_EH("import") || LX_EH("from"))
        return seguido_de_nome || (q > fim && q < n && s[q] == '.');
#undef LX_EH
    /* `... funct nome`: até 8 palavras separadas por espaço, e a primeira
     * `funct` tem que vir seguida de NOME (sem nome é lambda) */
    for (int k = 0; k < 8; k++) {
        if (fim - p == 5 && memcmp(s + p, "funct", 5) == 0) return seguido_de_nome;
        if (q == fim || q >= n) return 0;                 /* palavra colada em pontuação */
        p = q;
        if (!lx_palavra(s, n, p, &fim)) return 0;
        q = fim;
        while (q < n && (s[q] == ' ' || s[q] == '\t')) q++;
        seguido_de_nome = q > fim && q < n && (eh_alpha(s[q]) || s[q] == '_');
    }
    return 0;
}

/* ── quebra de linha + indentação ───────────────────────────────────────── */
static void trata_newline(Lexer *lx)
{
    lx->pos++;
    lx->linha++;
    lx->col = 1;

    /* SEGUNDA PASSADA (só quando a primeira achou grupo sem par): dentro de
     * grupo de EXPRESSÃO que a primeira passada achou sem par (um dos
     * `alvos`), a linha nova fecha os grupos abertos até o bloco de fora
     * quando (a) só pode começar declaração, ou (b) volta à indentação da
     * linha do abridor (ou menos) sem começar com um fechador. Sem (a),
     * `funct f(` esquecido engolia a `int funct depois(...)` de baixo como se
     * fosse parâmetro; sem (b), engolia o `post(1)` e o `x = = 2` de baixo,
     * e o erro real da linha 4 sumia da lista como "sintoma" do `(`.
     * Continuação indentada mais fundo (`f(\n    1,\n    2)`) segue sendo
     * continuação; linha vazia ou só comentário não decide nada. Grupo que
     * fecha direito nunca está em `alvos`, e o palpite não o toca. */
    if (lx->sincroniza && lx->ngrupos > 0 && lx->grupos[lx->ngrupos - 1].tipo != G_BLOCO) {
        int k = lx->ngrupos - 1;
        while (k - 1 >= 0 && lx->grupos[k - 1].tipo != G_BLOCO) k--;
        if (lx_eh_alvo(lx, lx->grupos[k].linha, lx->grupos[k].col)) {
            size_t q = lx->pos;
            int32_t c = 1;
            while (q < lx->len && (lx->src[q] == ' ' || lx->src[q] == '\t')) { q++; c++; }
            int vazia = q >= lx->len || lx->src[q] == '\n' || lx->src[q] == '\r' || lx->src[q] == '#';
            int fechador = !vazia && (lx->src[q] == ')' || lx->src[q] == ']' || lx->src[q] == '}');
            int recuou = !vazia && !fechador && c - 1 <= lx->grupos[k].indent;
            if (recuou || lx_so_declaracao(lx, lx->pos)) grupo_forca(lx, k, lx->linha, c);
        }
    }

    /* Dentro de `(` e `[` a indentação não conta (expressão multilinha). Mas
     * `{` NÃO entra aqui: ele também abre BLOCO, e um bloco pode ter um
     * sub-bloco `:` dentro — suprimir o NEWLINE/INDENT fazia
     * `if (x) { funct f(): ... }` morrer com "faltou quebra de linha apos
     * ':'". Dentro de dicionário literal quem ignora esses tokens é o parser
     * (pula_separadores com grupo_depth > 0). */
    if (lx->paren_depth > 0) return;
    /* dentro de `{ }` de DICIONÁRIO a indentação também não conta */
    if (lx->chave_depth > 0 && lx->chave_depth <= 64
            && lx->chave_dict[lx->chave_depth - 1]) return;

    /* Continuação com `.membro` na próxima linha: não emite NEWLINE nem
     * mexe na indentação, pra `obj()\n  .json()\n  .status()` funcionar. */
    size_t p = lx->pos;
    while (p < lx->len && (lx->src[p] == ' ' || lx->src[p] == '\t')) p++;
    if (p < lx->len && lx->src[p] == '.' && p + 1 < lx->len
            && (eh_alpha(lx->src[p + 1]) || lx->src[p + 1] == '_')) {
        lx->col += (int32_t)(p - lx->pos);
        lx->pos = p;
        return;
    }

    PSToken *nl = novo_token(lx, T_NEWLINE, lx->linha - 1, lx->col);
    if (!nl) return;

    int32_t indent = 0;
    int viu_tab = 0;
    while (lx->pos < lx->len && (lx->src[lx->pos] == ' ' || lx->src[lx->pos] == '\t')) {
        if (lx->src[lx->pos] == '\t') viu_tab = 1;
        else indent++;
        lx->pos++;
        lx->col++;
    }

    /* linha vazia ou só comentário não mexe na pilha */
    if (lx->pos >= lx->len || lx->src[lx->pos] == '\n' || lx->src[lx->pos] == '\r') return;
    /* `//` NAO e mais comentario — virou divisao inteira (I11). O comentario
     * de linha e `#`, e so. */
    if (lx->src[lx->pos] == '#') return;

    /* Dentro de `{ }` a indentação é cosmética: nada de exigir múltiplo de 4
     * nem avanço exato. Fora dela a regra continua estrita. */
    int livre = lx->chave_depth > 0;
    if (viu_tab && !livre) { erro(lx, "indentacao com TAB nao e permitida; use 4 espacos"); return; }
    if (!livre && indent % INDENT_UNIT != 0) {
        char m[128];
        snprintf(m, sizeof(m), "indentacao deve ser multiplo de %d espacos (achou %d)",
                 INDENT_UNIT, indent);
        erro(lx, m);
        return;
    }

    int32_t topo = lx->indent[lx->nindent - 1];
    if (indent > topo) {
        if (!livre && indent != topo + INDENT_UNIT) {
            char m[128];
            snprintf(m, sizeof(m), "indentacao avancou %d espacos; esperado exatamente %d",
                     indent - topo, INDENT_UNIT);
            erro(lx, m);
            return;
        }
        if (lx->nindent >= (int)(sizeof(lx->indent) / sizeof(lx->indent[0]))) {
            erro(lx, "indentacao profunda demais");
            return;
        }
        lx->indent[lx->nindent++] = indent;
        PSToken *tk = novo_token(lx, T_INDENT, lx->linha, 1);
        if (tk) tk->i = indent;
    } else {
        while (indent < lx->indent[lx->nindent - 1]) {
            lx->nindent--;
            PSToken *tk = novo_token(lx, T_DEDENT, lx->linha, 1);
            if (tk) tk->i = indent;
        }
        if (!livre && indent != lx->indent[lx->nindent - 1]) {
            char m[128];
            snprintf(m, sizeof(m), "indentacao inconsistente (esperado %d, achou %d)",
                     lx->indent[lx->nindent - 1], indent);
            erro(lx, m);
        }
    }
}

/* ── comentários ────────────────────────────────────────────────────────── */
static void pula_comentario_linha(Lexer *lx)
{
    int32_t l0 = lx->linha, c0 = lx->col;
    size_t p0 = lx->pos;
    while (lx->pos < lx->len && lx->src[lx->pos] != '\n') avanca1(lx);
    if (lx->marca_comentarios) marca_comentario(lx, l0, c0, p0);
}

static void pula_comentario_bloco(Lexer *lx)
{
    int32_t l0 = lx->linha, c0 = lx->col;
    size_t p0 = lx->pos;
    lx->pos += 3; lx->col += 3;
    while (lx->pos < lx->len) {
        if (lx->pos + 2 < lx->len && strncmp(lx->src + lx->pos, "\"\"\"", 3) == 0) {
            lx->pos += 3; lx->col += 3;
            if (lx->marca_comentarios) marca_comentario(lx, l0, c0, p0);
            return;
        }
        if (lx->src[lx->pos] == '\n') { lx->pos++; lx->linha++; lx->col = 1; }
        else avanca1(lx);
    }
    erro_em(lx, "bloco de comentario \"\"\" nao foi fechado", l0, c0);
}

/* ── strings ────────────────────────────────────────────────────────────── */
static int ehexdig(char c) { return (c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F'); }
static int hexval(char c)  { if (c>='0'&&c<='9') return c-'0'; if (c>='a'&&c<='f') return c-'a'+10; return c-'A'+10; }

/* codepoint -> UTF-8 no buffer. O interp faz `chr(cp)` (str) que vira UTF-8 na
 * saída; aqui codificamos igual, então `\033`/`\x1b` dão o MESMO byte ESC e um
 * `\xff` dá os mesmos 2 bytes UTF-8 nos dois motores. */
static int buf_push_utf8(Buf *bf, unsigned long cp)
{
    if (cp < 0x80) return buf_push(bf, (char)cp);
    if (cp < 0x800) {
        if (buf_push(bf, (char)(0xC0 | (cp >> 6))) != 0) return -1;
        return buf_push(bf, (char)(0x80 | (cp & 0x3F)));
    }
    if (cp < 0x10000) {
        if (buf_push(bf, (char)(0xE0 | (cp >> 12))) != 0) return -1;
        if (buf_push(bf, (char)(0x80 | ((cp >> 6) & 0x3F))) != 0) return -1;
        return buf_push(bf, (char)(0x80 | (cp & 0x3F)));
    }
    if (buf_push(bf, (char)(0xF0 | (cp >> 18))) != 0) return -1;
    if (buf_push(bf, (char)(0x80 | ((cp >> 12) & 0x3F))) != 0) return -1;
    if (buf_push(bf, (char)(0x80 | ((cp >> 6) & 0x3F))) != 0) return -1;
    return buf_push(bf, (char)(0x80 | (cp & 0x3F)));
}

/* Processa o escape que começa no '\\' em lx->pos, empurra o resultado (UTF-8)
 * em bf e avança lx->pos/col pelos chars consumidos. Escapes reconhecidos:
 * \n \t \r \a \b \f \v \e, \\ \" \', octal \033, hex \x1b,
 * unicode \uXXXX/\UXXXXXXXX.
 *
 * DESCONHECIDO MANTÉM A BARRA E AVISA. Antes a barra sumia, calada:
 * `"C:\pasta"` virava `C:pasta` (7 bytes) em vez de `C:\pasta` (8), e
 * ninguém ficava sabendo. Perder um byte do dado do usuário em silêncio é o
 * pior dos dois mundos; agora sai o aviso e os dois caracteres ficam no
 * texto. 0 ok, -1 mem. */
static int decode_escape(Lexer *lx, Buf *bf, int bytes)
{
    const char *s = lx->src;
    size_t n = lx->len, i = lx->pos;      /* i aponta pro '\\' */
    char nxt = s[i + 1];
    int consumido = 2;
    unsigned long cp;

    /* Guarda a barra e avisa: o chamador emite `nxt` logo em seguida. */
    #define ESCAPE_DESCONHECIDO()                                             \
        do {                                                                  \
            char _m[160];                                                     \
            snprintf(_m, sizeof(_m),                                          \
                     "sequencia de escape invalida '\\%c' — a barra fica no "  \
                     "texto; use '\\\\%c' se ela e mesmo pra estar ali", nxt, nxt); \
            aviso_em(lx, _m, lx->linha, lx->col);                             \
            if (buf_push_utf8(bf, (unsigned long)'\\') != 0) return -1;        \
            cp = (unsigned char)nxt;                                          \
            consumido = 2;                                                    \
        } while (0)
    switch (nxt) {
        case 'n': cp = '\n'; break;
        case 't': cp = '\t'; break;
        case 'r': cp = '\r'; break;
        case 'a': cp = '\a'; break;
        case 'b': cp = '\b'; break;
        case 'f': cp = '\f'; break;
        case 'v': cp = '\v'; break;
        case 'e': cp = 0x1b; break;                 /* ESC — sequências ANSI */
        case '\\': cp = '\\'; break;
        case '"':  cp = '"';  break;
        case '\'': cp = '\''; break;
        default:
            if (nxt >= '0' && nxt <= '7') {          /* octal \ooo (1-3) */
                unsigned long v = 0; int d = 0; size_t j = i + 1;
                while (j < n && s[j] >= '0' && s[j] <= '7' && d < 3) {
                    v = v * 8 + (unsigned long)(s[j] - '0'); j++; d++;
                }
                if (bytes && v > 255) { erro(lx, "octal fora de 0-255 em bytes"); return -2; }
                cp = v; consumido = (int)(j - i);
            } else if (nxt == 'x' || nxt == 'X') {   /* hex \xHH */
                if (i + 3 < n && ehexdig(s[i+2]) && ehexdig(s[i+3])) {
                    cp = (unsigned long)(hexval(s[i+2]) * 16 + hexval(s[i+3])); consumido = 4;
                } else { ESCAPE_DESCONHECIDO(); }
            } else if (nxt == 'u' || nxt == 'U') {   /* unicode \uXXXX / \UXXXXXXXX */
                /* em bytes não existe codepoint: é byte a byte, com \xHH */
                if (bytes) { erro(lx, "\\u nao vale em bytes: use \\xHH"); return -2; }
                int k = (nxt == 'u') ? 4 : 8, ok = 1; unsigned long v = 0;
                for (int t = 0; t < k; t++) {
                    if (i + 2 + (size_t)t >= n || !ehexdig(s[i+2+t])) { ok = 0; break; }
                    v = v * 16 + (unsigned long)hexval(s[i+2+t]);
                }
                if (ok) { cp = v; consumido = 2 + k; }
                else    { ESCAPE_DESCONHECIDO(); }
            } else {
                ESCAPE_DESCONHECIDO();
            }
    }
    lx->pos += (size_t)consumido; lx->col += consumido;
    /* bytes: o valor É o byte (`\xff` = 0xFF); str: vira UTF-8 */
    if (bytes) return buf_push(bf, (char)(unsigned char)cp);
    return buf_push_utf8(bf, cp);
}
#undef ESCAPE_DESCONHECIDO

/* `bytes`: literal `b"..."` — o token é T_BYTES e cada escape é UM byte;
 * caractere não-ASCII no fonte entra com os bytes UTF-8 dele.
 *
 * `c_tok` é a coluna onde o TOKEN começa no fonte: a da aspa numa string
 * pelada, a do prefixo em `f"…"`, `b"…"`, `br"…"`. O span (`nchars`) já era
 * medido a partir do prefixo; a coluna começava na aspa, e o token saía
 * deslocado um ou dois caracteres pra direita — o editor achava que o `.`
 * de `b"q".` estava DENTRO da string e não completava nada. */
/* Guarda um `{...}` de f-string: o pedaço no fonte e onde ele começa. O token
 * ainda não nasceu, então o índice dele é o próximo (`out->n`). */
static void interp_poe(Lexer *lx, size_t off, int32_t len, int32_t linha, int32_t col)
{
    PSTokenList *o = lx->out;
    /* f-string absurda não vira enxurrada. Conta o arquivo inteiro, e não o
     * que está na lista: no modo fluxo os de trás já saíram. */
    if (o->interps_total >= 512) return;
    if (o->ninterps >= o->cap_interps) {
        int32_t nc = o->cap_interps ? o->cap_interps * 2 : 8;
        struct PSInterp *nv = realloc(o->interps, sizeof(*nv) * (size_t)nc);
        if (!nv) return;
        o->interps = nv; o->cap_interps = nc;
    }
    o->interps[o->ninterps].tok = o->n;        /* o token da f-string nasce a seguir */
    o->interps[o->ninterps].off = off;
    o->interps[o->ninterps].len = len;
    o->interps[o->ninterps].linha = linha;
    o->interps[o->ninterps].col = col;
    /* o trecho CRU: é dele que saem os tokens e a árvore de dentro da
     * interpolação, com a posição de verdade */
    char *txt = malloc((size_t)len + 1);
    if (txt) { memcpy(txt, lx->src + off, (size_t)len); txt[len] = '\0'; }
    o->interps[o->ninterps].txt = txt;
    o->ninterps++;
    o->interps_total++;
}

static void le_string(Lexer *lx, char aspa, int fstring, int raw, int bytes, int32_t c_tok)
{
    int32_t l0 = lx->linha, c0 = c_tok;
    lx->pos++; lx->col++;
    Buf bf = {0};
    /* posição da interpolação ABERTA (0 = nenhuma) e profundidade de chaves */
    size_t i_off = 0; int32_t i_lin = 0, i_col = 0, i_prof = 0;

    while (lx->pos < lx->len) {
        char c = lx->src[lx->pos];
        if (fstring) {
            /* `{{` e `}}` são chave literal, não interpolação — a mesma regra
             * que o compilador aplica ao gerar o código da f-string. */
            if (c == '{' && !i_prof && lx->pos + 1 < lx->len && lx->src[lx->pos + 1] == '{') {
                buf_push(&bf, '{'); buf_push(&bf, '{');
                avanca1(lx); avanca1(lx);
                continue;
            }
            if (c == '}' && !i_prof && lx->pos + 1 < lx->len && lx->src[lx->pos + 1] == '}') {
                buf_push(&bf, '}'); buf_push(&bf, '}');
                avanca1(lx); avanca1(lx);
                continue;
            }
            if (c == '{') {
                if (!i_prof) { i_off = lx->pos + 1; i_lin = lx->linha; i_col = lx->col + 1; }
                i_prof++;
            } else if (c == '}' && i_prof) {
                i_prof--;
                if (!i_prof && i_off) {
                    interp_poe(lx, i_off, (int32_t)(lx->pos - i_off), i_lin, i_col);
                    i_off = 0;
                }
            }
        }
        if (!raw && c == '\\' && lx->pos + 1 < lx->len) {
            int rc = decode_escape(lx, &bf, bytes);
            if (rc == -2) { free(bf.b); return; }            /* erro ja registrado */
            if (rc != 0) { free(bf.b); erro(lx, "sem memoria"); return; }
            continue;
        }
        if (c == aspa) {
            /* `{` aberto e nunca fechado (`f"a {b"`): o trecho vale até aqui.
             * Quem digita passa a maior parte do tempo neste estado, e sem o
             * registro o editor tratava o que está sendo escrito como texto. */
            if (fstring && i_off) interp_poe(lx, i_off, (int32_t)(lx->pos - i_off), i_lin, i_col);
            lx->pos++; lx->col++;
            PSToken *tk = novo_token(lx, bytes ? T_BYTES : fstring ? T_FSTRING : T_STR, l0, c0);
            if (tk) guarda_texto(lx, tk, bf.b ? bf.b : "", bf.n);
            free(bf.b);
            return;
        }
        if (c == '\n') {
            erro(lx, "string nao fechada antes da quebra de linha");
            /* RECUPERAÇÃO: a string vale até o fim da linha e vira token. Sem
             * isto o realce perdia a linha inteira, e é assim que uma string
             * fica na maior parte do tempo em que se digita. */
            if (lx->recupera) {
                if (fstring && i_off) interp_poe(lx, i_off, (int32_t)(lx->pos - i_off), i_lin, i_col);
                PSToken *tk = novo_token(lx, bytes ? T_BYTES : fstring ? T_FSTRING : T_STR, l0, c0);
                if (tk) guarda_texto(lx, tk, bf.b ? bf.b : "", bf.n);
            }
            free(bf.b);
            return;
        }
        if (buf_push(&bf, c) != 0) { free(bf.b); erro(lx, "sem memoria"); return; }
        avanca1(lx);
    }
    erro_em(lx, "string nao fechada ate o fim do arquivo", l0, c0);
    if (lx->recupera) {
        if (fstring && i_off) interp_poe(lx, i_off, (int32_t)(lx->pos - i_off), i_lin, i_col);
        PSToken *tk = novo_token(lx, bytes ? T_BYTES : fstring ? T_FSTRING : T_STR, l0, c0);
        if (tk) guarda_texto(lx, tk, bf.b ? bf.b : "", bf.n);
    }
    free(bf.b);
}

static void le_string_tripla(Lexer *lx, char aspa, int fstring, int raw, int bytes, int32_t c_tok)
{
    int32_t l0 = lx->linha, c0 = c_tok;
    char tres[4] = { aspa, aspa, aspa, '\0' };
    lx->pos += 3; lx->col += 3;
    Buf bf = {0};
    /* a mesma contagem de `le_string`: a f-string tripla também interpola */
    size_t i_off = 0; int32_t i_lin = 0, i_col = 0, i_prof = 0;

    while (lx->pos < lx->len) {
        if (lx->pos + 2 < lx->len && strncmp(lx->src + lx->pos, tres, 3) == 0) {
            if (fstring && i_off) interp_poe(lx, i_off, (int32_t)(lx->pos - i_off), i_lin, i_col);
            lx->pos += 3; lx->col += 3;
            PSToken *tk = novo_token(lx, bytes ? T_BYTES : fstring ? T_FSTRING : T_STR, l0, c0);
            if (tk) guarda_texto(lx, tk, bf.b ? bf.b : "", bf.n);
            free(bf.b);
            return;
        }
        char c = lx->src[lx->pos];
        if (fstring) {
            if (c == '{' && !i_prof && lx->pos + 1 < lx->len && lx->src[lx->pos + 1] == '{') {
                buf_push(&bf, '{'); buf_push(&bf, '{');
                avanca1(lx); avanca1(lx);
                continue;
            }
            if (c == '}' && !i_prof && lx->pos + 1 < lx->len && lx->src[lx->pos + 1] == '}') {
                buf_push(&bf, '}'); buf_push(&bf, '}');
                avanca1(lx); avanca1(lx);
                continue;
            }
            if (c == '{') {
                if (!i_prof) { i_off = lx->pos + 1; i_lin = lx->linha; i_col = lx->col + 1; }
                i_prof++;
            } else if (c == '}' && i_prof) {
                i_prof--;
                if (!i_prof && i_off) {
                    interp_poe(lx, i_off, (int32_t)(lx->pos - i_off), i_lin, i_col);
                    i_off = 0;
                }
            }
        }
        if (!raw && c == '\\' && lx->pos + 1 < lx->len) {
            int rc = decode_escape(lx, &bf, bytes);
            if (rc == -2) { free(bf.b); return; }
            if (rc != 0) { free(bf.b); erro(lx, "sem memoria"); return; }
            continue;
        }
        if (c == '\n') {
            if (buf_push(&bf, c) != 0) { free(bf.b); erro(lx, "sem memoria"); return; }
            lx->pos++; lx->linha++; lx->col = 1;
            continue;
        }
        if (buf_push(&bf, c) != 0) { free(bf.b); erro(lx, "sem memoria"); return; }
        avanca1(lx);
    }
    free(bf.b);
    erro_em(lx, "string multi-linha nao foi fechada", l0, c0);
}

/* ── números ────────────────────────────────────────────────────────────── */
/* Avança enquanto for dígito da base pedida (ou `_`, que é só separador
 * visual). Devolve quantos dígitos DE VERDADE consumiu. */
static int corre_digitos(Lexer *lx, int base)
{
    int d = 0;
    while (lx->pos < lx->len) {
        char c = lx->src[lx->pos];
        int vale;
        if (c == '_') vale = -1;
        else if (base == 16) vale = ehexdig(c);
        else if (base == 8)  vale = (c >= '0' && c <= '7');
        else if (base == 2)  vale = (c == '0' || c == '1');
        else                 vale = eh_digito(c);
        if (!vale) break;
        if (vale > 0) d++;
        lx->pos++; lx->col++;
    }
    return d;
}

static void le_numero(Lexer *lx)
{
    int32_t c0 = lx->col;
    size_t ini = lx->pos;

    /* Bases: 0x1F, 0o17, 0b1010. Antes só existia decimal — `0x1F` lexava
     * como `0` seguido do identificador `x1F` e explodia em "variável não
     * definida". Sempre inteiro, nunca float. */
    if (lx->src[lx->pos] == '0' && lx->pos + 1 < lx->len) {
        char m = lx->src[lx->pos + 1];
        int base = (m == 'x' || m == 'X') ? 16
                 : (m == 'o' || m == 'O') ? 8
                 : (m == 'b' || m == 'B') ? 2 : 0;
        if (base) {
            lx->pos += 2; lx->col += 2;
            size_t d0 = lx->pos;
            if (corre_digitos(lx, base) == 0) {
                erro_em(lx, "numero sem digito depois da base", lx->linha, c0);
                return;
            }
            char tmp[80];
            int n = (int)(lx->pos - d0), j = 0;
            for (int i = 0; i < n && j < (int)sizeof(tmp) - 1; i++)
                if (lx->src[d0 + (size_t)i] != '_') tmp[j++] = lx->src[d0 + (size_t)i];
            tmp[j] = '\0';
            PSToken *tk = novo_token(lx, T_INT, lx->linha, c0);
            if (!tk) return;
            tk->i = (int64_t)strtoll(tmp, NULL, base);
            /* Texto NORMALIZADO: prefixo minúsculo + dígitos sem `_`, o mesmo
             * formato do ramo decimal abaixo. O parser relê o texto pra
             * detectar estouro e virar bignum, e a VM o entrega ao mpz — os
             * dois precisam saber a base pelo prefixo. Guardar o trecho cru
             * (com `_`, `0X`) fazia o parser reler em base 10, e `0xFFFF…`
             * grande saturava em INT64_MAX calado em vez de virar bignum. */
            char norm[84];
            norm[0] = '0';
            norm[1] = (base == 16) ? 'x' : (base == 8) ? 'o' : 'b';
            memcpy(norm + 2, tmp, (size_t)j + 1);
            guarda_texto(lx, tk, norm, j + 2);
            return;
        }
    }

    corre_digitos(lx, 10);
    int flutuante = 0;
    if (lx->pos < lx->len && lx->src[lx->pos] == '.'
            && lx->pos + 1 < lx->len && eh_digito(lx->src[lx->pos + 1])) {
        flutuante = 1;
        lx->pos++; lx->col++;
        corre_digitos(lx, 10);
    }
    /* Expoente `1e30`, `2.5E-3`. Só consome o `e` se vier dígito depois
     * (com sinal opcional), senão `1e` seria número seguido de nada. */
    if (lx->pos < lx->len && (lx->src[lx->pos] == 'e' || lx->src[lx->pos] == 'E')) {
        size_t j = lx->pos + 1;
        if (j < lx->len && (lx->src[j] == '+' || lx->src[j] == '-')) j++;
        if (j < lx->len && eh_digito(lx->src[j])) {
            flutuante = 1;
            lx->col += (int32_t)(j - lx->pos);
            lx->pos = j;
            corre_digitos(lx, 10);
        }
    }
    int n = (int)(lx->pos - ini);
    /* Copia sem os `_`: o strtod/strtoll não conhece separador. */
    char tmp[80];
    int j = 0;
    for (int i = 0; i < n && j < (int)sizeof(tmp) - 1; i++)
        if (lx->src[ini + (size_t)i] != '_') tmp[j++] = lx->src[ini + (size_t)i];
    tmp[j] = '\0';

    PSToken *tk = novo_token(lx, flutuante ? T_FLO : T_INT, lx->linha, c0);
    if (!tk) return;
    if (flutuante) tk->d = strtod(tmp, NULL);
    else           tk->i = (int64_t)strtoll(tmp, NULL, 10);
    /* O texto guardado é o do FONTE (com `_`); o parser relê com strtoll pra
     * detectar estouro e virar bignum, então tem que ser sem separador. */
    guarda_texto(lx, tk, tmp, j);
}

/* ── identificadores / keywords ─────────────────────────────────────────── */
static void le_ident(Lexer *lx)
{
    int32_t c0 = lx->col;
    size_t ini = lx->pos;
    while (lx->pos < lx->len
           && (eh_alpha(lx->src[lx->pos]) || eh_digito(lx->src[lx->pos]) || lx->src[lx->pos] == '_')) {
        lx->pos++; lx->col++;
    }
    int n = (int)(lx->pos - ini);
    const char *txt = lx->src + ini;

    /* prefixos de string: f"..." f'''...''' r"..." r'''...''' — e de BYTES:
     * b"..." B"..." br"..." rb"..." (cru). Só quando a aspa vem IMEDIATAMENTE
     * depois: `b = 1`, `b(x)` e `br = 2` continuam nomes. */
    {
        int fstring = 0, raw = 0, bytes = 0, prefixo = 0;
        if (n == 1 && (txt[0] == 'f' || txt[0] == 'r')) { fstring = (txt[0] == 'f'); raw = !fstring; prefixo = 1; }
        else if (n == 1 && (txt[0] == 'b' || txt[0] == 'B')) { bytes = 1; prefixo = 1; }
        else if (n == 2 && ((txt[0] == 'b' && txt[1] == 'r') || (txt[0] == 'r' && txt[1] == 'b'))) { bytes = 1; raw = 1; prefixo = 1; }
        if (prefixo) {
            char prox = lx->pos < lx->len ? lx->src[lx->pos] : '\0';
            /* Triplo com QUALQUER aspa: `"""` pelado é comentário de bloco,
             * mas com prefixo (`b"""…"""`, `f"""…"""`) só pode ser literal —
             * antes `b"""ab"""` virava `b""` + `"ab"` + `""`, três tokens. */
            if ((prox == '"' || prox == '\'') && espia(lx, 1) == prox && espia(lx, 2) == prox) {
                le_string_tripla(lx, prox, fstring, raw, bytes, c0);
                return;
            }
            /* f"..." e f'...' valem igual — aspas são equivalentes na linguagem */
            if (prox == '"' || prox == '\'') {
                le_string(lx, prox, fstring, raw, bytes, c0);
                return;
            }
        }
    }

    PSTokType tipo;
    int64_t valor_bool = 0;

    if ((n == 4 && strncmp(txt, "True", 4) == 0) || (n == 4 && strncmp(txt, "true", 4) == 0)) {
        tipo = T_BOOL; valor_bool = 1;
    } else if ((n == 5 && strncmp(txt, "False", 5) == 0) || (n == 5 && strncmp(txt, "false", 5) == 0)) {
        tipo = T_BOOL; valor_bool = 0;
    } else if ((n == 4 && strncmp(txt, "Null", 4) == 0) || (n == 4 && strncmp(txt, "null", 4) == 0)
            || (n == 4 && strncmp(txt, "None", 4) == 0) || (n == 4 && strncmp(txt, "none", 4) == 0)) {
        tipo = T_NULL;
    } else if (esta_na_lista(txt, n, KEYWORDS)) {
        tipo = T_KW;
    } else {
        tipo = (txt[0] >= 'A' && txt[0] <= 'Z') ? T_IDENT_UPPER : T_IDENT;
    }

    PSToken *tk = novo_token(lx, tipo, lx->linha, c0);
    if (!tk) return;
    if (tipo == T_BOOL) tk->i = valor_bool;
    guarda_texto(lx, tk, txt, n);
}

/* ── cor: <hex> ou <nome> ───────────────────────────────────────────────── */
static int le_cor(Lexer *lx)
{
    /* <([A-Za-z0-9]{1,7})> */
    size_t p = lx->pos + 1;
    size_t ini = p;
    while (p < lx->len && (eh_alpha(lx->src[p]) || eh_digito(lx->src[p]))) p++;
    int n = (int)(p - ini);
    if (n < 1 || n > 7) return 0;
    if (p >= lx->len || lx->src[p] != '>') return 0;

    const char *val = lx->src + ini;
    int hex_ok = (n == 3 || n == 6);
    if (hex_ok) {
        for (int i = 0; i < n; i++) if (!eh_hex(val[i])) { hex_ok = 0; break; }
    }
    if (!hex_ok && !esta_na_lista(val, n, CORES)) return 0;

    int32_t c0 = lx->col;
    int total = (int)(p + 1 - lx->pos);
    PSToken *tk = novo_token(lx, T_COLOR, lx->linha, c0);
    if (!tk) return 1;
    guarda_texto(lx, tk, val, n);
    lx->pos += (size_t)total;
    lx->col += total;
    return 1;
}

/* ── pontuação e operadores ─────────────────────────────────────────────── */
static int le_punct(Lexer *lx)
{
    char c = lx->src[lx->pos];
    PSTokType t;
    switch (c) {
        case '(': t = T_LPAREN; break;
        case ')': t = T_RPAREN; break;
        case '{': t = T_LBRACE; break;
        case '}': t = T_RBRACE; break;
        case '[': t = T_LBRACK; break;
        case ']': t = T_RBRACK; break;
        case ':': t = T_COLON;  break;
        case ';': t = T_SEMI;   break;
        case ',': t = T_COMMA;  break;
        case '.': t = T_DOT;    break;
        case '@': t = T_AT;     break;
        default: return 0;
    }
    int32_t l0 = lx->linha, c0 = lx->col;
    if (c == ')' || c == ']' || c == '}') grupo_fecha(lx, c);
    PSToken *tk = novo_token(lx, t, l0, c0);
    if (tk) guarda_texto(lx, tk, &c, 1);

    if (c == '(') grupo_abre(lx, G_PAREN, l0, c0);
    else if (c == '[') grupo_abre(lx, G_BRACK, l0, c0);

    if (c == '(' || c == '[') lx->paren_depth++;
    else if (c == ')' || c == ']') {
        if (lx->paren_depth > 0) lx->paren_depth--;
    }
    else if (c == '{') {
        /* Dicionário quando o `{` vem DEPOIS de algo que espera um VALOR:
         * operador, `(`, `[`, `,`, `:`, ou as palavras `return`/`yield`/
         * `case` (o `case {a: 1}` casa um dict). Depois de `)`, de um nome ou
         * de um literal, é BLOCO — `if (x) {`, `for each i in l {`,
         * `match x {`, `case 1 {`. */
        int dict = 1;                       /* início de arquivo abre valor */
        PSTokenList *o = lx->out;
        /* `n - 2`: o token do PRÓPRIO `{` já foi criado logo acima, então o
         * anterior é o penúltimo. Ler `n - 1` classificava o `{` por ele
         * mesmo e todo bloco virava dicionário. */
        if (o->n > 1) {
            PSToken *a = tok_de(o, o->n - 2);
            if (a->type == T_OP || a->type == T_COMMA || a->type == T_COLON
                    || a->type == T_LPAREN || a->type == T_LBRACK
                    || a->type == T_LBRACE) dict = 1;
            else if (a->type == T_KW && a->texto
                     && (!strcmp(a->texto, "return") || !strcmp(a->texto, "yield")
                      || !strcmp(a->texto, "case"))) dict = 1;
            else dict = 0;
        }
        if (lx->chave_depth < 64) lx->chave_dict[lx->chave_depth] = (unsigned char)dict;
        lx->chave_depth++;
        grupo_abre(lx, dict ? G_DICT : G_BLOCO, l0, c0);
    }
    else if (c == '}') { if (lx->chave_depth > 0) lx->chave_depth--; }
    lx->pos++; lx->col++;
    return 1;
}

static int le_operador(Lexer *lx)
{
    for (int i = 0; MULTI_OPS[i]; i++) {
        size_t n = strlen(MULTI_OPS[i]);
        if (lx->pos + n <= lx->len && strncmp(lx->src + lx->pos, MULTI_OPS[i], n) == 0) {
            PSToken *tk = novo_token(lx, T_OP, lx->linha, lx->col);
            if (tk) guarda_texto(lx, tk, MULTI_OPS[i], (int)n);
            lx->pos += n; lx->col += (int32_t)n;
            return 1;
        }
    }
    char c = lx->src[lx->pos];
    if (c && strchr(SINGLE_OPS, c)) {
        PSToken *tk = novo_token(lx, T_OP, lx->linha, lx->col);
        if (tk) guarda_texto(lx, tk, &c, 1);
        lx->pos++; lx->col++;
        return 1;
    }
    return 0;
}

/* ── laço principal ─────────────────────────────────────────────────────── */
static PSTokenList *tokeniza(const char *fonte, size_t len, int com_comentarios, int recupera,
                             const struct PSFechado *alvos, int32_t nalvos);

PSTokenList *ps_lexer_tokenize(const char *fonte, size_t len)
{ return tokeniza(fonte, len, 0, 0, NULL, 0); }

PSTokenList *ps_lexer_tokenize_editor(const char *fonte, size_t len)
{ return tokeniza(fonte, len, 1, 0, NULL, 0); }

/* Duas passadas no modo de recuperação. A primeira só usa regras EXATAS
 * (fechador que não casa, grupo aberto no fim). Se ela achou grupo sem par, a
 * segunda repete com a sincronia por declaração ligada — que é palpite, e por
 * isso só roda em arquivo que já está quebrado: código válido não tem grupo sem
 * par, nunca chega aqui, e o `--check` não pode recusá-lo por causa disto. */
PSTokenList *ps_lexer_tokenize_modo(const char *fonte, size_t len, int comentarios, int recupera)
{
    PSTokenList *l = tokeniza(fonte, len, comentarios, recupera, NULL, 0);
    if (recupera && l && l->grupos_forcados > 0) {
        PSTokenList *l2 = tokeniza(fonte, len, comentarios, recupera, l->fechados, l->nfechados);
        if (l2) { ps_lexer_free(l); return l2; }
    }
    return l;
}

/* Span do token no FONTE, medido num lugar só. Todo ramo do laço termina a
 * volta, então a medição acontece no TOPO da volta seguinte (e uma última
 * vez depois do laço): se desde a volta anterior nasceu UM token na MESMA
 * linha, o tamanho é a diferença de coluna. É o que o realce do editor
 * precisa e o `texto` não dá — string decodificada não tem aspas.
 *
 * O FIM sai daqui também, e sem a condição de "mesma linha": é o que faltava
 * pro editor sublinhar e dobrar string de várias linhas e comentário de
 * bloco. `nchars` continua só pro token de uma linha, que é onde ele faz
 * sentido. O T_SINC não ocupa texto e não conta: o fechador que nasce junto
 * com ele (`)` depois de um `(` fechado à força) continua tendo o span
 * medido. */
static void fecha_span(Lexer *lx)
{
    PSTokenList *out = lx->out;
    int32_t alvo = -1, quantos = 0;
    for (int32_t k = lx->span_n0; k < out->n; k++)
        if (tok_de(out, k)->type != T_SINC) { alvo = k; quantos++; }
    if (quantos == 1) {
        PSToken *t = tok_de(out, alvo);
        if (lx->linha == lx->span_lin0 && lx->col > lx->span_col0)
            t->nchars = lx->col - lx->span_col0;
        t->linha_fim = lx->linha;
        t->col_fim = lx->col;
    }
    lx->span_col0 = lx->col; lx->span_lin0 = lx->linha; lx->span_n0 = out->n;
}

static void lx_inicia(Lexer *lx, PSTokenList *out, const char *fonte, size_t len,
                      int com_comentarios, int recupera,
                      const struct PSFechado *alvos, int32_t nalvos)
{
    memset(lx, 0, sizeof(*lx));
    lx->src = fonte;
    lx->len = len;
    lx->linha = 1;
    lx->col = 1;
    lx->indent[0] = 0;
    lx->nindent = 1;
    lx->out = out;
    lx->marca_comentarios = com_comentarios;
    lx->recupera = recupera;
    /* a segunda passada existe pelos alvos: sem alvo, não há o que sincronizar.
     * Cópia própria: a lista da primeira passada pode ser liberada antes de o
     * lexer do fluxo terminar. */
    if (recupera && alvos && nalvos > 0) {
        lx->alvos = malloc(sizeof(*lx->alvos) * (size_t)nalvos);
        if (lx->alvos) { memcpy(lx->alvos, alvos, sizeof(*lx->alvos) * (size_t)nalvos); lx->nalvos = nalvos; }
    }
    lx->sincroniza = lx->nalvos > 0;

    /* espaços iniciais da primeira linha não geram INDENT */
    while (lx->pos < lx->len && (lx->src[lx->pos] == ' ' || lx->src[lx->pos] == '\t')) {
        lx->pos++; lx->col++;
    }
    lx->span_col0 = lx->col; lx->span_lin0 = lx->linha; lx->span_n0 = out->n;
}

/* Uma volta do laço principal. 0 = o laço acabou (fim do fonte, ou erro fora
 * do modo de recuperação) e falta só `lx_fim`. */
static int lx_passo(Lexer *lx)
{
    if (!(lx->pos < lx->len && (lx->out->ok || lx->recupera))) return 0;
    fecha_span(lx);
    char c = lx->src[lx->pos];

    if (c == '\n') { trata_newline(lx); return 1; }
    if (c == '\r') { lx->pos++; return 1; }
    if (c == ' ' || c == '\t') { lx->pos++; lx->col++; return 1; }

    /* `//` era COMENTARIO DE LINHA e agora e o operador de divisao
     * inteira (I11). Nao da pra ter os dois: `a // b` teria que ser
     * divisao num contexto e comentario no outro, e nenhuma regra de
     * desambiguacao sobrevive a `x = a //b` contra `x = a  // b`.
     *
     * O comentario de linha continua sendo `#`, que a linguagem sempre
     * aceitou e que a maior parte do repositorio ja usava. */
    if (c == '#') { pula_comentario_linha(lx); return 1; }

    if (c == '"' && espia(lx, 1) == '"' && espia(lx, 2) == '"') {
        pula_comentario_bloco(lx); return 1;
    }
    if (c == '\'' && espia(lx, 1) == '\'' && espia(lx, 2) == '\'') {
        le_string_tripla(lx, '\'', 0, 0, 0, lx->col); return 1;
    }
    if (c == '"' || c == '\'') { le_string(lx, c, 0, 0, 0, lx->col); return 1; }

    if (eh_digito(c)) { le_numero(lx); return 1; }
    if (eh_alpha(c) || c == '_') { le_ident(lx); return 1; }

    if (le_punct(lx)) return 1;
    if (c == '<' && le_cor(lx)) return 1;
    if (le_operador(lx)) return 1;

    /* O caractere sai INTEIRO, não o primeiro byte dele.
     *
     * Com `%c` a mensagem levava meio caractere: `ç` é 0xC3 0xA7, e
     * imprimir só o 0xC3 produz UTF-8 INVÁLIDO dentro da mensagem de
     * erro. Isso não é cosmético — o LSP serializa a mensagem em JSON,
     * e byte inválido quebra o JSON: o editor recusava a resposta
     * ("Expected ',' or '}' ... in JSON") e derrubava o servidor. Um
     * acento fora do lugar matava o suporte a editor inteiro.
     *
     * Um byte de continuação (10xxxxxx) sozinho não forma caractere;
     * nesse caso mostra o valor numérico, que é a informação útil. */
    char m[80];
    unsigned char b0 = (unsigned char)c;
    int nb = b0 < 0x80 ? 1 : (b0 & 0xE0) == 0xC0 ? 2
           : (b0 & 0xF0) == 0xE0 ? 3 : (b0 & 0xF8) == 0xF0 ? 4 : 0;
    char ch[5] = "";
    if (nb > 0 && lx->pos + (size_t)nb <= lx->len) {
        memcpy(ch, lx->src + lx->pos, (size_t)nb);
        ch[nb] = '\0';
        /* Letra fora do ASCII COLADA num nome (`ação`, `π`): a frase
         * de antes ("caractere inesperado") dizia o sintoma. A regra é
         * que nome só aceita letra sem acento — é isso que a pessoa
         * precisa saber pra consertar. */
        if (b0 >= 0x80 && (lx->pos > 0 && (eh_alpha(lx->src[lx->pos - 1])
                                           || eh_digito(lx->src[lx->pos - 1])
                                           || lx->src[lx->pos - 1] == '_')))
            snprintf(m, sizeof(m), "nome so aceita letra sem acento, digito e _: '%s'", ch);
        else if (b0 >= 0x80 && (eh_alpha(espia(lx, nb)) || espia(lx, nb) == '_'))
            snprintf(m, sizeof(m), "nome so aceita letra sem acento, digito e _: '%s'", ch);
        else
            snprintf(m, sizeof(m), "caractere inesperado: '%s'", ch);
    } else {
        snprintf(m, sizeof(m), "byte inesperado: 0x%02X", b0);
    }
    erro(lx, m);
    /* RECUPERAÇÃO: o trecho vira um token de erro e a análise segue.
     * Sem isto, um caractere estranho no meio do arquivo apagava os
     * tokens de TODAS as linhas — a tela inteira perdia a cor. */
    if (lx->recupera) {
        int32_t l0 = lx->linha, c0 = lx->col;
        int avanca = nb > 0 ? nb : 1;
        for (int k = 0; k < avanca && lx->pos < lx->len; k++) avanca1(lx);
        PSToken *tk = novo_token(lx, T_ERRO, l0, c0);
        if (tk) {
            const char *txt = ch[0] ? ch : "?";
            guarda_texto(lx, tk, txt, (int)strlen(txt));
            tk->nchars = 1;
        }
    }
    return 1;
}

/* Depois da última volta: o span do último token e o que o fim do arquivo
 * fecha. O EOF sai sempre no modo de recuperação: o parser precisa dele pra
 * parar, e uma lista sem EOF faria o `atual()` dele ler o último token para
 * sempre. No modo FLUXO ele sai também quando o lexer parou num erro: quem
 * parseia está no meio do arquivo e precisa parar (o `ok == 0` continua
 * dizendo que o arquivo não vale). */
static void lx_fim(Lexer *lx)
{
    PSTokenList *out = lx->out;
    fecha_span(lx);
    if (out->ok || lx->recupera) {
        grupo_fim(lx);
        while (lx->nindent > 1) {
            lx->nindent--;
            PSToken *tk = novo_token(lx, T_DEDENT, lx->linha, lx->col);
            if (tk) tk->i = 0;
        }
        novo_token(lx, T_EOF, lx->linha, lx->col);
    } else if (out->fluxo) {
        novo_token(lx, T_EOF, lx->linha, lx->col);
    }
    free(lx->alvos);
    lx->alvos = NULL; lx->nalvos = 0;
}

static PSTokenList *tokeniza(const char *fonte, size_t len, int com_comentarios, int recupera,
                             const struct PSFechado *alvos, int32_t nalvos)
{
    PSTokenList *out = calloc(1, sizeof(PSTokenList));
    if (!out) return NULL;
    out->ok = 1;

    Lexer lx;
    lx_inicia(&lx, out, fonte, len, com_comentarios, recupera, alvos, nalvos);
    while (lx_passo(&lx)) {}
    lx_fim(&lx);
    return out;
}

PSTokenList *ps_lexer_fluxo(const char *fonte, size_t len, int recupera,
                            const struct PSFechado *alvos, int32_t nalvos)
{
    PSTokenList *out = calloc(1, sizeof(PSTokenList));
    if (!out) return NULL;
    out->ok = 1;
    out->fluxo = 1;
    Lexer *lx = malloc(sizeof(Lexer));
    if (!lx) { free(out); return NULL; }
    lx_inicia(lx, out, fonte, len, 0, recupera, alvos, nalvos);
    out->lexer = lx;
    return out;
}

const char *ps_lexer_grupo_msg(unsigned char tipo) { return grupo_msg(tipo); }

static void fluxo_avanca(PSTokenList *o);

/* Lê o fonte inteiro no modo de recuperação soltando os blocos de tokens
 * conforme nascem: o que sobra é `fechados` (e `ok`/`erros`). Segunda passada
 * com os alvos da primeira, como o parser faz. */
static PSTokenList *sonda_passada(const char *fonte, size_t len,
                                  const struct PSFechado *alvos, int32_t nalvos)
{
    PSTokenList *o = ps_lexer_fluxo(fonte, len, 1, alvos, nalvos);
    if (!o) return NULL;
    while (o->lexer) {
        fluxo_avanca(o);
        if (o->n >= (o->soltos + 2) * PS_TOK_BLOCO) ps_lexer_solta_ate(o, o->n - PS_TOK_BLOCO);
    }
    ps_lexer_solta_ate(o, o->n > 0 ? o->n - 1 : 0);
    return o;
}

PSTokenList *ps_lexer_sonda_grupos(const char *fonte, size_t len)
{
    PSTokenList *l = sonda_passada(fonte, len, NULL, 0);
    if (l && l->grupos_forcados > 0) {
        PSTokenList *l2 = sonda_passada(fonte, len, l->fechados, l->nfechados);
        if (l2) { ps_lexer_free(l); return l2; }
    }
    return l;
}

/* Uma volta a mais do lexer do fluxo; no fim do fonte fecha a lista. */
static void fluxo_avanca(PSTokenList *o)
{
    Lexer *lx = o->lexer;
    if (lx_passo(lx)) return;
    lx_fim(lx);
    free(lx);
    o->lexer = NULL;
}

PSToken *ps_lexer_tok(PSTokenList *o, int32_t i)
{
    if (i < 0) return NULL;
    /* Dois tokens ALÉM do pedido: o span (`nchars`, `col_fim`) de um token só
     * fica pronto no começo da volta seguinte do laço (`fecha_span`), e o
     * parser lê o do token que acabou de consumir. */
    while (o->lexer && o->n <= i + 2) fluxo_avanca(o);
    return i < o->n ? tok_de(o, i) : NULL;
}

void ps_lexer_drena(PSTokenList *o)
{
    while (o->lexer) fluxo_avanca(o);
}

void ps_lexer_solta_ate(PSTokenList *o, int32_t i)
{
    if (!o->fluxo) return;
    int32_t ate = i / PS_TOK_BLOCO;              /* o bloco de `i` fica */
    if (ate > o->nblocos) ate = o->nblocos;
    if (ate <= o->soltos) return;
    for (int32_t b = o->soltos; b < ate; b++) { bloco_solta(o->blocos[b]); o->blocos[b] = NULL; }
    o->soltos = ate;
    /* os `{...}` das f-strings dos blocos soltos saem junto */
    int32_t corte = ate * PS_TOK_BLOCO, w = 0;
    for (int32_t k = 0; k < o->ninterps; k++) {
        if (o->interps[k].tok < corte) free(o->interps[k].txt);
        else o->interps[w++] = o->interps[k];
    }
    o->ninterps = w;
}

void ps_lexer_free(PSTokenList *lista)
{
    if (!lista) return;
    if (lista->fluxo) {
        for (int32_t b = 0; b < lista->nblocos; b++) bloco_solta(lista->blocos[b]);
        free(lista->blocos);
        free(lista->lexer);
    } else {
        for (int32_t i = 0; i < lista->n; i++) free(lista->tokens[i].texto);
        free(lista->tokens);
    }
    free(lista->avisos);
    free(lista->erros);
    for (int32_t i = 0; i < lista->ninterps; i++) free(lista->interps[i].txt);
    free(lista->interps);
    free(lista->fechados);
    free(lista);
}

const char *ps_tok_nome(PSTokType t)
{
    switch (t) {
        case T_COMMENT:     return "COMMENT";
        case T_ERRO:        return "ERRO";
        case T_SINC:        return "SINC";
        case T_EOF:         return "EOF";
        case T_KW:          return "KW";
        case T_IDENT:       return "IDENT";
        case T_IDENT_UPPER: return "IDENT_UPPER";
        case T_INT:         return "INT";
        case T_FLO:         return "FLO";
        case T_STR:         return "STR";
        case T_FSTRING:     return "FSTRING";
        case T_BYTES:       return "BYTES";
        case T_BOOL:        return "BOOL";
        case T_NULL:        return "NULL";
        case T_COLOR:       return "COLOR";
        case T_OP:          return "OP";
        case T_NEWLINE:     return "NEWLINE";
        case T_INDENT:      return "INDENT";
        case T_DEDENT:      return "DEDENT";
        case T_LPAREN:      return "LPAREN";
        case T_RPAREN:      return "RPAREN";
        case T_LBRACE:      return "LBRACE";
        case T_RBRACE:      return "RBRACE";
        case T_LBRACK:      return "LBRACK";
        case T_RBRACK:      return "RBRACK";
        case T_COLON:       return "COLON";
        case T_SEMI:        return "SEMI";
        case T_COMMA:       return "COMMA";
        case T_DOT:         return "DOT";
        case T_AT:          return "AT";
    }
    return "?";
}
