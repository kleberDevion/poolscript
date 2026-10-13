/*
 * Consultas às tabelas do Unicode (ps_unicode_tab.inc, gerado por
 * scripts/gera_unicode.pr). Ver ps_unicode.h.
 *
 * Antes cada método de texto tinha a sua aritmética por bloco (ASCII,
 * Latin-1, Latin Estendido-A, grego básico, cirílico): fora disso o
 * caractere passava intacto pelo upper()/lower() e o isalpha() dizia False
 * pro "漢". Agora a régua é a do Unicode inteiro.
 */
#include <stddef.h>
#include "ps_unicode.h"

typedef struct { uint32_t primeiro, ultimo; int32_t d_maiu, d_minu, d_tit; } UniCaixa;
typedef struct { uint32_t cp; uint32_t minu[3], tit[3], maiu[3]; } UniEspecial;
typedef struct { uint32_t cp; uint32_t dobra[3]; } UniDobra;
/* delta que marca a faixa de pares alternados (Āā Ăă ...): a maiúscula no
 * par, contando do primeiro da faixa, e a minúscula logo depois */
#define UNI_ALTERNA 0x7FFFFFFF

#include "ps_unicode_tab.inc"

#define N_ELEM(a) ((int)(sizeof(a) / sizeof((a)[0])))

const char *ps_uni_versao(void) { return UNI_VERSAO; }

int ps_uni_props(uint32_t c)
{
    if (c < 256) return UNI_PROPS_LATIN1[c];
    if (c > 0x10FFFF) return UNI_CN;
    /* a última faixa que começa em `c` ou antes */
    int lo = 0, hi = N_ELEM(UNI_FAIXA_INICIO) - 1;
    while (lo < hi) {
        int m = (lo + hi + 1) / 2;
        if (UNI_FAIXA_INICIO[m] <= c) lo = m;
        else hi = m - 1;
    }
    return UNI_FAIXA_PROPS[lo];
}

int ps_uni_eh_letra(uint32_t c)
{
    int cat = ps_uni_props(c) & UNI_CATEGORIA;
    return cat >= UNI_LU && cat <= UNI_LO;
}
int ps_uni_eh_decimal(uint32_t c)   { return (ps_uni_props(c) & UNI_DECIMAL) != 0; }
int ps_uni_eh_digito(uint32_t c)    { return (ps_uni_props(c) & (UNI_DECIMAL | UNI_DIGITO)) != 0; }
int ps_uni_eh_numerico(uint32_t c)  { return (ps_uni_props(c) & (UNI_DECIMAL | UNI_DIGITO | UNI_NUMERICO)) != 0; }
int ps_uni_eh_branco(uint32_t c)    { return (ps_uni_props(c) & UNI_BRANCO) != 0; }
int ps_uni_eh_maiuscula(uint32_t c) { return (ps_uni_props(c) & UNI_MAIUSCULA) != 0; }
int ps_uni_eh_minuscula(uint32_t c) { return (ps_uni_props(c) & UNI_MINUSCULA) != 0; }
int ps_uni_eh_titulo(uint32_t c)    { return (ps_uni_props(c) & UNI_CATEGORIA) == UNI_LT; }
int ps_uni_tem_caixa(uint32_t c)    { return (ps_uni_props(c) & UNI_TEM_CAIXA) != 0; }
int ps_uni_ignora_caixa(uint32_t c) { return (ps_uni_props(c) & UNI_IGNORA_CAIXA) != 0; }

/* Imprimível: o espaço, e tudo que não é controle, formatação, substituto,
 * uso privado, não atribuído (C*) nem separador (Z*). */
int ps_uni_eh_imprimivel(uint32_t c)
{
    if (c == ' ') return 1;
    int cat = ps_uni_props(c) & UNI_CATEGORIA;
    return cat != UNI_CN && cat < UNI_ZS;
}

static const UniCaixa *faixa_de_caixa(uint32_t c)
{
    int lo = 0, hi = N_ELEM(UNI_CAIXA) - 1;
    while (lo <= hi) {
        int m = (lo + hi) / 2;
        if (c < UNI_CAIXA[m].primeiro) hi = m - 1;
        else if (c > UNI_CAIXA[m].ultimo) lo = m + 1;
        else return &UNI_CAIXA[m];
    }
    return NULL;
}

/* qual: 0 maiúscula, 1 minúscula, 2 título */
static uint32_t caixa_simples(uint32_t c, int qual)
{
    if (c < 0x80) {
        if (qual == 1) return (c >= 'A' && c <= 'Z') ? c + 32 : c;
        return (c >= 'a' && c <= 'z') ? c - 32 : c;
    }
    const UniCaixa *f = faixa_de_caixa(c);
    if (!f) return c;
    int32_t d = qual == 0 ? f->d_maiu : qual == 1 ? f->d_minu : f->d_tit;
    if (d == UNI_ALTERNA) {
        uint32_t maiu = f->primeiro + ((c - f->primeiro) & ~1u);
        return qual == 1 ? maiu + 1 : maiu;
    }
    return (uint32_t)((int32_t)c + d);
}

uint32_t ps_uni_maiuscula(uint32_t c) { return caixa_simples(c, 0); }
uint32_t ps_uni_minuscula(uint32_t c) { return caixa_simples(c, 1); }
uint32_t ps_uni_titulo(uint32_t c)    { return caixa_simples(c, 2); }

static int copia3(const uint32_t m[3], uint32_t out[3])
{
    int n = 0;
    while (n < 3 && m[n]) { out[n] = m[n]; n++; }
    return n;
}

int ps_uni_caixa(uint32_t c, int qual, uint32_t out[3])
{
    /* o primeiro especial é o ß (U+00DF) */
    if (c >= 0xDF) {
        int lo = 0, hi = N_ELEM(UNI_ESPECIAL) - 1;
        while (lo <= hi) {
            int m = (lo + hi) / 2;
            if (c < UNI_ESPECIAL[m].cp) hi = m - 1;
            else if (c > UNI_ESPECIAL[m].cp) lo = m + 1;
            else {
                const UniEspecial *e = &UNI_ESPECIAL[m];
                return copia3(qual == 0 ? e->maiu : qual == 1 ? e->minu : e->tit, out);
            }
        }
    }
    out[0] = caixa_simples(c, qual);
    return 1;
}

int ps_uni_dobra(uint32_t c, uint32_t out[3])
{
    if (c >= 0xB5) {
        int lo = 0, hi = N_ELEM(UNI_DOBRA) - 1;
        while (lo <= hi) {
            int m = (lo + hi) / 2;
            if (c < UNI_DOBRA[m].cp) hi = m - 1;
            else if (c > UNI_DOBRA[m].cp) lo = m + 1;
            else return copia3(UNI_DOBRA[m].dobra, out);
        }
    }
    out[0] = caixa_simples(c, 1);
    return 1;
}

int ps_uni_valor_decimal(uint32_t c)
{
    if (c >= '0' && c <= '9') return (int)(c - '0');
    if (!ps_uni_eh_decimal(c)) return -1;
    /* o último zero que vem em `c` ou antes */
    int lo = 0, hi = N_ELEM(UNI_DECIMAL_ZERO) - 1;
    while (lo < hi) {
        int m = (lo + hi + 1) / 2;
        if (UNI_DECIMAL_ZERO[m] <= c) lo = m;
        else hi = m - 1;
    }
    uint32_t d = c - UNI_DECIMAL_ZERO[lo];
    return d <= 9 ? (int)d : -1;
}
