/*
 * A biblioteca padrão do C dentro da Jinga: os tipos do C, a tabela da
 * biblioteca (vm/c_padrao.decl) e a chamada pela libffi. Ver ps_c.h.
 *
 * Os tamanhos e alinhamentos saem do próprio compilador (`sizeof`,
 * `_Alignof`), então valem os do sistema em que o motor foi compilado. As
 * regras de chamada (registro, pilha, promoção das variádicas) são da libffi,
 * que conhece a convenção de cada sistema.
 *
 * Quem não usa o C não paga nada: nada daqui roda na partida. As tabelas são
 * só leitura e não guardam ponteiro (guardam posições num bloco de texto),
 * então o carregador não tem relocação delas pra aplicar. O endereço de cada
 * função do sistema é achado pelo nome (dlsym) na primeira chamada dela:
 * tomar o endereço no código faria o carregador resolver as 300 funções da
 * tabela em toda partida, de todo programa, mesmo com a ligação preguiçosa
 * (`perf stat -e instructions:u` de um programa vazio mostra a diferença).
 */
#include "ps_c.h"

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#ifndef _WIN32
#include <dlfcn.h>
#endif

/* O macOS traz a libffi no próprio sistema, com o cabeçalho em `ffi/` (o
 * SDK não põe o `ffi.h` no topo); Linux e MSYS2 põem em `ffi.h`. */
#ifdef __APPLE__
#include <ffi/ffi.h>
#else
#include <ffi.h>
#endif

/* ── a libffi, aberta na primeira chamada ao C ────────────────────────────
 * No Linux e no macOS ela não é ligada ao binário: é aberta (dlopen) na
 * primeira vez que o programa chama uma função do C, como os clientes de
 * banco (vm/ps_dl.h). Ligada, ela trazia nomes da biblioteca do C que o
 * carregador resolvia em toda partida (memfd_create, statfs, getmntent_r: os
 * da memória executável dos callbacks), e no macOS uma biblioteca a mais pra
 * carregar. Os tipos e as constantes continuam vindo do cabeçalho, então a
 * biblioteca aberta tem que ser a da mesma versão: a libffi.so.8 (3.4) do
 * Linux, a do próprio macOS. No Windows ela entra estática no jinga.exe. */
#ifndef _WIN32
#include "ps_dl.h"
#define FFI_FUNCS(X)                                                                     \
    X(ffi_prep_cif) X(ffi_prep_cif_var) X(ffi_call) X(ffi_closure_alloc) X(ffi_closure_free) \
    X(ffi_prep_closure_loc) X(ffi_type_void) X(ffi_type_uint8) X(ffi_type_sint8)          \
    X(ffi_type_uint16) X(ffi_type_sint16) X(ffi_type_uint32) X(ffi_type_sint32)           \
    X(ffi_type_uint64) X(ffi_type_sint64) X(ffi_type_float) X(ffi_type_double)            \
    X(ffi_type_longdouble) X(ffi_type_pointer)
FFI_FUNCS(PS_DL_PONTEIRO)
static int  g_ffi_estado;
static char g_ffi_motivo[320];
#ifdef __APPLE__
#define FFI_NOME "/usr/lib/libffi.dylib"
#else
#define FFI_NOME "libffi.so.8"
#endif
/* Daqui pra baixo, o nome da libffi é o ponteiro (os ffi_type_* são dados). */
#define ffi_prep_cif dl_ffi_prep_cif
#define ffi_prep_cif_var dl_ffi_prep_cif_var
#define ffi_call dl_ffi_call
#define ffi_closure_alloc dl_ffi_closure_alloc
#define ffi_closure_free dl_ffi_closure_free
#define ffi_prep_closure_loc dl_ffi_prep_closure_loc
#define ffi_type_void (*dl_ffi_type_void)
#define ffi_type_uint8 (*dl_ffi_type_uint8)
#define ffi_type_sint8 (*dl_ffi_type_sint8)
#define ffi_type_uint16 (*dl_ffi_type_uint16)
#define ffi_type_sint16 (*dl_ffi_type_sint16)
#define ffi_type_uint32 (*dl_ffi_type_uint32)
#define ffi_type_sint32 (*dl_ffi_type_sint32)
#define ffi_type_uint64 (*dl_ffi_type_uint64)
#define ffi_type_sint64 (*dl_ffi_type_sint64)
#define ffi_type_float (*dl_ffi_type_float)
#define ffi_type_double (*dl_ffi_type_double)
#define ffi_type_longdouble (*dl_ffi_type_longdouble)
#define ffi_type_pointer (*dl_ffi_type_pointer)
#endif

int ps_c_pronto(char *erro, size_t ecap)
{
#ifdef _WIN32
    (void)erro;
    (void)ecap;
    return 0;
#else
    if (g_ffi_estado == 1) return 0;
    char e[320];
    /* a lista na pilha, e não estática: lista estática de ponteiros é
     * relocação que o carregador aplica na partida */
    const char *nomes[] = { FFI_NOME, NULL };
    PS_DL_CARREGA(g_ffi_estado, g_ffi_motivo, nomes, "a biblioteca do C na Jinga", FFI_FUNCS, e, sizeof e);
    if (g_ffi_estado == 1) return 0;
    if (erro && ecap) snprintf(erro, ecap, "%s", g_ffi_motivo);
    return -1;
#endif
}

/* ── os escalares ─────────────────────────────────────────────────────── */
/* O nome de cada um entra na primeira chamada (nomeia_escalares), e não no
 * dado estático: ponteiro em dado estático é relocação que o carregador
 * aplica na partida de todo programa. */
#define INTEIRO(var, T, ch) \
    PSCTipo var = { .kind = PSC_INT, .tam = sizeof(T), .alinha = _Alignof(T), \
                    .sem_sinal = ((T)-1 > (T)0), .eh_char = ch }
#define FLUTUANTE(var, T) \
    PSCTipo var = { .kind = PSC_FLUT, .tam = sizeof(T), .alinha = _Alignof(T) }

PSCTipo ps_c_void = { .kind = PSC_VOID, .tam = 0, .alinha = 1 };
INTEIRO(ps_c_char,   char,               1);
INTEIRO(ps_c_schar,  signed char,        1);
INTEIRO(ps_c_uchar,  unsigned char,      1);
INTEIRO(ps_c_short,  short,              0);
INTEIRO(ps_c_ushort, unsigned short,     0);
INTEIRO(ps_c_int,    int,                0);
INTEIRO(ps_c_uint,   unsigned int,       0);
INTEIRO(ps_c_long,   long,               0);
INTEIRO(ps_c_ulong,  unsigned long,      0);
INTEIRO(ps_c_llong,  long long,          0);
INTEIRO(ps_c_ullong, unsigned long long, 0);
FLUTUANTE(ps_c_float,   float);
FLUTUANTE(ps_c_double,  double);
FLUTUANTE(ps_c_ldouble, long double);
PSCTipo ps_c_bool = { .kind = PSC_BOOL, .tam = sizeof(_Bool), .alinha = _Alignof(_Bool), .sem_sinal = 1 };

static void nomeia_escalares(void)
{
    if (ps_c_void.nome) return;
    ps_c_char.nome = "char";
    ps_c_schar.nome = "signed char";
    ps_c_uchar.nome = "unsigned char";
    ps_c_short.nome = "short";
    ps_c_ushort.nome = "unsigned short";
    ps_c_int.nome = "int";
    ps_c_uint.nome = "unsigned int";
    ps_c_long.nome = "long";
    ps_c_ulong.nome = "unsigned long";
    ps_c_llong.nome = "long long";
    ps_c_ullong.nome = "unsigned long long";
    ps_c_float.nome = "float";
    ps_c_double.nome = "double";
    ps_c_ldouble.nome = "long double";
    ps_c_bool.nome = "_Bool";
    ps_c_void.nome = "void";   /* o último: é ele que diz que os outros já têm nome */
}

/* time_t e clock_t: o padrão aceita inteiro ou flutuante. Nos três sistemas
 * do motor são inteiros, e o que não fosse viraria erro de compilação aqui
 * (o `_Static_assert` abaixo), em vez de valor errado rodando. */
_Static_assert((time_t)1.5 == (time_t)1, "time_t flutuante: falta tratar");
_Static_assert((clock_t)1.5 == (clock_t)1, "clock_t flutuante: falta tratar");

/* O escalar que uma expressão do C tem, pelo compilador: é o que diz o tipo
 * de cada constante e de cada typedef da tabela. */
enum {
    PSC_T_NADA, PSC_T_CHAR, PSC_T_SCHAR, PSC_T_UCHAR, PSC_T_SHORT, PSC_T_USHORT, PSC_T_INT,
    PSC_T_UINT, PSC_T_LONG, PSC_T_ULONG, PSC_T_LLONG, PSC_T_ULLONG, PSC_T_FLOAT, PSC_T_DOUBLE,
    PSC_T_LDOUBLE, PSC_T_BOOL
};
#define PS_C_TIPO(x) _Generic((x), \
    char: PSC_T_CHAR, signed char: PSC_T_SCHAR, unsigned char: PSC_T_UCHAR, \
    short: PSC_T_SHORT, unsigned short: PSC_T_USHORT, int: PSC_T_INT, unsigned int: PSC_T_UINT, \
    long: PSC_T_LONG, unsigned long: PSC_T_ULONG, long long: PSC_T_LLONG, \
    unsigned long long: PSC_T_ULLONG, float: PSC_T_FLOAT, double: PSC_T_DOUBLE, \
    long double: PSC_T_LDOUBLE, _Bool: PSC_T_BOOL, default: PSC_T_NADA)

/* Um switch, e não uma tabela de ponteiros: tabela de ponteiros é relocação
 * que o carregador aplica na partida. */
static PSCTipo *escalar_de(int codigo)
{
    nomeia_escalares();
    switch (codigo) {
        case PSC_T_CHAR:    return &ps_c_char;
        case PSC_T_SCHAR:   return &ps_c_schar;
        case PSC_T_UCHAR:   return &ps_c_uchar;
        case PSC_T_SHORT:   return &ps_c_short;
        case PSC_T_USHORT:  return &ps_c_ushort;
        case PSC_T_INT:     return &ps_c_int;
        case PSC_T_UINT:    return &ps_c_uint;
        case PSC_T_LONG:    return &ps_c_long;
        case PSC_T_ULONG:   return &ps_c_ulong;
        case PSC_T_LLONG:   return &ps_c_llong;
        case PSC_T_ULLONG:  return &ps_c_ullong;
        case PSC_T_FLOAT:   return &ps_c_float;
        case PSC_T_DOUBLE:  return &ps_c_double;
        case PSC_T_LDOUBLE: return &ps_c_ldouble;
        case PSC_T_BOOL:    return &ps_c_bool;
        default:            return NULL;
    }
}

/* ── a tabela da biblioteca padrão ────────────────────────────────────── */
/* As formas das linhas das tabelas de vm/ps_c_padrao.inc. Os números de
 * 16 bits são posições em PS_C_TEXTO. */
typedef struct {
    uint16_t nome, decl;   /* o nome; o protótipo, a declaração ou o motivo */
    uint16_t idx;          /* a linha na tabela da espécie */
    uint16_t glibc;        /* o símbolo na glibc, quando é outro (0 = o nome) */
    uint8_t  cab, especie, marcas;
} PSCNome;
typedef struct { unsigned long long v; unsigned char tipo; } PSCConstInt;
typedef struct { long double v; unsigned char tipo; } PSCConstFlo;
typedef struct { size_t tam, alinha; } PSCOpacoPadrao;
typedef struct { uint16_t nome, decl; size_t desloc; } PSCCampoPadrao;
typedef struct { uint16_t primeiro, n; size_t tam, alinha; } PSCEstruturaPadrao;
typedef struct { uint16_t expansao; unsigned char tipo; } PSCValorPadrao;

/* o texto da expansão de uma macro, como o compilador a vê */
#define PS_C_STR(x) #x
#define PS_C_XSTR(x) PS_C_STR(x)

#include "ps_c_padrao.inc"

int ps_c_busca(const char *nome)
{
    int lo = 0, hi = PS_C_NNOMES - 1;
    while (lo <= hi) {
        int m = (lo + hi) / 2;
        int c = strcmp(nome, PS_C_TEXTO + PS_C_NOMES[m].nome);
        if (c == 0) return m;
        if (c < 0) hi = m - 1;
        else lo = m + 1;
    }
    return -1;
}

int ps_c_nnomes(void) { return PS_C_NNOMES; }

int ps_c_valor_indice(const char *nome)
{
    int i = ps_c_busca(nome);
    if (i < 0) return -1;
    switch (PS_C_NOMES[i].especie) {
        case PSC_NOME_FUNCAO:
        case PSC_NOME_MACRO:
        case PSC_NOME_LITERAL:
        case PSC_NOME_CONST:
        case PSC_NOME_CONSTFLO:
        case PSC_NOME_TEXTO:
        case PSC_NOME_VALOR:
        case PSC_NOME_NULO:
            return i;
        case PSC_NOME_TALVEZ:
            return PS_C_KTALVEZ[PS_C_NOMES[i].idx] ? i : -1;
        default:
            return -1;
    }
}

const char *ps_c_por_que_nao(const char *nome, char *buf, size_t cap)
{
    int i = ps_c_busca(nome);
    if (i < 0 || ps_c_valor_indice(nome) >= 0 || !buf || !cap) return NULL;
    const PSCNome *n = &PS_C_NOMES[i];
    const char *cab = PS_C_TEXTO + PS_C_CAB[n->cab];
    switch (n->especie) {
        case PSC_NOME_FORA:
            snprintf(buf, cap, "%s fica de fora: %s (doc: o C na Jinga)", nome, PS_C_TEXTO + n->decl);
            return buf;
        case PSC_NOME_TALVEZ:
            snprintf(buf, cap, "%s não está definida neste sistema (o %s a deixa opcional)", nome, cab);
            return buf;
        case PSC_NOME_TIPO:
        case PSC_NOME_OPACO:
        case PSC_NOME_ESTRUTURA:
            snprintf(buf, cap, "%s é tipo do C (%s): vale em declaração, cast e sizeof, e não como valor",
                     nome, cab);
            return buf;
        case PSC_NOME_SINTAXE:
            snprintf(buf, cap, "%s é operador do C: %s", nome, PS_C_TEXTO + n->decl);
            return buf;
        default:
            return NULL;
    }
}

static int fora_da_tabela(int i) { return i < 0 || i >= PS_C_NNOMES; }

const char *ps_c_nome(int i) { return fora_da_tabela(i) ? NULL : PS_C_TEXTO + PS_C_NOMES[i].nome; }
PSCEspecie ps_c_especie(int i) { return fora_da_tabela(i) ? (PSCEspecie)0 : (PSCEspecie)PS_C_NOMES[i].especie; }
const char *ps_c_decl(int i) { return fora_da_tabela(i) ? NULL : PS_C_TEXTO + PS_C_NOMES[i].decl; }
int ps_c_marcas(int i) { return fora_da_tabela(i) ? 0 : PS_C_NOMES[i].marcas; }

const char *ps_c_cabecalho(int i)
{
    return fora_da_tabela(i) ? NULL : PS_C_TEXTO + PS_C_CAB[PS_C_NOMES[i].cab];
}

/* ── o formato do printf e do scanf ───────────────────────────────────── */
/* Cada conversão consome um argumento (o `*` de largura e de precisão do
 * printf consome um int; o `*` do scanf, que suprime a gravação, nenhum), e o
 * tipo é o que a conversão pede com o modificador de tamanho, depois da
 * promoção das variádicas. Formato que pede mais ou menos argumentos do que
 * vieram é erro de quem chama (no C seria comportamento indefinido): a
 * contagem volta, e a frase é a de ps_c_frase_formato_conta. */
int ps_c_formato(const char *fn, const char *fmt, int eh_scanf, PSCTipo **tipos, char (*conv)[24],
                 int max, char *erro, size_t ecap, const char **tipo_erro)
{
#define FERRO(tipo, ...) do { snprintf(erro, ecap, __VA_ARGS__); *tipo_erro = (tipo); return -1; } while (0)
    int k = 0;
    for (const char *s = fmt; *s; s++) {
        if (*s != '%') continue;
        const char *ini = s++;
        if (*s == '%') continue;
        int suprime = 0;
        if (eh_scanf && *s == '*') { suprime = 1; s++; }
        /* bandeiras, largura e precisão */
        while (!eh_scanf && *s && strchr("-+ #0'", *s)) s++;
        for (int vez = 0; vez < 2; vez++) {
            if (vez == 1) { if (eh_scanf || *s != '.') break; s++; }
            if (!eh_scanf && *s == '*') {
                if (k < max) {
                    tipos[k] = &ps_c_int;
                    snprintf(conv[k], sizeof conv[k], "o * de %.10s", ini);
                }
                k++; s++;
            } else {
                while (*s >= '0' && *s <= '9') s++;
            }
        }
        /* o modificador de tamanho */
        char mod[3] = "";
        if ((s[0] == 'h' && s[1] == 'h') || (s[0] == 'l' && s[1] == 'l')) { mod[0] = s[0]; mod[1] = s[1]; s += 2; }
        else if (*s && strchr("hljztL", *s)) { mod[0] = *s; s++; }
        char c = *s;
        if (!c) FERRO("ValueError", "%s: o formato acaba no meio de uma conversão (%s)", fn, ini);
        int tam_conv = (int)(s - ini) + 1;
        PSCTipo *t = NULL;
        const char *grande = NULL;   /* o modificador do wchar_t: lote C2 */
        if (strchr("di", c) || strchr("ouxX", c) || c == 'n') {
            int sem = strchr("ouxX", c) != NULL;
            if (!strcmp(mod, "hh")) t = eh_scanf || c == 'n' ? (sem ? &ps_c_uchar : &ps_c_schar) : &ps_c_int;
            else if (!strcmp(mod, "h")) t = eh_scanf || c == 'n' ? (sem ? &ps_c_ushort : &ps_c_short) : &ps_c_int;
            else if (!strcmp(mod, "l")) t = sem ? &ps_c_ulong : &ps_c_long;
            else if (!strcmp(mod, "ll")) t = sem ? &ps_c_ullong : &ps_c_llong;
            else if (!strcmp(mod, "j")) t = ps_c_tipo_por_nome(sem ? "uintmax_t" : "intmax_t");
            else if (!strcmp(mod, "z")) t = ps_c_tipo_por_nome(sem ? "size_t" : "ptrdiff_t");
            else if (!strcmp(mod, "t")) t = ps_c_tipo_por_nome(sem ? "size_t" : "ptrdiff_t");
            else if (!mod[0]) t = sem ? &ps_c_uint : &ps_c_int;
            if (t && (eh_scanf || c == 'n')) t = ps_c_ponteiro(t);
        } else if (strchr("fFeEgGaA", c)) {
            if (eh_scanf) t = ps_c_ponteiro(!strcmp(mod, "L") ? &ps_c_ldouble : !strcmp(mod, "l") ? &ps_c_double : &ps_c_float);
            else t = !strcmp(mod, "L") ? &ps_c_ldouble : &ps_c_double;
        } else if (c == 'c' || c == 's' || c == '[') {
            if (!strcmp(mod, "l")) grande = "l";
            else if (eh_scanf) t = ps_c_ponteiro(&ps_c_char);
            else t = c == 'c' ? &ps_c_int : ps_c_ponteiro(ps_c_const(&ps_c_char));
            if (c == '[') {
                if (!eh_scanf) FERRO("ValueError", "%s: %%[ é conversão do scanf", fn);
                s++;
                if (*s == '^') s++;
                if (*s == ']') s++;
                while (*s && *s != ']') s++;
                if (!*s) FERRO("ValueError", "%s: falta o ] no formato (%s)", fn, ini);
                tam_conv = (int)(s - ini) + 1;
            }
        } else if (c == 'p') {
            t = eh_scanf ? ps_c_ponteiro(ps_c_ponteiro(&ps_c_void)) : ps_c_ponteiro(&ps_c_void);
        }
        if (grande)
            FERRO("NotImplementedError", "%s: %.*s é do texto largo (wchar.h), que entra no lote C2 do C",
                  fn, tam_conv, ini);
        if (!t) FERRO("ValueError", "%s: conversão desconhecida no formato: %.*s", fn, tam_conv, ini);
        if (suprime) continue;
        if (k < max) {
            tipos[k] = t;
            snprintf(conv[k], sizeof conv[k], "%.*s", tam_conv > 23 ? 23 : tam_conv, ini);
        }
        k++;
    }
    return k;
#undef FERRO
}

void ps_c_frase_formato_conta(char *buf, size_t cap, const char *fn, int k, int n)
{
    snprintf(buf, cap, "%s: o formato pede %d argumento%s depois dele, e %s %d", fn, k,
             k == 1 ? "" : "s", n == 1 ? "veio" : "vieram", n);
}

/* ── o endereço ───────────────────────────────────────────────────────── */
/* Guardado na primeira vez: vetores em .bss, que nascem zerados sem custar
 * nada na partida. */
static void (*g_end[PS_C_NNOMES])(void);
static unsigned char g_end_feito[PS_C_NNOMES];

#ifndef _WIN32
static void *g_programa = NULL;   /* dlopen(NULL): o programa e as bibliotecas que ele carregou */

static void *simbolo_do_sistema(const char *nome)
{
    if (!g_programa) g_programa = dlopen(NULL, RTLD_LAZY);
    return g_programa ? dlsym(g_programa, nome) : NULL;
}
#endif

/* O símbolo do sistema pelo nome, pra função que o binário NÃO importa: o
 * binário é ligado com BIND_NOW, e cada nome importado é resolvido na
 * partida de todo programa (medido no `partida` do bench: 2 nomes a mais,
 * 11.165 instruções, +0,7%). O que só quem usa o C precisa (o `sigaltstack`
 * do tratador de queda) é achado aqui, no 1º uso. */
void *ps_c_simbolo(const char *nome)
{
#ifndef _WIN32
    return simbolo_do_sistema(nome);
#else
    (void)nome;
    return NULL;
#endif
}

void (*ps_c_endereco(int i))(void)
{
    if (fora_da_tabela(i)) return NULL;
    if (g_end_feito[i]) return g_end[i];
    const PSCNome *n = &PS_C_NOMES[i];
    void (*f)(void) = NULL;
    switch (n->especie) {
        case PSC_NOME_FUNCAO:
            if (n->marcas & PSC_MARCA_VM) break;
#ifdef _WIN32
            f = ps_c_endereco_win(i);
#else
            {
                const char *simbolo = PS_C_TEXTO + n->nome;
#ifdef __GLIBC__
                if (n->glibc) simbolo = PS_C_TEXTO + n->glibc;
#endif
                f = (void (*)(void))simbolo_do_sistema(simbolo);
            }
#endif
            break;
        case PSC_NOME_MACRO:
        case PSC_NOME_LITERAL:
        case PSC_NOME_VALOR:
            f = ps_c_local(i);
            break;
        default:
            break;
    }
    g_end[i] = f;
    g_end_feito[i] = 1;
    return f;
}

/* ── as constantes ────────────────────────────────────────────────────── */
int ps_c_const_int(int i, PSCTipo **tipo, uint64_t *valor)
{
    if (fora_da_tabela(i)) return -1;
    const PSCNome *n = &PS_C_NOMES[i];
    if (n->especie == PSC_NOME_CONST) {
        *valor = (uint64_t)PS_C_KINT[n->idx].v;
        *tipo = escalar_de(PS_C_KINT[n->idx].tipo);
        return *tipo ? 0 : -1;
    }
    if (n->especie == PSC_NOME_TALVEZ && PS_C_KTALVEZ[n->idx]) {
        *valor = 1;
        *tipo = &ps_c_int;
        return 0;
    }
    return -1;
}

int ps_c_const_flo(int i, PSCTipo **tipo, long double *valor)
{
    if (fora_da_tabela(i) || PS_C_NOMES[i].especie != PSC_NOME_CONSTFLO) return -1;
    const PSCConstFlo *k = &PS_C_KFLO[PS_C_NOMES[i].idx];
    *valor = k->v;
    *tipo = escalar_de(k->tipo);
    return *tipo ? 0 : -1;
}

const char *ps_c_const_texto(int i)
{
    if (fora_da_tabela(i) || PS_C_NOMES[i].especie != PSC_NOME_TEXTO) return NULL;
    return PS_C_KTEXTO + PS_C_KTEXTO_POS[PS_C_NOMES[i].idx];
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

/* "int (const void *, const void *)" com `meio` "" ou "(*)" no lugar do nome */
static char *nome_de_funcao(PSCTipo *ret, PSCTipo **params, int n, int variadica, const char *meio)
{
    size_t tam = strlen(ret->nome) + strlen(meio) + 16;
    for (int i = 0; i < n; i++) tam += strlen(params[i]->nome) + 2;
    char *s = malloc(tam);
    if (!s) return NULL;
    size_t p = (size_t)snprintf(s, tam, "%s %s(", ret->nome, meio);
    if (n == 0 && !variadica) p += (size_t)snprintf(s + p, tam - p, "void");
    for (int i = 0; i < n; i++)
        p += (size_t)snprintf(s + p, tam - p, "%s%s", i ? ", " : "", params[i]->nome);
    if (variadica) p += (size_t)snprintf(s + p, tam - p, "%s...", n ? ", " : "");
    snprintf(s + p, tam - p, ")");
    return s;
}

PSCTipo *ps_c_ponteiro(PSCTipo *alvo)
{
    nomeia_escalares();
    for (Montado *m = g_montados; m; m = m->prox)
        if (m->t.kind == PSC_PONT && m->t.alvo == alvo && !m->t.constante) return &m->t;
    PSCTipo *t = novo_montado();
    if (!t) return NULL;
    t->kind = PSC_PONT;
    t->alvo = alvo;
    t->tam = sizeof(void *);
    t->alinha = _Alignof(void *);
    t->sem_sinal = 1;
    if (alvo->kind == PSC_FUNC)
        t->nome = nome_de_funcao(alvo->ret, alvo->params, alvo->nparams, alvo->variadica, "(*)");
    else
        t->nome = junta_nome(alvo->nome, alvo->kind == PSC_PONT ? "*" : " *");
    return t;
}

/* `const T`. No ponteiro o const é do próprio ponteiro e vem depois do `*`
 * ("char *const"); nos outros vem antes ("const char"). */
PSCTipo *ps_c_const(PSCTipo *base)
{
    nomeia_escalares();
    if (base->constante) return base;
    for (Montado *m = g_montados; m; m = m->prox)
        if (m->t.constante && m->t.sem_const == base) return &m->t;
    PSCTipo *t = novo_montado();
    if (!t) return NULL;
    *t = *base;
    t->constante = 1;
    t->sem_const = base;
    t->cif = NULL;
    t->ffi = NULL;
    t->nome = base->kind == PSC_PONT ? junta_nome(base->nome, "const") : junta_nome("const ", base->nome);
    return t;
}

PSCTipo *ps_c_array(PSCTipo *elem, size_t n)
{
    nomeia_escalares();
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
    if (n) snprintf(suf, sizeof suf, "[%zu]", n);
    else snprintf(suf, sizeof suf, "[]");
    t->nome = junta_nome(elem->nome, suf);
    return t;
}

PSCTipo *ps_c_funcao(PSCTipo *ret, PSCTipo **params, int nparams, int variadica)
{
    nomeia_escalares();
    for (Montado *m = g_montados; m; m = m->prox) {
        PSCTipo *f = &m->t;
        if (f->kind != PSC_FUNC || f->ret != ret || f->nparams != nparams || f->variadica != variadica)
            continue;
        int igual = 1;
        for (int i = 0; i < nparams && igual; i++) igual = f->params[i] == params[i];
        if (igual) return f;
    }
    PSCTipo *t = novo_montado();
    if (!t) return NULL;
    t->kind = PSC_FUNC;
    t->ret = ret;
    t->nparams = nparams;
    t->variadica = variadica;
    if (nparams > 0) {
        t->params = malloc((size_t)nparams * sizeof *t->params);
        if (!t->params) return NULL;
        memcpy(t->params, params, (size_t)nparams * sizeof *t->params);
    }
    t->nome = nome_de_funcao(ret, params, nparams, variadica, "");
    return t;
}

PSCTipo *ps_c_opaco(const char *nome, size_t tam, size_t alinha)
{
    PSCTipo *t = novo_montado();
    if (!t) return NULL;
    t->kind = PSC_OPACO;
    t->nome = nome;
    t->tam = tam;
    t->alinha = alinha ? alinha : 1;
    return t;
}

/* ── a ponte pra libffi ───────────────────────────────────────────────── */
static ffi_type *ffi_de(PSCTipo *t);

/* a regra do C: cada campo no próximo múltiplo do alinhamento dele (no
 * union, todos no começo), e o tamanho arredondado pro maior alinhamento */
static void monta_layout(PSCTipo *t)
{
    int eh_union = t->kind == PSC_UNION;
    size_t pos = 0, maior = 1, maior_tam = 0;
    for (int i = 0; i < t->ncampos; i++) {
        PSCTipo *ti = t->campos[i].tipo;
        size_t a = ti->alinha ? ti->alinha : 1;
        if (a > maior) maior = a;
        if (eh_union) {
            t->campos[i].desloc = 0;
            if (ti->tam > maior_tam) maior_tam = ti->tam;
        } else {
            pos = (pos + a - 1) / a * a;
            t->campos[i].desloc = pos;
            pos += ti->tam;
        }
    }
    if (eh_union) pos = maior_tam;
    t->alinha = maior;
    t->tam = (pos + maior - 1) / maior * maior;
}

PSCTipo *ps_c_struct(const char *nome, int eh_union, const char **nomes, PSCTipo **tipos,
                     int n, const size_t *deslocs, size_t tam, size_t alinha)
{
    PSCTipo *t = novo_montado();
    if (!t) return NULL;
    t->kind = eh_union ? PSC_UNION : PSC_STRUCT;
    t->nome = nome;
    /* o vetor antes do contador: quem lê `ncampos` conta com `campos` pronto */
    t->campos = calloc((size_t)(n > 0 ? n : 1), sizeof *t->campos);
    if (!t->campos) return NULL;
    t->ncampos = n;
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
    monta_layout(t);
    return t;
}

/* ── os tipos da tabela ───────────────────────────────────────────────── */
static PSCTipo *g_tipo[PS_C_NNOMES];   /* o tipo de cada nome, montado na primeira vez */

/* typedef: o nome dele, com a forma do escalar que ele é neste sistema */
static PSCTipo *typedef_de(const PSCNome *n)
{
    PSCTipo *e = escalar_de(PS_C_TIPOS[n->idx]);
    if (!e) return NULL;
    PSCTipo *t = novo_montado();
    if (!t) return NULL;
    *t = *e;
    t->nome = PS_C_TEXTO + n->nome;
    t->canon = e;
    return t;
}

static PSCTipo *estrutura_de(const PSCNome *n)
{
    const PSCEstruturaPadrao *e = &PS_C_ESTRUTURAS[n->idx];
    const char *nomes[16];
    PSCTipo *tipos[16];
    size_t deslocs[16];
    if (e->n > 16) return NULL;
    for (int k = 0; k < e->n; k++) {
        const PSCCampoPadrao *c = &PS_C_CAMPOS[e->primeiro + k];
        nomes[k] = PS_C_TEXTO + c->nome;
        tipos[k] = ps_c_le_decl(PS_C_TEXTO + c->decl, NULL, 0, NULL, 0);
        if (!tipos[k]) return NULL;
        deslocs[k] = c->desloc;
    }
    return ps_c_struct(PS_C_TEXTO + n->nome, 0, nomes, tipos, e->n, deslocs, e->tam, e->alinha);
}

PSCTipo *ps_c_tipo_de(int i)
{
    nomeia_escalares();
    if (fora_da_tabela(i)) return NULL;
    if (g_tipo[i]) return g_tipo[i];
    const PSCNome *n = &PS_C_NOMES[i];
    PSCTipo *t = NULL;
    switch (n->especie) {
        case PSC_NOME_FUNCAO:
        case PSC_NOME_MACRO:
        case PSC_NOME_LITERAL:
        case PSC_NOME_VALOR:
            t = ps_c_le_decl(PS_C_TEXTO + n->decl, NULL, 0, NULL, 0);
            break;
        case PSC_NOME_TIPO:
            t = typedef_de(n);
            break;
        case PSC_NOME_OPACO:
            t = ps_c_opaco(PS_C_TEXTO + n->nome, PS_C_OPACOS[n->idx].tam, PS_C_OPACOS[n->idx].alinha);
            break;
        case PSC_NOME_ESTRUTURA:
            t = estrutura_de(n);
            break;
        default:
            break;
    }
    g_tipo[i] = t;
    return t;
}

/* ── o leitor de declarações do C ─────────────────────────────────────────
 * O que o compilador C faz com uma declaração (C11 6.7): os especificadores
 * dão o tipo de base, e o declarador monta em volta dele de dentro pra fora,
 * com o `[]` e o `()` ligando mais forte que o `*`. É o `dcl` do K&R (seção
 * 5.12) com a gramática do declarador do C11 (6.7.6). */
typedef enum { LT_FIM, LT_NOME, LT_NUM, LT_SIMB } LTipo;
typedef struct {
    const char *ini;     /* o texto inteiro, pra mensagem de erro */
    const char *p;       /* onde o próximo token começa a ser procurado */
    const char *tok;     /* o token atual */
    size_t      ntok;
    LTipo       tipo;
    char       *erro;
    size_t      ecap;
    int         falhou;
} Leitor;

static void lt_avanca(Leitor *l)
{
    const char *p = l->p;
    while (*p == ' ' || *p == '\t' || *p == '\n') p++;
    l->tok = p;
    if (!*p) {
        l->tipo = LT_FIM;
        l->ntok = 0;
    } else if (isalpha((unsigned char)*p) || *p == '_') {
        const char *q = p;
        while (isalnum((unsigned char)*q) || *q == '_') q++;
        l->tipo = LT_NOME;
        l->ntok = (size_t)(q - p);
    } else if (isdigit((unsigned char)*p)) {
        const char *q = p;
        while (isalnum((unsigned char)*q)) q++;
        l->tipo = LT_NUM;
        l->ntok = (size_t)(q - p);
    } else if (p[0] == '.' && p[1] == '.' && p[2] == '.') {
        l->tipo = LT_SIMB;
        l->ntok = 3;
    } else {
        l->tipo = LT_SIMB;
        l->ntok = 1;
    }
    l->p = p + l->ntok;
}

static int lt_eh(const Leitor *l, const char *s)
{
    size_t n = strlen(s);
    return l->ntok == n && strncmp(l->tok, s, n) == 0;
}

static void lt_erro(Leitor *l, const char *msg)
{
    if (l->falhou) return;
    l->falhou = 1;
    if (l->erro && l->ecap) {
        /* texto comprido sai cortado no fim; o motivo vem no começo */
        int n = snprintf(l->erro, l->ecap, "%s, na posição %d de \"%s\"", msg, (int)(l->tok - l->ini), l->ini);
        if (n < 0) l->erro[0] = '\0';
    }
}

/* ── os tipos que o programa declara (ps_c.h) ─────────────────────────── */
typedef struct Declarado { char *nome; PSCTipo *t; struct Declarado *prox; } Declarado;
static Declarado *g_declarados = NULL;

PSCTipo *ps_c_tipo_declarado(const char *nome)
{
    if (!nome || !*nome) return NULL;
    for (Declarado *d = g_declarados; d; d = d->prox)
        if (d->nome[0] == nome[0] && strcmp(d->nome, nome) == 0) return d->t;
    return NULL;
}

const char *ps_c_declarado_nome(int i)
{
    for (Declarado *d = g_declarados; d; d = d->prox)
        if (i-- == 0) return d->nome;
    return NULL;
}

static int registra_declarado(const char *nome, PSCTipo *t)
{
    Declarado *d = calloc(1, sizeof *d);
    if (!d) return -1;
    d->nome = strdup(nome);
    if (!d->nome) { free(d); return -1; }
    d->t = t;
    d->prox = g_declarados;
    g_declarados = d;
    return 0;
}

/* o tipo que um nome da tabela dá (typedef, opaco, struct), ou NULL; depois
 * da tabela, o que o programa declarou */
static PSCTipo *tipo_da_tabela(const char *nome)
{
    int i = ps_c_busca(nome);
    if (i < 0) return ps_c_tipo_declarado(nome);
    PSCEspecie e = (PSCEspecie)PS_C_NOMES[i].especie;
    if (e != PSC_NOME_TIPO && e != PSC_NOME_OPACO && e != PSC_NOME_ESTRUTURA) return NULL;
    return ps_c_tipo_de(i);
}

static PSCTipo *tipo_do_token(const Leitor *l)
{
    char nome[64];
    if (l->tipo != LT_NOME || l->ntok >= sizeof nome) return NULL;
    memcpy(nome, l->tok, l->ntok);
    nome[l->ntok] = '\0';
    return tipo_da_tabela(nome);
}

static int eh_palavra_de_tipo(const Leitor *l)
{
    static const char palavras[] =
        " void char short int long float double signed unsigned _Bool const volatile restrict"
        " struct union _Noreturn inline extern static register auto ";
    if (l->tipo != LT_NOME) return 0;
    char busca[40];
    if (l->ntok + 3 > sizeof busca) return 0;
    busca[0] = ' ';
    memcpy(busca + 1, l->tok, l->ntok);
    busca[l->ntok + 1] = ' ';
    busca[l->ntok + 2] = '\0';
    return strstr(palavras, busca) != NULL || tipo_do_token(l) != NULL;
}

/* os especificadores e qualificadores do começo da declaração -> o tipo de base */
static PSCTipo *le_especificadores(Leitor *l)
{
    int n_long = 0, curto = 0, sinal = 0, constante = 0;
    int base = 0;   /* 'v' void, 'c' char, 'i' int, 'f' float, 'd' double, 'b' _Bool */
    PSCTipo *nomeado = NULL;
    while (l->tipo == LT_NOME) {
        int tem_tipo = base || n_long || curto || sinal || nomeado;
        if (lt_eh(l, "const")) constante = 1;
        else if (lt_eh(l, "volatile") || lt_eh(l, "restrict") || lt_eh(l, "_Noreturn")
                 || lt_eh(l, "inline") || lt_eh(l, "extern") || lt_eh(l, "static")
                 || lt_eh(l, "register") || lt_eh(l, "auto")) {
        }
        else if (lt_eh(l, "void")) base = 'v';
        else if (lt_eh(l, "char")) base = 'c';
        else if (lt_eh(l, "int")) { if (!base) base = 'i'; }
        else if (lt_eh(l, "float")) base = 'f';
        else if (lt_eh(l, "double")) base = 'd';
        else if (lt_eh(l, "_Bool")) base = 'b';
        else if (lt_eh(l, "short")) curto = 1;
        else if (lt_eh(l, "long")) n_long++;
        else if (lt_eh(l, "signed")) sinal = 1;
        else if (lt_eh(l, "unsigned")) sinal = 2;
        else if (lt_eh(l, "struct") || lt_eh(l, "union")) {
            int eh_union = lt_eh(l, "union");
            lt_avanca(l);
            if (l->tipo != LT_NOME) { lt_erro(l, "falta o nome do struct"); return NULL; }
            char etiqueta[80];
            int ne = snprintf(etiqueta, sizeof etiqueta, "%s %.*s", eh_union ? "union" : "struct",
                              (int)l->ntok, l->tok);
            if (ne < 0 || ne >= (int)sizeof etiqueta) { lt_erro(l, "nome de struct comprido demais"); return NULL; }
            nomeado = tipo_da_tabela(etiqueta);
            if (!nomeado) { lt_erro(l, "struct desconhecido"); return NULL; }
        }
        else if (!tem_tipo && (nomeado = tipo_do_token(l)) != NULL) {
        }
        else break;   /* o nome do declarador */
        lt_avanca(l);
    }
    PSCTipo *t = NULL;
    if (nomeado) {
        if (base || n_long || curto || sinal) { lt_erro(l, "tipo com dois nomes"); return NULL; }
        t = nomeado;
    } else if (base == 'v') t = &ps_c_void;
    else if (base == 'b') t = &ps_c_bool;
    else if (base == 'f') t = &ps_c_float;
    else if (base == 'd') t = n_long ? &ps_c_ldouble : &ps_c_double;
    else if (base == 'c') t = sinal == 1 ? &ps_c_schar : sinal == 2 ? &ps_c_uchar : &ps_c_char;
    else if (base == 'i' || n_long || curto || sinal) {
        if (curto) t = sinal == 2 ? &ps_c_ushort : &ps_c_short;
        else if (n_long >= 2) t = sinal == 2 ? &ps_c_ullong : &ps_c_llong;
        else if (n_long == 1) t = sinal == 2 ? &ps_c_ulong : &ps_c_long;
        else t = sinal == 2 ? &ps_c_uint : &ps_c_int;
    } else {
        lt_erro(l, "falta o tipo");
        return NULL;
    }
    return constante ? ps_c_const(t) : t;
}

static PSCTipo *le_declarador(Leitor *l, PSCTipo *base, char *nome, size_t cap, int prof);

typedef struct {
    int       funcao;      /* 1 = (parâmetros); 0 = [n] */
    size_t    n;
    PSCTipo **params;
    int       nparams, variadica;
} Sufixo;

/* "(" seguido de "*", de "(" ou de um nome que não é tipo: é um declarador
 * entre parênteses, como em `int (*compar)(...)`. Senão é a lista de
 * parâmetros de uma função. */
static int comeca_aninhado(const Leitor *l)
{
    Leitor c = *l;
    lt_avanca(&c);
    if (lt_eh(&c, "*") || lt_eh(&c, "(")) return 1;
    return c.tipo == LT_NOME && !eh_palavra_de_tipo(&c);
}

static int le_parametros(Leitor *l, Sufixo *s, int prof)
{
    PSCTipo *ps[32];
    int n = 0;
    if (lt_eh(l, ")")) { lt_avanca(l); return 1; }
    if (lt_eh(l, "void")) {
        Leitor c = *l;
        lt_avanca(&c);
        if (lt_eh(&c, ")")) { *l = c; lt_avanca(l); return 1; }
    }
    for (;;) {
        if (lt_eh(l, "...")) {
            if (n == 0) { lt_erro(l, "o ... precisa de um parâmetro antes"); return 0; }
            s->variadica = 1;
            lt_avanca(l);
            break;
        }
        PSCTipo *b = le_especificadores(l);
        if (!b) return 0;
        PSCTipo *t = le_declarador(l, b, NULL, 0, prof + 1);
        if (!t) return 0;
        /* parâmetro array é ponteiro, e parâmetro função é ponteiro de função (C11 6.7.6.3) */
        if (t->kind == PSC_ARRAY) t = ps_c_ponteiro(t->alvo);
        else if (t->kind == PSC_FUNC) t = ps_c_ponteiro(t);
        if (!t) return 0;
        if (n == (int)(sizeof ps / sizeof ps[0])) { lt_erro(l, "parâmetros demais"); return 0; }
        ps[n++] = t;
        if (!lt_eh(l, ",")) break;
        lt_avanca(l);
    }
    if (!lt_eh(l, ")")) { lt_erro(l, "esperava , ou ) na lista de parâmetros"); return 0; }
    lt_avanca(l);
    if (n > 0) {
        s->params = malloc((size_t)n * sizeof *s->params);
        if (!s->params) return 0;
        memcpy(s->params, ps, (size_t)n * sizeof *ps);
    }
    s->nparams = n;
    return 1;
}

static PSCTipo *le_direto(Leitor *l, PSCTipo *base, char *nome, size_t cap, int prof)
{
    Leitor dentro;
    int tem_dentro = 0;
    if (lt_eh(l, "(") && comeca_aninhado(l)) {
        lt_avanca(l);
        dentro = *l;              /* o declarador de dentro começa aqui */
        tem_dentro = 1;
        int nivel = 1;
        for (;;) {
            if (l->tipo == LT_FIM) { lt_erro(l, "falta o )"); return NULL; }
            if (lt_eh(l, "(")) nivel++;
            else if (lt_eh(l, ")") && --nivel == 0) break;
            lt_avanca(l);
        }
        lt_avanca(l);
    } else if (l->tipo == LT_NOME) {
        if (nome && cap) {
            if (l->ntok >= cap) { lt_erro(l, "nome comprido demais"); return NULL; }
            memcpy(nome, l->tok, l->ntok);
            nome[l->ntok] = '\0';
        }
        lt_avanca(l);
    }
    Sufixo suf[8];
    int ns = 0, ok = 1;
    while (ok && (lt_eh(l, "[") || lt_eh(l, "("))) {
        if (ns == (int)(sizeof suf / sizeof suf[0])) { lt_erro(l, "declarador comprido demais"); ok = 0; break; }
        Sufixo *s = &suf[ns++];
        memset(s, 0, sizeof *s);
        if (lt_eh(l, "[")) {
            lt_avanca(l);
            if (l->tipo == LT_NUM) {
                s->n = (size_t)strtoull(l->tok, NULL, 0);
                lt_avanca(l);
            }
            if (!lt_eh(l, "]")) { lt_erro(l, "falta o ]"); ok = 0; break; }
            lt_avanca(l);
        } else {
            s->funcao = 1;
            lt_avanca(l);
            ok = le_parametros(l, s, prof);
        }
    }
    /* o sufixo mais perto do nome é o de fora: `int a[2][3]` é array de 2
     * arrays de 3; então se aplica do último pro primeiro */
    for (int i = ns - 1; i >= 0; i--) {
        if (ok && base) {
            if (suf[i].funcao) base = ps_c_funcao(base, suf[i].params, suf[i].nparams, suf[i].variadica);
            else base = ps_c_array(base, suf[i].n);
        }
        free(suf[i].params);
    }
    if (!ok || !base) return NULL;
    if (tem_dentro) {
        base = le_declarador(&dentro, base, nome, cap, prof + 1);
        if (!base || dentro.falhou) { l->falhou = 1; return NULL; }
        if (!lt_eh(&dentro, ")")) { lt_erro(&dentro, "sobrou texto dentro dos parênteses"); l->falhou = 1; return NULL; }
    }
    return base;
}

static PSCTipo *le_declarador(Leitor *l, PSCTipo *base, char *nome, size_t cap, int prof)
{
    if (prof > 32) { lt_erro(l, "declarador aninhado demais"); return NULL; }
    while (base && lt_eh(l, "*")) {
        lt_avanca(l);
        base = ps_c_ponteiro(base);
        while (base && (lt_eh(l, "const") || lt_eh(l, "volatile") || lt_eh(l, "restrict"))) {
            if (lt_eh(l, "const")) base = ps_c_const(base);
            lt_avanca(l);
        }
    }
    if (!base) return NULL;
    return le_direto(l, base, nome, cap, prof);
}

PSCTipo *ps_c_le_decl(const char *texto, char *nome, size_t cap, char *erro, size_t ecap)
{
    nomeia_escalares();
    Leitor l;
    memset(&l, 0, sizeof l);
    l.ini = l.p = texto;
    l.erro = erro;
    l.ecap = ecap;
    if (nome && cap) nome[0] = '\0';
    if (erro && ecap) erro[0] = '\0';
    lt_avanca(&l);
    PSCTipo *b = le_especificadores(&l);
    if (!b) return NULL;
    PSCTipo *t = le_declarador(&l, b, nome, cap, 0);
    if (!t || l.falhou) return NULL;
    if (lt_eh(&l, ";")) lt_avanca(&l);
    if (l.tipo != LT_FIM) { lt_erro(&l, "sobrou texto"); return NULL; }
    return t;
}

PSCTipo *ps_c_tipo_por_nome(const char *nome)
{
    char declarado[8];
    PSCTipo *t = ps_c_le_decl(nome, declarado, sizeof declarado, NULL, 0);
    return t && declarado[0] == '\0' ? t : NULL;
}

/* ── struct, union e typedef do programa (ps_c.h) ─────────────────────── */
static void poe_erro(char *erro, size_t ecap, const char *msg)
{
    if (erro && ecap) snprintf(erro, ecap, "%s", msg);
}

PSCTipo *ps_c_struct_abre(const char *etiqueta, int eh_union, char *erro, size_t ecap)
{
    nomeia_escalares();
    if (erro && ecap) erro[0] = '\0';
    PSCKind kind = eh_union ? PSC_UNION : PSC_STRUCT;
    if (etiqueta) {
        PSCTipo *ja = ps_c_tipo_declarado(etiqueta);
        if (ja) {
            if (ja->kind == kind) return ja;
            char m[200];
            snprintf(m, sizeof m, "%s já foi declarado como %s", etiqueta, ja->kind == PSC_UNION ? "union" : "struct");
            poe_erro(erro, ecap, m);
            return NULL;
        }
        if (ps_c_busca(etiqueta) >= 0) {
            char m[200];
            snprintf(m, sizeof m, "%s é da biblioteca padrão do C e não se redeclara", etiqueta);
            poe_erro(erro, ecap, m);
            return NULL;
        }
    }
    PSCTipo *t = novo_montado();
    if (!t) { poe_erro(erro, ecap, "sem memoria"); return NULL; }
    t->kind = kind;
    t->alinha = 1;
    if (etiqueta) {
        t->nome = strdup(etiqueta);
        if (!t->nome || registra_declarado(etiqueta, t) != 0) { poe_erro(erro, ecap, "sem memoria"); return NULL; }
    }
    return t;
}

int ps_c_struct_fecha(PSCTipo *t, const char *campos, char *erro, size_t ecap)
{
    enum { MAX_CAMPOS = 64 };
    const char *nomes[MAX_CAMPOS];
    PSCTipo *tipos[MAX_CAMPOS];
    int n = 0, rc = -1;
    char nome[64];
    Leitor l;
    memset(&l, 0, sizeof l);
    l.ini = l.p = campos;
    l.erro = erro;
    l.ecap = ecap;
    if (erro && ecap) erro[0] = '\0';
    lt_avanca(&l);
    while (l.tipo != LT_FIM) {
        if (n == MAX_CAMPOS) { lt_erro(&l, "campos demais (o máximo é 64)"); goto fim; }
        PSCTipo *b = le_especificadores(&l);
        if (!b) goto fim;
        PSCTipo *ct = le_declarador(&l, b, nome, sizeof nome, 0);
        if (!ct || l.falhou) goto fim;
        if (!nome[0]) { lt_erro(&l, "campo sem nome"); goto fim; }
        PSCTipo *nu = ct;
        while (nu->canon || nu->sem_const) nu = nu->canon ? nu->canon : nu->sem_const;
        if (nu->kind == PSC_VOID || nu->kind == PSC_FUNC || nu->kind == PSC_OPACO) {
            lt_erro(&l, "campo com tipo que não guarda valor (use um ponteiro)"); goto fim;
        }
        if ((nu->kind == PSC_STRUCT || nu->kind == PSC_UNION) && nu->tam == 0) {
            lt_erro(&l, nu == t ? "o struct dentro dele mesmo tem que ser ponteiro"
                               : "campo de um struct ainda sem campos");
            goto fim;
        }
        if (nu->kind == PSC_ARRAY && nu->tam == 0) { lt_erro(&l, "o array do campo precisa do tamanho"); goto fim; }
        for (int i = 0; i < n; i++)
            if (strcmp(nomes[i], nome) == 0) { lt_erro(&l, "campo repetido"); goto fim; }
        nomes[n] = strdup(nome);
        if (!nomes[n]) { lt_erro(&l, "sem memoria"); goto fim; }
        tipos[n] = ct;
        n++;
        if (lt_eh(&l, ";")) lt_avanca(&l);
        else if (l.tipo != LT_FIM) { lt_erro(&l, "falta o ; entre os campos"); goto fim; }
    }
    if (n == 0) { lt_erro(&l, "struct sem campo"); goto fim; }
    if (t->ncampos > 0) {
        /* já estava completo: a redeclaração tem que ser igual */
        int igual = t->ncampos == n;
        for (int i = 0; igual && i < n; i++)
            igual = strcmp(t->campos[i].nome, nomes[i]) == 0 && t->campos[i].tipo == tipos[i];
        if (!igual) {
            char m[200];
            snprintf(m, sizeof m, "%s já foi declarado com outros campos", t->nome ? t->nome : "o struct");
            poe_erro(erro, ecap, m);
            goto fim;
        }
        rc = 0;
        goto fim;
    }
    t->campos = calloc((size_t)n, sizeof *t->campos);
    if (!t->campos) { poe_erro(erro, ecap, "sem memoria"); goto fim; }
    for (int i = 0; i < n; i++) {
        t->campos[i].nome = nomes[i];
        t->campos[i].tipo = tipos[i];
        nomes[i] = NULL;                           /* agora são do tipo */
    }
    t->ncampos = n;
    monta_layout(t);
    rc = 0;
fim:
    for (int i = 0; i < n; i++) free((char *)nomes[i]);
    return rc;
}

PSCTipo *ps_c_typedef(const char *nome, PSCTipo *alvo, char *erro, size_t ecap)
{
    nomeia_escalares();
    if (erro && ecap) erro[0] = '\0';
    char m[200];
    if (ps_c_busca(nome) >= 0) {
        snprintf(m, sizeof m, "%s é nome da biblioteca padrão do C", nome);
        poe_erro(erro, ecap, m);
        return NULL;
    }
    PSCTipo *ja = ps_c_tipo_declarado(nome);
    if (ja) {
        if (ja == alvo || ja->canon == alvo) return ja;
        snprintf(m, sizeof m, "%s já foi declarado como %s", nome,
                 ja->canon && ja->canon->nome ? ja->canon->nome : ja->nome ? ja->nome : "outro tipo");
        poe_erro(erro, ecap, m);
        return NULL;
    }
    if ((alvo->kind == PSC_STRUCT || alvo->kind == PSC_UNION) && !alvo->nome) {
        /* o struct sem etiqueta ganha o nome do typedef: é o próprio tipo */
        alvo->nome = strdup(nome);
        if (!alvo->nome || registra_declarado(nome, alvo) != 0) { poe_erro(erro, ecap, "sem memoria"); return NULL; }
        return alvo;
    }
    PSCTipo *t = novo_montado();
    if (!t) { poe_erro(erro, ecap, "sem memoria"); return NULL; }
    *t = *alvo;
    t->nome = strdup(nome);
    t->canon = alvo->canon ? alvo->canon : alvo;
    t->cif = NULL;
    t->ffi = NULL;
    if (!t->nome || registra_declarado(nome, t) != 0) { poe_erro(erro, ecap, "sem memoria"); return NULL; }
    return t;
}

/* ── a chamada ────────────────────────────────────────────────────────── */
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
            if (t->tam == sizeof(double) && (t->canon ? t->canon : t) != &ps_c_ldouble) return &ffi_type_double;
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

/* Struct por valor só vai pela libffi se a forma que ela calcula (a regra do
 * C sobre os campos que o padrão lista) é a forma do sistema: um struct do
 * sistema com campo a mais teria outro tamanho, e a chamada leria memória
 * errada. A libffi preenche o tamanho no preparo do cif. */
static int struct_confere(PSCTipo *t)
{
    if (t->kind != PSC_STRUCT) return 1;
    ffi_type *ft = (ffi_type *)t->ffi;
    return ft && ft->size == t->tam;
}

static int cif_confere(PSCTipo *f, int nargs, PSCTipo **tipos_var)
{
    if (!struct_confere(f->ret)) return 0;
    for (int i = 0; i < nargs; i++)
        if (!struct_confere(i < f->nparams ? f->params[i] : tipos_var[i - f->nparams])) return 0;
    return 1;
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
    if (ps_c_pronto(NULL, 0) != 0) return -1;
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
            if (ffi_prep_cif(cif, FFI_DEFAULT_ABI, (unsigned)f->nparams, rt, tps) != FFI_OK
                || !cif_confere(f, f->nparams, NULL)) {
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
    if (rc == 0 && (ffi_prep_cif_var(&cif, FFI_DEFAULT_ABI, (unsigned)f->nparams, (unsigned)nargs,
                                     rt, tps) != FFI_OK || !cif_confere(f, nargs, tipos_var))) rc = -1;
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
    if (ps_c_pronto(NULL, 0) != 0) return NULL;
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
    if (ffi_prep_cif(&f->cif, FFI_DEFAULT_ABI, (unsigned)ftipo->nparams, rt, f->tps) != FFI_OK
        || !cif_confere(ftipo, ftipo->nparams, NULL)) {
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

/* ── os valores lidos na hora ─────────────────────────────────────────── */
/* um inteiro guardado na memória do C, no tamanho e no sinal do tipo */
static uint64_t le_inteiro(const void *p, const PSCTipo *t)
{
    switch (t->tam) {
        case 1: { uint8_t v; memcpy(&v, p, 1); return t->sem_sinal ? v : (uint64_t)(int64_t)(int8_t)v; }
        case 2: { uint16_t v; memcpy(&v, p, 2); return t->sem_sinal ? v : (uint64_t)(int64_t)(int16_t)v; }
        case 4: { uint32_t v; memcpy(&v, p, 4); return t->sem_sinal ? v : (uint64_t)(int64_t)(int32_t)v; }
        default: { uint64_t v = 0; memcpy(&v, p, t->tam < 8 ? t->tam : 8); return v; }
    }
}

/* grava no tamanho do tipo: o que passa dá a volta, como a atribuição do C */
static void grava_inteiro(const PSCTipo *t, void *dest, uint64_t x)
{
    switch (t->tam) {
        case 1: { uint8_t v = (uint8_t)x; memcpy(dest, &v, 1); break; }
        case 2: { uint16_t v = (uint16_t)x; memcpy(dest, &v, 2); break; }
        case 4: { uint32_t v = (uint32_t)x; memcpy(dest, &v, 4); break; }
        default: memcpy(dest, &x, t->tam < 8 ? t->tam : 8); break;
    }
}

#ifndef _WIN32
/* O valor de uma macro pelo texto da expansão dela, nas formas que o
 * MB_CUR_MAX e o math_errhandling têm nos sistemas do motor:
 * `(__ctype_get_mb_cur_max ())`, `__mb_cur_max`, `(__math_errhandling())`,
 * `(1 | 2)`. A gramática é só a que essas formas pedem: número, `|`,
 * parênteses, nome de variável (lida no tipo da expressão) e chamada sem
 * argumento (que devolve o tipo da expressão). */
static int expansao_expr(Leitor *l, PSCTipo *tipo, uint64_t *v);

static int expansao_termo(Leitor *l, PSCTipo *tipo, uint64_t *v)
{
    if (lt_eh(l, "(")) {
        lt_avanca(l);
        if (expansao_expr(l, tipo, v) != 0) return -1;
        if (!lt_eh(l, ")")) { lt_erro(l, "falta o )"); return -1; }
        lt_avanca(l);
        return 0;
    }
    if (l->tipo == LT_NUM) {
        *v = (uint64_t)strtoull(l->tok, NULL, 0);
        lt_avanca(l);
        return 0;
    }
    if (l->tipo == LT_NOME) {
        char nome[128];
        if (l->ntok >= sizeof nome) { lt_erro(l, "nome comprido demais"); return -1; }
        memcpy(nome, l->tok, l->ntok);
        nome[l->ntok] = '\0';
        lt_avanca(l);
        void *p = simbolo_do_sistema(nome);
        if (!p) { lt_erro(l, "o sistema não tem o nome que a macro usa"); return -1; }
        if (!lt_eh(l, "(")) {
            *v = le_inteiro(p, tipo);
            return 0;
        }
        lt_avanca(l);
        if (!lt_eh(l, ")")) { lt_erro(l, "a chamada da macro tem argumento"); return -1; }
        lt_avanca(l);
        unsigned char ret[32];
        PSCTipo *f = ps_c_funcao(tipo, NULL, 0, 0);
        if (!f || ps_c_chama((void (*)(void))p, f, NULL, 0, NULL, ret) != 0) {
            lt_erro(l, "a chamada da macro falhou");
            return -1;
        }
        *v = tipo->sem_sinal ? ps_c_ret_uint(tipo, ret) : (uint64_t)ps_c_ret_int(tipo, ret);
        return 0;
    }
    lt_erro(l, "forma de expansão que o motor não lê");
    return -1;
}

static int expansao_expr(Leitor *l, PSCTipo *tipo, uint64_t *v)
{
    if (expansao_termo(l, tipo, v) != 0) return -1;
    while (lt_eh(l, "|")) {
        uint64_t w = 0;
        lt_avanca(l);
        if (expansao_termo(l, tipo, &w) != 0) return -1;
        *v |= w;
    }
    return 0;
}
#endif

int ps_c_le_valor(int i, void *dest, char *erro, size_t ecap)
{
    if (erro && ecap) erro[0] = '\0';
    if (fora_da_tabela(i) || PS_C_NOMES[i].especie != PSC_NOME_VALOR || (PS_C_NOMES[i].marcas & PSC_MARCA_VM)) {
        if (erro && ecap) snprintf(erro, ecap, "não é valor da biblioteca do C");
        return -1;
    }
    PSCTipo *t = ps_c_tipo_de(i);            /* o tipo do padrão: FILE *, size_t, int */
    const PSCValorPadrao *v = &PS_C_VALORES[PS_C_NOMES[i].idx];
    if (!t) return -1;
#ifndef _WIN32
    if (v->expansao) {
        PSCTipo *tipo = escalar_de(v->tipo);  /* o tipo da expressão neste sistema */
        if (!tipo || tipo->kind != PSC_INT || (t->kind != PSC_INT && t->kind != PSC_BOOL)) {
            if (erro && ecap) snprintf(erro, ecap, "a macro não é inteira neste sistema");
            return -1;
        }
        Leitor l;
        memset(&l, 0, sizeof l);
        l.ini = l.p = PS_C_KEXP + v->expansao;
        l.erro = erro;
        l.ecap = ecap;
        lt_avanca(&l);
        uint64_t x = 0;
        if (expansao_expr(&l, tipo, &x) != 0) return -1;
        if (l.tipo != LT_FIM) { lt_erro(&l, "sobrou texto na expansão"); return -1; }
        grava_inteiro(t, dest, x);
        return 0;
    }
#else
    (void)v;
    (void)PS_C_KEXP;
#endif
    void (*f)(void) = ps_c_endereco(i);
    PSCTipo *ft = ps_c_funcao(t, NULL, 0, 0);
    unsigned char ret[64];
    if (!f || !ft || ps_c_chama(f, ft, NULL, 0, NULL, ret) != 0) {
        if (erro && ecap && !erro[0]) snprintf(erro, ecap, "não deu pra ler o valor");
        return -1;
    }
    if (t->kind == PSC_INT || t->kind == PSC_BOOL)
        grava_inteiro(t, dest, t->sem_sinal ? ps_c_ret_uint(t, ret) : (uint64_t)ps_c_ret_int(t, ret));
    else
        memcpy(dest, ret, t->tam);
    return 0;
}
