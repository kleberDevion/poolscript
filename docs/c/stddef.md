# stddef.h — definições comuns

O `stddef.h` define os tipos que a linguagem C usa no `sizeof` e na conta com
ponteiro (`size_t` e `ptrdiff_t`), o `wchar_t`, o `max_align_t`, o ponteiro
nulo `NULL` e o `offsetof`, que dá a posição de um campo dentro de uma
`struct`. Os nomes valem com `import <stddef.h>` no topo do arquivo, igual ao
`#include` do C; sem o import, o nome é `NameError`, com a dica do import que
falta ([o C na Jinga](c.md)).

> Esta página segue a cppreference
> ([en.cppreference.com/w/c/types](https://en.cppreference.com/w/c/types) e as
> páginas de cada nome, licença
> [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)), traduzida
> e adaptada à Jinga. Os protótipos são os do C11 (Anexo B do N1570),
> conferidos contra a tabela do motor. Cada exemplo foi rodado.

| Macro | O que faz |
|---|---|
| [`offsetof`](#offsetof) | a distância, em bytes, do começo da `struct` até um campo |

| Tipo ou constante | O que é |
|---|---|
| [`size_t`](#size_t) | o inteiro sem sinal do resultado do `sizeof` |
| [`ptrdiff_t`](#ptrdiff_t) | o inteiro com sinal da diferença entre dois ponteiros |
| [`max_align_t`](#max_align_t) | o tipo com o maior alinhamento entre os escalares |
| [`wchar_t`](#wchar_t) | o inteiro que guarda qualquer caractere largo válido |
| [`NULL`](#null) | o ponteiro nulo |

## Na Jinga

- O `NULL` e o `size_t` valem com qualquer cabeçalho que o C diz que os define:
  o `stddef.h`, o `stdio.h`, o `stdlib.h`, o `string.h` e o `time.h`. O
  `wchar_t` vem com o `stddef.h` e com o `stdlib.h`. O `ptrdiff_t`, o
  `max_align_t` e o `offsetof` são só do `stddef.h`.
- O `NULL` é o `Null` da Jinga: `post(NULL)` escreve `Null`, e o ponteiro nulo é
  igual aos dois.
- O `offsetof`, como o `sizeof`, é operador: o primeiro argumento é um tipo
  (`struct S`), e o segundo, o nome de um campo dele.
- Os tipos daqui valem nos dois lugares: na variável da Jinga
  (`size_t n = 0`), onde o valor é um `int` da Jinga, e na memória do C. A
  escrita dá a volta como no C: o `size_t` não tem sinal.

```ps
import <string.h>

size_t n = strlen("Jinga")      # o size_t veio com o string.h
char *p = NULL                  # o NULL também
post(n, p == NULL)
```

```saida
5 True
```

## Macro

### `offsetof`

```c
offsetof(type, member-designator);
```

Expande pra uma expressão constante inteira do tipo `size_t`, com a distância,
em bytes, do começo de um objeto do tipo `type` até o membro indicado,
contando o enchimento (os bytes de alinhamento), se houver.

| Parâmetro | O que é |
|---|---|
| `type` | o tipo da `struct` ou da `union` (`struct S`, ou o nome dado pelo `typedef`) |
| `member-designator` | o campo cuja posição se quer |

**Retorno:** a posição do campo, em bytes, como `size_t` (na Jinga, um `int`).

No C, o `offsetof` aplicado a um campo de bits é comportamento indefinido,
porque o endereço de um campo de bits não existe (e a Jinga não tem campo de
bits). O C aceita também um subobjeto do campo, como o elemento de um array
(`nome[3]`) ou o campo de uma `struct` de dentro (`a.y`); na Jinga, o segundo
argumento é o nome de um campo direto do tipo.

```ps
import <stddef.h>

struct S {
    char c
    double d
}

post(offsetof(struct S, c), offsetof(struct S, d), sizeof(struct S))
```

```saida
0 8 16
```

O `double` começa no byte 8, e não no 1: os 7 bytes entre o `char` e ele são
o enchimento que o alinhamento do `double` pede.

## Tipos

### `size_t`

```c
typedef /* definido pela implementação */ size_t;
```

O `size_t` é o tipo inteiro sem sinal do resultado do `sizeof`, do `offsetof`
e do `_Alignof`, conforme o modelo de dados do sistema. Ele tem pelo menos 16
bits; no Linux e no macOS (64 bits), é o `unsigned long`, de 64 bits.

O `size_t` guarda o tamanho máximo de um objeto teoricamente possível de
qualquer tipo, inclusive array, e por isso é o tipo comum de índice de array e
de contador de laço. O programa que usa outro tipo, como o `unsigned int`, pro
índice de array falha num sistema de 64 bits quando o índice passa do
`UINT_MAX`.

```ps
import <stddef.h>

int numeros[101]
size_t soma = 0
size_t i = 0
while (i < 101) {
    numeros[i] = i
    soma = soma + numeros[i]
    i++
}
post(soma, sizeof(numeros))
size_t n = 0
n = n - 1          # o size_t não tem sinal: dá a volta
post(n)
```

```saida
5050 404
18446744073709551615
```

### `ptrdiff_t`

```c
typedef /* definido pela implementação */ ptrdiff_t;
```

O `ptrdiff_t` é o tipo inteiro com sinal do resultado da subtração de dois
ponteiros. Ele tem pelo menos 17 bits; no Linux e no macOS (64 bits), é o
`long`, de 64 bits.

O `ptrdiff_t` serve na conta com ponteiro e no índice de array quando o
negativo é possível. Só se subtraem ponteiros pra elementos do mesmo array
(inclusive o ponteiro logo depois do último). Se o array é tão grande que a
diferença não cabe no `ptrdiff_t` (mais elementos que o `PTRDIFF_MAX`, mas não
mais bytes que o `SIZE_MAX`), o resultado da subtração é indefinido. Pra
arrays de `char` menores que o `PTRDIFF_MAX`, o `ptrdiff_t` é a contraparte
com sinal do `size_t`.

```ps
import <stddef.h>

int numeros[100]
int *p1 = &numeros[18]
int *p2 = &numeros[23]
ptrdiff_t d = p2 - p1
post(d, p1 - p2)
```

```saida
5 -5
```

### `max_align_t`

```c
typedef /* definido pela implementação */ max_align_t;
```

O `max_align_t` é um tipo cuja exigência de alinhamento é pelo menos tão
estrita (tão grande) quanto a de todo tipo escalar. Os ponteiros que as
funções de alocação, como o `malloc`, devolvem são alinhados pra qualquer
objeto, ou seja, pelo menos tão estritamente quanto o `max_align_t`.

Na Jinga, o `max_align_t` só existe atrás de ponteiro (`max_align_t *p`) e no
`sizeof`; declarar uma variável dele é erro. O tamanho muda de sistema pra
sistema: 32 bytes no Linux e 16 no macOS.

```ps
import <stddef.h>

post(sizeof(max_align_t) >= sizeof(long double), sizeof(max_align_t) >= sizeof(long long))
```

```saida
True True
```

### `wchar_t`

```c
typedef /* definido pela implementação */ wchar_t;
```

O `wchar_t` é o tipo inteiro que guarda qualquer caractere largo válido. No
Linux (x86_64) e no macOS, é o `int`, de 32 bits.

A Jinga não tem o texto largo literal do C (`L"..."`), e as funções do texto
largo (`wchar.h`) são do lote C2: o caractere entra pelo `ord()`, e o
`chr()` faz o caminho de volta.

```ps
import <stddef.h>

wchar_t w[4] = {ord("J"), ord("ã")}
post(w[0], w[1], chr(w[1]), w[2], sizeof(wchar_t))
```

```saida
74 227 ã 0 4
```

## Constante

### `NULL`

```c
#define NULL /* definido pela implementação */
```

O `NULL` é uma constante de ponteiro nulo definida pela implementação, que
pode ser uma expressão constante inteira de valor `0`, ou essa expressão
convertida pra `void *`. Uma constante de ponteiro nulo pode ser convertida em
qualquer tipo de ponteiro, e o resultado é o ponteiro nulo daquele tipo. O
POSIX exige o `NULL` como a expressão de valor `0` convertida pra `void *`.

Na Jinga, o `NULL` é o `Null`: o ponteiro nulo é igual aos dois, e é falso
num `if`.

```ps
import <stddef.h>

int *p = NULL
post(p == NULL, p == Null, NULL)
if not p {
    post("ponteiro nulo é falso")
}
```

```saida
True True Null
ponteiro nulo é falso
```

## Fica de fora

| Nome | Por quê |
|---|---|
| `rsize_t` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |

O `nullptr_t` entrou no C23 e não faz parte do C11, que a Jinga segue.
