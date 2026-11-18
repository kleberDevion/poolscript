/* GERADO por scripts/gera_c_padrao.pr a partir de vm/c_padrao.decl: não se
 * edita à mão. Mude o .decl e rode `./pool scripts/gera_c_padrao.pr --grava`.
 * Incluído só por vm/ps_c.c, que define os tipos das tabelas e o PS_C_TIPO. */

#define PS_C_NCAB 13
#define PS_C_NNOMES 685

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
    "__bool_true_false_are_defined\0"
    "abort\0"
    "_Noreturn void abort(void)\0"
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
    "fpclassify\0"
    "int fpclassify(double x)\0"
    "fpos_t\0"
    "fprintf\0"
    "int fprintf(FILE * restrict stream, const char * restrict format, ...)\0"
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
    "frexp\0"
    "double frexp(double value, int *exp)\0"
    "frexpf\0"
    "float frexpf(float value, int *exp)\0"
    "frexpl\0"
    "long double frexpl(long double value, int *exp)\0"
    "fscanf\0"
    "int fscanf(FILE * restrict stream, const char * restrict format, ...)\0"
    "__isoc99_fscanf\0"
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
    "gmtime\0"
    "struct tm *gmtime(const time_t *timer)\0"
    "hypot\0"
    "double hypot(double x, double y)\0"
    "hypotf\0"
    "float hypotf(float x, float y)\0"
    "hypotl\0"
    "long double hypotl(long double x, long double y)\0"
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
    "mbtowc\0"
    "int mbtowc(wchar_t * restrict pwc, const char * restrict s, size_t n)\0"
    "memchr\0"
    "void *memchr(const void *s, int c, size_t n)\0"
    "memcmp\0"
    "int memcmp(const void *s1, const void *s2, size_t n)\0"
    "memcpy\0"
    "void *memcpy(void * restrict s1, const void * restrict s2, size_t n)\0"
    "memmove\0"
    "void *memmove(void *s1, const void *s2, size_t n)\0"
    "memset\0"
    "void *memset(void *s, int c, size_t n)\0"
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
    "ptrdiff_t\0"
    "putc\0"
    "int putc(int c, FILE *stream)\0"
    "putchar\0"
    "int putchar(int c)\0"
    "puts\0"
    "int puts(const char *s)\0"
    "qsort\0"
    "void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))\0"
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
    "sprintf\0"
    "int sprintf(char * restrict s, const char * restrict format, ...)\0"
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
    "stderr\0"
    "FILE *stderr\0"
    "stdin\0"
    "FILE *stdin\0"
    "stdout\0"
    "FILE *stdout\0"
    "strcat\0"
    "char *strcat(char * restrict s1, const char * restrict s2)\0"
    "strchr\0"
    "char *strchr(const char *s, int c)\0"
    "strcmp\0"
    "int strcmp(const char *s1, const char *s2)\0"
    "strcoll\0"
    "int strcoll(const char *s1, const char *s2)\0"
    "strcpy\0"
    "char *strcpy(char * restrict s1, const char * restrict s2)\0"
    "strcspn\0"
    "size_t strcspn(const char *s1, const char *s2)\0"
    "strerror\0"
    "char *strerror(int errnum)\0"
    "strftime\0"
    "size_t strftime(char * restrict s, size_t maxsize, const char * restrict format, const struct tm * restrict timeptr)\0"
    "strlen\0"
    "size_t strlen(const char *s)\0"
    "strncat\0"
    "char *strncat(char * restrict s1, const char * restrict s2, size_t n)\0"
    "strncmp\0"
    "int strncmp(const char *s1, const char *s2, size_t n)\0"
    "strncpy\0"
    "char *strncpy(char * restrict s1, const char * restrict s2, size_t n)\0"
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
    "tmpnam\0"
    "char *tmpnam(char *s)\0"
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
    "va_list\0"
    "vfprintf\0"
    "int vfprintf(FILE * restrict stream, const char * restrict format, va_list arg)\0"
    "vfscanf\0"
    "int vfscanf(FILE * restrict stream, const char * restrict format, va_list arg)\0"
    "__isoc99_vfscanf\0"
    "vprintf\0"
    "int vprintf(const char * restrict format, va_list arg)\0"
    "vscanf\0"
    "int vscanf(const char * restrict format, va_list arg)\0"
    "__isoc99_vscanf\0"
    "vsnprintf\0"
    "int vsnprintf(char * restrict s, size_t n, const char * restrict format, va_list arg)\0"
    "vsprintf\0"
    "int vsprintf(char * restrict s, const char * restrict format, va_list arg)\0"
    "vsscanf\0"
    "int vsscanf(const char * restrict s, const char * restrict format, va_list arg)\0"
    "__isoc99_vsscanf\0"
    "wchar_t\0"
    "wcstoimax\0"
    "intmax_t wcstoimax(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base)\0"
    "wcstombs\0"
    "size_t wcstombs(char * restrict s, const wchar_t * restrict pwcs, size_t n)\0"
    "wcstoumax\0"
    "uintmax_t wcstoumax(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base)\0"
    "wctomb\0"
    "int wctomb(char *s, wchar_t wchar)\0";

/* os cabeçalhos, na ordem do .decl: o `cab` de cada nome */
static const uint16_t PS_C_CAB[PS_C_NCAB] = { 1, 9, 17, 25, 36, 45, 52, 62, 71, 80, 88, 97, 106 };

/* ── o import (`import <stdio.h>`) ─────────────────────────────────────────
 * PS_C_TAMBEM: o nome e os outros cabeçalhos com que ele vale, separados por
 * espaço (os nomes que o C11 declara em mais de um cabeçalho). PS_C_INCLUI:
 * o cabeçalho e o que ele traz. */
static const char PS_C_TAMBEM[] =
    "NULL\0locale.h stdio.h stdlib.h string.h time.h wchar.h\0"
    "size_t\0stdio.h stdlib.h string.h time.h uchar.h wchar.h\0"
    "wchar_t\0stdlib.h wchar.h\0"
    "FILE\0wchar.h\0"
    "struct tm\0wchar.h\0"
    "";
static const char PS_C_INCLUI[] =
    "inttypes.h\0stdint.h\0"
    "";

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
    { 1541, 0, 82, 0, 5, PSC_NOME_CONST, 0 },  /* MATH_ERREXCEPT */
    { 1556, 0, 83, 0, 5, PSC_NOME_CONST, 0 },  /* MATH_ERRNO */
    { 1567, 1578, 1, 0, 10, PSC_NOME_VALOR, 0 },  /* MB_CUR_MAX */
    { 1596, 0, 84, 0, 4, PSC_NOME_CONST, 0 },  /* MB_LEN_MAX */
    { 1607, 0, 16, 0, 5, PSC_NOME_CONSTFLO, 0 },  /* NAN */
    { 1611, 0, 0, 0, 7, PSC_NOME_NULO, 0 },  /* NULL */
    { 1616, 0, 0, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIX16 */
    { 1623, 0, 1, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIX32 */
    { 1630, 0, 2, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIX64 */
    { 1637, 0, 3, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIX8 */
    { 1643, 0, 4, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXFAST16 */
    { 1654, 0, 5, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXFAST32 */
    { 1665, 0, 6, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXFAST64 */
    { 1676, 0, 7, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXFAST8 */
    { 1686, 0, 8, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXLEAST16 */
    { 1698, 0, 9, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXLEAST32 */
    { 1710, 0, 10, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXLEAST64 */
    { 1722, 0, 11, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXLEAST8 */
    { 1733, 0, 12, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXMAX */
    { 1741, 0, 13, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIXPTR */
    { 1749, 0, 14, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRId16 */
    { 1756, 0, 15, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRId32 */
    { 1763, 0, 16, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRId64 */
    { 1770, 0, 17, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRId8 */
    { 1776, 0, 18, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdFAST16 */
    { 1787, 0, 19, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdFAST32 */
    { 1798, 0, 20, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdFAST64 */
    { 1809, 0, 21, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdFAST8 */
    { 1819, 0, 22, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdLEAST16 */
    { 1831, 0, 23, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdLEAST32 */
    { 1843, 0, 24, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdLEAST64 */
    { 1855, 0, 25, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdLEAST8 */
    { 1866, 0, 26, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdMAX */
    { 1874, 0, 27, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIdPTR */
    { 1882, 0, 28, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIi16 */
    { 1889, 0, 29, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIi32 */
    { 1896, 0, 30, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIi64 */
    { 1903, 0, 31, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIi8 */
    { 1909, 0, 32, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiFAST16 */
    { 1920, 0, 33, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiFAST32 */
    { 1931, 0, 34, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiFAST64 */
    { 1942, 0, 35, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiFAST8 */
    { 1952, 0, 36, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiLEAST16 */
    { 1964, 0, 37, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiLEAST32 */
    { 1976, 0, 38, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiLEAST64 */
    { 1988, 0, 39, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiLEAST8 */
    { 1999, 0, 40, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiMAX */
    { 2007, 0, 41, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIiPTR */
    { 2015, 0, 42, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIo16 */
    { 2022, 0, 43, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIo32 */
    { 2029, 0, 44, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIo64 */
    { 2036, 0, 45, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIo8 */
    { 2042, 0, 46, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoFAST16 */
    { 2053, 0, 47, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoFAST32 */
    { 2064, 0, 48, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoFAST64 */
    { 2075, 0, 49, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoFAST8 */
    { 2085, 0, 50, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoLEAST16 */
    { 2097, 0, 51, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoLEAST32 */
    { 2109, 0, 52, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoLEAST64 */
    { 2121, 0, 53, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoLEAST8 */
    { 2132, 0, 54, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoMAX */
    { 2140, 0, 55, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIoPTR */
    { 2148, 0, 56, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIu16 */
    { 2155, 0, 57, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIu32 */
    { 2162, 0, 58, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIu64 */
    { 2169, 0, 59, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIu8 */
    { 2175, 0, 60, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuFAST16 */
    { 2186, 0, 61, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuFAST32 */
    { 2197, 0, 62, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuFAST64 */
    { 2208, 0, 63, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuFAST8 */
    { 2218, 0, 64, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuLEAST16 */
    { 2230, 0, 65, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuLEAST32 */
    { 2242, 0, 66, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuLEAST64 */
    { 2254, 0, 67, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuLEAST8 */
    { 2265, 0, 68, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuMAX */
    { 2273, 0, 69, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIuPTR */
    { 2281, 0, 70, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIx16 */
    { 2288, 0, 71, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIx32 */
    { 2295, 0, 72, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIx64 */
    { 2302, 0, 73, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIx8 */
    { 2308, 0, 74, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxFAST16 */
    { 2319, 0, 75, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxFAST32 */
    { 2330, 0, 76, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxFAST64 */
    { 2341, 0, 77, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxFAST8 */
    { 2351, 0, 78, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxLEAST16 */
    { 2363, 0, 79, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxLEAST32 */
    { 2375, 0, 80, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxLEAST64 */
    { 2387, 0, 81, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxLEAST8 */
    { 2398, 0, 82, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxMAX */
    { 2406, 0, 83, 0, 3, PSC_NOME_TEXTO, 0 },  /* PRIxPTR */
    { 2414, 0, 85, 0, 8, PSC_NOME_CONST, 0 },  /* PTRDIFF_MAX */
    { 2426, 0, 86, 0, 8, PSC_NOME_CONST, 0 },  /* PTRDIFF_MIN */
    { 2438, 0, 87, 0, 10, PSC_NOME_CONST, 0 },  /* RAND_MAX */
    { 2447, 0, 88, 0, 4, PSC_NOME_CONST, 0 },  /* SCHAR_MAX */
    { 2457, 0, 89, 0, 4, PSC_NOME_CONST, 0 },  /* SCHAR_MIN */
    { 2467, 0, 84, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNd16 */
    { 2474, 0, 85, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNd32 */
    { 2481, 0, 86, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNd64 */
    { 2488, 0, 87, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNd8 */
    { 2494, 0, 88, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdFAST16 */
    { 2505, 0, 89, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdFAST32 */
    { 2516, 0, 90, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdFAST64 */
    { 2527, 0, 91, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdFAST8 */
    { 2537, 0, 92, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdLEAST16 */
    { 2549, 0, 93, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdLEAST32 */
    { 2561, 0, 94, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdLEAST64 */
    { 2573, 0, 95, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdLEAST8 */
    { 2584, 0, 96, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdMAX */
    { 2592, 0, 97, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNdPTR */
    { 2600, 0, 98, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNi16 */
    { 2607, 0, 99, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNi32 */
    { 2614, 0, 100, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNi64 */
    { 2621, 0, 101, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNi8 */
    { 2627, 0, 102, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiFAST16 */
    { 2638, 0, 103, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiFAST32 */
    { 2649, 0, 104, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiFAST64 */
    { 2660, 0, 105, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiFAST8 */
    { 2670, 0, 106, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiLEAST16 */
    { 2682, 0, 107, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiLEAST32 */
    { 2694, 0, 108, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiLEAST64 */
    { 2706, 0, 109, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiLEAST8 */
    { 2717, 0, 110, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiMAX */
    { 2725, 0, 111, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNiPTR */
    { 2733, 0, 112, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNo16 */
    { 2740, 0, 113, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNo32 */
    { 2747, 0, 114, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNo64 */
    { 2754, 0, 115, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNo8 */
    { 2760, 0, 116, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoFAST16 */
    { 2771, 0, 117, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoFAST32 */
    { 2782, 0, 118, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoFAST64 */
    { 2793, 0, 119, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoFAST8 */
    { 2803, 0, 120, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoLEAST16 */
    { 2815, 0, 121, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoLEAST32 */
    { 2827, 0, 122, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoLEAST64 */
    { 2839, 0, 123, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoLEAST8 */
    { 2850, 0, 124, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoMAX */
    { 2858, 0, 125, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNoPTR */
    { 2866, 0, 126, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNu16 */
    { 2873, 0, 127, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNu32 */
    { 2880, 0, 128, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNu64 */
    { 2887, 0, 129, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNu8 */
    { 2893, 0, 130, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuFAST16 */
    { 2904, 0, 131, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuFAST32 */
    { 2915, 0, 132, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuFAST64 */
    { 2926, 0, 133, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuFAST8 */
    { 2936, 0, 134, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuLEAST16 */
    { 2948, 0, 135, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuLEAST32 */
    { 2960, 0, 136, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuLEAST64 */
    { 2972, 0, 137, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuLEAST8 */
    { 2983, 0, 138, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuMAX */
    { 2991, 0, 139, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNuPTR */
    { 2999, 0, 140, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNx16 */
    { 3006, 0, 141, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNx32 */
    { 3013, 0, 142, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNx64 */
    { 3020, 0, 143, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNx8 */
    { 3026, 0, 144, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxFAST16 */
    { 3037, 0, 145, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxFAST32 */
    { 3048, 0, 146, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxFAST64 */
    { 3059, 0, 147, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxFAST8 */
    { 3069, 0, 148, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxLEAST16 */
    { 3081, 0, 149, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxLEAST32 */
    { 3093, 0, 150, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxLEAST64 */
    { 3105, 0, 151, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxLEAST8 */
    { 3116, 0, 152, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxMAX */
    { 3124, 0, 153, 0, 3, PSC_NOME_TEXTO, 0 },  /* SCNxPTR */
    { 3132, 0, 90, 0, 9, PSC_NOME_CONST, 0 },  /* SEEK_CUR */
    { 3141, 0, 91, 0, 9, PSC_NOME_CONST, 0 },  /* SEEK_END */
    { 3150, 0, 92, 0, 9, PSC_NOME_CONST, 0 },  /* SEEK_SET */
    { 3159, 0, 93, 0, 4, PSC_NOME_CONST, 0 },  /* SHRT_MAX */
    { 3168, 0, 94, 0, 4, PSC_NOME_CONST, 0 },  /* SHRT_MIN */
    { 3177, 0, 95, 0, 8, PSC_NOME_CONST, 0 },  /* SIG_ATOMIC_MAX */
    { 3192, 0, 96, 0, 8, PSC_NOME_CONST, 0 },  /* SIG_ATOMIC_MIN */
    { 3207, 0, 97, 0, 8, PSC_NOME_CONST, 0 },  /* SIZE_MAX */
    { 3216, 0, 98, 0, 12, PSC_NOME_CONST, 0 },  /* TIME_UTC */
    { 3225, 0, 99, 0, 9, PSC_NOME_CONST, 0 },  /* TMP_MAX */
    { 3233, 0, 100, 0, 4, PSC_NOME_CONST, 0 },  /* UCHAR_MAX */
    { 3243, 3252, 5, 0, 8, PSC_NOME_LITERAL, 0 },  /* UINT16_C */
    { 3276, 0, 101, 0, 8, PSC_NOME_CONST, 0 },  /* UINT16_MAX */
    { 3287, 3296, 6, 0, 8, PSC_NOME_LITERAL, 0 },  /* UINT32_C */
    { 3342, 0, 102, 0, 8, PSC_NOME_CONST, 0 },  /* UINT32_MAX */
    { 3353, 3362, 7, 0, 8, PSC_NOME_LITERAL, 0 },  /* UINT64_C */
    { 3408, 0, 103, 0, 8, PSC_NOME_CONST, 0 },  /* UINT64_MAX */
    { 3419, 3427, 8, 0, 8, PSC_NOME_LITERAL, 0 },  /* UINT8_C */
    { 3450, 0, 104, 0, 8, PSC_NOME_CONST, 0 },  /* UINT8_MAX */
    { 3460, 3470, 9, 0, 8, PSC_NOME_LITERAL, 0 },  /* UINTMAX_C */
    { 3507, 0, 105, 0, 8, PSC_NOME_CONST, 0 },  /* UINTMAX_MAX */
    { 3519, 0, 106, 0, 8, PSC_NOME_CONST, 0 },  /* UINTPTR_MAX */
    { 3531, 0, 107, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_FAST16_MAX */
    { 3547, 0, 108, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_FAST32_MAX */
    { 3563, 0, 109, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_FAST64_MAX */
    { 3579, 0, 110, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_FAST8_MAX */
    { 3594, 0, 111, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_LEAST16_MAX */
    { 3611, 0, 112, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_LEAST32_MAX */
    { 3628, 0, 113, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_LEAST64_MAX */
    { 3645, 0, 114, 0, 8, PSC_NOME_CONST, 0 },  /* UINT_LEAST8_MAX */
    { 3661, 0, 115, 0, 4, PSC_NOME_CONST, 0 },  /* UINT_MAX */
    { 3670, 0, 116, 0, 4, PSC_NOME_CONST, 0 },  /* ULLONG_MAX */
    { 3681, 0, 117, 0, 4, PSC_NOME_CONST, 0 },  /* ULONG_MAX */
    { 3691, 0, 118, 0, 4, PSC_NOME_CONST, 0 },  /* USHRT_MAX */
    { 3701, 0, 119, 0, 8, PSC_NOME_CONST, 0 },  /* WCHAR_MAX */
    { 3711, 0, 120, 0, 8, PSC_NOME_CONST, 0 },  /* WCHAR_MIN */
    { 3721, 0, 121, 0, 8, PSC_NOME_CONST, 0 },  /* WINT_MAX */
    { 3730, 0, 122, 0, 8, PSC_NOME_CONST, 0 },  /* WINT_MIN */
    { 3739, 3745, 0, 0, 10, PSC_NOME_FUNCAO, 0 },  /* _Exit */
    { 3778, 0, 123, 0, 9, PSC_NOME_CONST, 0 },  /* _IOFBF */
    { 3785, 0, 124, 0, 9, PSC_NOME_CONST, 0 },  /* _IOLBF */
    { 3792, 0, 125, 0, 9, PSC_NOME_CONST, 0 },  /* _IONBF */
    { 3799, 0, 126, 0, 6, PSC_NOME_CONST, 0 },  /* __bool_true_false_are_defined */
    { 3829, 3835, 1, 0, 10, PSC_NOME_FUNCAO, 0 },  /* abort */
    { 3862, 3866, 2, 0, 10, PSC_NOME_FUNCAO, 0 },  /* abs */
    { 3881, 3886, 3, 0, 5, PSC_NOME_FUNCAO, 0 },  /* acos */
    { 3908, 3914, 4, 0, 5, PSC_NOME_FUNCAO, 0 },  /* acosf */
    { 3935, 3941, 5, 0, 5, PSC_NOME_FUNCAO, 0 },  /* acosh */
    { 3964, 3971, 6, 0, 5, PSC_NOME_FUNCAO, 0 },  /* acoshf */
    { 3993, 4000, 7, 0, 5, PSC_NOME_FUNCAO, 0 },  /* acoshl */
    { 4034, 4040, 8, 0, 5, PSC_NOME_FUNCAO, 0 },  /* acosl */
    { 4073, 4087, 9, 0, 10, PSC_NOME_FUNCAO, 0 },  /* aligned_alloc */
    { 4138, 4146, 10, 0, 12, PSC_NOME_FUNCAO, 0 },  /* asctime */
    { 4186, 4191, 11, 0, 5, PSC_NOME_FUNCAO, 0 },  /* asin */
    { 4213, 4219, 12, 0, 5, PSC_NOME_FUNCAO, 0 },  /* asinf */
    { 4240, 4246, 13, 0, 5, PSC_NOME_FUNCAO, 0 },  /* asinh */
    { 4269, 4276, 14, 0, 5, PSC_NOME_FUNCAO, 0 },  /* asinhf */
    { 4298, 4305, 15, 0, 5, PSC_NOME_FUNCAO, 0 },  /* asinhl */
    { 4339, 4345, 16, 0, 5, PSC_NOME_FUNCAO, 0 },  /* asinl */
    { 4378, 4392, 17, 0, 10, PSC_NOME_FUNCAO, PSC_MARCA_VM },  /* at_quick_exit */
    { 4430, 4435, 18, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atan */
    { 4457, 4463, 19, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atan2 */
    { 4496, 4503, 20, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atan2f */
    { 4534, 4541, 21, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atan2l */
    { 4590, 4596, 22, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atanf */
    { 4617, 4623, 23, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atanh */
    { 4646, 4653, 24, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atanhf */
    { 4675, 4682, 25, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atanhl */
    { 4716, 4722, 26, 0, 5, PSC_NOME_FUNCAO, 0 },  /* atanl */
    { 4755, 4762, 27, 0, 10, PSC_NOME_FUNCAO, PSC_MARCA_VM },  /* atexit */
    { 4793, 4798, 28, 0, 10, PSC_NOME_FUNCAO, 0 },  /* atof */
    { 4828, 4833, 29, 0, 10, PSC_NOME_FUNCAO, 0 },  /* atoi */
    { 4860, 4865, 30, 0, 10, PSC_NOME_FUNCAO, 0 },  /* atol */
    { 4897, 4903, 31, 0, 10, PSC_NOME_FUNCAO, 0 },  /* atoll */
    { 4941, 0, 0, 0, 6, PSC_NOME_TIPO, 0 },  /* bool */
    { 4946, 4954, 32, 0, 10, PSC_NOME_FUNCAO, 0 },  /* bsearch */
    { 5073, 5080, 33, 0, 10, PSC_NOME_FUNCAO, 0 },  /* calloc */
    { 5120, 5125, 34, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cbrt */
    { 5147, 5153, 35, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cbrtf */
    { 5174, 5180, 36, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cbrtl */
    { 5213, 5218, 37, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ceil */
    { 5240, 5246, 38, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ceilf */
    { 5267, 5273, 39, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ceill */
    { 5306, 5315, 40, 0, 9, PSC_NOME_FUNCAO, 0 },  /* clearerr */
    { 5343, 5349, 41, 0, 12, PSC_NOME_FUNCAO, 0 },  /* clock */
    { 5369, 0, 1, 0, 12, PSC_NOME_TIPO, 0 },  /* clock_t */
    { 5377, 5386, 42, 0, 5, PSC_NOME_FUNCAO, 0 },  /* copysign */
    { 5422, 5432, 43, 0, 5, PSC_NOME_FUNCAO, 0 },  /* copysignf */
    { 5466, 5476, 44, 0, 5, PSC_NOME_FUNCAO, 0 },  /* copysignl */
    { 5528, 5532, 45, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cos */
    { 5553, 5558, 46, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cosf */
    { 5578, 5583, 47, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cosh */
    { 5605, 5611, 48, 0, 5, PSC_NOME_FUNCAO, 0 },  /* coshf */
    { 5632, 5638, 49, 0, 5, PSC_NOME_FUNCAO, 0 },  /* coshl */
    { 5671, 5676, 50, 0, 5, PSC_NOME_FUNCAO, 0 },  /* cosl */
    { 5708, 5714, 51, 0, 12, PSC_NOME_FUNCAO, 0 },  /* ctime */
    { 5747, 5756, 52, 0, 12, PSC_NOME_FUNCAO, 0 },  /* difftime */
    { 5800, 5804, 53, 0, 10, PSC_NOME_FUNCAO, 0 },  /* div */
    { 5836, 5836, 0, 0, 10, PSC_NOME_ESTRUTURA, 0 },  /* div_t */
    { 5868, 0, 2, 0, 5, PSC_NOME_TIPO, 0 },  /* double_t */
    { 5877, 5881, 54, 0, 5, PSC_NOME_FUNCAO, 0 },  /* erf */
    { 5902, 5907, 55, 0, 5, PSC_NOME_FUNCAO, 0 },  /* erfc */
    { 5929, 5935, 56, 0, 5, PSC_NOME_FUNCAO, 0 },  /* erfcf */
    { 5956, 5962, 57, 0, 5, PSC_NOME_FUNCAO, 0 },  /* erfcl */
    { 5995, 6000, 58, 0, 5, PSC_NOME_FUNCAO, 0 },  /* erff */
    { 6020, 6025, 59, 0, 5, PSC_NOME_FUNCAO, 0 },  /* erfl */
    { 6057, 6063, 2, 0, 1, PSC_NOME_VALOR, PSC_MARCA_VM },  /* errno */
    { 6073, 6078, 60, 0, 10, PSC_NOME_FUNCAO, PSC_MARCA_VM },  /* exit */
    { 6110, 6114, 61, 0, 5, PSC_NOME_FUNCAO, 0 },  /* exp */
    { 6135, 6140, 62, 0, 5, PSC_NOME_FUNCAO, 0 },  /* exp2 */
    { 6162, 6168, 63, 0, 5, PSC_NOME_FUNCAO, 0 },  /* exp2f */
    { 6189, 6195, 64, 0, 5, PSC_NOME_FUNCAO, 0 },  /* exp2l */
    { 6228, 6233, 65, 0, 5, PSC_NOME_FUNCAO, 0 },  /* expf */
    { 6253, 6258, 66, 0, 5, PSC_NOME_FUNCAO, 0 },  /* expl */
    { 6290, 6296, 67, 0, 5, PSC_NOME_FUNCAO, 0 },  /* expm1 */
    { 6319, 6326, 68, 0, 5, PSC_NOME_FUNCAO, 0 },  /* expm1f */
    { 6348, 6355, 69, 0, 5, PSC_NOME_FUNCAO, 0 },  /* expm1l */
    { 6389, 6394, 70, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fabs */
    { 6416, 6422, 71, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fabsf */
    { 6443, 6449, 72, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fabsl */
    { 6482, 6489, 73, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fclose */
    { 6514, 6519, 74, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fdim */
    { 6551, 6557, 75, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fdimf */
    { 6587, 6593, 76, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fdiml */
    { 6641, 6646, 77, 0, 9, PSC_NOME_FUNCAO, 0 },  /* feof */
    { 6669, 6676, 78, 0, 9, PSC_NOME_FUNCAO, 0 },  /* ferror */
    { 6701, 6708, 79, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fflush */
    { 6733, 6739, 80, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fgetc */
    { 6763, 6771, 81, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fgetpos */
    { 6830, 6836, 82, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fgets */
    { 6898, 0, 3, 0, 5, PSC_NOME_TIPO, 0 },  /* float_t */
    { 6906, 6912, 83, 0, 5, PSC_NOME_FUNCAO, 0 },  /* floor */
    { 6935, 6942, 84, 0, 5, PSC_NOME_FUNCAO, 0 },  /* floorf */
    { 6964, 6971, 85, 0, 5, PSC_NOME_FUNCAO, 0 },  /* floorl */
    { 7005, 7009, 86, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fma */
    { 7050, 7055, 87, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmaf */
    { 7093, 7098, 88, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmal */
    { 7160, 7165, 89, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmax */
    { 7197, 7203, 90, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmaxf */
    { 7233, 7239, 91, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmaxl */
    { 7287, 7292, 92, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmin */
    { 7324, 7330, 93, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fminf */
    { 7360, 7366, 94, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fminl */
    { 7414, 7419, 95, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmod */
    { 7451, 7457, 96, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmodf */
    { 7487, 7493, 97, 0, 5, PSC_NOME_FUNCAO, 0 },  /* fmodl */
    { 7541, 7547, 98, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fopen */
    { 7619, 7630, 0, 0, 5, PSC_NOME_MACRO, 0 },  /* fpclassify */
    { 7655, 0, 1, 0, 9, PSC_NOME_OPACO, 0 },  /* fpos_t */
    { 7662, 7670, 99, 0, 9, PSC_NOME_FUNCAO, PSC_MARCA_PRINTF },  /* fprintf */
    { 7741, 7747, 100, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fputc */
    { 7778, 7784, 101, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fputs */
    { 7843, 7849, 102, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fread */
    { 7934, 7939, 103, 0, 10, PSC_NOME_FUNCAO, 0 },  /* free */
    { 7960, 7968, 104, 0, 9, PSC_NOME_FUNCAO, 0 },  /* freopen */
    { 8066, 8072, 105, 0, 5, PSC_NOME_FUNCAO, 0 },  /* frexp */
    { 8109, 8116, 106, 0, 5, PSC_NOME_FUNCAO, 0 },  /* frexpf */
    { 8152, 8159, 107, 0, 5, PSC_NOME_FUNCAO, 0 },  /* frexpl */
    { 8207, 8214, 108, 8284, 9, PSC_NOME_FUNCAO, PSC_MARCA_SCANF },  /* fscanf */
    { 8300, 8306, 109, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fseek */
    { 8359, 8367, 110, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fsetpos */
    { 8412, 8418, 111, 0, 9, PSC_NOME_FUNCAO, 0 },  /* ftell */
    { 8447, 8454, 112, 0, 9, PSC_NOME_FUNCAO, 0 },  /* fwrite */
    { 8546, 8551, 113, 0, 9, PSC_NOME_FUNCAO, 0 },  /* getc */
    { 8574, 8582, 114, 0, 9, PSC_NOME_FUNCAO, 0 },  /* getchar */
    { 8600, 8607, 115, 0, 10, PSC_NOME_FUNCAO, 0 },  /* getenv */
    { 8638, 8645, 116, 0, 12, PSC_NOME_FUNCAO, 0 },  /* gmtime */
    { 8684, 8690, 117, 0, 5, PSC_NOME_FUNCAO, 0 },  /* hypot */
    { 8723, 8730, 118, 0, 5, PSC_NOME_FUNCAO, 0 },  /* hypotf */
    { 8761, 8768, 119, 0, 5, PSC_NOME_FUNCAO, 0 },  /* hypotl */
    { 8817, 8823, 120, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ilogb */
    { 8843, 8850, 121, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ilogbf */
    { 8870, 8877, 122, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ilogbl */
    { 8903, 8911, 123, 0, 3, PSC_NOME_FUNCAO, 0 },  /* imaxabs */
    { 8940, 8948, 124, 0, 3, PSC_NOME_FUNCAO, 0 },  /* imaxdiv */
    { 8998, 8998, 1, 0, 3, PSC_NOME_ESTRUTURA, 0 },  /* imaxdiv_t */
    { 9035, 0, 4, 0, 8, PSC_NOME_TIPO, 0 },  /* int16_t */
    { 9043, 0, 5, 0, 8, PSC_NOME_TIPO, 0 },  /* int32_t */
    { 9051, 0, 6, 0, 8, PSC_NOME_TIPO, 0 },  /* int64_t */
    { 9059, 0, 7, 0, 8, PSC_NOME_TIPO, 0 },  /* int8_t */
    { 9066, 0, 8, 0, 8, PSC_NOME_TIPO, 0 },  /* int_fast16_t */
    { 9079, 0, 9, 0, 8, PSC_NOME_TIPO, 0 },  /* int_fast32_t */
    { 9092, 0, 10, 0, 8, PSC_NOME_TIPO, 0 },  /* int_fast64_t */
    { 9105, 0, 11, 0, 8, PSC_NOME_TIPO, 0 },  /* int_fast8_t */
    { 9117, 0, 12, 0, 8, PSC_NOME_TIPO, 0 },  /* int_least16_t */
    { 9131, 0, 13, 0, 8, PSC_NOME_TIPO, 0 },  /* int_least32_t */
    { 9145, 0, 14, 0, 8, PSC_NOME_TIPO, 0 },  /* int_least64_t */
    { 9159, 0, 15, 0, 8, PSC_NOME_TIPO, 0 },  /* int_least8_t */
    { 9172, 0, 16, 0, 8, PSC_NOME_TIPO, 0 },  /* intmax_t */
    { 9181, 0, 17, 0, 8, PSC_NOME_TIPO, 0 },  /* intptr_t */
    { 9190, 9198, 125, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isalnum */
    { 9217, 9225, 126, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isalpha */
    { 9244, 9252, 127, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isblank */
    { 9271, 9279, 128, 0, 0, PSC_NOME_FUNCAO, 0 },  /* iscntrl */
    { 9298, 9306, 129, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isdigit */
    { 9325, 9334, 1, 0, 5, PSC_NOME_MACRO, 0 },  /* isfinite */
    { 9357, 9365, 130, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isgraph */
    { 9384, 9394, 2, 0, 5, PSC_NOME_MACRO, 0 },  /* isgreater */
    { 9428, 9443, 3, 0, 5, PSC_NOME_MACRO, 0 },  /* isgreaterequal */
    { 9482, 9488, 4, 0, 5, PSC_NOME_MACRO, 0 },  /* isinf */
    { 9508, 9515, 5, 0, 5, PSC_NOME_MACRO, 0 },  /* isless */
    { 9546, 9558, 6, 0, 5, PSC_NOME_MACRO, 0 },  /* islessequal */
    { 9594, 9608, 7, 0, 5, PSC_NOME_MACRO, 0 },  /* islessgreater */
    { 9646, 9654, 131, 0, 0, PSC_NOME_FUNCAO, 0 },  /* islower */
    { 9673, 9679, 8, 0, 5, PSC_NOME_MACRO, 0 },  /* isnan */
    { 9699, 9708, 9, 0, 5, PSC_NOME_MACRO, 0 },  /* isnormal */
    { 9731, 9739, 132, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isprint */
    { 9758, 9766, 133, 0, 0, PSC_NOME_FUNCAO, 0 },  /* ispunct */
    { 9785, 9793, 134, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isspace */
    { 9812, 9824, 10, 0, 5, PSC_NOME_MACRO, 0 },  /* isunordered */
    { 9860, 9868, 135, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isupper */
    { 9887, 9896, 136, 0, 0, PSC_NOME_FUNCAO, 0 },  /* isxdigit */
    { 9916, 9921, 137, 0, 10, PSC_NOME_FUNCAO, 0 },  /* labs */
    { 9947, 9953, 138, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ldexp */
    { 9985, 9992, 139, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ldexpf */
    { 10023, 10030, 140, 0, 5, PSC_NOME_FUNCAO, 0 },  /* ldexpl */
    { 10073, 10078, 141, 0, 10, PSC_NOME_FUNCAO, 0 },  /* ldiv */
    { 10122, 10122, 2, 0, 10, PSC_NOME_ESTRUTURA, 0 },  /* ldiv_t */
    { 10156, 10163, 142, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lgamma */
    { 10187, 10195, 143, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lgammaf */
    { 10218, 10226, 144, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lgammal */
    { 10261, 10267, 145, 0, 10, PSC_NOME_FUNCAO, 0 },  /* llabs */
    { 10304, 10310, 146, 0, 10, PSC_NOME_FUNCAO, 0 },  /* lldiv */
    { 10366, 10366, 3, 0, 10, PSC_NOME_ESTRUTURA, 0 },  /* lldiv_t */
    { 10411, 10418, 147, 0, 5, PSC_NOME_FUNCAO, 0 },  /* llrint */
    { 10449, 10457, 148, 0, 5, PSC_NOME_FUNCAO, 0 },  /* llrintf */
    { 10488, 10496, 149, 0, 5, PSC_NOME_FUNCAO, 0 },  /* llrintl */
    { 10533, 10541, 150, 0, 5, PSC_NOME_FUNCAO, 0 },  /* llround */
    { 10573, 10582, 151, 0, 5, PSC_NOME_FUNCAO, 0 },  /* llroundf */
    { 10614, 10623, 152, 0, 5, PSC_NOME_FUNCAO, 0 },  /* llroundl */
    { 10661, 10671, 153, 0, 12, PSC_NOME_FUNCAO, 0 },  /* localtime */
    { 10713, 10717, 154, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log */
    { 10738, 10744, 155, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log10 */
    { 10767, 10774, 156, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log10f */
    { 10796, 10803, 157, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log10l */
    { 10837, 10843, 158, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log1p */
    { 10866, 10873, 159, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log1pf */
    { 10895, 10902, 160, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log1pl */
    { 10936, 10941, 161, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log2 */
    { 10963, 10969, 162, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log2f */
    { 10990, 10996, 163, 0, 5, PSC_NOME_FUNCAO, 0 },  /* log2l */
    { 11029, 11034, 164, 0, 5, PSC_NOME_FUNCAO, 0 },  /* logb */
    { 11056, 11062, 165, 0, 5, PSC_NOME_FUNCAO, 0 },  /* logbf */
    { 11083, 11089, 166, 0, 5, PSC_NOME_FUNCAO, 0 },  /* logbl */
    { 11122, 11127, 167, 0, 5, PSC_NOME_FUNCAO, 0 },  /* logf */
    { 11147, 11152, 168, 0, 5, PSC_NOME_FUNCAO, 0 },  /* logl */
    { 11184, 11190, 169, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lrint */
    { 11215, 11222, 170, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lrintf */
    { 11247, 11254, 171, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lrintl */
    { 11285, 11292, 172, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lround */
    { 11318, 11326, 173, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lroundf */
    { 11352, 11360, 174, 0, 5, PSC_NOME_FUNCAO, 0 },  /* lroundl */
    { 11392, 11399, 175, 0, 10, PSC_NOME_FUNCAO, 0 },  /* malloc */
    { 11425, 11442, 3, 0, 5, PSC_NOME_VALOR, 0 },  /* math_errhandling */
    { 11463, 0, 2, 0, 7, PSC_NOME_OPACO, 0 },  /* max_align_t */
    { 11475, 11481, 176, 0, 10, PSC_NOME_FUNCAO, 0 },  /* mblen */
    { 11516, 11525, 177, 0, 10, PSC_NOME_FUNCAO, 0 },  /* mbstowcs */
    { 11601, 11608, 178, 0, 10, PSC_NOME_FUNCAO, 0 },  /* mbtowc */
    { 11678, 11685, 179, 0, 11, PSC_NOME_FUNCAO, 0 },  /* memchr */
    { 11730, 11737, 180, 0, 11, PSC_NOME_FUNCAO, 0 },  /* memcmp */
    { 11790, 11797, 181, 0, 11, PSC_NOME_FUNCAO, 0 },  /* memcpy */
    { 11866, 11874, 182, 0, 11, PSC_NOME_FUNCAO, 0 },  /* memmove */
    { 11924, 11931, 183, 0, 11, PSC_NOME_FUNCAO, 0 },  /* memset */
    { 11970, 11977, 184, 0, 12, PSC_NOME_FUNCAO, 0 },  /* mktime */
    { 12011, 12016, 185, 0, 5, PSC_NOME_FUNCAO, 0 },  /* modf */
    { 12056, 12062, 186, 0, 5, PSC_NOME_FUNCAO, 0 },  /* modff */
    { 12100, 12106, 187, 0, 5, PSC_NOME_FUNCAO, 0 },  /* modfl */
    { 12162, 12166, 188, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nan */
    { 12195, 12200, 189, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nanf */
    { 12229, 12234, 190, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nanl */
    { 12269, 12279, 191, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nearbyint */
    { 12306, 12317, 192, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nearbyintf */
    { 12343, 12354, 193, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nearbyintl */
    { 12392, 12402, 194, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nextafter */
    { 12439, 12450, 195, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nextafterf */
    { 12485, 12496, 196, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nextafterl */
    { 12549, 12560, 197, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nexttoward */
    { 12603, 12615, 198, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nexttowardf */
    { 12657, 12669, 199, 0, 5, PSC_NOME_FUNCAO, 0 },  /* nexttowardl */
    { 12723, 12732, 0, 0, 7, PSC_NOME_SINTAXE, 0 },  /* offsetof */
    { 12766, 12773, 200, 0, 9, PSC_NOME_FUNCAO, 0 },  /* perror */
    { 12800, 12804, 201, 0, 5, PSC_NOME_FUNCAO, 0 },  /* pow */
    { 12835, 12840, 202, 0, 5, PSC_NOME_FUNCAO, 0 },  /* powf */
    { 12869, 12874, 203, 0, 5, PSC_NOME_FUNCAO, 0 },  /* powl */
    { 12921, 12928, 204, 0, 9, PSC_NOME_FUNCAO, PSC_MARCA_PRINTF },  /* printf */
    { 12974, 0, 18, 0, 7, PSC_NOME_TIPO, 0 },  /* ptrdiff_t */
    { 12984, 12989, 205, 0, 9, PSC_NOME_FUNCAO, 0 },  /* putc */
    { 13019, 13027, 206, 0, 9, PSC_NOME_FUNCAO, 0 },  /* putchar */
    { 13046, 13051, 207, 0, 9, PSC_NOME_FUNCAO, 0 },  /* puts */
    { 13075, 13081, 208, 0, 10, PSC_NOME_FUNCAO, 0 },  /* qsort */
    { 13174, 13185, 209, 0, 10, PSC_NOME_FUNCAO, PSC_MARCA_VM },  /* quick_exit */
    { 13223, 13228, 210, 0, 10, PSC_NOME_FUNCAO, 0 },  /* rand */
    { 13243, 13251, 211, 0, 10, PSC_NOME_FUNCAO, 0 },  /* realloc */
    { 13289, 13299, 212, 0, 5, PSC_NOME_FUNCAO, 0 },  /* remainder */
    { 13336, 13347, 213, 0, 5, PSC_NOME_FUNCAO, 0 },  /* remainderf */
    { 13382, 13393, 214, 0, 5, PSC_NOME_FUNCAO, 0 },  /* remainderl */
    { 13446, 13453, 215, 0, 9, PSC_NOME_FUNCAO, 0 },  /* remove */
    { 13486, 13493, 216, 0, 5, PSC_NOME_FUNCAO, 0 },  /* remquo */
    { 13537, 13545, 217, 0, 5, PSC_NOME_FUNCAO, 0 },  /* remquof */
    { 13587, 13595, 218, 0, 5, PSC_NOME_FUNCAO, 0 },  /* remquol */
    { 13655, 13662, 219, 0, 9, PSC_NOME_FUNCAO, 0 },  /* rename */
    { 13707, 13714, 220, 0, 9, PSC_NOME_FUNCAO, 0 },  /* rewind */
    { 13740, 13745, 221, 0, 5, PSC_NOME_FUNCAO, 0 },  /* rint */
    { 13767, 13773, 222, 0, 5, PSC_NOME_FUNCAO, 0 },  /* rintf */
    { 13794, 13800, 223, 0, 5, PSC_NOME_FUNCAO, 0 },  /* rintl */
    { 13833, 13839, 224, 0, 5, PSC_NOME_FUNCAO, 0 },  /* round */
    { 13862, 13869, 225, 0, 5, PSC_NOME_FUNCAO, 0 },  /* roundf */
    { 13891, 13898, 226, 0, 5, PSC_NOME_FUNCAO, 0 },  /* roundl */
    { 13932, 13940, 227, 0, 5, PSC_NOME_FUNCAO, 0 },  /* scalbln */
    { 13977, 13986, 228, 0, 5, PSC_NOME_FUNCAO, 0 },  /* scalblnf */
    { 14022, 14031, 229, 0, 5, PSC_NOME_FUNCAO, 0 },  /* scalblnl */
    { 14079, 14086, 230, 0, 5, PSC_NOME_FUNCAO, 0 },  /* scalbn */
    { 14117, 14125, 231, 0, 5, PSC_NOME_FUNCAO, 0 },  /* scalbnf */
    { 14155, 14163, 232, 0, 5, PSC_NOME_FUNCAO, 0 },  /* scalbnl */
    { 14205, 14211, 233, 14256, 9, PSC_NOME_FUNCAO, PSC_MARCA_SCANF },  /* scanf */
    { 14271, 14278, 234, 0, 9, PSC_NOME_FUNCAO, 0 },  /* setbuf */
    { 14335, 14343, 235, 0, 9, PSC_NOME_FUNCAO, 0 },  /* setvbuf */
    { 14423, 14431, 11, 0, 5, PSC_NOME_MACRO, 0 },  /* signbit */
    { 14453, 14457, 236, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sin */
    { 14478, 14483, 237, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sinf */
    { 14503, 14508, 238, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sinh */
    { 14530, 14536, 239, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sinhf */
    { 14557, 14563, 240, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sinhl */
    { 14596, 14601, 241, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sinl */
    { 14633, 0, 19, 0, 7, PSC_NOME_TIPO, 0 },  /* size_t */
    { 14640, 14649, 242, 0, 9, PSC_NOME_FUNCAO, PSC_MARCA_PRINTF },  /* snprintf */
    { 14726, 14734, 243, 0, 9, PSC_NOME_FUNCAO, PSC_MARCA_PRINTF },  /* sprintf */
    { 14800, 14805, 244, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sqrt */
    { 14827, 14833, 245, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sqrtf */
    { 14854, 14860, 246, 0, 5, PSC_NOME_FUNCAO, 0 },  /* sqrtl */
    { 14893, 14899, 247, 0, 10, PSC_NOME_FUNCAO, 0 },  /* srand */
    { 14929, 14936, 248, 15007, 9, PSC_NOME_FUNCAO, PSC_MARCA_SCANF },  /* sscanf */
    { 15023, 15030, 4, 0, 9, PSC_NOME_VALOR, 0 },  /* stderr */
    { 15043, 15049, 5, 0, 9, PSC_NOME_VALOR, 0 },  /* stdin */
    { 15061, 15068, 6, 0, 9, PSC_NOME_VALOR, 0 },  /* stdout */
    { 15081, 15088, 249, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strcat */
    { 15147, 15154, 250, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strchr */
    { 15189, 15196, 251, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strcmp */
    { 15239, 15247, 252, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strcoll */
    { 15291, 15298, 253, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strcpy */
    { 15357, 15365, 254, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strcspn */
    { 15412, 15421, 255, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strerror */
    { 15448, 15457, 256, 0, 12, PSC_NOME_FUNCAO, 0 },  /* strftime */
    { 15574, 15581, 257, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strlen */
    { 15610, 15618, 258, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strncat */
    { 15688, 15696, 259, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strncmp */
    { 15750, 15758, 260, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strncpy */
    { 15828, 15836, 261, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strpbrk */
    { 15882, 15890, 262, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strrchr */
    { 15926, 15933, 263, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strspn */
    { 15979, 15986, 264, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strstr */
    { 16031, 16038, 265, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtod */
    { 16105, 16112, 266, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtof */
    { 16178, 16188, 267, 0, 3, PSC_NOME_FUNCAO, 0 },  /* strtoimax */
    { 16270, 16277, 268, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strtok */
    { 16336, 16343, 269, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtol */
    { 16422, 16430, 270, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtold */
    { 16503, 16511, 271, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtoll */
    { 16596, 16604, 272, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtoul */
    { 16693, 16702, 273, 0, 10, PSC_NOME_FUNCAO, 0 },  /* strtoull */
    { 16797, 16807, 274, 0, 3, PSC_NOME_FUNCAO, 0 },  /* strtoumax */
    { 16890, 16890, 4, 0, 12, PSC_NOME_ESTRUTURA, 0 },  /* struct timespec */
    { 16948, 16948, 5, 0, 12, PSC_NOME_ESTRUTURA, 0 },  /* struct tm */
    { 17134, 17142, 275, 0, 11, PSC_NOME_FUNCAO, 0 },  /* strxfrm */
    { 17213, 17220, 276, 0, 10, PSC_NOME_FUNCAO, 0 },  /* system */
    { 17251, 17255, 277, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tan */
    { 17276, 17281, 278, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tanf */
    { 17301, 17306, 279, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tanh */
    { 17328, 17334, 280, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tanhf */
    { 17355, 17361, 281, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tanhl */
    { 17394, 17399, 282, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tanl */
    { 17431, 17438, 283, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tgamma */
    { 17462, 17470, 284, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tgammaf */
    { 17493, 17501, 285, 0, 5, PSC_NOME_FUNCAO, 0 },  /* tgammal */
    { 17536, 17541, 286, 0, 12, PSC_NOME_FUNCAO, 0 },  /* time */
    { 17568, 0, 20, 0, 12, PSC_NOME_TIPO, 0 },  /* time_t */
    { 17575, 17588, 287, 0, 12, PSC_NOME_FUNCAO, 0 },  /* timespec_get */
    { 17636, 17644, 288, 0, 9, PSC_NOME_FUNCAO, 0 },  /* tmpfile */
    { 17664, 17671, 289, 0, 9, PSC_NOME_FUNCAO, 0 },  /* tmpnam */
    { 17693, 17701, 290, 0, 0, PSC_NOME_FUNCAO, 0 },  /* tolower */
    { 17720, 17728, 291, 0, 0, PSC_NOME_FUNCAO, 0 },  /* toupper */
    { 17747, 17753, 292, 0, 5, PSC_NOME_FUNCAO, 0 },  /* trunc */
    { 17776, 17783, 293, 0, 5, PSC_NOME_FUNCAO, 0 },  /* truncf */
    { 17805, 17812, 294, 0, 5, PSC_NOME_FUNCAO, 0 },  /* truncl */
    { 17846, 0, 21, 0, 8, PSC_NOME_TIPO, 0 },  /* uint16_t */
    { 17855, 0, 22, 0, 8, PSC_NOME_TIPO, 0 },  /* uint32_t */
    { 17864, 0, 23, 0, 8, PSC_NOME_TIPO, 0 },  /* uint64_t */
    { 17873, 0, 24, 0, 8, PSC_NOME_TIPO, 0 },  /* uint8_t */
    { 17881, 0, 25, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_fast16_t */
    { 17895, 0, 26, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_fast32_t */
    { 17909, 0, 27, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_fast64_t */
    { 17923, 0, 28, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_fast8_t */
    { 17936, 0, 29, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_least16_t */
    { 17951, 0, 30, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_least32_t */
    { 17966, 0, 31, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_least64_t */
    { 17981, 0, 32, 0, 8, PSC_NOME_TIPO, 0 },  /* uint_least8_t */
    { 17995, 0, 33, 0, 8, PSC_NOME_TIPO, 0 },  /* uintmax_t */
    { 18005, 0, 34, 0, 8, PSC_NOME_TIPO, 0 },  /* uintptr_t */
    { 18015, 18022, 295, 0, 9, PSC_NOME_FUNCAO, 0 },  /* ungetc */
    { 18054, 0, 3, 0, 7, PSC_NOME_OPACO, 0 },  /* va_list */
    { 18062, 18071, 296, 0, 9, PSC_NOME_FUNCAO, 0 },  /* vfprintf */
    { 18151, 18159, 297, 18238, 9, PSC_NOME_FUNCAO, 0 },  /* vfscanf */
    { 18255, 18263, 298, 0, 9, PSC_NOME_FUNCAO, 0 },  /* vprintf */
    { 18318, 18325, 299, 18379, 9, PSC_NOME_FUNCAO, 0 },  /* vscanf */
    { 18395, 18405, 300, 0, 9, PSC_NOME_FUNCAO, 0 },  /* vsnprintf */
    { 18491, 18500, 301, 0, 9, PSC_NOME_FUNCAO, 0 },  /* vsprintf */
    { 18575, 18583, 302, 18663, 9, PSC_NOME_FUNCAO, 0 },  /* vsscanf */
    { 18680, 0, 35, 0, 7, PSC_NOME_TIPO, 0 },  /* wchar_t */
    { 18688, 18698, 303, 0, 3, PSC_NOME_FUNCAO, 0 },  /* wcstoimax */
    { 18786, 18795, 304, 0, 10, PSC_NOME_FUNCAO, 0 },  /* wcstombs */
    { 18871, 18881, 305, 0, 3, PSC_NOME_FUNCAO, 0 },  /* wcstoumax */
    { 18970, 18977, 306, 0, 10, PSC_NOME_FUNCAO, 0 },  /* wctomb */
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
/* no Windows o cabeçalho declara as do ctype.h com `dllimport`, e a
 * declaração de novo, só pra conferir o tipo, avisa que o perdeu */
#pragma GCC diagnostic ignored "-Wattributes"
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
int (vfprintf)(FILE * restrict stream, const char * restrict format, va_list arg);
int (vfscanf)(FILE * restrict stream, const char * restrict format, va_list arg);
int (vprintf)(const char * restrict format, va_list arg);
int (vscanf)(const char * restrict format, va_list arg);
int (vsnprintf)(char * restrict s, size_t n, const char * restrict format, va_list arg);
int (vsprintf)(char * restrict s, const char * restrict format, va_list arg);
int (vsscanf)(const char * restrict s, const char * restrict format, va_list arg);
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
        case 284: return (void (*)(void))ps_c_m_UINT16_C;
        case 286: return (void (*)(void))ps_c_m_UINT32_C;
        case 288: return (void (*)(void))ps_c_m_UINT64_C;
        case 290: return (void (*)(void))ps_c_m_UINT8_C;
        case 292: return (void (*)(void))ps_c_m_UINTMAX_C;
        case 420: return (void (*)(void))ps_c_m_fpclassify;
        case 468: return (void (*)(void))ps_c_m_isfinite;
        case 470: return (void (*)(void))ps_c_m_isgreater;
        case 471: return (void (*)(void))ps_c_m_isgreaterequal;
        case 472: return (void (*)(void))ps_c_m_isinf;
        case 473: return (void (*)(void))ps_c_m_isless;
        case 474: return (void (*)(void))ps_c_m_islessequal;
        case 475: return (void (*)(void))ps_c_m_islessgreater;
        case 477: return (void (*)(void))ps_c_m_isnan;
        case 478: return (void (*)(void))ps_c_m_isnormal;
        case 482: return (void (*)(void))ps_c_m_isunordered;
        case 590: return (void (*)(void))ps_c_m_signbit;
        case 605: return (void (*)(void))ps_c_v_stderr;
        case 606: return (void (*)(void))ps_c_v_stdin;
        case 607: return (void (*)(void))ps_c_v_stdout;
#ifdef _WIN32
        case 110: return (void (*)(void))ps_c_v_MB_CUR_MAX;
        case 526: return (void (*)(void))ps_c_v_math_errhandling;
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
    { sizeof(va_list), _Alignof(va_list) },  /* va_list */
};
static const PSCCampoPadrao PS_C_CAMPOS[] = {
    { 5842, 5847, offsetof(div_t, quot) },  /* div_t.quot */
    { 5856, 5860, offsetof(div_t, rem) },  /* div_t.rem */
    { 5842, 9008, offsetof(imaxdiv_t, quot) },  /* imaxdiv_t.quot */
    { 5856, 9022, offsetof(imaxdiv_t, rem) },  /* imaxdiv_t.rem */
    { 5842, 10129, offsetof(ldiv_t, quot) },  /* ldiv_t.quot */
    { 5856, 10143, offsetof(ldiv_t, rem) },  /* ldiv_t.rem */
    { 5842, 10374, offsetof(lldiv_t, quot) },  /* lldiv_t.quot */
    { 5856, 10393, offsetof(lldiv_t, rem) },  /* lldiv_t.rem */
    { 16906, 16913, offsetof(struct timespec, tv_sec) },  /* struct timespec.tv_sec */
    { 16927, 16935, offsetof(struct timespec, tv_nsec) },  /* struct timespec.tv_nsec */
    { 16958, 16965, offsetof(struct tm, tm_sec) },  /* struct tm.tm_sec */
    { 16976, 16983, offsetof(struct tm, tm_min) },  /* struct tm.tm_min */
    { 16994, 17002, offsetof(struct tm, tm_hour) },  /* struct tm.tm_hour */
    { 17014, 17022, offsetof(struct tm, tm_mday) },  /* struct tm.tm_mday */
    { 17034, 17041, offsetof(struct tm, tm_mon) },  /* struct tm.tm_mon */
    { 17052, 17060, offsetof(struct tm, tm_year) },  /* struct tm.tm_year */
    { 17072, 17080, offsetof(struct tm, tm_wday) },  /* struct tm.tm_wday */
    { 17092, 17100, offsetof(struct tm, tm_yday) },  /* struct tm.tm_yday */
    { 17112, 17121, offsetof(struct tm, tm_isdst) },  /* struct tm.tm_isdst */
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
        case 311: return (void (*)(void))_Exit;
        case 316: return (void (*)(void))abort;
        case 317: return (void (*)(void))abs;
        case 318: return (void (*)(void))acos;
        case 319: return (void (*)(void))acosf;
        case 320: return (void (*)(void))acosh;
        case 321: return (void (*)(void))acoshf;
        case 322: return (void (*)(void))acoshl;
        case 323: return (void (*)(void))acosl;
        case 325: return (void (*)(void))asctime;
        case 326: return (void (*)(void))asin;
        case 327: return (void (*)(void))asinf;
        case 328: return (void (*)(void))asinh;
        case 329: return (void (*)(void))asinhf;
        case 330: return (void (*)(void))asinhl;
        case 331: return (void (*)(void))asinl;
        case 333: return (void (*)(void))atan;
        case 334: return (void (*)(void))atan2;
        case 335: return (void (*)(void))atan2f;
        case 336: return (void (*)(void))atan2l;
        case 337: return (void (*)(void))atanf;
        case 338: return (void (*)(void))atanh;
        case 339: return (void (*)(void))atanhf;
        case 340: return (void (*)(void))atanhl;
        case 341: return (void (*)(void))atanl;
        case 343: return (void (*)(void))atof;
        case 344: return (void (*)(void))atoi;
        case 345: return (void (*)(void))atol;
        case 346: return (void (*)(void))atoll;
        case 348: return (void (*)(void))bsearch;
        case 349: return (void (*)(void))calloc;
        case 350: return (void (*)(void))cbrt;
        case 351: return (void (*)(void))cbrtf;
        case 352: return (void (*)(void))cbrtl;
        case 353: return (void (*)(void))ceil;
        case 354: return (void (*)(void))ceilf;
        case 355: return (void (*)(void))ceill;
        case 356: return (void (*)(void))clearerr;
        case 357: return (void (*)(void))clock;
        case 359: return (void (*)(void))copysign;
        case 360: return (void (*)(void))copysignf;
        case 361: return (void (*)(void))copysignl;
        case 362: return (void (*)(void))cos;
        case 363: return (void (*)(void))cosf;
        case 364: return (void (*)(void))cosh;
        case 365: return (void (*)(void))coshf;
        case 366: return (void (*)(void))coshl;
        case 367: return (void (*)(void))cosl;
        case 368: return (void (*)(void))ctime;
        case 369: return (void (*)(void))difftime;
        case 370: return (void (*)(void))div;
        case 373: return (void (*)(void))erf;
        case 374: return (void (*)(void))erfc;
        case 375: return (void (*)(void))erfcf;
        case 376: return (void (*)(void))erfcl;
        case 377: return (void (*)(void))erff;
        case 378: return (void (*)(void))erfl;
        case 381: return (void (*)(void))exp;
        case 382: return (void (*)(void))exp2;
        case 383: return (void (*)(void))exp2f;
        case 384: return (void (*)(void))exp2l;
        case 385: return (void (*)(void))expf;
        case 386: return (void (*)(void))expl;
        case 387: return (void (*)(void))expm1;
        case 388: return (void (*)(void))expm1f;
        case 389: return (void (*)(void))expm1l;
        case 390: return (void (*)(void))fabs;
        case 391: return (void (*)(void))fabsf;
        case 392: return (void (*)(void))fabsl;
        case 393: return (void (*)(void))fclose;
        case 394: return (void (*)(void))fdim;
        case 395: return (void (*)(void))fdimf;
        case 396: return (void (*)(void))fdiml;
        case 397: return (void (*)(void))feof;
        case 398: return (void (*)(void))ferror;
        case 399: return (void (*)(void))fflush;
        case 400: return (void (*)(void))fgetc;
        case 401: return (void (*)(void))fgetpos;
        case 402: return (void (*)(void))fgets;
        case 404: return (void (*)(void))floor;
        case 405: return (void (*)(void))floorf;
        case 406: return (void (*)(void))floorl;
        case 407: return (void (*)(void))fma;
        case 408: return (void (*)(void))fmaf;
        case 409: return (void (*)(void))fmal;
        case 410: return (void (*)(void))fmax;
        case 411: return (void (*)(void))fmaxf;
        case 412: return (void (*)(void))fmaxl;
        case 413: return (void (*)(void))fmin;
        case 414: return (void (*)(void))fminf;
        case 415: return (void (*)(void))fminl;
        case 416: return (void (*)(void))fmod;
        case 417: return (void (*)(void))fmodf;
        case 418: return (void (*)(void))fmodl;
        case 419: return (void (*)(void))fopen;
        case 422: return (void (*)(void))fprintf;
        case 423: return (void (*)(void))fputc;
        case 424: return (void (*)(void))fputs;
        case 425: return (void (*)(void))fread;
        case 426: return (void (*)(void))free;
        case 427: return (void (*)(void))freopen;
        case 428: return (void (*)(void))frexp;
        case 429: return (void (*)(void))frexpf;
        case 430: return (void (*)(void))frexpl;
        case 431: return (void (*)(void))fscanf;
        case 432: return (void (*)(void))fseek;
        case 433: return (void (*)(void))fsetpos;
        case 434: return (void (*)(void))ftell;
        case 435: return (void (*)(void))fwrite;
        case 436: return (void (*)(void))getc;
        case 437: return (void (*)(void))getchar;
        case 438: return (void (*)(void))getenv;
        case 439: return (void (*)(void))gmtime;
        case 440: return (void (*)(void))hypot;
        case 441: return (void (*)(void))hypotf;
        case 442: return (void (*)(void))hypotl;
        case 443: return (void (*)(void))ilogb;
        case 444: return (void (*)(void))ilogbf;
        case 445: return (void (*)(void))ilogbl;
        case 446: return (void (*)(void))imaxabs;
        case 447: return (void (*)(void))imaxdiv;
        case 463: return (void (*)(void))isalnum;
        case 464: return (void (*)(void))isalpha;
        case 465: return (void (*)(void))isblank;
        case 466: return (void (*)(void))iscntrl;
        case 467: return (void (*)(void))isdigit;
        case 469: return (void (*)(void))isgraph;
        case 476: return (void (*)(void))islower;
        case 479: return (void (*)(void))isprint;
        case 480: return (void (*)(void))ispunct;
        case 481: return (void (*)(void))isspace;
        case 483: return (void (*)(void))isupper;
        case 484: return (void (*)(void))isxdigit;
        case 485: return (void (*)(void))labs;
        case 486: return (void (*)(void))ldexp;
        case 487: return (void (*)(void))ldexpf;
        case 488: return (void (*)(void))ldexpl;
        case 489: return (void (*)(void))ldiv;
        case 491: return (void (*)(void))lgamma;
        case 492: return (void (*)(void))lgammaf;
        case 493: return (void (*)(void))lgammal;
        case 494: return (void (*)(void))llabs;
        case 495: return (void (*)(void))lldiv;
        case 497: return (void (*)(void))llrint;
        case 498: return (void (*)(void))llrintf;
        case 499: return (void (*)(void))llrintl;
        case 500: return (void (*)(void))llround;
        case 501: return (void (*)(void))llroundf;
        case 502: return (void (*)(void))llroundl;
        case 503: return (void (*)(void))localtime;
        case 504: return (void (*)(void))log;
        case 505: return (void (*)(void))log10;
        case 506: return (void (*)(void))log10f;
        case 507: return (void (*)(void))log10l;
        case 508: return (void (*)(void))log1p;
        case 509: return (void (*)(void))log1pf;
        case 510: return (void (*)(void))log1pl;
        case 511: return (void (*)(void))log2;
        case 512: return (void (*)(void))log2f;
        case 513: return (void (*)(void))log2l;
        case 514: return (void (*)(void))logb;
        case 515: return (void (*)(void))logbf;
        case 516: return (void (*)(void))logbl;
        case 517: return (void (*)(void))logf;
        case 518: return (void (*)(void))logl;
        case 519: return (void (*)(void))lrint;
        case 520: return (void (*)(void))lrintf;
        case 521: return (void (*)(void))lrintl;
        case 522: return (void (*)(void))lround;
        case 523: return (void (*)(void))lroundf;
        case 524: return (void (*)(void))lroundl;
        case 525: return (void (*)(void))malloc;
        case 528: return (void (*)(void))mblen;
        case 529: return (void (*)(void))mbstowcs;
        case 530: return (void (*)(void))mbtowc;
        case 531: return (void (*)(void))memchr;
        case 532: return (void (*)(void))memcmp;
        case 533: return (void (*)(void))memcpy;
        case 534: return (void (*)(void))memmove;
        case 535: return (void (*)(void))memset;
        case 536: return (void (*)(void))mktime;
        case 537: return (void (*)(void))modf;
        case 538: return (void (*)(void))modff;
        case 539: return (void (*)(void))modfl;
        case 540: return (void (*)(void))nan;
        case 541: return (void (*)(void))nanf;
        case 542: return (void (*)(void))nanl;
        case 543: return (void (*)(void))nearbyint;
        case 544: return (void (*)(void))nearbyintf;
        case 545: return (void (*)(void))nearbyintl;
        case 546: return (void (*)(void))nextafter;
        case 547: return (void (*)(void))nextafterf;
        case 548: return (void (*)(void))nextafterl;
        case 549: return (void (*)(void))nexttoward;
        case 550: return (void (*)(void))nexttowardf;
        case 551: return (void (*)(void))nexttowardl;
        case 553: return (void (*)(void))perror;
        case 554: return (void (*)(void))pow;
        case 555: return (void (*)(void))powf;
        case 556: return (void (*)(void))powl;
        case 557: return (void (*)(void))printf;
        case 559: return (void (*)(void))putc;
        case 560: return (void (*)(void))putchar;
        case 561: return (void (*)(void))puts;
        case 562: return (void (*)(void))qsort;
        case 564: return (void (*)(void))rand;
        case 565: return (void (*)(void))realloc;
        case 566: return (void (*)(void))remainder;
        case 567: return (void (*)(void))remainderf;
        case 568: return (void (*)(void))remainderl;
        case 569: return (void (*)(void))remove;
        case 570: return (void (*)(void))remquo;
        case 571: return (void (*)(void))remquof;
        case 572: return (void (*)(void))remquol;
        case 573: return (void (*)(void))rename;
        case 574: return (void (*)(void))rewind;
        case 575: return (void (*)(void))rint;
        case 576: return (void (*)(void))rintf;
        case 577: return (void (*)(void))rintl;
        case 578: return (void (*)(void))round;
        case 579: return (void (*)(void))roundf;
        case 580: return (void (*)(void))roundl;
        case 581: return (void (*)(void))scalbln;
        case 582: return (void (*)(void))scalblnf;
        case 583: return (void (*)(void))scalblnl;
        case 584: return (void (*)(void))scalbn;
        case 585: return (void (*)(void))scalbnf;
        case 586: return (void (*)(void))scalbnl;
        case 587: return (void (*)(void))scanf;
        case 588: return (void (*)(void))setbuf;
        case 589: return (void (*)(void))setvbuf;
        case 591: return (void (*)(void))sin;
        case 592: return (void (*)(void))sinf;
        case 593: return (void (*)(void))sinh;
        case 594: return (void (*)(void))sinhf;
        case 595: return (void (*)(void))sinhl;
        case 596: return (void (*)(void))sinl;
        case 598: return (void (*)(void))snprintf;
        case 599: return (void (*)(void))sprintf;
        case 600: return (void (*)(void))sqrt;
        case 601: return (void (*)(void))sqrtf;
        case 602: return (void (*)(void))sqrtl;
        case 603: return (void (*)(void))srand;
        case 604: return (void (*)(void))sscanf;
        case 608: return (void (*)(void))strcat;
        case 609: return (void (*)(void))strchr;
        case 610: return (void (*)(void))strcmp;
        case 611: return (void (*)(void))strcoll;
        case 612: return (void (*)(void))strcpy;
        case 613: return (void (*)(void))strcspn;
        case 614: return (void (*)(void))strerror;
        case 615: return (void (*)(void))strftime;
        case 616: return (void (*)(void))strlen;
        case 617: return (void (*)(void))strncat;
        case 618: return (void (*)(void))strncmp;
        case 619: return (void (*)(void))strncpy;
        case 620: return (void (*)(void))strpbrk;
        case 621: return (void (*)(void))strrchr;
        case 622: return (void (*)(void))strspn;
        case 623: return (void (*)(void))strstr;
        case 624: return (void (*)(void))strtod;
        case 625: return (void (*)(void))strtof;
        case 626: return (void (*)(void))strtoimax;
        case 627: return (void (*)(void))strtok;
        case 628: return (void (*)(void))strtol;
        case 629: return (void (*)(void))strtold;
        case 630: return (void (*)(void))strtoll;
        case 631: return (void (*)(void))strtoul;
        case 632: return (void (*)(void))strtoull;
        case 633: return (void (*)(void))strtoumax;
        case 636: return (void (*)(void))strxfrm;
        case 637: return (void (*)(void))system;
        case 638: return (void (*)(void))tan;
        case 639: return (void (*)(void))tanf;
        case 640: return (void (*)(void))tanh;
        case 641: return (void (*)(void))tanhf;
        case 642: return (void (*)(void))tanhl;
        case 643: return (void (*)(void))tanl;
        case 644: return (void (*)(void))tgamma;
        case 645: return (void (*)(void))tgammaf;
        case 646: return (void (*)(void))tgammal;
        case 647: return (void (*)(void))time;
        case 649: return (void (*)(void))timespec_get;
        case 650: return (void (*)(void))tmpfile;
        case 651: return (void (*)(void))tmpnam;
        case 652: return (void (*)(void))tolower;
        case 653: return (void (*)(void))toupper;
        case 654: return (void (*)(void))trunc;
        case 655: return (void (*)(void))truncf;
        case 656: return (void (*)(void))truncl;
        case 671: return (void (*)(void))ungetc;
        case 673: return (void (*)(void))vfprintf;
        case 674: return (void (*)(void))vfscanf;
        case 675: return (void (*)(void))vprintf;
        case 676: return (void (*)(void))vscanf;
        case 677: return (void (*)(void))vsnprintf;
        case 678: return (void (*)(void))vsprintf;
        case 679: return (void (*)(void))vsscanf;
        case 681: return (void (*)(void))wcstoimax;
        case 682: return (void (*)(void))wcstombs;
        case 683: return (void (*)(void))wcstoumax;
        case 684: return (void (*)(void))wctomb;
    }
    return NULL;
}
#endif
