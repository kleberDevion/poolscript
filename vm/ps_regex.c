#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ps_regex.h"
#include "ps_pilha.h"
#include "ps_unicode.h"

/* Os dois motivos de parar: passos (retrocesso) e profundidade (pilha). Quem
 * chama distingue pelo retorno: -1 e -3. */
#define RX_ESTOUROU_PASSOS 1
#define RX_ESTOUROU_FUNDO  2

/* TETOS CONTADOS (motor-1). A folga medida da pilha decide perto do fim, mas
 * sozinha ela segue o `ulimit -s`: o mesmo programa dava erro limpo com 8 MB e
 * rodava até o fim com as 64 MB que o make 3.81 da Apple dá aos filhos. Estes
 * dois tetos dão o MESMO resultado em qualquer pilha, e a folga continua ali
 * como a rede de segurança do binário de verdade (o quadro do ASan é maior).
 *
 * RX_PADRAO_PROF_MAX conta o aninhamento de GRUPO na leitura do padrão (o
 * `le_atomo -> le_grupo -> le_alt`), e RX_SEQ_PROF_MAX o de sequência no
 * casamento (o `m_seq` descendo o corpo da repetição).
 *
 * Os valores são calibrados pela BORDA MEDIDA com o menor `ulimit -s` que a
 * linguagem promete (8 MB), com margem para o quadro de um binário `-O0` ou
 * sob ASan. Medido no 20.0.4: 10000 grupos aninhados compilavam e 12000 davam
 * `padrao aninhado demais` com 8 MB, e os 12000 compilavam com 64 MB — o
 * motor-1. O teto de 10000 põe a borda no mesmo lugar em qualquer pilha.
 *
 * O caso de crash que os exercita gera um nível acima do teto, e sobe o teto
 * junto. */
#define RX_PADRAO_PROF_MAX 10000
#define RX_SEQ_PROF_MAX    7000

/* ── árvore do padrão ────────────────────────────────────────────────────
 *
 *   Alt   := Seq ('|' Seq)*
 *   Seq   := Quant*
 *   Quant := Atomo ('*' | '+' | '?' | '{m,n}')? ('?' | '+')?
 *   Atomo := char | '.' | classe | '(' Alt ')' | '^' | '$' | ...
 *
 * Um Seq guarda os Quant num array; a alternância é uma lista de Seq. O
 * quantificador possessivo (`x*+`) vira, na leitura, o grupo atômico que ele
 * é no `re`: `(?>x*)`. */

typedef enum { A_CHAR, A_QUALQUER, A_CLASSE, A_GRUPO, A_BOL, A_EOL, A_BACKREF,
               A_WORDB,      /* \b (idx_grupo=0) e \B (idx_grupo=1, negado) */
               A_STARTA,     /* \A — início da string (ignora MULTILINE) */
               A_ENDZ,       /* \Z e \z — fim da string */
               A_LOOK,       /* lookahead/lookbehind — ver campos em Atomo */
               A_ATOMICO,    /* `(?>...)`: casa uma vez e não volta atrás nele */
               A_COND        /* `(?(n)sim|não)`: ramo 0 se o grupo n casou, 1 se não */
             } TipoAtomo;

/* As flags do `re`, inline: `(?i)` global ou `(?i:...)` com escopo. */
#define RX_I 1    /* IGNORECASE */
#define RX_M 2    /* MULTILINE  */
#define RX_S 4    /* DOTALL     */
#define RX_X 8    /* VERBOSE: branco e `#comentário` fora de classe não contam */
#define RX_A 16   /* ASCII: `\w \W \b \B \d \D \s \S` e a caixa só no ASCII */
#define RX_U 32   /* UNICODE: é o normal; existe pra conferir `a` x `u` */
#define RX_L 64   /* LOCALE: só vale em padrão de bytes, aqui é erro */
#define RX_TIPO (RX_A | RX_U | RX_L)

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
    unsigned int  props;          /* P_*: propriedade do Unicode acima de 127 (`\w \s \d`) */
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
    int          cond_grupo;      /* A_COND: o grupo cuja presença escolhe o ramo */
    int          g_de, g_ate;     /* A_ATOMICO: os grupos de dentro, g_de < g <= g_ate */
    int          flags;           /* flags efetivas NESTE átomo (RX_*), carimbadas
                                   * no parse — é o que faz `(?i:...)` com escopo
                                   * funcionar no matcher de continuação */
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
    Alt   *raiz;
    int    ngrupos;
    int    flags;      /* as globais (RX_*) */
    char **nomes;      /* [0..ngrupos]: nome do grupo g, ou NULL */
    int   *cap_ini;    /* [0..ngrupos]: as capturas da última busca (RxCaptura) */
    int   *cap_fim;
};

/* ── construção ──────────────────────────────────────────────────────────── */

typedef struct {
    const char *p;
    int         n, i;
    int         ngrupos;
    char       *erro;
    int         erro_cap;
    int         falhou;
    int         flags;        /* as flags valendo no ponto da leitura (RX_*) */
    int         no_inicio;    /* nada lido ainda: só aqui cabe a flag global */
    int         ult;          /* o item anterior da sequência (ULT_*): decide
                               * entre "nothing to repeat" e "multiple repeat" */
    int         look_grupos;  /* grupos que existiam ao entrar no lookbehind; -1 fora */
    int         cap_g;        /* tamanho dos três vetores por grupo */
    char      **nomes;        /* nome de cada grupo (1..ngrupos), ou NULL */
    unsigned char *aberto;    /* o grupo ainda não fechou */
    int        *larg;         /* largura fixa do grupo fechado; -1 variável */
    int        *cond_ref;     /* condicional que cita grupo ainda não aberto: */
    int        *cond_pos;     /* o número e onde ele foi escrito, conferidos no fim */
    int         ncond, cap_cond;
    int         prof;         /* profundidade de grupo ANINHADO no padrão. O teto
                               * é contado (RX_PADRAO_PROF_MAX), além da folga
                               * da pilha: é o que faz o MESMO padrão dar o mesmo
                               * resultado em qualquer `ulimit -s` (motor-1). */
} Leitor;

/* O que veio antes, na sequência que está sendo lida. */
#define ULT_NADA   0
#define ULT_ATOMO  1
#define ULT_REPETE 2   /* item já repetido: outro quantificador é "multiple repeat" */
#define ULT_ANCORA 3   /* âncora não se repete: "nothing to repeat", como no `re` */

/* Falha interna (so as de memoria usam esta): nao e erro de sintaxe do
 * padrao, entao sai no formato proprio `regex: <msg> (posicao N)`. */
static void rerro(Leitor *l, const char *msg)
{
    int car = 0;                                 /* a posição em CARACTERE */
    for (int k = 0; k < l->i && k < l->n; k++)
        if (((unsigned char)l->p[k] & 0xC0) != 0x80) car++;
    if (!l->falhou && l->erro && l->erro_cap > 0)
        snprintf(l->erro, (size_t)l->erro_cap, "regex: %s (posicao %d)", msg, car);
    l->falhou = 1;
}

/* Erro de sintaxe do padrao. A mensagem e o sufixo " at position N" sao os
 * que o teste diferencial compara entre os dois motores — e a posicao e o
 * INICIO do construto culpado, nao onde o cursor parou. A posicao conta
 * CARACTERE, como o indice do texto do padrao: em byte, `"ção[a-"` dizia
 * "position 5" com o '[' no indice 3. */
static void rerro_em(Leitor *l, int pos, const char *msg)
{
    int car = 0;
    for (int k = 0; k < pos && k < l->n; k++)
        if (((unsigned char)l->p[k] & 0xC0) != 0x80) car++;
    if (!l->falhou && l->erro && l->erro_cap > 0)
        snprintf(l->erro, (size_t)l->erro_cap, "%s at position %d", msg, car);
    l->falhou = 1;
}

/* As poucas mensagens de sintaxe que NAO trazem posicao. */
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

/* Propriedades do Unicode acima de 127, que as classes `\w \s \d` carregam no
 * lugar de faixas: a de letra tem centenas de faixas, e materializá-las a cada
 * padrão compilado custava mais que perguntar à tabela (vm/ps_unicode.c, a
 * mesma do `isalnum`, do `isspace` e do `isdecimal` do texto). O ASCII delas
 * continua no bitmap. Cada uma tem a negada: é como `[\W]` e `[a\S]` entram
 * numa classe. Antes o `\w` punha [128, 0x10FFFF] inteiro (`€`, `—`, aspas e
 * reticências eram "palavra") e o `\s` só via o branco ASCII (o NBSP não). */
#define P_PALAVRA      0x01u   /* letra ou número: a régua do `isalnum` */
#define P_NAO_PALAVRA  0x02u
#define P_BRANCO       0x04u   /* White_Space: a régua do `isspace` */
#define P_NAO_BRANCO   0x08u
#define P_DIGITO       0x10u   /* dígito decimal (Nd): a régua do `isdecimal` */
#define P_NAO_DIGITO   0x20u

static int rx_palavra_unicode(unsigned int cp)
{
    return ps_uni_eh_letra(cp) || ps_uni_eh_numerico(cp);
}

static int props_casa(unsigned int props, unsigned int cp)
{
    if (props & (P_PALAVRA | P_NAO_PALAVRA)) {
        int w = rx_palavra_unicode(cp);
        if (((props & P_PALAVRA) && w) || ((props & P_NAO_PALAVRA) && !w)) return 1;
    }
    if (props & (P_BRANCO | P_NAO_BRANCO)) {
        int b = ps_uni_eh_branco(cp);
        if (((props & P_BRANCO) && b) || ((props & P_NAO_BRANCO) && !b)) return 1;
    }
    if (props & (P_DIGITO | P_NAO_DIGITO)) {
        int d = ps_uni_eh_decimal(cp);
        if (((props & P_DIGITO) && d) || ((props & P_NAO_DIGITO) && !d)) return 1;
    }
    return 0;
}

/* Pertence ao conjunto ESCRITO, sem a negação `[^...]` — quem casa a classe
 * aplica a negação depois de olhar as duas caixas (ver A_CLASSE). */
static int classe_dentro(const Classe *cl, unsigned int cp)
{
    if (tem_bit(cl->mapa, cp)) return 1;
    for (int i = 0; i < cl->nfaixas; i++)
        if (cp >= cl->faixas[i * 2] && cp <= cl->faixas[i * 2 + 1]) return 1;
    return cp >= 128 && cl->props && props_casa(cl->props, cp);
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
static int classe_escape(unsigned char e, Classe *cl, int ascii)
{
    unsigned char letra = e;
    if (e == 'D' || e == 'W' || e == 'S') { letra = (unsigned char)(e + 32); cl->negado = 1; }
    /* Com a flag ASCII (`(?a)`) a classe é só a do ASCII, como no `re`; sem
     * ela, a propriedade do Unicode cobre o resto (`props`). */
    if (letra == 'd') {
        for (unsigned c = '0'; c <= '9'; c++) classe_add(cl, c);
        if (!ascii) cl->props |= P_DIGITO;     /* e os dígitos das outras escritas */
    } else if (letra == 'w') {
        for (unsigned c = '0'; c <= '9'; c++) classe_add(cl, c);
        for (unsigned c = 'a'; c <= 'z'; c++) classe_add(cl, c);
        for (unsigned c = 'A'; c <= 'Z'; c++) classe_add(cl, c);
        classe_add(cl, '_');
        if (!ascii) cl->props |= P_PALAVRA;    /* fora do ASCII, letra ou número */
    } else if (letra == 's') {
        classe_add(cl, ' '); classe_add(cl, '\t'); classe_add(cl, '\n');
        classe_add(cl, '\r'); classe_add(cl, '\f'); classe_add(cl, '\v');
        if (!ascii) {
            /* os separadores de arquivo, grupo, registro e unidade (0x1C a
             * 0x1F) são branco pro `isspace` do Unicode, e o `\s` do `re`
             * segue o `isspace` */
            for (unsigned c = 0x1C; c <= 0x1F; c++) classe_add(cl, c);
            cl->props |= P_BRANCO;             /* NBSP, U+2000..U+200A, U+3000... */
        }
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

    /* acima de 127, o complemento de uma propriedade é a propriedade negada
     * (o `sub` de um escape não tem faixa: só o ASCII e a propriedade) */
    if (sub->props) {
        unsigned int p = 0;
        if (sub->props & P_PALAVRA)     p |= P_NAO_PALAVRA;
        if (sub->props & P_NAO_PALAVRA) p |= P_PALAVRA;
        if (sub->props & P_BRANCO)      p |= P_NAO_BRANCO;
        if (sub->props & P_NAO_BRANCO)  p |= P_BRANCO;
        if (sub->props & P_DIGITO)      p |= P_NAO_DIGITO;
        if (sub->props & P_NAO_DIGITO)  p |= P_DIGITO;
        cl->props |= p;
        return 0;
    }

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

static int eh_letra_ascii(unsigned int c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int hexa_valor(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* O escape de UM caractere, com a letra `e` já lida e a barra em `p_barra`:
 * `\a \f \n \r \t \v \\`, `\b` (só dentro de classe, onde é o backspace),
 * `\xHH`, `\uHHHH`, `\UHHHHHHHH` e octal (`\0` mais até dois dígitos; dentro
 * de classe, todo escape numérico é octal). Devolve 1 com o codepoint em `*cp`,
 * 0 se `e` não é escape de caractere (quem chama decide) e -1 com o erro já
 * dito. As frases e as posições (a da barra) são as do `re`. */
static int le_escape_char(Leitor *l, unsigned char e, int p_barra, int na_classe, unsigned int *cp)
{
    char msgb[96];
    switch (e) {
        case 'a':  *cp = '\a'; return 1;
        case 'f':  *cp = '\f'; return 1;
        case 'n':  *cp = '\n'; return 1;
        case 'r':  *cp = '\r'; return 1;
        case 't':  *cp = '\t'; return 1;
        case 'v':  *cp = '\v'; return 1;
        case '\\': *cp = '\\'; return 1;
        case 'b':  if (na_classe) { *cp = '\b'; return 1; } return 0;
        default:   break;
    }
    if (e == 'x' || e == 'u' || e == 'U') {
        int quer = (e == 'x') ? 2 : (e == 'u') ? 4 : 8;
        int ini = l->i, k = 0;
        unsigned long v = 0;
        while (k < quer && l->i < l->n && hexa_valor(l->p[l->i]) >= 0) {
            v = v * 16 + (unsigned long)hexa_valor(l->p[l->i]);
            l->i++; k++;
        }
        if (k != quer) {
            snprintf(msgb, sizeof(msgb), "incomplete escape \\%c%.*s", e, k, l->p + ini);
            rerro_em(l, p_barra, msgb);
            return -1;
        }
        if (v > 0x10FFFF) {
            snprintf(msgb, sizeof(msgb), "bad escape \\%c%.*s", e, k, l->p + ini);
            rerro_em(l, p_barra, msgb);
            return -1;
        }
        *cp = (unsigned int)v;
        return 1;
    }
    if (e == '0' || (na_classe && e >= '1' && e <= '7')) {
        int ini = l->i - 1, k = 1;
        unsigned int v = (unsigned int)(e - '0');
        while (k < 3 && l->i < l->n && l->p[l->i] >= '0' && l->p[l->i] <= '7') {
            v = v * 8 + (unsigned int)(l->p[l->i] - '0');
            l->i++; k++;
        }
        if (v > 0377) {
            snprintf(msgb, sizeof(msgb), "octal escape value \\%.*s outside of range 0-0o377",
                     k, l->p + ini);
            rerro_em(l, p_barra, msgb);
            return -1;
        }
        *cp = v;
        return 1;
    }
    return 0;
}

/* Um item de `[...]`: um caractere (devolve 1, com ele em `*cp`) ou um escape
 * de classe (`\d \w \s` e negações), que já sai unido em `cl` (devolve 2).
 * 0 = erro dito. Dentro da classe o `re` só aceita escape que vale UM
 * caractere ou uma classe: `\A`, `\z`, `\B`, `\8` e letra sem sentido são
 * "bad escape" (antes viravam a letra, calados). */
static int le_item_classe(Leitor *l, Classe *cl, unsigned int *cp, int ascii)
{
    if (l->p[l->i] != '\\') {
        int usados = cp_le(l->p, l->n, l->i, cp);
        l->i += usados ? usados : 1;
        return 1;
    }
    int p_barra = l->i;
    l->i++;
    if (l->i >= l->n) { rerro_em(l, p_barra, "bad escape (end of pattern)"); return 0; }
    unsigned char e = (unsigned char)l->p[l->i];
    if (e >= 0x80) {                          /* caractere não-ASCII escapado: ele mesmo */
        int usados = cp_le(l->p, l->n, l->i, cp);
        l->i += usados ? usados : 1;
        return 1;
    }
    l->i++;
    int r = le_escape_char(l, e, p_barra, 1, cp);
    if (r < 0) return 0;
    if (r > 0) return 1;
    Classe sub;
    memset(&sub, 0, sizeof(sub));
    if (classe_escape(e, &sub, ascii)) {
        int erro = 0;
        if (sub.negado) {
            /* `[a\D]` é "a ou não-dígito": a negação se resolve AQUI,
             * porque o `negado` da classe vale pro conjunto inteiro. */
            erro = classe_uniao_negada(cl, &sub) != 0;
        } else {
            for (int k = 0; k < 16; k++) cl->mapa[k] |= sub.mapa[k];
            for (int k = 0; k < sub.nfaixas && !erro; k++)
                erro = faixa_add(cl, sub.faixas[k * 2], sub.faixas[k * 2 + 1]) != 0;
            cl->props |= sub.props;
        }
        free(sub.faixas);
        if (erro) { rerro(l, "sem memoria na classe"); return 0; }
        return 2;
    }
    if (eh_letra_ascii(e) || (e >= '0' && e <= '9')) {
        char msgb[32];
        snprintf(msgb, sizeof(msgb), "bad escape \\%c", e);
        rerro_em(l, p_barra, msgb);
        return 0;
    }
    *cp = e;                                  /* pontuação escapada: ela mesma */
    return 1;
}

static void le_classe(Leitor *l, Atomo *a)
{
    a->tipo = A_CLASSE;
    memset(&a->classe, 0, sizeof(a->classe));
    Classe *cl = &a->classe;
    int ascii = l->flags & RX_A;
    int p_abre = l->i;                        /* posicao do '[': e AQUI que o erro aponta */
    l->i++;                                   /* passa '[' */
    if (l->i < l->n && l->p[l->i] == '^') { cl->negado = 1; l->i++; }
    int primeiro = 1;
    while (l->i < l->n && (l->p[l->i] != ']' || primeiro)) {
        primeiro = 0;
        int c_ini = l->i;   /* inicio deste item: erro de FAIXA e acusado a partir daqui */
        unsigned int c = 0;
        int r = le_item_classe(l, cl, &c, ascii);
        if (r == 0) return;
        /* faixa `a-z`; um '-' antes do ']' é literal */
        if (l->i + 1 < l->n && l->p[l->i] == '-' && l->p[l->i + 1] != ']') {
            l->i++;
            unsigned int fim = 0;
            Classe lixo;
            memset(&lixo, 0, sizeof(lixo));
            int r2 = le_item_classe(l, &lixo, &fim, ascii);
            free(lixo.faixas);
            if (r2 == 0) return;
            /* uma ponta que é classe (`[\w-a]`) não faz faixa, como no `re` */
            if (r == 2 || r2 == 2 || fim < c) {
                char msgb[96];
                snprintf(msgb, sizeof(msgb), "bad character range %.*s",
                         l->i - c_ini, l->p + c_ini);
                rerro_em(l, c_ini, msgb);
                return;
            }
            if (c < 128 && fim < 128) for (unsigned k = c; k <= fim; k++) mapa_bit(cl->mapa, k);
            else if (faixa_add(cl, c, fim) != 0) { rerro(l, "sem memoria na classe"); return; }
        } else if (r == 1) {
            if (classe_add(cl, c) != 0) { rerro(l, "sem memoria na classe"); return; }
        }
    }
    if (l->i >= l->n) { rerro_em(l, p_abre, "unterminated character set"); return; }
    l->i++;                                   /* passa ']' */
}

static int  larg_fixa_alt(const Leitor *l, const Alt *a);   /* largura fixa em codepoints, -1 = variável */
static int  le_atomo(Leitor *l, Atomo *a);
static void le_seq(Leitor *l, Seq *s);

/* O que não é item: o branco e o `#comentário` do modo VERBOSE (fora de
 * classe) e o comentário `(?#...)`. Vale também entre um átomo e o
 * quantificador dele: `a(?#x)*` é `a*`, como no `re`. */
static void pula_nada(Leitor *l)
{
    for (;;) {
        if (l->flags & RX_X) {
            while (l->i < l->n) {
                char c = l->p[l->i];
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f') {
                    l->i++;
                } else if (c == '#') {
                    while (l->i < l->n && l->p[l->i] != '\n') l->i++;
                } else {
                    break;
                }
            }
        }
        if (l->i + 2 < l->n && l->p[l->i] == '(' && l->p[l->i + 1] == '?' && l->p[l->i + 2] == '#') {
            int p_abre = l->i;
            l->i += 3;
            while (l->i < l->n && l->p[l->i] != ')') l->i++;
            if (l->i >= l->n) { rerro_em(l, p_abre, "missing ), unterminated comment"); return; }
            l->i++;
            continue;
        }
        return;
    }
}

/* Abre o grupo seguinte. Os vetores por grupo crescem com ele: o `re` não
 * tem teto de grupos (o daqui era 32). Devolve o número, ou -1 sem memória. */
static int grupo_novo(Leitor *l)
{
    int g = l->ngrupos + 1;
    if (g + 1 > l->cap_g) {
        int novo = l->cap_g ? l->cap_g * 2 : 16;
        while (novo < g + 1) novo *= 2;
        char **nn = realloc(l->nomes, sizeof(char *) * (size_t)novo);
        if (!nn) return -1;
        l->nomes = nn;
        unsigned char *na = realloc(l->aberto, (size_t)novo);
        if (!na) return -1;
        l->aberto = na;
        int *nl = realloc(l->larg, sizeof(int) * (size_t)novo);
        if (!nl) return -1;
        l->larg = nl;
        for (int k = l->cap_g; k < novo; k++) { l->nomes[k] = NULL; l->aberto[k] = 0; l->larg[k] = -1; }
        l->cap_g = novo;
    }
    l->ngrupos = g;
    l->aberto[g] = 1;
    return g;
}

/* A letra no byte `i` (`str.isalpha` do `re`), fora do ASCII pela tabela. */
static int letra_em(const Leitor *l, int i)
{
    unsigned int cp;
    if (!cp_le(l->p, l->n, i, &cp)) return 0;
    return cp < 128 ? eh_letra_ascii(cp) : ps_uni_eh_letra(cp);
}

/* Nome de grupo do `re` (`str.isidentifier`): letra ou '_' no começo, depois
 * letra, dígito ou '_'; fora do ASCII pela tabela do Unicode. */
static int nome_valido(const char *s, int len)
{
    if (len <= 0) return 0;
    for (int i = 0, primeiro = 1; i < len; primeiro = 0) {
        unsigned int cp;
        int u = cp_le(s, len, i, &cp);
        if (!u) return 0;
        int letra = cp < 128 ? (eh_letra_ascii(cp) || cp == '_') : ps_uni_eh_letra(cp);
        int digito = cp < 128 ? (cp >= '0' && cp <= '9') : ps_uni_eh_decimal(cp);
        if (!letra && (primeiro || !digito)) return 0;
        i += u;
    }
    return 1;
}

static int grupo_por_nome_l(const Leitor *l, const char *nome, int len)
{
    for (int g = 1; g <= l->ngrupos; g++)
        if (l->nomes[g] && (int)strlen(l->nomes[g]) == len && !memcmp(l->nomes[g], nome, (size_t)len))
            return g;
    return -1;
}

/* O `getuntil` do `re`: lê o nome até `fim` e deixa `l->i` depois dele.
 * Devolve onde o nome começa (o tamanho em `*len`), ou -1 com o erro dito. */
static int le_nome_ate(Leitor *l, char fim, int *len)
{
    int n0 = l->i;
    while (l->i < l->n && l->p[l->i] != fim) l->i++;
    if (l->i >= l->n) {
        if (l->i == n0) {
            rerro_em(l, l->n, "missing group name");
        } else {
            char msgb[48];
            snprintf(msgb, sizeof(msgb), "missing %c, unterminated name", fim);
            rerro_em(l, n0, msgb);
        }
        return -1;
    }
    *len = l->i - n0;
    l->i++;                                   /* passa o terminador */
    if (*len == 0) { rerro_em(l, n0, "missing group name"); return -1; }
    return n0;
}

/* Frase com o nome entre aspas, na posição dele (`unknown group name 'x'`). */
static int erro_com_nome(Leitor *l, const char *frase, int n0, int len)
{
    char msgb[200];
    snprintf(msgb, sizeof(msgb), "%s '%.*s'", frase, len > 120 ? 120 : len, l->p + n0);
    rerro_em(l, n0, msgb);
    return 0;
}

/* Retrovisor dentro de lookbehind: o grupo tem que estar fechado e ter vindo
 * antes do lookbehind (a regra e as frases do `re`). Fora dele, nada. */
static int confere_ref_atras(Leitor *l, int g)
{
    if (l->look_grupos < 0) return 1;
    if (g > l->ngrupos || l->aberto[g]) { rerro_em(l, l->i, "cannot refer to an open group"); return 0; }
    if (g > l->look_grupos) {
        rerro_em(l, l->i, "cannot refer to group defined in the same lookbehind subpattern");
        return 0;
    }
    return 1;
}

static int flag_de(char c)
{
    switch (c) {
        case 'i': return RX_I;
        case 'm': return RX_M;
        case 's': return RX_S;
        case 'x': return RX_X;
        case 'a': return RX_A;
        case 'u': return RX_U;
        case 'L': return RX_L;
        default:  return 0;
    }
}

/* As flags de `(?aiLmsux)` e `(?aiLmsux-imsx:...)`, com `l->i` na primeira
 * letra (ou no '-'). Devolve 1 se é global (fechou em ')'), 2 se tem escopo
 * (o ':' já passou) e 0 com o erro dito. A regra, as frases e as posições
 * são as do `_parse_flags` do `re`. */
static int le_flags(Leitor *l, int *add, int *del)
{
    *add = *del = 0;
    char c = l->p[l->i];
    if (c != '-') {
        for (;;) {
            int f = flag_de(c);
            l->i++;
            if (f == RX_L) {
                rerro_em(l, l->i, "bad inline flags: cannot use 'L' flag with a str pattern");
                return 0;
            }
            *add |= f;
            if ((f & RX_TIPO) && (*add & RX_TIPO) != f) {
                rerro_em(l, l->i, "bad inline flags: flags 'a', 'u' and 'L' are incompatible");
                return 0;
            }
            if (l->i >= l->n) { rerro_em(l, l->i, "missing -, : or )"); return 0; }
            c = l->p[l->i];
            if (c == ')' || c == '-' || c == ':') break;
            if (!flag_de(c)) {
                rerro_em(l, l->i, letra_em(l, l->i) ? "unknown flag" : "missing -, : or )");
                return 0;
            }
        }
        if (c == ')') { l->i++; return 1; }
    }
    if (c == '-') {
        l->i++;
        if (l->i >= l->n) { rerro_em(l, l->i, "missing flag"); return 0; }
        c = l->p[l->i];
        if (!flag_de(c)) {
            rerro_em(l, l->i, letra_em(l, l->i) ? "unknown flag" : "missing flag");
            return 0;
        }
        for (;;) {
            int f = flag_de(c);
            l->i++;
            if (f & RX_TIPO) {
                rerro_em(l, l->i, "bad inline flags: cannot turn off flags 'a', 'u' and 'L'");
                return 0;
            }
            *del |= f;
            if (l->i >= l->n) { rerro_em(l, l->i, "missing :"); return 0; }
            c = l->p[l->i];
            if (c == ':') break;
            if (!flag_de(c)) {
                rerro_em(l, l->i, letra_em(l, l->i) ? "unknown flag" : "missing :");
                return 0;
            }
        }
    }
    l->i++;                                   /* passa o ':' */
    if (*add & *del) { rerro_em(l, l->i - 1, "bad inline flags: flag turned on and off"); return 0; }
    return 2;
}

/* O MAXREPEAT do `re`: o número de uma repetição vai até 4294967294. */
#define RX_MAXREPEAT 4294967295LL

/* `{m}`, `{m,}`, `{,n}`, `{m,n}` e `{,}`, com o '{' em `j`, pela regra do
 * `re`. Devolve o índice depois do '}', com o mínimo e o máximo (-1 = sem
 * teto); -1 se não é repetição (o '{' é literal: `{}`, `{x}`, `{1`); -2 com o
 * erro dito. Antes `{,n}` e `{,}` eram literais, calados. */
static int le_chaves(Leitor *l, int j, long long *min, long long *max)
{
    int k = j + 1;
    if (k < l->n && l->p[k] == '}') return -1;
    long long lo = 0, hi = -1;
    int temlo = 0, temhi = 0;
    while (k < l->n && l->p[k] >= '0' && l->p[k] <= '9') {
        if (lo < RX_MAXREPEAT) lo = lo * 10 + (l->p[k] - '0');
        temlo = 1; k++;
    }
    if (k < l->n && l->p[k] == ',') {
        k++;
        long long h = 0;
        while (k < l->n && l->p[k] >= '0' && l->p[k] <= '9') {
            if (h < RX_MAXREPEAT) h = h * 10 + (l->p[k] - '0');
            temhi = 1; k++;
        }
        if (temhi) hi = h;
    } else {
        if (!temlo) return -1;
        hi = lo; temhi = 1;
    }
    if (k >= l->n || l->p[k] != '}') return -1;
    /* COM TETO: `a{4294967295}` transbordava o `int` e casava o vazio. A
     * frase não leva posição, como no `re` (lá é OverflowError). */
    if ((temlo && lo >= RX_MAXREPEAT) || (temhi && hi >= RX_MAXREPEAT)) {
        rerro_txt(l, "the repetition number is too large");
        return -2;
    }
    if (hi >= 0 && hi < lo) {
        rerro_em(l, j + 1, "min repeat greater than max repeat");
        return -2;
    }
    *min = lo; *max = hi;
    return k + 1;
}

/* Quantificador sem alvo: no começo, depois de uma âncora (que não se repete)
 * ou em cima de outro quantificador. São as duas frases do `re`. */
static void erro_repete(Leitor *l, int pos)
{
    rerro_em(l, pos, l->ult == ULT_REPETE ? "multiple repeat" : "nothing to repeat");
}

/* Condicional que cita um grupo que ainda não abriu: vale se ele existir no
 * fim do padrão, e é conferido lá. */
static int cond_anota(Leitor *l, int g, int pos)
{
    if (l->ncond + 1 > l->cap_cond) {
        int novo = l->cap_cond ? l->cap_cond * 2 : 4;
        int *nr = realloc(l->cond_ref, sizeof(int) * (size_t)novo);
        if (!nr) return -1;
        l->cond_ref = nr;
        int *np = realloc(l->cond_pos, sizeof(int) * (size_t)novo);
        if (!np) return -1;
        l->cond_pos = np;
        l->cap_cond = novo;
    }
    l->cond_ref[l->ncond] = g;
    l->cond_pos[l->ncond] = pos;
    l->ncond++;
    return 0;
}

/* `\1`..`\99` (com a primeira já lida em `e`): retrovisor, ou o caractere de
 * três dígitos octais (`\101` é "A"), pela regra do `re`. Antes só havia
 * `\1`..`\9`: `\10` era o grupo 1 seguido de "0", calado. */
static int le_ref_numero(Leitor *l, Atomo *a, unsigned char e, int p_barra)
{
    char msgb[96];
    int g = e - '0';
    if (l->i < l->n && l->p[l->i] >= '0' && l->p[l->i] <= '9') {
        unsigned char d2 = (unsigned char)l->p[l->i];
        l->i++;
        if (e <= '7' && d2 <= '7' && l->i < l->n && l->p[l->i] >= '0' && l->p[l->i] <= '7') {
            unsigned int v = (unsigned int)((e - '0') * 64 + (d2 - '0') * 8 + (l->p[l->i] - '0'));
            l->i++;
            if (v > 0377) {
                snprintf(msgb, sizeof(msgb), "octal escape value \\%.3s outside of range 0-0o377",
                         l->p + p_barra + 1);
                rerro_em(l, p_barra, msgb);
                return 0;
            }
            a->tipo = A_CHAR; a->cp = v;
            return 1;
        }
        g = g * 10 + (d2 - '0');
    }
    if (g > l->ngrupos) {
        snprintf(msgb, sizeof(msgb), "invalid group reference %d", g);
        rerro_em(l, p_barra + 1, msgb);       /* a posicao e a do DIGITO, nao a da barra */
        return 0;
    }
    if (l->aberto[g]) { rerro_em(l, p_barra, "cannot refer to an open group"); return 0; }
    if (!confere_ref_atras(l, g)) return 0;
    a->tipo = A_BACKREF;
    a->idx_grupo = g;
    return 1;
}

/* `(?P=nome)`: o retrovisor pelo nome. */
static int le_ref_nome(Leitor *l, Atomo *a)
{
    int len;
    int n0 = le_nome_ate(l, ')', &len);
    if (n0 < 0) return 0;
    if (!nome_valido(l->p + n0, len)) return erro_com_nome(l, "bad character in group name", n0, len);
    int g = grupo_por_nome_l(l, l->p + n0, len);
    if (g < 0) return erro_com_nome(l, "unknown group name", n0, len);
    if (l->aberto[g]) { rerro_em(l, n0, "cannot refer to an open group"); return 0; }
    if (!confere_ref_atras(l, g)) return 0;
    l->no_inicio = 0;
    a->tipo = A_BACKREF;
    a->idx_grupo = g;
    return 1;
}

/* `(?(n)sim|não)` e `(?(nome)sim|não)`, com `l->i` depois do segundo '('. Os
 * dois ramos são o `Alt` do átomo: o 0 vale se o grupo casou, o 1 (vazio se
 * não foi escrito) se não. */
static int le_condicional(Leitor *l, Atomo *a, int p_abre)
{
    int len;
    int n0 = le_nome_ate(l, ')', &len);
    if (n0 < 0) return 0;
    const char *nome = l->p + n0;
    int g;
    if (nome_valido(nome, len)) {
        g = grupo_por_nome_l(l, nome, len);
        if (g < 0) return erro_com_nome(l, "unknown group name", n0, len);
    } else {
        long long v = 0;
        for (int k = 0; k < len; k++) {
            if (nome[k] < '0' || nome[k] > '9') return erro_com_nome(l, "bad character in group name", n0, len);
            if (v < RX_MAXREPEAT) v = v * 10 + (nome[k] - '0');
        }
        if (v == 0) { rerro_em(l, n0, "bad group number"); return 0; }
        if (v >= 1073741823LL) {              /* o MAXGROUPS do `re` */
            char msgb[96];
            snprintf(msgb, sizeof(msgb), "invalid group reference %lld", v);
            rerro_em(l, n0, msgb);
            return 0;
        }
        g = (int)v;
        if (g > l->ngrupos && cond_anota(l, g, n0) != 0) { rerro(l, "sem memoria"); return 0; }
    }
    if (!confere_ref_atras(l, g)) return 0;
    l->no_inicio = 0;
    Alt *alt = calloc(1, sizeof(Alt));
    if (!alt) { rerro(l, "sem memoria"); return 0; }
    alt->ramos = calloc(2, sizeof(Seq));
    if (!alt->ramos) { free(alt); rerro(l, "sem memoria"); return 0; }
    alt->n = alt->cap = 2;
    le_seq(l, &alt->ramos[0]);
    if (!l->falhou && l->i < l->n && l->p[l->i] == '|') {
        l->i++;
        le_seq(l, &alt->ramos[1]);
        if (!l->falhou && l->i < l->n && l->p[l->i] == '|')
            rerro_em(l, l->i, "conditional backref with more than two branches");
    }
    if (!l->falhou && (l->i >= l->n || l->p[l->i] != ')'))
        rerro_em(l, p_abre, "missing ), unterminated subpattern");
    if (l->falhou) { libera_alt(alt); return 0; }
    l->i++;
    a->tipo = A_COND;
    a->cond_grupo = g;
    a->grupo = alt;
    return 1;
}

/* A barra fora de classe: classe (`\d`), âncora (`\A \b \B \Z \z`),
 * caractere (`\n`, `\x41`, `é`, octal, pontuação), retrovisor (`\1`). */
static int le_escape_fora(Leitor *l, Atomo *a)
{
    int p_barra = l->i;
    l->i++;
    if (l->i >= l->n) { rerro_em(l, p_barra, "bad escape (end of pattern)"); return 0; }
    unsigned char e = (unsigned char)l->p[l->i];
    if (e >= 0x80) {
        /* caractere não-ASCII escapado é ele mesmo, INTEIRO (antes o primeiro
         * byte virava um caractere e o resto, outro) */
        unsigned int cp;
        int u = cp_le(l->p, l->n, l->i, &cp);
        l->i += u ? u : 1;
        a->tipo = A_CHAR;
        a->cp = u ? cp : e;
        return 1;
    }
    l->i++;
    memset(&a->classe, 0, sizeof(a->classe));
    if (classe_escape(e, &a->classe, l->flags & RX_A)) { a->tipo = A_CLASSE; return 1; }
    if (e == 'A') { a->tipo = A_STARTA; return 1; }
    if (e == 'Z' || e == 'z') { a->tipo = A_ENDZ; return 1; }
    if (e == 'b' || e == 'B') { a->tipo = A_WORDB; a->idx_grupo = (e == 'B'); return 1; }
    unsigned int cp;
    int r = le_escape_char(l, e, p_barra, 0, &cp);
    if (r < 0) return 0;
    if (r > 0) { a->tipo = A_CHAR; a->cp = cp; return 1; }
    if (e >= '1' && e <= '9') return le_ref_numero(l, a, e, p_barra);
    if (eh_letra_ascii(e)) {
        char msgb[32];
        snprintf(msgb, sizeof(msgb), "bad escape \\%c", e);
        rerro_em(l, p_barra, msgb);           /* a posicao e a da BARRA */
        return 0;
    }
    a->tipo = A_CHAR;                         /* pontuação escapada: ela mesma */
    a->cp = e;
    return 1;
}

/* `(` ... `)`: grupo capturante, `(?:...)`, nomeado, atômico, lookaround,
 * condicional e flags (o comentário `(?#...)` sai no `pula_nada`) — as
 * extensões do `re`, com as frases e as posições dele. */
static int le_grupo(Leitor *l, Atomo *a)
{
    int p_abre = l->i;                        /* posicao do '(': e AQUI que o erro aponta */
    l->i++;
    int captura = 1, eh_look = 0, lk_neg = 0, lk_atras = 0, atomico = 0;
    int fl_add = 0, fl_del = 0, com_flags = 0;
    int nome_ini = -1, nome_len = 0;
    char msgb[200];
    if (l->i < l->n && l->p[l->i] == '?') {
        l->i++;
        if (l->i >= l->n) { rerro_em(l, l->n, "unexpected end of pattern"); return 0; }
        char esp = l->p[l->i];
        if (esp == 'P') {
            l->i++;
            if (l->i < l->n && l->p[l->i] == '<') {
                l->i++;
                nome_ini = le_nome_ate(l, '>', &nome_len);
                if (nome_ini < 0) return 0;
            } else if (l->i < l->n && l->p[l->i] == '=') {
                l->i++;
                return le_ref_nome(l, a);
            } else {
                if (l->i >= l->n) { rerro_em(l, l->n, "unexpected end of pattern"); return 0; }
                /* a mensagem cita a extensao desconhecida: `?Pz` */
                snprintf(msgb, sizeof(msgb), "unknown extension ?P%c", l->p[l->i]);
                rerro_em(l, l->i - 2, msgb);
                return 0;
            }
        } else if (esp == ':') {
            captura = 0; l->i++;
        } else if (esp == '>') {
            captura = 0; atomico = 1; l->i++;
        } else if (esp == '=' || esp == '!') {
            eh_look = 1; lk_neg = (esp == '!'); captura = 0; l->i++;
        } else if (esp == '<') {
            l->i++;
            if (l->i >= l->n) { rerro_em(l, l->n, "unexpected end of pattern"); return 0; }
            char e2 = l->p[l->i];
            if (e2 == '=' || e2 == '!') {
                eh_look = 1; lk_atras = 1; lk_neg = (e2 == '!'); captura = 0; l->i++;
            } else {
                /* `(?<nome>...)`: a forma do Java, do .NET e do JavaScript (o
                 * `re` só tem `(?P<nome>...)`). O nome é GUARDADO, pra
                 * `\g<nome>` e `(?P=nome)` funcionarem. */
                nome_ini = le_nome_ate(l, '>', &nome_len);
                if (nome_ini < 0) return 0;
            }
        } else if (esp == '(') {
            l->i++;
            return le_condicional(l, a, p_abre);
        } else if (flag_de(esp) || esp == '-') {
            int tipo = le_flags(l, &fl_add, &fl_del);
            if (!tipo) return 0;
            if (tipo == 1) {
                /* flag global: só no começo do padrão, porque vale pro padrão
                 * INTEIRO (o `re` recusa no meio desde o 3.11) */
                if (!l->no_inicio) {
                    rerro_em(l, p_abre, "global flags not at the start of the expression");
                    return 0;
                }
                l->flags |= fl_add;
                return le_atomo(l, a);        /* a flag não é item: segue pro próximo */
            }
            captura = 0; com_flags = 1;
        } else {
            /* a mensagem cita a extensao desconhecida: `?y` */
            snprintf(msgb, sizeof(msgb), "unknown extension ?%c", esp);
            rerro_em(l, l->i - 1, msgb);
            return 0;
        }
    }
    l->no_inicio = 0;
    if (nome_ini >= 0 && !nome_valido(l->p + nome_ini, nome_len))
        return erro_com_nome(l, "bad character in group name", nome_ini, nome_len);
    a->tipo = atomico ? A_ATOMICO : eh_look ? A_LOOK : A_GRUPO;
    a->look_neg = lk_neg; a->look_atras = lk_atras;
    int g = 0;
    if (captura) {
        if (nome_ini >= 0) {
            int ja = grupo_por_nome_l(l, l->p + nome_ini, nome_len);
            /* (o teto de aninhamento está no `le_atomo`, que é o ponto de
             * entrada recursivo: o grupo sempre passa por ele) */
            if (ja > 0) {
                snprintf(msgb, sizeof(msgb), "redefinition of group name '%.*s' as group %d; was group %d",
                         nome_len > 100 ? 100 : nome_len, l->p + nome_ini, l->ngrupos + 1, ja);
                rerro_em(l, nome_ini, msgb);
                return 0;
            }
        }
        g = grupo_novo(l);
        if (g < 0) { rerro(l, "sem memoria"); return 0; }
        a->idx_grupo = g;
        if (nome_ini >= 0) {
            char *nm = malloc((size_t)nome_len + 1);
            if (!nm) { rerro(l, "sem memoria"); return 0; }
            memcpy(nm, l->p + nome_ini, (size_t)nome_len);
            nm[nome_len] = '\0';
            l->nomes[g] = nm;
        }
    }
    int flags_salvo = l->flags, look_salvo = l->look_grupos;
    if (com_flags) {
        /* `a` e `u` trocam o modo do grupo; as outras somam e tiram */
        if (fl_add & RX_TIPO) l->flags &= ~RX_TIPO;
        l->flags = (l->flags | fl_add) & ~fl_del;
    }
    if (lk_atras && l->look_grupos < 0) l->look_grupos = l->ngrupos;
    a->g_de = l->ngrupos;
    l->prof++;                                /* o teto de aninhamento (motor-1) */
    a->grupo = le_alt(l);
    l->prof--;
    a->g_ate = l->ngrupos;
    l->flags = flags_salvo;                   /* flags de escopo saem do grupo */
    l->look_grupos = look_salvo;
    if (l->falhou) { libera_alt(a->grupo); a->grupo = NULL; return 0; }
    if (l->i >= l->n || l->p[l->i] != ')') {
        rerro_em(l, p_abre, "missing ), unterminated subpattern");
        libera_alt(a->grupo); a->grupo = NULL;
        return 0;
    }
    l->i++;
    if (captura) {
        l->aberto[g] = 0;
        l->larg[g] = larg_fixa_alt(l, a->grupo);
    }
    if (eh_look && lk_atras) {
        a->look_larg = larg_fixa_alt(l, a->grupo);
        if (a->look_larg < 0) {
            rerro_txt(l, "look-behind requires fixed-width pattern");
            libera_alt(a->grupo); a->grupo = NULL; return 0;
        }
    }
    return 1;
}

static int le_atomo(Leitor *l, Atomo *a)
{
    pula_nada(l);
    if (l->falhou || l->i >= l->n) return 0;
    /* O leitor e recursivo (`le_alt -> le_seq -> le_atomo -> le_alt`) e so o
     * grupo capturante tinha teto: `(?:` 20 mil vezes derrubava o processo.
     * Dois freios, como no parser da linguagem: o TETO CONTADO, que dá o mesmo
     * resultado em qualquer `ulimit -s` (motor-1), e a folga medida da pilha,
     * que cobre o quadro do binário de verdade (o do ASan é maior). */
    if (l->prof >= RX_PADRAO_PROF_MAX || ps_pilha_apertada()) {
        /* sem a posicao: ela e onde a PILHA apertou, que muda com o tamanho
         * da pilha (fibra, ASan), e nao diz nada sobre o padrao */
        if (!l->falhou && l->erro && l->erro_cap > 0)
            snprintf(l->erro, (size_t)l->erro_cap, "regex: padrao aninhado demais");
        l->falhou = 1;
        return 0;
    }
    char c = l->p[l->i];
    a->grupo = NULL;
    a->idx_grupo = -1;
    a->cond_grupo = 0;
    a->g_de = a->g_ate = 0;
    a->flags = l->flags;          /* carimba as flags efetivas neste átomo */
    a->look_neg = a->look_atras = a->look_larg = 0;
    if (c == '(') return le_grupo(l, a);
    if (c == ')' || c == '|') return 0;        /* fim deste Seq */
    if (c == '*' || c == '+' || c == '?') { erro_repete(l, l->i); return 0; }
    if (c == '{') {
        long long mn, mx;
        int fim = le_chaves(l, l->i, &mn, &mx);
        if (fim == -2) return 0;
        if (fim > 0) { erro_repete(l, l->i); return 0; }
        /* o '{' que não é repetição é literal, abaixo */
    }
    l->no_inicio = 0;
    if (c == '[') {
        le_classe(l, a);
        if (l->falhou) { free(a->classe.faixas); a->classe.faixas = NULL; return 0; }
        return 1;
    }
    if (c == '.') { l->i++; a->tipo = A_QUALQUER; return 1; }
    if (c == '^') { l->i++; a->tipo = A_BOL; return 1; }
    if (c == '$') { l->i++; a->tipo = A_EOL; return 1; }
    if (c == '\\') return le_escape_fora(l, a);
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

static void libera_atomo(Atomo *at)
{
    if (at->tipo == A_GRUPO || at->tipo == A_LOOK || at->tipo == A_ATOMICO || at->tipo == A_COND)
        libera_alt(at->grupo);
    else if (at->tipo == A_CLASSE)
        free(at->classe.faixas);
}

/* `x*+` (possessivo) é `(?>x*)` no `re`: o item repetido vira o corpo de um
 * grupo atômico, que fica no lugar dele. Os grupos de dentro são os que o
 * item abriu (g_de < g <= g_ate). */
static int embrulha_atomico(Quant *q, int g_de, int g_ate)
{
    Alt *alt = calloc(1, sizeof(Alt));
    if (!alt) return -1;
    alt->ramos = calloc(1, sizeof(Seq));
    if (!alt->ramos) { free(alt); return -1; }
    alt->ramos[0].itens = malloc(sizeof(Quant));
    if (!alt->ramos[0].itens) { free(alt->ramos); free(alt); return -1; }
    alt->n = alt->cap = 1;
    alt->ramos[0].itens[0] = *q;
    alt->ramos[0].n = alt->ramos[0].cap = 1;
    int flags = q->atomo.flags;
    memset(q, 0, sizeof(*q));
    q->atomo.tipo = A_ATOMICO;
    q->atomo.grupo = alt;
    q->atomo.idx_grupo = -1;
    q->atomo.flags = flags;
    q->atomo.g_de = g_de;
    q->atomo.g_ate = g_ate;
    q->min = q->max = 1;
    q->guloso = 1;
    return 0;
}

static void le_seq(Leitor *l, Seq *s)
{
    memset(s, 0, sizeof(*s));
    l->ult = ULT_NADA;
    for (;;) {
        Quant q;
        memset(&q, 0, sizeof(q));
        int g_antes = l->ngrupos;
        if (!le_atomo(l, &q.atomo)) break;
        q.min = 1; q.max = 1; q.guloso = 1;
        TipoAtomo t = q.atomo.tipo;
        /* âncora não se repete: o quantificador depois dela é "nothing to
         * repeat" (sai no `le_atomo` seguinte), como no `re` */
        int ancora = (t == A_BOL || t == A_EOL || t == A_STARTA || t == A_ENDZ || t == A_WORDB);
        /* `teve_quant`: um quantificador foi de fato consumido. É o que decide
         * se o `?` seguinte é o marcador PREGUIÇOSO (e o `+`, o possessivo).
         * A guarda antiga olhava `q.min != 1 || q.max != 1`, e `{1}` deixa os
         * dois em 1 — então `a{1}?` era recusado com "multiple repeat",
         * quando é só o `{1}` preguiçoso. */
        int teve_quant = 0;
        if (!ancora) {
            pula_nada(l);
            if (l->falhou) { libera_atomo(&q.atomo); return; }
            if (l->i < l->n) {
                char c = l->p[l->i];
                if (c == '*')      { q.min = 0; q.max = -1; l->i++; teve_quant = 1; }
                else if (c == '+') { q.min = 1; q.max = -1; l->i++; teve_quant = 1; }
                else if (c == '?') { q.min = 0; q.max = 1;  l->i++; teve_quant = 1; }
                else if (c == '{') {
                    long long mn, mx;
                    int fim = le_chaves(l, l->i, &mn, &mx);
                    if (fim == -2) { libera_atomo(&q.atomo); return; }
                    if (fim > 0) {
                        /* acima do tamanho máximo de um texto, o número dá na
                         * mesma: nenhum texto tem tantos caracteres */
                        q.min = mn > INT_MAX ? INT_MAX : (int)mn;
                        q.max = mx < 0 ? -1 : (mx > INT_MAX ? INT_MAX : (int)mx);
                        l->i = fim;
                        teve_quant = 1;
                    }
                }
                if (teve_quant && l->i < l->n) {
                    if (l->p[l->i] == '?') {
                        q.guloso = 0; l->i++;
                    } else if (l->p[l->i] == '+') {
                        l->i++;
                        if (embrulha_atomico(&q, g_antes, l->ngrupos) != 0) {
                            libera_atomo(&q.atomo);
                            rerro(l, "sem memoria");
                            return;
                        }
                    }
                }
            }
        }
        seq_push(l, s, q);
        if (l->falhou) { libera_atomo(&q.atomo); return; }
        l->ult = ancora ? ULT_ANCORA : teve_quant ? ULT_REPETE : ULT_ATOMO;
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
        if (l->i < l->n && l->p[l->i] == '|') { l->i++; l->no_inicio = 0; continue; }
        break;
    }
    return a;
}

static void libera_seq(Seq *s)
{
    for (int i = 0; i < s->n; i++) libera_atomo(&s->itens[i].atomo);
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
 * Char/classe/`.` valem 1 codepoint; âncoras 0; grupo recorre; o retrovisor
 * vale a largura do grupo dele, se ela é fixa (o `re` aceita desde o 3.5);
 * quantificador variável dá -1 -> lookbehind recusado na compilação. */
static int larg_fixa_atomo(const Leitor *l, const Atomo *a);
static int larg_fixa_alt(const Leitor *l, const Alt *a)
{
    int larg = -1;
    for (int r = 0; r < a->n; r++) {
        const Seq *s = &a->ramos[r];
        long long soma = 0;
        for (int i = 0; i < s->n; i++) {
            const Quant *q = &s->itens[i];
            if (q->min != q->max) return -1;
            int w = larg_fixa_atomo(l, &q->atomo);
            if (w < 0) return -1;
            soma += (long long)w * q->min;
            if (soma > INT_MAX) soma = INT_MAX;
        }
        if (larg < 0) larg = (int)soma;
        else if (larg != soma) return -1;
    }
    return larg < 0 ? 0 : larg;
}
static int larg_fixa_atomo(const Leitor *l, const Atomo *a)
{
    switch (a->tipo) {
        case A_CHAR: case A_CLASSE: case A_QUALQUER: return 1;   /* 1 codepoint */
        case A_BOL: case A_EOL: case A_WORDB: case A_STARTA: case A_ENDZ: case A_LOOK: return 0;
        case A_GRUPO: case A_ATOMICO: case A_COND: return larg_fixa_alt(l, a->grupo);
        case A_BACKREF:
            return (a->idx_grupo >= 1 && a->idx_grupo <= l->ngrupos && !l->aberto[a->idx_grupo])
                   ? l->larg[a->idx_grupo] : -1;
        default:       return -1;
    }
}

static void leitor_libera(Leitor *l, int com_nomes)
{
    if (com_nomes)
        for (int g = 1; g <= l->ngrupos; g++) free(l->nomes[g]);
    free(l->nomes);
    free(l->aberto);
    free(l->larg);
    free(l->cond_ref);
    free(l->cond_pos);
}

PSRegex *ps_regex_compila(const char *padrao, int len, char *erro, int erro_cap)
{
    Leitor l;
    memset(&l, 0, sizeof(l));
    l.p = padrao; l.n = len;
    l.erro = erro; l.erro_cap = erro_cap;
    l.no_inicio = 1;
    l.look_grupos = -1;
    Alt *raiz = le_alt(&l);
    if (!l.falhou && l.i != l.n) {
        /* sobrou ')' sem abrir, ou coisa parecida */
        rerro_em(&l, l.i, "unbalanced parenthesis");
    }
    /* o condicional que citou um grupo de depois: ele tem que existir */
    for (int k = 0; !l.falhou && k < l.ncond; k++) {
        if (l.cond_ref[k] > l.ngrupos) {
            char msgb[64];
            snprintf(msgb, sizeof(msgb), "invalid group reference %d", l.cond_ref[k]);
            rerro_em(&l, l.cond_pos[k], msgb);
        }
    }
    if (!l.falhou && (l.flags & RX_A) && (l.flags & RX_U))
        rerro_txt(&l, "ASCII and UNICODE flags are incompatible");
    PSRegex *r = NULL;
    if (!l.falhou) {
        size_t ng = (size_t)l.ngrupos + 1;
        r = calloc(1, sizeof(PSRegex));
        if (r) {
            r->nomes = calloc(ng, sizeof(char *));
            r->cap_ini = malloc(sizeof(int) * ng);
            r->cap_fim = malloc(sizeof(int) * ng);
            if (!r->nomes || !r->cap_ini || !r->cap_fim) {
                free(r->nomes); free(r->cap_ini); free(r->cap_fim); free(r);
                r = NULL;
            }
        }
        if (!r) rerro(&l, "sem memoria");
    }
    if (!r) {
        libera_alt(raiz);
        leitor_libera(&l, 1);
        return NULL;
    }
    r->raiz = raiz;
    r->ngrupos = l.ngrupos;
    r->flags = l.flags;
    for (int g = 1; g <= l.ngrupos; g++) r->nomes[g] = l.nomes[g];
    leitor_libera(&l, 0);                     /* os nomes passaram pro PSRegex */
    return r;
}

void ps_regex_free(PSRegex *r)
{
    if (!r) return;
    libera_alt(r->raiz);
    for (int g = 1; g <= r->ngrupos; g++) free(r->nomes[g]);
    free(r->nomes);
    free(r->cap_ini);
    free(r->cap_fim);
    free(r);
}

int ps_regex_ngrupos(const PSRegex *r) { return r ? r->ngrupos : 0; }

int ps_regex_grupo_por_nome(const PSRegex *r, const char *nome, int len)
{
    if (!r || len <= 0) return -1;
    for (int g = 1; g <= r->ngrupos; g++)
        if (r->nomes[g] && (int)strlen(r->nomes[g]) == len && !memcmp(r->nomes[g], nome, (size_t)len))
            return g;
    return -1;
}

const char *ps_regex_nome_do_grupo(const PSRegex *r, int g)
{
    if (!r || g < 1 || g > r->ngrupos) return NULL;
    return r->nomes[g];
}

int ps_regex_nome_valido(const char *s, int len) { return nome_valido(s, len); }

/* ── casamento ───────────────────────────────────────────────────────────
 *
 * Backtracking com continuação explícita: `Cont` é a lista do que falta casar
 * depois do ponto atual. Sem ela, um grupo com alternância não saberia
 * retomar o resto do padrão ao voltar atrás. */

#define RX_MAX_PASSOS 2000000

/* PROFUNDIDADE, e ela é obrigatória junto com o teto de passos.
 *
 * O casador é recursivo: `m_seq -> m_pos_grupo -> m_rep_grupo -> m_alt ->
 * m_seq` gasta quadros de pilha C a cada repetição de grupo. O teto de PASSOS
 * (2 milhões) não protege disso — a pilha acaba antes, e o processo morria de
 * SIGSEGV sem mensagem nenhuma.
 *
 * Aconteceu de verdade: `"((?:[^"\\]|\\.)*)"` sobre um trecho de 29 mil
 * caracteres derrubou o `pool` inteiro. Um padrão vindo do usuário não pode
 * matar o processo — tem que virar erro, como qualquer outro limite. Quem
 * decide é a folga MEDIDA da pilha (`m_seq`), não um número de níveis. */

typedef struct Cont { const Seq *seq; int i; const struct Cont *prox; } Cont;

typedef struct {
    const char *s;
    int         n;
    RxCaptura  *cap;
    long        passos;
    int         estourou;
    int         fim_exigido;   /* só aceita terminando aqui (fullmatch: n); -1 = livre */
    int         flags;         /* as globais (RX_*) */
    int         prof;          /* profundidade de recursão do casador */
    int         vazio_proibido;/* o casamento VAZIO que começa aqui não vale; -1 = vale */
} Estado;

/* Continuação que marca o fim do corpo de um grupo atômico: a primeira vez
 * que o corpo chega nela, guarda onde parou e aceita — os pontos de retorno
 * de dentro do corpo nunca mais são tentados. */
typedef struct { Cont base; int *fim; } ContAtomo;
static const Seq SEQ_ATOMO = { 0 };   /* o endereço é a marca em `base.seq` */

/* Folding pro IGNORECASE: ASCII + Latin-1 Supplement (À-Þ<->à-þ), que cobre o
 * não-ASCII comum (café, ção, ñ). Outros scripts (grego, cirílico) não dobram.
 * É UMA regra, por CODEPOINT, nos três caminhos que comparam texto: o literal
 * (A_CHAR), a classe (A_CLASSE) e o retrovisor (A_BACKREF). O retrovisor
 * dobrava por BYTE, só A-Z, e `(?i)(é)\1` não casava `éÉ`. */
/* Caixa do IGNORECASE: o mapeamento simples do Unicode, a mesma tabela do
 * `lower()`/`upper()` do texto. Antes só ASCII e Latin-1: `(?i)ç` casava "Ç",
 * mas `(?i)ğ` não casava "Ğ" nem `(?i)σ` "Σ". */
static unsigned int rx_lower_cp(unsigned int cp) { return ps_uni_minuscula(cp); }
static unsigned int rx_upper_cp(unsigned int cp) { return ps_uni_maiuscula(cp); }

/* A outra caixa de uma letra ASCII (o IGNORECASE com a flag ASCII). */
static unsigned int rx_troca_ascii(unsigned int cp)
{
    if (cp >= 'A' && cp <= 'Z') return cp + 32;
    if (cp >= 'a' && cp <= 'z') return cp - 32;
    return cp;
}

/* Dois codepoints iguais sem olhar a caixa: pela minúscula OU pela maiúscula
 * (`ς` e `σ` só se encontram na `Σ`). Com a flag ASCII, só as letras ASCII
 * trocam de caixa, como no `re`. */
static int rx_mesma_letra(unsigned int a, unsigned int b, int ascii)
{
    if (a == b) return 1;
    if (ascii) return a < 128 && b < 128 && rx_troca_ascii(a) == b;
    return rx_lower_cp(a) == rx_lower_cp(b) || rx_upper_cp(a) == rx_upper_cp(b);
}

/* Caractere de palavra (\w, \b): ASCII alfanumérico e '_', e fora do ASCII
 * letra ou número pela tabela do Unicode — a mesma régua da classe `\w`.
 * Com a flag ASCII, só o ASCII. */
static int rx_is_word(unsigned int cp, int ascii)
{
    if (cp < 128)
        return (cp >= '0' && cp <= '9') || (cp >= 'a' && cp <= 'z')
            || (cp >= 'A' && cp <= 'Z') || cp == '_';
    return !ascii && rx_palavra_unicode(cp);
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
        if (e->fim_exigido >= 0 && pos != e->fim_exigido) return 0;
        /* depois de um casamento vazio em p, o seguinte começa em p mas não
         * pode ser vazio lá (o `must_advance` do `re`, desde o 3.7) */
        if (pos == e->vazio_proibido && e->cap->inicio[0] == pos) return 0;
        e->cap->fim[0] = pos;
        return 1;
    }
    /* o corpo de um grupo atômico chegou ao fim: guarda e aceita */
    if (k->seq == &SEQ_ATOMO) { *((const ContAtomo *)k)->fim = pos; return 1; }
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
    int sf = e->cap->fim[0], se = e->fim_exigido, sv = e->vazio_proibido;
    e->fim_exigido = -1;
    e->vazio_proibido = -1;                   /* a asserção pode casar vazio */
    int r = m_alt(e, alt, pos, NULL);
    e->cap->fim[0] = sf; e->fim_exigido = se; e->vazio_proibido = sv;
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

/* Lookbehind `(?<=P)`: P casa EXATAMENTE de `start` até `fim`? O texto não
 * encolhe: `$`, `\b` e `\Z` lá dentro olham o texto de verdade, como no
 * `re` (encolhido, o `\b` no fim do lookbehind via um fim de texto que não
 * existe). A largura fixa faz o casamento acabar em `fim`, e ele é exigido. */
static int look_atras_em(Estado *e, const Alt *alt, int start, int fim)
{
    if (start < 0) return 0;
    int se = e->fim_exigido, sf0 = e->cap->fim[0], sv = e->vazio_proibido;
    e->fim_exigido = fim;
    e->vazio_proibido = -1;
    int r = m_alt(e, alt, start, NULL);
    e->fim_exigido = se; e->cap->fim[0] = sf0; e->vazio_proibido = sv;
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
            /* o `$` do `re`: o fim, ou logo antes do '\n' que fecha o texto;
             * com MULTILINE, antes de todo '\n'. Antes só o fim valia, e
             * `foo.$` não achava nada em "foo1\nfoo2\n". */
            if (pos == e->n) return 0;
            if (a->flags & RX_M) return e->s[pos] == '\n' ? 0 : -1;
            return (pos == e->n - 1 && e->s[pos] == '\n') ? 0 : -1;
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
                return rx_mesma_letra(cp, a->cp, a->flags & RX_A) ? u : -1;
            return cp == a->cp ? u : -1;
        }
        case A_CLASSE: {
            if (pos >= e->n) return -1;
            unsigned int cp;
            int u = cp_le(e->s, e->n, pos, &cp);
            if (!u) return -1;
            /* IGNORECASE: o caractere ou outra caixa dele está no conjunto
             * escrito, e a negação `[^...]` vale DEPOIS. Antes a outra caixa
             * só era tentada quando o teste já com a negação falhava, e
             * `(?i)[^a-z]` casava "A" ("A" não está em a-z, logo passava). */
            int dentro = classe_dentro(&a->classe, cp);
            if (!dentro && (a->flags & RX_I)) {
                if (a->flags & RX_A) {
                    unsigned int o = rx_troca_ascii(cp);
                    if (o != cp) dentro = classe_dentro(&a->classe, o);
                } else {
                    /* as caixas e a volta por elas: o "ı" só chega ao `[a-z]`
                     * pelo "I" (a maiúscula dele, cuja minúscula é "i"), e o
                     * "ſ" pelo "S" — o `re` casa os dois com `(?i)[a-z]` */
                    unsigned int lo = rx_lower_cp(cp), up = rx_upper_cp(cp);
                    unsigned int cand[4] = { lo, up, rx_lower_cp(up), rx_upper_cp(lo) };
                    for (int k = 0; k < 4 && !dentro; k++)
                        if (cand[k] != cp) dentro = classe_dentro(&a->classe, cand[k]);
                }
            }
            int ok = a->classe.negado ? !dentro : dentro;
            return ok ? u : -1;
        }
        case A_STARTA: return pos == 0    ? 0 : -1;   /* \A */
        case A_ENDZ:   return pos == e->n ? 0 : -1;   /* \Z */
        case A_WORDB: {                               /* \b (idx 0) / \B (idx 1) */
            unsigned int cpa = 0, cpb = 0;
            int tem_a = (pos < e->n) && cp_le(e->s, e->n, pos, &cpa);
            int tem_b = rx_cp_antes(e->s, pos, &cpb);
            int wa = tem_a && rx_is_word(cpa, a->flags & RX_A);
            int wb = tem_b && rx_is_word(cpb, a->flags & RX_A);
            int fronteira = (wa != wb);
            int quer = (a->idx_grupo == 0) ? fronteira : !fronteira;
            return quer ? 0 : -1;
        }
        case A_BACKREF: {
            /* casa os MESMOS bytes que o grupo referenciado capturou */
            int gi = a->idx_grupo;
            if (gi < 1 || gi > e->cap->ngrupos) return -1;
            int gi_ini = e->cap->inicio[gi], gi_fim = e->cap->fim[gi];
            if (gi_ini < 0 || gi_fim < 0) return -1;   /* grupo não participou */
            int len = gi_fim - gi_ini;
            if (len == 0) return 0;
            if (!(a->flags & RX_I)) {
                if (pos + len > e->n) return -1;
                return memcmp(e->s + pos, e->s + gi_ini, (size_t)len) == 0 ? len : -1;
            }
            /* IGNORECASE: por CODEPOINT, com a mesma régua do literal e da
             * classe (`rx_mesma_letra`). Os comprimentos em bytes podem diferir
             * entre as duas caixas, então o retorno é o que foi consumido do
             * lado do texto. */
            int px = pos, py = gi_ini;
            while (py < gi_fim) {
                unsigned int cx, cy;
                int uy = cp_le(e->s, e->n, py, &cy);
                int ux = (px < e->n) ? cp_le(e->s, e->n, px, &cx) : 0;
                if (!ux || !uy) return -1;
                if (!rx_mesma_letra(cx, cy, a->flags & RX_A)) return -1;
                px += ux; py += uy;
            }
            return px - pos;
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
        int casou;
        if (a->tipo == A_COND) {
            /* o ramo 0 se o grupo citado casou (início e fim em ordem), o 1 se não */
            int g = a->cond_grupo;
            int existe = g <= e->cap->ngrupos && e->cap->inicio[g] >= 0
                         && e->cap->fim[g] >= e->cap->inicio[g];
            casou = m_seq(e, &a->grupo->ramos[existe ? 0 : 1], 0, pos, (const Cont *)&cr);
        } else {
            casou = m_alt(e, a->grupo, pos, (const Cont *)&cr);
        }
        if (casou) return 1;
        /* este caminho falhou: a captura tem que voltar ao que era */
        if (gi > 0) { e->cap->inicio[gi] = si; e->cap->fim[gi] = sf; }
        if (e->estourou) return 0;
    }

    if (q->guloso && ja_deu)
        return m_seq(e, rg->seq, rg->i + 1, pos, rg->k);
    return 0;
}

/* Uma passada do corpo do grupo atômico em `pos`: a PRIMEIRA solução dele,
 * sem pontos de retorno. Devolve os bytes andados, ou -1. */
static int casa_atomico(Estado *e, const Atomo *a, int pos)
{
    int fim = -1;
    ContAtomo ca;
    ca.base.seq = &SEQ_ATOMO; ca.base.i = 0; ca.base.prox = NULL;
    ca.fim = &fim;
    if (!m_alt(e, a->grupo, pos, (const Cont *)&ca)) return -1;
    return fim - pos;
}

/* Grupo atômico `(?>...)` (e o possessivo, que vira ele), com a repetição
 * dele: cada passada é atômica, e a repetição volta atrás contando passadas,
 * como a de um átomo simples. As capturas de dentro voltam ao que eram antes
 * de cada tentativa e são refeitas pelas passadas dela. */
static int m_atomico(Estado *e, const Seq *s, int i, int pos, const Cont *k)
{
    const Quant *q = &s->itens[i];
    const Atomo *a = &q->atomo;
    int g0 = a->g_de + 1, ng = a->g_ate - a->g_de;     /* os grupos de dentro */
    int local[2 * 16];
    int *salvo = local;
    if (ng > 16) {
        salvo = malloc(sizeof(int) * 2 * (size_t)ng);
        if (!salvo) { e->estourou = RX_ESTOUROU_FUNDO; return 0; }
    }
    for (int t = 0; t < ng; t++) {
        salvo[2 * t] = e->cap->inicio[g0 + t];
        salvo[2 * t + 1] = e->cap->fim[g0 + t];
    }
    int r = 0;
    /* quantas passadas cabem (a passada vazia satisfaz o mínimo: repetir não anda) */
    int max_pode = 0, p = pos;
    while (q->max < 0 || max_pode < q->max) {
        int c = casa_atomico(e, a, p);
        if (e->estourou || c < 0) break;
        p += c;
        max_pode++;
        if (c == 0) { if (max_pode < q->min) max_pode = q->min; break; }
    }
    if (!e->estourou && max_pode >= q->min) {
        int ini = q->guloso ? max_pode : q->min;
        int fim = q->guloso ? q->min : max_pode;
        int passo = q->guloso ? -1 : 1;
        for (int cont = ini; q->guloso ? cont >= fim : cont <= fim; cont += passo) {
            for (int t = 0; t < ng; t++) {
                e->cap->inicio[g0 + t] = salvo[2 * t];
                e->cap->fim[g0 + t] = salvo[2 * t + 1];
            }
            int np = pos, ok = 1;
            for (int t = 0; t < cont && ok; t++) {
                int c = casa_atomico(e, a, np);
                if (c < 0) ok = 0; else np += c;
                if (c == 0) break;            /* vazia: as próximas são iguais */
            }
            if (e->estourou) break;
            if (ok && m_seq(e, s, i + 1, np, k)) { r = 1; break; }
            if (e->estourou) break;
        }
    }
    if (!r)
        for (int t = 0; t < ng; t++) {
            e->cap->inicio[g0 + t] = salvo[2 * t];
            e->cap->fim[g0 + t] = salvo[2 * t + 1];
        }
    if (salvo != local) free(salvo);
    return r;
}

/* Casca que conta a profundidade. O corpo tem muitos `return`, e um contador
 * espalhado por todos eles é convite a esquecer um — a casca garante que
 * entrar e sair sempre fecham. */
static int m_seq(Estado *e, const Seq *s, int i, int pos, const Cont *k)
{
    /* DOIS freios, como no parser da linguagem (motor-1): o TETO CONTADO, que
     * dá o mesmo resultado em qualquer `ulimit -s` (era o defeito: o mesmo
     * texto dava erro com 8 MB e passava com os 64 MB do make da Apple), e a
     * folga medida da pilha, que cobre o quadro do binário de verdade (o do
     * ASan é maior).
     *
     * O teto é calibrado pela borda MEDIDA com 8 MB, com margem: 7500 `x`
     * casavam com `(?:a|(x))*` e 8000 davam "profundo demais"; com 64 MB os
     * 8500 casavam. Os 7000 põem a borda no mesmo lugar em qualquer pilha, e
     * ficam bem acima do uso real (um CSV de 3 mil campos passa). */
    if (e->prof >= RX_SEQ_PROF_MAX || ps_pilha_apertada()) {
        e->estourou = RX_ESTOUROU_FUNDO; return 0;
    }
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

    /* Átomos simples que casam UMA vez, seguidos (o literal de um `escape`
     * de 6 mil caracteres, o `,` de `(?:[a-z]+,)*`): cada um tem uma escolha
     * só, então casam em laço, sem voltar atrás e sem um quadro de pilha por
     * caractere — antes cada um descia um `m_seq`, e o literal longo
     * esbarrava no teto de profundidade sem retrocesso nenhum. */
    while (q->min == 1 && q->max == 1 && a->tipo != A_GRUPO && a->tipo != A_LOOK
           && a->tipo != A_ATOMICO && a->tipo != A_COND) {
        int c = casa_simples(e, a, pos);
        if (c < 0) return 0;
        pos += c;
        if (++i >= s->n) return m_cont(e, pos, k);
        if (++e->passos > RX_MAX_PASSOS) { e->estourou = 1; return 0; }
        q = &s->itens[i];
        a = &q->atomo;
    }

    if (a->tipo == A_GRUPO || a->tipo == A_COND) {
        RepGrupo rg = { e, q, s, i, k };
        return m_rep_grupo(&rg, 0, pos);
    }

    if (a->tipo == A_ATOMICO) return m_atomico(e, s, i, pos, k);

    if (a->tipo == A_LOOK) {
        /* asserção de largura zero: verifica e segue no MESMO pos. Repetida
         * (`(?=a)*`), o `re` aceita: repetir o que não anda não muda nada,
         * e com mínimo 0 ela vira opcional — com ela primeiro (gulosa) ou
         * sem ela primeiro (preguiçosa). */
        for (int volta = 0; volta < 2; volta++) {
            int com = (q->min > 0) || (q->guloso ? volta == 0 : volta == 1);
            if (com) {
                int ok = a->look_atras
                             ? look_atras_em(e, a->grupo, rx_recua_cp(e->s, pos, a->look_larg), pos)
                             : look_ahead(e, a->grupo, pos);
                if (a->look_neg) ok = !ok;
                if (ok && m_seq(e, s, i + 1, pos, k)) return 1;
            } else if (m_seq(e, s, i + 1, pos, k)) {
                return 1;
            }
            if (e->estourou || q->min > 0) return 0;
        }
        return 0;
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

int ps_regex_proximo(const char *s, int len, int pos)
{
    if (pos >= len) return pos + 1;
    unsigned int cp;
    int u = cp_le(s, len, pos, &cp);
    return pos + (u > 0 ? u : 1);
}

int ps_regex_busca(PSRegex *r, const char *s, int len, int de, RxCaptura *cap)
{
    return ps_regex_busca_vazio(r, s, len, de, -1, cap);
}

int ps_regex_busca_vazio(PSRegex *r, const char *s, int len, int de, int proibido, RxCaptura *cap)
{
    /* recomeça em cada CARACTERE, não em cada byte: no meio de um UTF-8 o
     * byte de continuação era lido como Latin-1 e `[^ç]` casava o resto do
     * "ç", devolvendo texto que não é UTF-8 */
    cap->inicio = r->cap_ini;
    cap->fim = r->cap_fim;
    cap->ngrupos = r->ngrupos;
    for (int inicio = de; inicio <= len; inicio = ps_regex_proximo(s, len, inicio)) {
        Estado e = { s, len, cap, 0, 0, -1, r->flags, 0, proibido };
        for (int g = 0; g <= r->ngrupos; g++) { cap->inicio[g] = -1; cap->fim[g] = -1; }
        cap->inicio[0] = inicio;
        if (m_alt(&e, r->raiz, inicio, NULL)) return 1;
        if (e.estourou) return e.estourou == RX_ESTOUROU_FUNDO ? -3 : -1;
    }
    return 0;
}

int ps_regex_casa_tudo(PSRegex *r, const char *s, int len, RxCaptura *cap)
{
    cap->inicio = r->cap_ini;
    cap->fim = r->cap_fim;
    cap->ngrupos = r->ngrupos;
    Estado e = { s, len, cap, 0, 0, len, r->flags, 0, -1 };
    for (int g = 0; g <= r->ngrupos; g++) { cap->inicio[g] = -1; cap->fim[g] = -1; }
    cap->inicio[0] = 0;
    if (m_alt(&e, r->raiz, 0, NULL)) return 1;
    if (e.estourou) return e.estourou == RX_ESTOUROU_FUNDO ? -3 : -1;
    return 0;
}
