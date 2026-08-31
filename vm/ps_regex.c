#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ps_regex.h"

/* ── árvore do padrão ────────────────────────────────────────────────────
 *
 *   Alt   := Seq ('|' Seq)*
 *   Seq   := Quant*
 *   Quant := Atomo ('*' | '+' | '?' | '{n,m}')? '?'?
 *   Atomo := char | '.' | classe | '(' Alt ')' | '^' | '$'
 *
 * Um Seq guarda os Quant num array; a alternância é uma lista de Seq. */

typedef enum { A_CHAR, A_QUALQUER, A_CLASSE, A_GRUPO, A_BOL, A_EOL, A_BACKREF,
               A_WORDB,      /* \b (idx_grupo=0) e \B (idx_grupo=1, negado) */
               A_STARTA,     /* \A — início da string (ignora MULTILINE) */
               A_ENDZ,       /* \Z — fim da string */
               A_LOOK        /* lookahead/lookbehind — ver campos em Atomo */
             } TipoAtomo;

/* flags inline `(?i)`/`(?m)`/`(?s)` — os mesmos do `re` do Python */
#define RX_I 1   /* IGNORECASE */
#define RX_M 2   /* MULTILINE  */
#define RX_S 4   /* DOTALL     */

typedef struct Alt Alt;

/* Classe de caracteres. O assunto é CODEPOINT, não byte: `[^x]` tem que
 * consumir o "ç" inteiro, senão o casamento parte um UTF-8 no meio.
 *
 * ASCII cabe num bitmap; acima de 127 vira lista de faixas, porque um bitmap
 * de 0x110000 bits não paga. `negado` fica separado em vez de inverter o
 * bitmap: a negação é sobre o codepoint, e inverter bits só negaria ASCII. */
typedef struct {
    unsigned char mapa[16];       /* bitmap dos 128 primeiros */
    unsigned int *faixas;         /* pares lo,hi para cp >= 128 */
    int           nfaixas, cap_faixas;
    int           negado;
} Classe;

typedef struct {
    TipoAtomo tipo;
    unsigned int cp;             /* A_CHAR — codepoint (não byte, pra unicode) */
    Classe      classe;           /* A_CLASSE */
    Alt         *grupo;           /* A_GRUPO e A_LOOK (o sub-padrão) */
    int          idx_grupo;       /* -1 = não capturante; A_WORDB: 0=\b 1=\B */
    int          look_neg;        /* A_LOOK: 1 = negativo (?! / ?<!) */
    int          look_atras;      /* A_LOOK: 1 = lookbehind (?<= / ?<!) */
    int          look_larg;       /* A_LOOK lookbehind: largura fixa em codepoints */
    int          flags;           /* flags efetivas NESTE átomo (RX_I|RX_M|RX_S),
                                   * carimbadas no parse — é o que faz `(?i:...)`
                                   * com escopo funcionar no matcher de continuação */
} Atomo;

typedef struct {
    Atomo atomo;
    int   min, max;               /* max = -1 é "sem limite" */
    int   guloso;
} Quant;

typedef struct {
    Quant *itens;
    int    n, cap;
} Seq;

struct Alt {
    Seq *ramos;
    int  n, cap;
};

struct PSRegex {
    Alt *raiz;
    int  ngrupos;
    int  flags;      /* RX_I | RX_M | RX_S */
    /* nomes[g] = nome do grupo g (1..ngrupos), ou NULL. Só nasce se o padrão
     * tiver algum `(?P<nome>...)`. */
    char *nomes[RX_MAX_GRUPOS];
};

/* ── construção ──────────────────────────────────────────────────────────── */

typedef struct {
    const char *p;
    int         n, i;
    int         ngrupos;
    char       *erro;
    int         erro_cap;
    int         falhou;
    int         flags;       /* acumula as flags inline `(?i)` etc. */
    char       *nomes[RX_MAX_GRUPOS];   /* nome de cada grupo, ou NULL */
} Leitor;

/* Falha interna, sem par no `re` (so as de memoria usam esta): mantem o
 * formato antigo, porque nao ha texto do CPython pra copiar. */
static void rerro(Leitor *l, const char *msg)
{
    if (!l->falhou && l->erro && l->erro_cap > 0)
        snprintf(l->erro, (size_t)l->erro_cap, "regex: %s (posicao %d)", msg, l->i);
    l->falhou = 1;
}

/* Erro de sintaxe do padrao. O texto e o do `re` do CPython, VERBATIM,
 * inclusive o sufixo " at position N" — e a posicao e a que ELE acusa: o
 * INICIO do construto culpado, nao onde o cursor parou. */
static void rerro_em(Leitor *l, int pos, const char *msg)
{
    if (!l->falhou && l->erro && l->erro_cap > 0)
        snprintf(l->erro, (size_t)l->erro_cap, "%s at position %d", msg, pos);
    l->falhou = 1;
}

/* As poucas mensagens do `re` que NAO trazem posicao. */
static void rerro_txt(Leitor *l, const char *msg)
{
    if (!l->falhou && l->erro && l->erro_cap > 0)
        snprintf(l->erro, (size_t)l->erro_cap, "%s", msg);
    l->falhou = 1;
}

static Alt *le_alt(Leitor *l);
static void libera_alt(Alt *a);

static void mapa_bit(unsigned char *m, unsigned int c) { if (c < 128) m[c >> 3] |= (unsigned char)(1u << (c & 7)); }
static int  tem_bit(const unsigned char *m, unsigned int c) { return c < 128 ? (m[c >> 3] >> (c & 7)) & 1 : 0; }

static int faixa_add(Classe *cl, unsigned int lo, unsigned int hi)
{
    if (cl->nfaixas + 1 > cl->cap_faixas) {
        int novo = cl->cap_faixas ? cl->cap_faixas * 2 : 4;
        unsigned int *nf = realloc(cl->faixas, sizeof(unsigned int) * 2 * (size_t)novo);
        if (!nf) return -1;
        cl->faixas = nf;
        cl->cap_faixas = novo;
    }
    cl->faixas[cl->nfaixas * 2] = lo;
    cl->faixas[cl->nfaixas * 2 + 1] = hi;
    cl->nfaixas++;
    return 0;
}

/* Adiciona um codepoint: no bitmap se ASCII, como faixa de 1 se não. */
static int classe_add(Classe *cl, unsigned int cp)
{
    if (cp < 128) { mapa_bit(cl->mapa, cp); return 0; }
    return faixa_add(cl, cp, cp);
}

static int classe_contem(const Classe *cl, unsigned int cp)
{
    int dentro = tem_bit(cl->mapa, cp);
    if (!dentro)
        for (int i = 0; i < cl->nfaixas && !dentro; i++)
            if (cp >= cl->faixas[i * 2] && cp <= cl->faixas[i * 2 + 1]) dentro = 1;
    return cl->negado ? !dentro : dentro;
}

/* Decodifica o codepoint em `s[pos]`; devolve quantos bytes ocupou. */
static int cp_le(const char *s, int n, int pos, unsigned int *cp)
{
    const unsigned char *b = (const unsigned char *)s;
    if (pos >= n) return 0;
    if (b[pos] < 0x80) { *cp = b[pos]; return 1; }
    if ((b[pos] & 0xE0) == 0xC0 && pos + 1 < n) { *cp = ((unsigned)(b[pos] & 0x1F) << 6) | (b[pos+1] & 0x3F); return 2; }
    if ((b[pos] & 0xF0) == 0xE0 && pos + 2 < n) {
        *cp = ((unsigned)(b[pos] & 0x0F) << 12) | ((unsigned)(b[pos+1] & 0x3F) << 6) | (b[pos+2] & 0x3F);
        return 3;
    }
    if ((b[pos] & 0xF8) == 0xF0 && pos + 3 < n) {
        *cp = ((unsigned)(b[pos] & 0x07) << 18) | ((unsigned)(b[pos+1] & 0x3F) << 12)
            | ((unsigned)(b[pos+2] & 0x3F) << 6) | (b[pos+3] & 0x3F);
        return 4;
    }
    *cp = b[pos];                 /* byte solto: trata como Latin-1 */
    return 1;
}

/* `\d \w \s` e as maiúsculas negadas. Devolve 0 se a letra não é classe.
 *
 * A negação chega como o conjunto POSITIVO com `negado` ligado. Usado sozinho
 * (`\D+`), esse `negado` é o da própria classe e resolve. Dentro de `[...]` ele
 * precisa ser MATERIALIZADO antes da união — ver `classe_uniao_negada`. */
static int classe_escape(unsigned char e, Classe *cl)
{
    unsigned char letra = e;
    if (e == 'D' || e == 'W' || e == 'S') { letra = (unsigned char)(e + 32); cl->negado = 1; }
    if (letra == 'd') {
        for (unsigned c = '0'; c <= '9'; c++) classe_add(cl, c);
    } else if (letra == 'w') {
        for (unsigned c = '0'; c <= '9'; c++) classe_add(cl, c);
        for (unsigned c = 'a'; c <= 'z'; c++) classe_add(cl, c);
        for (unsigned c = 'A'; c <= 'Z'; c++) classe_add(cl, c);
        classe_add(cl, '_');
        /* fora do ASCII entra inteiro: o `\w` do Python é Unicode-aware, e
         * sem isso "ção" não casaria com `\w+` */
        faixa_add(cl, 128, 0x10FFFF);
    } else if (letra == 's') {
        classe_add(cl, ' '); classe_add(cl, '\t'); classe_add(cl, '\n');
        classe_add(cl, '\r'); classe_add(cl, '\f'); classe_add(cl, '\v');
    } else {
        return 0;
    }
    return 1;
}

static int cmp_faixa(const void *a, const void *b)
{
    unsigned int x = ((const unsigned int *)a)[0], y = ((const unsigned int *)b)[0];
    return x < y ? -1 : (x > y ? 1 : 0);
}

/* Une em `cl` o COMPLEMENTO de `sub`.
 *
 * É o que faz `[\s\S]` (o "qualquer coisa, inclusive \n" de todo mundo),
 * `[a\D]` e `[^\S]` casarem em vez de virarem erro de sintaxe. Antes isto era
 * recusado na leitura: um `negado` dentro de `[...]` não compõe com os outros
 * itens, porque o `negado` da classe vale pro conjunto INTEIRO — `[a\D]` não é
 * "não (a ou dígito)", é "a ou não-dígito".
 *
 * Materializar é resolver a negação ali mesmo: no ASCII, bit a bit; acima de
 * 127, andando pelos BURACOS entre as faixas de `sub` (que precisam estar
 * ordenadas pra isso, daí o qsort — `\w` traz [128,0x10FFFF], `\d` e `\s` não
 * trazem nenhuma, mas o cálculo aqui é geral). */
static int classe_uniao_negada(Classe *cl, const Classe *sub)
{
    for (unsigned int c = 0; c < 128; c++)
        if (!tem_bit(sub->mapa, c)) mapa_bit(cl->mapa, c);

    unsigned int *f = NULL;
    int nf = 0;
    if (sub->nfaixas > 0) {
        f = malloc(sizeof(unsigned int) * 2 * (size_t)sub->nfaixas);
        if (!f) return -1;
        for (int i = 0; i < sub->nfaixas; i++) {
            unsigned int lo = sub->faixas[i * 2], hi = sub->faixas[i * 2 + 1];
            if (hi < 128) continue;              /* já resolvido no bitmap */
            if (lo < 128) lo = 128;
            f[nf * 2] = lo;
            f[nf * 2 + 1] = hi;
            nf++;
        }
        qsort(f, (size_t)nf, sizeof(unsigned int) * 2, cmp_faixa);
    }

    unsigned int prox = 128;
    int erro = 0;
    for (int i = 0; i < nf && !erro; i++) {
        unsigned int lo = f[i * 2], hi = f[i * 2 + 1];
        if (lo > prox) erro = faixa_add(cl, prox, lo - 1);
        if (hi >= 0x10FFFF) { prox = 0x110000; break; }
        if (hi + 1 > prox) prox = hi + 1;
    }
    free(f);
    if (erro) return -1;
    if (prox <= 0x10FFFF) return faixa_add(cl, prox, 0x10FFFF);
    return 0;
}

static unsigned char escape_simples(unsigned char e, int *ok)
{
    *ok = 1;
    switch (e) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'f': return '\f';
        case 'v': return '\v';
        case '0': return '\0';
        case 'a': return '\a';
        default:  break;
    }
    /* qualquer outro caractere escapado vale ele mesmo (`\.` `\*` `\\`) */
    if ((e >= 'a' && e <= 'z') || (e >= 'A' && e <= 'Z') || (e >= '0' && e <= '9')) {
        *ok = 0;                 /* letra/dígito escapado sem significado */
        return e;
    }
    return e;
}

static void le_classe(Leitor *l, Atomo *a)
{
    a->tipo = A_CLASSE;
    memset(&a->classe, 0, sizeof(a->classe));
    Classe *cl = &a->classe;
    int p_abre = l->i;                        /* posicao do '[': e AQUI que o `re` acusa */
    l->i++;                                   /* passa '[' */
    if (l->i < l->n && l->p[l->i] == '^') { cl->negado = 1; l->i++; }
    int primeiro = 1;
    while (l->i < l->n && (l->p[l->i] != ']' || primeiro)) {
        primeiro = 0;
        int c_ini = l->i;   /* inicio deste item: o `re` acusa a FAIXA a partir daqui */
        unsigned int c;
        if (l->p[l->i] == '\\') {
            l->i++;
            if (l->i >= l->n) { rerro_em(l, l->i - 1, "bad escape (end of pattern)"); return; }
            unsigned char e = (unsigned char)l->p[l->i++];
            Classe sub;
            memset(&sub, 0, sizeof(sub));
            if (classe_escape(e, &sub)) {
                if (sub.negado) {
                    /* `[a\D]` é "a ou não-dígito": a negação se resolve AQUI,
                     * porque o `negado` da classe vale pro conjunto inteiro. */
                    int r = classe_uniao_negada(cl, &sub);
                    free(sub.faixas);
                    if (r != 0) { rerro(l, "sem memoria na classe"); return; }
                    continue;
                }
                for (int k = 0; k < 16; k++) cl->mapa[k] |= sub.mapa[k];
                for (int k = 0; k < sub.nfaixas; k++)
                    faixa_add(cl, sub.faixas[k * 2], sub.faixas[k * 2 + 1]);
                free(sub.faixas);
                continue;
            }
            int ok;
            c = escape_simples(e, &ok);
        } else {
            int usados = cp_le(l->p, l->n, l->i, &c);
            l->i += usados ? usados : 1;
        }
        /* faixa `a-z`; um '-' no fim é literal */
        if (l->i + 1 < l->n && l->p[l->i] == '-' && l->p[l->i + 1] != ']') {
            l->i++;
            unsigned int fim;
            if (l->p[l->i] == '\\') {
                l->i++;
                if (l->i >= l->n) { rerro_em(l, l->i - 1, "bad escape (end of pattern)"); return; }
                int ok;
                fim = escape_simples((unsigned char)l->p[l->i++], &ok);
            } else {
                int usados = cp_le(l->p, l->n, l->i, &fim);
                l->i += usados ? usados : 1;
            }
            if (fim < c) {
                char msgb[80];
                snprintf(msgb, sizeof(msgb), "bad character range %.*s",
                         l->i - c_ini, l->p + c_ini);
                rerro_em(l, c_ini, msgb);
                return;
            }
            if (c < 128 && fim < 128) for (unsigned k = c; k <= fim; k++) mapa_bit(cl->mapa, k);
            else faixa_add(cl, c, fim);
        } else {
            classe_add(cl, c);
        }
    }
    if (l->i >= l->n) { rerro_em(l, p_abre, "unterminated character set"); return; }
    l->i++;                                   /* passa ']' */
}

static int larg_fixa_alt(const Alt *a);   /* largura fixa em codepoints, -1 = variável */

static int le_atomo(Leitor *l, Atomo *a)
{
    if (l->i >= l->n) return 0;
    char c = l->p[l->i];
    a->grupo = NULL;
    a->idx_grupo = -1;
    a->flags = l->flags;          /* carimba as flags efetivas neste átomo */
    a->look_neg = a->look_atras = a->look_larg = 0;
    if (c == '(') {
        int p_abre = l->i;                /* posicao do '(': e AQUI que o `re` acusa */
        l->i++;
        int captura = 1;
        int eh_look = 0, lk_neg = 0, lk_atras = 0;
        int flags_escopo = 0, restaura_flags = 0, flags_salvo = l->flags;
        int nome_ini = -1, nome_len = 0;
        if (l->i + 1 < l->n && l->p[l->i] == '?') {
            char esp = l->p[l->i + 1];
            if (esp == 'i' || esp == 'm' || esp == 's') {
                /* flags inline: `(?ims)` global (sem átomo) ou `(?ims:...)` com
                 * escopo (só dentro do grupo). Distinção: ')' vs ':'. */
                int j = l->i + 1, fl = 0;
                while (j < l->n && (l->p[j]=='i' || l->p[j]=='m' || l->p[j]=='s')) {
                    if      (l->p[j]=='i') fl |= RX_I;
                    else if (l->p[j]=='m') fl |= RX_M;
                    else                    fl |= RX_S;
                    j++;
                }
                if (j < l->n && l->p[j] == ')') {          /* global */
                    l->flags |= fl; l->i = j + 1;
                    return le_atomo(l, a);
                }
                if (j < l->n && l->p[j] == ':') {          /* escopo */
                    captura = 0; flags_escopo = fl; restaura_flags = 1;
                    l->flags |= fl; l->i = j + 1;
                } else { rerro_em(l, j, j < l->n ? "unknown flag"
                                                 : "missing -, : or )"); return 0; }
            }
            else if (esp == ':') { captura = 0; l->i += 2; }
            else if (esp == '=') { eh_look = 1; lk_neg = 0; lk_atras = 0; captura = 0; l->i += 2; }
            else if (esp == '!') { eh_look = 1; lk_neg = 1; lk_atras = 0; captura = 0; l->i += 2; }
            else if (esp == '<' && l->i + 2 < l->n && (l->p[l->i+2] == '=' || l->p[l->i+2] == '!')) {
                eh_look = 1; lk_atras = 1; lk_neg = (l->p[l->i+2] == '!'); captura = 0; l->i += 3;
            }
            else if ((esp == 'P' && l->i + 2 < l->n && l->p[l->i + 2] == '<')
                     || (esp == '<' && l->i + 2 < l->n
                         && l->p[l->i+2] != '=' && l->p[l->i+2] != '!')) {
                /* grupo nomeado `(?P<nome>...)` — e também a forma curta
                 * `(?<nome>...)`, que o `re` aceita desde o 3.12. O nome é
                 * GUARDADO (era descartado), pra `\g<nome>` funcionar. */
                l->i += (esp == 'P') ? 3 : 2;
                int n0 = l->i;
                while (l->i < l->n && l->p[l->i] != '>') l->i++;
                if (l->i >= l->n) { rerro_em(l, n0, "missing >, unterminated name"); return 0; }
                nome_ini = n0; nome_len = l->i - n0;
                if (nome_len <= 0) { rerro_em(l, n0, "missing group name"); return 0; }
                l->i++;
            }
            else {
                /* o `re` cita a extensao que ele nao conhece: `?y`, `?Pz`, ... */
                char msgb[48];
                if (esp == 'P' && l->i + 2 < l->n)
                    snprintf(msgb, sizeof(msgb), "unknown extension ?P%c", l->p[l->i + 2]);
                else
                    snprintf(msgb, sizeof(msgb), "unknown extension ?%c", esp);
                rerro_em(l, l->i, msgb);
                return 0;
            }
        }
        a->tipo = eh_look ? A_LOOK : A_GRUPO;
        a->look_neg = lk_neg; a->look_atras = lk_atras;
        (void)flags_escopo;
        if (captura) {
            if (l->ngrupos + 1 >= RX_MAX_GRUPOS) { rerro(l, "grupos demais"); return 0; }
            a->idx_grupo = ++l->ngrupos;
            if (nome_ini >= 0) {
                char *nm = malloc((size_t)nome_len + 1);
                if (!nm) { rerro(l, "sem memoria"); return 0; }
                memcpy(nm, l->p + nome_ini, (size_t)nome_len);
                nm[nome_len] = '\0';
                l->nomes[a->idx_grupo] = nm;
            }
        }
        a->grupo = le_alt(l);
        if (restaura_flags) l->flags = flags_salvo;   /* flags de escopo saem do grupo */
        if (l->falhou) { libera_alt(a->grupo); a->grupo = NULL; return 0; }
        if (l->i >= l->n || l->p[l->i] != ')') {
            rerro_em(l, p_abre, "missing ), unterminated subpattern");
            libera_alt(a->grupo); a->grupo = NULL;
            return 0;
        }
        l->i++;
        if (eh_look && lk_atras) {
            a->look_larg = larg_fixa_alt(a->grupo);
            if (a->look_larg < 0) {
                rerro_txt(l, "look-behind requires fixed-width pattern");
                libera_alt(a->grupo); a->grupo = NULL; return 0;
            }
        }
        return 1;
    }
    if (c == '[') {
        le_classe(l, a);
        if (l->falhou) { free(a->classe.faixas); a->classe.faixas = NULL; return 0; }
        return 1;
    }
    if (c == '.') { l->i++; a->tipo = A_QUALQUER; return 1; }
    if (c == '^') { l->i++; a->tipo = A_BOL; return 1; }
    if (c == '$') { l->i++; a->tipo = A_EOL; return 1; }
    if (c == '\\') {
        l->i++;
        if (l->i >= l->n) { rerro_em(l, l->i - 1, "bad escape (end of pattern)"); return 0; }
        unsigned char e = (unsigned char)l->p[l->i++];
        memset(&a->classe, 0, sizeof(a->classe));
        if (classe_escape(e, &a->classe)) { a->tipo = A_CLASSE; return 1; }
        if (e >= '1' && e <= '9') {           /* retrovisor `\1`..`\9` */
            if (e - '0' > l->ngrupos) {
                char msgb[48];
                snprintf(msgb, sizeof(msgb), "invalid group reference %c", e);
                rerro_em(l, l->i - 1, msgb);   /* o `re` aponta o DIGITO, nao a barra */
                return 0;
            }
            a->tipo = A_BACKREF;
            a->idx_grupo = e - '0';
            return 1;
        }
        if (e == 'b' || e == 'B') { a->tipo = A_WORDB; a->idx_grupo = (e == 'B'); return 1; }
        if (e == 'A') { a->tipo = A_STARTA; return 1; }
        if (e == 'Z') { a->tipo = A_ENDZ;   return 1; }
        int ok;
        unsigned char v = escape_simples(e, &ok);
        if (!ok) {
            char msgb[48];
            snprintf(msgb, sizeof(msgb), "bad escape \\%c", e);
            rerro_em(l, l->i - 2, msgb);       /* o `re` aponta a BARRA */
            return 0;
        }
        a->tipo = A_CHAR; a->cp = v;
        return 1;
    }
    if (c == ')' || c == '|') return 0;        /* fim deste Seq */
    if (c == '*' || c == '+' || c == '?') {
        /* O `re` separa os dois casos: quantificador sem alvo nenhum e
         * quantificador EM CIMA de outro. O que distingue e o caractere
         * anterior — se ele fecha um quantificador ja consumido pelo
         * `le_seq`, entao e repeticao de repeticao. */
        char ant = l->i > 0 ? l->p[l->i - 1] : '\0';
        int repetido = (ant == '*' || ant == '+' || ant == '?' || ant == '}');
        rerro_em(l, l->i, repetido ? "multiple repeat" : "nothing to repeat");
        return 0;
    }
    /* literal: lê o CODEPOINT inteiro (não um byte), pra `é`/`ção` casarem
     * como um caractere só — inclusive com quantificador (`é+`) e folding. */
    {
        unsigned int cp;
        int u = cp_le(l->p, l->n, l->i, &cp);
        l->i += u ? u : 1;
        a->tipo = A_CHAR;
        a->cp = u ? cp : (unsigned char)c;
    }
    return 1;
}

static void seq_push(Leitor *l, Seq *s, Quant q)
{
    if (s->n + 1 > s->cap) {
        int novo = s->cap ? s->cap * 2 : 8;
        Quant *nq = realloc(s->itens, sizeof(Quant) * (size_t)novo);
        if (!nq) { rerro(l, "sem memoria"); return; }
        s->itens = nq;
        s->cap = novo;
    }
    s->itens[s->n++] = q;
}

static void le_seq(Leitor *l, Seq *s)
{
    memset(s, 0, sizeof(*s));
    for (;;) {
        Quant q;
        memset(&q, 0, sizeof(q));
        if (!le_atomo(l, &q.atomo)) break;
        if (l->falhou) return;
        q.min = 1; q.max = 1; q.guloso = 1;
        if (l->i < l->n) {
            char t = l->p[l->i];
            if (t == '*')      { q.min = 0; q.max = -1; l->i++; }
            else if (t == '+') { q.min = 1; q.max = -1; l->i++; }
            else if (t == '?') { q.min = 0; q.max = 1;  l->i++; }
            else if (t == '{') {
                /* `{` sem número é literal, como no `re` */
                int salvo = l->i, j = l->i + 1, lo = 0, hi = -1, temlo = 0;
                while (j < l->n && l->p[j] >= '0' && l->p[j] <= '9') { lo = lo * 10 + (l->p[j] - '0'); j++; temlo = 1; }
                if (j < l->n && l->p[j] == ',') {
                    j++;
                    int temhi = 0, h = 0;
                    while (j < l->n && l->p[j] >= '0' && l->p[j] <= '9') { h = h * 10 + (l->p[j] - '0'); j++; temhi = 1; }
                    hi = temhi ? h : -1;
                } else {
                    hi = lo;
                }
                if (temlo && j < l->n && l->p[j] == '}') {
                    q.min = lo; q.max = hi; l->i = j + 1;
                    if (hi >= 0 && hi < lo) {
                        rerro_em(l, salvo + 1, "min repeat greater than max repeat");
                        return;
                    }
                } else {
                    l->i = salvo;              /* trata como literal '{' */
                }
            }
            if (l->i < l->n && l->p[l->i] == '?' && (q.min != 1 || q.max != 1)) {
                q.guloso = 0; l->i++;
            }
        }
        seq_push(l, s, q);
        if (l->falhou) return;
    }
}

static Alt *le_alt(Leitor *l)
{
    Alt *a = calloc(1, sizeof(Alt));
    if (!a) { rerro(l, "sem memoria"); return NULL; }
    for (;;) {
        if (a->n + 1 > a->cap) {
            int novo = a->cap ? a->cap * 2 : 4;
            Seq *ns = realloc(a->ramos, sizeof(Seq) * (size_t)novo);
            if (!ns) { rerro(l, "sem memoria"); return a; }
            a->ramos = ns;
            a->cap = novo;
        }
        le_seq(l, &a->ramos[a->n]);
        a->n++;
        if (l->falhou) return a;
        if (l->i < l->n && l->p[l->i] == '|') { l->i++; continue; }
        break;
    }
    return a;
}

static void libera_alt(Alt *a);

static void libera_seq(Seq *s)
{
    for (int i = 0; i < s->n; i++) {
        TipoAtomo t = s->itens[i].atomo.tipo;
        if (t == A_GRUPO || t == A_LOOK) libera_alt(s->itens[i].atomo.grupo);
        else if (t == A_CLASSE) free(s->itens[i].atomo.classe.faixas);
    }
    free(s->itens);
}

static void libera_alt(Alt *a)
{
    if (!a) return;
    for (int i = 0; i < a->n; i++) libera_seq(&a->ramos[i]);
    free(a->ramos);
    free(a);
}

/* Largura fixa (em CODEPOINTS) de um sub-padrão, pro lookbehind. -1 = variável.
 * Char/classe/`.` valem 1 codepoint; âncoras 0; grupo recorre; retrovisor e
 * quantificador variável dão -1 -> lookbehind recusado na compilação. */
static int larg_fixa_atomo(const Atomo *a);
static int larg_fixa_alt(const Alt *a)
{
    int larg = -1;
    for (int r = 0; r < a->n; r++) {
        const Seq *s = &a->ramos[r];
        int soma = 0;
        for (int i = 0; i < s->n; i++) {
            const Quant *q = &s->itens[i];
            if (q->min != q->max) return -1;
            int w = larg_fixa_atomo(&q->atomo);
            if (w < 0) return -1;
            soma += w * q->min;
        }
        if (larg < 0) larg = soma;
        else if (larg != soma) return -1;
    }
    return larg < 0 ? 0 : larg;
}
static int larg_fixa_atomo(const Atomo *a)
{
    switch (a->tipo) {
        case A_CHAR: case A_CLASSE: case A_QUALQUER: return 1;   /* 1 codepoint */
        case A_BOL: case A_EOL: case A_WORDB: case A_STARTA: case A_ENDZ: case A_LOOK: return 0;
        case A_GRUPO:  return larg_fixa_alt(a->grupo);
        default:       return -1;   /* A_BACKREF */
    }
}

PSRegex *ps_regex_compila(const char *padrao, int len, char *erro, int erro_cap)
{
    Leitor l = { padrao, len, 0, 0, erro, erro_cap, 0, 0, { NULL } };
    Alt *raiz = le_alt(&l);
    if (!l.falhou && l.i != l.n) {
        /* sobrou ')' sem abrir, ou coisa parecida */
        rerro_em(&l, l.i, "unbalanced parenthesis");
    }
    PSRegex *r = l.falhou ? NULL : malloc(sizeof(PSRegex));
    if (!r) {
        libera_alt(raiz);
        for (int g = 0; g < RX_MAX_GRUPOS; g++) free(l.nomes[g]);
        return NULL;
    }
    r->raiz = raiz;
    r->ngrupos = l.ngrupos;
    r->flags = l.flags;
    memcpy(r->nomes, l.nomes, sizeof(r->nomes));
    return r;
}

void ps_regex_free(PSRegex *r)
{
    if (!r) return;
    libera_alt(r->raiz);
    for (int g = 0; g < RX_MAX_GRUPOS; g++) free(r->nomes[g]);
    free(r);
}

int ps_regex_ngrupos(const PSRegex *r) { return r ? r->ngrupos : 0; }

int ps_regex_grupo_por_nome(const PSRegex *r, const char *nome, int len)
{
    if (!r || len <= 0) return -1;
    for (int g = 1; g <= r->ngrupos && g < RX_MAX_GRUPOS; g++)
        if (r->nomes[g] && (int)strlen(r->nomes[g]) == len && !memcmp(r->nomes[g], nome, (size_t)len))
            return g;
    return -1;
}

const char *ps_regex_nome_do_grupo(const PSRegex *r, int g)
{
    if (!r || g < 1 || g >= RX_MAX_GRUPOS) return NULL;
    return r->nomes[g];
}

/* ── casamento ───────────────────────────────────────────────────────────
 *
 * Backtracking com continuação explícita: `Cont` é a lista do que falta casar
 * depois do ponto atual. Sem ela, um grupo com alternância não saberia
 * retomar o resto do padrão ao voltar atrás. */

#define RX_MAX_PASSOS 2000000

/* TETO DE PROFUNDIDADE, e ele é obrigatório junto com o de passos.
 *
 * O casador é recursivo: `m_seq -> m_pos_grupo -> m_rep_grupo -> m_alt ->
 * m_seq` gasta um quadro de pilha C por caractere consumido. O teto de PASSOS
 * (2 milhões) não protege disso — a pilha de 8 MB acaba muito antes, e o
 * processo morre de SIGSEGV sem mensagem nenhuma.
 *
 * Aconteceu de verdade: `"((?:[^"\\]|\\.)*)"` sobre um trecho de 29 mil
 * caracteres derrubou o `pool` inteiro. Um padrão vindo do usuário não pode
 * matar o processo — tem que virar erro, como qualquer outro limite.
 *
 * 6000 quadros cabem com folga nos 8 MB padrão e ainda dão conta de qualquer
 * padrão de uso real. */
#define RX_MAX_PROF 6000

typedef struct Cont { const Seq *seq; int i; const struct Cont *prox; } Cont;

typedef struct {
    const char *s;
    int         n;
    RxCaptura  *cap;
    long        passos;
    int         estourou;
    int         exigir_fim;    /* fullmatch: só aceita terminando em n */
    int         flags;         /* RX_I | RX_M | RX_S */
    int         prof;          /* profundidade de recursão do casador */
} Estado;

/* Folding pro IGNORECASE: ASCII + Latin-1 Supplement (À-Þ<->à-þ), que cobre o
 * não-ASCII comum (café, ção, ñ). Outros scripts (grego, cirílico) não dobram
 * — é o único ponto onde IGNORECASE ainda pode divergir do `re`. */
static unsigned char rx_lower(unsigned char c) { return (c >= 'A' && c <= 'Z') ? (unsigned char)(c + 32) : c; }
static unsigned int  rx_lower_cp(unsigned int cp)
{
    if (cp >= 'A' && cp <= 'Z') return cp + 32;
    if (cp >= 0xC0 && cp <= 0xDE && cp != 0xD7) return cp + 0x20;   /* À-Þ -> à-þ */
    return cp;
}
static unsigned int  rx_swap_cp(unsigned int cp)
{
    if (cp >= 'A' && cp <= 'Z') return cp + 32;
    if (cp >= 'a' && cp <= 'z') return cp - 32;
    if (cp >= 0xC0 && cp <= 0xDE && cp != 0xD7) return cp + 0x20;   /* maiúscula Latin-1 */
    if (cp >= 0xE0 && cp <= 0xFE && cp != 0xF7) return cp - 0x20;   /* minúscula Latin-1 */
    return cp;
}

/* Caractere de palavra (\w): igual ao classe_escape('w') — ASCII alnum + '_'
 * e qualquer não-ASCII (o \w do Python é Unicode-aware). */
static int rx_is_word(unsigned int cp)
{
    return (cp >= '0' && cp <= '9') || (cp >= 'a' && cp <= 'z')
        || (cp >= 'A' && cp <= 'Z') || cp == '_' || cp >= 128;
}

/* Codepoint que TERMINA em `pos` (o caractere logo antes). 0 se pos<=0. */
static int rx_cp_antes(const char *s, int pos, unsigned int *cp)
{
    if (pos <= 0) return 0;
    int i = pos - 1;
    while (i > 0 && ((unsigned char)s[i] & 0xC0) == 0x80) i--;   /* volta ao lead byte */
    return cp_le(s, pos, i, cp);
}

static int m_seq(Estado *e, const Seq *s, int i, int pos, const Cont *k);
static int m_seq_corpo(Estado *e, const Seq *s, int i, int pos, const Cont *k);
static int m_pos_grupo(Estado *e, int pos, const Cont *k);

static int m_cont(Estado *e, int pos, const Cont *k)
{
    if (!k) {
        if (e->exigir_fim && pos != e->n) return 0;
        e->cap->fim[0] = pos;
        return 1;
    }
    /* `seq == NULL` marca a continuação de uma repetição de grupo: o corpo
     * acabou de casar e quem decide repetir ou seguir é o m_pos_grupo. */
    if (k->seq == NULL) return m_pos_grupo(e, pos, k);
    return m_seq(e, k->seq, k->i, pos, k->prox);
}

static int m_alt(Estado *e, const Alt *a, int pos, const Cont *k)
{
    for (int r = 0; r < a->n; r++) {
        if (m_seq(e, &a->ramos[r], 0, pos, k)) return 1;
        if (e->estourou) return 0;
    }
    return 0;
}

/* Lookahead `(?=P)`: P casa QUALQUER prefixo a partir de `pos`? Zero-width —
 * a posição não anda. Restaura fim[0] (o sub-match não é o casamento). */
static int look_ahead(Estado *e, const Alt *alt, int pos)
{
    int sf = e->cap->fim[0], se = e->exigir_fim;
    e->exigir_fim = 0;
    int r = m_alt(e, alt, pos, NULL);
    e->cap->fim[0] = sf; e->exigir_fim = se;
    return r;
}

/* Recua `ncp` codepoints a partir de `pos`; devolve o byte inicial, ou -1 se
 * não houver caracteres suficientes antes. */
static int rx_recua_cp(const char *s, int pos, int ncp)
{
    int i = pos;
    while (ncp-- > 0) {
        if (i <= 0) return -1;
        i--;
        while (i > 0 && ((unsigned char)s[i] & 0xC0) == 0x80) i--;
    }
    return i;
}

/* Lookbehind `(?<=P)`: P casa EXATAMENTE de `start` até `fim`? Encolhe o `n`
 * pro sub-padrão não ler além de `fim`, e exige terminar lá. */
static int look_atras_em(Estado *e, const Alt *alt, int start, int fim)
{
    if (start < 0) return 0;
    int sn = e->n, se = e->exigir_fim, sf0 = e->cap->fim[0];
    e->n = fim; e->exigir_fim = 1;
    int r = m_alt(e, alt, start, NULL);
    e->n = sn; e->exigir_fim = se; e->cap->fim[0] = sf0;
    return r;
}

/* Casa um átomo SIMPLES (não-grupo) em `pos`. Devolve bytes consumidos, ou -1. */
static int casa_simples(Estado *e, const Atomo *a, int pos)
{
    switch (a->tipo) {
        case A_BOL:
            if (pos == 0) return 0;
            if ((a->flags & RX_M) && pos > 0 && e->s[pos - 1] == '\n') return 0;  /* MULTILINE */
            return -1;
        case A_EOL:
            if (pos == e->n) return 0;
            if ((a->flags & RX_M) && pos < e->n && e->s[pos] == '\n') return 0;   /* MULTILINE */
            return -1;
        case A_QUALQUER: {
            /* consome o CODEPOINT inteiro — um `.` não pode partir UTF-8 no
             * meio. Sem DOTALL não casa '\n'; com DOTALL casa. */
            if (pos >= e->n) return -1;
            if (!(a->flags & RX_S) && e->s[pos] == '\n') return -1;
            unsigned int cp;
            int u = cp_le(e->s, e->n, pos, &cp);
            return u ? u : -1;
        }
        case A_CHAR: {
            if (pos >= e->n) return -1;
            unsigned int cp;
            int u = cp_le(e->s, e->n, pos, &cp);
            if (!u) return -1;
            if (a->flags & RX_I)
                return rx_lower_cp(cp) == rx_lower_cp(a->cp) ? u : -1;
            return cp == a->cp ? u : -1;
        }
        case A_CLASSE: {
            if (pos >= e->n) return -1;
            unsigned int cp;
            int u = cp_le(e->s, e->n, pos, &cp);
            if (!u) return -1;
            int ok = classe_contem(&a->classe, cp);
            if (!ok && (a->flags & RX_I)) {          /* IGNORECASE: tenta o outro caso */
                unsigned int alt = rx_swap_cp(cp);
                if (alt != cp) ok = classe_contem(&a->classe, alt);
            }
            return ok ? u : -1;
        }
        case A_STARTA: return pos == 0    ? 0 : -1;   /* \A */
        case A_ENDZ:   return pos == e->n ? 0 : -1;   /* \Z */
        case A_WORDB: {                               /* \b (idx 0) / \B (idx 1) */
            unsigned int cpa = 0, cpb = 0;
            int tem_a = (pos < e->n) && cp_le(e->s, e->n, pos, &cpa);
            int tem_b = rx_cp_antes(e->s, pos, &cpb);
            int wa = tem_a && rx_is_word(cpa);
            int wb = tem_b && rx_is_word(cpb);
            int fronteira = (wa != wb);
            int quer = (a->idx_grupo == 0) ? fronteira : !fronteira;
            return quer ? 0 : -1;
        }
        case A_BACKREF: {
            /* casa os MESMOS bytes que o grupo referenciado capturou */
            int gi = a->idx_grupo;
            if (gi < 1 || gi >= RX_MAX_GRUPOS) return -1;
            int gi_ini = e->cap->inicio[gi], gi_fim = e->cap->fim[gi];
            if (gi_ini < 0 || gi_fim < 0) return -1;   /* grupo não participou */
            int len = gi_fim - gi_ini;
            if (len == 0) return 0;
            if (pos + len > e->n) return -1;
            for (int t = 0; t < len; t++) {
                unsigned char x = (unsigned char)e->s[pos + t];
                unsigned char y = (unsigned char)e->s[gi_ini + t];
                if (a->flags & RX_I) { x = rx_lower(x); y = rx_lower(y); }
                if (x != y) return -1;
            }
            return len;
        }
        default:
            return -1;
    }
}

/* Estado de uma repetição de grupo em andamento. */
typedef struct {
    Estado      *e;
    const Quant *q;
    const Seq   *seq;      /* sequência que contém o grupo */
    int          i;        /* posição do grupo dentro dela */
    const Cont  *k;        /* o que vem depois da sequência */
} RepGrupo;

/* Continuação plantada entre repetições. `base.seq = NULL` é a marca. */
typedef struct { Cont base; RepGrupo *rg; int feitos; int pos_ini; } ContRep;

static int m_rep_grupo(RepGrupo *rg, int feitos, int pos);

static int m_pos_grupo(Estado *e, int pos, const Cont *k)
{
    (void)e;
    const ContRep *cr = (const ContRep *)k;
    RepGrupo *rg = cr->rg;
    int gi = rg->q->atomo.idx_grupo;
    if (gi > 0) rg->e->cap->fim[gi] = pos;      /* esta repetição fechou aqui */
    /* corpo que não consumiu nada: repetir de novo seria laço infinito */
    if (pos == cr->pos_ini)
        return m_seq(rg->e, rg->seq, rg->i + 1, pos, rg->k);
    return m_rep_grupo(rg, cr->feitos + 1, pos);
}

static int m_rep_grupo(RepGrupo *rg, int feitos, int pos)
{
    Estado *e = rg->e;
    if (++e->passos > RX_MAX_PASSOS) { e->estourou = 1; return 0; }
    const Quant *q = rg->q;
    const Atomo *a = &q->atomo;

    int pode_mais = (q->max < 0 || feitos < q->max);
    int ja_deu    = (feitos >= q->min);

    /* preguiçoso tenta SAIR antes de repetir */
    if (!q->guloso && ja_deu) {
        if (m_seq(e, rg->seq, rg->i + 1, pos, rg->k)) return 1;
        if (e->estourou) return 0;
    }

    if (pode_mais) {
        ContRep cr;
        cr.base.seq = NULL; cr.base.i = 0; cr.base.prox = NULL;
        cr.rg = rg; cr.feitos = feitos; cr.pos_ini = pos;

        int gi = a->idx_grupo;
        int si = 0, sf = 0;
        if (gi > 0) { si = e->cap->inicio[gi]; sf = e->cap->fim[gi]; e->cap->inicio[gi] = pos; }
        if (m_alt(e, a->grupo, pos, (const Cont *)&cr)) return 1;
        /* este caminho falhou: a captura tem que voltar ao que era */
        if (gi > 0) { e->cap->inicio[gi] = si; e->cap->fim[gi] = sf; }
        if (e->estourou) return 0;
    }

    if (q->guloso && ja_deu)
        return m_seq(e, rg->seq, rg->i + 1, pos, rg->k);
    return 0;
}

/* Casca que conta a profundidade. O corpo tem muitos `return`, e um contador
 * espalhado por todos eles é convite a esquecer um — a casca garante que
 * entrar e sair sempre fecham. */
static int m_seq(Estado *e, const Seq *s, int i, int pos, const Cont *k)
{
    if (e->prof >= RX_MAX_PROF) { e->estourou = 1; return 0; }
    e->prof++;
    int r = m_seq_corpo(e, s, i, pos, k);
    e->prof--;
    return r;
}

static int m_seq_corpo(Estado *e, const Seq *s, int i, int pos, const Cont *k)
{
    if (++e->passos > RX_MAX_PASSOS) { e->estourou = 1; return 0; }
    if (i >= s->n) return m_cont(e, pos, k);

    const Quant *q = &s->itens[i];
    const Atomo *a = &q->atomo;

    if (a->tipo == A_GRUPO) {
        RepGrupo rg = { e, q, s, i, k };
        return m_rep_grupo(&rg, 0, pos);
    }

    if (a->tipo == A_LOOK) {
        /* asserção de largura zero: verifica e segue no MESMO pos */
        int ok = a->look_atras
                     ? look_atras_em(e, a->grupo, rx_recua_cp(e->s, pos, a->look_larg), pos)
                     : look_ahead(e, a->grupo, pos);
        if (a->look_neg) ok = !ok;
        if (!ok) return 0;
        return m_seq(e, s, i + 1, pos, k);
    }

    /* átomo simples: mede o máximo e volta atrás conforme a ganância */
    int max_pode = 0;
    int p = pos;
    while (q->max < 0 || max_pode < q->max) {
        int c = casa_simples(e, a, p);
        if (c < 0) break;
        p += c;
        max_pode++;
        if (c == 0) break;                 /* âncora não consome, não repete */
    }
    if (max_pode < q->min) return 0;

    int ini = q->guloso ? max_pode : q->min;
    int fim = q->guloso ? q->min : max_pode;
    int passo = q->guloso ? -1 : 1;
    for (int cont = ini; q->guloso ? cont >= fim : cont <= fim; cont += passo) {
        int np = pos;
        for (int t = 0; t < cont; t++) {
            int c = casa_simples(e, a, np);
            if (c < 0) break;
            np += c;
        }
        if (m_seq(e, s, i + 1, np, k)) return 1;
        if (e->estourou) return 0;
    }
    return 0;
}

int ps_regex_busca(PSRegex *r, const char *s, int len, int de, RxCaptura *cap)
{
    for (int inicio = de; inicio <= len; inicio++) {
        Estado e = { s, len, cap, 0, 0, 0, r->flags, 0 };
        for (int g = 0; g < RX_MAX_GRUPOS; g++) { cap->inicio[g] = -1; cap->fim[g] = -1; }
        cap->ngrupos = r->ngrupos;
        cap->inicio[0] = inicio;
        if (m_alt(&e, r->raiz, inicio, NULL)) return 1;
        if (e.estourou) return -1;
    }
    return 0;
}

int ps_regex_casa_tudo(PSRegex *r, const char *s, int len, RxCaptura *cap)
{
    Estado e = { s, len, cap, 0, 0, 1, r->flags, 0 };
    for (int g = 0; g < RX_MAX_GRUPOS; g++) { cap->inicio[g] = -1; cap->fim[g] = -1; }
    cap->ngrupos = r->ngrupos;
    cap->inicio[0] = 0;
    if (m_alt(&e, r->raiz, 0, NULL)) return 1;
    if (e.estourou) return -1;
    return 0;
}
