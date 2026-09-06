/*
 * Lexer da PoolScript em C puro.
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
 * Sem dependência do CPython: só a libc.
 */
#include "ps_lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── tabelas da linguagem ───────────────────────────────────────────────── */

static const char *KEYWORDS[] = {
    "if", "else", "elif", "while", "for", "each", "in", "is",
    "and", "or", "not", "Not",
    "action", "reaction", "return", "continue", "break", "pass", "model", "enum", "async", "await",
    "try", "catch", "as", "with", "of", "using",
    "import", "from", "PUSH", "GET",
    "str", "int", "flo", "bool",
    "post", "input", "listen", "route", "create",
    "clear", "space", "addEnd", "char", "list",
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
    NULL
};

/* A lista de palavras-chave, pra quem precisa dela FORA do lexer — o
 * `--metadata` a publica e o editor sugere `action`, `if`, `for each`… a
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

    PSTokenList *out;
} Lexer;

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
 * bytes) empurrava a coluna 1 casa à frente do lexer em Python. */
static void avanca1(Lexer *lx)
{
    if (((unsigned char)lx->src[lx->pos] & 0xC0) != 0x80) lx->col++;
    lx->pos++;
}

static void erro(Lexer *lx, const char *msg)
{
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
    o->navisos++;
}

static void erro_em(Lexer *lx, const char *msg, int32_t l, int32_t c)
{
    if (!lx->out->ok) return;
    lx->out->ok = 0;
    snprintf(lx->out->erro, sizeof(lx->out->erro), "%s", msg);
    lx->out->erro_linha = l;
    lx->out->erro_col = c;
}

static PSToken *novo_token(Lexer *lx, PSTokType t, int32_t linha, int32_t col)
{
    PSTokenList *o = lx->out;
    if (o->n + 1 > o->cap) {
        int32_t novo = o->cap < 64 ? 64 : o->cap * 2;
        PSToken *p = realloc(o->tokens, sizeof(PSToken) * (size_t)novo);
        if (!p) { erro(lx, "sem memoria"); return NULL; }
        o->tokens = p;
        o->cap = novo;
    }
    PSToken *tk = &o->tokens[o->n++];
    memset(tk, 0, sizeof(*tk));
    tk->type = t;
    tk->line = linha;
    tk->col = col;
    return tk;
}

static int guarda_texto(Lexer *lx, PSToken *tk, const char *s, int n)
{
    tk->texto = malloc((size_t)n + 1);
    if (!tk->texto) { erro(lx, "sem memoria"); return -1; }
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

/* ── quebra de linha + indentação ───────────────────────────────────────── */
static void trata_newline(Lexer *lx)
{
    lx->pos++;
    lx->linha++;
    lx->col = 1;

    /* Dentro de `(` e `[` a indentação não conta (expressão multilinha). Mas
     * `{` NÃO entra aqui: ele também abre BLOCO, e um bloco pode ter um
     * sub-bloco `:` dentro — suprimir o NEWLINE/INDENT fazia
     * `if (x) { action f(): ... }` morrer com "faltou quebra de linha apos
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
 * em bf e avança lx->pos/col pelos chars consumidos. Espelha o _decode_escape
 * reconhecidos: \n \t \r \a \b \f \v \e, \\ \" \', octal \033,
 * hex \x1b, unicode \uXXXX/\UXXXXXXXX.
 *
 * DESCONHECIDO MANTÉM A BARRA E AVISA — como o CPython. Antes a barra sumia,
 * calada: `"C:\pasta"` virava `C:pasta` (7 bytes) em vez de `C:\pasta` (8), e
 * ninguém ficava sabendo. Perder um byte do dado do usuário em silêncio é o
 * pior dos dois mundos; o Python emite `SyntaxWarning: invalid escape
 * sequence '\p'` e preserva os dois caracteres. 0 ok, -1 mem. */
static int decode_escape(Lexer *lx, Buf *bf)
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
                cp = v; consumido = (int)(j - i);
            } else if (nxt == 'x' || nxt == 'X') {   /* hex \xHH */
                if (i + 3 < n && ehexdig(s[i+2]) && ehexdig(s[i+3])) {
                    cp = (unsigned long)(hexval(s[i+2]) * 16 + hexval(s[i+3])); consumido = 4;
                } else { ESCAPE_DESCONHECIDO(); }
            } else if (nxt == 'u' || nxt == 'U') {   /* unicode \uXXXX / \UXXXXXXXX */
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
    return buf_push_utf8(bf, cp);
}
#undef ESCAPE_DESCONHECIDO

static void le_string(Lexer *lx, char aspa, int fstring, int raw)
{
    int32_t l0 = lx->linha, c0 = lx->col;
    lx->pos++; lx->col++;
    Buf bf = {0};

    while (lx->pos < lx->len) {
        char c = lx->src[lx->pos];
        if (!raw && c == '\\' && lx->pos + 1 < lx->len) {
            if (decode_escape(lx, &bf) != 0) { free(bf.b); erro(lx, "sem memoria"); return; }
            continue;
        }
        if (c == aspa) {
            lx->pos++; lx->col++;
            PSToken *tk = novo_token(lx, fstring ? T_FSTRING : T_STR, l0, c0);
            if (tk) guarda_texto(lx, tk, bf.b ? bf.b : "", bf.n);
            free(bf.b);
            return;
        }
        if (c == '\n') {
            free(bf.b);
            erro(lx, "string nao fechada antes da quebra de linha");
            return;
        }
        if (buf_push(&bf, c) != 0) { free(bf.b); erro(lx, "sem memoria"); return; }
        avanca1(lx);
    }
    free(bf.b);
    erro_em(lx, "string nao fechada ate o fim do arquivo", l0, c0);
}

static void le_string_tripla(Lexer *lx, char aspa, int fstring, int raw)
{
    int32_t l0 = lx->linha, c0 = lx->col;
    char tres[4] = { aspa, aspa, aspa, '\0' };
    lx->pos += 3; lx->col += 3;
    Buf bf = {0};

    while (lx->pos < lx->len) {
        if (lx->pos + 2 < lx->len && strncmp(lx->src + lx->pos, tres, 3) == 0) {
            lx->pos += 3; lx->col += 3;
            PSToken *tk = novo_token(lx, fstring ? T_FSTRING : T_STR, l0, c0);
            if (tk) guarda_texto(lx, tk, bf.b ? bf.b : "", bf.n);
            free(bf.b);
            return;
        }
        char c = lx->src[lx->pos];
        if (!raw && c == '\\' && lx->pos + 1 < lx->len) {
            if (decode_escape(lx, &bf) != 0) { free(bf.b); erro(lx, "sem memoria"); return; }
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
            guarda_texto(lx, tk, lx->src + ini, (int)(lx->pos - ini));
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

    /* prefixos de string: f"..." f'''...''' r"..." r'''...''' */
    if (n == 1 && (txt[0] == 'f' || txt[0] == 'r')) {
        int fstring = (txt[0] == 'f');
        char prox = lx->pos < lx->len ? lx->src[lx->pos] : '\0';
        if (prox == '\'' && espia(lx, 1) == '\'' && espia(lx, 2) == '\'') {
            le_string_tripla(lx, '\'', fstring, !fstring);
            return;
        }
        /* f"..." e f'...' valem igual — aspas são equivalentes na linguagem */
        if (prox == '"' || prox == '\'') {
            le_string(lx, prox, fstring, !fstring);
            return;
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
    PSToken *tk = novo_token(lx, t, lx->linha, lx->col);
    if (tk) guarda_texto(lx, tk, &c, 1);

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
            PSToken *a = &o->tokens[o->n - 2];
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
static PSTokenList *tokeniza(const char *fonte, size_t len, int com_comentarios);

PSTokenList *ps_lexer_tokenize(const char *fonte, size_t len)
{ return tokeniza(fonte, len, 0); }

PSTokenList *ps_lexer_tokenize_editor(const char *fonte, size_t len)
{ return tokeniza(fonte, len, 1); }

static PSTokenList *tokeniza(const char *fonte, size_t len, int com_comentarios)
{
    PSTokenList *out = calloc(1, sizeof(PSTokenList));
    if (!out) return NULL;
    out->ok = 1;

    Lexer lx;
    memset(&lx, 0, sizeof(lx));
    lx.src = fonte;
    lx.len = len;
    lx.linha = 1;
    lx.col = 1;
    lx.indent[0] = 0;
    lx.nindent = 1;
    lx.out = out;
    lx.marca_comentarios = com_comentarios;

    /* espaços iniciais da primeira linha não geram INDENT */
    while (lx.pos < lx.len && (lx.src[lx.pos] == ' ' || lx.src[lx.pos] == '\t')) {
        lx.pos++; lx.col++;
    }

    /* Span do token no FONTE, medido num lugar só. Todo ramo do laço faz
     * `continue`, então a medição acontece no TOPO da iteração seguinte (e uma
     * última vez depois do laço): se desde a iteração anterior nasceu UM token
     * na MESMA linha, o tamanho é a diferença de coluna. É o que o realce do
     * editor precisa e o `texto` não dá — string decodificada não tem aspas. */
    int32_t col0 = lx.col, lin0 = lx.linha, n0 = out->n;
#define FECHA_SPAN()                                                          \
    do {                                                                      \
        if (out->n == n0 + 1 && lx.linha == lin0 && lx.col > col0)            \
            out->tokens[n0].nchars = lx.col - col0;                           \
        col0 = lx.col; lin0 = lx.linha; n0 = out->n;                          \
    } while (0)

    while (lx.pos < lx.len && out->ok) {
        FECHA_SPAN();
        char c = lx.src[lx.pos];

        if (c == '\n') { trata_newline(&lx); continue; }
        if (c == '\r') { lx.pos++; continue; }
        if (c == ' ' || c == '\t') { lx.pos++; lx.col++; continue; }

        /* `//` era COMENTARIO DE LINHA e agora e o operador de divisao
         * inteira (I11). Nao da pra ter os dois: `a // b` teria que ser
         * divisao num contexto e comentario no outro, e nenhuma regra de
         * desambiguacao sobrevive a `x = a //b` contra `x = a  // b`.
         *
         * O comentario de linha continua sendo `#`, que a linguagem sempre
         * aceitou e que a maior parte do repositorio ja usava. */
        if (c == '#') { pula_comentario_linha(&lx); continue; }

        if (c == '"' && espia(&lx, 1) == '"' && espia(&lx, 2) == '"') {
            pula_comentario_bloco(&lx); continue;
        }
        if (c == '\'' && espia(&lx, 1) == '\'' && espia(&lx, 2) == '\'') {
            le_string_tripla(&lx, '\'', 0, 0); continue;
        }
        if (c == '"' || c == '\'') { le_string(&lx, c, 0, 0); continue; }

        if (eh_digito(c)) { le_numero(&lx); continue; }
        if (eh_alpha(c) || c == '_') { le_ident(&lx); continue; }

        if (le_punct(&lx)) continue;
        if (c == '<' && le_cor(&lx)) continue;
        if (le_operador(&lx)) continue;

        {
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
            if (nb > 0 && lx.pos + (size_t)nb <= lx.len) {
                char ch[5];
                memcpy(ch, lx.src + lx.pos, (size_t)nb);
                ch[nb] = '\0';
                snprintf(m, sizeof(m), "caractere inesperado: '%s'", ch);
            } else {
                snprintf(m, sizeof(m), "byte inesperado: 0x%02X", b0);
            }
            erro(&lx, m);
        }
    }
    FECHA_SPAN();
#undef FECHA_SPAN

    if (out->ok) {
        while (lx.nindent > 1) {
            lx.nindent--;
            PSToken *tk = novo_token(&lx, T_DEDENT, lx.linha, lx.col);
            if (tk) tk->i = 0;
        }
        novo_token(&lx, T_EOF, lx.linha, lx.col);
    }
    return out;
}

void ps_lexer_free(PSTokenList *lista)
{
    if (!lista) return;
    for (int32_t i = 0; i < lista->n; i++) free(lista->tokens[i].texto);
    free(lista->tokens);
    free(lista->avisos);
    free(lista);
}

const char *ps_tok_nome(PSTokType t)
{
    switch (t) {
        case T_COMMENT:     return "COMMENT";
        case T_EOF:         return "EOF";
        case T_KW:          return "KW";
        case T_IDENT:       return "IDENT";
        case T_IDENT_UPPER: return "IDENT_UPPER";
        case T_INT:         return "INT";
        case T_FLO:         return "FLO";
        case T_STR:         return "STR";
        case T_FSTRING:     return "FSTRING";
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
