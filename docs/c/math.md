# math.h — funções matemáticas

As funções do `math.h` fazem as contas de ponto flutuante: valor absoluto e
resto, exponenciais e logaritmos, potências e raízes, trigonométricas e
hiperbólicas, erro e gama, arredondamento pro inteiro, a manipulação do
número de ponto flutuante e a classificação dele. Os nomes do `math.h` valem
com `import <math.h>` no topo do arquivo, igual ao `#include <math.h>` do C;
sem o import, o nome é `NameError`, com a dica do import que falta
([o C na Jinga](c.md)).

> Esta página segue a cppreference
> ([en.cppreference.com/w/c/numeric/math](https://en.cppreference.com/w/c/numeric/math),
> licença [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)),
> traduzida e adaptada à Jinga. Os protótipos são os do C11 (Anexo B do
> N1570), conferidos contra a tabela do motor. Cada exemplo foi rodado.

| Função | O que faz |
|---|---|
| [`fabs`, `fabsf`, `fabsl`](#fabs-fabsf-fabsl) | valor absoluto |
| [`fmod`, `fmodf`, `fmodl`](#fmod-fmodf-fmodl) | resto da divisão, com o sinal do dividendo |
| [`remainder`, `remainderf`, `remainderl`](#remainder-remainderf-remainderl) | resto IEEE da divisão |
| [`remquo`, `remquof`, `remquol`](#remquo-remquof-remquol) | resto IEEE e os últimos bits do quociente |
| [`fma`, `fmaf`, `fmal`](#fma-fmaf-fmal) | `x * y + z` com um arredondamento só |
| [`fmax`, `fmaxf`, `fmaxl`](#fmax-fmaxf-fmaxl) | o maior de dois |
| [`fmin`, `fminf`, `fminl`](#fmin-fminf-fminl) | o menor de dois |
| [`fdim`, `fdimf`, `fdiml`](#fdim-fdimf-fdiml) | diferença positiva |
| [`nan`, `nanf`, `nanl`](#nan-nanf-nanl) | um NaN a partir de um texto |
| [`exp`, `expf`, `expl`](#exp-expf-expl) | *e* elevado a `x` |
| [`exp2`, `exp2f`, `exp2l`](#exp2-exp2f-exp2l) | 2 elevado a `x` |
| [`expm1`, `expm1f`, `expm1l`](#expm1-expm1f-expm1l) | *e* elevado a `x`, menos 1 |
| [`log`, `logf`, `logl`](#log-logf-logl) | logaritmo natural |
| [`log10`, `log10f`, `log10l`](#log10-log10f-log10l) | logaritmo na base 10 |
| [`log2`, `log2f`, `log2l`](#log2-log2f-log2l) | logaritmo na base 2 |
| [`log1p`, `log1pf`, `log1pl`](#log1p-log1pf-log1pl) | logaritmo natural de 1 + `x` |
| [`pow`, `powf`, `powl`](#pow-powf-powl) | potência (o `pow` é o da Jinga) |
| [`sqrt`, `sqrtf`, `sqrtl`](#sqrt-sqrtf-sqrtl) | raiz quadrada |
| [`cbrt`, `cbrtf`, `cbrtl`](#cbrt-cbrtf-cbrtl) | raiz cúbica |
| [`hypot`, `hypotf`, `hypotl`](#hypot-hypotf-hypotl) | raiz da soma dos quadrados |
| [`sin`, `sinf`, `sinl`](#sin-sinf-sinl) | seno |
| [`cos`, `cosf`, `cosl`](#cos-cosf-cosl) | cosseno |
| [`tan`, `tanf`, `tanl`](#tan-tanf-tanl) | tangente |
| [`asin`, `asinf`, `asinl`](#asin-asinf-asinl) | arco seno |
| [`acos`, `acosf`, `acosl`](#acos-acosf-acosl) | arco cosseno |
| [`atan`, `atanf`, `atanl`](#atan-atanf-atanl) | arco tangente |
| [`atan2`, `atan2f`, `atan2l`](#atan2-atan2f-atan2l) | arco tangente de `y / x`, no quadrante certo |
| [`sinh`, `sinhf`, `sinhl`](#sinh-sinhf-sinhl) | seno hiperbólico |
| [`cosh`, `coshf`, `coshl`](#cosh-coshf-coshl) | cosseno hiperbólico |
| [`tanh`, `tanhf`, `tanhl`](#tanh-tanhf-tanhl) | tangente hiperbólica |
| [`asinh`, `asinhf`, `asinhl`](#asinh-asinhf-asinhl) | seno hiperbólico inverso |
| [`acosh`, `acoshf`, `acoshl`](#acosh-acoshf-acoshl) | cosseno hiperbólico inverso |
| [`atanh`, `atanhf`, `atanhl`](#atanh-atanhf-atanhl) | tangente hiperbólica inversa |
| [`erf`, `erff`, `erfl`](#erf-erff-erfl) | função erro |
| [`erfc`, `erfcf`, `erfcl`](#erfc-erfcf-erfcl) | função erro complementar |
| [`tgamma`, `tgammaf`, `tgammal`](#tgamma-tgammaf-tgammal) | função gama |
| [`lgamma`, `lgammaf`, `lgammal`](#lgamma-lgammaf-lgammal) | logaritmo natural da função gama |
| [`ceil`, `ceilf`, `ceill`](#ceil-ceilf-ceill) | o menor inteiro que não é menor que `x` |
| [`floor`, `floorf`, `floorl`](#floor-floorf-floorl) | o maior inteiro que não é maior que `x` |
| [`trunc`, `truncf`, `truncl`](#trunc-truncf-truncl) | o inteiro na direção do zero |
| [`round`, `roundf`, `roundl`, `lround`, `lroundf`, `lroundl`, `llround`, `llroundf`, `llroundl`](#round-roundf-roundl-lround-lroundf-lroundl-llround-llroundf-llroundl) | o inteiro mais próximo, com o meio pra longe do zero (o `round` é o da Jinga) |
| [`nearbyint`, `nearbyintf`, `nearbyintl`](#nearbyint-nearbyintf-nearbyintl) | o inteiro mais próximo pelo modo de arredondamento |
| [`rint`, `rintf`, `rintl`, `lrint`, `lrintf`, `lrintl`, `llrint`, `llrintf`, `llrintl`](#rint-rintf-rintl-lrint-lrintf-lrintl-llrint-llrintf-llrintl) | o inteiro mais próximo pelo modo de arredondamento |
| [`frexp`, `frexpf`, `frexpl`](#frexp-frexpf-frexpl) | fração e expoente de dois |
| [`ldexp`, `ldexpf`, `ldexpl`](#ldexp-ldexpf-ldexpl) | `x` vezes 2 elevado a `exp` |
| [`modf`, `modff`, `modfl`](#modf-modff-modfl) | parte inteira e parte fracionária |
| [`scalbn`, `scalbnf`, `scalbnl`, `scalbln`, `scalblnf`, `scalblnl`](#scalbn-scalbnf-scalbnl-scalbln-scalblnf-scalblnl) | `x` vezes `FLT_RADIX` elevado a `n` |
| [`ilogb`, `ilogbf`, `ilogbl`](#ilogb-ilogbf-ilogbl) | o expoente, como `int` |
| [`logb`, `logbf`, `logbl`](#logb-logbf-logbl) | o expoente, como ponto flutuante |
| [`nextafter`, `nextafterf`, `nextafterl`, `nexttoward`, `nexttowardf`, `nexttowardl`](#nextafter-nextafterf-nextafterl-nexttoward-nexttowardf-nexttowardl) | o próximo número representável numa direção |
| [`copysign`, `copysignf`, `copysignl`](#copysign-copysignf-copysignl) | a magnitude de um com o sinal do outro |
| [`fpclassify`](#fpclassify) | a categoria do número |
| [`isfinite`](#isfinite) | finito? |
| [`isinf`](#isinf) | infinito? |
| [`isnan`](#isnan) | NaN? |
| [`isnormal`](#isnormal) | normal? |
| [`signbit`](#signbit) | negativo (pelo bit de sinal)? |
| [`isgreater`](#isgreater) | `x > y`, sem exceção de ponto flutuante |
| [`isgreaterequal`](#isgreaterequal) | `x >= y`, sem exceção de ponto flutuante |
| [`isless`](#isless) | `x < y`, sem exceção de ponto flutuante |
| [`islessequal`](#islessequal) | `x <= y`, sem exceção de ponto flutuante |
| [`islessgreater`](#islessgreater) | `x < y` ou `x > y`, sem exceção de ponto flutuante |
| [`isunordered`](#isunordered) | algum dos dois é NaN? |

| Tipo ou constante | O que é |
|---|---|
| [`float_t`, `double_t`](#float_t-double_t) | os tipos em que o sistema faz a conta do `float` e do `double` |
| [`HUGE_VAL`, `HUGE_VALF`, `HUGE_VALL`](#huge_val-huge_valf-huge_vall) | o valor que as funções devolvem no estouro |
| [`INFINITY`](#infinity) | o infinito positivo |
| [`NAN`](#nan) | um NaN silencioso |
| [`FP_NORMAL`, `FP_SUBNORMAL`, `FP_ZERO`, `FP_INFINITE`, `FP_NAN`](#fp_normal-fp_subnormal-fp_zero-fp_infinite-fp_nan) | as categorias do `fpclassify` |
| [`FP_ILOGB0`, `FP_ILOGBNAN`](#fp_ilogb0-fp_ilogbnan) | o que o `ilogb` devolve pra zero e pra NaN |
| [`FP_FAST_FMA`, `FP_FAST_FMAF`, `FP_FAST_FMAL`](#fp_fast_fma-fp_fast_fmaf-fp_fast_fmal) | a `fma` é mais rápida que `x * y + z`? |
| [`math_errhandling`, `MATH_ERRNO`, `MATH_ERREXCEPT`](#math_errhandling-math_errno-math_errexcept) | como as funções avisam o erro |

## Na Jinga

- Os nomes do `math.h` valem com `import <math.h>` no topo do arquivo, igual
  ao `#include <math.h>` do C. Sem o import, o nome é `NameError`, com a dica
  do import que falta:

  ```ps
  post(sqrt(2.0))    # NameError: falta o import <math.h>
  ```

  ```saida
  ```

  ```erro
  NameError: name 'sqrt' is not defined. Did you forget to import <math.h>?
  ```

  Com o import:

  ```ps
  import <math.h>

  post(sqrt(2.0))
  ```

  ```saida
  1.4142135623730951
  ```

  Nome de outro cabeçalho pede o import dele, um por linha: o `printf` dos
  exemplos é do `<stdio.h>`; o `errno`, o `EDOM` e o `ERANGE` são do
  `<errno.h>`; o `INT_MAX` é do `<limits.h>`.
- O `flo` da Jinga é o `double` do C. As versões com `f` (`float`) e com `l`
  (`long double`) recebem e devolvem `flo`: o argumento vira `float` ou
  `long double` na chamada, e o resultado volta como `flo`. O do `float` volta
  com a precisão do `float` (`sqrtf(2.0)` dá `1.4142135381698608`); o do
  `long double` perde o que passa do `double`.
- Um `int` entra onde o C pede ponto flutuante (`sqrt(16)` dá `4.0`). Um
  `flo` onde o C pede inteiro (o `exp` do `ldexp`, o `n` do `scalbn`) é
  `TypeError` antes de rodar. O resultado inteiro (`ilogb`, `lround`,
  `llrint`...) chega como `int`.
- As macros de classificação e de comparação (`fpclassify`, `isnan`,
  `isgreater`...) recebem o número como `double`: o padrão aceita qualquer
  tipo de ponto flutuante, e na Jinga o número chega como `flo`. Elas
  devolvem **diferente de zero** pra sim e **zero** pra não, e qual número
  sai no "sim" depende do sistema (`isinf(-INFINITY)` dá `-1` no Linux e `1`
  no macOS): compare com `!= 0`. O `fpclassify` devolve uma das constantes
  `FP_*`, cujo valor muda de sistema pra sistema: compare com o nome.
- `frexp`, `modf` e `remquo` devolvem um segundo resultado por ponteiro:
  passe o endereço de uma variável do tipo que o protótipo pede. `int e` pro
  `int *exp` (`frexp(x, &e)`), `flo` ou `double` pro `double *iptr`, e
  `float` ou `long double` pras versões com `f` e com `l`. Ponteiro de outro
  tipo é `TypeError`.
- Onde o nome existe nos dois, vale o da Jinga, com ou sem o import:
  `round` e `pow` são os embutidos da Jinga ([`round`](../builtins/round/round.md),
  [`pow`](../builtins/pow/pow.md)). O `round` do C fica pelos outros nomes
  dele (`roundf`, `roundl`, `lround`, `llround`...), e o `pow` do C pelo
  `powf` e pelo `powl`; as seções [`round`](#round-roundf-roundl-lround-lroundf-lroundl-llround-llroundf-llroundl)
  e [`pow`](#pow-powf-powl) mostram a diferença.
- A conta é a da Jinga, com o valor já lido: `sqrt(2.0) * 2.0` multiplica na
  Jinga. Na Jinga, `1.0 / 0.0` levanta `ZeroDivisionError` (no C dá
  infinito); o infinito e o NaN vêm das constantes (`INFINITY`, `NAN`,
  `HUGE_VAL`), do `flo("inf")` e das funções daqui. O `%` com `flo` segue o
  sinal do divisor (`-7.5 % 2.0` dá `0.5`); o `fmod` segue o do dividendo.
- O C não levanta exceção: no erro, a função devolve um valor combinado (NaN,
  `±HUGE_VAL`, zero) e avisa do jeito que o
  [`math_errhandling`](#math_errhandling-math_errno-math_errexcept) diz. Com
  `MATH_ERRNO`, o `errno` recebe `EDOM` no erro de domínio (o argumento está
  fora de onde a função é definida, como `sqrt(-1.0)`) e `ERANGE` no erro de
  polo (o resultado exato é infinito, como `log(0.0)`) e no de faixa (o
  resultado não cabe, como `exp(1000.0)`). No Linux (glibc) o
  `math_errhandling` é `MATH_ERRNO | MATH_ERREXCEPT`, e o `errno` é escrito;
  no macOS é só `MATH_ERREXCEPT`: a biblioteca não toca no `errno`, e o erro
  aparece no valor devolvido e na exceção de ponto flutuante. O motor guarda
  o `errno` logo depois de cada chamada: zere antes (`errno = 0`) e leia
  depois.
- As exceções de ponto flutuante (`FE_INVALID`, `FE_DIVBYZERO`,
  `FE_OVERFLOW`, `FE_UNDERFLOW`, `FE_INEXACT`) e o modo de arredondamento
  são do `fenv.h`, que é da segunda parte da biblioteca do C e ainda não está
  na Jinga. Esta página diz quando o C levanta cada exceção, mas por enquanto
  não há como lê-las. As funções que usam "o modo de arredondamento atual"
  (`rint`, `nearbyint`, `lrint`, `llrint`) usam o de fábrica: o mais próximo,
  com o meio pro par.
- O `post` escreve o NaN como `nan`, sem sinal; o `printf` escreve o sinal do
  NaN do jeito do sistema (`-nan` na glibc pro NaN com o bit de sinal, que é o
  que `sqrt(-1.0)` dá no x86-64; `nan` no macOS). Os exemplos mostram o NaN
  pelo `post` ou pelo `isnan`.

## Operações básicas

### `fabs`, `fabsf`, `fabsl`

```c
double fabs(double x);
float fabsf(float x);
long double fabsl(long double x);
```

Calcula o valor absoluto do número de ponto flutuante `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o valor absoluto de `x` (|x|). O valor é exato e não depende do
modo de arredondamento.

**Erros:** nenhum dos de `math_errhandling`. Com o ponto flutuante IEEE (IEC
60559):

- `x` é ±0: devolve +0;
- `x` é ±∞: devolve +∞;
- `x` é NaN: devolve NaN.

```ps
import <math.h>

post(fabs(-3.5), fabs(3.5), fabs(-0.0), fabs(-INFINITY))
post(fabsf(-2.25), fabsl(-7.0))
```

```saida
3.5 3.5 0.0 inf
2.25 7.0
```

### `fmod`, `fmodf`, `fmodl`

```c
double fmod(double x, double y);
float fmodf(float x, float y);
long double fmodl(long double x, long double y);
```

Calcula o resto de ponto flutuante da divisão `x / y`: exatamente o valor
`x - n * y`, onde `n` é `x / y` com a parte fracionária cortada. O valor
devolvido tem o sinal de `x` e magnitude menor ou igual à de `y`.

Na Jinga, o `%` com `flo` segue o sinal do divisor, como no Python
(`-7.5 % 2.0` dá `0.5`); o `fmod` segue o do dividendo (`fmod(-7.5, 2.0)` dá
`-1.5`).

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** o resto de ponto flutuante de `x / y`, como acima. Com erro de
domínio, um valor definido pela implementação (NaN onde há NaN). Com erro de
faixa por perda, o resultado correto (depois de arredondar).

**Erros:** pode haver erro de domínio (`EDOM`) se `y` é zero. Com o ponto
flutuante IEEE (IEC 60559):

- `x` é ±0 e `y` não é zero: devolve ±0;
- `x` é ±∞ e `y` não é NaN: devolve NaN e levanta `FE_INVALID`;
- `y` é ±0 e `x` não é NaN: devolve NaN e levanta `FE_INVALID`;
- `y` é ±∞ e `x` é finito: devolve `x`;
- algum argumento é NaN: devolve NaN.

O POSIX exige erro de domínio quando `x` é infinito ou `y` é zero.

O `fmod`, e não o `remainder`, serve pra dar a volta num tipo sem sinal:
`fmod(rint(x), 65536.0)`, somado de `65536.0` quando sai negativo, fica em
[-0.0, 65535.0], a faixa do `unsigned short`; o
`remainder(rint(x), 65536.0)` fica em [-32767.0, +32768.0], fora da faixa do
`signed short`.

```ps
import <math.h>

post(fmod(7.5, 2.0), fmod(-7.5, 2.0), fmod(7.5, -2.0))
post(-7.5 % 2.0)
post(fmod(1.0, 0.0), fmodf(5.5, 2.0), fmodl(-5.5, 2.0))
```

```saida
1.5 -1.5 1.5
0.5
nan 1.5 -1.5
```

### `remainder`, `remainderf`, `remainderl`

```c
double remainder(double x, double y);
float remainderf(float x, float y);
long double remainderl(long double x, long double y);
```

Calcula o resto IEEE da divisão `x / y`: exatamente o valor `x - n * y`,
onde `n` é o inteiro mais próximo do valor exato de `x / y`. Quando a
distância |n - x/y| é ½, `n` é o par.

Ao contrário do `fmod`, o valor devolvido não tem garantia de ter o sinal de
`x`. Se o valor devolvido é zero, ele tem o sinal de `x`.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** o resto IEEE de `x / y`, como acima. Com erro de domínio, um
valor definido pela implementação (NaN onde há NaN). Com erro de faixa por
perda, o resultado correto. Se `y` é zero e o erro de domínio não acontece,
zero.

**Erros:** pode haver erro de domínio (`EDOM`) se `y` é zero. Com o ponto
flutuante IEEE (IEC 60559):

- o modo de arredondamento não muda nada;
- `FE_INEXACT` nunca é levantada: o resultado é sempre exato;
- `x` é ±∞ e `y` não é NaN: devolve NaN e levanta `FE_INVALID`;
- `y` é ±0 e `x` não é NaN: devolve NaN e levanta `FE_INVALID`;
- algum argumento é NaN: devolve NaN.

O POSIX exige erro de domínio quando `x` é infinito ou `y` é zero.

```ps
import <math.h>

# 7.5 / 2 = 3.75: o n é 4, e o resto sai negativo
post(remainder(7.5, 2.0), remainder(-7.5, 2.0))
# no meio (2.5 e 3.5), o n é o par: 2 e 4
post(remainder(5.0, 2.0), remainder(7.0, 2.0))
post(remainderf(5.0, 3.0), remainderl(8.0, 3.0))
```

```saida
-0.5 0.5
1.0 -1.0
-1.0 -1.0
```

### `remquo`, `remquof`, `remquol`

```c
double remquo(double x, double y, int *quo);
float remquof(float x, float y, int *quo);
long double remquol(long double x, long double y, int *quo);
```

Calcula o resto de ponto flutuante da divisão `x / y`, como o `remainder`.
Além disso, guarda em `*quo` o sinal de `x / y` e pelo menos os três últimos
bits do quociente: o bastante pra saber o octante do resultado dentro de um
período.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |
| `quo` | ponteiro pra um `int`, que recebe o sinal e alguns bits de `x / y` |

**Retorno:** o mesmo resto do `remainder`. Em `*quo` fica um valor com o
sinal de `x / y` e magnitude congruente, módulo 2^n, à magnitude do quociente
inteiro de `x / y`, com `n` definido pela implementação e maior ou igual a 3.
Se `y` é zero, o valor guardado em `*quo` não é especificado. Com erro de
domínio, um valor definido pela implementação (NaN onde há NaN). Com erro de
faixa por perda, o resultado correto, se o sistema tem subnormais. Se `y` é
zero e o erro de domínio não acontece, zero.

**Erros:** pode haver erro de domínio (`EDOM`) se `y` é zero. Com o ponto
flutuante IEEE (IEC 60559):

- o modo de arredondamento não muda nada;
- `FE_INEXACT` nunca é levantada;
- `x` é ±∞ e `y` não é NaN: devolve NaN e levanta `FE_INVALID`;
- `y` é ±0 e `x` não é NaN: devolve NaN e levanta `FE_INVALID`;
- `x` ou `y` é NaN: devolve NaN.

O POSIX exige erro de domínio quando `x` é infinito ou `y` é zero.

A função serve pras funções periódicas cujo período é exato em ponto
flutuante: pra calcular sin(πx) com `x` muito grande, chamar o `sin` direto
pode dar um erro grande; reduzindo antes o argumento com o `remquo`, os bits
de baixo do quociente dão o sinal e o octante do resultado dentro do período,
e o resto dá o valor com precisão. Em alguns processadores a operação é do
hardware (nos da Intel, o `FPREM1` deixa exatamente 3 bits de precisão no
quociente quando termina).

Na Jinga, o `quo` é o endereço de uma variável `int` (`int q = 0` e `&q`).
Só os três bits de baixo são garantidos: com o quociente acima de 7, o valor
guardado muda de sistema pra sistema (`remquo(100.0, 3.0, &q)` guarda `33`
no macOS e `1` na glibc).

```ps
import <math.h>

int q = 0
flo r = remquo(10.0, 3.0, &q)
post(r, q)
r = remquo(-10.0, 3.0, &q)
post(r, q)
r = remquof(7.0, 2.0, &q)
post(r, q)
r = remquol(5.0, 2.0, &q)
post(r, q)
```

```saida
1.0 3
-1.0 -3
-1.0 4
1.0 2
```

### `fma`, `fmaf`, `fmal`

```c
double fma(double x, double y, double z);
float fmaf(float x, float y, float z);
long double fmal(long double x, long double y, long double z);
```

Calcula `(x * y) + z` como se fosse com precisão infinita, arredondando uma
vez só pro tipo do resultado.

Quando o sistema define as constantes `FP_FAST_FMA`, `FP_FAST_FMAF` e
`FP_FAST_FMAL`, a `fma`, a `fmaf` e a `fmal` são mais rápidas (além de mais
precisas) que a expressão `x * y + z` com `double`, `float` e `long double`
([`FP_FAST_FMA`](#fp_fast_fma-fp_fast_fmaf-fp_fast_fmal)).

| Parâmetro | O que é |
|---|---|
| `x`, `y`, `z` | números de ponto flutuante |

**Retorno:** o valor de `(x * y) + z`, como se calculado com precisão
infinita e arredondado uma vez pro tipo do resultado (ou calculado como uma
única operação de ponto flutuante de três termos). Com erro de faixa por
estouro, `±HUGE_VAL`, `±HUGE_VALF` ou `±HUGE_VALL`. Com erro de faixa por
perda, o valor correto (depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `x` é zero e `y` é infinito, ou `x` é infinito e `y` é zero, e `z` não é
  NaN: devolve NaN e levanta `FE_INVALID`;
- o mesmo, com `z` NaN: devolve NaN, e `FE_INVALID` pode ser levantada;
- `x * y` é um infinito exato e `z` é um infinito de sinal oposto: devolve
  NaN e levanta `FE_INVALID`;
- `x` ou `y` é NaN: devolve NaN;
- `z` é NaN e `x * y` não é 0 × ∞ nem ∞ × 0: devolve NaN (sem
  `FE_INVALID`).

A operação costuma ser uma instrução do processador, a multiplicação e soma
fundidas; o processador que a tem deveria definir as `FP_FAST_FMA*`, mas
muitas implementações usam a instrução mesmo sem defini-las. O POSIX diz que
`x * y` inválido com `z` NaN é erro de domínio. Pela precisão infinita no
meio da conta, a `fma` é a base de outras operações corretamente
arredondadas, como o `sqrt` e até a divisão (onde o processador não a tem,
como no Itanium). Na Jinga, `x * y + z` são duas operações, com dois
arredondamentos.

```ps
import <math.h>

post(fma(2.0, 3.0, 4.0))
# 0.1 * 10 arredonda pra 1.0; a fma mostra o que esse arredondamento escondeu
flo x = 0.1
post(fma(x, 10.0, -1.0), x * 10.0 - 1.0)
post(fmaf(1.5, 2.0, 0.25), fmal(2.0, 2.0, 2.0))
```

```saida
10.0
5.551115123125783e-17 0.0
3.25 6.0
```

### `fmax`, `fmaxf`, `fmaxl`

```c
double fmax(double x, double y);
float fmaxf(float x, float y);
long double fmaxl(long double x, long double y);
```

Devolve o maior de dois números de ponto flutuante, tratando o NaN como dado
que falta: entre um NaN e um número, fica o número.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** o maior dos dois. O valor é exato e não depende do modo de
arredondamento.

**Erros:** nenhum dos de `math_errhandling`. Com o ponto flutuante IEEE (IEC
60559):

- se um dos dois é NaN, devolve o outro;
- só devolve NaN se os dois são NaN.

A função não precisa ligar pro sinal do zero, mas há implementações que, com
+0 e -0, devolvem +0.

```ps
import <math.h>

post(fmax(2.0, 3.5), fmax(-1.0, NAN), fmax(NAN, NAN))
post(fmaxf(1.0, 2.0), fmaxl(-1.0, -2.0))
```

```saida
3.5 -1.0 nan
2.0 -1.0
```

### `fmin`, `fminf`, `fminl`

```c
double fmin(double x, double y);
float fminf(float x, float y);
long double fminl(long double x, long double y);
```

Devolve o menor de dois números de ponto flutuante, tratando o NaN como dado
que falta: entre um NaN e um número, fica o número.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** o menor dos dois. O valor é exato e não depende do modo de
arredondamento.

**Erros:** nenhum dos de `math_errhandling`. Com o ponto flutuante IEEE (IEC
60559):

- se um dos dois é NaN, devolve o outro;
- só devolve NaN se os dois são NaN.

A função não precisa ligar pro sinal do zero, mas há implementações que, com
+0 e -0, devolvem -0.

```ps
import <math.h>

post(fmin(2.0, 3.5), fmin(NAN, 4.0))
post(fminf(1.0, 2.0), fminl(-1.0, -2.0))
```

```saida
2.0 4.0
1.0 -2.0
```

### `fdim`, `fdimf`, `fdiml`

```c
double fdim(double x, double y);
float fdimf(float x, float y);
long double fdiml(long double x, long double y);
```

Devolve a diferença positiva entre `x` e `y`: se `x > y`, `x - y`; senão
(`x <= y`), +0.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** a diferença positiva entre `x` e `y`. Com erro de faixa por
estouro, `+HUGE_VAL`, `+HUGE_VALF` ou `+HUGE_VALL`. Com erro de faixa por
perda, o valor correto (depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- algum argumento é NaN: devolve NaN.

Equivale a `fmax(x - y, 0)`, fora o que se exige com o NaN.

```ps
import <math.h>

post(fdim(5.0, 3.0), fdim(3.0, 5.0), fdim(4.0, -1.5))
post(fdimf(1.0, 0.5), fdiml(0.5, 1.0))
```

```saida
2.0 0.0 5.5
0.5 0.0
```

### `nan`, `nanf`, `nanl`

```c
double nan(const char *tagp);
float nanf(const char *tagp);
long double nanl(const char *tagp);
```

Converte o texto `tagp`, cujo sentido é definido pela implementação, no NaN
silencioso correspondente, como se chamasse a função de leitura do tipo
(`strtod` no `nan`, `strtof` no `nanf`, `strtold` no `nanl`):

- `nan("seq")`, com `seq` feita de dígitos, letras latinas e `_`, é o mesmo
  que `strtod("NAN(seq)", NULL)`;
- `nan("")` é o mesmo que `strtod("NAN()", NULL)`;
- `nan("texto")`, com um texto que não é nem `seq` nem vazio, é o mesmo que
  `strtod("NAN", NULL)`.

| Parâmetro | O que é |
|---|---|
| `tagp` | o texto que identifica o conteúdo do NaN |

**Retorno:** o NaN silencioso que corresponde ao texto `tagp`, ou zero se a
implementação não tem NaN silencioso. Quem tem o ponto flutuante IEEE (IEC
60559) tem o NaN silencioso.

**Erros:** nenhum dos de `math_errhandling`.

Na Jinga, o `tagp` aceita um `str`, copiado pra chamada.

```ps
import <math.h>

flo n = nan("")
post(n, isnan(n) != 0, n == n)
post(isnan(nanf("1")) != 0, isnan(nanl("")) != 0)
```

```saida
nan True False
True True
```

## Exponenciais e logaritmos

### `exp`, `expf`, `expl`

```c
double exp(double x);
float expf(float x);
long double expl(long double x);
```

Calcula *e* (o número de Euler, `2.7182818...`) elevado a `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** *e* elevado a `x`. Com erro de faixa por estouro, `+HUGE_VAL`,
`+HUGE_VALF` ou `+HUGE_VALL`. Com erro de faixa por perda, o resultado
correto (depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve 1;
- `x` é -∞: devolve +0;
- `x` é +∞: devolve +∞;
- `x` é NaN: devolve NaN.

Com o `double` do IEEE, o estouro é garantido se 709.8 < `x`, e a perda se
`x` < -708.4.

```ps
import <math.h>
import <stdio.h>

printf("%.6f %.6f\n", exp(1.0), exp(-1.0))
post(exp(0.0), exp(-INFINITY), exp(1000.0))
printf("%.6f %.6f\n", expf(1.0), expl(2.0))
```

```saida
2.718282 0.367879
1.0 0.0 inf
2.718282 7.389056
```

### `exp2`, `exp2f`, `exp2l`

```c
double exp2(double x);
float exp2f(float x);
long double exp2l(long double x);
```

Calcula 2 elevado a `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** 2 elevado a `x`. Com erro de faixa por estouro, `+HUGE_VAL`,
`+HUGE_VALF` ou `+HUGE_VALL`. Com erro de faixa por perda, o resultado
correto (depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve 1;
- `x` é -∞: devolve +0;
- `x` é +∞: devolve +∞;
- `x` é NaN: devolve NaN.

```ps
import <math.h>

post(exp2(10.0), exp2(-1.0), exp2(1024.0))
post(exp2f(3.0), exp2l(-2.0))
```

```saida
1024.0 0.5 inf
8.0 0.25
```

### `expm1`, `expm1f`, `expm1l`

```c
double expm1(double x);
float expm1f(float x);
long double expm1l(long double x);
```

Calcula *e* (o número de Euler, `2.7182818...`) elevado a `x`, menos `1.0`.
É mais precisa que a expressão `exp(x) - 1.0` quando `x` é perto de zero.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** *e* elevado a `x`, menos 1. Com erro de faixa por estouro,
`+HUGE_VAL`, `+HUGE_VALF` ou `+HUGE_VALL`. Com erro de faixa por perda, o
resultado correto (depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve `x` sem mudar;
- `x` é -∞: devolve -1;
- `x` é +∞: devolve +∞;
- `x` é NaN: devolve NaN.

As funções `expm1` e `log1p` servem pra conta financeira, como a de juros
diários pequenos: (1 + x)^n - 1 se escreve `expm1(n * log1p(x))`. Elas também
deixam mais fácil escrever as hiperbólicas inversas com precisão. Com o
`double` do IEEE, o estouro é garantido se 709.8 < `x`.

```ps
import <math.h>
import <stdio.h>

flo x = 1.0e-10
# exp(x) - 1.0 perde os dígitos na subtração; o expm1 não
printf("%.10e %.10e\n", expm1(x), exp(x) - 1.0)
# juros de 0,01% ao dia, por 365 dias
printf("%.6f\n", expm1(365.0 * log1p(0.0001)))
post(expm1(0.0), expm1(-INFINITY), expm1f(0.0), expm1l(-0.0))
```

```saida
1.0000000001e-10 1.0000000827e-10
0.037172
0.0 -1.0 0.0 -0.0
```

### `log`, `logf`, `logl`

```c
double log(double x);
float logf(float x);
long double logl(long double x);
```

Calcula o logaritmo natural (na base *e*) de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o logaritmo natural de `x` (ln(x)). Com erro de domínio, um
valor definido pela implementação (NaN onde há NaN). Com erro de polo,
`-HUGE_VAL`, `-HUGE_VALF` ou `-HUGE_VALL`.

**Erros:** erro de domínio (`EDOM`) se `x` é menor que zero; pode haver erro
de polo (`ERANGE`) se `x` é zero. Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve -∞ e levanta `FE_DIVBYZERO`;
- `x` é 1: devolve +0;
- `x` é negativo: devolve NaN e levanta `FE_INVALID`;
- `x` é +∞: devolve +∞;
- `x` é NaN: devolve NaN.

```ps
import <math.h>
import <stdio.h>

printf("%.6f %.6f\n", log(10.0), log(2.0))
post(log(1.0), log(0.0), log(-1.0), log(INFINITY))
printf("%.6f %.6f\n", logf(10.0), logl(10.0))
```

```saida
2.302585 0.693147
0.0 -inf nan inf
2.302585 2.302585
```

### `log10`, `log10f`, `log10l`

```c
double log10(double x);
float log10f(float x);
long double log10l(long double x);
```

Calcula o logaritmo comum (na base 10) de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o logaritmo na base 10 de `x` (log10(x) ou lg(x)). Com erro de
domínio, um valor definido pela implementação (NaN onde há NaN). Com erro de
polo, `-HUGE_VAL`, `-HUGE_VALF` ou `-HUGE_VALL`.

**Erros:** erro de domínio (`EDOM`) se `x` é menor que zero; pode haver erro
de polo (`ERANGE`) se `x` é zero. Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve -∞ e levanta `FE_DIVBYZERO`;
- `x` é 1: devolve +0;
- `x` é negativo: devolve NaN e levanta `FE_INVALID`;
- `x` é +∞: devolve +∞;
- `x` é NaN: devolve NaN.

```ps
import <math.h>
import <stdio.h>

post(log10(1000.0), log10(1.0), log10(0.0))
printf("%.6f\n", log10(2.0))
post(log10f(100.0), log10l(10.0))
```

```saida
3.0 0.0 -inf
0.301030
2.0 1.0
```

### `log2`, `log2f`, `log2l`

```c
double log2(double x);
float log2f(float x);
long double log2l(long double x);
```

Calcula o logaritmo na base 2 de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o logaritmo na base 2 de `x` (log2(x) ou lb(x)). Com erro de
domínio, um valor definido pela implementação (NaN onde há NaN). Com erro de
polo, `-HUGE_VAL`, `-HUGE_VALF` ou `-HUGE_VALL`.

**Erros:** erro de domínio (`EDOM`) se `x` é menor que zero; pode haver erro
de polo (`ERANGE`) se `x` é zero. Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve -∞ e levanta `FE_DIVBYZERO`;
- `x` é 1: devolve +0;
- `x` é negativo: devolve NaN e levanta `FE_INVALID`;
- `x` é +∞: devolve +∞;
- `x` é NaN: devolve NaN.

Com `x` inteiro, a parte inteira do resultado é a posição (contada do zero)
do bit 1 mais alto de `x`.

```ps
import <math.h>

post(log2(8.0), log2(1024.0), log2(0.5))
# 1000 é 0b1111101000: o bit mais alto está na posição 9
post(int(log2(1000.0)))
post(log2f(16.0), log2l(0.25))
```

```saida
3.0 10.0 -1.0
9
4.0 -2.0
```

### `log1p`, `log1pf`, `log1pl`

```c
double log1p(double x);
float log1pf(float x);
long double log1pl(long double x);
```

Calcula o logaritmo natural (na base *e*) de `1 + x`. É mais precisa que a
expressão `log(1 + x)` quando `x` é perto de zero.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** ln(1 + x). Com erro de domínio, um valor definido pela
implementação (NaN onde há NaN). Com erro de polo, `-HUGE_VAL`, `-HUGE_VALF`
ou `-HUGE_VALL`. Com erro de faixa por perda, o resultado correto (depois de
arredondar).

**Erros:** erro de domínio (`EDOM`) se `x` é menor que -1; pode haver erro
de polo (`ERANGE`) se `x` é -1. Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve `x` sem mudar;
- `x` é -1: devolve -∞ e levanta `FE_DIVBYZERO`;
- `x` é menor que -1: devolve NaN e levanta `FE_INVALID`;
- `x` é +∞: devolve +∞;
- `x` é NaN: devolve NaN.

As funções `expm1` e `log1p` servem pra conta financeira com taxa pequena:
(1 + x)^n - 1 se escreve `expm1(n * log1p(x))` (o exemplo está no
[`expm1`](#expm1-expm1f-expm1l)). Elas também deixam mais fácil escrever as
hiperbólicas inversas com precisão.

```ps
import <math.h>
import <stdio.h>

flo x = 1.0e-10
# 1.0 + x já arredonda; o log1p usa o x inteiro
printf("%.10e %.10e\n", log1p(x), log(1.0 + x))
post(log1p(0.0), log1p(-1.0), log1p(-2.0))
post(log1pf(0.0), log1pl(-0.0))
```

```saida
9.9999999995e-11 1.0000000827e-10
0.0 -inf nan
0.0 -0.0
```

## Potências e raízes

### `pow`, `powf`, `powl`

```c
double pow(double x, double y);
float powf(float x, float y);
long double powl(long double x, long double y);
```

Calcula `x` elevado a `y`.

Na Jinga, o nome `pow` é o embutido da Jinga, mesmo com o `<math.h>`
importado ([`pow`](../builtins/pow/pow.md)): com dois `int` (e o expoente
não negativo) a conta é exata e o resultado é `int` (`pow(2, 10)` dá
`1024`), e onde o C devolve o valor do erro ele levanta exceção
(`pow(0.0, -1.0)` é `ZeroDivisionError`). O `pow` do C fica pelo `powl`, que
recebe os mesmos argumentos sem perda (o `long double` guarda todo `double`)
e devolve `flo`, e pelo `powf`, que faz a conta em `float`.

| Parâmetro | O que é |
|---|---|
| `x` | a base, número de ponto flutuante |
| `y` | o expoente, número de ponto flutuante |

**Retorno:** `x` elevado a `y`. Com erro de domínio, um valor definido pela
implementação (NaN onde há NaN). Com erro de polo ou de faixa por estouro,
`±HUGE_VAL`, `±HUGE_VALF` ou `±HUGE_VALL`. Com erro de faixa por perda, o
resultado correto (depois de arredondar).

**Erros:**

- `x` finito e negativo com `y` finito e não inteiro: erro de domínio
  (`EDOM`), e pode haver erro de faixa;
- `x` zero e `y` zero: pode haver erro de domínio;
- `x` zero e `y` negativo: pode haver erro de domínio ou de polo (`ERANGE`).

Com o ponto flutuante IEEE (IEC 60559):

- `pow(+0, y)`, com `y` inteiro ímpar negativo: devolve +∞ e levanta
  `FE_DIVBYZERO`;
- `pow(-0, y)`, com `y` inteiro ímpar negativo: devolve -∞ e levanta
  `FE_DIVBYZERO`;
- `pow(±0, y)`, com `y` negativo, finito e inteiro par ou não inteiro:
  devolve +∞ e levanta `FE_DIVBYZERO`;
- `pow(±0, -∞)`: devolve +∞ e pode levantar `FE_DIVBYZERO`;
- `pow(+0, y)`, com `y` inteiro ímpar positivo: devolve +0;
- `pow(-0, y)`, com `y` inteiro ímpar positivo: devolve -0;
- `pow(±0, y)`, com `y` positivo não inteiro ou inteiro par positivo:
  devolve +0;
- `pow(-1, ±∞)`: devolve 1;
- `pow(+1, y)`: devolve 1 pra qualquer `y`, mesmo NaN;
- `pow(x, ±0)`: devolve 1 pra qualquer `x`, mesmo NaN;
- `pow(x, y)`, com `x` finito e negativo e `y` finito e não inteiro: devolve
  NaN e levanta `FE_INVALID`;
- `pow(x, -∞)`: devolve +∞ se |x| < 1, e +0 se |x| > 1;
- `pow(x, +∞)`: devolve +0 se |x| < 1, e +∞ se |x| > 1;
- `pow(-∞, y)`: devolve -0 se `y` é inteiro ímpar negativo; +0 se `y` é
  negativo não inteiro ou inteiro par negativo; -∞ se `y` é inteiro ímpar
  positivo; +∞ se `y` é positivo não inteiro ou inteiro par positivo;
- `pow(+∞, y)`: devolve +0 com `y` negativo, e +∞ com `y` positivo;
- fora os casos acima, se algum argumento é NaN, devolve NaN.

O `pow` não tira raiz de número negativo; pra raiz cúbica, que é o caso
comum, há o [`cbrt`](#cbrt-cbrtf-cbrtl).

```ps
import <math.h>

# o pow da Jinga: int com int dá int
post(pow(2, 10), powl(2.0, 10.0), powf(1.5, 2.0))
# no erro de polo, o do C devolve infinito; o da Jinga levanta exceção
post(powl(0.0, -1.0), powl(-0.0, -1.0))
try {
    post(pow(0.0, -1.0))
} catch (ZeroDivisionError e) {
    post("o pow da Jinga levantou ZeroDivisionError")
}
post(powl(-8.0, 1.0 / 3.0), powl(-2.0, 3.0), powl(1.0, NAN), powl(NAN, 0.0))
```

```saida
1024 1024.0 2.25
inf -inf
o pow da Jinga levantou ZeroDivisionError
nan -8.0 1.0 1.0
```

### `sqrt`, `sqrtf`, `sqrtl`

```c
double sqrt(double x);
float sqrtf(float x);
long double sqrtl(long double x);
```

Calcula a raiz quadrada de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** a raiz quadrada de `x` (√x). Com erro de domínio, um valor
definido pela implementação (NaN onde há NaN). Com erro de faixa por perda,
o resultado correto (depois de arredondar).

**Erros:** erro de domínio (`EDOM`) se `x` é menor que zero. Com o ponto
flutuante IEEE (IEC 60559):

- `x` é menor que -0: levanta `FE_INVALID` e devolve NaN;
- `x` é +∞ ou ±0: devolve `x` sem mudar;
- `x` é NaN: devolve NaN.

O IEEE exige que o `sqrt` seja corretamente arredondado a partir do
resultado exato; em particular, quando o resultado cabe no tipo, ele sai
exato. As únicas outras operações com essa exigência são as aritméticas e a
`fma`; as outras funções, o `pow` entre elas, não têm.

```ps
import <math.h>
import <stdio.h>

printf("%.6f\n", sqrt(2.0))
# o sqrtf faz a conta em float: a raiz volta com a precisão do float
post(sqrt(2.0), sqrtf(2.0))
post(sqrt(16.0), sqrt(-0.0), sqrt(-1.0), sqrt(INFINITY), sqrtl(6.25))
```

```saida
1.414214
1.4142135623730951 1.4142135381698608
4.0 -0.0 nan inf 2.5
```

### `cbrt`, `cbrtf`, `cbrtl`

```c
double cbrt(double x);
float cbrtf(float x);
long double cbrtl(long double x);
```

Calcula a raiz cúbica de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** a raiz cúbica de `x` (∛x). Com erro de faixa por perda, o
resultado correto (depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0 ou ±∞: devolve `x` sem mudar;
- `x` é NaN: devolve NaN.

`cbrt(x)` não é o mesmo que `pow(x, 1.0 / 3)`: o racional 1/3 em geral não
é igual a `1.0 / 3`, e o `pow` não eleva base negativa a expoente
fracionário. Além disso, o `cbrt(x)` costuma dar resultado mais preciso que
`pow(x, 1.0 / 3)`.

```ps
import <math.h>
import <stdio.h>

printf("%.6f %.6f %.6f\n", cbrt(27.0), cbrt(-8.0), cbrtf(64.0))
# o pow não tira raiz de número negativo; o cbrt tira
post(powl(-8.0, 1.0 / 3.0))
printf("%.6f\n", cbrtl(2.0))
post(cbrt(-0.0), cbrt(-INFINITY))
```

```saida
3.000000 -2.000000 4.000000
nan
1.259921
-0.0 -inf
```

### `hypot`, `hypotf`, `hypotl`

```c
double hypot(double x, double y);
float hypotf(float x, float y);
long double hypotl(long double x, long double y);
```

Calcula a raiz quadrada da soma dos quadrados de `x` e `y`, sem estouro nem
perda indevidos nas etapas do meio da conta.

O valor é o comprimento da hipotenusa de um triângulo retângulo de catetos
`x` e `y`, ou a distância do ponto `(x, y)` à origem `(0, 0)`, ou o módulo do
número complexo `x + iy`.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** a hipotenusa de um triângulo retângulo, √(x² + y²). Com erro de
faixa por estouro, `+HUGE_VAL`, `+HUGE_VALF` ou `+HUGE_VALL`. Com erro de
faixa por perda, o resultado correto (depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `hypot(x, y)`, `hypot(y, x)` e `hypot(x, -y)` são iguais;
- se um dos argumentos é ±0, o `hypot` é o `fabs` do outro;
- se um dos argumentos é ±∞, o `hypot` devolve +∞ mesmo que o outro seja
  NaN;
- fora isso, se algum argumento é NaN, devolve NaN.

As implementações costumam garantir erro menor que 1 ulp (uma unidade na
última casa). O POSIX diz que só pode haver perda quando os dois argumentos
são subnormais e o resultado correto também é subnormal, o que proíbe a
implementação ingênua. `hypot(INFINITY, NAN)` devolve +∞, mas
`sqrt(INFINITY * INFINITY + NAN * NAN)` devolve NaN.

```ps
import <math.h>
import <stdio.h>

printf("%.6f\n", hypot(3.0, 4.0))
post(hypotf(5.0, 12.0), hypotl(8.0, 15.0))
# sem estourar no meio: o quadrado de 1e200 já não cabe no double
flo g = 1.0e200
printf("%.6e\n", hypot(g, g))
post(sqrt(g * g + g * g))
post(hypot(INFINITY, NAN), hypot(NAN, 1.0))
```

```saida
5.000000
13.0 17.0
1.414214e+200
inf
inf nan
```

## Trigonométricas

Os ângulos são em radianos. O C11 não tem a constante π (o `M_PI` é do
POSIX, e a tabela do motor segue o C11): os exemplos tiram o π de
`acos(-1.0)`, que devolve o `double` mais perto dele.

### `sin`, `sinf`, `sinl`

```c
double sin(double x);
float sinf(float x);
long double sinl(long double x);
```

Calcula o seno de `x` (medido em radianos).

| Parâmetro | O que é |
|---|---|
| `x` | o ângulo, em radianos |

**Retorno:** o seno de `x` (sin(x)), em [-1, +1]. Com erro de domínio, um
valor definido pela implementação (NaN onde há NaN). Com erro de faixa por
perda, o resultado correto (depois de arredondar).

**Erros:** os do `math_errhandling`. Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve `x` sem mudar;
- `x` é ±∞: devolve NaN e levanta `FE_INVALID`;
- `x` é NaN: devolve NaN.

O C não diz que o argumento infinito é erro de domínio, mas o POSIX diz. O
POSIX também diz que, com perda, a função devolve `x` sem mudar, ou, onde
isso não dá, um valor definido pela implementação não maior que `DBL_MIN`,
`FLT_MIN` e `LDBL_MIN`.

```ps
import <math.h>
import <stdio.h>

flo pi = acos(-1.0)
printf("%.6f %.6f %.6f\n", sin(pi / 6.0), sin(pi / 2.0), sin(-pi / 4.0))
post(sin(0.0), sin(-0.0), sin(INFINITY))
printf("%.6f %.6f\n", sinf(1.0), sinl(1.0))
```

```saida
0.500000 1.000000 -0.707107
0.0 -0.0 nan
0.841471 0.841471
```

### `cos`, `cosf`, `cosl`

```c
double cos(double x);
float cosf(float x);
long double cosl(long double x);
```

Calcula o cosseno de `x` (medido em radianos).

| Parâmetro | O que é |
|---|---|
| `x` | o ângulo, em radianos |

**Retorno:** o cosseno de `x` (cos(x)), em [-1, +1]. Com erro de domínio, um
valor definido pela implementação (NaN onde há NaN). Com erro de faixa por
perda, o resultado correto (depois de arredondar).

**Erros:** os do `math_errhandling`. Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve 1.0;
- `x` é ±∞: devolve NaN e levanta `FE_INVALID`;
- `x` é NaN: devolve NaN.

O C não diz que o argumento infinito é erro de domínio, mas o POSIX diz.

```ps
import <math.h>
import <stdio.h>

flo pi = acos(-1.0)
printf("%.6f %.6f %.6f\n", cos(pi / 3.0), cos(pi), cos(2.0 * pi))
post(cos(0.0), cos(-0.0), cos(-INFINITY))
printf("%.6f %.6f\n", cosf(1.0), cosl(1.0))
```

```saida
0.500000 -1.000000 1.000000
1.0 1.0 nan
0.540302 0.540302
```

### `tan`, `tanf`, `tanl`

```c
double tan(double x);
float tanf(float x);
long double tanl(long double x);
```

Calcula a tangente de `x` (medido em radianos).

| Parâmetro | O que é |
|---|---|
| `x` | o ângulo, em radianos |

**Retorno:** a tangente de `x` (tan(x)). Com erro de domínio, um valor
definido pela implementação (NaN onde há NaN). Com erro de faixa por perda,
o resultado correto (depois de arredondar).

**Erros:** os do `math_errhandling`. Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve `x` sem mudar;
- `x` é ±∞: devolve NaN e levanta `FE_INVALID`;
- `x` é NaN: devolve NaN.

O C não diz que o argumento infinito é erro de domínio, mas o POSIX diz.

A função tem polos matemáticos em π(1/2 + n), mas nenhuma representação
comum de ponto flutuante guarda π/2 exato: não há argumento que dê erro de
polo.

```ps
import <math.h>
import <stdio.h>

flo pi = acos(-1.0)
printf("%.6f %.6f\n", tan(pi / 4.0), tan(-pi / 4.0))
# perto do polo, mas não nele: o pi / 2 do double não é o pi / 2 de verdade
printf("%.6e\n", tan(pi / 2.0))
post(tan(-0.0), tan(INFINITY))
printf("%.6f %.6f\n", tanf(1.0), tanl(1.0))
```

```saida
1.000000 -1.000000
1.633124e+16
-0.0 nan
1.557408 1.557408
```

### `asin`, `asinf`, `asinl`

```c
double asin(double x);
float asinf(float x);
long double asinl(long double x);
```

Calcula o valor principal do arco seno de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o arco seno de `x` (arcsen(x)), em [-π/2, +π/2]. Com erro de
domínio, um valor definido pela implementação (NaN onde há NaN). Com erro de
faixa por perda, o resultado correto (depois de arredondar).

**Erros:** erro de domínio (`EDOM`) se `x` está fora de [-1.0, 1.0]. Com o
ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve `x` sem mudar;
- |x| > 1: há erro de domínio, e devolve NaN;
- `x` é NaN: devolve NaN.

```ps
import <math.h>
import <stdio.h>

printf("%.6f %.6f\n", asin(1.0), asin(-0.5))
post(asin(-0.0), asin(2.0))
printf("%.6f %.6f\n", asinf(0.5), asinl(1.0))
```

```saida
1.570796 -0.523599
-0.0 nan
0.523599 1.570796
```

### `acos`, `acosf`, `acosl`

```c
double acos(double x);
float acosf(float x);
long double acosl(long double x);
```

Calcula o valor principal do arco cosseno de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o arco cosseno de `x` (arccos(x)), em [0, π]. Com erro de
domínio, um valor definido pela implementação (NaN onde há NaN). Com erro de
faixa por perda, o resultado correto (depois de arredondar).

**Erros:** erro de domínio (`EDOM`) se `x` está fora de [-1.0, 1.0]. Com o
ponto flutuante IEEE (IEC 60559):

- `x` é +1: devolve +0;
- |x| > 1: há erro de domínio, e devolve NaN;
- `x` é NaN: devolve NaN.

```ps
import <math.h>
import <stdio.h>

printf("%.6f %.6f\n", acos(-1.0), acos(0.5))
post(acos(1.0), acos(1.1))
printf("%.6f %.6f\n", acosf(0.0), acosl(-1.0))
```

```saida
3.141593 1.047198
0.0 nan
1.570796 3.141593
```

### `atan`, `atanf`, `atanl`

```c
double atan(double x);
float atanf(float x);
long double atanl(long double x);
```

Calcula o valor principal do arco tangente de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o arco tangente de `x` (arctg(x)), em [-π/2, +π/2] radianos.
Com erro de faixa por perda, o resultado correto (depois de arredondar).

**Erros:** os do `math_errhandling`. Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve `x` sem mudar;
- `x` é +∞: devolve +π/2;
- `x` é -∞: devolve -π/2;
- `x` é NaN: devolve NaN.

O POSIX diz que, com perda, a função devolve `x` sem mudar, ou, onde isso
não dá, um valor definido pela implementação não maior que `DBL_MIN`,
`FLT_MIN` e `LDBL_MIN`.

```ps
import <math.h>
import <stdio.h>

printf("%.6f %.6f\n", 4.0 * atan(1.0), atan(INFINITY))
post(atan(-0.0))
printf("%.6f %.6f\n", atanf(1.0), atanl(-INFINITY))
```

```saida
3.141593 1.570796
-0.0
0.785398 -1.570796
```

### `atan2`, `atan2f`, `atan2l`

```c
double atan2(double y, double x);
float atan2f(float y, float x);
long double atan2l(long double y, long double x);
```

Calcula o arco tangente de `y / x`, usando o sinal dos dois argumentos pra
achar o quadrante.

| Parâmetro | O que é |
|---|---|
| `y` | número de ponto flutuante (a ordenada) |
| `x` | número de ponto flutuante (a abscissa) |

**Retorno:** o arco tangente de `y / x` (arctg(y/x)), em [-π, +π] radianos.
Com erro de domínio, um valor definido pela implementação. Com erro de faixa
por perda, o resultado correto (depois de arredondar).

**Erros:** pode haver erro de domínio (`EDOM`) se `x` e `y` são os dois
zero. Com o ponto flutuante IEEE (IEC 60559):

- `x` e `y` são os dois zero: não há erro de domínio, nem de faixa;
- `y` é zero: não há erro de polo;
- `y` é ±0 e `x` é negativo ou -0: devolve ±π;
- `y` é ±0 e `x` é positivo ou +0: devolve ±0;
- `y` é ±∞ e `x` é finito: devolve ±π/2;
- `y` é ±∞ e `x` é -∞: devolve ±3π/4;
- `y` é ±∞ e `x` é +∞: devolve ±π/4;
- `x` é ±0 e `y` é negativo: devolve -π/2;
- `x` é ±0 e `y` é positivo: devolve +π/2;
- `x` é -∞ e `y` é finito e positivo: devolve +π;
- `x` é -∞ e `y` é finito e negativo: devolve -π;
- `x` é +∞ e `y` é finito e positivo: devolve +0;
- `x` é +∞ e `y` é finito e negativo: devolve -0;
- `x` ou `y` é NaN: devolve NaN.

O POSIX diz que, com perda, a função devolve `y / x`, ou, onde isso não dá,
um valor definido pela implementação não maior que `DBL_MIN`, `FLT_MIN` e
`LDBL_MIN`.

```ps
import <math.h>
import <stdio.h>

# um ponto em cada quadrante
printf("%+.6f %+.6f\n", atan2(1.0, 1.0), atan2(1.0, -1.0))
printf("%+.6f %+.6f\n", atan2(-1.0, -1.0), atan2(-1.0, 1.0))
post(atan2(0.0, 0.0), atan2(-0.0, 1.0))
printf("%.6f %.6f\n", atan2f(1.0, 0.0), atan2l(INFINITY, INFINITY))
```

```saida
+0.785398 +2.356194
-2.356194 -0.785398
0.0 -0.0
1.570796 0.785398
```

## Hiperbólicas

### `sinh`, `sinhf`, `sinhl`

```c
double sinh(double x);
float sinhf(float x);
long double sinhl(long double x);
```

Calcula o seno hiperbólico de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante (um ângulo hiperbólico) |

**Retorno:** o seno hiperbólico de `x` (sinh(x), ou (e^x - e^-x)/2). Com erro
de faixa por estouro, `±HUGE_VAL`, `±HUGE_VALF` ou `±HUGE_VALL`. Com erro de
faixa por perda, o resultado correto (depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0 ou ±∞: devolve `x` sem mudar;
- `x` é NaN: devolve NaN.

O POSIX diz que, com perda, a função devolve `x` sem mudar, ou, onde isso
não dá, um valor definido pela implementação não maior que `DBL_MIN`,
`FLT_MIN` e `LDBL_MIN`.

```ps
import <math.h>
import <stdio.h>

printf("%.6f %.6f\n", sinh(1.0), sinh(-1.0))
post(sinh(-0.0), sinh(1000.0), sinh(-1000.0))
printf("%.6f %.6f\n", sinhf(1.0), sinhl(2.0))
```

```saida
1.175201 -1.175201
-0.0 inf -inf
1.175201 3.626860
```

### `cosh`, `coshf`, `coshl`

```c
double cosh(double x);
float coshf(float x);
long double coshl(long double x);
```

Calcula o cosseno hiperbólico de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante (um ângulo hiperbólico) |

**Retorno:** o cosseno hiperbólico de `x` (cosh(x), ou (e^x + e^-x)/2). Com
erro de faixa por estouro, `+HUGE_VAL`, `+HUGE_VALF` ou `+HUGE_VALL`.

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve 1;
- `x` é ±∞: devolve +∞;
- `x` é NaN: devolve NaN.

Com o `double` do IEEE, o `cosh(x)` estoura se |x| > 710.5.

```ps
import <math.h>
import <stdio.h>

printf("%.6f\n", cosh(1.0))
post(cosh(0.0), cosh(-INFINITY), cosh(1000.0))
printf("%.6f %.6f\n", coshf(1.0), coshl(2.0))
```

```saida
1.543081
1.0 inf inf
1.543081 3.762196
```

### `tanh`, `tanhf`, `tanhl`

```c
double tanh(double x);
float tanhf(float x);
long double tanhl(long double x);
```

Calcula a tangente hiperbólica de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante (um ângulo hiperbólico) |

**Retorno:** a tangente hiperbólica de `x` (tanh(x), ou
(e^x - e^-x)/(e^x + e^-x)). Com erro de faixa por perda, o resultado correto
(depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve ±0;
- `x` é ±∞: devolve ±1;
- `x` é NaN: devolve NaN.

O POSIX diz que, com perda, a função devolve `x` sem mudar, ou, onde isso
não dá, um valor definido pela implementação não maior que `DBL_MIN`,
`FLT_MIN` e `LDBL_MIN`.

```ps
import <math.h>
import <stdio.h>

printf("%.6f %.6f\n", tanh(1.0), tanh(-1.0))
post(tanh(INFINITY), tanh(-INFINITY), tanh(-0.0))
printf("%.6f %.6f\n", tanhf(0.5), tanhl(2.0))
```

```saida
0.761594 -0.761594
1.0 -1.0 -0.0
0.462117 0.964028
```

### `asinh`, `asinhf`, `asinhl`

```c
double asinh(double x);
float asinhf(float x);
long double asinhl(long double x);
```

Calcula o seno hiperbólico inverso de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante (a área de um setor hiperbólico) |

**Retorno:** o seno hiperbólico inverso de `x` (sinh⁻¹(x), ou arsinh(x)).
Com erro de faixa por perda, o resultado correto (depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0 ou ±∞: devolve `x` sem mudar;
- `x` é NaN: devolve NaN.

O padrão do C chama a função de "arco seno hiperbólico", mas as inversas das
hiperbólicas são as funções de área: o argumento delas é a área de um setor
hiperbólico, não um arco. O nome certo é "seno hiperbólico inverso" (o do
POSIX) ou "área do seno hiperbólico".

```ps
import <math.h>
import <stdio.h>

printf("%.6f %.6f\n", asinh(1.0), asinh(-1.0))
post(asinh(0.0), asinh(-INFINITY))
printf("%.6f %.6f\n", asinhf(1.0), asinhl(2.0))
```

```saida
0.881374 -0.881374
0.0 -inf
0.881374 1.443635
```

### `acosh`, `acoshf`, `acoshl`

```c
double acosh(double x);
float acoshf(float x);
long double acoshl(long double x);
```

Calcula o cosseno hiperbólico inverso de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante (a área de um setor hiperbólico) |

**Retorno:** o cosseno hiperbólico inverso de `x` (cosh⁻¹(x), ou
arcosh(x)), em [0, +∞]. Com erro de domínio, um valor definido pela
implementação (NaN onde há NaN).

**Erros:** erro de domínio (`EDOM`) se `x` é menor que 1. Com o ponto
flutuante IEEE (IEC 60559):

- `x` é menor que 1: levanta `FE_INVALID` e devolve NaN;
- `x` é 1: devolve +0;
- `x` é +∞: devolve +∞;
- `x` é NaN: devolve NaN.

O padrão do C chama a função de "arco cosseno hiperbólico", mas as inversas
das hiperbólicas são as funções de área: o argumento delas é a área de um
setor hiperbólico, não um arco. O nome certo é "cosseno hiperbólico inverso"
(o do POSIX) ou "área do cosseno hiperbólico".

```ps
import <math.h>
import <stdio.h>

printf("%.6f\n", acosh(2.0))
post(acosh(1.0), acosh(0.5), acosh(INFINITY))
printf("%.6f %.6f\n", acoshf(2.0), acoshl(3.0))
```

```saida
1.316958
0.0 nan inf
1.316958 1.762747
```

### `atanh`, `atanhf`, `atanhl`

```c
double atanh(double x);
float atanhf(float x);
long double atanhl(long double x);
```

Calcula a tangente hiperbólica inversa de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante (a área de um setor hiperbólico) |

**Retorno:** a tangente hiperbólica inversa de `x` (tanh⁻¹(x), ou
artanh(x)). Com erro de domínio, um valor definido pela implementação (NaN
onde há NaN). Com erro de polo, `±HUGE_VAL`, `±HUGE_VALF` ou `±HUGE_VALL`
(com o sinal certo). Com erro de faixa por perda, o resultado correto
(depois de arredondar).

**Erros:** erro de domínio (`EDOM`) se `x` está fora de [-1, +1]; erro de
polo (`ERANGE`) se `x` é ±1. Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve `x` sem mudar;
- `x` é ±1: devolve ±∞ e levanta `FE_DIVBYZERO`;
- |x| > 1: devolve NaN e levanta `FE_INVALID`;
- `x` é NaN: devolve NaN.

O padrão do C chama a função de "arco tangente hiperbólica", mas as inversas
das hiperbólicas são as funções de área: o argumento delas é a área de um
setor hiperbólico, não um arco. O nome certo é "tangente hiperbólica
inversa" (o do POSIX) ou "área da tangente hiperbólica". O POSIX diz que,
com perda, a função devolve `x` sem mudar, ou, onde isso não dá, um valor
definido pela implementação não maior que `DBL_MIN`, `FLT_MIN` e `LDBL_MIN`.

```ps
import <math.h>
import <stdio.h>

printf("%.6f\n", atanh(0.5))
post(atanh(1.0), atanh(-1.0), atanh(2.0), atanh(-0.0))
printf("%.6f %.6f\n", atanhf(0.5), atanhl(-0.5))
```

```saida
0.549306
inf -inf nan -0.0
0.549306 -0.549306
```

## Erro e gama

### `erf`, `erff`, `erfl`

```c
double erf(double x);
float erff(float x);
long double erfl(long double x);
```

Calcula a função erro de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o valor da função erro de `x`: 2/√π vezes a integral de
e^(-t²) de 0 a `x`. Com erro de faixa por perda, o resultado correto
(depois de arredondar), que é 2·x/√π.

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `x` é ±0: devolve ±0;
- `x` é ±∞: devolve ±1;
- `x` é NaN: devolve NaN.

A perda é garantida se |x| < `DBL_MIN` · (√π/2). `erf(x / (σ√2))` é a
probabilidade de uma medida, com erros que seguem a distribuição normal de
desvio padrão σ, ficar a menos de `x` da média.

```ps
import <math.h>
import <stdio.h>

# a chance de uma medida com erro normal ficar a menos de um desvio padrão da média
printf("%.6f %.6f\n", erf(1.0), erf(1.0 / sqrt(2.0)))
post(erf(0.0), erf(-0.0), erf(INFINITY), erf(-INFINITY))
printf("%.6f %.6f\n", erff(0.5), erfl(2.0))
```

```saida
0.842701 0.682689
0.0 -0.0 1.0 -1.0
0.520500 0.995322
```

### `erfc`, `erfcf`, `erfcl`

```c
double erfc(double x);
float erfcf(float x);
long double erfcl(long double x);
```

Calcula a função erro complementar de `x`, isto é, `1.0 - erf(x)`, mas sem a
perda de precisão quando `x` é grande.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o valor da função erro complementar de `x`: 2/√π vezes a
integral de e^(-t²) de `x` a ∞, ou 1 - erf(x). Com erro de faixa por perda,
o resultado correto (depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `x` é +∞: devolve +0;
- `x` é -∞: devolve 2;
- `x` é NaN: devolve NaN.

Com o `double` do IEEE, a perda é garantida se `x` > 26.55.

```ps
import <math.h>
import <stdio.h>

printf("%.6f\n", erfc(1.0))
# 1.0 - erf(10.0) dá zero; o erfc guarda o valor
printf("%.6e\n", erfc(10.0))
post(1.0 - erf(10.0), erfc(INFINITY), erfc(-INFINITY))
printf("%.6f %.6f\n", erfcf(0.5), erfcl(2.0))
```

```saida
0.157299
2.088488e-45
0.0 0.0 2.0
0.479500 0.004678
```

### `tgamma`, `tgammaf`, `tgammal`

```c
double tgamma(double x);
float tgammaf(float x);
long double tgammal(long double x);
```

Calcula a função gama de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o valor da função gama de `x`, Γ(x): a integral de
t^(x-1) · e^(-t) de 0 a ∞. Com erro de domínio, um valor definido pela
implementação (NaN onde há NaN). Com erro de polo, `±HUGE_VAL`,
`±HUGE_VALF` ou `±HUGE_VALL`. Com erro de faixa por estouro, `±HUGE_VAL`,
`±HUGE_VALF` ou `±HUGE_VALL`. Com erro de faixa por perda, o valor correto
(depois de arredondar).

**Erros:** se `x` é zero ou um inteiro menor que zero, pode haver erro de
polo (`ERANGE`) ou de domínio (`EDOM`). Com o ponto flutuante IEEE (IEC
60559):

- `x` é ±0: devolve ±∞ e levanta `FE_DIVBYZERO`;
- `x` é um inteiro negativo: devolve NaN e levanta `FE_INVALID`;
- `x` é -∞: devolve NaN e levanta `FE_INVALID`;
- `x` é +∞: devolve +∞;
- `x` é NaN: devolve NaN.

Se `x` é um número natural, `tgamma(x)` é o fatorial de `x - 1`. Muitas
implementações calculam o fatorial inteiro exato quando o argumento é um
inteiro pequeno.

Com o `double` do IEEE, há estouro se 0 < `x` < 1/`DBL_MAX` ou se
`x` > 171.7.

O POSIX exige erro de polo com o argumento zero e erro de domínio com um
inteiro negativo, e prevê que no futuro o erro de domínio pode virar erro de
polo nos inteiros negativos (aí o valor devolvido nesses casos passaria de
NaN pra ±∞).

Há uma função fora do padrão chamada `gamma` em várias implementações, mas a
definição dela muda: a da glibc e a do 4.2BSD fazem o `lgamma`, e a do
4.4BSD faz o `tgamma`.

```ps
import <math.h>
import <stdio.h>

# o fatorial de 4 e a raiz de pi
printf("%.6f %.6f\n", tgamma(5.0), tgamma(0.5))
post(tgamma(0.0), tgamma(-0.0), tgamma(-1.0), tgamma(INFINITY), tgamma(200.0))
printf("%.6f %.6f\n", tgammaf(4.0), tgammal(6.0))
```

```saida
24.000000 1.772454
inf -inf nan inf inf
6.000000 120.000000
```

### `lgamma`, `lgammaf`, `lgammal`

```c
double lgamma(double x);
float lgammaf(float x);
long double lgammal(long double x);
```

Calcula o logaritmo natural do valor absoluto da função gama de `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o logaritmo natural do valor absoluto da função gama de `x`,
ln|Γ(x)|. Com erro de polo, `+HUGE_VAL`, `+HUGE_VALF` ou `+HUGE_VALL`. Com
erro de faixa por estouro, `±HUGE_VAL`, `±HUGE_VALF` ou `±HUGE_VALL`.

**Erros:** se `x` é zero ou um inteiro menor que zero, pode haver erro de
polo (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- `x` é 1: devolve +0;
- `x` é 2: devolve +0;
- `x` é ±0: devolve +∞ e levanta `FE_DIVBYZERO`;
- `x` é um inteiro negativo: devolve +∞ e levanta `FE_DIVBYZERO`;
- `x` é ±∞: devolve +∞;
- `x` é NaN: devolve NaN.

Se `x` é um número natural, `lgamma(x)` é o logaritmo do fatorial de
`x - 1`.

A versão do POSIX guarda o sinal da gama de `x` na variável externa
`signgam`, e por isso não serve entre threads; algumas implementações têm o
`lgamma_r`, que recebe um ponteiro pra onde guardar o sinal. Nem o `signgam`
nem o `lgamma_r` são do C11, e a tabela do motor não tem nenhum dos dois.

Há uma função fora do padrão chamada `gamma` em várias implementações, mas a
definição dela muda: a da glibc e a do 4.2BSD fazem o `lgamma`, e a do
4.4BSD faz o `tgamma`.

```ps
import <math.h>
import <stdio.h>

# o fatorial de 199 estoura o double; o logaritmo dele, não
printf("%.6f %.6f\n", lgamma(10.0), lgamma(200.0))
post(tgamma(200.0))
post(lgamma(1.0), lgamma(2.0), lgamma(0.0), lgamma(-1.0))
printf("%.6f %.6f\n", lgammaf(10.0), lgammal(0.5))
```

```saida
12.801827 857.933670
inf
0.0 0.0 inf inf
12.801827 0.572365
```

## Inteiro mais próximo

### `ceil`, `ceilf`, `ceill`

```c
double ceil(double x);
float ceilf(float x);
long double ceill(long double x);
```

Calcula o menor inteiro que não é menor que `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o menor inteiro que não é menor que `x` (⌈x⌉), em ponto
flutuante.

**Erros:** os do `math_errhandling`. Com o ponto flutuante IEEE (IEC 60559):

- o modo de arredondamento não muda nada;
- `x` é ±∞: devolve `x` sem mudar;
- `x` é ±0: devolve `x` sem mudar;
- `x` é NaN: devolve NaN.

`FE_INEXACT` pode (sem obrigação) ser levantada ao arredondar um finito que
não é inteiro.

Os maiores valores de ponto flutuante já são inteiros exatos em todos os
formatos padrão, então a função nunca estoura sozinha; mas o resultado pode
não caber em tipo inteiro nenhum (nem no `intmax_t`) quando guardado numa
variável inteira.

```ps
import <math.h>

post(ceil(2.4), ceil(-2.4), ceil(-0.5), ceil(INFINITY))
post(ceilf(1.5), ceill(-1.5))
```

```saida
3.0 -2.0 -0.0 inf
2.0 -1.0
```

### `floor`, `floorf`, `floorl`

```c
double floor(double x);
float floorf(float x);
long double floorl(long double x);
```

Calcula o maior inteiro que não é maior que `x`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o maior inteiro que não é maior que `x` (⌊x⌋), em ponto
flutuante.

**Erros:** os do `math_errhandling`. Com o ponto flutuante IEEE (IEC 60559):

- o modo de arredondamento não muda nada;
- `x` é ±∞: devolve `x` sem mudar;
- `x` é ±0: devolve `x` sem mudar;
- `x` é NaN: devolve NaN.

`FE_INEXACT` pode (sem obrigação) ser levantada ao arredondar um finito que
não é inteiro.

Os maiores valores de ponto flutuante já são inteiros exatos em todos os
formatos padrão, então a função nunca estoura sozinha; mas o resultado pode
não caber em tipo inteiro nenhum (nem no `intmax_t`) quando guardado numa
variável inteira. Pra ter o `int` da Jinga, `int(floor(x))`.

```ps
import <math.h>

post(floor(2.7), floor(-2.7), floor(-0.0))
post(int(floor(-2.7)))
post(floorf(1.5), floorl(-1.5))
```

```saida
2.0 -3.0 -0.0
-3
1.0 -2.0
```

### `trunc`, `truncf`, `truncl`

```c
double trunc(double x);
float truncf(float x);
long double truncl(long double x);
```

Calcula o inteiro mais próximo de `x` que não é maior que ele em magnitude.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o inteiro mais próximo de `x` que não é maior que ele em
magnitude (em outras palavras, `x` arredondado na direção do zero), em ponto
flutuante.

**Erros:** os do `math_errhandling`. Com o ponto flutuante IEEE (IEC 60559):

- o modo de arredondamento não muda nada;
- `x` é ±∞: devolve `x` sem mudar;
- `x` é ±0: devolve `x` sem mudar;
- `x` é NaN: devolve NaN.

`FE_INEXACT` pode (sem obrigação) ser levantada ao cortar um finito que não
é inteiro.

Os maiores valores de ponto flutuante já são inteiros exatos em todos os
formatos padrão, então a função nunca estoura sozinha; mas o resultado pode
não caber em tipo inteiro nenhum (nem no `intmax_t`) quando guardado numa
variável inteira.

A conversão de ponto flutuante pra inteiro também arredonda na direção do
zero, mas só vale pros valores que cabem no tipo de destino. Na Jinga, o
`int()` de um `flo` também corta na direção do zero (`int(-2.7)` dá `-2`).

```ps
import <math.h>

post(trunc(2.7), trunc(-2.7), trunc(-0.5))
post(int(-2.7))
post(truncf(1.5), truncl(-1.5))
```

```saida
2.0 -2.0 -0.0
-2
1.0 -1.0
```

### `round`, `roundf`, `roundl`, `lround`, `lroundf`, `lroundl`, `llround`, `llroundf`, `llroundl`

```c
double round(double x);
float roundf(float x);
long double roundl(long double x);
long int lround(double x);
long int lroundf(float x);
long int lroundl(long double x);
long long int llround(double x);
long long int llroundf(float x);
long long int llroundl(long double x);
```

`round`, `roundf` e `roundl` calculam o inteiro mais próximo de `x` (em
ponto flutuante), levando o meio pra longe do zero, qualquer que seja o modo
de arredondamento. `lround`, `lroundf`, `lroundl`, `llround`, `llroundf` e
`llroundl` calculam o mesmo inteiro e o devolvem como inteiro (`long` e
`long long`).

Na Jinga, o nome `round` é o embutido da Jinga, mesmo com o `<math.h>`
importado ([`round`](../builtins/round/round.md)): ele leva o meio pro
**par** (`round(2.5)` dá `2`, como no Python), devolve `int` e aceita casas
decimais (`round(3.567, 2)`). O `round` do C fica pelos outros nomes: o
`roundl` dá, pra todo `flo`, o mesmo valor que o `round` do C daria (o
`long double` guarda todo `double`, e o inteiro arredondado cabe de volta no
`double`); o `lround` e o `llround` dão o inteiro.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o inteiro mais próximo de `x`, com o meio pra longe do zero.
Com erro de domínio, um valor definido pela implementação.

**Erros:** se o resultado do `lround` ou do `llround` não cabe no tipo de
retorno, pode haver erro de domínio ou de faixa. Com o ponto flutuante IEEE
(IEC 60559), no `round`, `roundf` e `roundl`:

- o modo de arredondamento não muda nada;
- `x` é ±∞: devolve `x` sem mudar;
- `x` é ±0: devolve `x` sem mudar;
- `x` é NaN: devolve NaN.

E nas famílias do `lround` e do `llround`:

- `FE_INEXACT` nunca é levantada;
- o modo de arredondamento não muda nada;
- `x` é ±∞: levanta `FE_INVALID` e devolve um valor definido pela
  implementação;
- o resultado do arredondamento não cabe no tipo de retorno: levanta
  `FE_INVALID` e devolve um valor definido pela implementação;
- `x` é NaN: levanta `FE_INVALID` e devolve um valor definido pela
  implementação.

O `round` pode (sem obrigação) levantar `FE_INEXACT` ao arredondar um finito
que não é inteiro. Os maiores valores de ponto flutuante já são inteiros
exatos em todos os formatos padrão, então o `round` nunca estoura sozinho;
mas o resultado pode não caber em tipo inteiro nenhum (nem no `intmax_t`)
quando guardado numa variável inteira. O POSIX diz que todo caso em que o
`lround` ou o `llround` levanta `FE_INVALID` é erro de domínio.

```ps
import <math.h>

# o round é o da Jinga: o meio vai pro par, e o resultado é int
post(round(2.5), round(3.5), round(-2.5))
# o do C leva o meio pra longe do zero
post(roundl(2.5), roundl(3.5), roundl(-2.5), roundf(-0.4))
post(lround(2.5), llround(-2.5), lroundf(0.5), lroundl(-3.5))
post(llroundf(1.5), llroundl(2.4))
```

```saida
2 4 -2
3.0 4.0 -3.0 -0.0
3 -3 1 -4
2 2
```

### `nearbyint`, `nearbyintf`, `nearbyintl`

```c
double nearbyint(double x);
float nearbyintf(float x);
long double nearbyintl(long double x);
```

Arredonda `x` pra um inteiro em ponto flutuante, pelo modo de arredondamento
atual.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o inteiro mais próximo de `x` pelo modo de arredondamento atual.

**Erros:** nenhum dos de `math_errhandling`. Com o ponto flutuante IEEE (IEC
60559):

- `FE_INEXACT` nunca é levantada;
- `x` é ±∞: devolve `x` sem mudar;
- `x` é ±0: devolve `x` sem mudar;
- `x` é NaN: devolve NaN.

A única diferença entre o `nearbyint` e o `rint` é que o `nearbyint` nunca
levanta `FE_INEXACT`. Os maiores valores de ponto flutuante já são inteiros
exatos em todos os formatos padrão, então o `nearbyint` nunca estoura
sozinho; mas o resultado pode não caber em tipo inteiro nenhum (nem no
`intmax_t`) quando guardado numa variável inteira. Com o modo
`FE_TONEAREST`, o de fábrica, o meio vai pro par (como no `rint`, e ao
contrário do `round` do C).

```ps
import <math.h>

# no modo de fábrica, o meio vai pro par
post(nearbyint(2.5), nearbyint(3.5), nearbyint(-2.5), nearbyint(2.6))
post(nearbyintf(0.5), nearbyintl(-1.5))
```

```saida
2.0 4.0 -2.0 3.0
0.0 -2.0
```

### `rint`, `rintf`, `rintl`, `lrint`, `lrintf`, `lrintl`, `llrint`, `llrintf`, `llrintl`

```c
double rint(double x);
float rintf(float x);
long double rintl(long double x);
long int lrint(double x);
long int lrintf(float x);
long int lrintl(long double x);
long long int llrint(double x);
long long int llrintf(float x);
long long int llrintl(long double x);
```

`rint`, `rintf` e `rintl` arredondam `x` pra um inteiro em ponto flutuante,
pelo modo de arredondamento atual. `lrint`, `lrintf`, `lrintl`, `llrint`,
`llrintf` e `llrintl` arredondam `x` pra um inteiro, pelo modo de
arredondamento atual, e o devolvem como inteiro (`long` e `long long`).

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o inteiro mais próximo de `x` pelo modo de arredondamento atual.

**Erros:** se o resultado do `lrint` ou do `llrint` não cabe no tipo de
retorno, pode haver erro de domínio ou de faixa. Com o ponto flutuante IEEE
(IEC 60559), no `rint`:

- `x` é ±∞: devolve `x` sem mudar;
- `x` é ±0: devolve `x` sem mudar;
- `x` é NaN: devolve NaN.

E no `lrint` e no `llrint`:

- `x` é ±∞: levanta `FE_INVALID` e devolve um valor definido pela
  implementação;
- o resultado do arredondamento não cabe no tipo de retorno: levanta
  `FE_INVALID` e devolve um valor definido pela implementação;
- `x` é NaN: levanta `FE_INVALID` e devolve um valor definido pela
  implementação.

O POSIX diz que todo caso em que o `lrint` ou o `llrint` levanta
`FE_INVALID` é erro de domínio. O `rint` pode levantar `FE_INEXACT` ao
arredondar um finito que não é inteiro (e só fora do IEEE isso não é
obrigatório). A única diferença entre o `rint` e o `nearbyint` é que o
`nearbyint` nunca levanta `FE_INEXACT`. Os maiores valores de ponto
flutuante já são inteiros exatos em todos os formatos padrão, então o `rint`
nunca estoura sozinho; mas o resultado pode não caber em tipo inteiro nenhum
(nem no `intmax_t`) quando guardado numa variável inteira.

Conforme o modo de arredondamento atual:

- `FE_DOWNWARD`: o `rint` é o `floor`;
- `FE_UPWARD`: o `rint` é o `ceil`;
- `FE_TOWARDZERO`: o `rint` é o `trunc`;
- `FE_TONEAREST` (o de fábrica): o `rint` difere do `round` do C no meio,
  que vai pro par em vez de ir pra longe do zero.

```ps
import <math.h>

# o modo de fábrica: o meio vai pro par
post(rint(2.5), rint(3.5), rintf(-0.5), rintl(1.5))
post(lrint(2.5), llrint(3.5), lrintl(-7.5))
post(lrintf(0.5), llrintf(-1.5), llrintl(4.5))
```

```saida
2.0 4.0 -0.0 2.0
2 4 -8
0 -2 4
```

## Manipulação do ponto flutuante

### `frexp`, `frexpf`, `frexpl`

```c
double frexp(double value, int *exp);
float frexpf(float value, int *exp);
long double frexpl(long double value, int *exp);
```

Decompõe o número de ponto flutuante `value` numa fração normalizada e numa
potência inteira de dois.

| Parâmetro | O que é |
|---|---|
| `value` | número de ponto flutuante |
| `exp` | ponteiro pra um `int`, onde o expoente é guardado |

**Retorno:** se `value` é zero, devolve zero e guarda zero em `*exp`. Senão,
sem erro, devolve um valor `f` em (-1, -0.5] ou em [0.5, 1) e guarda em
`*exp` o inteiro tal que f × 2^(*exp) = `value`. Se o valor a guardar em
`*exp` não cabe no `int`, o comportamento não é especificado.

**Erros:** nenhum dos de `math_errhandling`. Com o ponto flutuante IEEE (IEC
60559):

- `value` é ±0: devolve `value` sem mudar e guarda 0 em `*exp`;
- `value` é ±∞: devolve `value` e guarda um valor não especificado em
  `*exp`;
- `value` é NaN: devolve NaN e guarda um valor não especificado em `*exp`;
- nenhuma exceção de ponto flutuante é levantada;
- com `FLT_RADIX` 2 (ou uma potência de 2), o valor devolvido é exato, e o
  modo de arredondamento não conta.

O `frexp`, com o par dele, o [`ldexp`](#ldexp-ldexpf-ldexpl), serve pra
mexer na representação de um número de ponto flutuante sem mexer nos bits.

```ps
import <math.h>

int e = 0
flo m = frexp(123.45, &e)
# 123.45 = 0.964453125 * 2^7, e o ldexp desfaz
post(m, e, ldexp(m, e))
m = frexp(0.0, &e)
post(m, e)
m = frexpf(8.0, &e)
post(m, e)
m = frexpl(-1.0, &e)
post(m, e)
```

```saida
0.964453125 7 123.45
0.0 0
0.5 4
-0.5 1
```

### `ldexp`, `ldexpf`, `ldexpl`

```c
double ldexp(double x, int exp);
float ldexpf(float x, int exp);
long double ldexpl(long double x, int exp);
```

Multiplica o número de ponto flutuante `x` por 2 elevado a `exp`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |
| `exp` | inteiro |

**Retorno:** `x` vezes 2 elevado a `exp`. Com erro de faixa por estouro,
`±HUGE_VAL`, `±HUGE_VALF` ou `±HUGE_VALL`. Com erro de faixa por perda, o
resultado correto (depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- sem erro de faixa, `FE_INEXACT` nunca é levantada (o resultado é exato);
- sem erro de faixa, o modo de arredondamento não conta;
- `x` é ±0: devolve `x` sem mudar;
- `x` é ±∞: devolve `x` sem mudar;
- `exp` é 0: devolve `x` sem mudar;
- `x` é NaN: devolve NaN.

Nos sistemas binários (onde `FLT_RADIX` é 2), o `ldexp` é o mesmo que o
[`scalbn`](#scalbn-scalbnf-scalbnl-scalbln-scalblnf-scalblnl). O `ldexp`
("carregar o expoente"), com o par dele, o [`frexp`](#frexp-frexpf-frexpl),
serve pra mexer na representação de um número de ponto flutuante sem mexer
nos bits. Em muitas implementações, o `ldexp` é menos eficiente que
multiplicar ou dividir por uma potência de dois.

Na Jinga, o `exp` é inteiro: um `flo` ali é `TypeError` antes de rodar.

```ps
import <math.h>

post(ldexp(1.0, 10), ldexp(3.0, -2), ldexp(1.0, 1024))
post(ldexpf(1.5, 2), ldexpl(1.0, -1))
```

```saida
1024.0 0.75 inf
6.0 0.5
```

### `modf`, `modff`, `modfl`

```c
double modf(double value, double *iptr);
float modff(float value, float *iptr);
long double modfl(long double value, long double *iptr);
```

Decompõe o número de ponto flutuante `value` em parte inteira e parte
fracionária, cada uma com o tipo e o sinal de `value`. A parte inteira (em
ponto flutuante) vai pro objeto que `iptr` aponta.

| Parâmetro | O que é |
|---|---|
| `value` | número de ponto flutuante |
| `iptr` | ponteiro pro número de ponto flutuante onde a parte inteira é guardada |

**Retorno:** a parte fracionária de `value`, com o sinal dele. A parte
inteira vai pra `*iptr`. A soma do valor devolvido com o de `*iptr` dá
`value` (descontado o arredondamento).

**Erros:** nenhum dos de `math_errhandling`. Com o ponto flutuante IEEE (IEC
60559):

- `value` é ±0: devolve ±0 e guarda ±0 em `*iptr`;
- `value` é ±∞: devolve ±0 e guarda ±∞ em `*iptr`;
- `value` é NaN: devolve NaN e guarda NaN em `*iptr`;
- o valor devolvido é exato, e o modo de arredondamento não conta.

Na Jinga, o `iptr` é o endereço de uma variável do tipo do protótipo: `flo`
(ou `double`) no `modf`, `float` no `modff` e `long double` no `modfl`.

```ps
import <math.h>

flo inteira = 0.0
flo frac = modf(3.75, &inteira)
post(frac, inteira)
frac = modf(-3.75, &inteira)
post(frac, inteira)
frac = modf(INFINITY, &inteira)
post(frac, inteira)
# o modff escreve num float, e o modfl num long double
float f = 0.0
long double l = 0.0
post(modff(2.5, &f), f, modfl(-0.25, &l), l)
```

```saida
0.75 3.0
-0.75 -3.0
0.0 inf
0.5 2.0 -0.25 -0.0
```

### `scalbn`, `scalbnf`, `scalbnl`, `scalbln`, `scalblnf`, `scalblnl`

```c
double scalbn(double x, int n);
float scalbnf(float x, int n);
long double scalbnl(long double x, int n);
double scalbln(double x, long int n);
float scalblnf(float x, long int n);
long double scalblnl(long double x, long int n);
```

Multiplicam o número de ponto flutuante `x` por `FLT_RADIX` elevado a `n`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |
| `n` | inteiro (`int` no `scalbn`, `long` no `scalbln`) |

**Retorno:** `x` vezes `FLT_RADIX` elevado a `n`. Com erro de faixa por
estouro, `±HUGE_VAL`, `±HUGE_VALF` ou `±HUGE_VALL`. Com erro de faixa por
perda, o resultado correto (depois de arredondar).

**Erros:** o de faixa (`ERANGE`). Com o ponto flutuante IEEE (IEC 60559):

- sem erro de faixa, `FE_INEXACT` nunca é levantada (o resultado é exato);
- sem erro de faixa, o modo de arredondamento não conta;
- `x` é ±0: devolve `x` sem mudar;
- `x` é ±∞: devolve `x` sem mudar;
- `n` é 0: devolve `x` sem mudar;
- `x` é NaN: devolve NaN.

Nos sistemas binários (onde `FLT_RADIX` é 2, como no Linux e no macOS), o
`scalbn` é o mesmo que o [`ldexp`](#ldexp-ldexpf-ldexpl). O padrão pede que
o `scalbn` e o `scalbln` façam a conta de forma eficiente, mas em muitas
implementações eles são menos eficientes que multiplicar ou dividir por uma
potência de dois.

O `scalbln` existe porque o fator que leva do menor valor positivo ao maior
valor finito pode passar de 32767, o `INT_MAX` que o padrão garante. No
`long double` de 80 bits, o fator é 32828.

```ps
import <math.h>

post(scalbn(1.5, 3), scalbln(1.5, -3), scalbnf(1.0, 10))
post(scalbnl(1.0, -1), scalblnf(3.0, 1), scalblnl(1.0, 2))
```

```saida
12.0 0.1875 1024.0
0.5 6.0 4.0
```

### `ilogb`, `ilogbf`, `ilogbl`

```c
int ilogb(double x);
int ilogbf(float x);
int ilogbl(long double x);
```

Extrai o expoente sem viés de `x` e o devolve como inteiro com sinal.
Formalmente, com `x` diferente de zero, é a parte inteira de log_r |x|, com
r = `FLT_RADIX`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o expoente sem viés de `x`, como `int`. Se `x` é zero, devolve
`FP_ILOGB0`; se é infinito, `INT_MAX`; se é NaN, `FP_ILOGBNAN`. Se o
resultado correto passa de `INT_MAX` ou fica abaixo de `INT_MIN`, o valor
devolvido não é especificado, e pode haver erro de domínio ou de faixa.

**Erros:** pode haver erro de domínio ou de faixa se `x` é zero, infinito ou
NaN, e também se o resultado correto não cabe no `int`. Com o ponto
flutuante IEEE (IEC 60559):

- o resultado correto passa de `INT_MAX` ou fica abaixo de `INT_MIN`:
  levanta `FE_INVALID`;
- `x` é ±0, ±∞ ou NaN: levanta `FE_INVALID`;
- nos outros casos, o resultado é exato (`FE_INEXACT` nunca é levantada), e
  o modo de arredondamento não conta.

Se `x` não é zero, infinito nem NaN, o valor devolvido é exatamente
`(int)logb(x)`. O POSIX exige erro de domínio se `x` é zero, infinito ou NaN,
ou se o resultado correto não cabe no `int`; nos sistemas XSI, exige também
que o valor devolvido nesse último caso seja `INT_MAX` (acima) ou `INT_MIN`
(abaixo). Em todas as implementações conhecidas o resultado correto cabe no
`int`.

O expoente do `ilogb` é sempre 1 a menos que o do
[`frexp`](#frexp-frexpf-frexpl), pela normalização diferente: com o expoente
`e` do `ilogb`, |x × r^-e| fica entre 1 e r (em geral, entre 1 e 2); com o
do `frexp`, |x × 2^-e| fica entre 0.5 e 1.

```ps
import <math.h>
import <limits.h>

post(ilogb(1.0), ilogb(1024.0), ilogb(0.75), ilogb(1000.0))
post(ilogb(0.0) == FP_ILOGB0, ilogb(NAN) == FP_ILOGBNAN, ilogb(INFINITY) == INT_MAX)
post(ilogbf(8.0), ilogbl(0.5))
```

```saida
0 10 -1 9
True True True
3 -1
```

### `logb`, `logbf`, `logbl`

```c
double logb(double x);
float logbf(float x);
long double logbl(long double x);
```

Extrai o expoente sem viés, independente da base, de `x`, e o devolve como
ponto flutuante. Formalmente, com `x` diferente de zero, é a parte inteira
com sinal de log_r |x|, com r = `FLT_RADIX`. Se `x` é subnormal, é tratado
como se fosse normalizado.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** o expoente sem viés de `x`, como ponto flutuante com sinal. Com
erro de domínio, um valor definido pela implementação. Com erro de polo,
`-HUGE_VAL`, `-HUGE_VALF` ou `-HUGE_VALL`.

**Erros:** pode haver erro de domínio ou de faixa se `x` é zero. Com o ponto
flutuante IEEE (IEC 60559):

- `x` é ±0: devolve -∞ e levanta `FE_DIVBYZERO`;
- `x` é ±∞: devolve +∞;
- `x` é NaN: devolve NaN;
- nos outros casos, o resultado é exato (`FE_INEXACT` nunca é levantada), e
  o modo de arredondamento não conta.

O POSIX exige erro de polo se `x` é ±0. O expoente do `logb` é sempre 1 a
menos que o do [`frexp`](#frexp-frexpf-frexpl), pela normalização diferente:
com o expoente `e` do `logb`, |x × r^-e| fica entre 1 e r (em geral, entre 1
e 2); com o do `frexp`, |x × 2^-e| fica entre 0.5 e 1.

```ps
import <math.h>

post(logb(1024.0), logb(0.75), logb(-8.0), logb(0.0))
# o frexp dá um a mais: 0.75 é 1.5 * 2^-1 pro logb, e 0.75 * 2^0 pro frexp
int e = 0
flo m = frexp(0.75, &e)
post(logb(0.75), e)
post(logbf(16.0), logbl(INFINITY))
```

```saida
10.0 -1.0 3.0 -inf
-1.0 0
4.0 inf
```

### `nextafter`, `nextafterf`, `nextafterl`, `nexttoward`, `nexttowardf`, `nexttowardl`

```c
double nextafter(double x, double y);
float nextafterf(float x, float y);
long double nextafterl(long double x, long double y);
double nexttoward(double x, long double y);
float nexttowardf(float x, long double y);
long double nexttowardl(long double x, long double y);
```

`nextafter`, `nextafterf` e `nextafterl` convertem os dois argumentos pro
tipo da função e devolvem o próximo valor representável depois de `x`, na
direção de `y`. Se `x` é igual a `y`, devolvem `y`.

`nexttoward`, `nexttowardf` e `nexttowardl` convertem o primeiro argumento
pro tipo da função e devolvem o próximo valor representável depois de `x`,
na direção de `y`, que fica `long double`. Se `x` é igual a `y`, devolvem
`y`, convertido de `long double` pro tipo de retorno sem perda de faixa nem
de precisão.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante, o ponto de partida |
| `y` | número de ponto flutuante, a direção (`long double` no `nexttoward`) |

**Retorno:** o próximo valor representável depois de `x`, na direção de
`y`; se `x` é igual a `y`, `y`. Com erro de faixa por estouro, `±HUGE_VAL`,
`±HUGE_VALF` ou `±HUGE_VALL`, com o sinal de `x`. Com erro de faixa por
perda, o resultado correto.

**Erros:** os do `math_errhandling`. Com o ponto flutuante IEEE (IEC 60559):

- `x` é finito e o resultado esperado é infinito: levanta `FE_INEXACT` e
  `FE_OVERFLOW`;
- `x` é diferente de `y` e o resultado é subnormal ou zero: levanta
  `FE_INEXACT` e `FE_UNDERFLOW`;
- em todo caso, o valor devolvido não depende do modo de arredondamento;
- `x` ou `y` é NaN: devolve NaN.

O POSIX diz que o estouro e a perda são erros de faixa (o `errno` pode
receber `ERANGE`). O IEC 60559 recomenda devolver `x` quando `x == y`; estas
funções devolvem `y`, o que deixa o comportamento perto do zero coerente:
`nextafter(-0.0, +0.0)` devolve +0.0, e `nextafter(+0.0, -0.0)` devolve
-0.0. As implementações costumam mexer direto na representação IEEE.

Na Jinga, o `y` chega de um `flo`, e o `long double` guarda todo `double`:
o `nexttoward` dá o mesmo que o `nextafter`. A diferença aparece no
`nexttowardf`, que não arredonda o alvo pra `float` antes.

```ps
import <math.h>

# o próximo double depois de 1.0 fica a 2^-52 dele; o anterior, a 2^-53
post(nextafter(1.0, 2.0) - 1.0, nextafter(1.0, 0.0) - 1.0)
post(nextafter(0.0, 1.0), nextafter(-0.0, 0.0))
# o nextafterf arredonda o alvo pra float (e ele vira 1.0); o nexttowardf não
flo alvo = 1.0 + ldexp(1.0, -30)
post(nextafterf(1.0, alvo), nexttowardf(1.0, alvo))
post(nextafterl(1.0, 1.0), nexttoward(1.0, 2.0) - 1.0)
```

```saida
2.220446049250313e-16 -1.1102230246251565e-16
5e-324 0.0
1.0 1.0000001192092896
1.0 2.220446049250313e-16
```

### `copysign`, `copysignf`, `copysignl`

```c
double copysign(double x, double y);
float copysignf(float x, float y);
long double copysignl(long double x, long double y);
```

Compõe um valor de ponto flutuante com a magnitude de `x` e o sinal de `y`.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** o valor de ponto flutuante com a magnitude de `x` e o sinal de
`y`. Se `x` é NaN, devolve NaN com o sinal de `y`. Se `y` é -0, o resultado
só é negativo se a implementação trata o zero com sinal de forma coerente
nas operações aritméticas.

**Erros:** nenhum dos de `math_errhandling`. Com o ponto flutuante IEEE (IEC
60559):

- o valor devolvido é exato (`FE_INEXACT` nunca é levantada) e não depende
  do modo de arredondamento.

O `copysign` é o único jeito portátil de mexer no sinal de um NaN (pra ver o
sinal de um NaN, há também o [`signbit`](#signbit)).

```ps
import <math.h>

post(copysign(3.0, -0.0), copysign(-3.0, 1.0), copysign(INFINITY, -2.0))
# o sinal de um NaN: o copysign põe, o signbit lê
post(signbit(copysign(NAN, -1.0)) != 0, signbit(copysign(NAN, 1.0)) != 0)
post(copysignf(1.5, -1.0), copysignl(-2.0, 0.0))
```

```saida
-3.0 3.0 -inf
True False
-1.5 2.0
```

## Classificação e comparação

As macros desta seção recebem o número como `double` (o padrão aceita
qualquer tipo de ponto flutuante real; na Jinga o número chega como `flo`) e
devolvem um `int`: diferente de zero pra sim, zero pra não. Qual número sai
no "sim" depende do sistema; compare com `!= 0`.

### `fpclassify`

```c
int fpclassify(double x);
```

Classifica o número de ponto flutuante `x` numa destas categorias: zero,
subnormal, normal, infinito, NaN, ou uma categoria definida pela
implementação. A macro devolve um inteiro.

O `FLT_EVAL_METHOD` não conta: mesmo que o argumento tenha sido calculado
com mais faixa e precisão que o tipo dele, ele é convertido pro tipo
semântico antes, e a classificação é a desse tipo (um `long double` normal
pode virar subnormal convertido pra `double`, e zero convertido pra
`float`). Na Jinga o número chega como `double`, e a classificação é a do
`double`.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** uma das constantes `FP_INFINITE`, `FP_NAN`, `FP_NORMAL`,
`FP_SUBNORMAL`, `FP_ZERO`, ou um tipo definido pela implementação, com a
categoria de `x`. O valor de cada constante muda de sistema pra sistema
([as constantes](#fp_normal-fp_subnormal-fp_zero-fp_infinite-fp_nan)):
compare com o nome.

```ps
import <math.h>

str funct categoria(flo x) {
    int c = fpclassify(x)
    if c == FP_NAN { return "NaN" }
    if c == FP_INFINITE { return "infinito" }
    if c == FP_ZERO { return "zero" }
    if c == FP_SUBNORMAL { return "subnormal" }
    return "normal"
}

post(categoria(NAN), categoria(-INFINITY), categoria(-0.0))
post(categoria(5e-324), categoria(1.0))
```

```saida
NaN infinito zero
subnormal normal
```

### `isfinite`

```c
int isfinite(double x);
```

Confere se o número de ponto flutuante `x` é finito: normal, subnormal ou
zero, mas não infinito nem NaN. A macro devolve um inteiro.

O `FLT_EVAL_METHOD` não conta: mesmo que o argumento tenha sido calculado
com mais faixa e precisão que o tipo dele, ele é convertido pro tipo
semântico antes, e a classificação é a desse tipo.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** diferente de zero se `x` é finito; zero se não é.

```ps
import <math.h>

post(isfinite(1.0) != 0, isfinite(INFINITY) != 0, isfinite(NAN) != 0)
```

```saida
True False False
```

### `isinf`

```c
int isinf(double x);
```

Confere se o número de ponto flutuante `x` é infinito, positivo ou negativo.
A macro devolve um inteiro.

O `FLT_EVAL_METHOD` não conta: mesmo que o argumento tenha sido calculado
com mais faixa e precisão que o tipo dele, ele é convertido pro tipo
semântico antes, e a classificação é a desse tipo.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** diferente de zero se `x` é infinito; zero se não é. No Linux, o
`isinf(-INFINITY)` dá `-1`; no macOS, `1`.

```ps
import <math.h>

post(isinf(-INFINITY) != 0, isinf(HUGE_VAL) != 0, isinf(1.0e308) != 0, isinf(NAN) != 0)
```

```saida
True True False False
```

### `isnan`

```c
int isnan(double x);
```

Confere se o número de ponto flutuante `x` é NaN (não é um número). A macro
devolve um inteiro.

O `FLT_EVAL_METHOD` não conta: mesmo que o argumento tenha sido calculado
com mais faixa e precisão que o tipo dele, ele é convertido pro tipo
semântico antes, e a classificação é a desse tipo (isso importa se o tipo do
cálculo tem NaN e o tipo semântico não tem).

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** diferente de zero se `x` é NaN; zero se não é.

Há muitos NaN diferentes, com bits de sinal e conteúdos diferentes (ver o
[`nan`](#nan-nanf-nanl)). O NaN nunca é igual a ele mesmo nem a outro NaN.
Copiar um NaN pode mudar os bits dele. Outro jeito de ver se um número é NaN
é compará-lo com ele mesmo: `x != x` dá verdadeiro só pro NaN.

```ps
import <math.h>

flo n = NAN
post(isnan(n) != 0, isnan(1.0) != 0)
# o NaN não é igual a nada, nem a ele mesmo
post(n == n, n != n)
```

```saida
True False
False True
```

### `isnormal`

```c
int isnormal(double x);
```

Confere se o número de ponto flutuante `x` é normal: nem zero, nem
subnormal, nem infinito, nem NaN. A macro devolve um inteiro.

O `FLT_EVAL_METHOD` não conta: mesmo que o argumento tenha sido calculado
com mais faixa e precisão que o tipo dele, ele é convertido pro tipo
semântico antes, e a classificação é a desse tipo.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** diferente de zero se `x` é normal; zero se não é.

```ps
import <math.h>

post(isnormal(1.0) != 0, isnormal(0.0) != 0, isnormal(5e-324) != 0, isnormal(INFINITY) != 0)
```

```saida
True False False False
```

### `signbit`

```c
int signbit(double x);
```

Confere se o número de ponto flutuante `x` é negativo. A macro devolve um
inteiro.

| Parâmetro | O que é |
|---|---|
| `x` | número de ponto flutuante |

**Retorno:** diferente de zero se `x` é negativo; zero se não é.

A macro vê o bit de sinal dos zeros, dos infinitos e dos NaN. Com o
[`copysign`](#copysign-copysignf-copysignl), é um dos dois únicos jeitos
portáteis de ver o sinal de um NaN.

```ps
import <math.h>

# o zero negativo é igual ao positivo; o signbit vê a diferença
post(-0.0 == 0.0, signbit(-0.0) != 0, signbit(0.0) != 0)
post(signbit(-INFINITY) != 0, signbit(copysign(NAN, -1.0)) != 0)
```

```saida
True True False
True True
```

### `isgreater`

```c
int isgreater(double x, double y);
```

Confere se o número de ponto flutuante `x` é maior que `y`, sem levantar
exceção de ponto flutuante.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** diferente de zero se `x > y`; zero se não.

O `>` do C com ponto flutuante pode levantar `FE_INVALID` se um dos
argumentos (ou os dois) é NaN. Esta macro é a versão "silenciosa" do `>`. Na
Jinga, o `>` com NaN dá `False`, sem erro.

```ps
import <math.h>

post(isgreater(2.0, 1.0) != 0, isgreater(1.0, 2.0) != 0, isgreater(NAN, 1.0) != 0)
```

```saida
True False False
```

### `isgreaterequal`

```c
int isgreaterequal(double x, double y);
```

Confere se o número de ponto flutuante `x` é maior ou igual a `y`, sem
levantar exceção de ponto flutuante.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** diferente de zero se `x >= y`; zero se não.

O `>=` do C com ponto flutuante pode levantar `FE_INVALID` se um dos
argumentos (ou os dois) é NaN. Esta macro é a versão "silenciosa" do `>=`.

```ps
import <math.h>

post(isgreaterequal(2.0, 2.0) != 0, isgreaterequal(1.0, 2.0) != 0, isgreaterequal(NAN, NAN) != 0)
```

```saida
True False False
```

### `isless`

```c
int isless(double x, double y);
```

Confere se o número de ponto flutuante `x` é menor que `y`, sem levantar
exceção de ponto flutuante.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** diferente de zero se `x < y`; zero se não.

O `<` do C com ponto flutuante pode levantar `FE_INVALID` se um dos
argumentos (ou os dois) é NaN. Esta macro é a versão "silenciosa" do `<`.

```ps
import <math.h>

post(isless(1.0, 2.0) != 0, isless(2.0, 1.0) != 0, isless(1.0, NAN) != 0)
```

```saida
True False False
```

### `islessequal`

```c
int islessequal(double x, double y);
```

Confere se o número de ponto flutuante `x` é menor ou igual a `y`, sem
levantar exceção de ponto flutuante.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** diferente de zero se `x <= y`; zero se não.

O `<=` do C com ponto flutuante pode levantar `FE_INVALID` se um dos
argumentos (ou os dois) é NaN. Esta macro é a versão "silenciosa" do `<=`.

```ps
import <math.h>

post(islessequal(2.0, 2.0) != 0, islessequal(3.0, 2.0) != 0, islessequal(NAN, 2.0) != 0)
```

```saida
True False False
```

### `islessgreater`

```c
int islessgreater(double x, double y);
```

Confere se o número de ponto flutuante `x` é menor ou maior que `y`, sem
levantar exceção de ponto flutuante.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** diferente de zero se `x < y || x > y`; zero se não.

O `<` e o `>` do C com ponto flutuante podem levantar `FE_INVALID` se um dos
argumentos (ou os dois) é NaN. Esta macro é a versão "silenciosa" da
expressão `x < y || x > y`, e não calcula `x` e `y` duas vezes.

```ps
import <math.h>

post(islessgreater(1.0, 2.0) != 0, islessgreater(2.0, 2.0) != 0, islessgreater(1.0, NAN) != 0)
```

```saida
True False False
```

### `isunordered`

```c
int isunordered(double x, double y);
```

Confere se os números de ponto flutuante `x` e `y` não têm ordem, isto é, se
um dos dois (ou os dois) é NaN e por isso a comparação entre eles não tem
sentido.

| Parâmetro | O que é |
|---|---|
| `x`, `y` | números de ponto flutuante |

**Retorno:** diferente de zero se `x` ou `y` é NaN; zero se não.

```ps
import <math.h>

post(isunordered(1.0, NAN) != 0, isunordered(NAN, NAN) != 0, isunordered(1.0, 2.0) != 0)
```

```saida
True True False
```

## Tipos

### `float_t`, `double_t`

```c
typedef float float_t;
typedef double double_t;
```

Os tipos de ponto flutuante pelo menos tão largos quanto o `float` e o
`double`, com o `double_t` pelo menos tão largo quanto o `float_t`. O valor
do `FLT_EVAL_METHOD` (do `float.h`) decide quais são:

| `FLT_EVAL_METHOD` | O que quer dizer |
|---|---|
| `0` | `float_t` e `double_t` são o `float` e o `double` |
| `1` | os dois são o `double` |
| `2` | os dois são o `long double` |
| outro | os dois são definidos pela implementação |

No Linux e no macOS de 64 bits, o `FLT_EVAL_METHOD` é `0`: o `float_t` é o
`float`, e o `double_t` é o `double` (o `typedef` acima). Numa variável da
Jinga, o `float_t` guarda com a precisão do `float`, e os dois são lidos
como `flo`.

```ps
import <math.h>

# o float_t aqui é o float: guarda o 0.1 com a precisão do float
float_t a = 0.1
double_t b = 0.1
post(a, b)
post(sizeof(float_t), sizeof(double_t))
```

```saida
0.10000000149011612 0.1
4 8
```

## Constantes

O valor das constantes inteiras é o de cada sistema; as tabelas mostram o
do macOS e o do Linux (glibc, x86-64), medidos.

### `HUGE_VAL`, `HUGE_VALF`, `HUGE_VALL`

| Constante | Tipo no C | O que é | Valor |
|---|---|---|---|
| `HUGE_VALF` | `float` | o valor positivo que indica estouro no `float` | `inf` |
| `HUGE_VAL` | `double` | o valor positivo que indica estouro no `double`, que não precisa caber no `float` | `inf` |
| `HUGE_VALL` | `long double` | o valor positivo que indica estouro no `long double`, que não precisa caber no `float` nem no `double` | `inf` |

As três são expressões constantes positivas, iguais ao valor que as funções
de ponto flutuante e os operadores devolvem no estouro (ver o
[`math_errhandling`](#math_errhandling-math_errno-math_errexcept)). Onde o
ponto flutuante tem infinito, elas são o +∞ do `float`, do `double` e do
`long double`. Na Jinga, as três chegam como `flo`.

```ps
import <math.h>

post(HUGE_VAL, -HUGE_VAL, HUGE_VALF == HUGE_VALL)
# o estouro devolve HUGE_VAL
post(exp(1000.0) == HUGE_VAL)
```

```saida
inf -inf True
True
```

### `INFINITY`

| Constante | Tipo no C | O que é | Valor |
|---|---|---|---|
| `INFINITY` | `float` | o infinito positivo (ou sem sinal) | `inf` |

Se o `float` tem infinito, o `INFINITY` é uma expressão constante do tipo
`float` que vale o infinito positivo (ou sem sinal). Se não tem, é um valor
positivo que estoura o `float` na compilação, e usá-lo dá um aviso do
compilador. Na Jinga, chega como `flo`.

```ps
import <math.h>

post(INFINITY, -INFINITY, INFINITY == HUGE_VAL, isinf(INFINITY) != 0)
```

```saida
inf -inf True True
```

### `NAN`

| Constante | Tipo no C | O que é | Valor |
|---|---|---|---|
| `NAN` | `float` | um NaN silencioso | `nan` |

O `NAN` é uma expressão constante do tipo `float` que vale um NaN silencioso
(QNaN). Se o `float` não tem NaN silencioso, o `NAN` não é definido.

Há muitos NaN diferentes, com conteúdos e bits de sinal diferentes; o
conteúdo e o sinal do NaN do `NAN` são definidos pela implementação. Na
Jinga, chega como `flo`.

```ps
import <math.h>

post(NAN, isnan(NAN) != 0, NAN == NAN)
```

```saida
nan True False
```

### `FP_NORMAL`, `FP_SUBNORMAL`, `FP_ZERO`, `FP_INFINITE`, `FP_NAN`

As categorias que o [`fpclassify`](#fpclassify) devolve. São `int`, e o
valor muda de sistema pra sistema: compare com o nome, nunca com o número.

| Constante | O que indica | macOS | Linux (glibc) |
|---|---|---|---|
| `FP_NORMAL` | o valor é normal: não é infinito, subnormal, NaN nem zero | `4` | `4` |
| `FP_SUBNORMAL` | o valor é subnormal | `5` | `3` |
| `FP_ZERO` | o valor é zero, positivo ou negativo | `3` | `2` |
| `FP_INFINITE` | o valor não cabe no tipo: infinito, positivo ou negativo | `2` | `1` |
| `FP_NAN` | o valor não é um número (NaN) | `1` | `0` |

O exemplo está no [`fpclassify`](#fpclassify); este confere a categoria do
menor `double` positivo, que é subnormal:

```ps
import <math.h>

flo menor = nextafter(0.0, 1.0)
post(fpclassify(menor) == FP_SUBNORMAL, fpclassify(menor * 0.5) == FP_ZERO)
```

```saida
True True
```

### `FP_ILOGB0`, `FP_ILOGBNAN`

| Constante | O que é | macOS | Linux (glibc, x86-64) |
|---|---|---|---|
| `FP_ILOGB0` | o que o `ilogb` devolve pra zero: `INT_MIN` ou `-INT_MAX` | `-2147483648` | `-2147483648` |
| `FP_ILOGBNAN` | o que o `ilogb` devolve pra NaN: `INT_MIN` ou `+INT_MAX` | `-2147483648` | `-2147483648` |

As duas são expressões constantes inteiras, definidas pela implementação.

```ps
import <math.h>

post(ilogb(0.0) == FP_ILOGB0, ilogb(-0.0) == FP_ILOGB0, ilogb(NAN) == FP_ILOGBNAN)
```

```saida
True True True
```

### `FP_FAST_FMA`, `FP_FAST_FMAF`, `FP_FAST_FMAL`

| Constante | O que diz, quando definida | macOS | Linux (glibc, x86-64) |
|---|---|---|---|
| `FP_FAST_FMA` | a `fma` é mais rápida que `x * y + z` com `double` | não definida | não definida |
| `FP_FAST_FMAF` | a `fmaf` é mais rápida que `x * y + z` com `float` | não definida | não definida |
| `FP_FAST_FMAL` | a `fmal` é mais rápida que `x * y + z` com `long double` | não definida | não definida |

São opcionais: quando o sistema as define, valem o inteiro `1`, e a função
correspondente é mais rápida (além de mais precisa) que a expressão. Onde o
sistema não as define, o nome é `NameError` antes de rodar, com o motivo
(`FP_FAST_FMA não está definida neste sistema (o math.h a deixa opcional)`).
A [`fma`](#fma-fmaf-fmal) existe nos dois casos.

```ps
import <math.h>

# a fma vale com ou sem as FP_FAST_FMA
post(fma(3.0, 4.0, 5.0))
```

```saida
17.0
```

### `math_errhandling`, `MATH_ERRNO`, `MATH_ERREXCEPT`

| Constante | O que é | macOS | Linux (glibc) |
|---|---|---|---|
| `MATH_ERRNO` | as operações de ponto flutuante avisam o erro no `errno` | `1` | `1` |
| `MATH_ERREXCEPT` | as operações avisam o erro pelas exceções de ponto flutuante: pelo menos `FE_DIVBYZERO`, `FE_INVALID` e `FE_OVERFLOW`, do `fenv.h` | `2` | `2` |
| `math_errhandling` | o jeito deste sistema: `MATH_ERRNO`, `MATH_ERREXCEPT` ou os dois (`MATH_ERRNO \| MATH_ERREXCEPT`) | `2` | `3` |

O `math_errhandling` é um `int` que diz como os operadores e as funções de
ponto flutuante avisam o erro. Com o ponto flutuante IEEE (IEC 60559),
`math_errhandling & MATH_ERREXCEPT` tem que dar diferente de zero.

Os erros que o C reconhece:

| Erro | Quando | `errno` | Exceção | Exemplo |
|---|---|---|---|---|
| de domínio | o argumento está fora da faixa em que a operação é definida (cada função diz os erros de domínio dela) | `EDOM` | `FE_INVALID` | `acos(2.0)` |
| de polo | o resultado matemático é exatamente infinito ou indefinido | `ERANGE` | `FE_DIVBYZERO` | `log(0.0)` |
| de faixa por estouro | o resultado matemático é finito, mas vira infinito ao arredondar, ou vira o maior finito ao arredondar pra baixo | `ERANGE` | `FE_OVERFLOW` | `exp(1000.0)` |
| de faixa por perda | o resultado não é zero, mas vira zero ao arredondar, ou vira subnormal perdendo precisão | `ERANGE` ou nada (definido pela implementação) | `FE_UNDERFLOW` ou nada (definido pela implementação) | `nextafter(0.0, 1.0) / 2` |
| inexato | o resultado precisa ser arredondado pra caber no tipo | nada | `FE_INEXACT` ou nada (não especificado) | `sqrt(2.0)` |

Se as funções da biblioteca levantam `FE_INEXACT`, em geral não é
especificado, mas a descrição de uma função pode dizer (é a diferença do
`rint` pro `nearbyint`). Antes do C99, as exceções de ponto flutuante não
eram especificadas: o erro de domínio tinha que pôr `EDOM`, o estouro tinha
que pôr `ERANGE`, e a perda era definida pela implementação.

Na Jinga, o `errno`, o `EDOM` e o `ERANGE` são do `<errno.h>`, e as exceções
de ponto flutuante são do `fenv.h`, que ainda não está na Jinga. O teste que
vale nos dois sistemas olha o `errno` só onde o `math_errhandling` diz que a
biblioteca o escreve:

```ps
import <math.h>
import <errno.h>

bool pelo_errno = (math_errhandling & MATH_ERRNO) != 0
errno = 0
flo r = sqrt(-1.0)
post(isnan(r) != 0, not pelo_errno or errno == EDOM)
errno = 0
r = log(0.0)
post(r, not pelo_errno or errno == ERANGE)
errno = 0
r = exp(1000.0)
post(r, not pelo_errno or errno == ERANGE)
post((math_errhandling & MATH_ERREXCEPT) != 0)
```

```saida
True True
-inf True
inf True
True
```
