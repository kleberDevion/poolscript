/*
 * A biblioteca padrão do C dentro da Jinga: os tipos do C e a chamada pela
 * libffi. Ver ps_c.h.
 *
 * Os tamanhos e alinhamentos saem do próprio compilador (`sizeof`,
 * `_Alignof`), então valem os do sistema em que o motor foi compilado. As
 * regras de chamada (registro, pilha, promoção das variádicas) são da libffi,
 * que conhece a convenção de cada sistema.
 */
#include "ps_c.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <wchar.h>
#include <math.h>

/* O macOS traz a libffi no próprio sistema, com o cabeçalho em `ffi/` (o
 * SDK não põe o `ffi.h` no topo); Linux e MSYS2 põem em `ffi.h`. */
#ifdef __APPLE__
#include <ffi/ffi.h>
#else
#include <ffi.h>
#endif

/* ── os escalares ─────────────────────────────────────────────────────── */
#define INTEIRO(var, nm, T, ch) \
    PSCTipo var = { .kind = PSC_INT, .nome = nm, .tam = sizeof(T), .alinha = _Alignof(T), \
                    .sem_sinal = ((T)-1 > (T)0), .eh_char = ch }
#define FLUTUANTE(var, nm, T) \
    PSCTipo var = { .kind = PSC_FLUT, .nome = nm, .tam = sizeof(T), .alinha = _Alignof(T) }

PSCTipo ps_c_void = { .kind = PSC_VOID, .nome = "void", .tam = 0, .alinha = 1 };
INTEIRO(ps_c_char,   "char",               char,               1);
INTEIRO(ps_c_schar,  "signed char",        signed char,        1);
INTEIRO(ps_c_uchar,  "unsigned char",      unsigned char,      1);
INTEIRO(ps_c_short,  "short",              short,              0);
INTEIRO(ps_c_ushort, "unsigned short",     unsigned short,     0);
INTEIRO(ps_c_int,    "int",                int,                0);
INTEIRO(ps_c_uint,   "unsigned int",       unsigned int,       0);
INTEIRO(ps_c_long,   "long",               long,               0);
INTEIRO(ps_c_ulong,  "unsigned long",      unsigned long,      0);
INTEIRO(ps_c_llong,  "long long",          long long,          0);
INTEIRO(ps_c_ullong, "unsigned long long", unsigned long long, 0);
FLUTUANTE(ps_c_float,   "float",       float);
FLUTUANTE(ps_c_double,  "double",      double);
FLUTUANTE(ps_c_ldouble, "long double", long double);
PSCTipo ps_c_bool = { .kind = PSC_BOOL, .nome = "_Bool", .tam = sizeof(_Bool), .alinha = _Alignof(_Bool),
                      .sem_sinal = 1 };

/* ── os typedefs do padrão ─────────────────────────────────────────────── */
/* Um typedef é um tipo novo com o nome dele e a forma do tipo que ele é no
 * sistema: o `size_t` é `unsigned long` no Linux e no macOS e `unsigned long
 * long` no Windows; o que a chamada precisa (tamanho, alinhamento, sinal)
 * vem do compilador daqui. */
static PSCTipo *typedef_int(const char *nome, size_t tam, size_t alinha, int sem_sinal)
{
    PSCTipo *t = calloc(1, sizeof *t);
    if (!t) return NULL;
    t->kind = PSC_INT;
    t->nome = nome;
    t->tam = tam;
    t->alinha = alinha;
    t->sem_sinal = sem_sinal;
    return t;
}
static PSCTipo *typedef_flut(const char *nome, size_t tam, size_t alinha)
{
    PSCTipo *t = calloc(1, sizeof *t);
    if (!t) return NULL;
    t->kind = PSC_FLUT;
    t->nome = nome;
    t->tam = tam;
    t->alinha = alinha;
    return t;
}
#define TD_INT(nm, T)  { nm, sizeof(T), _Alignof(T), ((T)-1 > (T)0), 0 }
#define TD_FLUT(nm, T) { nm, sizeof(T), _Alignof(T), 0, 1 }
typedef struct { const char *nome; size_t tam; size_t alinha; int sem_sinal; int flut; } TypedefPadrao;

/* time_t e clock_t: o padrão aceita inteiro ou flutuante. Nos três sistemas
 * do motor são inteiros, e o que não fosse viraria erro de compilação aqui
 * (o `_Static_assert` abaixo), em vez de valor errado rodando. */
_Static_assert((time_t)1.5 == (time_t)1, "time_t flutuante: falta tratar");
_Static_assert((clock_t)1.5 == (clock_t)1, "clock_t flutuante: falta tratar");

static const TypedefPadrao TYPEDEFS[] = {
    /* stddef.h */
    TD_INT("size_t", size_t), TD_INT("ptrdiff_t", ptrdiff_t), TD_INT("wchar_t", wchar_t),
    /* stdint.h */
    TD_INT("int8_t", int8_t), TD_INT("int16_t", int16_t), TD_INT("int32_t", int32_t), TD_INT("int64_t", int64_t),
    TD_INT("uint8_t", uint8_t), TD_INT("uint16_t", uint16_t), TD_INT("uint32_t", uint32_t), TD_INT("uint64_t", uint64_t),
    TD_INT("int_least8_t", int_least8_t), TD_INT("int_least16_t", int_least16_t),
    TD_INT("int_least32_t", int_least32_t), TD_INT("int_least64_t", int_least64_t),
    TD_INT("uint_least8_t", uint_least8_t), TD_INT("uint_least16_t", uint_least16_t),
    TD_INT("uint_least32_t", uint_least32_t), TD_INT("uint_least64_t", uint_least64_t),
    TD_INT("int_fast8_t", int_fast8_t), TD_INT("int_fast16_t", int_fast16_t),
    TD_INT("int_fast32_t", int_fast32_t), TD_INT("int_fast64_t", int_fast64_t),
    TD_INT("uint_fast8_t", uint_fast8_t), TD_INT("uint_fast16_t", uint_fast16_t),
    TD_INT("uint_fast32_t", uint_fast32_t), TD_INT("uint_fast64_t", uint_fast64_t),
    TD_INT("intptr_t", intptr_t), TD_INT("uintptr_t", uintptr_t),
    TD_INT("intmax_t", intmax_t), TD_INT("uintmax_t", uintmax_t),
    /* time.h */
    TD_INT("time_t", time_t), TD_INT("clock_t", clock_t),
    /* math.h */
    TD_FLUT("float_t", float_t), TD_FLUT("double_t", double_t),
};

/* Os nomes do C pros escalares, com as grafias que o padrão aceita. */
static const struct { const char *nome; PSCTipo *tipo; } ESCALARES[] = {
    { "void", &ps_c_void },
    { "char", &ps_c_char },
    { "signed char", &ps_c_schar },
    { "unsigned char", &ps_c_uchar },
    { "short", &ps_c_short }, { "short int", &ps_c_short }, { "signed short", &ps_c_short },
    { "signed short int", &ps_c_short },
    { "unsigned short", &ps_c_ushort }, { "unsigned short int", &ps_c_ushort },
    { "int", &ps_c_int }, { "signed", &ps_c_int }, { "signed int", &ps_c_int },
    { "unsigned", &ps_c_uint }, { "unsigned int", &ps_c_uint },
    { "long", &ps_c_long }, { "long int", &ps_c_long }, { "signed long", &ps_c_long },
    { "signed long int", &ps_c_long },
    { "unsigned long", &ps_c_ulong }, { "unsigned long int", &ps_c_ulong },
    { "long long", &ps_c_llong }, { "long long int", &ps_c_llong }, { "signed long long", &ps_c_llong },
    { "signed long long int", &ps_c_llong },
    { "unsigned long long", &ps_c_ullong }, { "unsigned long long int", &ps_c_ullong },
    { "float", &ps_c_float },
    { "double", &ps_c_double },
    { "long double", &ps_c_ldouble },
    { "_Bool", &ps_c_bool }, { "bool", &ps_c_bool },
};

static PSCTipo **g_typedefs = NULL;   /* um por linha de TYPEDEFS, montado na 1ª vez */

PSCTipo *ps_c_tipo_por_nome(const char *nome)
{
    for (size_t i = 0; i < sizeof ESCALARES / sizeof ESCALARES[0]; i++)
        if (strcmp(ESCALARES[i].nome, nome) == 0) return ESCALARES[i].tipo;
    size_t n = sizeof TYPEDEFS / sizeof TYPEDEFS[0];
    if (!g_typedefs) {
        g_typedefs = calloc(n, sizeof *g_typedefs);
        if (!g_typedefs) return NULL;
    }
    for (size_t i = 0; i < n; i++) {
        if (strcmp(TYPEDEFS[i].nome, nome) != 0) continue;
        if (!g_typedefs[i]) {
            const TypedefPadrao *d = &TYPEDEFS[i];
            g_typedefs[i] = d->flut ? typedef_flut(d->nome, d->tam, d->alinha)
                                    : typedef_int(d->nome, d->tam, d->alinha, d->sem_sinal);
        }
        return g_typedefs[i];
    }
    return NULL;
}

/* ── tipos montados ───────────────────────────────────────────────────── */
/* Guardados numa lista só (são poucos: os que o programa e as funções do C
 * usam), pra o mesmo tipo ser o mesmo objeto. */
typedef struct Montado { PSCTipo t; struct Montado *prox; } Montado;
static Montado *g_montados = NULL;

static char *junta_nome(const char *a, const char *b)
{
    size_t na = strlen(a), nb = strlen(b);
    char *s = malloc(na + nb + 1);
    if (!s) return NULL;
    memcpy(s, a, na);
    memcpy(s + na, b, nb + 1);
    return s;
}

static PSCTipo *novo_montado(void)
{
    Montado *m = calloc(1, sizeof *m);
    if (!m) return NULL;
    m->prox = g_montados;
    g_montados = m;
    return &m->t;
}

PSCTipo *ps_c_ponteiro(PSCTipo *alvo)
{
    for (Montado *m = g_montados; m; m = m->prox)
        if (m->t.kind == PSC_PONT && m->t.alvo == alvo && !m->t.constante) return &m->t;
    PSCTipo *t = novo_montado();
    if (!t) return NULL;
    t->kind = PSC_PONT;
    t->alvo = alvo;
    t->tam = sizeof(void *);
    t->alinha = _Alignof(void *);
    t->sem_sinal = 1;
    t->nome = junta_nome(alvo->nome, alvo->kind == PSC_PONT ? "*" : " *");
    return t;
}

PSCTipo *ps_c_const(PSCTipo *base)
{
    if (base->constante) return base;
    for (Montado *m = g_montados; m; m = m->prox)
        if (m->t.constante && m->t.kind == base->kind && m->t.alvo == base->alvo
            && strcmp(m->t.nome + 6, base->nome) == 0 && m->t.tam == base->tam) return &m->t;
    PSCTipo *t = novo_montado();
    if (!t) return NULL;
    *t = *base;
    t->constante = 1;
    t->cif = NULL;
    t->nome = junta_nome("const ", base->nome);
    return t;
}

PSCTipo *ps_c_array(PSCTipo *elem, size_t n)
{
    for (Montado *m = g_montados; m; m = m->prox)
        if (m->t.kind == PSC_ARRAY && m->t.alvo == elem && m->t.n == n) return &m->t;
    PSCTipo *t = novo_montado();
    if (!t) return NULL;
    t->kind = PSC_ARRAY;
    t->alvo = elem;
    t->n = n;
    t->tam = elem->tam * n;
    t->alinha = elem->alinha;
    char suf[32];
    snprintf(suf, sizeof suf, "[%zu]", n);
    t->nome = junta_nome(elem->nome, suf);
    return t;
}

PSCTipo *ps_c_funcao(PSCTipo *ret, PSCTipo **params, int nparams, int variadica)
{
    PSCTipo *t = novo_montado();
    if (!t) return NULL;
    t->kind = PSC_FUNC;
    t->ret = ret;
    t->nparams = nparams;
    t->variadica = variadica;
    t->nome = "funct";
    if (nparams > 0) {
        t->params = malloc((size_t)nparams * sizeof *t->params);
        if (!t->params) return NULL;
        memcpy(t->params, params, (size_t)nparams * sizeof *t->params);
    }
    return t;
}

PSCTipo *ps_c_opaco(const char *nome, size_t tam)
{
    PSCTipo *t = novo_montado();
    if (!t) return NULL;
    t->kind = PSC_OPACO;
    t->nome = nome;
    t->tam = tam;
    t->alinha = 1;
    return t;
}

/* ── a ponte pra libffi ───────────────────────────────────────────────── */
static ffi_type *ffi_de(PSCTipo *t);

PSCTipo *ps_c_struct(const char *nome, int eh_union, const char **nomes, PSCTipo **tipos,
                     int n, const size_t *deslocs, size_t tam, size_t alinha)
{
    PSCTipo *t = novo_montado();
    if (!t) return NULL;
    t->kind = eh_union ? PSC_UNION : PSC_STRUCT;
    t->nome = nome;
    t->ncampos = n;
    t->campos = calloc((size_t)(n > 0 ? n : 1), sizeof *t->campos);
    if (!t->campos) return NULL;
    for (int i = 0; i < n; i++) {
        t->campos[i].nome = nomes[i];
        t->campos[i].tipo = tipos[i];
    }
    if (deslocs) {
        /* os structs do padrão: a forma que o compilador do sistema deu */
        for (int i = 0; i < n; i++) t->campos[i].desloc = deslocs[i];
        t->tam = tam;
        t->alinha = alinha;
        return t;
    }
    /* a regra do C: cada campo no próximo múltiplo do alinhamento dele (no
     * union, todos no começo), e o tamanho arredondado pro maior alinhamento */
    size_t pos = 0, maior = 1, maior_tam = 0;
    for (int i = 0; i < n; i++) {
        size_t a = tipos[i]->alinha ? tipos[i]->alinha : 1;
        if (a > maior) maior = a;
        if (eh_union) {
            t->campos[i].desloc = 0;
            if (tipos[i]->tam > maior_tam) maior_tam = tipos[i]->tam;
        } else {
            pos = (pos + a - 1) / a * a;
            t->campos[i].desloc = pos;
            pos += tipos[i]->tam;
        }
    }
    if (eh_union) pos = maior_tam;
    t->alinha = maior;
    t->tam = (pos + maior - 1) / maior * maior;
    return t;
}

static ffi_type *ffi_inteiro(size_t tam, int sem_sinal)
{
    switch (tam) {
        case 1: return sem_sinal ? &ffi_type_uint8  : &ffi_type_sint8;
        case 2: return sem_sinal ? &ffi_type_uint16 : &ffi_type_sint16;
        case 4: return sem_sinal ? &ffi_type_uint32 : &ffi_type_sint32;
        case 8: return sem_sinal ? &ffi_type_uint64 : &ffi_type_sint64;
        default: return NULL;
    }
}

static ffi_type *ffi_de(PSCTipo *t)
{
    switch (t->kind) {
        case PSC_VOID: return &ffi_type_void;
        case PSC_INT:
        case PSC_BOOL: return ffi_inteiro(t->tam, t->sem_sinal);
        case PSC_FLUT:
            if (t->tam == sizeof(float)) return &ffi_type_float;
            if (t->tam == sizeof(double) && t != &ps_c_ldouble) return &ffi_type_double;
            return &ffi_type_longdouble;
        case PSC_PONT:
        case PSC_ARRAY:     /* parâmetro array é ponteiro, como no C */
        case PSC_FUNC:      /* parâmetro função é ponteiro de função */
            return &ffi_type_pointer;
        case PSC_STRUCT: {
            if (t->ffi) return (ffi_type *)t->ffi;
            ffi_type *ft = calloc(1, sizeof *ft);
            ffi_type **els = calloc((size_t)t->ncampos + 1, sizeof *els);
            if (!ft || !els) { free(ft); free(els); return NULL; }
            for (int i = 0; i < t->ncampos; i++) {
                els[i] = ffi_de(t->campos[i].tipo);
                if (!els[i]) { free(ft); free(els); return NULL; }
            }
            ft->type = FFI_TYPE_STRUCT;
            ft->elements = els;
            t->ffi = ft;
            return ft;
        }
        case PSC_UNION:     /* a libffi não tem union por valor: só atrás de ponteiro */
        case PSC_OPACO:
        default:
            return NULL;
    }
}

size_t ps_c_tam_ret(PSCTipo *ret)
{
    size_t t = ret->tam;
    return t < sizeof(ffi_arg) ? sizeof(ffi_arg) : t;
}

/* O inteiro de retorno mais estreito que um registro vem alargado num
 * `ffi_arg` (a regra da libffi): lê pelo tamanho do registro. */
int64_t ps_c_ret_int(PSCTipo *ret, const void *buf)
{
    if (ret->tam < sizeof(ffi_arg)) {
        ffi_sarg v;
        memcpy(&v, buf, sizeof v);
        return (int64_t)v;
    }
    int64_t v = 0;
    memcpy(&v, buf, ret->tam < sizeof v ? ret->tam : sizeof v);
    return v;
}

uint64_t ps_c_ret_uint(PSCTipo *ret, const void *buf)
{
    if (ret->tam < sizeof(ffi_arg)) {
        ffi_arg v;
        memcpy(&v, buf, sizeof v);
        return (uint64_t)v;
    }
    uint64_t v = 0;
    memcpy(&v, buf, ret->tam < sizeof v ? ret->tam : sizeof v);
    return v;
}

void ps_c_ret_grava_int(PSCTipo *ret, void *buf, int64_t v)
{
    if (ret->tam >= sizeof(ffi_arg)) {
        memcpy(buf, &v, ret->tam < sizeof v ? ret->tam : sizeof v);
        return;
    }
    if (ret->sem_sinal) {
        uint64_t u = (uint64_t)v;
        if (ret->tam == 1) u = (uint8_t)u;
        else if (ret->tam == 2) u = (uint16_t)u;
        else if (ret->tam == 4) u = (uint32_t)u;
        ffi_arg a = (ffi_arg)u;
        memcpy(buf, &a, sizeof a);
    } else {
        int64_t s = v;
        if (ret->tam == 1) s = (int8_t)(uint8_t)(uint64_t)v;
        else if (ret->tam == 2) s = (int16_t)(uint16_t)(uint64_t)v;
        else if (ret->tam == 4) s = (int32_t)(uint32_t)(uint64_t)v;
        ffi_sarg a = (ffi_sarg)s;
        memcpy(buf, &a, sizeof a);
    }
}

int ps_c_chama(void (*fn)(void), PSCTipo *f, void **args, int nargs,
               PSCTipo **tipos_var, void *ret)
{
    ffi_type *rt = ffi_de(f->ret);
    if (!rt) return -1;
    if (!f->variadica) {
        if (!f->cif) {
            ffi_cif *cif = calloc(1, sizeof *cif);
            ffi_type **tps = calloc((size_t)(f->nparams > 0 ? f->nparams : 1), sizeof *tps);
            if (!cif || !tps) { free(cif); free(tps); return -1; }
            for (int i = 0; i < f->nparams; i++) {
                tps[i] = ffi_de(f->params[i]);
                if (!tps[i]) { free(cif); free(tps); return -1; }
            }
            if (ffi_prep_cif(cif, FFI_DEFAULT_ABI, (unsigned)f->nparams, rt, tps) != FFI_OK) {
                free(cif); free(tps); return -1;
            }
            f->cif = cif;
        }
        ffi_call((ffi_cif *)f->cif, fn, ret, args);
        return 0;
    }
    /* variádica: o cif muda com os argumentos a mais, então é um por chamada */
    ffi_cif cif;
    ffi_type *fixos[16];
    ffi_type **tps = nargs <= 16 ? fixos : malloc((size_t)nargs * sizeof *tps);
    if (!tps) return -1;
    int rc = 0;
    for (int i = 0; i < nargs && rc == 0; i++) {
        tps[i] = ffi_de(i < f->nparams ? f->params[i] : tipos_var[i - f->nparams]);
        if (!tps[i]) rc = -1;
    }
    if (rc == 0 && ffi_prep_cif_var(&cif, FFI_DEFAULT_ABI, (unsigned)f->nparams, (unsigned)nargs,
                                    rt, tps) != FFI_OK) rc = -1;
    if (rc == 0) ffi_call(&cif, fn, ret, args);
    if (tps != fixos) free(tps);
    return rc;
}

/* ── callbacks ────────────────────────────────────────────────────────── */
typedef struct {
    ffi_closure *cl;
    ffi_cif      cif;
    ffi_type   **tps;
    PSCTratador  tratador;
    void        *dados;
} Fecho;

static void fecho_ponte(ffi_cif *cif, void *ret, void **args, void *user)
{
    (void)cif;
    Fecho *f = (Fecho *)user;
    f->tratador(ret, args, f->dados);
}

void *ps_c_closure_nova(PSCTipo *ftipo, PSCTratador tratador, void *dados, void (**codigo)(void))
{
    if (ftipo->kind != PSC_FUNC || ftipo->variadica) return NULL;
    ffi_type *rt = ffi_de(ftipo->ret);
    if (!rt) return NULL;
    Fecho *f = calloc(1, sizeof *f);
    if (!f) return NULL;
    f->tps = calloc((size_t)(ftipo->nparams > 0 ? ftipo->nparams : 1), sizeof *f->tps);
    if (!f->tps) { free(f); return NULL; }
    for (int i = 0; i < ftipo->nparams; i++) {
        f->tps[i] = ffi_de(ftipo->params[i]);
        if (!f->tps[i]) { free(f->tps); free(f); return NULL; }
    }
    if (ffi_prep_cif(&f->cif, FFI_DEFAULT_ABI, (unsigned)ftipo->nparams, rt, f->tps) != FFI_OK) {
        free(f->tps); free(f); return NULL;
    }
    void *exec = NULL;
    f->cl = ffi_closure_alloc(sizeof(ffi_closure), &exec);
    if (!f->cl) { free(f->tps); free(f); return NULL; }
    f->tratador = tratador;
    f->dados = dados;
    if (ffi_prep_closure_loc(f->cl, &f->cif, fecho_ponte, f, exec) != FFI_OK) {
        ffi_closure_free(f->cl); free(f->tps); free(f); return NULL;
    }
    *codigo = (void (*)(void))exec;
    return f;
}

void ps_c_closure_solta(void *closure)
{
    Fecho *f = (Fecho *)closure;
    if (!f) return;
    ffi_closure_free(f->cl);
    free(f->tps);
    free(f);
}
