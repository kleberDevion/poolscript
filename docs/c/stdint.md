# stdint.h — inteiros de largura fixa

O `stdint.h` dá nome aos inteiros pela largura em bits (`int32_t`, `uint8_t`,
`int_least16_t`...), define os limites de cada um (`INT32_MAX`, `SIZE_MAX`...)
e as macros que escrevem uma constante no tipo certo (`INT64_C`...). Os nomes
valem com `import <stdint.h>` no topo do arquivo, igual ao `#include` do C;
sem o import, o nome é `NameError`, com a dica do import que falta
([o C na Jinga](c.md)).

> Esta página segue a cppreference
> ([en.cppreference.com/w/c/types/integer](https://en.cppreference.com/w/c/types/integer)
> e [en.cppreference.com/w/c/types/limits](https://en.cppreference.com/w/c/types/limits),
> licença [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)),
> traduzida e adaptada à Jinga. Os protótipos são os do C11 (Anexo B do
> N1570), conferidos contra a tabela do motor. Cada exemplo foi rodado.

| Macro | O que faz |
|---|---|
| [`INT8_C`, `INT16_C`, `INT32_C`, `INT64_C`](#int8_c-int16_c-int32_c-int64_c) | uma constante do tipo `int_leastN_t` |
| [`UINT8_C`, `UINT16_C`, `UINT32_C`, `UINT64_C`](#uint8_c-uint16_c-uint32_c-uint64_c) | uma constante do tipo `uint_leastN_t` |
| [`INTMAX_C`, `UINTMAX_C`](#intmax_c-uintmax_c) | uma constante do tipo `intmax_t` ou `uintmax_t` |

| Seção | O que tem |
|---|---|
| [Tipos](#tipos) | `int8_t` a `uint64_t`, os `least` e os `fast`, `intmax_t`, `intptr_t` |
| [Limites](#limites) | o mínimo e o máximo de cada tipo, e o `SIZE_MAX`, o `PTRDIFF_MAX`... |

## Na Jinga

- O `import <inttypes.h>` também traz estes nomes, como no C, onde o
  `inttypes.h` inclui o `stdint.h`.
- Os tipos daqui valem nos dois lugares: na variável da Jinga
  (`int32_t x = 5`) e na memória do C (o que o ponteiro aponta, o elemento de
  array, o campo de `struct`). Na variável da Jinga, o valor é um `int` da
  Jinga.
- A conta é a da Jinga: `x + 1` com `x` no `INT32_MAX` dá `2147483648`. O C
  manda na hora de guardar: o valor que vai pra um tipo daqui é convertido
  pra faixa dele e dá a volta, como na atribuição do C (o `int8_t x = 300`
  guarda `44`).
- O `flo` não vira um tipo daqui calado: `int8_t x = 1.5` é `TypeError`, que
  pede o `int(x)`.
- Qual tipo básico do C cada nome é muda de sistema pra sistema (a tabela
  está em [Tipos](#tipos)), e os limites dos tipos `fast` acompanham.

```ps
import <stdint.h>

int32_t x = INT32_MAX
post(x + 1)       # a conta é da Jinga
x = x + 1         # guardar no int32_t dá a volta, como no C
post(x)
uint8_t b = 250
b = b + 10
post(b)
```

```saida
2147483648
-2147483648
4
```

Na memória do C, a escrita dá a volta do mesmo jeito:

```ps
import <stdint.h>
import <stdlib.h>

uint16_t *v = calloc(3, sizeof(uint16_t))
v[0] = 65535
v[1] = 65536      # não cabe em 16 bits: dá a volta
v[2] = -1
post(v[0], v[1], v[2], sizeof(uint16_t))
free(v)
```

```saida
65535 0 65535 2
```

## Tipos

| Tipo | O que é |
|---|---|
| `int8_t`, `int16_t`, `int32_t`, `int64_t` | inteiro com sinal de largura exata de 8, 16, 32 e 64 bits, sem bits de enchimento (opcional) |
| `int_fast8_t`, `int_fast16_t`, `int_fast32_t`, `int_fast64_t` | o inteiro com sinal mais rápido com pelo menos 8, 16, 32 e 64 bits |
| `int_least8_t`, `int_least16_t`, `int_least32_t`, `int_least64_t` | o menor inteiro com sinal com pelo menos 8, 16, 32 e 64 bits |
| `intmax_t` | o inteiro com sinal de largura máxima |
| `intptr_t` | inteiro com sinal capaz de guardar um ponteiro pra `void` (opcional) |
| `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t` | inteiro sem sinal de largura exata de 8, 16, 32 e 64 bits, sem bits de enchimento (opcional) |
| `uint_fast8_t`, `uint_fast16_t`, `uint_fast32_t`, `uint_fast64_t` | o inteiro sem sinal mais rápido com pelo menos 8, 16, 32 e 64 bits |
| `uint_least8_t`, `uint_least16_t`, `uint_least32_t`, `uint_least64_t` | o menor inteiro sem sinal com pelo menos 8, 16, 32 e 64 bits |
| `uintmax_t` | o inteiro sem sinal de largura máxima |
| `uintptr_t` | inteiro sem sinal capaz de guardar um ponteiro pra `void` (opcional) |

O tipo marcado como opcional só existe quando a implementação tem um tipo que
sirva; no Linux e no macOS, existem todos. O negativo dos tipos de largura
exata é em complemento de dois, e por isso os limites deles são os mesmos em
todo sistema.

Cada nome é um `typedef` de um tipo básico do C, e qual muda de sistema pra
sistema. Onde o Linux e o macOS diferem:

| Tipo | Linux (64 bits) | macOS |
|---|---|---|
| `int64_t`, `int_least64_t` | `long` | `long long` |
| `int_fast16_t` | `long` (64 bits) | `short` (16 bits) |
| `int_fast32_t` | `long` (64 bits) | `int` (32 bits) |
| `int_fast64_t` | `long` | `long long` |

Os sem sinal acompanham (`uint64_t` é o `unsigned long` no Linux e o
`unsigned long long` no macOS). Nos dois, o `int8_t` é o `signed char`, o
`int16_t` é o `short`, o `int32_t` é o `int`, e o `intmax_t` e o `intptr_t`
são o `long`.

```ps
import <stdint.h>

post(sizeof(int8_t), sizeof(int16_t), sizeof(int32_t), sizeof(int64_t))
post(sizeof(uint_least8_t), sizeof(uint_least16_t), sizeof(intmax_t), sizeof(uintptr_t))
post(sizeof(int_fast8_t), sizeof(int_fast16_t) >= 2, sizeof(int_fast32_t) >= 4)
```

```saida
1 2 4 8
1 2 8 8
1 True True
```

## Limites

Cada macro é uma constante inteira com o limite do tipo, no tipo que ele tem
depois da promoção dos inteiros do C. As dos tipos opcionais existem quando o
tipo existe.

| Macro | O que é | Valor |
|---|---|---|
| `INT8_MIN`, `INT16_MIN`, `INT32_MIN`, `INT64_MIN` | o mínimo do `intN_t` | `-128`, `-32768`, `-2147483648`, `-9223372036854775808` |
| `INT8_MAX`, `INT16_MAX`, `INT32_MAX`, `INT64_MAX` | o máximo do `intN_t` | `127`, `32767`, `2147483647`, `9223372036854775807` |
| `UINT8_MAX`, `UINT16_MAX`, `UINT32_MAX`, `UINT64_MAX` | o máximo do `uintN_t` | `255`, `65535`, `4294967295`, `18446744073709551615` |
| `INT_LEAST8_MIN`, `INT_LEAST16_MIN`, `INT_LEAST32_MIN`, `INT_LEAST64_MIN` | o mínimo do `int_leastN_t` | no Linux e no macOS, os do `intN_t` |
| `INT_LEAST8_MAX`, `INT_LEAST16_MAX`, `INT_LEAST32_MAX`, `INT_LEAST64_MAX` | o máximo do `int_leastN_t` | no Linux e no macOS, os do `intN_t` |
| `UINT_LEAST8_MAX`, `UINT_LEAST16_MAX`, `UINT_LEAST32_MAX`, `UINT_LEAST64_MAX` | o máximo do `uint_leastN_t` | no Linux e no macOS, os do `uintN_t` |
| `INT_FAST8_MIN`, `INT_FAST16_MIN`, `INT_FAST32_MIN`, `INT_FAST64_MIN` | o mínimo do `int_fastN_t` | o do tipo de cada sistema |
| `INT_FAST8_MAX`, `INT_FAST16_MAX`, `INT_FAST32_MAX`, `INT_FAST64_MAX` | o máximo do `int_fastN_t` | o do tipo de cada sistema: o `INT_FAST16_MAX` é `9223372036854775807` no Linux e `32767` no macOS |
| `UINT_FAST8_MAX`, `UINT_FAST16_MAX`, `UINT_FAST32_MAX`, `UINT_FAST64_MAX` | o máximo do `uint_fastN_t` | o do tipo de cada sistema |
| `INTMAX_MIN`, `INTMAX_MAX` | o mínimo e o máximo do `intmax_t` | os de 64 bits no Linux e no macOS |
| `UINTMAX_MAX` | o máximo do `uintmax_t` | `18446744073709551615` no Linux e no macOS |
| `INTPTR_MIN`, `INTPTR_MAX` | o mínimo e o máximo do `intptr_t` | os de 64 bits no Linux e no macOS |
| `UINTPTR_MAX` | o máximo do `uintptr_t` | `18446744073709551615` no Linux e no macOS |
| `PTRDIFF_MIN`, `PTRDIFF_MAX` | o mínimo e o máximo do `ptrdiff_t` (do `stddef.h`) | os de 64 bits no Linux e no macOS |
| `SIZE_MAX` | o máximo do `size_t` | `18446744073709551615` no Linux e no macOS |
| `SIG_ATOMIC_MIN`, `SIG_ATOMIC_MAX` | o mínimo e o máximo do `sig_atomic_t` (do `signal.h`) | `-2147483648` e `2147483647` no Linux e no macOS |
| `WCHAR_MIN`, `WCHAR_MAX` | o mínimo e o máximo do `wchar_t` | `-2147483648` e `2147483647` no Linux (x86_64) e no macOS |
| `WINT_MIN`, `WINT_MAX` | o mínimo e o máximo do `wint_t` (do `wchar.h`) | `0` e `4294967295` no Linux; `-2147483648` e `2147483647` no macOS |

```ps
import <stdint.h>

post(INT8_MIN, INT8_MAX, UINT8_MAX)
post(INT16_MIN, INT16_MAX, UINT16_MAX)
post(INT32_MIN, INT32_MAX, UINT32_MAX)
post(INT64_MIN, INT64_MAX, UINT64_MAX)
post(INT_LEAST16_MAX == INT16_MAX, INT_FAST16_MAX >= INT16_MAX)
post(SIZE_MAX == UINTPTR_MAX, INTMAX_MAX == INT64_MAX, PTRDIFF_MAX == INTPTR_MAX)
```

```saida
-128 127 255
-32768 32767 65535
-2147483648 2147483647 4294967295
-9223372036854775808 9223372036854775807 18446744073709551615
True True
True True True
```

## Macros de constante

Cada uma expande pra uma expressão constante inteira do tipo indicado, com o
valor dado pelo argumento. No C, o argumento é uma constante inteira sem
sufixo, que cabe no tipo, e a macro só põe o sufixo certo (`UINT64_C(0x123)`
pode expandir pra `0x123ULL` ou pra `0x123UL`). Na Jinga, a macro se chama como
função e devolve um `int` da Jinga; o protótipo diz o tipo que o argumento
recebe. O tipo de 8 e de 16 bits sobe pro `int` na promoção dos inteiros, e
por isso o protótipo dessas macros diz `int`.

### `INT8_C`, `INT16_C`, `INT32_C`, `INT64_C`

```c
int INT8_C(int value);
int INT16_C(int value);
int_least32_t INT32_C(int_least32_t value);
int_least64_t INT64_C(int_least64_t value);
```

Expandem pra uma expressão constante inteira do tipo `int_leastN_t`, com o
valor dado pelo argumento.

| Parâmetro | O que é |
|---|---|
| `value` | a constante inteira, que cabe no `int_leastN_t` |

**Retorno:** o valor de `value`, no tipo `int_leastN_t` (na Jinga, um `int`).

```ps
import <stdint.h>

post(INT8_C(-5), INT16_C(1000), INT32_C(2147483647), INT64_C(-9223372036854775807))
post(type(INT64_C(1)))
```

```saida
-5 1000 2147483647 -9223372036854775807
int
```

### `UINT8_C`, `UINT16_C`, `UINT32_C`, `UINT64_C`

```c
int UINT8_C(int value);
int UINT16_C(int value);
uint_least32_t UINT32_C(uint_least32_t value);
uint_least64_t UINT64_C(uint_least64_t value);
```

Expandem pra uma expressão constante inteira do tipo `uint_leastN_t`, com o
valor dado pelo argumento.

| Parâmetro | O que é |
|---|---|
| `value` | a constante inteira, que cabe no `uint_leastN_t` |

**Retorno:** o valor de `value`, no tipo `uint_leastN_t` (na Jinga, um `int`).

```ps
import <stdint.h>

post(UINT8_C(255), UINT16_C(65535), UINT32_C(4294967295), UINT64_C(0x123))
```

```saida
255 65535 4294967295 291
```

### `INTMAX_C`, `UINTMAX_C`

```c
intmax_t INTMAX_C(intmax_t value);
uintmax_t UINTMAX_C(uintmax_t value);
```

Expandem pra uma expressão constante inteira do tipo `intmax_t` e
`uintmax_t`, com o valor dado pelo argumento.

| Parâmetro | O que é |
|---|---|
| `value` | a constante inteira, que cabe no `intmax_t` ou no `uintmax_t` |

**Retorno:** o valor de `value`, no tipo `intmax_t` ou `uintmax_t` (na Jinga,
um `int`).

```ps
import <stdint.h>

post(INTMAX_C(9223372036854775807), UINTMAX_C(18446744073709551615))
```

```saida
9223372036854775807 18446744073709551615
```

## Fica de fora

| Nome | Por quê |
|---|---|
| `RSIZE_MAX` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |

As macros de largura (`INT8_WIDTH`, `UINTMAX_WIDTH`...) entraram no C23 e não
fazem parte do C11, que a Jinga segue.
