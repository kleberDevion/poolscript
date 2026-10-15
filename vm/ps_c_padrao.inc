/* GERADO por scripts/gera_c_padrao.pr a partir de vm/c_padrao.decl: não se
 * edita à mão. Mude o .decl e rode `./pool scripts/gera_c_padrao.pr --grava`.
 * Incluído só por vm/ps_c.c, que define os tipos das tabelas e o PS_C_TIPO. */

#define PS_C_NCAB 13
#define PS_C_NNOMES 735

static const char PS_C_TEXTO[] =
    "\0"
    "ctype.h\0"
    "errno.h\0"
    "float.h\0"
    "inttypes.h\0"
    "limits.h\0"
    "math.h\0"
    "stdbool.h\0"
    "stddef.h\0"
    "stdint.h\0"
    "stdio.h\0"
    "stdlib.h\0"
    "string.h\0"
    "time.h\0"
    "BUFSIZ\0"
    "CHAR_BIT\0"
    "CHAR_MAX\0"
    "CHAR_MIN\0"
    "CLOCKS_PER_SEC\0"
    "DBL_DECIMAL_DIG\0"
    "DBL_DIG\0"
    "DBL_EPSILON\0"
    "DBL_HAS_SUBNORM\0"
    "DBL_MANT_DIG\0"
    "DBL_MAX\0"
    "DBL_MAX_10_EXP\0"
    "DBL_MAX_EXP\0"
    "DBL_MIN\0"
    "DBL_MIN_10_EXP\0"
    "DBL_MIN_EXP\0"
    "DBL_TRUE_MIN\0"
    "DECIMAL_DIG\0"
    "EDOM\0"
    "EILSEQ\0"
    "EOF\0"
    "ERANGE\0"
    "EXIT_FAILURE\0"
    "EXIT_SUCCESS\0"
    "FILE\0"
    "FILENAME_MAX\0"
    "FLT_DECIMAL_DIG\0"
    "FLT_DIG\0"
    "FLT_EPSILON\0"
    "FLT_EVAL_METHOD\0"
    "FLT_HAS_SUBNORM\0"
    "FLT_MANT_DIG\0"
    "FLT_MAX\0"
    "FLT_MAX_10_EXP\0"
    "FLT_MAX_EXP\0"
    "FLT_MIN\0"
    "FLT_MIN_10_EXP\0"
    "FLT_MIN_EXP\0"
    "FLT_RADIX\0"
    "FLT_ROUNDS\0"
    "int FLT_ROUNDS\0"
    "FLT_TRUE_MIN\0"
    "FOPEN_MAX\0"
    "FP_FAST_FMA\0"
    "FP_FAST_FMAF\0"
    "FP_FAST_FMAL\0"
    "FP_ILOGB0\0"
    "FP_ILOGBNAN\0"
    "FP_INFINITE\0"
    "FP_NAN\0"
    "FP_NORMAL\0"
    "FP_SUBNORMAL\0"
    "FP_ZERO\0"
    "HUGE_VAL\0"
    "HUGE_VALF\0"
    "HUGE_VALL\0"
    "INFINITY\0"
    "INT16_C\0"
    "int INT16_C(int value)\0"
    "INT16_MAX\0"
    "INT16_MIN\0"
    "INT32_C\0"
    "int_least32_t INT32_C(int_least32_t value)\0"
    "INT32_MAX\0"
    "INT32_MIN\0"
    "INT64_C\0"
    "int_least64_t INT64_C(int_least64_t value)\0"
    "INT64_MAX\0"
    "INT64_MIN\0"
    "INT8_C\0"
    "int INT8_C(int value)\0"
    "INT8_MAX\0"
    "INT8_MIN\0"
    "INTMAX_C\0"
    "intmax_t INTMAX_C(intmax_t value)\0"
    "INTMAX_MAX\0"
    "INTMAX_MIN\0"
    "INTPTR_MAX\0"
    "INTPTR_MIN\0"
    "INT_FAST16_MAX\0"
    "INT_FAST16_MIN\0"
    "INT_FAST32_MAX\0"
    "INT_FAST32_MIN\0"
    "INT_FAST64_MAX\0"
    "INT_FAST64_MIN\0"
    "INT_FAST8_MAX\0"
    "INT_FAST8_MIN\0"
    "INT_LEAST16_MAX\0"
    "INT_LEAST16_MIN\0"
    "INT_LEAST32_MAX\0"
    "INT_LEAST32_MIN\0"
    "INT_LEAST64_MAX\0"
    "INT_LEAST64_MIN\0"
    "INT_LEAST8_MAX\0"
    "INT_LEAST8_MIN\0"
    "INT_MAX\0"
    "INT_MIN\0"
    "LDBL_DECIMAL_DIG\0"
    "LDBL_DIG\0"
    "LDBL_EPSILON\0"
    "LDBL_HAS_SUBNORM\0"
    "LDBL_MANT_DIG\0"
    "LDBL_MAX\0"
    "LDBL_MAX_10_EXP\0"
    "LDBL_MAX_EXP\0"
    "LDBL_MIN\0"
    "LDBL_MIN_10_EXP\0"
    "LDBL_MIN_EXP\0"
    "LDBL_TRUE_MIN\0"
    "LLONG_MAX\0"
    "LLONG_MIN\0"
    "LONG_MAX\0"
    "LONG_MIN\0"
    "L_tmpnam\0"
    "L_tmpnam_s\0"
    "é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS\0"
    "MATH_ERREXCEPT\0"
    "MATH_ERRNO\0"
    "MB_CUR_MAX\0"
    "size_t MB_CUR_MAX\0"
    "MB_LEN_MAX\0"
    "NAN\0"
    "NULL\0"
    "PRIX16\0"
    "PRIX32\0"
    "PRIX64\0"
    "PRIX8\0"
    "PRIXFAST16\0"
    "PRIXFAST32\0"
    "PRIXFAST64\0"
    "PRIXFAST8\0"
    "PRIXLEAST16\0"
    "PRIXLEAST32\0"
    "PRIXLEAST64\0"
    "PRIXLEAST8\0"
    "PRIXMAX\0"
    "PRIXPTR\0"
    "PRId16\0"
    "PRId32\0"
    "PRId64\0"
    "PRId8\0"
    "PRIdFAST16\0"
    "PRIdFAST32\0"
    "PRIdFAST64\0"
    "PRIdFAST8\0"
    "PRIdLEAST16\0"
    "PRIdLEAST32\0"
    "PRIdLEAST64\0"
    "PRIdLEAST8\0"
    "PRIdMAX\0"
    "PRIdPTR\0"
    "PRIi16\0"
    "PRIi32\0"
    "PRIi64\0"
    "PRIi8\0"
    "PRIiFAST16\0"
    "PRIiFAST32\0"
    "PRIiFAST64\0"
    "PRIiFAST8\0"
    "PRIiLEAST16\0"
    "PRIiLEAST32\0"
    "PRIiLEAST64\0"
    "PRIiLEAST8\0"
    "PRIiMAX\0"
    "PRIiPTR\0"
    "PRIo16\0"
    "PRIo32\0"
    "PRIo64\0"
    "PRIo8\0"
    "PRIoFAST16\0"
    "PRIoFAST32\0"
    "PRIoFAST64\0"
    "PRIoFAST8\0"
    "PRIoLEAST16\0"
    "PRIoLEAST32\0"
    "PRIoLEAST64\0"
    "PRIoLEAST8\0"
    "PRIoMAX\0"
    "PRIoPTR\0"
    "PRIu16\0"
    "PRIu32\0"
    "PRIu64\0"
    "PRIu8\0"
    "PRIuFAST16\0"
    "PRIuFAST32\0"
    "PRIuFAST64\0"
    "PRIuFAST8\0"
    "PRIuLEAST16\0"
    "PRIuLEAST32\0"
    "PRIuLEAST64\0"
    "PRIuLEAST8\0"
    "PRIuMAX\0"
    "PRIuPTR\0"
    "PRIx16\0"
    "PRIx32\0"
    "PRIx64\0"
    "PRIx8\0"
    "PRIxFAST16\0"
    "PRIxFAST32\0"
    "PRIxFAST64\0"
    "PRIxFAST8\0"
    "PRIxLEAST16\0"
    "PRIxLEAST32\0"
    "PRIxLEAST64\0"
    "PRIxLEAST8\0"
    "PRIxMAX\0"
    "PRIxPTR\0"
    "PTRDIFF_MAX\0"
    "PTRDIFF_MIN\0"
    "RAND_MAX\0"
    "RSIZE_MAX\0"
    "SCHAR_MAX\0"
    "SCHAR_MIN\0"
    "SCNd16\0"
    "SCNd32\0"
    "SCNd64\0"
    "SCNd8\0"
    "SCNdFAST16\0"
    "SCNdFAST32\0"
    "SCNdFAST64\0"
    "SCNdFAST8\0"
    "SCNdLEAST16\0"
    "SCNdLEAST32\0"
    "SCNdLEAST64\0"
    "SCNdLEAST8\0"
    "SCNdMAX\0"
    "SCNdPTR\0"
    "SCNi16\0"
    "SCNi32\0"
    "SCNi64\0"
    "SCNi8\0"
    "SCNiFAST16\0"
    "SCNiFAST32\0"
    "SCNiFAST64\0"
    "SCNiFAST8\0"
    "SCNiLEAST16\0"
    "SCNiLEAST32\0"
    "SCNiLEAST64\0"
    "SCNiLEAST8\0"
    "SCNiMAX\0"
    "SCNiPTR\0"
    "SCNo16\0"
    "SCNo32\0"
    "SCNo64\0"
    "SCNo8\0"
    "SCNoFAST16\0"
    "SCNoFAST32\0"
    "SCNoFAST64\0"
    "SCNoFAST8\0"
    "SCNoLEAST16\0"
    "SCNoLEAST32\0"
    "SCNoLEAST64\0"
    "SCNoLEAST8\0"
    "SCNoMAX\0"
    "SCNoPTR\0"
    "SCNu16\0"
    "SCNu32\0"
    "SCNu64\0"
    "SCNu8\0"
    "SCNuFAST16\0"
    "SCNuFAST32\0"
    "SCNuFAST64\0"
    "SCNuFAST8\0"
    "SCNuLEAST16\0"
    "SCNuLEAST32\0"
    "SCNuLEAST64\0"
    "SCNuLEAST8\0"
    "SCNuMAX\0"
    "SCNuPTR\0"
    "SCNx16\0"
    "SCNx32\0"
    "SCNx64\0"
    "SCNx8\0"
    "SCNxFAST16\0"
    "SCNxFAST32\0"
    "SCNxFAST64\0"
    "SCNxFAST8\0"
    "SCNxLEAST16\0"
    "SCNxLEAST32\0"
    "SCNxLEAST64\0"
    "SCNxLEAST8\0"
    "SCNxMAX\0"
    "SCNxPTR\0"
    "SEEK_CUR\0"
    "SEEK_END\0"
    "SEEK_SET\0"
    "SHRT_MAX\0"
    "SHRT_MIN\0"
    "SIG_ATOMIC_MAX\0"
    "SIG_ATOMIC_MIN\0"
    "SIZE_MAX\0"
    "TIME_UTC\0"
    "TMP_MAX\0"
    "TMP_MAX_S\0"
    "UCHAR_MAX\0"
    "UINT16_C\0"
    "int UINT16_C(int value)\0"
    "UINT16_MAX\0"
    "UINT32_C\0"
    "uint_least32_t UINT32_C(uint_least32_t value)\0"
    "UINT32_MAX\0"
    "UINT64_C\0"
    "uint_least64_t UINT64_C(uint_least64_t value)\0"
    "UINT64_MAX\0"
    "UINT8_C\0"
    "int UINT8_C(int value)\0"
    "UINT8_MAX\0"
    "UINTMAX_C\0"
    "uintmax_t UINTMAX_C(uintmax_t value)\0"
    "UINTMAX_MAX\0"
    "UINTPTR_MAX\0"
    "UINT_FAST16_MAX\0"
    "UINT_FAST32_MAX\0"
    "UINT_FAST64_MAX\0"
    "UINT_FAST8_MAX\0"
    "UINT_LEAST16_MAX\0"
    "UINT_LEAST32_MAX\0"
    "UINT_LEAST64_MAX\0"
    "UINT_LEAST8_MAX\0"
    "UINT_MAX\0"
    "ULLONG_MAX\0"
    "ULONG_MAX\0"
    "USHRT_MAX\0"
    "WCHAR_MAX\0"
    "WCHAR_MIN\0"
    "WINT_MAX\0"
    "WINT_MIN\0"
    "_Exit\0"
    "_Noreturn void _Exit(int status)\0"
    "_IOFBF\0"
    "_IOLBF\0"
    "_IONBF\0"
    "__STDC_WANT_LIB_EXT1__\0"
    "__bool_true_false_are_defined\0"
    "abort\0"
    "_Noreturn void abort(void)\0"
    "abort_handler_s\0"
    "abs\0"
    "int abs(int j)\0"
    "acos\0"
    "double acos(double x)\0"
    "acosf\0"
    "float acosf(float x)\0"
    "acosh\0"
    "double acosh(double x)\0"
    "acoshf\0"
    "float acoshf(float x)\0"
    "acoshl\0"
    "long double acoshl(long double x)\0"
    "acosl\0"
    "long double acosl(long double x)\0"
    "aligned_alloc\0"
    "void *aligned_alloc(size_t alignment, size_t size)\0"
    "asctime\0"
    "char *asctime(const struct tm *timeptr)\0"
    "asctime_s\0"
    "asin\0"
    "double asin(double x)\0"
    "asinf\0"
    "float asinf(float x)\0"
    "asinh\0"
    "double asinh(double x)\0"
    "asinhf\0"
    "float asinhf(float x)\0"
    "asinhl\0"
    "long double asinhl(long double x)\0"
    "asinl\0"
    "long double asinl(long double x)\0"
    "at_quick_exit\0"
    "int at_quick_exit(void (*func)(void))\0"
    "atan\0"
    "double atan(double x)\0"
    "atan2\0"
    "double atan2(double y, double x)\0"
    "atan2f\0"
    "float atan2f(float y, float x)\0"
    "atan2l\0"
    "long double atan2l(long double y, long double x)\0"
    "atanf\0"
    "float atanf(float x)\0"
    "atanh\0"
    "double atanh(double x)\0"
    "atanhf\0"
    "float atanhf(float x)\0"
    "atanhl\0"
    "long double atanhl(long double x)\0"
    "atanl\0"
    "long double atanl(long double x)\0"
    "atexit\0"
    "int atexit(void (*func)(void))\0"
    "atof\0"
    "double atof(const char *nptr)\0"
    "atoi\0"
    "int atoi(const char *nptr)\0"
    "atol\0"
    "long int atol(const char *nptr)\0"
    "atoll\0"
    "long long int atoll(const char *nptr)\0"
    "bool\0"
    "bsearch\0"
    "void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))\0"
    "bsearch_s\0"
    "calloc\0"
    "void *calloc(size_t nmemb, size_t size)\0"
    "cbrt\0"
    "double cbrt(double x)\0"
    "cbrtf\0"
    "float cbrtf(float x)\0"
    "cbrtl\0"
    "long double cbrtl(long double x)\0"
    "ceil\0"
    "double ceil(double x)\0"
    "ceilf\0"
    "float ceilf(float x)\0"
    "ceill\0"
    "long double ceill(long double x)\0"
    "clearerr\0"
    "void clearerr(FILE *stream)\0"
    "clock\0"
    "clock_t clock(void)\0"
    "clock_t\0"
    "constraint_handler_t\0"
    "copysign\0"
    "double copysign(double x, double y)\0"
    "copysignf\0"
    "float copysignf(float x, float y)\0"
    "copysignl\0"
    "long double copysignl(long double x, long double y)\0"
    "cos\0"
    "double cos(double x)\0"
    "cosf\0"
    "float cosf(float x)\0"
    "cosh\0"
    "double cosh(double x)\0"
    "coshf\0"
    "float coshf(float x)\0"
    "coshl\0"
    "long double coshl(long double x)\0"
    "cosl\0"
    "long double cosl(long double x)\0"
    "ctime\0"
    "char *ctime(const time_t *timer)\0"
    "ctime_s\0"
    "difftime\0"
    "double difftime(time_t time1, time_t time0)\0"
    "div\0"
    "div_t div(int numer, int denom)\0"
    "div_t\0"
    "quot\0"
    "int quot\0"
    "rem\0"
    "int rem\0"
    "double_t\0"
    "erf\0"
    "double erf(double x)\0"
    "erfc\0"
    "double erfc(double x)\0"
    "erfcf\0"
    "float erfcf(float x)\0"
    "erfcl\0"
    "long double erfcl(long double x)\0"
    "erff\0"
    "float erff(float x)\0"
    "erfl\0"
    "long double erfl(long double x)\0"
    "errno\0"
    "int errno\0"
    "errno_t\0"
    "exit\0"
    "_Noreturn void exit(int status)\0"
    "exp\0"
    "double exp(double x)\0"
    "exp2\0"
    "double exp2(double x)\0"
    "exp2f\0"
    "float exp2f(float x)\0"
    "exp2l\0"
    "long double exp2l(long double x)\0"
    "expf\0"
    "float expf(float x)\0"
    "expl\0"
    "long double expl(long double x)\0"
    "expm1\0"
    "double expm1(double x)\0"
    "expm1f\0"
    "float expm1f(float x)\0"
    "expm1l\0"
    "long double expm1l(long double x)\0"
    "fabs\0"
    "double fabs(double x)\0"
    "fabsf\0"
    "float fabsf(float x)\0"
    "fabsl\0"
    "long double fabsl(long double x)\0"
    "fclose\0"
    "int fclose(FILE *stream)\0"
    "fdim\0"
    "double fdim(double x, double y)\0"
    "fdimf\0"
    "float fdimf(float x, float y)\0"
    "fdiml\0"
    "long double fdiml(long double x, long double y)\0"
    "feof\0"
    "int feof(FILE *stream)\0"
    "ferror\0"
    "int ferror(FILE *stream)\0"
    "fflush\0"
    "int fflush(FILE *stream)\0"
    "fgetc\0"
    "int fgetc(FILE *stream)\0"
    "fgetpos\0"
    "int fgetpos(FILE * restrict stream, fpos_t * restrict pos)\0"
    "fgets\0"
    "char *fgets(char * restrict s, int n, FILE * restrict stream)\0"
    "float_t\0"
    "floor\0"
    "double floor(double x)\0"
    "floorf\0"
    "float floorf(float x)\0"
    "floorl\0"
    "long double floorl(long double x)\0"
    "fma\0"
    "double fma(double x, double y, double z)\0"
    "fmaf\0"
    "float fmaf(float x, float y, float z)\0"
    "fmal\0"
    "long double fmal(long double x, long double y, long double z)\0"
    "fmax\0"
    "double fmax(double x, double y)\0"
    "fmaxf\0"
    "float fmaxf(float x, float y)\0"
    "fmaxl\0"
    "long double fmaxl(long double x, long double y)\0"
    "fmin\0"
    "double fmin(double x, double y)\0"
    "fminf\0"
    "float fminf(float x, float y)\0"
    "fminl\0"
    "long double fminl(long double x, long double y)\0"
    "fmod\0"
    "double fmod(double x, double y)\0"
    "fmodf\0"
    "float fmodf(float x, float y)\0"
    "fmodl\0"
    "long double fmodl(long double x, long double y)\0"
    "fopen\0"
    "FILE *fopen(const char * restrict filename, const char * restrict mode)\0"
    "fopen_s\0"
    "fpclassify\0"
    "int fpclassify(double x)\0"
    "fpos_t\0"
    "fprintf\0"
    "int fprintf(FILE * restrict stream, const char * restrict format, ...)\0"
    "fprintf_s\0"
    "fputc\0"
    "int fputc(int c, FILE *stream)\0"
    "fputs\0"
    "int fputs(const char * restrict s, FILE * restrict stream)\0"
    "fread\0"
    "size_t fread(void * restrict ptr, size_t size, size_t nmemb, FILE * restrict stream)\0"
    "free\0"
    "void free(void *ptr)\0"
    "freopen\0"
    "FILE *freopen(const char * restrict filename, const char * restrict mode, FILE * restrict stream)\0"
    "freopen_s\0"
    "frexp\0"
    "double frexp(double value, int *exp)\0"
    "frexpf\0"
    "float frexpf(float value, int *exp)\0"
    "frexpl\0"
    "long double frexpl(long double value, int *exp)\0"
    "fscanf\0"
    "int fscanf(FILE * restrict stream, const char * restrict format, ...)\0"
    "__isoc99_fscanf\0"
    "fscanf_s\0"
    "fseek\0"
    "int fseek(FILE *stream, long int offset, int whence)\0"
    "fsetpos\0"
    "int fsetpos(FILE *stream, const fpos_t *pos)\0"
    "ftell\0"
    "long int ftell(FILE *stream)\0"
    "fwrite\0"
    "size_t fwrite(const void * restrict ptr, size_t size, size_t nmemb, FILE * restrict stream)\0"
    "getc\0"
    "int getc(FILE *stream)\0"
    "getchar\0"
    "int getchar(void)\0"
    "getenv\0"
    "char *getenv(const char *name)\0"
    "getenv_s\0"
    "gets\0"
    "saiu do C11, porque lia sem limite e passava do fim do buffer; use fgets\0"
    "gets_s\0"
    "gmtime\0"
    "struct tm *gmtime(const time_t *timer)\0"
    "gmtime_s\0"
    "hypot\0"
    "double hypot(double x, double y)\0"
    "hypotf\0"
    "float hypotf(float x, float y)\0"
    "hypotl\0"
    "long double hypotl(long double x, long double y)\0"
    "ignore_handler_s\0"
    "ilogb\0"
    "int ilogb(double x)\0"
    "ilogbf\0"
    "int ilogbf(float x)\0"
    "ilogbl\0"
    "int ilogbl(long double x)\0"
    "imaxabs\0"
    "intmax_t imaxabs(intmax_t j)\0"
    "imaxdiv\0"
    "imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom)\0"
    "imaxdiv_t\0"
    "intmax_t quot\0"
    "intmax_t rem\0"
    "int16_t\0"
    "int32_t\0"
    "int64_t\0"
    "int8_t\0"
    "int_fast16_t\0"
    "int_fast32_t\0"
    "int_fast64_t\0"
    "int_fast8_t\0"
    "int_least16_t\0"
    "int_least32_t\0"
    "int_least64_t\0"
    "int_least8_t\0"
    "intmax_t\0"
    "intptr_t\0"
    "isalnum\0"
    "int isalnum(int c)\0"
    "isalpha\0"
    "int isalpha(int c)\0"
    "isblank\0"
    "int isblank(int c)\0"
    "iscntrl\0"
    "int iscntrl(int c)\0"
    "isdigit\0"
    "int isdigit(int c)\0"
    "isfinite\0"
    "int isfinite(double x)\0"
    "isgraph\0"
    "int isgraph(int c)\0"
    "isgreater\0"
    "int isgreater(double x, double y)\0"
    "isgreaterequal\0"
    "int isgreaterequal(double x, double y)\0"
    "isinf\0"
    "int isinf(double x)\0"
    "isless\0"
    "int isless(double x, double y)\0"
    "islessequal\0"
    "int islessequal(double x, double y)\0"
    "islessgreater\0"
    "int islessgreater(double x, double y)\0"
    "islower\0"
    "int islower(int c)\0"
    "isnan\0"
    "int isnan(double x)\0"
    "isnormal\0"
    "int isnormal(double x)\0"
    "isprint\0"
    "int isprint(int c)\0"
    "ispunct\0"
    "int ispunct(int c)\0"
    "isspace\0"
    "int isspace(int c)\0"
    "isunordered\0"
    "int isunordered(double x, double y)\0"
    "isupper\0"
    "int isupper(int c)\0"
    "isxdigit\0"
    "int isxdigit(int c)\0"
    "labs\0"
    "long int labs(long int j)\0"
    "ldexp\0"
    "double ldexp(double x, int exp)\0"
    "ldexpf\0"
    "float ldexpf(float x, int exp)\0"
    "ldexpl\0"
    "long double ldexpl(long double x, int exp)\0"
    "ldiv\0"
    "ldiv_t ldiv(long int numer, long int denom)\0"
    "ldiv_t\0"
    "long int quot\0"
    "long int rem\0"
    "lgamma\0"
    "double lgamma(double x)\0"
    "lgammaf\0"
    "float lgammaf(float x)\0"
    "lgammal\0"
    "long double lgammal(long double x)\0"
    "llabs\0"
    "long long int llabs(long long int j)\0"
    "lldiv\0"
    "lldiv_t lldiv(long long int numer, long long int denom)\0"
    "lldiv_t\0"
    "long long int quot\0"
    "long long int rem\0"
    "llrint\0"
    "long long int llrint(double x)\0"
    "llrintf\0"
    "long long int llrintf(float x)\0"
    "llrintl\0"
    "long long int llrintl(long double x)\0"
    "llround\0"
    "long long int llround(double x)\0"
    "llroundf\0"
    "long long int llroundf(float x)\0"
    "llroundl\0"
    "long long int llroundl(long double x)\0"
    "localtime\0"
    "struct tm *localtime(const time_t *timer)\0"
    "localtime_s\0"
    "log\0"
    "double log(double x)\0"
    "log10\0"
    "double log10(double x)\0"
    "log10f\0"
    "float log10f(float x)\0"
    "log10l\0"
    "long double log10l(long double x)\0"
    "log1p\0"
    "double log1p(double x)\0"
    "log1pf\0"
    "float log1pf(float x)\0"
    "log1pl\0"
    "long double log1pl(long double x)\0"
    "log2\0"
    "double log2(double x)\0"
    "log2f\0"
    "float log2f(float x)\0"
    "log2l\0"
    "long double log2l(long double x)\0"
    "logb\0"
    "double logb(double x)\0"
    "logbf\0"
    "float logbf(float x)\0"
    "logbl\0"
    "long double logbl(long double x)\0"
    "logf\0"
    "float logf(float x)\0"
    "logl\0"
    "long double logl(long double x)\0"
    "lrint\0"
    "long int lrint(double x)\0"
    "lrintf\0"
    "long int lrintf(float x)\0"
    "lrintl\0"
    "long int lrintl(long double x)\0"
    "lround\0"
    "long int lround(double x)\0"
    "lroundf\0"
    "long int lroundf(float x)\0"
    "lroundl\0"
    "long int lroundl(long double x)\0"
    "malloc\0"
    "void *malloc(size_t size)\0"
    "math_errhandling\0"
    "int math_errhandling\0"
    "max_align_t\0"
    "mblen\0"
    "int mblen(const char *s, size_t n)\0"
    "mbstowcs\0"
    "size_t mbstowcs(wchar_t * restrict pwcs, const char * restrict s, size_t n)\0"
    "mbstowcs_s\0"
    "mbtowc\0"
    "int mbtowc(wchar_t * restrict pwc, const char * restrict s, size_t n)\0"
    "memchr\0"
    "void *memchr(const void *s, int c, size_t n)\0"
    "memcmp\0"
    "int memcmp(const void *s1, const void *s2, size_t n)\0"
    "memcpy\0"
    "void *memcpy(void * restrict s1, const void * restrict s2, size_t n)\0"
    "memcpy_s\0"
    "memmove\0"
    "void *memmove(void *s1, const void *s2, size_t n)\0"
    "memmove_s\0"
    "memset\0"
    "void *memset(void *s, int c, size_t n)\0"
    "memset_s\0"
    "mktime\0"
    "time_t mktime(struct tm *timeptr)\0"
    "modf\0"
    "double modf(double value, double *iptr)\0"
    "modff\0"
    "float modff(float value, float *iptr)\0"
    "modfl\0"
    "long double modfl(long double value, long double *iptr)\0"
    "nan\0"
    "double nan(const char *tagp)\0"
    "nanf\0"
    "float nanf(const char *tagp)\0"
    "nanl\0"
    "long double nanl(const char *tagp)\0"
    "nearbyint\0"
    "double nearbyint(double x)\0"
    "nearbyintf\0"
    "float nearbyintf(float x)\0"
    "nearbyintl\0"
    "long double nearbyintl(long double x)\0"
    "nextafter\0"
    "double nextafter(double x, double y)\0"
    "nextafterf\0"
    "float nextafterf(float x, float y)\0"
    "nextafterl\0"
    "long double nextafterl(long double x, long double y)\0"
    "nexttoward\0"
    "double nexttoward(double x, long double y)\0"
    "nexttowardf\0"
    "float nexttowardf(float x, long double y)\0"
    "nexttowardl\0"
    "long double nexttowardl(long double x, long double y)\0"
    "offsetof\0"
    "offsetof(type, member-designator)\0"
    "perror\0"
    "void perror(const char *s)\0"
    "pow\0"
    "double pow(double x, double y)\0"
    "powf\0"
    "float powf(float x, float y)\0"
    "powl\0"
    "long double powl(long double x, long double y)\0"
    "printf\0"
    "int printf(const char * restrict format, ...)\0"
    "printf_s\0"
    "ptrdiff_t\0"
    "putc\0"
    "int putc(int c, FILE *stream)\0"
    "putchar\0"
    "int putchar(int c)\0"
    "puts\0"
    "int puts(const char *s)\0"
    "qsort\0"
    "void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))\0"
    "qsort_s\0"
    "quick_exit\0"
    "_Noreturn void quick_exit(int status)\0"
    "rand\0"
    "int rand(void)\0"
    "realloc\0"
    "void *realloc(void *ptr, size_t size)\0"
    "remainder\0"
    "double remainder(double x, double y)\0"
    "remainderf\0"
    "float remainderf(float x, float y)\0"
    "remainderl\0"
    "long double remainderl(long double x, long double y)\0"
    "remove\0"
    "int remove(const char *filename)\0"
    "remquo\0"
    "double remquo(double x, double y, int *quo)\0"
    "remquof\0"
    "float remquof(float x, float y, int *quo)\0"
    "remquol\0"
    "long double remquol(long double x, long double y, int *quo)\0"
    "rename\0"
    "int rename(const char *old, const char *new)\0"
    "rewind\0"
    "void rewind(FILE *stream)\0"
    "rint\0"
    "double rint(double x)\0"
    "rintf\0"
    "float rintf(float x)\0"
    "rintl\0"
    "long double rintl(long double x)\0"
    "round\0"
    "double round(double x)\0"
    "roundf\0"
    "float roundf(float x)\0"
    "roundl\0"
    "long double roundl(long double x)\0"
    "rsize_t\0"
    "scalbln\0"
    "double scalbln(double x, long int n)\0"
    "scalblnf\0"
    "float scalblnf(float x, long int n)\0"
    "scalblnl\0"
    "long double scalblnl(long double x, long int n)\0"
    "scalbn\0"
    "double scalbn(double x, int n)\0"
    "scalbnf\0"
    "float scalbnf(float x, int n)\0"
    "scalbnl\0"
    "long double scalbnl(long double x, int n)\0"
    "scanf\0"
    "int scanf(const char * restrict format, ...)\0"
    "__isoc99_scanf\0"
    "scanf_s\0"
    "set_constraint_handler_s\0"
    "setbuf\0"
    "void setbuf(FILE * restrict stream, char * restrict buf)\0"
    "setvbuf\0"
    "int setvbuf(FILE * restrict stream, char * restrict buf, int mode, size_t size)\0"
    "signbit\0"
    "int signbit(double x)\0"
    "sin\0"
    "double sin(double x)\0"
    "sinf\0"
    "float sinf(float x)\0"
    "sinh\0"
    "double sinh(double x)\0"
    "sinhf\0"
    "float sinhf(float x)\0"
    "sinhl\0"
    "long double sinhl(long double x)\0"
    "sinl\0"
    "long double sinl(long double x)\0"
    "size_t\0"
    "snprintf\0"
    "int snprintf(char * restrict s, size_t n, const char * restrict format, ...)\0"
    "snprintf_s\0"
    "sprintf\0"
    "int sprintf(char * restrict s, const char * restrict format, ...)\0"
    "sprintf_s\0"
    "sqrt\0"
    "double sqrt(double x)\0"
    "sqrtf\0"
    "float sqrtf(float x)\0"
    "sqrtl\0"
    "long double sqrtl(long double x)\0"
    "srand\0"
    "void srand(unsigned int seed)\0"
    "sscanf\0"
    "int sscanf(const char * restrict s, const char * restrict format, ...)\0"
    "__isoc99_sscanf\0"
    "sscanf_s\0"
    "stderr\0"
    "FILE *stderr\0"
    "stdin\0"
    "FILE *stdin\0"
    "stdout\0"
    "FILE *stdout\0"
    "strcat\0"
    "char *strcat(char * restrict s1, const char * restrict s2)\0"
    "strcat_s\0"
    "strchr\0"
    "char *strchr(const char *s, int c)\0"
    "strcmp\0"
    "int strcmp(const char *s1, const char *s2)\0"
    "strcoll\0"
    "int strcoll(const char *s1, const char *s2)\0"
    "strcpy\0"
    "char *strcpy(char * restrict s1, const char * restrict s2)\0"
    "strcpy_s\0"
    "strcspn\0"
    "size_t strcspn(const char *s1, const char *s2)\0"
    "strerror\0"
    "char *strerror(int errnum)\0"
    "strerror_s\0"
    "strerrorlen_s\0"
    "strftime\0"
    "size_t strftime(char * restrict s, size_t maxsize, const char * restrict format, const struct tm * restrict timeptr)\0"
    "strlen\0"
    "size_t strlen(const char *s)\0"
    "strncat\0"
    "char *strncat(char * restrict s1, const char * restrict s2, size_t n)\0"
    "strncat_s\0"
    "strncmp\0"
    "int strncmp(const char *s1, const char *s2, size_t n)\0"
    "strncpy\0"
    "char *strncpy(char * restrict s1, const char * restrict s2, size_t n)\0"
    "strncpy_s\0"
    "strnlen_s\0"
    "strpbrk\0"
    "char *strpbrk(const char *s1, const char *s2)\0"
    "strrchr\0"
    "char *strrchr(const char *s, int c)\0"
    "strspn\0"
    "size_t strspn(const char *s1, const char *s2)\0"
    "strstr\0"
    "char *strstr(const char *s1, const char *s2)\0"
    "strtod\0"
    "double strtod(const char * restrict nptr, char ** restrict endptr)\0"
    "strtof\0"
    "float strtof(const char * restrict nptr, char ** restrict endptr)\0"
    "strtoimax\0"
    "intmax_t strtoimax(const char * restrict nptr, char ** restrict endptr, int base)\0"
    "strtok\0"
    "char *strtok(char * restrict s1, const char * restrict s2)\0"
    "strtok_s\0"
    "strtol\0"
    "long int strtol(const char * restrict nptr, char ** restrict endptr, int base)\0"
    "strtold\0"
    "long double strtold(const char * restrict nptr, char ** restrict endptr)\0"
    "strtoll\0"
    "long long int strtoll(const char * restrict nptr, char ** restrict endptr, int base)\0"
    "strtoul\0"
    "unsigned long int strtoul(const char * restrict nptr, char ** restrict endptr, int base)\0"
    "strtoull\0"
    "unsigned long long int strtoull(const char * restrict nptr, char ** restrict endptr, int base)\0"
    "strtoumax\0"
    "uintmax_t strtoumax(const char * restrict nptr, char ** restrict endptr, int base)\0"
    "struct timespec\0"
    "tv_sec\0"
    "time_t tv_sec\0"
    "tv_nsec\0"
    "long tv_nsec\0"
    "struct tm\0"
    "tm_sec\0"
    "int tm_sec\0"
    "tm_min\0"
    "int tm_min\0"
    "tm_hour\0"
    "int tm_hour\0"
    "tm_mday\0"
    "int tm_mday\0"
    "tm_mon\0"
    "int tm_mon\0"
    "tm_year\0"
    "int tm_year\0"
    "tm_wday\0"
    "int tm_wday\0"
    "tm_yday\0"
    "int tm_yday\0"
    "tm_isdst\0"
    "int tm_isdst\0"
    "strxfrm\0"
    "size_t strxfrm(char * restrict s1, const char * restrict s2, size_t n)\0"
    "system\0"
    "int system(const char *string)\0"
    "tan\0"
    "double tan(double x)\0"
    "tanf\0"
    "float tanf(float x)\0"
    "tanh\0"
    "double tanh(double x)\0"
    "tanhf\0"
    "float tanhf(float x)\0"
    "tanhl\0"
    "long double tanhl(long double x)\0"
    "tanl\0"
    "long double tanl(long double x)\0"
    "tgamma\0"
    "double tgamma(double x)\0"
    "tgammaf\0"
    "float tgammaf(float x)\0"
    "tgammal\0"
    "long double tgammal(long double x)\0"
    "time\0"
    "time_t time(time_t *timer)\0"
    "time_t\0"
    "timespec_get\0"
    "int timespec_get(struct timespec *ts, int base)\0"
    "tmpfile\0"
    "FILE *tmpfile(void)\0"
    "tmpfile_s\0"
    "tmpnam\0"
    "char *tmpnam(char *s)\0"
    "tmpnam_s\0"
    "tolower\0"
    "int tolower(int c)\0"
    "toupper\0"
    "int toupper(int c)\0"
    "trunc\0"
    "double trunc(double x)\0"
    "truncf\0"
    "float truncf(float x)\0"
    "truncl\0"
    "long double truncl(long double x)\0"
    "uint16_t\0"
    "uint32_t\0"
    "uint64_t\0"
    "uint8_t\0"
    "uint_fast16_t\0"
    "uint_fast32_t\0"
    "uint_fast64_t\0"
    "uint_fast8_t\0"
    "uint_least16_t\0"
    "uint_least32_t\0"
    "uint_least64_t\0"
    "uint_least8_t\0"
    "uintmax_t\0"
    "uintptr_t\0"
    "ungetc\0"
    "int ungetc(int c, FILE *stream)\0"
    "vfprintf\0"
    "o va_list só existe dentro de uma função variádica do C\0"
    "vfprintf_s\0"
    "vfscanf\0"
    "vfscanf_s\0"
    "vprintf\0"
    "vprintf_s\0"
    "vscanf\0"
    "vscanf_s\0"
    "vsnprintf\0"
    "vsnprintf_s\0"
    "vsprintf\0"
    "vsprintf_s\0"
    "vsscanf\0"
    "vsscanf_s\0"
    "wchar_t\0"
    "wcstoimax\0"
    "intmax_t wcstoimax(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base)\0"
    "wcstombs\0"
    "size_t wcstombs(char * restrict s, const wchar_t * restrict pwcs, size_t n)\0"
    "wcstombs_s\0"
    "wcstoumax\0"
    "uintmax_t wcstoumax(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base)\0"
    "wctomb\0"
    "int wctomb(char *s, wchar_t wchar)\0"
    "wctomb_s\0";

/* os cabeçalhos, na ordem do .decl: o `cab` de cada nome */
static const uint16_t PS_C_CAB[PS_C_NCAB] = { 1, 9, 17, 25, 36, 45, 52, 62, 71, 80, 88, 97, 106 };

/* Os nomes em ordem de strcmp: a busca é binária. Os números são posições
 * em PS_C_TEXTO; `idx` é a linha na tabela da espécie. */
static const PSCNome PS_C_NOMES[PS_C_NNOMES] = {
    /* nome, decl, idx, glibc, cab, especie, marcas */
    { 113, 0, 0, 0, 9, PSC_NOME_CONST, 0 },  /* BUFSIZ */
    { 120, 0, 1, 0, 4, PSC_NOME_CONST, 0 },  /* CHAR_BIT */
    { 129, 0, 2, 0, 4, PSC_NOME_CONST, 0 },  /* CHAR_MAX */
    { 138, 0, 3, 0, 4, PSC_NOME_CONST, 0 },  /* CHAR_MIN */
    { 147, 0, 4, 0, 12, PSC_NOME_CONST, 0 },  /* CLOCKS_PER_SEC */
    { 162, 0, 5, 0, 2, PSC_NOME_CONST, 0 },  /* DBL_DECIMAL_DIG */
    { 178, 0, 6, 0, 2, PSC_NOME_CONST, 0 },  /* DBL_DIG */
    { 186, 0, 0, 0, 2, PSC_NOME_CONSTFLO, 0 },  /* DBL_EPSILON */
    { 198, 0, 7, 0, 2, PSC_NOME_CONST, 0 },  /* DBL_HAS_SUBNORM */
    { 214, 0, 8, 0, 2, PSC_NOME_CONST, 0 },  /* DBL_MANT_DIG */
    { 227, 0, 1, 0, 2, PSC_NOME_CONSTFLO, 0 },  /* DBL_MAX */
    { 235, 0, 9, 0, 2, PSC_NOME_CONST, 0 },  /* DBL_MAX_10_EXP */
    { 250, 0, 10, 0, 2, PSC_NOME_CONST, 0 },  /* DBL_MAX_EXP */
    { 262, 0, 2, 0, 2, PSC_NOME_CONSTFLO, 0 },  /* DBL_MIN */
    { 270, 0, 11, 0, 2, PSC_NOME_CONST, 0 },  /* DBL_MIN_10_EXP */
    { 285, 0, 12, 0, 2, PSC_NOME_CONST, 0 },  /* DBL_MIN_EXP */
    { 297, 0, 3, 0, 2, PSC_NOME_CONSTFLO, 0 },  /* DBL_TRUE_MIN */
    { 310, 0, 13, 0, 2, PSC_NOME_CONST, 0 },  /* DECIMAL_DIG */
    { 322, 0, 14, 0, 1, PSC_NOME_CONST, 0 },  /* EDOM */
    { 327, 0, 15, 0, 1, PSC_NOME_CONST, 0 },  /* EILSEQ */
    { 334, 0, 16, 0, 9, PSC_NOME_CONST, 0 },  /* EOF */
    { 338, 0, 17, 0, 1, PSC_NOME_CONST, 0 },  /* ERANGE */
    { 345, 0, 18, 0, 10, PSC_NOME_CONST, 0 },  /* EXIT_FAILURE */
    { 358, 0, 19, 0, 10, PSC_NOME_CONST, 0 },  /* EXIT_SUCCESS */
    { 371, 0, 0, 0, 9, PSC_NOME_OPACO, 0 },  /* FILE */
    { 376, 0, 20, 0, 9, PSC_NOME_CONST, 0 },  /* FILENAME_MAX */
    { 389, 0, 21, 0, 2, PSC_NOME_CONST, 0 },  /* FLT_DECIMAL_DIG */
    { 405, 0, 22, 0, 2, PSC_NOME_CONST, 0 },  /* FLT_DIG */
    { 413, 0, 4, 0, 2, PSC_NOME_CONSTFLO, 0 },  /* FLT_EPSILON */
    { 425, 0, 23, 0, 2, PSC_NOME_CONST, 0 },  /* FLT_EVAL_METHOD */
    { 441, 0, 24, 0, 2, PSC_NOME_CONST, 0 },  /* FLT_HAS_SUBNORM */
    { 457, 0, 25, 0, 2, PSC_NOME_CONST, 0 },  /* FLT_MANT_DIG */
    { 470, 0, 5, 0, 2, PSC_NOME_CONSTFLO, 0 },  /* FLT_MAX */
    { 478, 0, 26, 0, 2, PSC_NOME_CONST, 0 },  /* FLT_MAX_10_EXP */
    { 493, 0, 27, 0, 2, PSC_NOME_CONST, 0 },  /* FLT_MAX_EXP */
    { 505, 0, 6, 0, 2, PSC_NOME_CONSTFLO, 0 },  /* FLT_MIN */
    { 513, 0, 28, 0, 2, PSC_NOME_CONST, 0 },  /* FLT_MIN_10_EXP */
    { 528, 0, 29, 0, 2, PSC_NOME_CONST, 0 },  /* FLT_MIN_EXP */
    { 540, 0, 30, 0, 2, PSC_NOME_CONST, 0 },  /* FLT_RADIX */
    { 550, 561, 0, 0, 2, PSC_NOME_VALOR, 0 },  /* FLT_ROUNDS */
    { 576, 0, 7, 0, 2, PSC_NOME_CONSTFLO, 0 },  /* FLT_TRUE_MIN */
    { 589, 0, 31, 0, 9, PSC_NOME_CONST, 0 },  /* FOPEN_MAX */
    { 599, 0, 0, 0, 5, PSC_NOME_TALVEZ, 0 },  /* FP_FAST_FMA */
    { 611, 0, 1, 0, 5, PSC_NOME_TALVEZ, 0 },  /* FP_FAST_FMAF */
    { 624, 0, 2, 0, 5, PSC_NOME_TALVEZ, 0 },  /* FP_FAST_FMAL */
    { 637, 0, 32, 0, 5, PSC_NOME_CONST, 0 },  /* FP_ILOGB0 */
    { 647, 0, 33, 0, 5, PSC_NOME_CONST, 0 },  /* FP_ILOGBNAN */
    { 659, 0, 34, 0, 5, PSC_NOME_CONST, 0 },  /* FP_INFINITE */
    { 671, 0, 35, 0, 5, PSC_NOME_CONST, 0 },  /* FP_NAN */
    { 678, 0, 36, 0, 5, PSC_NOME_CONST, 0 },  /* FP_NORMAL */
    { 688, 0, 37, 0, 5, PSC_NOME_CONST, 0 },  /* FP_SUBNORMAL */
    { 701, 0, 38, 0, 5, PSC_NOME_CONST, 0 },  /* FP_ZERO */
    { 709, 0, 8, 0, 5, PSC_NOME_CONSTFLO, 0 },  /* HUGE_VAL */
    { 718, 0, 9, 0, 5, PSC_NOME_CONSTFLO, 0 },  /* HUGE_VALF */
    { 728, 0, 10, 0, 5, PSC_NOME_CONSTFLO, 0 },  /* HUGE_VALL */
    { 738, 0, 11, 0, 5, PSC_NOME_CONSTFLO, 0 },  /* INFINITY */
    { 747, 755, 0, 0, 8, PSC_NOME_LITERAL, 0 },  /* INT16_C */
    { 778, 0, 39, 0, 8, PSC_NOME_CONST, 0 },  /* INT16_MAX */
    { 788, 0, 40, 0, 8, PSC_NOME_CONST, 0 },  /* INT16_MIN */
    { 798, 806, 1, 0, 8, PSC_NOME_LITERAL, 0 },  /* INT32_C */
    { 849, 0, 41, 0, 8, PSC_NOME_CONST, 0 },  /* INT32_MAX */
    { 859, 0, 42, 0, 8, PSC_NOME_CONST, 0 },  /* INT32_MIN */
    { 869, 877, 2, 0, 8, PSC_NOME_LITERAL, 0 },  /* INT64_C */
    { 920, 0, 43, 0, 8, PSC_NOME_CONST, 0 },  /* INT64_MAX */
    { 930, 0, 44, 0, 8, PSC_NOME_CONST, 0 },  /* INT64_MIN */
    { 940, 947, 3, 0, 8, PSC_NOME_LITERAL, 0 },  /* INT8_C */
    { 969, 0, 45, 0, 8, PSC_NOME_CONST, 0 },  /* INT8_MAX */
    { 978, 0, 46, 0, 8, PSC_NOME_CONST, 0 },  /* INT8_MIN */
    { 987, 996, 4, 0, 8, PSC_NOME_LITERAL, 0 },  /* INTMAX_C */
    { 1030, 0, 47, 0, 8, PSC_NOME_CONST, 0 },  /* INTMAX_MAX */
    { 1041, 0, 48, 0, 8, PSC_NOME_CONST, 0 },  /* INTMAX_MIN */
    { 1052, 0, 49, 0, 8, PSC_NOME_CONST, 0 },  /* INTPTR_MAX */
    { 1063, 0, 50, 0, 8, PSC_NOME_CONST, 0 },  /* INTPTR_MIN */
    { 1074, 0, 51, 0, 8, PSC_NOME_CONST, 0 },  /* INT_FAST16_MAX */
    { 1089, 0, 52, 0, 8, PSC_NOME_CONST, 0 },  /* INT_FAST16_MIN */
    { 1104, 0, 53, 0, 8, PSC_NOME_CONST, 0 },  /* INT_FAST32_MAX */
    { 1119, 0, 54, 0, 8, PSC_NOME_CONST, 0 },  /* INT_FAST32_MIN */
    { 1134, 0, 55, 0, 8, PSC_NOME_CONST, 0 },  /* INT_FAST64_MAX */
    { 1149, 0, 56, 0, 8, PSC_NOME_CONST, 0 },  /* INT_FAST64_MIN */
    { 1164, 0, 57, 0, 8, PSC_NOME_CONST, 0 },  /* INT_FAST8_MAX */
    { 1178, 0, 58, 0, 8, PSC_NOME_CONST, 0 },  /* INT_FAST8_MIN */
    { 1192, 0, 59, 0, 8, PSC_NOME_CONST, 0 },  /* INT_LEAST16_MAX */
    { 1208, 0, 60, 0, 8, PSC_NOME_CONST, 0 },  /* INT_LEAST16_MIN */
    { 1224, 0, 61, 0, 8, PSC_NOME_CONST, 0 },  /* INT_LEAST32_MAX */
    { 1240, 0, 62, 0, 8, PSC_NOME_CONST, 0 },  /* INT_LEAST32_MIN */
    { 1256, 0, 63, 0, 8, PSC_NOME_CONST, 0 },  /* INT_LEAST64_MAX */
    { 1272, 0, 64, 0, 8, PSC_NOME_CONST, 0 },  /* INT_LEAST64_MIN */
    { 1288, 0, 65, 0, 8, PSC_NOME_CONST, 0 },  /* INT_LEAST8_MAX */
    { 1303, 0, 66, 0, 8, PSC_NOME_CONST, 0 },  /* INT_LEAST8_MIN */
    { 1318, 0, 67, 0, 4, PSC_NOME_CONST, 0 },  /* INT_MAX */
    { 1326, 0, 68, 0, 4, PSC_NOME_CONST, 0 },  /* INT_MIN */
    { 1334, 0, 69, 0, 2, PSC_NOME_CONST, 0 },  /* LDBL_DECIMAL_DIG */
    { 1351, 0, 70, 0, 2, PSC_NOME_CONST, 0 },  /* LDBL_DIG */
    { 1360, 0, 12, 0, 2, PSC_NOME_CONSTFLO, 0 },  /* LDBL_EPSILON */
    { 1373, 0, 71, 0, 2, PSC_NOME_CONST, 0 },  /* LDBL_HAS_SUBNORM */
    { 1390, 0, 72, 0, 2, PSC_NOME_CONST, 0 },  /* LDBL_MANT_DIG */
    { 1404, 0, 13, 0, 2, PSC_NOME_CONSTFLO, 0 },  /* LDBL_MAX */
    { 1413, 0, 73, 0, 2, PSC_NOME_CONST, 0 },  /* LDBL_MAX_10_EXP */
    { 1429, 0, 74, 0, 2, PSC_NOME_CONST, 0 },  /* LDBL_MAX_EXP */
    { 1442, 0, 14, 0, 2, PSC_NOME_CONSTFLO, 0 },  /* LDBL_MIN */
    { 1451, 0, 75, 0, 2, PSC_NOME_CONST, 0 },  /* LDBL_MIN_10_EXP */
    { 1467, 0, 76, 0, 2, PSC_NOME_CONST, 0 },  /* LDBL_MIN_EXP */
    { 1480, 0, 15, 0, 2, PSC_NOME_CONSTFLO, 0 },  /* LDBL_TRUE_MIN */
    { 1494, 0, 77, 0, 4, PSC_NOME_CONST, 0 },  /* LLONG_MAX */
    { 1504, 0, 78, 0, 4, PSC_NOME_CONST, 0 },  /* LLONG_MIN */
    { 1514, 0, 79, 0, 4, PSC_NOME_CONST, 0 },  /* LONG_MAX */
    { 1523, 0, 80, 0, 4, PSC_NOME_CONST, 0 },  /* LONG_MIN */
    { 1532, 0, 81, 0, 9, PSC_NOME_CONST, 0 },  /* L_tmpnam */
    { 1541, 1552, 0, 0, 9, PSC_NOME_FORA, 0 },  /* L_tmpnam_s */
    { 1626, 0, 82, 0, 5, PSC_NOME_CONST, 0 },  /* MATH_ERREXCEPT */
    { 1641, 0, 83, 0, 5, PSC_NOME_CONST, 0 },  /* MATH_ERRNO */
    { 1652, 1663, 1, 0, 10, PSC_NOME_VALOR, 0 },  /* MB_CUR_MAX */
    { 1681, 0, 84, 0, 4, PSC_NOME_CONST, 0 },  /* MB_LEN_MAX */
    { 1692, 0, 16, 0, 5, PSC_NOME_CONSTFLO, 0 },  /* NAN */
    { 1696, 0, 0, 0, 7, PSC_NOME_NULO, 0 },  /* NULL */
    { 1701, 0, 0, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIX16 */
    { 1708, 0, 1, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIX32 */
    { 1715, 0, 2, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIX64 */
    { 1722, 0, 3, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIX8 */
    { 1728, 0, 4, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXFAST16 */
    { 1739, 0, 5, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXFAST32 */
    { 1750, 0, 6, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXFAST64 */
    { 1761, 0, 7, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXFAST8 */
    { 1771, 0, 8, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXLEAST16 */
    { 1783, 0, 9, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXLEAST32 */
    { 1795, 0, 10, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXLEAST64 */
    { 1807, 0, 11, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXLEAST8 */
    { 1818, 0, 12, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXMAX */
    { 1826, 0, 13, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXPTR */
    { 1834, 0, 14, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRId16 */
    { 1841, 0, 15, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRId32 */
    { 1848, 0, 16, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRId64 */
    { 1855, 0, 17, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRId8 */
    { 1861, 0, 18, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdFAST16 */
    { 1872, 0, 19, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdFAST32 */
    { 1883, 0, 20, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdFAST64 */
    { 1894, 0, 21, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdFAST8 */
    { 1904, 0, 22, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdLEAST16 */
    { 1916, 0, 23, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdLEAST32 */
    { 1928, 0, 24, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdLEAST64 */
    { 1940, 0, 25, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdLEAST8 */
    { 1951, 0, 26, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdMAX */
    { 1959, 0, 27, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdPTR */
    { 1967, 0, 28, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIi16 */
    { 1974, 0, 29, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIi32 */
    { 1981, 0, 30, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIi64 */
    { 1988, 0, 31, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIi8 */
    { 1994, 0, 32, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiFAST16 */
    { 2005, 0, 33, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiFAST32 */
    { 2016, 0, 34, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiFAST64 */
    { 2027, 0, 35, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiFAST8 */
    { 2037, 0, 36, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiLEAST16 */
    { 2049, 0, 37, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiLEAST32 */
    { 2061, 0, 38, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiLEAST64 */
    { 2073, 0, 39, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiLEAST8 */
    { 2084, 0, 40, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiMAX */
    { 2092, 0, 41, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiPTR */
    { 2100, 0, 42, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIo16 */
    { 2107, 0, 43, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIo32 */
    { 2114, 0, 44, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIo64 */
    { 2121, 0, 45, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIo8 */
    { 2127, 0, 46, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoFAST16 */
    { 2138, 0, 47, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoFAST32 */
    { 2149, 0, 48, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoFAST64 */
    { 2160, 0, 49, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoFAST8 */
    { 2170, 0, 50, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoLEAST16 */
    { 2182, 0, 51, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoLEAST32 */
    { 2194, 0, 52, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoLEAST64 */
    { 2206, 0, 53, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoLEAST8 */
    { 2217, 0, 54, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoMAX */
    { 2225, 0, 55, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoPTR */
    { 2233, 0, 56, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIu16 */
    { 2240, 0, 57, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIu32 */
    { 2247, 0, 58, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIu64 */
    { 2254, 0, 59, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIu8 */
    { 2260, 0, 60, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuFAST16 */
    { 2271, 0, 61, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuFAST32 */
    { 2282, 0, 62, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuFAST64 */
    { 2293, 0, 63, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuFAST8 */
    { 2303, 0, 64, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuLEAST16 */
    { 2315, 0, 65, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuLEAST32 */
    { 2327, 0, 66, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuLEAST64 */
    { 2339, 0, 67, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuLEAST8 */
    { 2350, 0, 68, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuMAX */
    { 2358, 0, 69, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuPTR */
    { 2366, 0, 70, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIx16 */
    { 2373, 0, 71, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIx32 */
    { 2380, 0, 72, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIx64 */
    { 2387, 0, 73, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIx8 */
    { 2393, 0, 74, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxFAST16 */
    { 2404, 0, 75, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxFAST32 */
    { 2415, 0, 76, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxFAST64 */
    { 2426, 0, 77, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxFAST8 */
    { 2436, 0, 78, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxLEAST16 */
    { 2448, 0, 79, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxLEAST32 */
    { 2460, 0, 80, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxLEAST64 */
    { 2472, 0, 81, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxLEAST8 */
    { 2483, 0, 82, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxMAX */
    { 2491, 0, 83, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxPTR */
    { 2499, 0, 85, 0, 8, PSC_NOME_CONST, 0 },  /* PTRDIFF_MAX */
    { 2511, 0, 86, 0, 8, PSC_NOME_CONST, 0 },  /* PTRDIFF_MIN */
    { 2523, 0, 87, 0, 10, PSC_NOME_CONST, 0 },  /* RAND_MAX */
    { 2532, 1552, 1, 0, 8, PSC_NOME_FORA, 0 },  /* RSIZE_MAX */
    { 2542, 0, 88, 0, 4, PSC_NOME_CONST, 0 },  /* SCHAR_MAX */
    { 2552, 0, 89, 0, 4, PSC_NOME_CONST, 0 },  /* SCHAR_MIN */
    { 2562, 0, 84, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNd16 */
    { 2569, 0, 85, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNd32 */
    { 2576, 0, 86, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNd64 */
    { 2583, 0, 87, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNd8 */
    { 2589, 0, 88, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdFAST16 */
    { 2600, 0, 89, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdFAST32 */
    { 2611, 0, 90, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdFAST64 */
    { 2622, 0, 91, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdFAST8 */
    { 2632, 0, 92, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdLEAST16 */
    { 2644, 0, 93, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdLEAST32 */
    { 2656, 0, 94, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdLEAST64 */
    { 2668, 0, 95, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdLEAST8 */
    { 2679, 0, 96, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdMAX */
    { 2687, 0, 97, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdPTR */
    { 2695, 0, 98, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNi16 */
    { 2702, 0, 99, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNi32 */
    { 2709, 0, 100, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNi64 */
    { 2716, 0, 101, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNi8 */
    { 2722, 0, 102, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiFAST16 */
    { 2733, 0, 103, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiFAST32 */
    { 2744, 0, 104, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiFAST64 */
    { 2755, 0, 105, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiFAST8 */
    { 2765, 0, 106, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiLEAST16 */
    { 2777, 0, 107, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiLEAST32 */
    { 2789, 0, 108, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiLEAST64 */
    { 2801, 0, 109, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiLEAST8 */
    { 2812, 0, 110, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiMAX */
    { 2820, 0, 111, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiPTR */
    { 2828, 0, 112, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNo16 */
    { 2835, 0, 113, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNo32 */
    { 2842, 0, 114, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNo64 */
    { 2849, 0, 115, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNo8 */
    { 2855, 0, 116, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoFAST16 */
    { 2866, 0, 117, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoFAST32 */
    { 2877, 0, 118, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoFAST64 */
    { 2888, 0, 119, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoFAST8 */
    { 2898, 0, 120, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoLEAST16 */
    { 2910, 0, 121, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoLEAST32 */
    { 2922, 0, 122, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoLEAST64 */
    { 2934, 0, 123, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoLEAST8 */
    { 2945, 0, 124, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoMAX */
    { 2953, 0, 125, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoPTR */
    { 2961, 0, 126, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNu16 */
    { 2968, 0, 127, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNu32 */
    { 2975, 0, 128, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNu64 */
    { 2982, 0, 129, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNu8 */
    { 2988, 0, 130, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuFAST16 */
    { 2999, 0, 131, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuFAST32 */
    { 3010, 0, 132, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuFAST64 */
    { 3021, 0, 133, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuFAST8 */
    { 3031, 0, 134, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuLEAST16 */
    { 3043, 0, 135, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuLEAST32 */
    { 3055, 0, 136, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuLEAST64 */
    { 3067, 0, 137, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuLEAST8 */
    { 3078, 0, 138, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuMAX */
    { 3086, 0, 139, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuPTR */
    { 3094, 0, 140, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNx16 */
    { 3101, 0, 141, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNx32 */
    { 3108, 0, 142, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNx64 */
    { 3115, 0, 143, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNx8 */
    { 3121, 0, 144, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxFAST16 */
    { 3132, 0, 145, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxFAST32 */
    { 3143, 0, 146, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxFAST64 */
    { 3154, 0, 147, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxFAST8 */
    { 3164, 0, 148, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxLEAST16 */
    { 3176, 0, 149, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxLEAST32 */
    { 3188, 0, 150, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxLEAST64 */
    { 3200, 0, 151, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxLEAST8 */
    { 3211, 0, 152, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxMAX */
    { 3219, 0, 153, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxPTR */
    { 3227, 0, 90, 0, 9, PSC_NOME_CONST, 0 },  /* SEEK_CUR */
    { 3236, 0, 91, 0, 9, PSC_NOME_CONST, 0 },  /* SEEK_END */
    { 3245, 0, 92, 0, 9, PSC_NOME_CONST, 0 },  /* SEEK_SET */
    { 3254, 0, 93, 0, 4, PSC_NOME_CONST, 0 },  /* SHRT_MAX */
    { 3263, 0, 94, 0, 4, PSC_NOME_CONST, 0 },  /* SHRT_MIN */
    { 3272, 0, 95, 0, 8, PSC_NOME_CONST, 0 },  /* SIG_ATOMIC_MAX */
    { 3287, 0, 96, 0, 8, PSC_NOME_CONST, 0 },  /* SIG_ATOMIC_MIN */
    { 3302, 0, 97, 0, 8, PSC_NOME_CONST, 0 },  /* SIZE_MAX */
    { 3311, 0, 98, 0, 12, PSC_NOME_CONST, 0 },  /* TIME_UTC */
    { 3320, 0, 99, 0, 9, PSC_NOME_CONST, 0 },  /* TMP_MAX */
    { 3328, 1552, 2, 0, 9, PSC_NOME_FORA, 0 },  /* TMP_MAX_S */
    { 3338, 0, 100, 0, 4, PSC_NOME_CONST, 0 },  /* UCHAR_MAX */
    { 3348, 3357, 5, 0, 8, PSC_NOME_LITERAL, 0 },  /* UINT16_C */
    { 3381, 0, 101, 0, 8, PSC_NOME_CONST, 0 },  /* UINT16_MAX */
    { 3392, 3401, 6, 0, 8, PSC_NOME_LITERAL, 0 },  /* UINT32_C */
    { 3447, 0, 102, 0, 8, PSC_NOME_CONST, 0 },  /* UINT32_MAX */
    { 3458, 3467, 7, 0, 8, PSC_NOME_LITERAL, 0 },  /* UINT64_C */
    { 3513, 0, 103, 0, 8, PSC_NOME_CONST, 0 },  /* UINT64_MAX */
    { 3524, 3532, 8, 0, 8, PSC_NOME_LITERAL, 0 },  /* UINT8_C */
    { 3555, 0, 104, 0, 8, PSC_NOME_CONST, 0 },  /* UINT8_MAX */
    { 3565, 3575, 9, 0, 8, PSC_NOME_LITERAL, 0 },  /* UINTMAX_C */
    { 3612, 0, 105, 0, 8, PSC_NOME_CONST, 0 },  /* UINTMAX_MAX */
    { 3624, 0, 106, 0, 8, PSC_NOME_CONST, 0 },  /* UINTPTR_MAX */
    { 3636, 0, 107, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_FAST16_MAX */
    { 3652, 0, 108, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_FAST32_MAX */
    { 3668, 0, 109, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_FAST64_MAX */
    { 3684, 0, 110, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_FAST8_MAX */
    { 3699, 0, 111, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_LEAST16_MAX */
    { 3716, 0, 112, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_LEAST32_MAX */
    { 3733, 0, 113, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_LEAST64_MAX */
    { 3750, 0, 114, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_LEAST8_MAX */
    { 3766, 0, 115, 0, 4, PSC_NOME_CONST, 0 },  /* UINT_MAX */
    { 3775, 0, 116, 0, 4, PSC_NOME_CONST, 0 },  /* ULLONG_MAX */
    { 3786, 0, 117, 0, 4, PSC_NOME_CONST, 0 },  /* ULONG_MAX */
    { 3796, 0, 118, 0, 4, PSC_NOME_CONST, 0 },  /* USHRT_MAX */
    { 3806, 0, 119, 0, 8, PSC_NOME_CONST, 0 },  /* WCHAR_MAX */
    { 3816, 0, 120, 0, 8, PSC_NOME_CONST, 0 },  /* WCHAR_MIN */
    { 3826, 0, 121, 0, 8, PSC_NOME_CONST, 0 },  /* WINT_MAX */
    { 3835, 0, 122, 0, 8, PSC_NOME_CONST, 0 },  /* WINT_MIN */
    { 3844, 3850, 0, 0, 10, PSC_NOME_FUNCAO, 0 },  /* _Exit */
    { 3883, 0, 123, 0, 9, PSC_NOME_CONST, 0 },  /* _IOFBF */
    { 3890, 0, 124, 0, 9, PSC_NOME_CONST, 0 },  /* _IOLBF */
    { 3897, 0, 125, 0, 9, PSC_NOME_CONST, 0 },  /* _IONBF */
    { 3904, 1552, 3, 0, 1, PSC_NOME_FORA, 0 },  /* __STDC_WANT_LIB_EXT1__ */
    { 3927, 0, 126, 0, 6, PSC_NOME_CONST, 0 },  /* __bool_true_false_are_defined */
    { 3957, 3963, 1, 0, 10, PSC_NOME_FUNCAO, 0 },  /* abort */
    { 3990, 1552, 4, 0, 10, PSC_NOME_FORA, 0 },  /* abort_handler_s */
    { 4006, 4010, 2, 0, 10, PSC_NOME_FUNCAO, 0 },  /* abs */
    { 4025, 4030, 3, 0, 5, PSC_NOME_FUNCAO, 0 },  /* acos */
    { 4052, 4058, 4, 0, 5, PSC_NOME_FUNCAO, 0 },  /* acosf */
    { 4079, 4085, 5, 0, 5, PSC_NOME_FUNCAO, 0 },  /* acosh */
    { 4108, 4115, 6, 0, 5, PSC_NOME_FUNCAO, 0 },  /* acoshf */
    { 4137, 4144, 7, 0, 5, PSC_NOME_FUNCAO, 0 },  /* acoshl */
    { 4178, 4184, 8, 0, 5, PSC_NOME_FUNCAO, 0 },  /* acosl */
    { 4217, 4231, 9, 0, 10, PSC_NOME_FUNCAO, 0 },  /* aligned_alloc */
    { 4282, 4290, 10, 0, 12, PSC_NOME_FUNCAO, 0 },  /* asctime */
    { 4330, 1552, 5, 0, 12, PSC_NOME_FORA, 0 },  /* asctime_s */
    { 4340, 4345, 11, 0, 5, PSC_NOME_FUNCAO, 0 },  /* asin */
    { 4367, 4373, 12, 0, 5, PSC_NOME_FUNCAO, 0 },  /* asinf */
    { 4394, 4400, 13, 0, 5, PSC_NOME_FUNCAO, 0 },  /* asinh */
    { 4423, 4430, 14, 0, 5, PSC_NOME_FUNCAO, 0 },  /* asinhf */
    { 4452, 4459, 15, 0, 5, PSC_NOME_FUNCAO, 0 },  /* asinhl */
    { 4493, 4499, 16, 0, 5, PSC_NOME_FUNCAO, 0 },  /* asinl */
    { 4532, 4546, 17, 0, 10, PSC_NOME_FUNCAO, PSC_MARCA_VM },  /* at_quick_exit */
    { 4584, 4589, 18, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atan */
    { 4611, 4617, 19, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atan2 */
    { 4650, 4657, 20, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atan2f */
    { 4688, 4695, 21, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atan2l */
    { 4744, 4750, 22, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atanf */
    { 4771, 4777, 23, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atanh */
    { 4800, 4807, 24, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atanhf */
    { 4829, 4836, 25, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atanhl */
    { 4870, 4876, 26, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atanl */
    { 4909, 4916, 27, 0, 10, PSC_NOME_FUNCAO, PSC_MARCA_VM },  /* atexit */
    { 4947, 4952, 28, 0, 10, PSC_NOME_FUNCAO, 0 },  /* atof */
    { 4982, 4987, 29, 0, 10, PSC_NOME_FUNCAO, 0 },  /* atoi */
    { 5014, 5019, 30, 0, 10, PSC_NOME_FUNCAO, 0 },  /* atol */
    { 5051, 5057, 31, 0, 10, PSC_NOME_FUNCAO, 0 },  /* atoll */
    { 5095, 0, 0, 0, 6, PSC_NOME_TIPO, 0 },  /* bool */
    { 5100, 5108, 32, 0, 10, PSC_NOME_FUNCAO, 0 },  /* bsearch */
    { 5227, 1552, 6, 0, 10, PSC_NOME_FORA, 0 },  /* bsearch_s */
    { 5237, 5244, 33, 0, 10, PSC_NOME_FUNCAO, 0 },  /* calloc */
    { 5284, 5289, 34, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cbrt */
    { 5311, 5317, 35, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cbrtf */
    { 5338, 5344, 36, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cbrtl */
    { 5377, 5382, 37, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ceil */
    { 5404, 5410, 38, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ceilf */
    { 5431, 5437, 39, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ceill */
    { 5470, 5479, 40, 0, 9, PSC_NOME_FUNCAO, 0 },  /* clearerr */
    { 5507, 5513, 41, 0, 12, PSC_NOME_FUNCAO, 0 },  /* clock */
    { 5533, 0, 1, 0, 12, PSC_NOME_TIPO, 0 },  /* clock_t */
    { 5541, 1552, 7, 0, 10, PSC_NOME_FORA, 0 },  /* constraint_handler_t */
    { 5562, 5571, 42, 0, 5, PSC_NOME_FUNCAO, 0 },  /* copysign */
    { 5607, 5617, 43, 0, 5, PSC_NOME_FUNCAO, 0 },  /* copysignf */
    { 5651, 5661, 44, 0, 5, PSC_NOME_FUNCAO, 0 },  /* copysignl */
    { 5713, 5717, 45, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cos */
    { 5738, 5743, 46, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cosf */
    { 5763, 5768, 47, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cosh */
    { 5790, 5796, 48, 0, 5, PSC_NOME_FUNCAO, 0 },  /* coshf */
    { 5817, 5823, 49, 0, 5, PSC_NOME_FUNCAO, 0 },  /* coshl */
    { 5856, 5861, 50, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cosl */
    { 5893, 5899, 51, 0, 12, PSC_NOME_FUNCAO, 0 },  /* ctime */
    { 5932, 1552, 8, 0, 12, PSC_NOME_FORA, 0 },  /* ctime_s */
    { 5940, 5949, 52, 0, 12, PSC_NOME_FUNCAO, 0 },  /* difftime */
    { 5993, 5997, 53, 0, 10, PSC_NOME_FUNCAO, 0 },  /* div */
    { 6029, 6029, 0, 0, 10, PSC_NOME_ESTRUTURA, 0 },  /* div_t */
    { 6061, 0, 2, 0, 5, PSC_NOME_TIPO, 0 },  /* double_t */
    { 6070, 6074, 54, 0, 5, PSC_NOME_FUNCAO, 0 },  /* erf */
    { 6095, 6100, 55, 0, 5, PSC_NOME_FUNCAO, 0 },  /* erfc */
    { 6122, 6128, 56, 0, 5, PSC_NOME_FUNCAO, 0 },  /* erfcf */
    { 6149, 6155, 57, 0, 5, PSC_NOME_FUNCAO, 0 },  /* erfcl */
    { 6188, 6193, 58, 0, 5, PSC_NOME_FUNCAO, 0 },  /* erff */
    { 6213, 6218, 59, 0, 5, PSC_NOME_FUNCAO, 0 },  /* erfl */
    { 6250, 6256, 2, 0, 1, PSC_NOME_VALOR, PSC_MARCA_VM },  /* errno */
    { 6266, 1552, 9, 0, 1, PSC_NOME_FORA, 0 },  /* errno_t */
    { 6274, 6279, 60, 0, 10, PSC_NOME_FUNCAO, PSC_MARCA_VM },  /* exit */
    { 6311, 6315, 61, 0, 5, PSC_NOME_FUNCAO, 0 },  /* exp */
    { 6336, 6341, 62, 0, 5, PSC_NOME_FUNCAO, 0 },  /* exp2 */
    { 6363, 6369, 63, 0, 5, PSC_NOME_FUNCAO, 0 },  /* exp2f */
    { 6390, 6396, 64, 0, 5, PSC_NOME_FUNCAO, 0 },  /* exp2l */
    { 6429, 6434, 65, 0, 5, PSC_NOME_FUNCAO, 0 },  /* expf */
    { 6454, 6459, 66, 0, 5, PSC_NOME_FUNCAO, 0 },  /* expl */
    { 6491, 6497, 67, 0, 5, PSC_NOME_FUNCAO, 0 },  /* expm1 */
    { 6520, 6527, 68, 0, 5, PSC_NOME_FUNCAO, 0 },  /* expm1f */
    { 6549, 6556, 69, 0, 5, PSC_NOME_FUNCAO, 0 },  /* expm1l */
    { 6590, 6595, 70, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fabs */
    { 6617, 6623, 71, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fabsf */
    { 6644, 6650, 72, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fabsl */
    { 6683, 6690, 73, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fclose */
    { 6715, 6720, 74, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fdim */
    { 6752, 6758, 75, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fdimf */
    { 6788, 6794, 76, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fdiml */
    { 6842, 6847, 77, 0, 9, PSC_NOME_FUNCAO, 0 },  /* feof */
    { 6870, 6877, 78, 0, 9, PSC_NOME_FUNCAO, 0 },  /* ferror */
    { 6902, 6909, 79, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fflush */
    { 6934, 6940, 80, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fgetc */
    { 6964, 6972, 81, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fgetpos */
    { 7031, 7037, 82, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fgets */
    { 7099, 0, 3, 0, 5, PSC_NOME_TIPO, 0 },  /* float_t */
    { 7107, 7113, 83, 0, 5, PSC_NOME_FUNCAO, 0 },  /* floor */
    { 7136, 7143, 84, 0, 5, PSC_NOME_FUNCAO, 0 },  /* floorf */
    { 7165, 7172, 85, 0, 5, PSC_NOME_FUNCAO, 0 },  /* floorl */
    { 7206, 7210, 86, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fma */
    { 7251, 7256, 87, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmaf */
    { 7294, 7299, 88, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmal */
    { 7361, 7366, 89, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmax */
    { 7398, 7404, 90, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmaxf */
    { 7434, 7440, 91, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmaxl */
    { 7488, 7493, 92, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmin */
    { 7525, 7531, 93, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fminf */
    { 7561, 7567, 94, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fminl */
    { 7615, 7620, 95, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmod */
    { 7652, 7658, 96, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmodf */
    { 7688, 7694, 97, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmodl */
    { 7742, 7748, 98, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fopen */
    { 7820, 1552, 10, 0, 9, PSC_NOME_FORA, 0 },  /* fopen_s */
    { 7828, 7839, 0, 0, 5, PSC_NOME_MACRO, 0 },  /* fpclassify */
    { 7864, 0, 1, 0, 9, PSC_NOME_OPACO, 0 },  /* fpos_t */
    { 7871, 7879, 99, 0, 9, PSC_NOME_FUNCAO, PSC_MARCA_PRINTF },  /* fprintf */
    { 7950, 1552, 11, 0, 9, PSC_NOME_FORA, 0 },  /* fprintf_s */
    { 7960, 7966, 100, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fputc */
    { 7997, 8003, 101, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fputs */
    { 8062, 8068, 102, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fread */
    { 8153, 8158, 103, 0, 10, PSC_NOME_FUNCAO, 0 },  /* free */
    { 8179, 8187, 104, 0, 9, PSC_NOME_FUNCAO, 0 },  /* freopen */
    { 8285, 1552, 12, 0, 9, PSC_NOME_FORA, 0 },  /* freopen_s */
    { 8295, 8301, 105, 0, 5, PSC_NOME_FUNCAO, 0 },  /* frexp */
    { 8338, 8345, 106, 0, 5, PSC_NOME_FUNCAO, 0 },  /* frexpf */
    { 8381, 8388, 107, 0, 5, PSC_NOME_FUNCAO, 0 },  /* frexpl */
    { 8436, 8443, 108, 8513, 9, PSC_NOME_FUNCAO, PSC_MARCA_SCANF },  /* fscanf */
    { 8529, 1552, 13, 0, 9, PSC_NOME_FORA, 0 },  /* fscanf_s */
    { 8538, 8544, 109, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fseek */
    { 8597, 8605, 110, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fsetpos */
    { 8650, 8656, 111, 0, 9, PSC_NOME_FUNCAO, 0 },  /* ftell */
    { 8685, 8692, 112, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fwrite */
    { 8784, 8789, 113, 0, 9, PSC_NOME_FUNCAO, 0 },  /* getc */
    { 8812, 8820, 114, 0, 9, PSC_NOME_FUNCAO, 0 },  /* getchar */
    { 8838, 8845, 115, 0, 10, PSC_NOME_FUNCAO, 0 },  /* getenv */
    { 8876, 1552, 14, 0, 10, PSC_NOME_FORA, 0 },  /* getenv_s */
    { 8885, 8890, 15, 0, 9, PSC_NOME_FORA, 0 },  /* gets */
    { 8963, 1552, 16, 0, 9, PSC_NOME_FORA, 0 },  /* gets_s */
    { 8970, 8977, 116, 0, 12, PSC_NOME_FUNCAO, 0 },  /* gmtime */
    { 9016, 1552, 17, 0, 12, PSC_NOME_FORA, 0 },  /* gmtime_s */
    { 9025, 9031, 117, 0, 5, PSC_NOME_FUNCAO, 0 },  /* hypot */
    { 9064, 9071, 118, 0, 5, PSC_NOME_FUNCAO, 0 },  /* hypotf */
    { 9102, 9109, 119, 0, 5, PSC_NOME_FUNCAO, 0 },  /* hypotl */
    { 9158, 1552, 18, 0, 10, PSC_NOME_FORA, 0 },  /* ignore_handler_s */
    { 9175, 9181, 120, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ilogb */
    { 9201, 9208, 121, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ilogbf */
    { 9228, 9235, 122, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ilogbl */
    { 9261, 9269, 123, 0, 3, PSC_NOME_FUNCAO, 0 },  /* imaxabs */
    { 9298, 9306, 124, 0, 3, PSC_NOME_FUNCAO, 0 },  /* imaxdiv */
    { 9356, 9356, 1, 0, 3, PSC_NOME_ESTRUTURA, 0 },  /* imaxdiv_t */
    { 9393, 0, 4, 0, 8, PSC_NOME_TIPO, 0 },  /* int16_t */
    { 9401, 0, 5, 0, 8, PSC_NOME_TIPO, 0 },  /* int32_t */
    { 9409, 0, 6, 0, 8, PSC_NOME_TIPO, 0 },  /* int64_t */
    { 9417, 0, 7, 0, 8, PSC_NOME_TIPO, 0 },  /* int8_t */
    { 9424, 0, 8, 0, 8, PSC_NOME_TIPO, 0 },  /* int_fast16_t */
    { 9437, 0, 9, 0, 8, PSC_NOME_TIPO, 0 },  /* int_fast32_t */
    { 9450, 0, 10, 0, 8, PSC_NOME_TIPO, 0 },  /* int_fast64_t */
    { 9463, 0, 11, 0, 8, PSC_NOME_TIPO, 0 },  /* int_fast8_t */
    { 9475, 0, 12, 0, 8, PSC_NOME_TIPO, 0 },  /* int_least16_t */
    { 9489, 0, 13, 0, 8, PSC_NOME_TIPO, 0 },  /* int_least32_t */
    { 9503, 0, 14, 0, 8, PSC_NOME_TIPO, 0 },  /* int_least64_t */
    { 9517, 0, 15, 0, 8, PSC_NOME_TIPO, 0 },  /* int_least8_t */
    { 9530, 0, 16, 0, 8, PSC_NOME_TIPO, 0 },  /* intmax_t */
    { 9539, 0, 17, 0, 8, PSC_NOME_TIPO, 0 },  /* intptr_t */
    { 9548, 9556, 125, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isalnum */
    { 9575, 9583, 126, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isalpha */
    { 9602, 9610, 127, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isblank */
    { 9629, 9637, 128, 0, 0, PSC_NOME_FUNCAO, 0 },  /* iscntrl */
    { 9656, 9664, 129, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isdigit */
    { 9683, 9692, 1, 0, 5, PSC_NOME_MACRO, 0 },  /* isfinite */
    { 9715, 9723, 130, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isgraph */
    { 9742, 9752, 2, 0, 5, PSC_NOME_MACRO, 0 },  /* isgreater */
    { 9786, 9801, 3, 0, 5, PSC_NOME_MACRO, 0 },  /* isgreaterequal */
    { 9840, 9846, 4, 0, 5, PSC_NOME_MACRO, 0 },  /* isinf */
    { 9866, 9873, 5, 0, 5, PSC_NOME_MACRO, 0 },  /* isless */
    { 9904, 9916, 6, 0, 5, PSC_NOME_MACRO, 0 },  /* islessequal */
    { 9952, 9966, 7, 0, 5, PSC_NOME_MACRO, 0 },  /* islessgreater */
    { 10004, 10012, 131, 0, 0, PSC_NOME_FUNCAO, 0 },  /* islower */
    { 10031, 10037, 8, 0, 5, PSC_NOME_MACRO, 0 },  /* isnan */
    { 10057, 10066, 9, 0, 5, PSC_NOME_MACRO, 0 },  /* isnormal */
    { 10089, 10097, 132, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isprint */
    { 10116, 10124, 133, 0, 0, PSC_NOME_FUNCAO, 0 },  /* ispunct */
    { 10143, 10151, 134, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isspace */
    { 10170, 10182, 10, 0, 5, PSC_NOME_MACRO, 0 },  /* isunordered */
    { 10218, 10226, 135, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isupper */
    { 10245, 10254, 136, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isxdigit */
    { 10274, 10279, 137, 0, 10, PSC_NOME_FUNCAO, 0 },  /* labs */
    { 10305, 10311, 138, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ldexp */
    { 10343, 10350, 139, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ldexpf */
    { 10381, 10388, 140, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ldexpl */
    { 10431, 10436, 141, 0, 10, PSC_NOME_FUNCAO, 0 },  /* ldiv */
    { 10480, 10480, 2, 0, 10, PSC_NOME_ESTRUTURA, 0 },  /* ldiv_t */
    { 10514, 10521, 142, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lgamma */
    { 10545, 10553, 143, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lgammaf */
    { 10576, 10584, 144, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lgammal */
    { 10619, 10625, 145, 0, 10, PSC_NOME_FUNCAO, 0 },  /* llabs */
    { 10662, 10668, 146, 0, 10, PSC_NOME_FUNCAO, 0 },  /* lldiv */
    { 10724, 10724, 3, 0, 10, PSC_NOME_ESTRUTURA, 0 },  /* lldiv_t */
    { 10769, 10776, 147, 0, 5, PSC_NOME_FUNCAO, 0 },  /* llrint */
    { 10807, 10815, 148, 0, 5, PSC_NOME_FUNCAO, 0 },  /* llrintf */
    { 10846, 10854, 149, 0, 5, PSC_NOME_FUNCAO, 0 },  /* llrintl */
    { 10891, 10899, 150, 0, 5, PSC_NOME_FUNCAO, 0 },  /* llround */
    { 10931, 10940, 151, 0, 5, PSC_NOME_FUNCAO, 0 },  /* llroundf */
    { 10972, 10981, 152, 0, 5, PSC_NOME_FUNCAO, 0 },  /* llroundl */
    { 11019, 11029, 153, 0, 12, PSC_NOME_FUNCAO, 0 },  /* localtime */
    { 11071, 1552, 19, 0, 12, PSC_NOME_FORA, 0 },  /* localtime_s */
    { 11083, 11087, 154, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log */
    { 11108, 11114, 155, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log10 */
    { 11137, 11144, 156, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log10f */
    { 11166, 11173, 157, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log10l */
    { 11207, 11213, 158, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log1p */
    { 11236, 11243, 159, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log1pf */
    { 11265, 11272, 160, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log1pl */
    { 11306, 11311, 161, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log2 */
    { 11333, 11339, 162, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log2f */
    { 11360, 11366, 163, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log2l */
    { 11399, 11404, 164, 0, 5, PSC_NOME_FUNCAO, 0 },  /* logb */
    { 11426, 11432, 165, 0, 5, PSC_NOME_FUNCAO, 0 },  /* logbf */
    { 11453, 11459, 166, 0, 5, PSC_NOME_FUNCAO, 0 },  /* logbl */
    { 11492, 11497, 167, 0, 5, PSC_NOME_FUNCAO, 0 },  /* logf */
    { 11517, 11522, 168, 0, 5, PSC_NOME_FUNCAO, 0 },  /* logl */
    { 11554, 11560, 169, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lrint */
    { 11585, 11592, 170, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lrintf */
    { 11617, 11624, 171, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lrintl */
    { 11655, 11662, 172, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lround */
    { 11688, 11696, 173, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lroundf */
    { 11722, 11730, 174, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lroundl */
    { 11762, 11769, 175, 0, 10, PSC_NOME_FUNCAO, 0 },  /* malloc */
    { 11795, 11812, 3, 0, 5, PSC_NOME_VALOR, 0 },  /* math_errhandling */
    { 11833, 0, 2, 0, 7, PSC_NOME_OPACO, 0 },  /* max_align_t */
    { 11845, 11851, 176, 0, 10, PSC_NOME_FUNCAO, 0 },  /* mblen */
    { 11886, 11895, 177, 0, 10, PSC_NOME_FUNCAO, 0 },  /* mbstowcs */
    { 11971, 1552, 20, 0, 10, PSC_NOME_FORA, 0 },  /* mbstowcs_s */
    { 11982, 11989, 178, 0, 10, PSC_NOME_FUNCAO, 0 },  /* mbtowc */
    { 12059, 12066, 179, 0, 11, PSC_NOME_FUNCAO, 0 },  /* memchr */
    { 12111, 12118, 180, 0, 11, PSC_NOME_FUNCAO, 0 },  /* memcmp */
    { 12171, 12178, 181, 0, 11, PSC_NOME_FUNCAO, 0 },  /* memcpy */
    { 12247, 1552, 21, 0, 11, PSC_NOME_FORA, 0 },  /* memcpy_s */
    { 12256, 12264, 182, 0, 11, PSC_NOME_FUNCAO, 0 },  /* memmove */
    { 12314, 1552, 22, 0, 11, PSC_NOME_FORA, 0 },  /* memmove_s */
    { 12324, 12331, 183, 0, 11, PSC_NOME_FUNCAO, 0 },  /* memset */
    { 12370, 1552, 23, 0, 11, PSC_NOME_FORA, 0 },  /* memset_s */
    { 12379, 12386, 184, 0, 12, PSC_NOME_FUNCAO, 0 },  /* mktime */
    { 12420, 12425, 185, 0, 5, PSC_NOME_FUNCAO, 0 },  /* modf */
    { 12465, 12471, 186, 0, 5, PSC_NOME_FUNCAO, 0 },  /* modff */
    { 12509, 12515, 187, 0, 5, PSC_NOME_FUNCAO, 0 },  /* modfl */
    { 12571, 12575, 188, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nan */
    { 12604, 12609, 189, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nanf */
    { 12638, 12643, 190, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nanl */
    { 12678, 12688, 191, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nearbyint */
    { 12715, 12726, 192, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nearbyintf */
    { 12752, 12763, 193, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nearbyintl */
    { 12801, 12811, 194, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nextafter */
    { 12848, 12859, 195, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nextafterf */
    { 12894, 12905, 196, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nextafterl */
    { 12958, 12969, 197, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nexttoward */
    { 13012, 13024, 198, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nexttowardf */
    { 13066, 13078, 199, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nexttowardl */
    { 13132, 13141, 0, 0, 7, PSC_NOME_SINTAXE, 0 },  /* offsetof */
    { 13175, 13182, 200, 0, 9, PSC_NOME_FUNCAO, 0 },  /* perror */
    { 13209, 13213, 201, 0, 5, PSC_NOME_FUNCAO, 0 },  /* pow */
    { 13244, 13249, 202, 0, 5, PSC_NOME_FUNCAO, 0 },  /* powf */
    { 13278, 13283, 203, 0, 5, PSC_NOME_FUNCAO, 0 },  /* powl */
    { 13330, 13337, 204, 0, 9, PSC_NOME_FUNCAO, PSC_MARCA_PRINTF },  /* printf */
    { 13383, 1552, 24, 0, 9, PSC_NOME_FORA, 0 },  /* printf_s */
    { 13392, 0, 18, 0, 7, PSC_NOME_TIPO, 0 },  /* ptrdiff_t */
    { 13402, 13407, 205, 0, 9, PSC_NOME_FUNCAO, 0 },  /* putc */
    { 13437, 13445, 206, 0, 9, PSC_NOME_FUNCAO, 0 },  /* putchar */
    { 13464, 13469, 207, 0, 9, PSC_NOME_FUNCAO, 0 },  /* puts */
    { 13493, 13499, 208, 0, 10, PSC_NOME_FUNCAO, 0 },  /* qsort */
    { 13592, 1552, 25, 0, 10, PSC_NOME_FORA, 0 },  /* qsort_s */
    { 13600, 13611, 209, 0, 10, PSC_NOME_FUNCAO, PSC_MARCA_VM },  /* quick_exit */
    { 13649, 13654, 210, 0, 10, PSC_NOME_FUNCAO, 0 },  /* rand */
    { 13669, 13677, 211, 0, 10, PSC_NOME_FUNCAO, 0 },  /* realloc */
    { 13715, 13725, 212, 0, 5, PSC_NOME_FUNCAO, 0 },  /* remainder */
    { 13762, 13773, 213, 0, 5, PSC_NOME_FUNCAO, 0 },  /* remainderf */
    { 13808, 13819, 214, 0, 5, PSC_NOME_FUNCAO, 0 },  /* remainderl */
    { 13872, 13879, 215, 0, 9, PSC_NOME_FUNCAO, 0 },  /* remove */
    { 13912, 13919, 216, 0, 5, PSC_NOME_FUNCAO, 0 },  /* remquo */
    { 13963, 13971, 217, 0, 5, PSC_NOME_FUNCAO, 0 },  /* remquof */
    { 14013, 14021, 218, 0, 5, PSC_NOME_FUNCAO, 0 },  /* remquol */
    { 14081, 14088, 219, 0, 9, PSC_NOME_FUNCAO, 0 },  /* rename */
    { 14133, 14140, 220, 0, 9, PSC_NOME_FUNCAO, 0 },  /* rewind */
    { 14166, 14171, 221, 0, 5, PSC_NOME_FUNCAO, 0 },  /* rint */
    { 14193, 14199, 222, 0, 5, PSC_NOME_FUNCAO, 0 },  /* rintf */
    { 14220, 14226, 223, 0, 5, PSC_NOME_FUNCAO, 0 },  /* rintl */
    { 14259, 14265, 224, 0, 5, PSC_NOME_FUNCAO, 0 },  /* round */
    { 14288, 14295, 225, 0, 5, PSC_NOME_FUNCAO, 0 },  /* roundf */
    { 14317, 14324, 226, 0, 5, PSC_NOME_FUNCAO, 0 },  /* roundl */
    { 14358, 1552, 26, 0, 7, PSC_NOME_FORA, 0 },  /* rsize_t */
    { 14366, 14374, 227, 0, 5, PSC_NOME_FUNCAO, 0 },  /* scalbln */
    { 14411, 14420, 228, 0, 5, PSC_NOME_FUNCAO, 0 },  /* scalblnf */
    { 14456, 14465, 229, 0, 5, PSC_NOME_FUNCAO, 0 },  /* scalblnl */
    { 14513, 14520, 230, 0, 5, PSC_NOME_FUNCAO, 0 },  /* scalbn */
    { 14551, 14559, 231, 0, 5, PSC_NOME_FUNCAO, 0 },  /* scalbnf */
    { 14589, 14597, 232, 0, 5, PSC_NOME_FUNCAO, 0 },  /* scalbnl */
    { 14639, 14645, 233, 14690, 9, PSC_NOME_FUNCAO, PSC_MARCA_SCANF },  /* scanf */
    { 14705, 1552, 27, 0, 9, PSC_NOME_FORA, 0 },  /* scanf_s */
    { 14713, 1552, 28, 0, 10, PSC_NOME_FORA, 0 },  /* set_constraint_handler_s */
    { 14738, 14745, 234, 0, 9, PSC_NOME_FUNCAO, 0 },  /* setbuf */
    { 14802, 14810, 235, 0, 9, PSC_NOME_FUNCAO, 0 },  /* setvbuf */
    { 14890, 14898, 11, 0, 5, PSC_NOME_MACRO, 0 },  /* signbit */
    { 14920, 14924, 236, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sin */
    { 14945, 14950, 237, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sinf */
    { 14970, 14975, 238, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sinh */
    { 14997, 15003, 239, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sinhf */
    { 15024, 15030, 240, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sinhl */
    { 15063, 15068, 241, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sinl */
    { 15100, 0, 19, 0, 7, PSC_NOME_TIPO, 0 },  /* size_t */
    { 15107, 15116, 242, 0, 9, PSC_NOME_FUNCAO, PSC_MARCA_PRINTF },  /* snprintf */
    { 15193, 1552, 29, 0, 9, PSC_NOME_FORA, 0 },  /* snprintf_s */
    { 15204, 15212, 243, 0, 9, PSC_NOME_FUNCAO, PSC_MARCA_PRINTF },  /* sprintf */
    { 15278, 1552, 30, 0, 9, PSC_NOME_FORA, 0 },  /* sprintf_s */
    { 15288, 15293, 244, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sqrt */
    { 15315, 15321, 245, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sqrtf */
    { 15342, 15348, 246, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sqrtl */
    { 15381, 15387, 247, 0, 10, PSC_NOME_FUNCAO, 0 },  /* srand */
    { 15417, 15424, 248, 15495, 9, PSC_NOME_FUNCAO, PSC_MARCA_SCANF },  /* sscanf */
    { 15511, 1552, 31, 0, 9, PSC_NOME_FORA, 0 },  /* sscanf_s */
    { 15520, 15527, 4, 0, 9, PSC_NOME_VALOR, 0 },  /* stderr */
    { 15540, 15546, 5, 0, 9, PSC_NOME_VALOR, 0 },  /* stdin */
    { 15558, 15565, 6, 0, 9, PSC_NOME_VALOR, 0 },  /* stdout */
    { 15578, 15585, 249, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strcat */
    { 15644, 1552, 32, 0, 11, PSC_NOME_FORA, 0 },  /* strcat_s */
    { 15653, 15660, 250, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strchr */
    { 15695, 15702, 251, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strcmp */
    { 15745, 15753, 252, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strcoll */
    { 15797, 15804, 253, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strcpy */
    { 15863, 1552, 33, 0, 11, PSC_NOME_FORA, 0 },  /* strcpy_s */
    { 15872, 15880, 254, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strcspn */
    { 15927, 15936, 255, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strerror */
    { 15963, 1552, 34, 0, 11, PSC_NOME_FORA, 0 },  /* strerror_s */
    { 15974, 1552, 35, 0, 11, PSC_NOME_FORA, 0 },  /* strerrorlen_s */
    { 15988, 15997, 256, 0, 12, PSC_NOME_FUNCAO, 0 },  /* strftime */
    { 16114, 16121, 257, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strlen */
    { 16150, 16158, 258, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strncat */
    { 16228, 1552, 36, 0, 11, PSC_NOME_FORA, 0 },  /* strncat_s */
    { 16238, 16246, 259, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strncmp */
    { 16300, 16308, 260, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strncpy */
    { 16378, 1552, 37, 0, 11, PSC_NOME_FORA, 0 },  /* strncpy_s */
    { 16388, 1552, 38, 0, 11, PSC_NOME_FORA, 0 },  /* strnlen_s */
    { 16398, 16406, 261, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strpbrk */
    { 16452, 16460, 262, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strrchr */
    { 16496, 16503, 263, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strspn */
    { 16549, 16556, 264, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strstr */
    { 16601, 16608, 265, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtod */
    { 16675, 16682, 266, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtof */
    { 16748, 16758, 267, 0, 3, PSC_NOME_FUNCAO, 0 },  /* strtoimax */
    { 16840, 16847, 268, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strtok */
    { 16906, 1552, 39, 0, 11, PSC_NOME_FORA, 0 },  /* strtok_s */
    { 16915, 16922, 269, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtol */
    { 17001, 17009, 270, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtold */
    { 17082, 17090, 271, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtoll */
    { 17175, 17183, 272, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtoul */
    { 17272, 17281, 273, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtoull */
    { 17376, 17386, 274, 0, 3, PSC_NOME_FUNCAO, 0 },  /* strtoumax */
    { 17469, 17469, 4, 0, 12, PSC_NOME_ESTRUTURA, 0 },  /* struct timespec */
    { 17527, 17527, 5, 0, 12, PSC_NOME_ESTRUTURA, 0 },  /* struct tm */
    { 17713, 17721, 275, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strxfrm */
    { 17792, 17799, 276, 0, 10, PSC_NOME_FUNCAO, 0 },  /* system */
    { 17830, 17834, 277, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tan */
    { 17855, 17860, 278, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tanf */
    { 17880, 17885, 279, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tanh */
    { 17907, 17913, 280, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tanhf */
    { 17934, 17940, 281, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tanhl */
    { 17973, 17978, 282, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tanl */
    { 18010, 18017, 283, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tgamma */
    { 18041, 18049, 284, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tgammaf */
    { 18072, 18080, 285, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tgammal */
    { 18115, 18120, 286, 0, 12, PSC_NOME_FUNCAO, 0 },  /* time */
    { 18147, 0, 20, 0, 12, PSC_NOME_TIPO, 0 },  /* time_t */
    { 18154, 18167, 287, 0, 12, PSC_NOME_FUNCAO, 0 },  /* timespec_get */
    { 18215, 18223, 288, 0, 9, PSC_NOME_FUNCAO, 0 },  /* tmpfile */
    { 18243, 1552, 40, 0, 9, PSC_NOME_FORA, 0 },  /* tmpfile_s */
    { 18253, 18260, 289, 0, 9, PSC_NOME_FUNCAO, 0 },  /* tmpnam */
    { 18282, 1552, 41, 0, 9, PSC_NOME_FORA, 0 },  /* tmpnam_s */
    { 18291, 18299, 290, 0, 0, PSC_NOME_FUNCAO, 0 },  /* tolower */
    { 18318, 18326, 291, 0, 0, PSC_NOME_FUNCAO, 0 },  /* toupper */
    { 18345, 18351, 292, 0, 5, PSC_NOME_FUNCAO, 0 },  /* trunc */
    { 18374, 18381, 293, 0, 5, PSC_NOME_FUNCAO, 0 },  /* truncf */
    { 18403, 18410, 294, 0, 5, PSC_NOME_FUNCAO, 0 },  /* truncl */
    { 18444, 0, 21, 0, 8, PSC_NOME_TIPO, 0 },  /* uint16_t */
    { 18453, 0, 22, 0, 8, PSC_NOME_TIPO, 0 },  /* uint32_t */
    { 18462, 0, 23, 0, 8, PSC_NOME_TIPO, 0 },  /* uint64_t */
    { 18471, 0, 24, 0, 8, PSC_NOME_TIPO, 0 },  /* uint8_t */
    { 18479, 0, 25, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_fast16_t */
    { 18493, 0, 26, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_fast32_t */
    { 18507, 0, 27, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_fast64_t */
    { 18521, 0, 28, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_fast8_t */
    { 18534, 0, 29, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_least16_t */
    { 18549, 0, 30, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_least32_t */
    { 18564, 0, 31, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_least64_t */
    { 18579, 0, 32, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_least8_t */
    { 18593, 0, 33, 0, 8, PSC_NOME_TIPO, 0 },  /* uintmax_t */
    { 18603, 0, 34, 0, 8, PSC_NOME_TIPO, 0 },  /* uintptr_t */
    { 18613, 18620, 295, 0, 9, PSC_NOME_FUNCAO, 0 },  /* ungetc */
    { 18652, 18661, 42, 0, 9, PSC_NOME_FORA, 0 },  /* vfprintf */
    { 18721, 1552, 43, 0, 9, PSC_NOME_FORA, 0 },  /* vfprintf_s */
    { 18732, 18661, 44, 0, 9, PSC_NOME_FORA, 0 },  /* vfscanf */
    { 18740, 1552, 45, 0, 9, PSC_NOME_FORA, 0 },  /* vfscanf_s */
    { 18750, 18661, 46, 0, 9, PSC_NOME_FORA, 0 },  /* vprintf */
    { 18758, 1552, 47, 0, 9, PSC_NOME_FORA, 0 },  /* vprintf_s */
    { 18768, 18661, 48, 0, 9, PSC_NOME_FORA, 0 },  /* vscanf */
    { 18775, 1552, 49, 0, 9, PSC_NOME_FORA, 0 },  /* vscanf_s */
    { 18784, 18661, 50, 0, 9, PSC_NOME_FORA, 0 },  /* vsnprintf */
    { 18794, 1552, 51, 0, 9, PSC_NOME_FORA, 0 },  /* vsnprintf_s */
    { 18806, 18661, 52, 0, 9, PSC_NOME_FORA, 0 },  /* vsprintf */
    { 18815, 1552, 53, 0, 9, PSC_NOME_FORA, 0 },  /* vsprintf_s */
    { 18826, 18661, 54, 0, 9, PSC_NOME_FORA, 0 },  /* vsscanf */
    { 18834, 1552, 55, 0, 9, PSC_NOME_FORA, 0 },  /* vsscanf_s */
    { 18844, 0, 35, 0, 7, PSC_NOME_TIPO, 0 },  /* wchar_t */
    { 18852, 18862, 296, 0, 3, PSC_NOME_FUNCAO, 0 },  /* wcstoimax */
    { 18950, 18959, 297, 0, 10, PSC_NOME_FUNCAO, 0 },  /* wcstombs */
    { 19035, 1552, 56, 0, 10, PSC_NOME_FORA, 0 },  /* wcstombs_s */
    { 19046, 19056, 298, 0, 3, PSC_NOME_FUNCAO, 0 },  /* wcstoumax */
    { 19145, 19152, 299, 0, 10, PSC_NOME_FUNCAO, 0 },  /* wctomb */
    { 19187, 1552, 57, 0, 10, PSC_NOME_FORA, 0 },  /* wctomb_s */
};

/* ── os protótipos, conferidos contra os do sistema ───────────────────────
 * Declarar de novo com outro tipo é erro de compilação: o protótipo do .decl
 * é o do cabeçalho do sistema. O nome entre parênteses impede uma macro de
 * cara de função de expandir. Nada aqui é referência: o carregador não
 * resolve nenhum destes nomes na partida.
 * A glibc declara `tmpnam(char[L_tmpnam])`, o mesmo tipo do `char *` do
 * padrão (parâmetro array é ponteiro); o gcc avisa só pela forma. */
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-parameter"
#endif
_Noreturn void (_Exit)(int status);
_Noreturn void (abort)(void);
int (abs)(int j);
double (acos)(double x);
float (acosf)(float x);
double (acosh)(double x);
float (acoshf)(float x);
long double (acoshl)(long double x);
long double (acosl)(long double x);
void *(aligned_alloc)(size_t alignment, size_t size);
char *(asctime)(const struct tm *timeptr);
double (asin)(double x);
float (asinf)(float x);
double (asinh)(double x);
float (asinhf)(float x);
long double (asinhl)(long double x);
long double (asinl)(long double x);
int (at_quick_exit)(void (*func)(void));
double (atan)(double x);
double (atan2)(double y, double x);
float (atan2f)(float y, float x);
long double (atan2l)(long double y, long double x);
float (atanf)(float x);
double (atanh)(double x);
float (atanhf)(float x);
long double (atanhl)(long double x);
long double (atanl)(long double x);
int (atexit)(void (*func)(void));
double (atof)(const char *nptr);
int (atoi)(const char *nptr);
long int (atol)(const char *nptr);
long long int (atoll)(const char *nptr);
void *(bsearch)(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
void *(calloc)(size_t nmemb, size_t size);
double (cbrt)(double x);
float (cbrtf)(float x);
long double (cbrtl)(long double x);
double (ceil)(double x);
float (ceilf)(float x);
long double (ceill)(long double x);
void (clearerr)(FILE *stream);
clock_t (clock)(void);
double (copysign)(double x, double y);
float (copysignf)(float x, float y);
long double (copysignl)(long double x, long double y);
double (cos)(double x);
float (cosf)(float x);
double (cosh)(double x);
float (coshf)(float x);
long double (coshl)(long double x);
long double (cosl)(long double x);
char *(ctime)(const time_t *timer);
double (difftime)(time_t time1, time_t time0);
div_t (div)(int numer, int denom);
double (erf)(double x);
double (erfc)(double x);
float (erfcf)(float x);
long double (erfcl)(long double x);
float (erff)(float x);
long double (erfl)(long double x);
_Noreturn void (exit)(int status);
double (exp)(double x);
double (exp2)(double x);
float (exp2f)(float x);
long double (exp2l)(long double x);
float (expf)(float x);
long double (expl)(long double x);
double (expm1)(double x);
float (expm1f)(float x);
long double (expm1l)(long double x);
double (fabs)(double x);
float (fabsf)(float x);
long double (fabsl)(long double x);
int (fclose)(FILE *stream);
double (fdim)(double x, double y);
float (fdimf)(float x, float y);
long double (fdiml)(long double x, long double y);
int (feof)(FILE *stream);
int (ferror)(FILE *stream);
int (fflush)(FILE *stream);
int (fgetc)(FILE *stream);
int (fgetpos)(FILE * restrict stream, fpos_t * restrict pos);
char *(fgets)(char * restrict s, int n, FILE * restrict stream);
double (floor)(double x);
float (floorf)(float x);
long double (floorl)(long double x);
double (fma)(double x, double y, double z);
float (fmaf)(float x, float y, float z);
long double (fmal)(long double x, long double y, long double z);
double (fmax)(double x, double y);
float (fmaxf)(float x, float y);
long double (fmaxl)(long double x, long double y);
double (fmin)(double x, double y);
float (fminf)(float x, float y);
long double (fminl)(long double x, long double y);
double (fmod)(double x, double y);
float (fmodf)(float x, float y);
long double (fmodl)(long double x, long double y);
FILE *(fopen)(const char * restrict filename, const char * restrict mode);
int (fprintf)(FILE * restrict stream, const char * restrict format, ...);
int (fputc)(int c, FILE *stream);
int (fputs)(const char * restrict s, FILE * restrict stream);
size_t (fread)(void * restrict ptr, size_t size, size_t nmemb, FILE * restrict stream);
void (free)(void *ptr);
FILE *(freopen)(const char * restrict filename, const char * restrict mode, FILE * restrict stream);
double (frexp)(double value, int *exp);
float (frexpf)(float value, int *exp);
long double (frexpl)(long double value, int *exp);
int (fscanf)(FILE * restrict stream, const char * restrict format, ...);
int (fseek)(FILE *stream, long int offset, int whence);
int (fsetpos)(FILE *stream, const fpos_t *pos);
long int (ftell)(FILE *stream);
size_t (fwrite)(const void * restrict ptr, size_t size, size_t nmemb, FILE * restrict stream);
int (getc)(FILE *stream);
int (getchar)(void);
char *(getenv)(const char *name);
struct tm *(gmtime)(const time_t *timer);
double (hypot)(double x, double y);
float (hypotf)(float x, float y);
long double (hypotl)(long double x, long double y);
int (ilogb)(double x);
int (ilogbf)(float x);
int (ilogbl)(long double x);
intmax_t (imaxabs)(intmax_t j);
imaxdiv_t (imaxdiv)(intmax_t numer, intmax_t denom);
int (isalnum)(int c);
int (isalpha)(int c);
int (isblank)(int c);
int (iscntrl)(int c);
int (isdigit)(int c);
int (isgraph)(int c);
int (islower)(int c);
int (isprint)(int c);
int (ispunct)(int c);
int (isspace)(int c);
int (isupper)(int c);
int (isxdigit)(int c);
long int (labs)(long int j);
double (ldexp)(double x, int exp);
float (ldexpf)(float x, int exp);
long double (ldexpl)(long double x, int exp);
ldiv_t (ldiv)(long int numer, long int denom);
double (lgamma)(double x);
float (lgammaf)(float x);
long double (lgammal)(long double x);
long long int (llabs)(long long int j);
lldiv_t (lldiv)(long long int numer, long long int denom);
long long int (llrint)(double x);
long long int (llrintf)(float x);
long long int (llrintl)(long double x);
long long int (llround)(double x);
long long int (llroundf)(float x);
long long int (llroundl)(long double x);
struct tm *(localtime)(const time_t *timer);
double (log)(double x);
double (log10)(double x);
float (log10f)(float x);
long double (log10l)(long double x);
double (log1p)(double x);
float (log1pf)(float x);
long double (log1pl)(long double x);
double (log2)(double x);
float (log2f)(float x);
long double (log2l)(long double x);
double (logb)(double x);
float (logbf)(float x);
long double (logbl)(long double x);
float (logf)(float x);
long double (logl)(long double x);
long int (lrint)(double x);
long int (lrintf)(float x);
long int (lrintl)(long double x);
long int (lround)(double x);
long int (lroundf)(float x);
long int (lroundl)(long double x);
void *(malloc)(size_t size);
int (mblen)(const char *s, size_t n);
size_t (mbstowcs)(wchar_t * restrict pwcs, const char * restrict s, size_t n);
int (mbtowc)(wchar_t * restrict pwc, const char * restrict s, size_t n);
void *(memchr)(const void *s, int c, size_t n);
int (memcmp)(const void *s1, const void *s2, size_t n);
void *(memcpy)(void * restrict s1, const void * restrict s2, size_t n);
void *(memmove)(void *s1, const void *s2, size_t n);
void *(memset)(void *s, int c, size_t n);
time_t (mktime)(struct tm *timeptr);
double (modf)(double value, double *iptr);
float (modff)(float value, float *iptr);
long double (modfl)(long double value, long double *iptr);
double (nan)(const char *tagp);
float (nanf)(const char *tagp);
long double (nanl)(const char *tagp);
double (nearbyint)(double x);
float (nearbyintf)(float x);
long double (nearbyintl)(long double x);
double (nextafter)(double x, double y);
float (nextafterf)(float x, float y);
long double (nextafterl)(long double x, long double y);
double (nexttoward)(double x, long double y);
float (nexttowardf)(float x, long double y);
long double (nexttowardl)(long double x, long double y);
void (perror)(const char *s);
double (pow)(double x, double y);
float (powf)(float x, float y);
long double (powl)(long double x, long double y);
int (printf)(const char * restrict format, ...);
int (putc)(int c, FILE *stream);
int (putchar)(int c);
int (puts)(const char *s);
void (qsort)(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
_Noreturn void (quick_exit)(int status);
int (rand)(void);
void *(realloc)(void *ptr, size_t size);
double (remainder)(double x, double y);
float (remainderf)(float x, float y);
long double (remainderl)(long double x, long double y);
int (remove)(const char *filename);
double (remquo)(double x, double y, int *quo);
float (remquof)(float x, float y, int *quo);
long double (remquol)(long double x, long double y, int *quo);
int (rename)(const char *old, const char *new);
void (rewind)(FILE *stream);
double (rint)(double x);
float (rintf)(float x);
long double (rintl)(long double x);
double (round)(double x);
float (roundf)(float x);
long double (roundl)(long double x);
double (scalbln)(double x, long int n);
float (scalblnf)(float x, long int n);
long double (scalblnl)(long double x, long int n);
double (scalbn)(double x, int n);
float (scalbnf)(float x, int n);
long double (scalbnl)(long double x, int n);
int (scanf)(const char * restrict format, ...);
void (setbuf)(FILE * restrict stream, char * restrict buf);
int (setvbuf)(FILE * restrict stream, char * restrict buf, int mode, size_t size);
double (sin)(double x);
float (sinf)(float x);
double (sinh)(double x);
float (sinhf)(float x);
long double (sinhl)(long double x);
long double (sinl)(long double x);
int (snprintf)(char * restrict s, size_t n, const char * restrict format, ...);
int (sprintf)(char * restrict s, const char * restrict format, ...);
double (sqrt)(double x);
float (sqrtf)(float x);
long double (sqrtl)(long double x);
void (srand)(unsigned int seed);
int (sscanf)(const char * restrict s, const char * restrict format, ...);
char *(strcat)(char * restrict s1, const char * restrict s2);
char *(strchr)(const char *s, int c);
int (strcmp)(const char *s1, const char *s2);
int (strcoll)(const char *s1, const char *s2);
char *(strcpy)(char * restrict s1, const char * restrict s2);
size_t (strcspn)(const char *s1, const char *s2);
char *(strerror)(int errnum);
size_t (strftime)(char * restrict s, size_t maxsize, const char * restrict format, const struct tm * restrict timeptr);
size_t (strlen)(const char *s);
char *(strncat)(char * restrict s1, const char * restrict s2, size_t n);
int (strncmp)(const char *s1, const char *s2, size_t n);
char *(strncpy)(char * restrict s1, const char * restrict s2, size_t n);
char *(strpbrk)(const char *s1, const char *s2);
char *(strrchr)(const char *s, int c);
size_t (strspn)(const char *s1, const char *s2);
char *(strstr)(const char *s1, const char *s2);
double (strtod)(const char * restrict nptr, char ** restrict endptr);
float (strtof)(const char * restrict nptr, char ** restrict endptr);
intmax_t (strtoimax)(const char * restrict nptr, char ** restrict endptr, int base);
char *(strtok)(char * restrict s1, const char * restrict s2);
long int (strtol)(const char * restrict nptr, char ** restrict endptr, int base);
long double (strtold)(const char * restrict nptr, char ** restrict endptr);
long long int (strtoll)(const char * restrict nptr, char ** restrict endptr, int base);
unsigned long int (strtoul)(const char * restrict nptr, char ** restrict endptr, int base);
unsigned long long int (strtoull)(const char * restrict nptr, char ** restrict endptr, int base);
uintmax_t (strtoumax)(const char * restrict nptr, char ** restrict endptr, int base);
size_t (strxfrm)(char * restrict s1, const char * restrict s2, size_t n);
int (system)(const char *string);
double (tan)(double x);
float (tanf)(float x);
double (tanh)(double x);
float (tanhf)(float x);
long double (tanhl)(long double x);
long double (tanl)(long double x);
double (tgamma)(double x);
float (tgammaf)(float x);
long double (tgammal)(long double x);
time_t (time)(time_t *timer);
int (timespec_get)(struct timespec *ts, int base);
FILE *(tmpfile)(void);
char *(tmpnam)(char *s);
int (tolower)(int c);
int (toupper)(int c);
double (trunc)(double x);
float (truncf)(float x);
long double (truncl)(long double x);
int (ungetc)(int c, FILE *stream);
intmax_t (wcstoimax)(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
size_t (wcstombs)(char * restrict s, const wchar_t * restrict pwcs, size_t n);
uintmax_t (wcstoumax)(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
int (wctomb)(char *s, wchar_t wchar);
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

/* ── as funções nossas: macro com cara de função, INTN_C e valor lido na hora ── */
static int ps_c_v_FLT_ROUNDS(void) { return FLT_ROUNDS; }
static int ps_c_m_INT16_C(int value) { return value; }
static int_least32_t ps_c_m_INT32_C(int_least32_t value) { return value; }
static int_least64_t ps_c_m_INT64_C(int_least64_t value) { return value; }
static int ps_c_m_INT8_C(int value) { return value; }
static intmax_t ps_c_m_INTMAX_C(intmax_t value) { return value; }
#ifdef _WIN32
static size_t ps_c_v_MB_CUR_MAX(void) { return MB_CUR_MAX; }
#endif
static int ps_c_m_UINT16_C(int value) { return value; }
static uint_least32_t ps_c_m_UINT32_C(uint_least32_t value) { return value; }
static uint_least64_t ps_c_m_UINT64_C(uint_least64_t value) { return value; }
static int ps_c_m_UINT8_C(int value) { return value; }
static uintmax_t ps_c_m_UINTMAX_C(uintmax_t value) { return value; }
static int ps_c_m_fpclassify(double x) { return isnan(x) ? FP_NAN : isinf(x) ? FP_INFINITE : x == 0 ? FP_ZERO : isnormal(x) ? FP_NORMAL : FP_SUBNORMAL; }
static int ps_c_m_isfinite(double x) { return isfinite(x); }
static int ps_c_m_isgreater(double x, double y) { return isgreater(x, y); }
static int ps_c_m_isgreaterequal(double x, double y) { return isgreaterequal(x, y); }
static int ps_c_m_isinf(double x) { return isinf(x); }
static int ps_c_m_isless(double x, double y) { return isless(x, y); }
static int ps_c_m_islessequal(double x, double y) { return islessequal(x, y); }
static int ps_c_m_islessgreater(double x, double y) { return islessgreater(x, y); }
static int ps_c_m_isnan(double x) { return isnan(x); }
static int ps_c_m_isnormal(double x) { return isnormal(x); }
static int ps_c_m_isunordered(double x, double y) { return isunordered(x, y); }
#ifdef _WIN32
static int ps_c_v_math_errhandling(void) { return math_errhandling; }
#endif
static int ps_c_m_signbit(double x) { return signbit(x); }
static FILE * ps_c_v_stderr(void) { return stderr; }
static FILE * ps_c_v_stdin(void) { return stdin; }
static FILE * ps_c_v_stdout(void) { return stdout; }

/* O endereço de uma função nossa, pelo índice do nome. É um switch, e não uma
 * tabela de ponteiros: tabela de ponteiros vira relocação que o carregador
 * aplica na partida de todo programa. */
static void (*ps_c_local(int i))(void)
{
    switch (i) {
        case 39: return (void (*)(void))ps_c_v_FLT_ROUNDS;
        case 56: return (void (*)(void))ps_c_m_INT16_C;
        case 59: return (void (*)(void))ps_c_m_INT32_C;
        case 62: return (void (*)(void))ps_c_m_INT64_C;
        case 65: return (void (*)(void))ps_c_m_INT8_C;
        case 68: return (void (*)(void))ps_c_m_INTMAX_C;
        case 287: return (void (*)(void))ps_c_m_UINT16_C;
        case 289: return (void (*)(void))ps_c_m_UINT32_C;
        case 291: return (void (*)(void))ps_c_m_UINT64_C;
        case 293: return (void (*)(void))ps_c_m_UINT8_C;
        case 295: return (void (*)(void))ps_c_m_UINTMAX_C;
        case 431: return (void (*)(void))ps_c_m_fpclassify;
        case 487: return (void (*)(void))ps_c_m_isfinite;
        case 489: return (void (*)(void))ps_c_m_isgreater;
        case 490: return (void (*)(void))ps_c_m_isgreaterequal;
        case 491: return (void (*)(void))ps_c_m_isinf;
        case 492: return (void (*)(void))ps_c_m_isless;
        case 493: return (void (*)(void))ps_c_m_islessequal;
        case 494: return (void (*)(void))ps_c_m_islessgreater;
        case 496: return (void (*)(void))ps_c_m_isnan;
        case 497: return (void (*)(void))ps_c_m_isnormal;
        case 501: return (void (*)(void))ps_c_m_isunordered;
        case 619: return (void (*)(void))ps_c_m_signbit;
        case 637: return (void (*)(void))ps_c_v_stderr;
        case 638: return (void (*)(void))ps_c_v_stdin;
        case 639: return (void (*)(void))ps_c_v_stdout;
#ifdef _WIN32
        case 111: return (void (*)(void))ps_c_v_MB_CUR_MAX;
        case 546: return (void (*)(void))ps_c_v_math_errhandling;
#endif
    }
    return NULL;
}

/* ── os valores lidos na hora: o tipo da expressão e, nos `@expansao`, o
 * texto da expansão da macro, guardado pelo compilador (vm/ps_c.c lê dele o
 * nome da função ou da variável e acha pelo nome na primeira leitura) ── */
static const char PS_C_KEXP[] = "\0"
    PS_C_XSTR(MB_CUR_MAX) "\0"
    PS_C_XSTR(math_errhandling) "\0"
    ;
enum {
    PS_C_VE0 = 1,
    PS_C_VE1 = PS_C_VE0 + sizeof(PS_C_XSTR(MB_CUR_MAX)),
};
static const PSCValorPadrao PS_C_VALORES[] = {
    { 0, PS_C_TIPO(FLT_ROUNDS) },  /* FLT_ROUNDS */
    { PS_C_VE0, PS_C_TIPO(MB_CUR_MAX) },  /* MB_CUR_MAX */
    { 0, PS_C_TIPO(errno) },  /* errno */
    { PS_C_VE1, PS_C_TIPO(math_errhandling) },  /* math_errhandling */
    { 0, PS_C_TIPO(stderr) },  /* stderr */
    { 0, PS_C_TIPO(stdin) },  /* stdin */
    { 0, PS_C_TIPO(stdout) },  /* stdout */
};

/* ── os tipos do .decl, conferidos pelo compilador ───────────────────────── */
_Static_assert(_Generic(((div_t *)0)->quot, int: 1, default: 0), "div_t.quot: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((div_t *)0)->rem, int: 1, default: 0), "div_t.rem: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((imaxdiv_t *)0)->quot, intmax_t: 1, default: 0), "imaxdiv_t.quot: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((imaxdiv_t *)0)->rem, intmax_t: 1, default: 0), "imaxdiv_t.rem: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((ldiv_t *)0)->quot, long int: 1, default: 0), "ldiv_t.quot: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((ldiv_t *)0)->rem, long int: 1, default: 0), "ldiv_t.rem: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((lldiv_t *)0)->quot, long long int: 1, default: 0), "lldiv_t.quot: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((lldiv_t *)0)->rem, long long int: 1, default: 0), "lldiv_t.rem: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((struct timespec *)0)->tv_sec, time_t: 1, default: 0), "struct timespec.tv_sec: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((struct timespec *)0)->tv_nsec, long: 1, default: 0), "struct timespec.tv_nsec: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((struct tm *)0)->tm_sec, int: 1, default: 0), "struct tm.tm_sec: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((struct tm *)0)->tm_min, int: 1, default: 0), "struct tm.tm_min: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((struct tm *)0)->tm_hour, int: 1, default: 0), "struct tm.tm_hour: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((struct tm *)0)->tm_mday, int: 1, default: 0), "struct tm.tm_mday: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((struct tm *)0)->tm_mon, int: 1, default: 0), "struct tm.tm_mon: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((struct tm *)0)->tm_year, int: 1, default: 0), "struct tm.tm_year: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((struct tm *)0)->tm_wday, int: 1, default: 0), "struct tm.tm_wday: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((struct tm *)0)->tm_yday, int: 1, default: 0), "struct tm.tm_yday: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(((struct tm *)0)->tm_isdst, int: 1, default: 0), "struct tm.tm_isdst: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic((FLT_ROUNDS), int: 1, default: PS_C_TIPO(FLT_ROUNDS) != PSC_T_NADA && PS_C_TIPO((int)0) != PSC_T_NADA), "FLT_ROUNDS: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(INT16_C(0), int: 1, default: 0), "INT16_C: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(INT32_C(0), int_least32_t: 1, default: 0), "INT32_C: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(INT64_C(0), int_least64_t: 1, default: 0), "INT64_C: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(INT8_C(0), int: 1, default: 0), "INT8_C: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(INTMAX_C(0), intmax_t: 1, default: 0), "INTMAX_C: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic((MB_CUR_MAX), size_t: 1, default: PS_C_TIPO(MB_CUR_MAX) != PSC_T_NADA && PS_C_TIPO((size_t)0) != PSC_T_NADA), "MB_CUR_MAX: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(UINT16_C(0), int: 1, default: 0), "UINT16_C: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(UINT32_C(0), uint_least32_t: 1, default: 0), "UINT32_C: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(UINT64_C(0), uint_least64_t: 1, default: 0), "UINT64_C: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(UINT8_C(0), int: 1, default: 0), "UINT8_C: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic(UINTMAX_C(0), uintmax_t: 1, default: 0), "UINTMAX_C: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic((errno), int: 1, default: PS_C_TIPO(errno) != PSC_T_NADA && PS_C_TIPO((int)0) != PSC_T_NADA), "errno: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic((math_errhandling), int: 1, default: PS_C_TIPO(math_errhandling) != PSC_T_NADA && PS_C_TIPO((int)0) != PSC_T_NADA), "math_errhandling: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic((stderr), FILE *: 1, default: PS_C_TIPO(stderr) != PSC_T_NADA && PS_C_TIPO((FILE *)0) != PSC_T_NADA), "stderr: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic((stdin), FILE *: 1, default: PS_C_TIPO(stdin) != PSC_T_NADA && PS_C_TIPO((FILE *)0) != PSC_T_NADA), "stdin: o tipo de vm/c_padrao.decl");
_Static_assert(_Generic((stdout), FILE *: 1, default: PS_C_TIPO(stdout) != PSC_T_NADA && PS_C_TIPO((FILE *)0) != PSC_T_NADA), "stdout: o tipo de vm/c_padrao.decl");

/* ── constantes: o valor e o tipo vêm do compilador deste sistema ─────────── */
static const PSCConstInt PS_C_KINT[] = {
    { (unsigned long long)(BUFSIZ), PS_C_TIPO(BUFSIZ) },  /* BUFSIZ */
    { (unsigned long long)(CHAR_BIT), PS_C_TIPO(CHAR_BIT) },  /* CHAR_BIT */
    { (unsigned long long)(CHAR_MAX), PS_C_TIPO(CHAR_MAX) },  /* CHAR_MAX */
    { (unsigned long long)(CHAR_MIN), PS_C_TIPO(CHAR_MIN) },  /* CHAR_MIN */
    { (unsigned long long)(CLOCKS_PER_SEC), PS_C_TIPO(CLOCKS_PER_SEC) },  /* CLOCKS_PER_SEC */
    { (unsigned long long)(DBL_DECIMAL_DIG), PS_C_TIPO(DBL_DECIMAL_DIG) },  /* DBL_DECIMAL_DIG */
    { (unsigned long long)(DBL_DIG), PS_C_TIPO(DBL_DIG) },  /* DBL_DIG */
    { (unsigned long long)(DBL_HAS_SUBNORM), PS_C_TIPO(DBL_HAS_SUBNORM) },  /* DBL_HAS_SUBNORM */
    { (unsigned long long)(DBL_MANT_DIG), PS_C_TIPO(DBL_MANT_DIG) },  /* DBL_MANT_DIG */
    { (unsigned long long)(DBL_MAX_10_EXP), PS_C_TIPO(DBL_MAX_10_EXP) },  /* DBL_MAX_10_EXP */
    { (unsigned long long)(DBL_MAX_EXP), PS_C_TIPO(DBL_MAX_EXP) },  /* DBL_MAX_EXP */
    { (unsigned long long)(DBL_MIN_10_EXP), PS_C_TIPO(DBL_MIN_10_EXP) },  /* DBL_MIN_10_EXP */
    { (unsigned long long)(DBL_MIN_EXP), PS_C_TIPO(DBL_MIN_EXP) },  /* DBL_MIN_EXP */
    { (unsigned long long)(DECIMAL_DIG), PS_C_TIPO(DECIMAL_DIG) },  /* DECIMAL_DIG */
    { (unsigned long long)(EDOM), PS_C_TIPO(EDOM) },  /* EDOM */
    { (unsigned long long)(EILSEQ), PS_C_TIPO(EILSEQ) },  /* EILSEQ */
    { (unsigned long long)(EOF), PS_C_TIPO(EOF) },  /* EOF */
    { (unsigned long long)(ERANGE), PS_C_TIPO(ERANGE) },  /* ERANGE */
    { (unsigned long long)(EXIT_FAILURE), PS_C_TIPO(EXIT_FAILURE) },  /* EXIT_FAILURE */
    { (unsigned long long)(EXIT_SUCCESS), PS_C_TIPO(EXIT_SUCCESS) },  /* EXIT_SUCCESS */
    { (unsigned long long)(FILENAME_MAX), PS_C_TIPO(FILENAME_MAX) },  /* FILENAME_MAX */
    { (unsigned long long)(FLT_DECIMAL_DIG), PS_C_TIPO(FLT_DECIMAL_DIG) },  /* FLT_DECIMAL_DIG */
    { (unsigned long long)(FLT_DIG), PS_C_TIPO(FLT_DIG) },  /* FLT_DIG */
    { (unsigned long long)(FLT_EVAL_METHOD), PS_C_TIPO(FLT_EVAL_METHOD) },  /* FLT_EVAL_METHOD */
    { (unsigned long long)(FLT_HAS_SUBNORM), PS_C_TIPO(FLT_HAS_SUBNORM) },  /* FLT_HAS_SUBNORM */
    { (unsigned long long)(FLT_MANT_DIG), PS_C_TIPO(FLT_MANT_DIG) },  /* FLT_MANT_DIG */
    { (unsigned long long)(FLT_MAX_10_EXP), PS_C_TIPO(FLT_MAX_10_EXP) },  /* FLT_MAX_10_EXP */
    { (unsigned long long)(FLT_MAX_EXP), PS_C_TIPO(FLT_MAX_EXP) },  /* FLT_MAX_EXP */
    { (unsigned long long)(FLT_MIN_10_EXP), PS_C_TIPO(FLT_MIN_10_EXP) },  /* FLT_MIN_10_EXP */
    { (unsigned long long)(FLT_MIN_EXP), PS_C_TIPO(FLT_MIN_EXP) },  /* FLT_MIN_EXP */
    { (unsigned long long)(FLT_RADIX), PS_C_TIPO(FLT_RADIX) },  /* FLT_RADIX */
    { (unsigned long long)(FOPEN_MAX), PS_C_TIPO(FOPEN_MAX) },  /* FOPEN_MAX */
    { (unsigned long long)(FP_ILOGB0), PS_C_TIPO(FP_ILOGB0) },  /* FP_ILOGB0 */
    { (unsigned long long)(FP_ILOGBNAN), PS_C_TIPO(FP_ILOGBNAN) },  /* FP_ILOGBNAN */
    { (unsigned long long)(FP_INFINITE), PS_C_TIPO(FP_INFINITE) },  /* FP_INFINITE */
    { (unsigned long long)(FP_NAN), PS_C_TIPO(FP_NAN) },  /* FP_NAN */
    { (unsigned long long)(FP_NORMAL), PS_C_TIPO(FP_NORMAL) },  /* FP_NORMAL */
    { (unsigned long long)(FP_SUBNORMAL), PS_C_TIPO(FP_SUBNORMAL) },  /* FP_SUBNORMAL */
    { (unsigned long long)(FP_ZERO), PS_C_TIPO(FP_ZERO) },  /* FP_ZERO */
    { (unsigned long long)(INT16_MAX), PS_C_TIPO(INT16_MAX) },  /* INT16_MAX */
    { (unsigned long long)(INT16_MIN), PS_C_TIPO(INT16_MIN) },  /* INT16_MIN */
    { (unsigned long long)(INT32_MAX), PS_C_TIPO(INT32_MAX) },  /* INT32_MAX */
    { (unsigned long long)(INT32_MIN), PS_C_TIPO(INT32_MIN) },  /* INT32_MIN */
    { (unsigned long long)(INT64_MAX), PS_C_TIPO(INT64_MAX) },  /* INT64_MAX */
    { (unsigned long long)(INT64_MIN), PS_C_TIPO(INT64_MIN) },  /* INT64_MIN */
    { (unsigned long long)(INT8_MAX), PS_C_TIPO(INT8_MAX) },  /* INT8_MAX */
    { (unsigned long long)(INT8_MIN), PS_C_TIPO(INT8_MIN) },  /* INT8_MIN */
    { (unsigned long long)(INTMAX_MAX), PS_C_TIPO(INTMAX_MAX) },  /* INTMAX_MAX */
    { (unsigned long long)(INTMAX_MIN), PS_C_TIPO(INTMAX_MIN) },  /* INTMAX_MIN */
    { (unsigned long long)(INTPTR_MAX), PS_C_TIPO(INTPTR_MAX) },  /* INTPTR_MAX */
    { (unsigned long long)(INTPTR_MIN), PS_C_TIPO(INTPTR_MIN) },  /* INTPTR_MIN */
    { (unsigned long long)(INT_FAST16_MAX), PS_C_TIPO(INT_FAST16_MAX) },  /* INT_FAST16_MAX */
    { (unsigned long long)(INT_FAST16_MIN), PS_C_TIPO(INT_FAST16_MIN) },  /* INT_FAST16_MIN */
    { (unsigned long long)(INT_FAST32_MAX), PS_C_TIPO(INT_FAST32_MAX) },  /* INT_FAST32_MAX */
    { (unsigned long long)(INT_FAST32_MIN), PS_C_TIPO(INT_FAST32_MIN) },  /* INT_FAST32_MIN */
    { (unsigned long long)(INT_FAST64_MAX), PS_C_TIPO(INT_FAST64_MAX) },  /* INT_FAST64_MAX */
    { (unsigned long long)(INT_FAST64_MIN), PS_C_TIPO(INT_FAST64_MIN) },  /* INT_FAST64_MIN */
    { (unsigned long long)(INT_FAST8_MAX), PS_C_TIPO(INT_FAST8_MAX) },  /* INT_FAST8_MAX */
    { (unsigned long long)(INT_FAST8_MIN), PS_C_TIPO(INT_FAST8_MIN) },  /* INT_FAST8_MIN */
    { (unsigned long long)(INT_LEAST16_MAX), PS_C_TIPO(INT_LEAST16_MAX) },  /* INT_LEAST16_MAX */
    { (unsigned long long)(INT_LEAST16_MIN), PS_C_TIPO(INT_LEAST16_MIN) },  /* INT_LEAST16_MIN */
    { (unsigned long long)(INT_LEAST32_MAX), PS_C_TIPO(INT_LEAST32_MAX) },  /* INT_LEAST32_MAX */
    { (unsigned long long)(INT_LEAST32_MIN), PS_C_TIPO(INT_LEAST32_MIN) },  /* INT_LEAST32_MIN */
    { (unsigned long long)(INT_LEAST64_MAX), PS_C_TIPO(INT_LEAST64_MAX) },  /* INT_LEAST64_MAX */
    { (unsigned long long)(INT_LEAST64_MIN), PS_C_TIPO(INT_LEAST64_MIN) },  /* INT_LEAST64_MIN */
    { (unsigned long long)(INT_LEAST8_MAX), PS_C_TIPO(INT_LEAST8_MAX) },  /* INT_LEAST8_MAX */
    { (unsigned long long)(INT_LEAST8_MIN), PS_C_TIPO(INT_LEAST8_MIN) },  /* INT_LEAST8_MIN */
    { (unsigned long long)(INT_MAX), PS_C_TIPO(INT_MAX) },  /* INT_MAX */
    { (unsigned long long)(INT_MIN), PS_C_TIPO(INT_MIN) },  /* INT_MIN */
    { (unsigned long long)(LDBL_DECIMAL_DIG), PS_C_TIPO(LDBL_DECIMAL_DIG) },  /* LDBL_DECIMAL_DIG */
    { (unsigned long long)(LDBL_DIG), PS_C_TIPO(LDBL_DIG) },  /* LDBL_DIG */
    { (unsigned long long)(LDBL_HAS_SUBNORM), PS_C_TIPO(LDBL_HAS_SUBNORM) },  /* LDBL_HAS_SUBNORM */
    { (unsigned long long)(LDBL_MANT_DIG), PS_C_TIPO(LDBL_MANT_DIG) },  /* LDBL_MANT_DIG */
    { (unsigned long long)(LDBL_MAX_10_EXP), PS_C_TIPO(LDBL_MAX_10_EXP) },  /* LDBL_MAX_10_EXP */
    { (unsigned long long)(LDBL_MAX_EXP), PS_C_TIPO(LDBL_MAX_EXP) },  /* LDBL_MAX_EXP */
    { (unsigned long long)(LDBL_MIN_10_EXP), PS_C_TIPO(LDBL_MIN_10_EXP) },  /* LDBL_MIN_10_EXP */
    { (unsigned long long)(LDBL_MIN_EXP), PS_C_TIPO(LDBL_MIN_EXP) },  /* LDBL_MIN_EXP */
    { (unsigned long long)(LLONG_MAX), PS_C_TIPO(LLONG_MAX) },  /* LLONG_MAX */
    { (unsigned long long)(LLONG_MIN), PS_C_TIPO(LLONG_MIN) },  /* LLONG_MIN */
    { (unsigned long long)(LONG_MAX), PS_C_TIPO(LONG_MAX) },  /* LONG_MAX */
    { (unsigned long long)(LONG_MIN), PS_C_TIPO(LONG_MIN) },  /* LONG_MIN */
    { (unsigned long long)(L_tmpnam), PS_C_TIPO(L_tmpnam) },  /* L_tmpnam */
    { (unsigned long long)(MATH_ERREXCEPT), PS_C_TIPO(MATH_ERREXCEPT) },  /* MATH_ERREXCEPT */
    { (unsigned long long)(MATH_ERRNO), PS_C_TIPO(MATH_ERRNO) },  /* MATH_ERRNO */
    { (unsigned long long)(MB_LEN_MAX), PS_C_TIPO(MB_LEN_MAX) },  /* MB_LEN_MAX */
    { (unsigned long long)(PTRDIFF_MAX), PS_C_TIPO(PTRDIFF_MAX) },  /* PTRDIFF_MAX */
    { (unsigned long long)(PTRDIFF_MIN), PS_C_TIPO(PTRDIFF_MIN) },  /* PTRDIFF_MIN */
    { (unsigned long long)(RAND_MAX), PS_C_TIPO(RAND_MAX) },  /* RAND_MAX */
    { (unsigned long long)(SCHAR_MAX), PS_C_TIPO(SCHAR_MAX) },  /* SCHAR_MAX */
    { (unsigned long long)(SCHAR_MIN), PS_C_TIPO(SCHAR_MIN) },  /* SCHAR_MIN */
    { (unsigned long long)(SEEK_CUR), PS_C_TIPO(SEEK_CUR) },  /* SEEK_CUR */
    { (unsigned long long)(SEEK_END), PS_C_TIPO(SEEK_END) },  /* SEEK_END */
    { (unsigned long long)(SEEK_SET), PS_C_TIPO(SEEK_SET) },  /* SEEK_SET */
    { (unsigned long long)(SHRT_MAX), PS_C_TIPO(SHRT_MAX) },  /* SHRT_MAX */
    { (unsigned long long)(SHRT_MIN), PS_C_TIPO(SHRT_MIN) },  /* SHRT_MIN */
    { (unsigned long long)(SIG_ATOMIC_MAX), PS_C_TIPO(SIG_ATOMIC_MAX) },  /* SIG_ATOMIC_MAX */
    { (unsigned long long)(SIG_ATOMIC_MIN), PS_C_TIPO(SIG_ATOMIC_MIN) },  /* SIG_ATOMIC_MIN */
    { (unsigned long long)(SIZE_MAX), PS_C_TIPO(SIZE_MAX) },  /* SIZE_MAX */
    { (unsigned long long)(TIME_UTC), PS_C_TIPO(TIME_UTC) },  /* TIME_UTC */
    { (unsigned long long)(TMP_MAX), PS_C_TIPO(TMP_MAX) },  /* TMP_MAX */
    { (unsigned long long)(UCHAR_MAX), PS_C_TIPO(UCHAR_MAX) },  /* UCHAR_MAX */
    { (unsigned long long)(UINT16_MAX), PS_C_TIPO(UINT16_MAX) },  /* UINT16_MAX */
    { (unsigned long long)(UINT32_MAX), PS_C_TIPO(UINT32_MAX) },  /* UINT32_MAX */
    { (unsigned long long)(UINT64_MAX), PS_C_TIPO(UINT64_MAX) },  /* UINT64_MAX */
    { (unsigned long long)(UINT8_MAX), PS_C_TIPO(UINT8_MAX) },  /* UINT8_MAX */
    { (unsigned long long)(UINTMAX_MAX), PS_C_TIPO(UINTMAX_MAX) },  /* UINTMAX_MAX */
    { (unsigned long long)(UINTPTR_MAX), PS_C_TIPO(UINTPTR_MAX) },  /* UINTPTR_MAX */
    { (unsigned long long)(UINT_FAST16_MAX), PS_C_TIPO(UINT_FAST16_MAX) },  /* UINT_FAST16_MAX */
    { (unsigned long long)(UINT_FAST32_MAX), PS_C_TIPO(UINT_FAST32_MAX) },  /* UINT_FAST32_MAX */
    { (unsigned long long)(UINT_FAST64_MAX), PS_C_TIPO(UINT_FAST64_MAX) },  /* UINT_FAST64_MAX */
    { (unsigned long long)(UINT_FAST8_MAX), PS_C_TIPO(UINT_FAST8_MAX) },  /* UINT_FAST8_MAX */
    { (unsigned long long)(UINT_LEAST16_MAX), PS_C_TIPO(UINT_LEAST16_MAX) },  /* UINT_LEAST16_MAX */
    { (unsigned long long)(UINT_LEAST32_MAX), PS_C_TIPO(UINT_LEAST32_MAX) },  /* UINT_LEAST32_MAX */
    { (unsigned long long)(UINT_LEAST64_MAX), PS_C_TIPO(UINT_LEAST64_MAX) },  /* UINT_LEAST64_MAX */
    { (unsigned long long)(UINT_LEAST8_MAX), PS_C_TIPO(UINT_LEAST8_MAX) },  /* UINT_LEAST8_MAX */
    { (unsigned long long)(UINT_MAX), PS_C_TIPO(UINT_MAX) },  /* UINT_MAX */
    { (unsigned long long)(ULLONG_MAX), PS_C_TIPO(ULLONG_MAX) },  /* ULLONG_MAX */
    { (unsigned long long)(ULONG_MAX), PS_C_TIPO(ULONG_MAX) },  /* ULONG_MAX */
    { (unsigned long long)(USHRT_MAX), PS_C_TIPO(USHRT_MAX) },  /* USHRT_MAX */
    { (unsigned long long)(WCHAR_MAX), PS_C_TIPO(WCHAR_MAX) },  /* WCHAR_MAX */
    { (unsigned long long)(WCHAR_MIN), PS_C_TIPO(WCHAR_MIN) },  /* WCHAR_MIN */
    { (unsigned long long)(WINT_MAX), PS_C_TIPO(WINT_MAX) },  /* WINT_MAX */
    { (unsigned long long)(WINT_MIN), PS_C_TIPO(WINT_MIN) },  /* WINT_MIN */
    { (unsigned long long)(_IOFBF), PS_C_TIPO(_IOFBF) },  /* _IOFBF */
    { (unsigned long long)(_IOLBF), PS_C_TIPO(_IOLBF) },  /* _IOLBF */
    { (unsigned long long)(_IONBF), PS_C_TIPO(_IONBF) },  /* _IONBF */
    { (unsigned long long)(__bool_true_false_are_defined), PS_C_TIPO(__bool_true_false_are_defined) },  /* __bool_true_false_are_defined */
};
static const PSCConstFlo PS_C_KFLO[] = {
    { (long double)(DBL_EPSILON), PS_C_TIPO(DBL_EPSILON) },  /* DBL_EPSILON */
    { (long double)(DBL_MAX), PS_C_TIPO(DBL_MAX) },  /* DBL_MAX */
    { (long double)(DBL_MIN), PS_C_TIPO(DBL_MIN) },  /* DBL_MIN */
    { (long double)(DBL_TRUE_MIN), PS_C_TIPO(DBL_TRUE_MIN) },  /* DBL_TRUE_MIN */
    { (long double)(FLT_EPSILON), PS_C_TIPO(FLT_EPSILON) },  /* FLT_EPSILON */
    { (long double)(FLT_MAX), PS_C_TIPO(FLT_MAX) },  /* FLT_MAX */
    { (long double)(FLT_MIN), PS_C_TIPO(FLT_MIN) },  /* FLT_MIN */
    { (long double)(FLT_TRUE_MIN), PS_C_TIPO(FLT_TRUE_MIN) },  /* FLT_TRUE_MIN */
    { (long double)(HUGE_VAL), PS_C_TIPO(HUGE_VAL) },  /* HUGE_VAL */
    { (long double)(HUGE_VALF), PS_C_TIPO(HUGE_VALF) },  /* HUGE_VALF */
    { (long double)(HUGE_VALL), PS_C_TIPO(HUGE_VALL) },  /* HUGE_VALL */
    { (long double)(INFINITY), PS_C_TIPO(INFINITY) },  /* INFINITY */
    { (long double)(LDBL_EPSILON), PS_C_TIPO(LDBL_EPSILON) },  /* LDBL_EPSILON */
    { (long double)(LDBL_MAX), PS_C_TIPO(LDBL_MAX) },  /* LDBL_MAX */
    { (long double)(LDBL_MIN), PS_C_TIPO(LDBL_MIN) },  /* LDBL_MIN */
    { (long double)(LDBL_TRUE_MIN), PS_C_TIPO(LDBL_TRUE_MIN) },  /* LDBL_TRUE_MIN */
    { (long double)(NAN), PS_C_TIPO(NAN) },  /* NAN */
};
/* a macro que o sistema define ou não: 1 se definiu */
static const unsigned char PS_C_KTALVEZ[] = {
#ifdef FP_FAST_FMA
    1,  /* FP_FAST_FMA */
#else
    0,  /* FP_FAST_FMA */
#endif
#ifdef FP_FAST_FMAF
    1,  /* FP_FAST_FMAF */
#else
    0,  /* FP_FAST_FMAF */
#endif
#ifdef FP_FAST_FMAL
    1,  /* FP_FAST_FMAL */
#else
    0,  /* FP_FAST_FMAL */
#endif
};
/* as de texto num bloco só; a posição de cada uma é medida pelo compilador */
static const char PS_C_KTEXTO[] =
    PRIX16 "\0"
    PRIX32 "\0"
    PRIX64 "\0"
    PRIX8 "\0"
    PRIXFAST16 "\0"
    PRIXFAST32 "\0"
    PRIXFAST64 "\0"
    PRIXFAST8 "\0"
    PRIXLEAST16 "\0"
    PRIXLEAST32 "\0"
    PRIXLEAST64 "\0"
    PRIXLEAST8 "\0"
    PRIXMAX "\0"
    PRIXPTR "\0"
    PRId16 "\0"
    PRId32 "\0"
    PRId64 "\0"
    PRId8 "\0"
    PRIdFAST16 "\0"
    PRIdFAST32 "\0"
    PRIdFAST64 "\0"
    PRIdFAST8 "\0"
    PRIdLEAST16 "\0"
    PRIdLEAST32 "\0"
    PRIdLEAST64 "\0"
    PRIdLEAST8 "\0"
    PRIdMAX "\0"
    PRIdPTR "\0"
    PRIi16 "\0"
    PRIi32 "\0"
    PRIi64 "\0"
    PRIi8 "\0"
    PRIiFAST16 "\0"
    PRIiFAST32 "\0"
    PRIiFAST64 "\0"
    PRIiFAST8 "\0"
    PRIiLEAST16 "\0"
    PRIiLEAST32 "\0"
    PRIiLEAST64 "\0"
    PRIiLEAST8 "\0"
    PRIiMAX "\0"
    PRIiPTR "\0"
    PRIo16 "\0"
    PRIo32 "\0"
    PRIo64 "\0"
    PRIo8 "\0"
    PRIoFAST16 "\0"
    PRIoFAST32 "\0"
    PRIoFAST64 "\0"
    PRIoFAST8 "\0"
    PRIoLEAST16 "\0"
    PRIoLEAST32 "\0"
    PRIoLEAST64 "\0"
    PRIoLEAST8 "\0"
    PRIoMAX "\0"
    PRIoPTR "\0"
    PRIu16 "\0"
    PRIu32 "\0"
    PRIu64 "\0"
    PRIu8 "\0"
    PRIuFAST16 "\0"
    PRIuFAST32 "\0"
    PRIuFAST64 "\0"
    PRIuFAST8 "\0"
    PRIuLEAST16 "\0"
    PRIuLEAST32 "\0"
    PRIuLEAST64 "\0"
    PRIuLEAST8 "\0"
    PRIuMAX "\0"
    PRIuPTR "\0"
    PRIx16 "\0"
    PRIx32 "\0"
    PRIx64 "\0"
    PRIx8 "\0"
    PRIxFAST16 "\0"
    PRIxFAST32 "\0"
    PRIxFAST64 "\0"
    PRIxFAST8 "\0"
    PRIxLEAST16 "\0"
    PRIxLEAST32 "\0"
    PRIxLEAST64 "\0"
    PRIxLEAST8 "\0"
    PRIxMAX "\0"
    PRIxPTR "\0"
    SCNd16 "\0"
    SCNd32 "\0"
    SCNd64 "\0"
    SCNd8 "\0"
    SCNdFAST16 "\0"
    SCNdFAST32 "\0"
    SCNdFAST64 "\0"
    SCNdFAST8 "\0"
    SCNdLEAST16 "\0"
    SCNdLEAST32 "\0"
    SCNdLEAST64 "\0"
    SCNdLEAST8 "\0"
    SCNdMAX "\0"
    SCNdPTR "\0"
    SCNi16 "\0"
    SCNi32 "\0"
    SCNi64 "\0"
    SCNi8 "\0"
    SCNiFAST16 "\0"
    SCNiFAST32 "\0"
    SCNiFAST64 "\0"
    SCNiFAST8 "\0"
    SCNiLEAST16 "\0"
    SCNiLEAST32 "\0"
    SCNiLEAST64 "\0"
    SCNiLEAST8 "\0"
    SCNiMAX "\0"
    SCNiPTR "\0"
    SCNo16 "\0"
    SCNo32 "\0"
    SCNo64 "\0"
    SCNo8 "\0"
    SCNoFAST16 "\0"
    SCNoFAST32 "\0"
    SCNoFAST64 "\0"
    SCNoFAST8 "\0"
    SCNoLEAST16 "\0"
    SCNoLEAST32 "\0"
    SCNoLEAST64 "\0"
    SCNoLEAST8 "\0"
    SCNoMAX "\0"
    SCNoPTR "\0"
    SCNu16 "\0"
    SCNu32 "\0"
    SCNu64 "\0"
    SCNu8 "\0"
    SCNuFAST16 "\0"
    SCNuFAST32 "\0"
    SCNuFAST64 "\0"
    SCNuFAST8 "\0"
    SCNuLEAST16 "\0"
    SCNuLEAST32 "\0"
    SCNuLEAST64 "\0"
    SCNuLEAST8 "\0"
    SCNuMAX "\0"
    SCNuPTR "\0"
    SCNx16 "\0"
    SCNx32 "\0"
    SCNx64 "\0"
    SCNx8 "\0"
    SCNxFAST16 "\0"
    SCNxFAST32 "\0"
    SCNxFAST64 "\0"
    SCNxFAST8 "\0"
    SCNxLEAST16 "\0"
    SCNxLEAST32 "\0"
    SCNxLEAST64 "\0"
    SCNxLEAST8 "\0"
    SCNxMAX "\0"
    SCNxPTR "\0"
    ;
enum {
    PS_C_KT0 = 0,
    PS_C_KT1 = PS_C_KT0 + sizeof(PRIX16),
    PS_C_KT2 = PS_C_KT1 + sizeof(PRIX32),
    PS_C_KT3 = PS_C_KT2 + sizeof(PRIX64),
    PS_C_KT4 = PS_C_KT3 + sizeof(PRIX8),
    PS_C_KT5 = PS_C_KT4 + sizeof(PRIXFAST16),
    PS_C_KT6 = PS_C_KT5 + sizeof(PRIXFAST32),
    PS_C_KT7 = PS_C_KT6 + sizeof(PRIXFAST64),
    PS_C_KT8 = PS_C_KT7 + sizeof(PRIXFAST8),
    PS_C_KT9 = PS_C_KT8 + sizeof(PRIXLEAST16),
    PS_C_KT10 = PS_C_KT9 + sizeof(PRIXLEAST32),
    PS_C_KT11 = PS_C_KT10 + sizeof(PRIXLEAST64),
    PS_C_KT12 = PS_C_KT11 + sizeof(PRIXLEAST8),
    PS_C_KT13 = PS_C_KT12 + sizeof(PRIXMAX),
    PS_C_KT14 = PS_C_KT13 + sizeof(PRIXPTR),
    PS_C_KT15 = PS_C_KT14 + sizeof(PRId16),
    PS_C_KT16 = PS_C_KT15 + sizeof(PRId32),
    PS_C_KT17 = PS_C_KT16 + sizeof(PRId64),
    PS_C_KT18 = PS_C_KT17 + sizeof(PRId8),
    PS_C_KT19 = PS_C_KT18 + sizeof(PRIdFAST16),
    PS_C_KT20 = PS_C_KT19 + sizeof(PRIdFAST32),
    PS_C_KT21 = PS_C_KT20 + sizeof(PRIdFAST64),
    PS_C_KT22 = PS_C_KT21 + sizeof(PRIdFAST8),
    PS_C_KT23 = PS_C_KT22 + sizeof(PRIdLEAST16),
    PS_C_KT24 = PS_C_KT23 + sizeof(PRIdLEAST32),
    PS_C_KT25 = PS_C_KT24 + sizeof(PRIdLEAST64),
    PS_C_KT26 = PS_C_KT25 + sizeof(PRIdLEAST8),
    PS_C_KT27 = PS_C_KT26 + sizeof(PRIdMAX),
    PS_C_KT28 = PS_C_KT27 + sizeof(PRIdPTR),
    PS_C_KT29 = PS_C_KT28 + sizeof(PRIi16),
    PS_C_KT30 = PS_C_KT29 + sizeof(PRIi32),
    PS_C_KT31 = PS_C_KT30 + sizeof(PRIi64),
    PS_C_KT32 = PS_C_KT31 + sizeof(PRIi8),
    PS_C_KT33 = PS_C_KT32 + sizeof(PRIiFAST16),
    PS_C_KT34 = PS_C_KT33 + sizeof(PRIiFAST32),
    PS_C_KT35 = PS_C_KT34 + sizeof(PRIiFAST64),
    PS_C_KT36 = PS_C_KT35 + sizeof(PRIiFAST8),
    PS_C_KT37 = PS_C_KT36 + sizeof(PRIiLEAST16),
    PS_C_KT38 = PS_C_KT37 + sizeof(PRIiLEAST32),
    PS_C_KT39 = PS_C_KT38 + sizeof(PRIiLEAST64),
    PS_C_KT40 = PS_C_KT39 + sizeof(PRIiLEAST8),
    PS_C_KT41 = PS_C_KT40 + sizeof(PRIiMAX),
    PS_C_KT42 = PS_C_KT41 + sizeof(PRIiPTR),
    PS_C_KT43 = PS_C_KT42 + sizeof(PRIo16),
    PS_C_KT44 = PS_C_KT43 + sizeof(PRIo32),
    PS_C_KT45 = PS_C_KT44 + sizeof(PRIo64),
    PS_C_KT46 = PS_C_KT45 + sizeof(PRIo8),
    PS_C_KT47 = PS_C_KT46 + sizeof(PRIoFAST16),
    PS_C_KT48 = PS_C_KT47 + sizeof(PRIoFAST32),
    PS_C_KT49 = PS_C_KT48 + sizeof(PRIoFAST64),
    PS_C_KT50 = PS_C_KT49 + sizeof(PRIoFAST8),
    PS_C_KT51 = PS_C_KT50 + sizeof(PRIoLEAST16),
    PS_C_KT52 = PS_C_KT51 + sizeof(PRIoLEAST32),
    PS_C_KT53 = PS_C_KT52 + sizeof(PRIoLEAST64),
    PS_C_KT54 = PS_C_KT53 + sizeof(PRIoLEAST8),
    PS_C_KT55 = PS_C_KT54 + sizeof(PRIoMAX),
    PS_C_KT56 = PS_C_KT55 + sizeof(PRIoPTR),
    PS_C_KT57 = PS_C_KT56 + sizeof(PRIu16),
    PS_C_KT58 = PS_C_KT57 + sizeof(PRIu32),
    PS_C_KT59 = PS_C_KT58 + sizeof(PRIu64),
    PS_C_KT60 = PS_C_KT59 + sizeof(PRIu8),
    PS_C_KT61 = PS_C_KT60 + sizeof(PRIuFAST16),
    PS_C_KT62 = PS_C_KT61 + sizeof(PRIuFAST32),
    PS_C_KT63 = PS_C_KT62 + sizeof(PRIuFAST64),
    PS_C_KT64 = PS_C_KT63 + sizeof(PRIuFAST8),
    PS_C_KT65 = PS_C_KT64 + sizeof(PRIuLEAST16),
    PS_C_KT66 = PS_C_KT65 + sizeof(PRIuLEAST32),
    PS_C_KT67 = PS_C_KT66 + sizeof(PRIuLEAST64),
    PS_C_KT68 = PS_C_KT67 + sizeof(PRIuLEAST8),
    PS_C_KT69 = PS_C_KT68 + sizeof(PRIuMAX),
    PS_C_KT70 = PS_C_KT69 + sizeof(PRIuPTR),
    PS_C_KT71 = PS_C_KT70 + sizeof(PRIx16),
    PS_C_KT72 = PS_C_KT71 + sizeof(PRIx32),
    PS_C_KT73 = PS_C_KT72 + sizeof(PRIx64),
    PS_C_KT74 = PS_C_KT73 + sizeof(PRIx8),
    PS_C_KT75 = PS_C_KT74 + sizeof(PRIxFAST16),
    PS_C_KT76 = PS_C_KT75 + sizeof(PRIxFAST32),
    PS_C_KT77 = PS_C_KT76 + sizeof(PRIxFAST64),
    PS_C_KT78 = PS_C_KT77 + sizeof(PRIxFAST8),
    PS_C_KT79 = PS_C_KT78 + sizeof(PRIxLEAST16),
    PS_C_KT80 = PS_C_KT79 + sizeof(PRIxLEAST32),
    PS_C_KT81 = PS_C_KT80 + sizeof(PRIxLEAST64),
    PS_C_KT82 = PS_C_KT81 + sizeof(PRIxLEAST8),
    PS_C_KT83 = PS_C_KT82 + sizeof(PRIxMAX),
    PS_C_KT84 = PS_C_KT83 + sizeof(PRIxPTR),
    PS_C_KT85 = PS_C_KT84 + sizeof(SCNd16),
    PS_C_KT86 = PS_C_KT85 + sizeof(SCNd32),
    PS_C_KT87 = PS_C_KT86 + sizeof(SCNd64),
    PS_C_KT88 = PS_C_KT87 + sizeof(SCNd8),
    PS_C_KT89 = PS_C_KT88 + sizeof(SCNdFAST16),
    PS_C_KT90 = PS_C_KT89 + sizeof(SCNdFAST32),
    PS_C_KT91 = PS_C_KT90 + sizeof(SCNdFAST64),
    PS_C_KT92 = PS_C_KT91 + sizeof(SCNdFAST8),
    PS_C_KT93 = PS_C_KT92 + sizeof(SCNdLEAST16),
    PS_C_KT94 = PS_C_KT93 + sizeof(SCNdLEAST32),
    PS_C_KT95 = PS_C_KT94 + sizeof(SCNdLEAST64),
    PS_C_KT96 = PS_C_KT95 + sizeof(SCNdLEAST8),
    PS_C_KT97 = PS_C_KT96 + sizeof(SCNdMAX),
    PS_C_KT98 = PS_C_KT97 + sizeof(SCNdPTR),
    PS_C_KT99 = PS_C_KT98 + sizeof(SCNi16),
    PS_C_KT100 = PS_C_KT99 + sizeof(SCNi32),
    PS_C_KT101 = PS_C_KT100 + sizeof(SCNi64),
    PS_C_KT102 = PS_C_KT101 + sizeof(SCNi8),
    PS_C_KT103 = PS_C_KT102 + sizeof(SCNiFAST16),
    PS_C_KT104 = PS_C_KT103 + sizeof(SCNiFAST32),
    PS_C_KT105 = PS_C_KT104 + sizeof(SCNiFAST64),
    PS_C_KT106 = PS_C_KT105 + sizeof(SCNiFAST8),
    PS_C_KT107 = PS_C_KT106 + sizeof(SCNiLEAST16),
    PS_C_KT108 = PS_C_KT107 + sizeof(SCNiLEAST32),
    PS_C_KT109 = PS_C_KT108 + sizeof(SCNiLEAST64),
    PS_C_KT110 = PS_C_KT109 + sizeof(SCNiLEAST8),
    PS_C_KT111 = PS_C_KT110 + sizeof(SCNiMAX),
    PS_C_KT112 = PS_C_KT111 + sizeof(SCNiPTR),
    PS_C_KT113 = PS_C_KT112 + sizeof(SCNo16),
    PS_C_KT114 = PS_C_KT113 + sizeof(SCNo32),
    PS_C_KT115 = PS_C_KT114 + sizeof(SCNo64),
    PS_C_KT116 = PS_C_KT115 + sizeof(SCNo8),
    PS_C_KT117 = PS_C_KT116 + sizeof(SCNoFAST16),
    PS_C_KT118 = PS_C_KT117 + sizeof(SCNoFAST32),
    PS_C_KT119 = PS_C_KT118 + sizeof(SCNoFAST64),
    PS_C_KT120 = PS_C_KT119 + sizeof(SCNoFAST8),
    PS_C_KT121 = PS_C_KT120 + sizeof(SCNoLEAST16),
    PS_C_KT122 = PS_C_KT121 + sizeof(SCNoLEAST32),
    PS_C_KT123 = PS_C_KT122 + sizeof(SCNoLEAST64),
    PS_C_KT124 = PS_C_KT123 + sizeof(SCNoLEAST8),
    PS_C_KT125 = PS_C_KT124 + sizeof(SCNoMAX),
    PS_C_KT126 = PS_C_KT125 + sizeof(SCNoPTR),
    PS_C_KT127 = PS_C_KT126 + sizeof(SCNu16),
    PS_C_KT128 = PS_C_KT127 + sizeof(SCNu32),
    PS_C_KT129 = PS_C_KT128 + sizeof(SCNu64),
    PS_C_KT130 = PS_C_KT129 + sizeof(SCNu8),
    PS_C_KT131 = PS_C_KT130 + sizeof(SCNuFAST16),
    PS_C_KT132 = PS_C_KT131 + sizeof(SCNuFAST32),
    PS_C_KT133 = PS_C_KT132 + sizeof(SCNuFAST64),
    PS_C_KT134 = PS_C_KT133 + sizeof(SCNuFAST8),
    PS_C_KT135 = PS_C_KT134 + sizeof(SCNuLEAST16),
    PS_C_KT136 = PS_C_KT135 + sizeof(SCNuLEAST32),
    PS_C_KT137 = PS_C_KT136 + sizeof(SCNuLEAST64),
    PS_C_KT138 = PS_C_KT137 + sizeof(SCNuLEAST8),
    PS_C_KT139 = PS_C_KT138 + sizeof(SCNuMAX),
    PS_C_KT140 = PS_C_KT139 + sizeof(SCNuPTR),
    PS_C_KT141 = PS_C_KT140 + sizeof(SCNx16),
    PS_C_KT142 = PS_C_KT141 + sizeof(SCNx32),
    PS_C_KT143 = PS_C_KT142 + sizeof(SCNx64),
    PS_C_KT144 = PS_C_KT143 + sizeof(SCNx8),
    PS_C_KT145 = PS_C_KT144 + sizeof(SCNxFAST16),
    PS_C_KT146 = PS_C_KT145 + sizeof(SCNxFAST32),
    PS_C_KT147 = PS_C_KT146 + sizeof(SCNxFAST64),
    PS_C_KT148 = PS_C_KT147 + sizeof(SCNxFAST8),
    PS_C_KT149 = PS_C_KT148 + sizeof(SCNxLEAST16),
    PS_C_KT150 = PS_C_KT149 + sizeof(SCNxLEAST32),
    PS_C_KT151 = PS_C_KT150 + sizeof(SCNxLEAST64),
    PS_C_KT152 = PS_C_KT151 + sizeof(SCNxLEAST8),
    PS_C_KT153 = PS_C_KT152 + sizeof(SCNxMAX),
};
static const uint16_t PS_C_KTEXTO_POS[] = { PS_C_KT0, PS_C_KT1, PS_C_KT2, PS_C_KT3, PS_C_KT4, PS_C_KT5, PS_C_KT6, PS_C_KT7, PS_C_KT8, PS_C_KT9, PS_C_KT10, PS_C_KT11, PS_C_KT12, PS_C_KT13, PS_C_KT14, PS_C_KT15, PS_C_KT16, PS_C_KT17, PS_C_KT18, PS_C_KT19, PS_C_KT20, PS_C_KT21, PS_C_KT22, PS_C_KT23, PS_C_KT24, PS_C_KT25, PS_C_KT26, PS_C_KT27, PS_C_KT28, PS_C_KT29, PS_C_KT30, PS_C_KT31, PS_C_KT32, PS_C_KT33, PS_C_KT34, PS_C_KT35, PS_C_KT36, PS_C_KT37, PS_C_KT38, PS_C_KT39, PS_C_KT40, PS_C_KT41, PS_C_KT42, PS_C_KT43, PS_C_KT44, PS_C_KT45, PS_C_KT46, PS_C_KT47, PS_C_KT48, PS_C_KT49, PS_C_KT50, PS_C_KT51, PS_C_KT52, PS_C_KT53, PS_C_KT54, PS_C_KT55, PS_C_KT56, PS_C_KT57, PS_C_KT58, PS_C_KT59, PS_C_KT60, PS_C_KT61, PS_C_KT62, PS_C_KT63, PS_C_KT64, PS_C_KT65, PS_C_KT66, PS_C_KT67, PS_C_KT68, PS_C_KT69, PS_C_KT70, PS_C_KT71, PS_C_KT72, PS_C_KT73, PS_C_KT74, PS_C_KT75, PS_C_KT76, PS_C_KT77, PS_C_KT78, PS_C_KT79, PS_C_KT80, PS_C_KT81, PS_C_KT82, PS_C_KT83, PS_C_KT84, PS_C_KT85, PS_C_KT86, PS_C_KT87, PS_C_KT88, PS_C_KT89, PS_C_KT90, PS_C_KT91, PS_C_KT92, PS_C_KT93, PS_C_KT94, PS_C_KT95, PS_C_KT96, PS_C_KT97, PS_C_KT98, PS_C_KT99, PS_C_KT100, PS_C_KT101, PS_C_KT102, PS_C_KT103, PS_C_KT104, PS_C_KT105, PS_C_KT106, PS_C_KT107, PS_C_KT108, PS_C_KT109, PS_C_KT110, PS_C_KT111, PS_C_KT112, PS_C_KT113, PS_C_KT114, PS_C_KT115, PS_C_KT116, PS_C_KT117, PS_C_KT118, PS_C_KT119, PS_C_KT120, PS_C_KT121, PS_C_KT122, PS_C_KT123, PS_C_KT124, PS_C_KT125, PS_C_KT126, PS_C_KT127, PS_C_KT128, PS_C_KT129, PS_C_KT130, PS_C_KT131, PS_C_KT132, PS_C_KT133, PS_C_KT134, PS_C_KT135, PS_C_KT136, PS_C_KT137, PS_C_KT138, PS_C_KT139, PS_C_KT140, PS_C_KT141, PS_C_KT142, PS_C_KT143, PS_C_KT144, PS_C_KT145, PS_C_KT146, PS_C_KT147, PS_C_KT148, PS_C_KT149, PS_C_KT150, PS_C_KT151, PS_C_KT152, PS_C_KT153 };

/* ── tipos: o escalar que cada typedef é neste sistema ───────────────────── */
static const unsigned char PS_C_TIPOS[] = {
    PS_C_TIPO((bool)0),  /* bool */
    PS_C_TIPO((clock_t)0),  /* clock_t */
    PS_C_TIPO((double_t)0),  /* double_t */
    PS_C_TIPO((float_t)0),  /* float_t */
    PS_C_TIPO((int16_t)0),  /* int16_t */
    PS_C_TIPO((int32_t)0),  /* int32_t */
    PS_C_TIPO((int64_t)0),  /* int64_t */
    PS_C_TIPO((int8_t)0),  /* int8_t */
    PS_C_TIPO((int_fast16_t)0),  /* int_fast16_t */
    PS_C_TIPO((int_fast32_t)0),  /* int_fast32_t */
    PS_C_TIPO((int_fast64_t)0),  /* int_fast64_t */
    PS_C_TIPO((int_fast8_t)0),  /* int_fast8_t */
    PS_C_TIPO((int_least16_t)0),  /* int_least16_t */
    PS_C_TIPO((int_least32_t)0),  /* int_least32_t */
    PS_C_TIPO((int_least64_t)0),  /* int_least64_t */
    PS_C_TIPO((int_least8_t)0),  /* int_least8_t */
    PS_C_TIPO((intmax_t)0),  /* intmax_t */
    PS_C_TIPO((intptr_t)0),  /* intptr_t */
    PS_C_TIPO((ptrdiff_t)0),  /* ptrdiff_t */
    PS_C_TIPO((size_t)0),  /* size_t */
    PS_C_TIPO((time_t)0),  /* time_t */
    PS_C_TIPO((uint16_t)0),  /* uint16_t */
    PS_C_TIPO((uint32_t)0),  /* uint32_t */
    PS_C_TIPO((uint64_t)0),  /* uint64_t */
    PS_C_TIPO((uint8_t)0),  /* uint8_t */
    PS_C_TIPO((uint_fast16_t)0),  /* uint_fast16_t */
    PS_C_TIPO((uint_fast32_t)0),  /* uint_fast32_t */
    PS_C_TIPO((uint_fast64_t)0),  /* uint_fast64_t */
    PS_C_TIPO((uint_fast8_t)0),  /* uint_fast8_t */
    PS_C_TIPO((uint_least16_t)0),  /* uint_least16_t */
    PS_C_TIPO((uint_least32_t)0),  /* uint_least32_t */
    PS_C_TIPO((uint_least64_t)0),  /* uint_least64_t */
    PS_C_TIPO((uint_least8_t)0),  /* uint_least8_t */
    PS_C_TIPO((uintmax_t)0),  /* uintmax_t */
    PS_C_TIPO((uintptr_t)0),  /* uintptr_t */
    PS_C_TIPO((wchar_t)0),  /* wchar_t */
};
static const PSCOpacoPadrao PS_C_OPACOS[] = {
    { sizeof(FILE), _Alignof(FILE) },  /* FILE */
    { sizeof(fpos_t), _Alignof(fpos_t) },  /* fpos_t */
    { sizeof(max_align_t), _Alignof(max_align_t) },  /* max_align_t */
};
static const PSCCampoPadrao PS_C_CAMPOS[] = {
    { 6035, 6040, offsetof(div_t, quot) },  /* div_t.quot */
    { 6049, 6053, offsetof(div_t, rem) },  /* div_t.rem */
    { 6035, 9366, offsetof(imaxdiv_t, quot) },  /* imaxdiv_t.quot */
    { 6049, 9380, offsetof(imaxdiv_t, rem) },  /* imaxdiv_t.rem */
    { 6035, 10487, offsetof(ldiv_t, quot) },  /* ldiv_t.quot */
    { 6049, 10501, offsetof(ldiv_t, rem) },  /* ldiv_t.rem */
    { 6035, 10732, offsetof(lldiv_t, quot) },  /* lldiv_t.quot */
    { 6049, 10751, offsetof(lldiv_t, rem) },  /* lldiv_t.rem */
    { 17485, 17492, offsetof(struct timespec, tv_sec) },  /* struct timespec.tv_sec */
    { 17506, 17514, offsetof(struct timespec, tv_nsec) },  /* struct timespec.tv_nsec */
    { 17537, 17544, offsetof(struct tm, tm_sec) },  /* struct tm.tm_sec */
    { 17555, 17562, offsetof(struct tm, tm_min) },  /* struct tm.tm_min */
    { 17573, 17581, offsetof(struct tm, tm_hour) },  /* struct tm.tm_hour */
    { 17593, 17601, offsetof(struct tm, tm_mday) },  /* struct tm.tm_mday */
    { 17613, 17620, offsetof(struct tm, tm_mon) },  /* struct tm.tm_mon */
    { 17631, 17639, offsetof(struct tm, tm_year) },  /* struct tm.tm_year */
    { 17651, 17659, offsetof(struct tm, tm_wday) },  /* struct tm.tm_wday */
    { 17671, 17679, offsetof(struct tm, tm_yday) },  /* struct tm.tm_yday */
    { 17691, 17700, offsetof(struct tm, tm_isdst) },  /* struct tm.tm_isdst */
};
static const PSCEstruturaPadrao PS_C_ESTRUTURAS[] = {
    { 0, 2, sizeof(div_t), _Alignof(div_t) },  /* div_t */
    { 2, 2, sizeof(imaxdiv_t), _Alignof(imaxdiv_t) },  /* imaxdiv_t */
    { 4, 2, sizeof(ldiv_t), _Alignof(ldiv_t) },  /* ldiv_t */
    { 6, 2, sizeof(lldiv_t), _Alignof(lldiv_t) },  /* lldiv_t */
    { 8, 2, sizeof(struct timespec), _Alignof(struct timespec) },  /* struct timespec */
    { 10, 9, sizeof(struct tm), _Alignof(struct tm) },  /* struct tm */
};

/* ── Windows: o endereço de cada função, ligado pelo compilador ────────────
 * Lá o MinGW liga estáticas várias funções da biblioteca (o printf é o dele),
 * e o que é estático não se acha pelo nome. No Linux e no macOS o endereço
 * sai do dlsym na primeira chamada (vm/ps_c.c). */
#ifdef _WIN32
static void (*ps_c_endereco_win(int i))(void)
{
    switch (i) {
        case 314: return (void (*)(void))_Exit;
        case 320: return (void (*)(void))abort;
        case 322: return (void (*)(void))abs;
        case 323: return (void (*)(void))acos;
        case 324: return (void (*)(void))acosf;
        case 325: return (void (*)(void))acosh;
        case 326: return (void (*)(void))acoshf;
        case 327: return (void (*)(void))acoshl;
        case 328: return (void (*)(void))acosl;
        case 329: return (void (*)(void))aligned_alloc;
        case 330: return (void (*)(void))asctime;
        case 332: return (void (*)(void))asin;
        case 333: return (void (*)(void))asinf;
        case 334: return (void (*)(void))asinh;
        case 335: return (void (*)(void))asinhf;
        case 336: return (void (*)(void))asinhl;
        case 337: return (void (*)(void))asinl;
        case 339: return (void (*)(void))atan;
        case 340: return (void (*)(void))atan2;
        case 341: return (void (*)(void))atan2f;
        case 342: return (void (*)(void))atan2l;
        case 343: return (void (*)(void))atanf;
        case 344: return (void (*)(void))atanh;
        case 345: return (void (*)(void))atanhf;
        case 346: return (void (*)(void))atanhl;
        case 347: return (void (*)(void))atanl;
        case 349: return (void (*)(void))atof;
        case 350: return (void (*)(void))atoi;
        case 351: return (void (*)(void))atol;
        case 352: return (void (*)(void))atoll;
        case 354: return (void (*)(void))bsearch;
        case 356: return (void (*)(void))calloc;
        case 357: return (void (*)(void))cbrt;
        case 358: return (void (*)(void))cbrtf;
        case 359: return (void (*)(void))cbrtl;
        case 360: return (void (*)(void))ceil;
        case 361: return (void (*)(void))ceilf;
        case 362: return (void (*)(void))ceill;
        case 363: return (void (*)(void))clearerr;
        case 364: return (void (*)(void))clock;
        case 367: return (void (*)(void))copysign;
        case 368: return (void (*)(void))copysignf;
        case 369: return (void (*)(void))copysignl;
        case 370: return (void (*)(void))cos;
        case 371: return (void (*)(void))cosf;
        case 372: return (void (*)(void))cosh;
        case 373: return (void (*)(void))coshf;
        case 374: return (void (*)(void))coshl;
        case 375: return (void (*)(void))cosl;
        case 376: return (void (*)(void))ctime;
        case 378: return (void (*)(void))difftime;
        case 379: return (void (*)(void))div;
        case 382: return (void (*)(void))erf;
        case 383: return (void (*)(void))erfc;
        case 384: return (void (*)(void))erfcf;
        case 385: return (void (*)(void))erfcl;
        case 386: return (void (*)(void))erff;
        case 387: return (void (*)(void))erfl;
        case 391: return (void (*)(void))exp;
        case 392: return (void (*)(void))exp2;
        case 393: return (void (*)(void))exp2f;
        case 394: return (void (*)(void))exp2l;
        case 395: return (void (*)(void))expf;
        case 396: return (void (*)(void))expl;
        case 397: return (void (*)(void))expm1;
        case 398: return (void (*)(void))expm1f;
        case 399: return (void (*)(void))expm1l;
        case 400: return (void (*)(void))fabs;
        case 401: return (void (*)(void))fabsf;
        case 402: return (void (*)(void))fabsl;
        case 403: return (void (*)(void))fclose;
        case 404: return (void (*)(void))fdim;
        case 405: return (void (*)(void))fdimf;
        case 406: return (void (*)(void))fdiml;
        case 407: return (void (*)(void))feof;
        case 408: return (void (*)(void))ferror;
        case 409: return (void (*)(void))fflush;
        case 410: return (void (*)(void))fgetc;
        case 411: return (void (*)(void))fgetpos;
        case 412: return (void (*)(void))fgets;
        case 414: return (void (*)(void))floor;
        case 415: return (void (*)(void))floorf;
        case 416: return (void (*)(void))floorl;
        case 417: return (void (*)(void))fma;
        case 418: return (void (*)(void))fmaf;
        case 419: return (void (*)(void))fmal;
        case 420: return (void (*)(void))fmax;
        case 421: return (void (*)(void))fmaxf;
        case 422: return (void (*)(void))fmaxl;
        case 423: return (void (*)(void))fmin;
        case 424: return (void (*)(void))fminf;
        case 425: return (void (*)(void))fminl;
        case 426: return (void (*)(void))fmod;
        case 427: return (void (*)(void))fmodf;
        case 428: return (void (*)(void))fmodl;
        case 429: return (void (*)(void))fopen;
        case 433: return (void (*)(void))fprintf;
        case 435: return (void (*)(void))fputc;
        case 436: return (void (*)(void))fputs;
        case 437: return (void (*)(void))fread;
        case 438: return (void (*)(void))free;
        case 439: return (void (*)(void))freopen;
        case 441: return (void (*)(void))frexp;
        case 442: return (void (*)(void))frexpf;
        case 443: return (void (*)(void))frexpl;
        case 444: return (void (*)(void))fscanf;
        case 446: return (void (*)(void))fseek;
        case 447: return (void (*)(void))fsetpos;
        case 448: return (void (*)(void))ftell;
        case 449: return (void (*)(void))fwrite;
        case 450: return (void (*)(void))getc;
        case 451: return (void (*)(void))getchar;
        case 452: return (void (*)(void))getenv;
        case 456: return (void (*)(void))gmtime;
        case 458: return (void (*)(void))hypot;
        case 459: return (void (*)(void))hypotf;
        case 460: return (void (*)(void))hypotl;
        case 462: return (void (*)(void))ilogb;
        case 463: return (void (*)(void))ilogbf;
        case 464: return (void (*)(void))ilogbl;
        case 465: return (void (*)(void))imaxabs;
        case 466: return (void (*)(void))imaxdiv;
        case 482: return (void (*)(void))isalnum;
        case 483: return (void (*)(void))isalpha;
        case 484: return (void (*)(void))isblank;
        case 485: return (void (*)(void))iscntrl;
        case 486: return (void (*)(void))isdigit;
        case 488: return (void (*)(void))isgraph;
        case 495: return (void (*)(void))islower;
        case 498: return (void (*)(void))isprint;
        case 499: return (void (*)(void))ispunct;
        case 500: return (void (*)(void))isspace;
        case 502: return (void (*)(void))isupper;
        case 503: return (void (*)(void))isxdigit;
        case 504: return (void (*)(void))labs;
        case 505: return (void (*)(void))ldexp;
        case 506: return (void (*)(void))ldexpf;
        case 507: return (void (*)(void))ldexpl;
        case 508: return (void (*)(void))ldiv;
        case 510: return (void (*)(void))lgamma;
        case 511: return (void (*)(void))lgammaf;
        case 512: return (void (*)(void))lgammal;
        case 513: return (void (*)(void))llabs;
        case 514: return (void (*)(void))lldiv;
        case 516: return (void (*)(void))llrint;
        case 517: return (void (*)(void))llrintf;
        case 518: return (void (*)(void))llrintl;
        case 519: return (void (*)(void))llround;
        case 520: return (void (*)(void))llroundf;
        case 521: return (void (*)(void))llroundl;
        case 522: return (void (*)(void))localtime;
        case 524: return (void (*)(void))log;
        case 525: return (void (*)(void))log10;
        case 526: return (void (*)(void))log10f;
        case 527: return (void (*)(void))log10l;
        case 528: return (void (*)(void))log1p;
        case 529: return (void (*)(void))log1pf;
        case 530: return (void (*)(void))log1pl;
        case 531: return (void (*)(void))log2;
        case 532: return (void (*)(void))log2f;
        case 533: return (void (*)(void))log2l;
        case 534: return (void (*)(void))logb;
        case 535: return (void (*)(void))logbf;
        case 536: return (void (*)(void))logbl;
        case 537: return (void (*)(void))logf;
        case 538: return (void (*)(void))logl;
        case 539: return (void (*)(void))lrint;
        case 540: return (void (*)(void))lrintf;
        case 541: return (void (*)(void))lrintl;
        case 542: return (void (*)(void))lround;
        case 543: return (void (*)(void))lroundf;
        case 544: return (void (*)(void))lroundl;
        case 545: return (void (*)(void))malloc;
        case 548: return (void (*)(void))mblen;
        case 549: return (void (*)(void))mbstowcs;
        case 551: return (void (*)(void))mbtowc;
        case 552: return (void (*)(void))memchr;
        case 553: return (void (*)(void))memcmp;
        case 554: return (void (*)(void))memcpy;
        case 556: return (void (*)(void))memmove;
        case 558: return (void (*)(void))memset;
        case 560: return (void (*)(void))mktime;
        case 561: return (void (*)(void))modf;
        case 562: return (void (*)(void))modff;
        case 563: return (void (*)(void))modfl;
        case 564: return (void (*)(void))nan;
        case 565: return (void (*)(void))nanf;
        case 566: return (void (*)(void))nanl;
        case 567: return (void (*)(void))nearbyint;
        case 568: return (void (*)(void))nearbyintf;
        case 569: return (void (*)(void))nearbyintl;
        case 570: return (void (*)(void))nextafter;
        case 571: return (void (*)(void))nextafterf;
        case 572: return (void (*)(void))nextafterl;
        case 573: return (void (*)(void))nexttoward;
        case 574: return (void (*)(void))nexttowardf;
        case 575: return (void (*)(void))nexttowardl;
        case 577: return (void (*)(void))perror;
        case 578: return (void (*)(void))pow;
        case 579: return (void (*)(void))powf;
        case 580: return (void (*)(void))powl;
        case 581: return (void (*)(void))printf;
        case 584: return (void (*)(void))putc;
        case 585: return (void (*)(void))putchar;
        case 586: return (void (*)(void))puts;
        case 587: return (void (*)(void))qsort;
        case 590: return (void (*)(void))rand;
        case 591: return (void (*)(void))realloc;
        case 592: return (void (*)(void))remainder;
        case 593: return (void (*)(void))remainderf;
        case 594: return (void (*)(void))remainderl;
        case 595: return (void (*)(void))remove;
        case 596: return (void (*)(void))remquo;
        case 597: return (void (*)(void))remquof;
        case 598: return (void (*)(void))remquol;
        case 599: return (void (*)(void))rename;
        case 600: return (void (*)(void))rewind;
        case 601: return (void (*)(void))rint;
        case 602: return (void (*)(void))rintf;
        case 603: return (void (*)(void))rintl;
        case 604: return (void (*)(void))round;
        case 605: return (void (*)(void))roundf;
        case 606: return (void (*)(void))roundl;
        case 608: return (void (*)(void))scalbln;
        case 609: return (void (*)(void))scalblnf;
        case 610: return (void (*)(void))scalblnl;
        case 611: return (void (*)(void))scalbn;
        case 612: return (void (*)(void))scalbnf;
        case 613: return (void (*)(void))scalbnl;
        case 614: return (void (*)(void))scanf;
        case 617: return (void (*)(void))setbuf;
        case 618: return (void (*)(void))setvbuf;
        case 620: return (void (*)(void))sin;
        case 621: return (void (*)(void))sinf;
        case 622: return (void (*)(void))sinh;
        case 623: return (void (*)(void))sinhf;
        case 624: return (void (*)(void))sinhl;
        case 625: return (void (*)(void))sinl;
        case 627: return (void (*)(void))snprintf;
        case 629: return (void (*)(void))sprintf;
        case 631: return (void (*)(void))sqrt;
        case 632: return (void (*)(void))sqrtf;
        case 633: return (void (*)(void))sqrtl;
        case 634: return (void (*)(void))srand;
        case 635: return (void (*)(void))sscanf;
        case 640: return (void (*)(void))strcat;
        case 642: return (void (*)(void))strchr;
        case 643: return (void (*)(void))strcmp;
        case 644: return (void (*)(void))strcoll;
        case 645: return (void (*)(void))strcpy;
        case 647: return (void (*)(void))strcspn;
        case 648: return (void (*)(void))strerror;
        case 651: return (void (*)(void))strftime;
        case 652: return (void (*)(void))strlen;
        case 653: return (void (*)(void))strncat;
        case 655: return (void (*)(void))strncmp;
        case 656: return (void (*)(void))strncpy;
        case 659: return (void (*)(void))strpbrk;
        case 660: return (void (*)(void))strrchr;
        case 661: return (void (*)(void))strspn;
        case 662: return (void (*)(void))strstr;
        case 663: return (void (*)(void))strtod;
        case 664: return (void (*)(void))strtof;
        case 665: return (void (*)(void))strtoimax;
        case 666: return (void (*)(void))strtok;
        case 668: return (void (*)(void))strtol;
        case 669: return (void (*)(void))strtold;
        case 670: return (void (*)(void))strtoll;
        case 671: return (void (*)(void))strtoul;
        case 672: return (void (*)(void))strtoull;
        case 673: return (void (*)(void))strtoumax;
        case 676: return (void (*)(void))strxfrm;
        case 677: return (void (*)(void))system;
        case 678: return (void (*)(void))tan;
        case 679: return (void (*)(void))tanf;
        case 680: return (void (*)(void))tanh;
        case 681: return (void (*)(void))tanhf;
        case 682: return (void (*)(void))tanhl;
        case 683: return (void (*)(void))tanl;
        case 684: return (void (*)(void))tgamma;
        case 685: return (void (*)(void))tgammaf;
        case 686: return (void (*)(void))tgammal;
        case 687: return (void (*)(void))time;
        case 689: return (void (*)(void))timespec_get;
        case 690: return (void (*)(void))tmpfile;
        case 692: return (void (*)(void))tmpnam;
        case 694: return (void (*)(void))tolower;
        case 695: return (void (*)(void))toupper;
        case 696: return (void (*)(void))trunc;
        case 697: return (void (*)(void))truncf;
        case 698: return (void (*)(void))truncl;
        case 713: return (void (*)(void))ungetc;
        case 729: return (void (*)(void))wcstoimax;
        case 730: return (void (*)(void))wcstombs;
        case 732: return (void (*)(void))wcstoumax;
        case 733: return (void (*)(void))wctomb;
    }
    return NULL;
}
#endif
