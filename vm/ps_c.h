/*
 * A biblioteca padrão do C dentro da Jinga (Plano 1): os tipos do C e a
 * chamada de função do C pela libffi.
 *
 * Este módulo não depende da VM. Ele sabe:
 *   - descrever um tipo do C (escalar, ponteiro, array, struct, union,
 *     função), com o tamanho e o alinhamento do sistema em que o motor foi
 *     compilado — o `long` tem 8 bytes no Linux e no macOS e 4 no Windows,
 *     como o C de cada um;
 *   - achar os tipos do padrão pelo nome (`int`, `unsigned long`, `size_t`,
 *     `int32_t`, `time_t`...);
 *   - chamar uma função do C com valores já no formato do C, pela libffi
 *     (1996, licença MIT; é a mesma que o `ctypes` do Python usa), inclusive
 *     as de argumento variável (`printf`), em que cada chamada diz o tipo de
 *     cada argumento a mais;
 *   - fazer de uma função de quem chama um ponteiro de função do C de
 *     verdade (o comparador do `qsort`), por closure da libffi.
 *
 * Quem converte valor da Jinga pra memória do C (e de volta) é a VM; aqui
 * entra e sai só memória do C.
 */
#ifndef PS_C_H
#define PS_C_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    PSC_VOID,
    PSC_INT,       /* char, short, int, long, long long, com e sem sinal */
    PSC_FLUT,      /* float, double, long double */
    PSC_BOOL,      /* _Bool */
    PSC_PONT,      /* T * */
    PSC_ARRAY,     /* T[N] */
    PSC_STRUCT,
    PSC_UNION,
    PSC_FUNC,      /* tipo de função: retorno, parâmetros, variádica */
    PSC_OPACO      /* FILE, mbstate_t: só existe atrás de ponteiro */
} PSCKind;

typedef struct PSCTipo PSCTipo;

typedef struct {
    const char *nome;
    PSCTipo    *tipo;
    size_t      desloc;    /* deslocamento dentro do struct, em bytes */
} PSCCampo;

struct PSCTipo {
    PSCKind     kind;
    const char *nome;      /* como o C escreve: "unsigned long", "struct tm", "char *" */
    size_t      tam;
    size_t      alinha;
    int         sem_sinal; /* PSC_INT: 1 = unsigned */
    int         eh_char;   /* PSC_INT: char, signed char, unsigned char */
    int         constante; /* o tipo é `const T` */
    PSCTipo    *alvo;      /* PSC_PONT: o apontado; PSC_ARRAY: o elemento */
    size_t      n;         /* PSC_ARRAY: quantos elementos */
    PSCCampo   *campos;    /* PSC_STRUCT e PSC_UNION */
    int         ncampos;
    PSCTipo    *ret;       /* PSC_FUNC */
    PSCTipo   **params;
    int         nparams;
    int         variadica;
    void       *ffi;       /* o ffi_type montado sob demanda (struct) */
    void       *cif;       /* PSC_FUNC não variádica: o ffi_cif preparado uma vez */
};

/* Os tipos escalares do C, um objeto cada (comparar ponteiro = comparar tipo). */
extern PSCTipo ps_c_void, ps_c_char, ps_c_schar, ps_c_uchar, ps_c_short, ps_c_ushort,
               ps_c_int, ps_c_uint, ps_c_long, ps_c_ulong, ps_c_llong, ps_c_ullong,
               ps_c_float, ps_c_double, ps_c_ldouble, ps_c_bool;

/* O tipo pelo nome que o C usa: os escalares ("unsigned long int", "long
 * double"...) e os typedefs do padrão ("size_t", "int32_t", "time_t"...).
 * NULL se o nome não é tipo do C. */
PSCTipo *ps_c_tipo_por_nome(const char *nome);

/* Tipos montados. Os resultados são guardados: pedir duas vezes o mesmo
 * ponteiro devolve o mesmo objeto. Devolvem NULL só sem memória. */
PSCTipo *ps_c_ponteiro(PSCTipo *alvo);
PSCTipo *ps_c_const(PSCTipo *t);                  /* `const T` */
PSCTipo *ps_c_array(PSCTipo *elem, size_t n);
PSCTipo *ps_c_funcao(PSCTipo *ret, PSCTipo **params, int nparams, int variadica);
/* struct e union: os campos com tipo; os deslocamentos, o tamanho e o
 * alinhamento saem da regra do C (o que a libffi calcula), a não ser que
 * `deslocs` venha preenchido (os structs do padrão, medidos pelo compilador
 * com `offsetof`, porque o padrão não fixa a ordem nem os campos a mais). */
PSCTipo *ps_c_struct(const char *nome, int eh_union, const char **nomes, PSCTipo **tipos,
                     int n, const size_t *deslocs, size_t tam, size_t alinha);
PSCTipo *ps_c_opaco(const char *nome, size_t tam);

/* Chama `fn` com os argumentos já na memória do C: `args[i]` aponta pro valor
 * do i-ésimo argumento. `ret` recebe o retorno, com espaço de pelo menos
 * `ps_c_tam_ret(ftipo->ret)` bytes (a libffi devolve inteiro estreito num
 * registro inteiro). Nas variádicas, `tipos_var` diz o tipo de cada um dos
 * `nargs - nparams` argumentos a mais, já promovido (float vira double,
 * inteiro menor que int vira int: a regra das variádicas do C).
 * Devolve 0, ou -1 se a libffi recusou o tipo. */
int ps_c_chama(void (*fn)(void), PSCTipo *ftipo, void **args, int nargs,
               PSCTipo **tipos_var, void *ret);
size_t ps_c_tam_ret(PSCTipo *ret);

/* Lê o retorno guardado por `ps_c_chama` no tipo de retorno (o inteiro
 * estreito vem alargado no registro). */
int64_t  ps_c_ret_int(PSCTipo *ret, const void *buf);
uint64_t ps_c_ret_uint(PSCTipo *ret, const void *buf);

/* No callback, o tratador grava o retorno inteiro com esta: o tipo mais
 * estreito que um registro vai alargado num `ffi_arg` (a regra da libffi),
 * depois de dar a volta no tamanho dele, como a conversão do C. */
void ps_c_ret_grava_int(PSCTipo *ret, void *buf, int64_t v);

/* Callback: um ponteiro de função do C de verdade que, chamado pelo C, chama
 * `tratador(ret, args, dados)` com os argumentos na memória do C. O tipo da
 * função não pode ser variádico (a libffi não tem closure variádica).
 * Devolve o objeto da closure (pra soltar com `ps_c_closure_solta`) e grava
 * em `*codigo` o endereço que o C chama; NULL se não deu. */
typedef void (*PSCTratador)(void *ret, void **args, void *dados);
void *ps_c_closure_nova(PSCTipo *ftipo, PSCTratador tratador, void *dados, void (**codigo)(void));
void  ps_c_closure_solta(void *closure);

#endif /* PS_C_H */
