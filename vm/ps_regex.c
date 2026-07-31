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

typedef enum { A_CHAR, A_QUALQUER, A_CLASSE, A_GRUPO, A_BOL, A_EOL } TipoAtomo;

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
    unsigned char c;              /* A_CHAR */
    Classe      classe;           /* A_CLASSE */
    Alt         *grupo;           /* A_GRUPO */
    int          idx_grupo;       /* -1 = não capturante */
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
};

/* ── construção ──────────────────────────────────────────────────────────── */

typedef struct {
    const char *p;
    int         n, i;
    int         ngrupos;
    char       *erro;
    int         erro_cap;
    int         falhou;
} Leitor;

static void rerro(Leitor *l, const char *msg)
{
    if (!l->falhou && l->erro && l->erro_cap > 0)
        snprintf(l->erro, (size_t)l->erro_cap, "regex: %s (posicao %d)", msg, l->i);
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
 * Só serve pro escape usado SOZINHO (`\d+`), onde a negação do `\D` vira o
 * `negado` da própria classe. Dentro de `[...]` a negação não composta é
 * possível — `[\D]` funciona, mas `[a\D]` sairia errado; o `re` também
 * trata esse caso como raro e aqui ele é recusado na leitura da classe. */
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
    l->i++;                                   /* passa '[' */
    if (l->i < l->n && l->p[l->i] == '^') { cl->negado = 1; l->i++; }
    int primeiro = 1;
    while (l->i < l->n && (l->p[l->i] != ']' || primeiro)) {
        primeiro = 0;
        unsigned int c;
        if (l->p[l->i] == '\\') {
            l->i++;
            if (l->i >= l->n) { rerro(l, "escape incompleto na classe"); return; }
            unsigned char e = (unsigned char)l->p[l->i++];
            Classe sub;
            memset(&sub, 0, sizeof(sub));
            if (classe_escape(e, &sub)) {
                if (sub.negado) {
                    /* `[a\D]` precisaria de união com um conjunto negado —
                     * recusa em vez de casar errado em silêncio. */
                    free(sub.faixas);
                    rerro(l, "classe negada (\\D \\W \\S) dentro de [] nao suportada");
                    return;
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
                if (l->i >= l->n) { rerro(l, "escape incompleto na classe"); return; }
                int ok;
                fim = escape_simples((unsigned char)l->p[l->i++], &ok);
            } else {
                int usados = cp_le(l->p, l->n, l->i, &fim);
                l->i += usados ? usados : 1;
            }
            if (fim < c) { rerro(l, "faixa invertida na classe"); return; }
            if (c < 128 && fim < 128) for (unsigned k = c; k <= fim; k++) mapa_bit(cl->mapa, k);
            else faixa_add(cl, c, fim);
        } else {
            classe_add(cl, c);
        }
    }
    if (l->i >= l->n) { rerro(l, "classe nao fechada"); return; }
    l->i++;                                   /* passa ']' */
}

static int le_atomo(Leitor *l, Atomo *a)
{
    if (l->i >= l->n) return 0;
    char c = l->p[l->i];
    a->grupo = NULL;
    a->idx_grupo = -1;
    if (c == '(') {
        l->i++;
        int captura = 1;
        if (l->i + 1 < l->n && l->p[l->i] == '?') {
            if (l->p[l->i + 1] == ':') { captura = 0; l->i += 2; }
            else { rerro(l, "grupo especial nao suportado (so (?:...))"); return 0; }
        }
        a->tipo = A_GRUPO;
        if (captura) {
            if (l->ngrupos + 1 >= RX_MAX_GRUPOS) { rerro(l, "grupos demais"); return 0; }
            a->idx_grupo = ++l->ngrupos;
        }
        a->grupo = le_alt(l);
        /* Em erro o átomo NÃO entra na sequência, então o libera_seq nunca
         * chegaria neste Alt — quem abandona, libera. */
        if (l->falhou) { libera_alt(a->grupo); a->grupo = NULL; return 0; }
        if (l->i >= l->n || l->p[l->i] != ')') {
            rerro(l, "faltou ')'");
            libera_alt(a->grupo); a->grupo = NULL;
            return 0;
        }
        l->i++;
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
        if (l->i >= l->n) { rerro(l, "escape incompleto"); return 0; }
        unsigned char e = (unsigned char)l->p[l->i++];
        memset(&a->classe, 0, sizeof(a->classe));
        if (classe_escape(e, &a->classe)) { a->tipo = A_CLASSE; return 1; }
        if (e >= '1' && e <= '9') { rerro(l, "retrovisor (\\1) nao suportado"); return 0; }
        if (e == 'b' || e == 'B') { rerro(l, "\\b nao suportado"); return 0; }
        int ok;
        unsigned char v = escape_simples(e, &ok);
        if (!ok) { rerro(l, "escape desconhecido"); return 0; }
        a->tipo = A_CHAR; a->c = v;
        return 1;
    }
    if (c == ')' || c == '|') return 0;        /* fim deste Seq */
    if (c == '*' || c == '+' || c == '?') { rerro(l, "quantificador sem alvo"); return 0; }
    l->i++;
    a->tipo = A_CHAR;
    a->c = (unsigned char)c;
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
                    if (hi >= 0 && hi < lo) { rerro(l, "{n,m} com m < n"); return; }
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
        if (s->itens[i].atomo.tipo == A_GRUPO) libera_alt(s->itens[i].atomo.grupo);
        else if (s->itens[i].atomo.tipo == A_CLASSE) free(s->itens[i].atomo.classe.faixas);
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

PSRegex *ps_regex_compila(const char *padrao, int len, char *erro, int erro_cap)
{
    Leitor l = { padrao, len, 0, 0, erro, erro_cap, 0 };
    Alt *raiz = le_alt(&l);
    if (!l.falhou && l.i != l.n) {
        /* sobrou ')' sem abrir, ou coisa parecida */
        rerro(&l, "caractere inesperado");
    }
    if (l.falhou) { libera_alt(raiz); return NULL; }
    PSRegex *r = malloc(sizeof(PSRegex));
    if (!r) { libera_alt(raiz); return NULL; }
    r->raiz = raiz;
    r->ngrupos = l.ngrupos;
    return r;
}

void ps_regex_free(PSRegex *r)
{
    if (!r) return;
    libera_alt(r->raiz);
    free(r);
}

int ps_regex_ngrupos(const PSRegex *r) { return r ? r->ngrupos : 0; }

/* ── casamento ───────────────────────────────────────────────────────────
 *
 * Backtracking com continuação explícita: `Cont` é a lista do que falta casar
 * depois do ponto atual. Sem ela, um grupo com alternância não saberia
 * retomar o resto do padrão ao voltar atrás. */

#define RX_MAX_PASSOS 2000000

typedef struct Cont { const Seq *seq; int i; const struct Cont *prox; } Cont;

typedef struct {
    const char *s;
    int         n;
    RxCaptura  *cap;
    long        passos;
    int         estourou;
    int         exigir_fim;    /* fullmatch: só aceita terminando em n */
} Estado;

static int m_seq(Estado *e, const Seq *s, int i, int pos, const Cont *k);
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

/* Casa um átomo SIMPLES (não-grupo) em `pos`. Devolve bytes consumidos, ou -1. */
static int casa_simples(Estado *e, const Atomo *a, int pos)
{
    switch (a->tipo) {
        case A_BOL: return pos == 0 ? 0 : -1;
        case A_EOL: return pos == e->n ? 0 : -1;
        case A_QUALQUER: {
            /* consome o CODEPOINT inteiro — um `.` não pode partir UTF-8 no
             * meio. E não casa com '\n', igual ao `re` sem DOTALL. */
            if (pos >= e->n || e->s[pos] == '\n') return -1;
            unsigned int cp;
            int u = cp_le(e->s, e->n, pos, &cp);
            return u ? u : -1;
        }
        case A_CHAR:
            return (pos < e->n && (unsigned char)e->s[pos] == a->c) ? 1 : -1;
        case A_CLASSE: {
            if (pos >= e->n) return -1;
            unsigned int cp;
            int u = cp_le(e->s, e->n, pos, &cp);
            if (!u) return -1;
            return classe_contem(&a->classe, cp) ? u : -1;
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

static int m_seq(Estado *e, const Seq *s, int i, int pos, const Cont *k)
{
    if (++e->passos > RX_MAX_PASSOS) { e->estourou = 1; return 0; }
    if (i >= s->n) return m_cont(e, pos, k);

    const Quant *q = &s->itens[i];
    const Atomo *a = &q->atomo;

    if (a->tipo == A_GRUPO) {
        RepGrupo rg = { e, q, s, i, k };
        return m_rep_grupo(&rg, 0, pos);
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
        Estado e = { s, len, cap, 0, 0, 0 };
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
    Estado e = { s, len, cap, 0, 0, 1 };
    for (int g = 0; g < RX_MAX_GRUPOS; g++) { cap->inicio[g] = -1; cap->fim[g] = -1; }
    cap->ngrupos = r->ngrupos;
    cap->inicio[0] = 0;
    if (m_alt(&e, r->raiz, 0, NULL)) return 1;
    if (e.estourou) return -1;
    return 0;
}
