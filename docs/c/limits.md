# limits.h — limites dos tipos inteiros

O `limits.h` diz a largura do byte e o mínimo e o máximo de cada tipo inteiro
básico do C (`char`, `short`, `int`, `long` e `long long`, com e sem sinal). Os
nomes valem com `import <limits.h>` no topo do arquivo, igual ao `#include`
do C; sem o import, o nome é `NameError`, com a dica do import que falta
([o C na Jinga](c.md)).

> Esta página segue a cppreference
> ([en.cppreference.com/w/c/types/limits](https://en.cppreference.com/w/c/types/limits),
> licença [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)),
> traduzida e adaptada à Jinga. As declarações são as do C11, conferidas
> contra a tabela do motor. Cada exemplo foi rodado.

| Constante | O que é |
|---|---|
| [`CHAR_BIT`](#as-constantes) | a largura do byte, em bits |
| [`MB_LEN_MAX`](#as-constantes) | o máximo de bytes num caractere multibyte |
| [`CHAR_MIN`, `CHAR_MAX`](#as-constantes) | o mínimo e o máximo do `char` |
| [`SCHAR_MIN`, `SHRT_MIN`, `INT_MIN`, `LONG_MIN`, `LLONG_MIN`](#as-constantes) | o mínimo dos tipos com sinal |
| [`SCHAR_MAX`, `SHRT_MAX`, `INT_MAX`, `LONG_MAX`, `LLONG_MAX`](#as-constantes) | o máximo dos tipos com sinal |
| [`UCHAR_MAX`, `USHRT_MAX`, `UINT_MAX`, `ULONG_MAX`, `ULLONG_MAX`](#as-constantes) | o máximo dos tipos sem sinal |

## Na Jinga

- Numa variável da Jinga, o `int`, o `char` e o `long` são os da Jinga: o
  `int x` tem 64 bits e passa do `INT_MAX` sem dar a volta, o `long` da Jinga
  é inteiro de qualquer tamanho, e o `char c = 'a'` é texto.
- Os limites daqui descrevem os tipos do C: valem na memória do C (o que o
  ponteiro aponta, o elemento de array, o campo de `struct`, o cast, o
  `sizeof`) e nos tipos que só o C tem (`short`, `unsigned`...), que valem
  também numa variável da Jinga. Escrever lá um valor fora da faixa dá a volta,
  como na atribuição do C.
- Os valores vêm do compilador C de cada sistema: o `long` tem 8 bytes no
  Linux e no macOS e 4 no Windows.

```ps
import <limits.h>
import <stdlib.h>

int x = INT_MAX
x = x + 1                  # o int da Jinga tem 64 bits
post(x, x > INT_MAX)
int *p = malloc(sizeof(int))
*p = x                     # o int do C tem 32 bits: dá a volta
post(*p == INT_MIN)
free(p)
short s = SHRT_MAX
s = s + 1                  # o short é só do C: dá a volta
post(s == SHRT_MIN)
```

```saida
2147483648 True
True
True
```

## As constantes

Cada uma é uma constante inteira. Fora o `CHAR_BIT` e o `MB_LEN_MAX`, o tipo
dela é o que o tipo descrito ganha na promoção dos inteiros do C: o `CHAR_MAX`
pode ser `int` ou `unsigned int`, mas nunca `char`, e o `USHRT_MAX` não pode
ser de tipo sem sinal (pode ser `int`).

| Constante | O que é | Linux e macOS (64 bits) |
|---|---|---|
| `CHAR_BIT` | a largura do byte, em bits | `8` |
| `MB_LEN_MAX` | o máximo de bytes de um caractere multibyte, em qualquer locale | `16` no Linux, `6` no macOS |
| `CHAR_MIN` | o mínimo do `char` | `-128` no x86_64 e no macOS ARM, onde o `char` tem sinal; `0` no Linux ARM, onde não tem |
| `CHAR_MAX` | o máximo do `char` | `127` no x86_64 e no macOS ARM; `255` no Linux ARM |
| `SCHAR_MIN` | o mínimo do `signed char` | `-128` |
| `SCHAR_MAX` | o máximo do `signed char` | `127` |
| `UCHAR_MAX` | o máximo do `unsigned char` | `255` |
| `SHRT_MIN` | o mínimo do `short` | `-32768` |
| `SHRT_MAX` | o máximo do `short` | `32767` |
| `USHRT_MAX` | o máximo do `unsigned short` | `65535` |
| `INT_MIN` | o mínimo do `int` | `-2147483648` |
| `INT_MAX` | o máximo do `int` | `2147483647` |
| `UINT_MAX` | o máximo do `unsigned int` | `4294967295` |
| `LONG_MIN` | o mínimo do `long` | `-9223372036854775808` |
| `LONG_MAX` | o máximo do `long` | `9223372036854775807` |
| `ULONG_MAX` | o máximo do `unsigned long` | `18446744073709551615` |
| `LLONG_MIN` | o mínimo do `long long` | `-9223372036854775808` |
| `LLONG_MAX` | o máximo do `long long` | `9223372036854775807` |
| `ULLONG_MAX` | o máximo do `unsigned long long` | `18446744073709551615` |

O padrão deixa o valor de cada uma pra implementação, com um piso: o
`CHAR_BIT` tem pelo menos `8`, o `MB_LEN_MAX` pelo menos `1`, o `short` e o
`int` pelo menos 16 bits, o `long` pelo menos 32 e o `long long` pelo menos 64.
No Windows, o `long` e o `unsigned long` têm 32 bits, e o `LONG_MAX` é o
`2147483647`.

```ps
import <limits.h>

post(CHAR_BIT, SCHAR_MIN, SCHAR_MAX, UCHAR_MAX)
post(SHRT_MIN, SHRT_MAX, USHRT_MAX)
post(INT_MIN, INT_MAX, UINT_MAX)
post(LLONG_MIN, LLONG_MAX, ULLONG_MAX)
post(LONG_MAX >= INT_MAX, ULONG_MAX >= UINT_MAX)
post(CHAR_MIN == SCHAR_MIN or CHAR_MIN == 0, MB_LEN_MAX >= 1)
```

```saida
8 -128 127 255
-32768 32767 65535
-2147483648 2147483647 4294967295
-9223372036854775808 9223372036854775807 18446744073709551615
True True
True True
```
