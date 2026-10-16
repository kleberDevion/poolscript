# ctype.h — classificação de caracteres

As funções do `ctype.h` dizem a que classe um caractere pertence (letra,
dígito, branco, pontuação...) e trocam a caixa de uma letra. Como toda a
biblioteca padrão do C, elas valem com o cabeçalho importado no topo do
arquivo, igual ao `#include <ctype.h>` do C ([o C na Jinga](c.md)):

```ps
import <ctype.h>

post(isdigit(ord("7")) != 0)
```

```saida
True
```

> Esta página segue a cppreference
> ([en.cppreference.com/w/c/string/byte](https://en.cppreference.com/w/c/string/byte),
> licença [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)),
> traduzida e adaptada à Jinga. Os protótipos são os do C11 (Anexo B do
> N1570), conferidos contra a tabela do motor. Cada exemplo foi rodado.

| Função | O que faz |
|---|---|
| [`isalnum`](#isalnum) | letra ou dígito? |
| [`isalpha`](#isalpha) | letra? |
| [`isblank`](#isblank) | espaço ou tabulação? |
| [`iscntrl`](#iscntrl) | caractere de controle? |
| [`isdigit`](#isdigit) | dígito decimal? |
| [`isgraph`](#isgraph) | tem desenho (imprimível, menos o espaço)? |
| [`islower`](#islower) | letra minúscula? |
| [`isprint`](#isprint) | imprimível (inclui o espaço)? |
| [`ispunct`](#ispunct) | pontuação? |
| [`isspace`](#isspace) | branco (espaço, quebra de linha, tabulação...)? |
| [`isupper`](#isupper) | letra maiúscula? |
| [`isxdigit`](#isxdigit) | dígito hexadecimal? |
| [`tolower`](#tolower) | a letra em minúscula |
| [`toupper`](#toupper) | a letra em maiúscula |

## Na Jinga

- Sem o `import <ctype.h>`, o nome é `NameError` com a dica do import que
  falta: `NameError: name 'isdigit' is not defined. Did you forget to import
  <ctype.h>?`. Os exemplos que usam nome de outro cabeçalho importam ele
  também (o `EOF` é do `<stdio.h>`, o `strcpy` é do `<string.h>`).
- O argumento é um `int`: o código do caractere, como o C o vê (um
  `unsigned char` ou o `EOF`). O caractere de um texto da Jinga é `str`, e
  entra pelo `ord()`: `isdigit(ord("7"))`. O byte de um buffer do C
  (`char buf[64]`) já é número: `isdigit(buf[0])`.
- O valor fora da faixa de um `unsigned char` que não é o `EOF` é
  comportamento indefinido no C, e é o mesmo na Jinga: a função do C é
  chamada direto.
- As funções de teste devolvem **diferente de zero** pra sim e **zero** pra
  não. Qual número sai no "sim" depende do sistema; compare com `!= 0`.
- A Jinga não muda o locale (o `locale.h` ainda não está na Jinga): vale o
  locale "C", e as classes são as do padrão, só ASCII. Nenhuma função daqui
  mexe no `errno`.
- Pra texto da Jinga, os métodos do `str` (`isdigit`, `isalpha`, `upper`...)
  olham a tabela do Unicode inteira; as funções daqui olham um byte no locale
  "C" (o `"á"` não é letra pra elas).

## Classificação

### `isalnum`

```c
int isalnum(int c);
```

Confere se o caractere é alfanumérico pelo locale atual do C. No locale "C",
são alfanuméricos os dígitos (`0123456789`), as letras maiúsculas
(`ABCDEFGHIJKLMNOPQRSTUVWXYZ`) e as minúsculas (`abcdefghijklmnopqrstuvwxyz`).

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a classificar |

**Retorno:** diferente de zero se o caractere é alfanumérico; zero se não é.

```ps
import <ctype.h>

for each ch in "a1B?" {
    post(ch, isalnum(ord(ch)) != 0)
}
```

```saida
a True
1 True
B True
? False
```

### `isalpha`

```c
int isalpha(int c);
```

Confere se o caractere é alfabético: uma letra maiúscula
(`ABCDEFGHIJKLMNOPQRSTUVWXYZ`) ou minúscula (`abcdefghijklmnopqrstuvwxyz`).
Num locale que não é o "C", é alfabético também o que o locale diz que é; em
qualquer locale, `iscntrl`, `isdigit`, `ispunct` e `isspace` dão zero pra um
caractere alfabético.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a classificar |

**Retorno:** diferente de zero se o caractere é alfabético; zero se não é.

```ps
import <ctype.h>
import <stdio.h>

str s = "Olá, Jinga!"
int letras = 0
for each ch in s {
    if ord(ch) < 128 and isalpha(ord(ch)) != 0 {
        letras = letras + 1
    }
}
post(letras)
post(isalpha(EOF) != 0)
```

```saida
7
False
```

### `isblank`

```c
int isblank(int c);
```

Confere se o caractere é um branco que separa palavras numa linha. No locale
"C", só o espaço (`0x20`) e a tabulação horizontal (`0x09`) são brancos.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a classificar |

**Retorno:** diferente de zero se o caractere é branco; zero se não é.

```ps
import <ctype.h>

post(isblank(ord(" ")) != 0, isblank(ord("\t")) != 0, isblank(ord("\n")) != 0)
```

```saida
True True False
```

### `iscntrl`

```c
int iscntrl(int c);
```

Confere se o caractere é de controle: no locale "C", os códigos `0x00` a
`0x1F` e o `0x7F` (DEL).

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a classificar |

**Retorno:** diferente de zero se o caractere é de controle; zero se não é.

```ps
import <ctype.h>

post(iscntrl(10) != 0, iscntrl(127) != 0, iscntrl(ord("a")) != 0)
```

```saida
True True False
```

### `isdigit`

```c
int isdigit(int c);
```

Confere se o caractere é um dígito decimal (`0123456789`). É a única
classe que não depende do locale.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a classificar |

**Retorno:** diferente de zero se o caractere é um dígito; zero se não é.

```ps
import <ctype.h>
import <string.h>

char buf[16]
strcpy(buf, "ano 2026")
int i = 0
str digitos = ""
while (buf[i] != 0) {
    if isdigit(buf[i]) != 0 {
        digitos = digitos + chr(buf[i])
    }
    i++
}
post(digitos)
```

```saida
2026
```

### `isgraph`

```c
int isgraph(int c);
```

Confere se o caractere tem representação gráfica: um dígito, uma letra
maiúscula ou minúscula, uma pontuação
(``!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~``) ou outro caractere gráfico do
locale. O espaço não é gráfico.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a classificar |

**Retorno:** diferente de zero se o caractere é gráfico; zero se não é.

```ps
import <ctype.h>

post(isgraph(ord("a")) != 0, isgraph(ord("~")) != 0, isgraph(ord(" ")) != 0)
```

```saida
True True False
```

### `islower`

```c
int islower(int c);
```

Confere se o caractere é uma letra minúscula pelo locale atual. No locale
"C", só `abcdefghijklmnopqrstuvwxyz`. Quando `islower` dá diferente de zero,
`iscntrl`, `isdigit`, `ispunct` e `isspace` dão zero pro mesmo caractere.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a classificar |

**Retorno:** diferente de zero se o caractere é minúsculo; zero se não é.

```ps
import <ctype.h>

post(islower(ord("a")) != 0, islower(ord("A")) != 0, islower(ord("1")) != 0)
```

```saida
True False False
```

### `isprint`

```c
int isprint(int c);
```

Confere se o caractere é imprimível: um dígito, uma letra, uma pontuação, o
espaço ou outro caractere imprimível do locale. É o `isgraph` mais o espaço.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a classificar |

**Retorno:** diferente de zero se o caractere é imprimível; zero se não é.

```ps
import <ctype.h>

post(isprint(ord(" ")) != 0, isprint(ord("a")) != 0, isprint(7) != 0)
```

```saida
True True False
```

### `ispunct`

```c
int ispunct(int c);
```

Confere se o caractere é uma pontuação pelo locale atual. No locale "C", são
pontuação ``!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~``.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a classificar |

**Retorno:** diferente de zero se o caractere é pontuação; zero se não é.

```ps
import <ctype.h>

str s = "olá, mundo! tudo bem?"
int n = 0
for each ch in s {
    if ord(ch) < 128 and ispunct(ord(ch)) != 0 {
        n = n + 1
    }
}
post(n)
```

```saida
3
```

### `isspace`

```c
int isspace(int c);
```

Confere se o caractere é um branco: no locale "C", o espaço (`0x20`), o
avanço de página (`0x0c`), a quebra de linha (`0x0a`), o retorno de carro
(`0x0d`), a tabulação horizontal (`0x09`) e a vertical (`0x0b`).

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a classificar |

**Retorno:** diferente de zero se o caractere é branco; zero se não é.

```ps
import <ctype.h>

post(isspace(ord("\n")) != 0, isspace(ord("\r")) != 0, isspace(ord("x")) != 0)
```

```saida
True True False
```

### `isupper`

```c
int isupper(int c);
```

Confere se o caractere é uma letra maiúscula pelo locale atual. No locale
"C", só `ABCDEFGHIJKLMNOPQRSTUVWXYZ`.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a classificar |

**Retorno:** diferente de zero se o caractere é maiúsculo; zero se não é.

```ps
import <ctype.h>

post(isupper(ord("A")) != 0, isupper(ord("a")) != 0)
```

```saida
True False
```

### `isxdigit`

```c
int isxdigit(int c);
```

Confere se o caractere é um dígito hexadecimal (`0123456789abcdefABCDEF`) ou
outro que o locale classifique como hexadecimal.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a classificar |

**Retorno:** diferente de zero se o caractere é um dígito hexadecimal; zero
se não é.

```ps
import <ctype.h>

post(isxdigit(ord("F")) != 0, isxdigit(ord("7")) != 0, isxdigit(ord("g")) != 0)
```

```saida
True True False
```

## Conversão

### `tolower`

```c
int tolower(int c);
```

Converte o caractere em minúscula pelas regras do locale atual. No locale
"C", as letras `ABCDEFGHIJKLMNOPQRSTUVWXYZ` viram
`abcdefghijklmnopqrstuvwxyz`.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a converter |

**Retorno:** a minúscula de `c`, ou o próprio `c` quando o locale não tem
minúscula pra ele (o `EOF` volta `EOF`).

```ps
import <ctype.h>
import <stdio.h>

post(chr(tolower(ord("A"))), chr(tolower(ord("a"))), chr(tolower(ord("1"))), tolower(EOF))
```

```saida
a a 1 -1
```

### `toupper`

```c
int toupper(int c);
```

Converte o caractere em maiúscula pelas regras do locale atual. No locale
"C", as letras `abcdefghijklmnopqrstuvwxyz` viram
`ABCDEFGHIJKLMNOPQRSTUVWXYZ`.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a converter |

**Retorno:** a maiúscula de `c`, ou o próprio `c` quando o locale não tem
maiúscula pra ele.

O exemplo troca a caixa dentro da memória do C, byte a byte:

```ps
import <ctype.h>
import <string.h>

char buf[16]
strcpy(buf, "Texto")
int i = 0
while (buf[i] != 0) {
    buf[i] = toupper(buf[i])
    i++
}
post(str(buf))
```

```saida
TEXTO
```
