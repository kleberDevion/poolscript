# float.h — limites dos tipos de ponto flutuante

O `float.h` descreve os três tipos de ponto flutuante do C (`float`, `double` e
`long double`): a base, a precisão, a faixa dos expoentes, o maior e o menor
valor, o épsilon, o arredondamento e a precisão das contas. Os nomes valem com
`import <float.h>` no topo do arquivo, igual ao `#include` do C; sem o import,
o nome é `NameError`, com a dica do import que falta
([o C na Jinga](c.md)).

> Esta página segue a cppreference
> ([en.cppreference.com/w/c/types/limits](https://en.cppreference.com/w/c/types/limits),
> com as páginas do `FLT_ROUNDS` e do `FLT_EVAL_METHOD`, licença
> [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)),
> traduzida e adaptada à Jinga. As declarações são as do C11, conferidas
> contra a tabela do motor. Cada exemplo foi rodado.

| Seção | O que tem |
|---|---|
| [`FLT_ROUNDS`](#flt_rounds) | o modo de arredondamento atual |
| [`FLT_EVAL_METHOD`](#flt_eval_method) | a faixa e a precisão em que as contas são feitas |
| [`FLT_RADIX`](#flt_radix) | a base da representação dos três tipos |
| [`DECIMAL_DIG`](#decimal_dig) | os dígitos decimais pra ida e volta do maior tipo |
| [Os limites de cada tipo](#os-limites-de-cada-tipo) | as 36 constantes `FLT_*`, `DBL_*` e `LDBL_*` |

## Na Jinga

- O `flo` da Jinga é o `double` do C (o binário de 64 bits do IEEE 754): as
  constantes `DBL_*` descrevem o `flo`, e a conta com `flo` é feita no
  `double`.
- As constantes de ponto flutuante (`FLT_MAX`, `DBL_EPSILON`...) chegam como
  `flo`; as inteiras (`FLT_DIG`, `DBL_MANT_DIG`...), como `int`. As do `float`
  chegam com o valor exato, que o `double` representa.
- As do `long double` também chegam como `flo` e perdem o que passa do
  `double`. No x86_64 (Linux e macOS Intel), o `long double` vai muito além do
  `double`: na Jinga, o `LDBL_MAX` vira `inf`, e o `LDBL_MIN` e o
  `LDBL_TRUE_MIN` viram `0.0`. As inteiras do `long double` (`LDBL_MANT_DIG`,
  `LDBL_MAX_EXP`...) chegam exatas.
- O `FLT_ROUNDS` é lido na hora, porque o modo de arredondamento pode mudar
  durante o programa.

```ps
import <float.h>

flo x = DBL_MAX
post(x, x * 2)
post(1.0 + DBL_EPSILON > 1.0, 1.0 + DBL_EPSILON / 2 == 1.0)
post(DBL_TRUE_MIN, DBL_TRUE_MIN / 2)
post(type(FLT_EPSILON), type(LDBL_EPSILON), type(DBL_DIG))
```

```saida
1.7976931348623157e+308 inf
True True
5e-324 0.0
flo flo int
```

## Arredondamento e avaliação

### `FLT_ROUNDS`

```c
#define FLT_ROUNDS /* definido pela implementação */
```

O modo de arredondamento atual da aritmética de ponto flutuante:

| Valor | O que diz |
|---|---|
| `-1` | a direção padrão não é conhecida |
| `0` | na direção do zero (o mesmo que o `FE_TOWARDZERO`) |
| `1` | pro mais perto, com o empate indo pro par (o mesmo que o `FE_TONEAREST`) |
| `2` | na direção do infinito positivo (o mesmo que o `FE_UPWARD`) |
| `3` | na direção do infinito negativo (o mesmo que o `FE_DOWNWARD`) |
| outro | comportamento definido pela implementação |

O modo de arredondamento muda pelo `fesetround` (do `fenv.h`, que é do lote
C2), e o `FLT_ROUNDS` acompanha a mudança. Sem mudança, vale o padrão do IEEE
754, o mais perto com o empate pro par.

```ps
import <float.h>

post(FLT_ROUNDS)
```

```saida
1
```

### `FLT_EVAL_METHOD`

```c
#define FLT_EVAL_METHOD /* definido pela implementação */
```

Diz a faixa e a precisão dos valores de ponto flutuante que vêm das constantes
e de todas as operações (os operadores e as conversões implícitas dos
operandos), menos a atribuição, o cast e a chamada de função da biblioteca:

| Valor | O que diz |
|---|---|
| negativo, menos o `-1` | comportamento definido pela implementação |
| `-1` | a precisão padrão não é conhecida |
| `0` | todas as operações e constantes avaliam na faixa e na precisão do tipo usado; o `float_t` e o `double_t` são o `float` e o `double` |
| `1` | todas as operações e constantes avaliam na faixa e na precisão do `double`; o `float_t` e o `double_t` são o `double` |
| `2` | todas as operações e constantes avaliam na faixa e na precisão do `long double`; o `float_t` e o `double_t` são o `long double` |

Qualquer que seja o `FLT_EVAL_METHOD`, uma expressão de ponto flutuante pode
ser contraída, ou seja, calculada como se os resultados intermediários
tivessem faixa e precisão infinitas (a não ser que o
`#pragma STDC FP_CONTRACT` esteja desligado). O cast e a atribuição tiram a
faixa e a precisão a mais: é o modelo de guardar um valor de um registrador de
precisão estendida num lugar da memória de tamanho padrão.

No x86_64 e no ARM de 64 bits, o `FLT_EVAL_METHOD` vale `0`.

```ps
import <float.h>

post(FLT_EVAL_METHOD)
```

```saida
0
```

## Os limites de cada tipo

### `FLT_RADIX`

```c
#define FLT_RADIX /* definido pela implementação */
```

A base (a raiz inteira) da representação dos três tipos de ponto flutuante.
Vale `2` em todo sistema com o IEEE 754, e a precisão de cada tipo se conta em
dígitos dessa base (o `FLT_MANT_DIG` abaixo).

### `DECIMAL_DIG`

```c
#define DECIMAL_DIG /* definido pela implementação */
```

Converter um `long double` em decimal com pelo menos `DECIMAL_DIG` dígitos e
voltar pro `long double` devolve o mesmo valor: é a precisão decimal que basta
pra escrever e ler de volta um valor do maior tipo de ponto flutuante. Vale
`21` no x86_64 (Linux e macOS Intel), o mesmo do `LDBL_DECIMAL_DIG`.

### Os três tipos

Cada linha abaixo é uma característica, com a macro de cada tipo e o valor
dela. Os do `float` e do `double` são os do IEEE 754, os mesmos em todo
sistema; os do `long double` são os do x86_64 (Linux e macOS Intel), onde ele
é o formato estendido de 80 bits do x87. No macOS com Apple Silicon, o
`long double` é o próprio `double`, e os `LDBL_*` são iguais aos `DBL_*`; no
Linux ARM de 64 bits, é o quádruplo do IEEE 754, com 113 dígitos binários de
mantissa.

| O que é | `float` | `double` | `long double` |
|---|---|---|---|
| se o tipo tem os números subnormais: `-1` não se sabe, `0` não tem, `1` tem | `FLT_HAS_SUBNORM`: `1` | `DBL_HAS_SUBNORM`: `1` | `LDBL_HAS_SUBNORM`: `1` |
| quantos dígitos na base `FLT_RADIX` o tipo representa sem perder precisão | `FLT_MANT_DIG`: `24` | `DBL_MANT_DIG`: `53` | `LDBL_MANT_DIG`: `64` |
| com quantos dígitos decimais a ida e volta tipo → decimal → tipo devolve o mesmo valor: a precisão decimal que basta pra escrever e ler de volta | `FLT_DECIMAL_DIG`: `9` | `DBL_DECIMAL_DIG`: `17` | `LDBL_DECIMAL_DIG`: `21` |
| quantos dígitos decimais sobrevivem garantidos à ida e volta texto → tipo → texto, sem mudança por arredondamento ou estouro | `FLT_DIG`: `6` | `DBL_DIG`: `15` | `LDBL_DIG`: `18` |
| o menor inteiro negativo tal que `FLT_RADIX` elevado a ele menos um é um valor normalizado do tipo | `FLT_MIN_EXP`: `-125` | `DBL_MIN_EXP`: `-1021` | `LDBL_MIN_EXP`: `-16381` |
| o menor inteiro negativo tal que `10` elevado a ele é um valor normalizado do tipo | `FLT_MIN_10_EXP`: `-37` | `DBL_MIN_10_EXP`: `-307` | `LDBL_MIN_10_EXP`: `-4931` |
| o maior inteiro positivo tal que `FLT_RADIX` elevado a ele menos um é um valor finito do tipo | `FLT_MAX_EXP`: `128` | `DBL_MAX_EXP`: `1024` | `LDBL_MAX_EXP`: `16384` |
| o maior inteiro positivo tal que `10` elevado a ele é um valor finito do tipo | `FLT_MAX_10_EXP`: `38` | `DBL_MAX_10_EXP`: `308` | `LDBL_MAX_10_EXP`: `4932` |
| o maior valor finito | `FLT_MAX`: `3.4028234663852886e+38` | `DBL_MAX`: `1.7976931348623157e+308` | `LDBL_MAX`: na Jinga, `inf` |
| a diferença entre `1.0` e o próximo valor representável | `FLT_EPSILON`: `1.1920928955078125e-07` | `DBL_EPSILON`: `2.220446049250313e-16` | `LDBL_EPSILON`: `1.0842021724855044e-19` |
| o menor valor positivo normalizado | `FLT_MIN`: `1.1754943508222875e-38` | `DBL_MIN`: `2.2250738585072014e-308` | `LDBL_MIN`: na Jinga, `0.0` |
| o menor valor positivo (subnormal) | `FLT_TRUE_MIN`: `1.401298464324817e-45` | `DBL_TRUE_MIN`: `5e-324` | `LDBL_TRUE_MIN`: na Jinga, `0.0` |

Os valores de ponto flutuante estão como a Jinga os escreve: o número exato do
tipo, na forma mais curta que volta pro mesmo `double`.

```ps
import <float.h>

post(FLT_RADIX, FLT_MANT_DIG, DBL_MANT_DIG)
post(FLT_DIG, DBL_DIG, FLT_DECIMAL_DIG, DBL_DECIMAL_DIG)
post(FLT_MIN_EXP, FLT_MAX_EXP, DBL_MIN_EXP, DBL_MAX_EXP)
post(FLT_MIN_10_EXP, FLT_MAX_10_EXP, DBL_MIN_10_EXP, DBL_MAX_10_EXP)
post(FLT_MAX, FLT_EPSILON, FLT_MIN, FLT_TRUE_MIN)
post(DBL_MAX, DBL_EPSILON, DBL_MIN, DBL_TRUE_MIN)
post(FLT_HAS_SUBNORM, DBL_HAS_SUBNORM)
post(LDBL_MANT_DIG >= DBL_MANT_DIG, LDBL_MAX_EXP >= DBL_MAX_EXP, DECIMAL_DIG >= DBL_DECIMAL_DIG)
```

```saida
2 24 53
6 15 9 17
-125 128 -1021 1024
-37 38 -307 308
3.4028234663852886e+38 1.1920928955078125e-07 1.1754943508222875e-38 1.401298464324817e-45
1.7976931348623157e+308 2.220446049250313e-16 2.2250738585072014e-308 5e-324
1 1
True True True
```

O `DBL_DIG` e o `DBL_DECIMAL_DIG` na prática: com 15 dígitos, o `0.1` sai
como foi escrito; com 17, aparece o valor que o `double` guarda de verdade.

```ps
import <float.h>
import <stdio.h>

printf("%.*g\n", DBL_DIG, 0.1)
printf("%.*g\n", DBL_DECIMAL_DIG, 0.1)
```

```saida
0.1
0.10000000000000001
```
