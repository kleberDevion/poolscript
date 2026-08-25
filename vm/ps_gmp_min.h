/*
 * ps_gmp_min — interface MÍNIMA da GMP (bignum), declarada À MÃO.
 *
 * Igual ao ps_x11_min.h: a libgmp.so.10 já vem no sistema, mas sem header
 * (libgmp-dev). A ABI da GMP é estável e pública; declaro só o `mpz` e as
 * funções que o bignum da PoolScript usa, e linko direto contra libgmp.so.10.
 * Zero download.
 *
 * Os símbolos reais são `__gmpz_*` (o gmp.h só faz `#define mpz_add __gmpz_add`);
 * replico esses defines aqui.
 */
#ifndef PS_GMP_MIN_H
#define PS_GMP_MIN_H

#include <stddef.h>

typedef unsigned long mp_limb_t;

/* Layout da ABI (64-bit): alloc/size int, ponteiro pros limbs. */
typedef struct {
    int        _mp_alloc;
    int        _mp_size;
    mp_limb_t *_mp_d;
} __mpz_struct;

typedef __mpz_struct        mpz_t[1];
typedef __mpz_struct       *mpz_ptr;
typedef const __mpz_struct *mpz_srcptr;

extern void   __gmpz_init(mpz_ptr);
extern void   __gmpz_set_d(mpz_ptr, double);
extern void   __gmpz_clear(mpz_ptr);
extern void   __gmpz_set(mpz_ptr, mpz_srcptr);
extern void   __gmpz_set_si(mpz_ptr, long);
extern double __gmpz_get_d(mpz_srcptr);
extern int    __gmpz_set_str(mpz_ptr, const char *, int);
extern char  *__gmpz_get_str(char *, int, mpz_srcptr);
extern void   __gmpz_add(mpz_ptr, mpz_srcptr, mpz_srcptr);
extern void   __gmpz_sub(mpz_ptr, mpz_srcptr, mpz_srcptr);
extern void   __gmpz_mul(mpz_ptr, mpz_srcptr, mpz_srcptr);
extern void   __gmpz_fdiv_q(mpz_ptr, mpz_srcptr, mpz_srcptr);   /* divisão pra baixo (Python) */
extern void   __gmpz_fdiv_r(mpz_ptr, mpz_srcptr, mpz_srcptr);   /* módulo com sinal do divisor */
extern int    __gmpz_cmp(mpz_srcptr, mpz_srcptr);
extern int    __gmpz_cmp_si(mpz_srcptr, long);
extern int    __gmpz_fits_slong_p(mpz_srcptr);
extern long   __gmpz_get_si(mpz_srcptr);
extern void   __gmpz_neg(mpz_ptr, mpz_srcptr);
extern void   __gmpz_pow_ui(mpz_ptr, mpz_srcptr, unsigned long);
extern size_t __gmpz_sizeinbase(mpz_srcptr, int);
/* bitwise e deslocamento — o `|`, `^`, `&`, `<<` e `>>` da linguagem passam
 * por aqui quando um dos lados não cabe no int64 (ou quando o `<<` estouraria) */
extern void   __gmpz_ior(mpz_ptr, mpz_srcptr, mpz_srcptr);
extern void   __gmpz_xor(mpz_ptr, mpz_srcptr, mpz_srcptr);
extern void   __gmpz_and(mpz_ptr, mpz_srcptr, mpz_srcptr);
extern void   __gmpz_com(mpz_ptr, mpz_srcptr);                  /* ~x */
extern void   __gmpz_mul_2exp(mpz_ptr, mpz_srcptr, unsigned long);   /* x << n */
extern void   __gmpz_fdiv_q_2exp(mpz_ptr, mpz_srcptr, unsigned long); /* x >> n */
extern int    __gmpz_fits_ulong_p(mpz_srcptr);
extern unsigned long __gmpz_get_ui(mpz_srcptr);

#define mpz_init         __gmpz_init
#define mpz_set_d        __gmpz_set_d
#define mpz_clear        __gmpz_clear
#define mpz_set          __gmpz_set
#define mpz_set_si       __gmpz_set_si
#define mpz_get_d        __gmpz_get_d
#define mpz_set_str      __gmpz_set_str
#define mpz_get_str      __gmpz_get_str
#define mpz_add          __gmpz_add
#define mpz_sub          __gmpz_sub
#define mpz_mul          __gmpz_mul
#define mpz_fdiv_q       __gmpz_fdiv_q
#define mpz_fdiv_r       __gmpz_fdiv_r
#define mpz_cmp          __gmpz_cmp
#define mpz_cmp_si       __gmpz_cmp_si
#define mpz_fits_slong_p __gmpz_fits_slong_p
#define mpz_get_si       __gmpz_get_si
#define mpz_neg          __gmpz_neg
#define mpz_pow_ui       __gmpz_pow_ui
#define mpz_sizeinbase   __gmpz_sizeinbase
#define mpz_ior          __gmpz_ior
#define mpz_xor          __gmpz_xor
#define mpz_and          __gmpz_and
#define mpz_com          __gmpz_com
#define mpz_mul_2exp     __gmpz_mul_2exp
#define mpz_fdiv_q_2exp  __gmpz_fdiv_q_2exp
#define mpz_fits_ulong_p __gmpz_fits_ulong_p
#define mpz_get_ui       __gmpz_get_ui
/* sgn é macro no gmp.h de verdade: olha o campo _mp_size direto */
#define mpz_sgn(z)       ((z)->_mp_size < 0 ? -1 : (z)->_mp_size > 0)

#endif /* PS_GMP_MIN_H */
