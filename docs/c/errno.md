# errno.h — códigos de erro

O `errno.h` define o `errno`, onde as funções da biblioteca do C deixam o
código do último erro, e os três códigos de erro do padrão: `EDOM`, `ERANGE` e
`EILSEQ`. Os nomes valem com `import <errno.h>` no topo do arquivo, igual ao
`#include` do C; sem o import, o nome é `NameError`, com a dica do import que
falta ([o C na Jinga](c.md)).

> Esta página segue a cppreference
> ([en.cppreference.com/w/c/error/errno](https://en.cppreference.com/w/c/error/errno)
> e [en.cppreference.com/w/c/error/errno_macros](https://en.cppreference.com/w/c/error/errno_macros),
> licença [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)),
> traduzida e adaptada à Jinga. As declarações são as do C11, conferidas
> contra a tabela do motor. Cada exemplo foi rodado.

| Nome | O que é |
|---|---|
| [`errno`](#errno) | o código do último erro de uma função da biblioteca do C |
| [`EDOM`](#edom-erange-eilseq) | o argumento está fora do domínio da função matemática |
| [`ERANGE`](#edom-erange-eilseq) | o resultado é grande demais |
| [`EILSEQ`](#edom-erange-eilseq) | a sequência de bytes é ilegal |

## Na Jinga

- A função do C não levanta exceção: ela devolve um valor que avisa do erro e
  põe o motivo no `errno`. Quem quer o motivo lê o `errno` logo depois.
- A Jinga guarda o `errno` logo depois de cada chamada de função do C, como o
  `ctypes` do Python faz: o que a própria Jinga faz entre uma chamada e outra
  (ler um arquivo pelo `os`, por exemplo) não mexe nele. Ler o `errno` dá o
  valor da última função do C chamada.
- `errno = 0` escreve no `errno` do C e vale pra chamada seguinte. Como
  nenhuma função da biblioteca põe `0` no `errno`, zerar antes da chamada é o
  jeito de saber se ela deu erro.
- O `errno` é um `int`. Os códigos que o POSIX acrescenta (`ENOENT`,
  `EACCES`, `EOVERFLOW`...) não têm nome na Jinga, que segue o C11: chegam como
  número, e o `strerror` (do `string.h`) diz o que são.

```ps
import <errno.h>
import <stdlib.h>
import os

post(errno)                    # 0 no começo do programa
int n = strtol("99999999999999999999", NULL, 10)
try {
    str s = os.readFile("/nao/existe/mesmo.txt")
}
catch (e) {
    post("a Jinga falhou por conta própria")
}
post(errno == ERANGE)          # ainda o do strtol
errno = 0                      # escreve no errno do C
n = strtol("123", NULL, 10)
post(errno, n)
```

```saida
0
a Jinga falhou por conta própria
True
0 123
```

## O código do erro

### `errno`

```c
#define errno /* definido pela implementação */
```

O `errno` é uma macro do pré-processador que expande pra um lvalue modificável
do tipo `int`, local da thread (desde o C11). Várias funções da biblioteca
padrão indicam erro escrevendo um inteiro positivo no `errno`. Em geral, o
valor é um dos códigos de erro do `errno.h`, macros que começam com a letra `E`
seguida de maiúsculas ou dígitos.

O `errno` vale `0` no começo do programa e, embora as funções da biblioteca
possam escrever inteiros positivos no `errno` tenha havido erro ou não,
nenhuma delas guarda `0` no `errno`.

As funções `perror` (do `stdio.h`) e `strerror` (do `string.h`) dão a descrição
em texto do erro que corresponde ao valor atual do `errno`.

Até o C11, o padrão se contradizia: dizia que o `errno` é uma macro e também
que "não se especifica se o `errno` é uma macro ou um identificador declarado
com ligação externa". O C11 resolveu, exigindo a macro.

O exemplo traduz o código do erro em texto, depois de uma conversão que deu
certo e de outra que estourou a faixa do `long`:

```ps
import <errno.h>
import <stdlib.h>

str funct descreve(int e) {
    if e == EDOM {
        return "erro de domínio"
    }
    if e == EILSEQ {
        return "sequência ilegal"
    }
    if e == ERANGE {
        return "polo ou fora da faixa"
    }
    if e == 0 {
        return "nenhum erro"
    }
    return "erro desconhecido"
}

errno = 0
int n = strtol("42", NULL, 10)
post(descreve(errno))
errno = 0
n = strtol("99999999999999999999", NULL, 10)
post(descreve(errno))
```

```saida
nenhum erro
polo ou fora da faixa
```

O código do POSIX chega como número. Abrir um arquivo que não existe dá o
`ENOENT`, que vale `2` no Linux e no macOS:

```ps
import <errno.h>
import <stdio.h>
import <string.h>

FILE *f = fopen("nao_existe.txt", "r")
if f == NULL {
    printf("errno %d: %s\n", errno, strerror(errno))
}
```

```saida
errno 2: No such file or directory
```

## Os códigos do padrão

### `EDOM`, `ERANGE`, `EILSEQ`

```c
#define EDOM   /* definido pela implementação */
#define ERANGE /* definido pela implementação */
#define EILSEQ /* definido pela implementação */
```

Cada macro do `errno.h` expande pra uma expressão constante inteira do tipo
`int`, com um valor positivo diferente das outras. O ISO C define as três
abaixo; a implementação pode definir mais, desde que comecem com `E` seguido
de dígitos ou de maiúsculas.

| Macro | O que é | Valor |
|---|---|---|
| `EDOM` | o argumento está fora do domínio da função matemática | `33` no Linux e no macOS |
| `ERANGE` | o resultado é grande demais | `34` no Linux e no macOS |
| `EILSEQ` | a sequência de bytes é ilegal (desde o C95) | `84` no Linux, `92` no macOS |

O POSIX e cada sistema definem muitos códigos a mais (o `errno(3)` no Linux,
o `intro(2)` no BSD e no macOS).

O `EDOM` e o `ERANGE` vêm das funções do `math.h` quando o `math_errhandling`
tem o `MATH_ERRNO`: é o caso do Linux. No macOS, as funções do `math.h` não
mexem no `errno` (o `math_errhandling` só tem o `MATH_ERREXCEPT`), e o erro
aparece só nas exceções de ponto flutuante. O exemplo vale nos dois: o
`sqrt(-1.0)` põe o `EDOM` exatamente onde o `math_errhandling` diz que o
`errno` é usado.

```ps
import <errno.h>
import <math.h>

errno = 0
flo r = sqrt(-1.0)
bool deu_edom = errno == EDOM
bool usa_errno = (math_errhandling & MATH_ERRNO) != 0
post(deu_edom == usa_errno, r)
post(EDOM != ERANGE, EDOM != EILSEQ, ERANGE != EILSEQ)
```

```saida
True nan
True True True
```

## Fica de fora

| Nome | Por quê |
|---|---|
| `errno_t`, `__STDC_WANT_LIB_EXT1__` | são do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
