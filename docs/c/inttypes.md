# inttypes.h — conversão dos inteiros de largura fixa

O `inttypes.h` traz o `stdint.h` inteiro e acrescenta as macros de formato do
`printf` e do `scanf` pros inteiros de largura fixa (`PRId64`, `SCNu32`...),
duas funções pro `intmax_t` (`imaxabs` e `imaxdiv`) e a leitura de texto em
`intmax_t` e `uintmax_t` (`strtoimax` e família). Os nomes valem com
`import <inttypes.h>` no topo do arquivo, igual ao `#include` do C; sem o
import, o nome é `NameError`, com a dica do import que falta
([o C na Jinga](c.md)).

> Esta página segue a cppreference
> ([en.cppreference.com/w/c/types/integer](https://en.cppreference.com/w/c/types/integer)
> e as páginas de cada função, licença
> [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)),
> traduzida e adaptada à Jinga. Os protótipos são os do C11 (Anexo B do
> N1570), conferidos contra a tabela do motor. Cada exemplo foi rodado.

| Função | O que faz |
|---|---|
| [`imaxabs`](#imaxabs) | o valor absoluto de um `intmax_t` |
| [`imaxdiv`](#imaxdiv) | o quociente e o resto de uma divisão, de uma vez |
| [`strtoimax`, `strtoumax`](#strtoimax-strtoumax) | texto em `intmax_t` ou `uintmax_t` |
| [`wcstoimax`, `wcstoumax`](#wcstoimax-wcstoumax) | texto largo (`wchar_t`) em `intmax_t` ou `uintmax_t` |

| Tipo ou macro | O que é |
|---|---|
| [`imaxdiv_t`](#imaxdiv_t) | o quociente e o resto que o `imaxdiv` devolve |
| [`PRId64`, `SCNu32`...](#macros-de-formato) | as 154 macros de formato do `printf` e do `scanf` |

## Na Jinga

- O `import <inttypes.h>` traz também todos os nomes do `stdint.h`
  (`int64_t`, `INT64_MAX`, `INT64_C`...), como no C, onde o `inttypes.h`
  inclui o `stdint.h`.
- As macros de formato são texto: `str` na Jinga. No C, o formato se monta
  pondo um literal ao lado do outro (`"%" PRId64`); na Jinga, com o `+`
  (`"%" + PRId64`). Com o formato montado, a conferência dos argumentos do
  `printf` é na chamada.
- O valor de cada macro depende do sistema (a tabela está em
  [Macros de formato](#macros-de-formato)): é pra isso que elas existem.
- O `intmax_t` e o `uintmax_t` têm 64 bits no Linux e no macOS. Numa variável
  da Jinga (`intmax_t n = 5`), o valor é um `int` da Jinga.
- O `NULL` não vem com o `inttypes.h`: ele é do `stddef.h` (e dos outros
  cabeçalhos que o C diz que o definem), assim como o `wchar_t`. O `errno` e o
  `ERANGE` são do `errno.h`.
- A Jinga não tem o texto largo literal do C (`L"..."`), e o `wchar.h` é do
  lote C2: o texto do `wcstoimax` se monta num array de `wchar_t`, um
  `ord()` por caractere.

```ps
import <inttypes.h>

int64_t n = INT64_MAX          # o int64_t e o INT64_MAX são do stdint.h
post(n, imaxabs(INT32_MIN))
```

```saida
9223372036854775807 2147483648
```

## Funções

### `imaxabs`

```c
intmax_t imaxabs(intmax_t j);
```

Calcula o valor absoluto de um inteiro. O comportamento é indefinido se o
resultado não cabe no tipo de retorno.

| Parâmetro | O que é |
|---|---|
| `j` | o valor inteiro |

**Retorno:** o valor absoluto de `j` (`|j|`), quando ele cabe no `intmax_t`.

No complemento de dois, o valor absoluto do número mais negativo fica fora da
faixa: o `INTMAX_MIN` é `-9223372036854775808` no Linux e no macOS, e o
resultado, `9223372036854775808`, passa do `INTMAX_MAX`, que é
`9223372036854775807`.

```ps
import <inttypes.h>

post(imaxabs(-42), imaxabs(42))
post(imaxabs(INTMAX_MIN + 1) == INTMAX_MAX)
```

```saida
42 42
True
```

### `imaxdiv`

```c
imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom);
```

Calcula o quociente e o resto da divisão de `numer` por `denom`, de uma vez. O
quociente é o quociente algébrico com a parte fracionária descartada
(truncado na direção do zero), e o resto é tal que `quot * denom + rem` dá
`numer`.

| Parâmetro | O que é |
|---|---|
| `numer` | o dividendo |
| `denom` | o divisor |

**Retorno:** se o quociente e o resto cabem no `intmax_t`, os dois numa
`imaxdiv_t`; se algum não cabe, o comportamento é indefinido.

Em muitas plataformas, uma instrução só do processador dá o quociente e o
resto, e o `imaxdiv` pode aproveitar isso.

```ps
import <inttypes.h>

imaxdiv_t r = imaxdiv(17, 5)
post(r.quot, r.rem)
r = imaxdiv(-7, 2)
post(r.quot, r.rem)
```

```saida
3 2
-3 -1
```

### `strtoimax`, `strtoumax`

```c
intmax_t strtoimax(const char * restrict nptr, char ** restrict endptr, int base);
uintmax_t strtoumax(const char * restrict nptr, char ** restrict endptr, int base);
```

Interpretam um valor inteiro no texto de bytes apontado por `nptr`.

Descartam os brancos do começo (os que o `isspace` reconhece) até o primeiro
caractere que não é branco e então pegam o máximo de caracteres que formam um
inteiro válido na base `base`, convertendo-os num valor inteiro. O inteiro
válido tem estas partes:

- um sinal de mais ou de menos (opcional);
- o prefixo `0`, de base octal (opcional; vale só quando a base é `8` ou `0`);
- o prefixo `0x` ou `0X`, de base hexadecimal (opcional; vale só quando a base
  é `16` ou `0`);
- uma sequência de dígitos.

Os valores válidos de `base` são `{0, 2, 3, ..., 36}`. Os dígitos válidos da
base `2` são `{0, 1}`, os da base `3` são `{0, 1, 2}`, e assim por diante. Nas
bases acima de `10`, os dígitos incluem as letras, de `Aa` na base `11` a
`Zz` na base `36`, sem diferença entre maiúscula e minúscula.

O locale do C instalado pode aceitar outros formatos numéricos.

Se `base` é `0`, a base é detectada: com o prefixo `0`, octal; com `0x` ou
`0X`, hexadecimal; sem prefixo, decimal.

Se o sinal de menos fazia parte da entrada, o valor calculado dos dígitos é
negado como pelo menos unário no tipo do resultado.

As funções põem no ponteiro apontado por `endptr` o endereço do caractere
depois do último interpretado. Se `endptr` é um ponteiro nulo, ele é
ignorado. Se o texto é vazio ou não tem a forma esperada, nada é convertido, e
(se `endptr` não é nulo) o próprio `nptr` vai pro `*endptr`.

| Parâmetro | O que é |
|---|---|
| `nptr` | ponteiro pro texto terminado em nulo a interpretar |
| `endptr` | ponteiro pra um ponteiro de caractere, ou `NULL` |
| `base` | a base do inteiro |

**Retorno:**
- se deu certo, o valor inteiro do texto;
- se o valor convertido sai da faixa do tipo de retorno, um erro de faixa: o
  `errno` recebe o `ERANGE`, e a função devolve o `INTMAX_MAX`, o
  `INTMAX_MIN`, o `UINTMAX_MAX` ou `0`, conforme o caso;
- se nada pôde ser convertido, `0`.

Na Jinga, o `endptr` é um `char *` declarado com `NULL`, passado com `&`; o
`str(fim)` lê o que sobrou do texto.

```ps
import <inttypes.h>
import <stddef.h>
import <errno.h>

char *fim = NULL
post(strtoimax(" -123junk", &fim, 10), str(fim))    # base 10
post(strtoimax("11111111", NULL, 2))                 # base 2
post(strtoimax("XyZ", NULL, 36))                     # base 36
post(strtoimax("010", NULL, 0), strtoimax("10", NULL, 0), strtoimax("0x10", NULL, 0))
errno = 0
post(strtoimax("9223372036854775808", NULL, 10), errno == ERANGE)
post(strtoumax("18446744073709551615", NULL, 10))
```

```saida
-123 junk
255
44027
8 10 16
9223372036854775807 True
18446744073709551615
```

### `wcstoimax`, `wcstoumax`

```c
intmax_t wcstoimax(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
uintmax_t wcstoumax(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
```

Interpretam um valor inteiro no texto largo apontado por `nptr`, com as mesmas
regras do [`strtoimax`](#strtoimax-strtoumax): os brancos do começo (os que o
`iswspace` reconhece) são descartados; o sinal, o prefixo `0` ou `0x` e os
dígitos vêm em seguida; `base` vai de `2` a `36`, ou `0` pra detectar pelo
prefixo; o `*endptr` recebe o endereço do caractere largo depois do último
interpretado. Se o sinal de menos fazia parte da entrada, o valor é negado no
tipo do resultado, e no `wcstoumax` isso dá a volta do inteiro sem sinal.

| Parâmetro | O que é |
|---|---|
| `nptr` | ponteiro pro texto largo terminado em nulo a interpretar |
| `endptr` | ponteiro pra um ponteiro de caractere largo, ou `NULL` |
| `base` | a base do inteiro |

**Retorno:** o valor inteiro do texto, se deu certo. Se o valor sai da faixa do
tipo de retorno, um erro de faixa (o `errno` recebe o `ERANGE`), e a função
devolve o `INTMAX_MAX`, o `INTMAX_MIN`, o `UINTMAX_MAX` ou `0`, conforme o
caso. Se nada pôde ser convertido, `0`.

O texto largo se monta num array de `wchar_t` (o `wchar_t` é do `stddef.h`),
um caractere por elemento, e o elemento zerado depois do último é o nulo do
fim:

```ps
import <inttypes.h>
import <stddef.h>

wchar_t w[16]
str s = " -123junk"
int i = 0
for each ch in s {
    w[i] = ord(ch)
    i++
}
wchar_t *fim = NULL
post(wcstoimax(w, &fim, 10), chr(fim[0]))
wchar_t h[8] = {ord("0"), ord("x"), ord("f"), ord("f")}
post(wcstoumax(h, NULL, 0))
```

```saida
-123 j
255
```

## Tipo

### `imaxdiv_t`

```c
typedef struct {
    intmax_t quot;
    intmax_t rem;
} imaxdiv_t;
```

A estrutura que o `imaxdiv` devolve, com o quociente (`quot`) e o resto
(`rem`). O padrão manda ter os dois campos, em qualquer ordem: use os nomes,
não a posição. O exemplo está no [`imaxdiv`](#imaxdiv).

## Macros de formato

Cada macro é o texto da conversão do `printf` ou do `scanf` pra um tipo do
`stdint.h`, sem o `%`: o `%` e as opções (`+`, a largura...) vêm antes, no
formato. O nome junta três partes:

| Parte do nome | Valores | O que diz |
|---|---|---|
| a família | `PRI` | as funções do `printf` (`printf`, `fprintf`, `sprintf`...) |
| | `SCN` | as funções do `scanf` (`scanf`, `fscanf`, `sscanf`...) |
| a conversão | `d`, `i` | inteiro com sinal, em decimal; no `scanf`, o `i` lê a base pelo prefixo (`0` octal, `0x` hexadecimal) |
| | `o`, `u`, `x`, `X` | inteiro sem sinal: em octal, em decimal, em hexadecimal com minúsculas e com maiúsculas; o `X` é só do `printf` (no `scanf`, o `x` lê as duas) |
| o tipo | `8`, `16`, `32`, `64` | `int8_t` a `int64_t`, `uint8_t` a `uint64_t` |
| | `LEAST8` a `LEAST64` | `int_least8_t` a `int_least64_t` e os sem sinal |
| | `FAST8` a `FAST64` | `int_fast8_t` a `int_fast64_t` e os sem sinal |
| | `MAX` | `intmax_t`, `uintmax_t` |
| | `PTR` | `intptr_t`, `uintptr_t` |

Cada macro `PRI` existe quando o sistema define o tipo dela, e cada `SCN`
quando, além disso, o `fscanf` tem um modificador de tamanho pro tipo. No
Linux e no macOS, existem as 154:

| Tipo | `printf`, com sinal | `printf`, sem sinal | `scanf`, com sinal | `scanf`, sem sinal |
|---|---|---|---|---|
| `int8_t`, `uint8_t` | `PRId8` `PRIi8` | `PRIo8` `PRIu8` `PRIx8` `PRIX8` | `SCNd8` `SCNi8` | `SCNo8` `SCNu8` `SCNx8` |
| `int16_t`, `uint16_t` | `PRId16` `PRIi16` | `PRIo16` `PRIu16` `PRIx16` `PRIX16` | `SCNd16` `SCNi16` | `SCNo16` `SCNu16` `SCNx16` |
| `int32_t`, `uint32_t` | `PRId32` `PRIi32` | `PRIo32` `PRIu32` `PRIx32` `PRIX32` | `SCNd32` `SCNi32` | `SCNo32` `SCNu32` `SCNx32` |
| `int64_t`, `uint64_t` | `PRId64` `PRIi64` | `PRIo64` `PRIu64` `PRIx64` `PRIX64` | `SCNd64` `SCNi64` | `SCNo64` `SCNu64` `SCNx64` |
| `int_least8_t`, `uint_least8_t` | `PRIdLEAST8` `PRIiLEAST8` | `PRIoLEAST8` `PRIuLEAST8` `PRIxLEAST8` `PRIXLEAST8` | `SCNdLEAST8` `SCNiLEAST8` | `SCNoLEAST8` `SCNuLEAST8` `SCNxLEAST8` |
| `int_least16_t`, `uint_least16_t` | `PRIdLEAST16` `PRIiLEAST16` | `PRIoLEAST16` `PRIuLEAST16` `PRIxLEAST16` `PRIXLEAST16` | `SCNdLEAST16` `SCNiLEAST16` | `SCNoLEAST16` `SCNuLEAST16` `SCNxLEAST16` |
| `int_least32_t`, `uint_least32_t` | `PRIdLEAST32` `PRIiLEAST32` | `PRIoLEAST32` `PRIuLEAST32` `PRIxLEAST32` `PRIXLEAST32` | `SCNdLEAST32` `SCNiLEAST32` | `SCNoLEAST32` `SCNuLEAST32` `SCNxLEAST32` |
| `int_least64_t`, `uint_least64_t` | `PRIdLEAST64` `PRIiLEAST64` | `PRIoLEAST64` `PRIuLEAST64` `PRIxLEAST64` `PRIXLEAST64` | `SCNdLEAST64` `SCNiLEAST64` | `SCNoLEAST64` `SCNuLEAST64` `SCNxLEAST64` |
| `int_fast8_t`, `uint_fast8_t` | `PRIdFAST8` `PRIiFAST8` | `PRIoFAST8` `PRIuFAST8` `PRIxFAST8` `PRIXFAST8` | `SCNdFAST8` `SCNiFAST8` | `SCNoFAST8` `SCNuFAST8` `SCNxFAST8` |
| `int_fast16_t`, `uint_fast16_t` | `PRIdFAST16` `PRIiFAST16` | `PRIoFAST16` `PRIuFAST16` `PRIxFAST16` `PRIXFAST16` | `SCNdFAST16` `SCNiFAST16` | `SCNoFAST16` `SCNuFAST16` `SCNxFAST16` |
| `int_fast32_t`, `uint_fast32_t` | `PRIdFAST32` `PRIiFAST32` | `PRIoFAST32` `PRIuFAST32` `PRIxFAST32` `PRIXFAST32` | `SCNdFAST32` `SCNiFAST32` | `SCNoFAST32` `SCNuFAST32` `SCNxFAST32` |
| `int_fast64_t`, `uint_fast64_t` | `PRIdFAST64` `PRIiFAST64` | `PRIoFAST64` `PRIuFAST64` `PRIxFAST64` `PRIXFAST64` | `SCNdFAST64` `SCNiFAST64` | `SCNoFAST64` `SCNuFAST64` `SCNxFAST64` |
| `intmax_t`, `uintmax_t` | `PRIdMAX` `PRIiMAX` | `PRIoMAX` `PRIuMAX` `PRIxMAX` `PRIXMAX` | `SCNdMAX` `SCNiMAX` | `SCNoMAX` `SCNuMAX` `SCNxMAX` |
| `intptr_t`, `uintptr_t` | `PRIdPTR` `PRIiPTR` | `PRIoPTR` `PRIuPTR` `PRIxPTR` `PRIXPTR` | `SCNdPTR` `SCNiPTR` | `SCNoPTR` `SCNuPTR` `SCNxPTR` |

O texto de cada macro é o modificador de tamanho que o tipo pede no sistema,
seguido da letra da conversão. Muda de um sistema pro outro, porque o mesmo
`int64_t` é o `long` no Linux e o `long long` no macOS:

| Macro | Linux (64 bits) | macOS |
|---|---|---|
| `PRId64` | `"ld"` | `"lld"` |
| `PRIdMAX` | `"ld"` | `"jd"` |
| `PRIdFAST16` | `"ld"` | `"hd"` |
| `PRIX8` | `"X"` | `"hhX"` |
| `SCNd8` | `"hhd"` | `"hhd"` |

Escrever `"%lld"` à mão funciona num sistema e não no outro; o
`"%" + PRId64` funciona nos dois.

```ps
import <inttypes.h>
import <stdio.h>

int64_t n = INT64_MIN
printf("%" + PRId64 + "\n", n)
printf("%+" + PRId64 + "\n", INT64_MAX)
uint32_t m = UINT32_MAX
printf("%" + PRIu32 + " = 0x%" + PRIX32 + "\n", m, m)
int32_t a = 0
uint16_t b = 0
int lidos = sscanf("-123 ff", "%" + SCNd32 + " %" + SCNx16, &a, &b)
post(lidos, a, b)
post(type(PRId64), PRId64.endswith("d"))
```

```saida
-9223372036854775808
+9223372036854775807
4294967295 = 0xFFFFFFFF
2 -123 255
str True
```
