# string.h — texto e memória

As funções do `string.h` copiam, juntam, comparam e procuram em textos do C
(arrays de `char` terminados pelo byte nulo) e em blocos de memória. Como
toda a biblioteca padrão do C, elas valem com o cabeçalho importado no topo do
arquivo, igual ao `#include <string.h>` do C ([o C na Jinga](c.md)):

```ps
import <string.h>

post(strlen("Jinga"))
```

```saida
5
```

> Esta página segue a cppreference
> ([en.cppreference.com/w/c/string/byte](https://en.cppreference.com/w/c/string/byte),
> licença [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)),
> traduzida e adaptada à Jinga. Os protótipos são os do C11 (Anexo B do
> N1570), conferidos contra a tabela do motor. Cada exemplo foi rodado.

| Função | O que faz |
|---|---|
| [`strcpy`](#strcpy) | copia um texto |
| [`strncpy`](#strncpy) | copia até um número de caracteres |
| [`strcat`](#strcat) | junta um texto no fim de outro |
| [`strncat`](#strncat) | junta até um número de caracteres |
| [`strxfrm`](#strxfrm) | transforma o texto pra comparar pelo locale |
| [`strlen`](#strlen) | o tamanho do texto, em bytes |
| [`strcmp`](#strcmp) | compara dois textos |
| [`strncmp`](#strncmp) | compara até um número de caracteres |
| [`strcoll`](#strcoll) | compara pela ordem do locale |
| [`strchr`](#strchr) | a primeira ocorrência de um caractere |
| [`strrchr`](#strrchr) | a última ocorrência de um caractere |
| [`strspn`](#strspn) | o trecho do começo feito só de certos caracteres |
| [`strcspn`](#strcspn) | o trecho do começo sem certos caracteres |
| [`strpbrk`](#strpbrk) | o primeiro caractere de um conjunto |
| [`strstr`](#strstr) | a primeira ocorrência de um texto |
| [`strtok`](#strtok) | o próximo pedaço de um texto |
| [`memchr`](#memchr) | a primeira ocorrência de um byte num bloco |
| [`memcmp`](#memcmp) | compara dois blocos |
| [`memset`](#memset) | preenche um bloco com um byte |
| [`memcpy`](#memcpy) | copia um bloco |
| [`memmove`](#memmove) | copia um bloco, mesmo com sobreposição |
| [`strerror`](#strerror) | o texto de um código de erro |

## Na Jinga

- Sem o `import <string.h>`, o nome é `NameError` com a dica do import que
  falta: `NameError: name 'strlen' is not defined. Did you forget to import
  <string.h>?`. Os exemplos que usam nome de outro cabeçalho importam ele
  também, um por linha (o `printf` e o `puts` são do `<stdio.h>`, o `malloc`
  é do `<stdlib.h>`, o `errno` é do `<errno.h>`).
- O `import <string.h>` traz também o `NULL` e o `size_t`, que o C declara em
  mais de um cabeçalho.
- O `str` da Jinga entra onde o C só lê (`const char *`, `const void *`):
  `strlen("Jinga")`. Onde o C escreve (o `s1` do `strcpy`, do `strcat` e do
  `strtok`, o `s` do `memset`), entra memória do C: um buffer (`char buf[64]`)
  ou o que o `malloc` deu. O `str` nesse lugar é erro antes de rodar:
  `TypeError: strtok() altera o texto (char *), e o str da Jinga só entra como const char *: copie pra um buffer`.
  O `str(buf)` traz o texto do buffer de volta, como `str`.
- O C não sabe o tamanho do buffer: escrever além do fim é comportamento
  indefinido, como no C, e pode derrubar o programa.
- Os tamanhos e as posições são em bytes. O `str` da Jinga é UTF-8:
  `strlen("Olá")` dá 4, e `"Olá".len()` dá 3.
- O byte de um buffer é número: `buf[0]` dá o código dele, e a escrita pede
  número, `buf[0] = ord("M")`. O `'M'` da Jinga é um `str` e não vira `char`
  (`TypeError: str não vira char`).
- O ponteiro que volta (o do `strchr`, do `strstr`, do `strtok`) aponta pra
  dentro do texto que entrou. Num buffer do C, ele vale enquanto o buffer
  existir; o `str` da Jinga entra pelo endereço do próprio texto, que o motor
  só garante durante a chamada. Pra guardar o ponteiro, o texto vai antes pra
  um buffer, com o `strcpy`.
- Nas comparações (`strcmp`, `strncmp`, `strcoll`, `memcmp`), o padrão só fixa
  o sinal do resultado: confira com `< 0`, `== 0` e `> 0`.
- O `locale.h` ainda não está na Jinga: vale o locale "C", em que o `strcoll`
  compara como o `strcmp` e o `strxfrm` deixa o texto como está.

## Cópia e junção

### `strcpy`

```c
char *strcpy(char * restrict s1, const char * restrict s2);
```

Copia o texto terminado em nulo apontado por `s2`, com o nulo do fim, pro
array de char cujo primeiro elemento é apontado por `s1`. O comportamento é
indefinido se o array `s1` não é grande o bastante, se os textos se
sobrepõem, se `s1` não aponta pra um array de char e se `s2` não aponta pra um
texto terminado em nulo.

| Parâmetro | O que é |
|---|---|
| `s1` | o ponteiro pro array de char a escrever |
| `s2` | o ponteiro pro texto terminado em nulo a copiar |

**Retorno:** uma cópia de `s1`.

Na Jinga, o `s2` pode ser um `str`, e o `s1` é um buffer do C com espaço pros
bytes do texto e o nulo do fim:

```ps
import <string.h>

str origem = "Pato no lago."
char destino[32]
strcpy(destino, origem)
destino[0] = ord("G")
post(origem)
post(str(destino))
```

```saida
Pato no lago.
Gato no lago.
```

### `strncpy`

```c
char *strncpy(char * restrict s1, const char * restrict s2, size_t n);
```

Copia no máximo `n` caracteres do array apontado por `s2` pro array apontado
por `s1`, com o nulo do fim de `s2`, mas nada depois dele. Se o `n` acaba
antes de o `s2` inteiro ser copiado, o resultado não termina em nulo. Se o
nulo de `s2` é copiado e o `n` não acabou, o `strncpy` escreve mais nulos em
`s1` até completar `n` caracteres.

O comportamento é indefinido se os arrays se sobrepõem, se `s1` ou `s2` não
aponta pra um array de char (o ponteiro nulo incluído), se o array de `s1` tem
menos que `n` caracteres, e se o array de `s2` tem menos que `n` caracteres e
nenhum nulo.

| Parâmetro | O que é |
|---|---|
| `s1` | o ponteiro pro array de char a escrever |
| `s2` | o ponteiro pro array de char a copiar |
| `n` | o máximo de caracteres a copiar |

**Retorno:** uma cópia de `s1`.

O exemplo mostra os bytes do buffer: o `"oi"`, os três nulos de enchimento, o
`f` e o `g` que já estavam lá; e, no buffer de 2 bytes, o resultado sem nulo,
porque o `n` acabou antes:

```ps
import <string.h>

char destino[8]
strcpy(destino, "abcdefg")
strncpy(destino, "oi", 5)
list bytes = []
int i = 0
while (i < 8) {
    if destino[i] == 0 {
        addEnd(bytes, "\\0")
    }
    else {
        addEnd(bytes, chr(destino[i]))
    }
    i++
}
post(" ".join(bytes))
char curto[2]
strncpy(curto, "oi", 2)
post(chr(curto[0]), chr(curto[1]))
```

```saida
o i \0 \0 \0 f g \0
o i
```

### `strcat`

```c
char *strcat(char * restrict s1, const char * restrict s2);
```

Junta uma cópia do texto terminado em nulo apontado por `s2` no fim do texto
terminado em nulo apontado por `s1`. O caractere `s2[0]` entra no lugar do
nulo do fim de `s1`, e o resultado termina em nulo. O comportamento é
indefinido se o array de `s1` não tem espaço pro conteúdo dos dois e o nulo
do fim, se os textos se sobrepõem, e se `s1` ou `s2` não aponta pra um texto
terminado em nulo.

Como o `strcat` procura o fim de `s1` a cada chamada, juntar muitos textos num
só com ele é ineficiente.

| Parâmetro | O que é |
|---|---|
| `s1` | o ponteiro pro texto terminado em nulo que recebe a junção |
| `s2` | o ponteiro pro texto terminado em nulo a copiar |

**Retorno:** uma cópia de `s1`.

```ps
import <stdio.h>
import <string.h>

char s[50]
strcpy(s, "Olá ")
strcat(s, "mundo!")
strcat(s, " Tudo bem?")
strcat(s, " Tchau, mundo!")
puts(s)
```

```saida
Olá mundo! Tudo bem? Tchau, mundo!
```

### `strncat`

```c
char *strncat(char * restrict s1, const char * restrict s2, size_t n);
```

Junta no máximo `n` caracteres do array apontado por `s2` no fim do texto
terminado em nulo apontado por `s1`; a cópia para no nulo de `s2`, se ele vem
antes. O caractere `s2[0]` entra no lugar do nulo do fim de `s1`, e o nulo do
fim sempre é escrito: a função escreve no máximo `n + 1` bytes.

O comportamento é indefinido se o array de `s1` não tem espaço pro conteúdo de
`s1`, os primeiros `n` caracteres de `s2` e o nulo do fim, se os objetos se
sobrepõem, se `s1` não aponta pra um texto terminado em nulo e se `s2` não
aponta pra um array de char. Como no `strcat`, juntar muitos textos com ele é
ineficiente.

| Parâmetro | O que é |
|---|---|
| `s1` | o ponteiro pro texto terminado em nulo que recebe a junção |
| `s2` | o ponteiro pro array de char a copiar |
| `n` | o máximo de caracteres a copiar |

**Retorno:** uma cópia de `s1`.

```ps
import <stdio.h>
import <string.h>

char s[50]
strcpy(s, "Olá mundo!")
strncat(s, " Tchau, mundo!", 3)
puts(s)
```

```saida
Olá mundo! Tc
```

### `strxfrm`

```c
size_t strxfrm(char * restrict s1, const char * restrict s2, size_t n);
```

Transforma o texto terminado em nulo apontado por `s2` numa forma que fica a
cargo da implementação: comparar dois textos transformados com o `strcmp` dá
o mesmo que comparar os originais com o `strcoll`, no locale atual. Os
primeiros `n` caracteres do texto transformado, com o nulo do fim, vão pro
array apontado por `s1`, e a função devolve o tamanho do texto transformado
inteiro, sem o nulo. O comportamento é indefinido se o array `s1` não é grande
o bastante e se `s1` e `s2` se sobrepõem. Com `n` zero, o `s1` pode ser o
ponteiro nulo.

O tamanho certo do buffer pro texto transformado inteiro é
`1 + strxfrm(NULL, s2, 0)`. A função serve quando os mesmos textos entram em
muitas comparações pelo locale: transformar uma vez e comparar com o `strcmp`
sai mais barato.

| Parâmetro | O que é |
|---|---|
| `s1` | o ponteiro pro primeiro elemento do array que recebe o texto transformado |
| `s2` | o ponteiro pro primeiro caractere do texto terminado em nulo a transformar |
| `n` | o máximo de caracteres a escrever |

**Retorno:** o tamanho do texto transformado, sem o nulo do fim.

O exemplo da cppreference usa o locale tcheco, em que `"hrnec"` vem antes de
`"chrt"`. Na Jinga vale o locale "C", e a ordem é a dos bytes:

```ps
import <stdlib.h>
import <string.h>

str a = "hrnec"
str b = "chrt"
size_t na = 1 + strxfrm(NULL, a, 0)
char *ta = malloc(na)
strxfrm(ta, a, na)
size_t nb = 1 + strxfrm(NULL, b, 0)
char *tb = malloc(nb)
strxfrm(tb, b, nb)
post(na, nb)
post(strcmp(ta, tb) > 0, strcoll(a, b) > 0)
free(ta)
free(tb)
```

```saida
6 5
True True
```

## Exame

### `strlen`

```c
size_t strlen(const char *s);
```

Devolve o tamanho do texto terminado em nulo: o número de caracteres do array
cujo primeiro elemento é apontado por `s`, até o primeiro nulo, sem contar
ele. O comportamento é indefinido se `s` não aponta pra um texto terminado em
nulo.

| Parâmetro | O que é |
|---|---|
| `s` | o ponteiro pro texto terminado em nulo a medir |

**Retorno:** o tamanho do texto `s`.

Na Jinga, o tamanho é em bytes, e o `len()` de um `str` conta caracteres. O
`sizeof` do buffer conta o array inteiro:

```ps
import <stdio.h>
import <string.h>

char texto[64]
strcpy(texto, "Quantos caracteres tem este texto?")
printf("sem o nulo: %zu\n", strlen(texto))
printf("o array inteiro: %zu\n", sizeof(texto))
post(strlen("Olá"), "Olá".len())
```

```saida
sem o nulo: 34
o array inteiro: 64
4 3
```

### `strcmp`

```c
int strcmp(const char *s1, const char *s2);
```

Compara dois textos terminados em nulo na ordem lexicográfica. O sinal do
resultado é o sinal da diferença entre os valores do primeiro par de
caracteres diferentes (os dois lidos como `unsigned char`). O comportamento é
indefinido se `s1` ou `s2` não aponta pra um texto terminado em nulo. O
`strcmp` não depende do locale, ao contrário do `strcoll` e do `strxfrm`.

| Parâmetro | O que é |
|---|---|
| `s1`, `s2` | os ponteiros pros textos terminados em nulo a comparar |

**Retorno:** um valor negativo se `s1` vem antes de `s2` na ordem
lexicográfica; zero se são iguais; um valor positivo se `s1` vem depois.

A ordem é a dos bytes: o espaço vem antes do `!`, e a maiúscula antes da
minúscula:

```ps
import <stdio.h>
import <string.h>

Null funct mostra(str a, str b) {
    int rc = strcmp(a, b)
    str rel = "é igual a"
    if rc < 0 {
        rel = "vem antes de"
    }
    elif rc > 0 {
        rel = "vem depois de"
    }
    printf("[%s] %s [%s]\n", a, rel, b)
}

mostra("Olá mundo!", "Olá!")
mostra("Olá mundo!", "Olá")
mostra("Olá mundo!", "Olá todos")
mostra("Zebra", "abelha")
mostra("Jinga", "Jinga")
```

```saida
[Olá mundo!] vem antes de [Olá!]
[Olá mundo!] vem depois de [Olá]
[Olá mundo!] vem antes de [Olá todos]
[Zebra] vem antes de [abelha]
[Jinga] é igual a [Jinga]
```

### `strncmp`

```c
int strncmp(const char *s1, const char *s2, size_t n);
```

Compara no máximo `n` caracteres de dois arrays, que podem terminar em nulo,
na ordem lexicográfica. Os caracteres depois de um nulo não são comparados. O
sinal do resultado é o sinal da diferença entre os valores do primeiro par de
caracteres diferentes (os dois lidos como `unsigned char`). O comportamento é
indefinido se a leitura passa do fim de `s1` ou de `s2`, e se um dos dois é o
ponteiro nulo. O `strncmp` não depende do locale.

| Parâmetro | O que é |
|---|---|
| `s1`, `s2` | os ponteiros pros arrays a comparar, que podem terminar em nulo |
| `n` | o máximo de caracteres a comparar |

**Retorno:** um valor negativo se `s1` vem antes de `s2` na ordem
lexicográfica; zero se são iguais ou se `n` é zero; um valor positivo se `s1`
vem depois.

```ps
import <stdio.h>
import <string.h>

Null funct mostra(str a, str b, int n) {
    int rc = strncmp(a, b, n)
    str rel = "são iguais aos de"
    if rc < 0 {
        rel = "vêm antes dos de"
    }
    elif rc > 0 {
        rel = "vêm depois dos de"
    }
    printf("os %d primeiros de [%s] %s [%s]\n", n, a, rel, b)
}

mostra("Bom dia!", "Bom!", 3)
mostra("Bom dia!", "Bom", 10)
mostra("Bom dia!", "Bom dia, gente", 10)
mostra("Bom dia!", "Boa noite", 0)
```

```saida
os 3 primeiros de [Bom dia!] são iguais aos de [Bom!]
os 10 primeiros de [Bom dia!] vêm depois dos de [Bom]
os 10 primeiros de [Bom dia!] vêm antes dos de [Bom dia, gente]
os 0 primeiros de [Bom dia!] são iguais aos de [Boa noite]
```

### `strcoll`

```c
int strcoll(const char *s1, const char *s2);
```

Compara dois textos terminados em nulo pela ordem do locale atual, a da
categoria `LC_COLLATE`. A ordem de agrupamento é a do dicionário: a posição
da letra no alfabeto nacional (a classe de equivalência dela) pesa mais que a
caixa ou a variante; dentro de uma classe, a minúscula vem antes da
maiúscula; e os caracteres com diacrítico podem ter a ordem do locale. Em
alguns locales, grupos de caracteres comparam como uma unidade só: o `"ch"` do
tcheco vem depois do `"h"` e antes do `"i"`, e o `"dzs"` do húngaro vem depois
do `"dz"` e antes do `"g"`.

| Parâmetro | O que é |
|---|---|
| `s1`, `s2` | os ponteiros pros textos terminados em nulo a comparar |

**Retorno:** um valor negativo se `s1` vem antes de `s2`; zero se são iguais;
um valor positivo se `s1` vem depois.

Na Jinga vale o locale "C", e nele o `strcoll` dá a mesma ordem do `strcmp`, a
dos bytes. No locale tcheco do exemplo da cppreference, `"hrnec"` viria antes
de `"chrt"`:

```ps
import <string.h>

str a = "hrnec"
str b = "chrt"
post(strcoll(a, b) > 0, strcmp(a, b) > 0)
```

```saida
True True
```

### `strchr`

```c
char *strchr(const char *s, int c);
```

Procura a primeira ocorrência de `c` (convertido em `char`, como por
`(char) c`) no texto terminado em nulo apontado por `s`, com cada caractere
lido como `unsigned char`. O nulo do fim conta como parte do texto: procurar o
`'\0'` acha o fim. O comportamento é indefinido se `s` não aponta pra um texto
terminado em nulo.

| Parâmetro | O que é |
|---|---|
| `s` | o ponteiro pro texto terminado em nulo a examinar |
| `c` | o caractere a procurar |

**Retorno:** o ponteiro pro caractere achado em `s`, ou o ponteiro nulo se ele
não está lá.

Na Jinga, o caractere entra como número, pelo `ord()`:

```ps
import <stdio.h>
import <string.h>

char frase[64]
strcpy(frase, "Tente. Faça, ou não faça. Tentar não existe.")
char *r = strchr(frase, ord("T"))
while (r != NULL) {
    printf("achou 'T' no começo de '%s'\n", r)
    r = strchr(r + 1, ord("T"))
}
```

```saida
achou 'T' no começo de 'Tente. Faça, ou não faça. Tentar não existe.'
achou 'T' no começo de 'Tentar não existe.'
```

### `strrchr`

```c
char *strrchr(const char *s, int c);
```

Procura a última ocorrência de `c` (convertido em `char`, como por
`(char) c`) no texto terminado em nulo apontado por `s`, com cada caractere
lido como `unsigned char`. O nulo do fim conta como parte do texto e pode ser
achado procurando o `'\0'`. O comportamento é indefinido se `s` não aponta pra
um texto terminado em nulo.

| Parâmetro | O que é |
|---|---|
| `s` | o ponteiro pro texto terminado em nulo a examinar |
| `c` | o caractere a procurar |

**Retorno:** o ponteiro pro caractere achado em `s`, ou o ponteiro nulo se ele
não está lá.

O exemplo tira o nome do arquivo de um caminho: o nome começa depois da
última barra, e sem barra o nome é o caminho inteiro:

```ps
import <stdio.h>
import <string.h>

char caminho[32]
strcpy(caminho, "foo/bar/foobar.txt")
char *barra = strrchr(caminho, ord("/"))
char *nome = caminho
if barra != NULL {
    nome = barra + 1
}
printf("nome: %s\n", nome)
```

```saida
nome: foobar.txt
```

### `strspn`

```c
size_t strspn(const char *s1, const char *s2);
```

Devolve o tamanho do maior trecho do começo (o *span*) do texto terminado em
nulo apontado por `s1` feito só de caracteres que estão no texto terminado em
nulo apontado por `s2`. O comportamento é indefinido se `s1` ou `s2` não
aponta pra um texto terminado em nulo.

| Parâmetro | O que é |
|---|---|
| `s1` | o ponteiro pro texto terminado em nulo a examinar |
| `s2` | o ponteiro pro texto terminado em nulo com os caracteres aceitos |

**Retorno:** o tamanho do maior trecho do começo feito só de caracteres de
`s2`.

Na Jinga, a soma de um buffer com o tamanho (`texto + n`) aponta pro resto,
como no C:

```ps
import <stdio.h>
import <string.h>

char texto[16]
strcpy(texto, "abcde312$#@")
size_t n = strspn(texto, "qwertyuiopasdfghjklzxcvbnm")
printf("pulando as minúsculas do começo de '%s'\n", texto)
printf("sobra '%s'\n", texto + n)
```

```saida
pulando as minúsculas do começo de 'abcde312$#@'
sobra '312$#@'
```

### `strcspn`

```c
size_t strcspn(const char *s1, const char *s2);
```

Devolve o tamanho do maior trecho do começo do texto terminado em nulo
apontado por `s1` feito só de caracteres que não estão no texto terminado em
nulo apontado por `s2`. O comportamento é indefinido se `s1` ou `s2` não
aponta pra um texto terminado em nulo. O nome vem de *complementary span*: os
caracteres procurados são o complemento de `s2`.

| Parâmetro | O que é |
|---|---|
| `s1` | o ponteiro pro texto terminado em nulo a examinar |
| `s2` | o ponteiro pro texto terminado em nulo com os caracteres recusados |

**Retorno:** o tamanho do maior trecho do começo sem nenhum caractere de `s2`.

```ps
import <stdio.h>
import <string.h>

str texto = "abcde312$#@"
str proibidos = "*$#"
size_t n = strcspn(texto, proibidos)
if n != strlen(texto) {
    printf("'%s' tem caractere proibido a partir da posição %zu\n", texto, n)
}
```

```saida
'abcde312$#@' tem caractere proibido a partir da posição 8
```

### `strpbrk`

```c
char *strpbrk(const char *s1, const char *s2);
```

Procura no texto terminado em nulo apontado por `s1` qualquer caractere do
texto terminado em nulo apontado por `s2` e devolve o ponteiro pra ele. O
comportamento é indefinido se `s1` ou `s2` não aponta pra um texto terminado
em nulo. O nome vem de *string pointer break*: a função devolve o ponteiro pro
primeiro dos caracteres separadores.

| Parâmetro | O que é |
|---|---|
| `s1` | o ponteiro pro texto terminado em nulo a examinar |
| `s2` | o ponteiro pro texto terminado em nulo com os caracteres procurados |

**Retorno:** o ponteiro pro primeiro caractere de `s1` que também está em
`s2`, ou o ponteiro nulo se não há nenhum.

O exemplo conta as palavras: acha o separador, pula os separadores seguidos
com o `strspn` e conta:

```ps
import <stdio.h>
import <string.h>

char frase[64]
strcpy(frase, "olá mundo, amigo de todos!")
str sep = " ,!"
char *p = frase
int palavras = 0
while (True) {
    p = strpbrk(p, sep)
    if p != NULL {
        p = p + strspn(p, sep)
    }
    palavras++
    if p == NULL or *p == 0 {
        break
    }
}
printf("são %d palavras\n", palavras)
```

```saida
são 5 palavras
```

### `strstr`

```c
char *strstr(const char *s1, const char *s2);
```

Procura a primeira ocorrência do texto terminado em nulo apontado por `s2` no
texto terminado em nulo apontado por `s1`. Os nulos do fim não entram na
comparação. O comportamento é indefinido se `s1` ou `s2` não aponta pra um
texto terminado em nulo.

| Parâmetro | O que é |
|---|---|
| `s1` | o ponteiro pro texto terminado em nulo a examinar |
| `s2` | o ponteiro pro texto terminado em nulo a procurar |

**Retorno:** o ponteiro pro primeiro caractere do trecho achado em `s1`, ou o
ponteiro nulo se ele não está lá. Com `s2` vazio, a volta é o próprio `s1`.

Na Jinga, a diferença entre o ponteiro achado e o buffer (`pos - texto`) dá a
posição, em bytes:

```ps
import <stdio.h>
import <string.h>

char texto[32]
strcpy(texto, "um dois três")

Null funct acha(str trecho) {
    char *pos = strstr(texto, trecho)
    if pos != NULL {
        printf("achou [%s] em [%s] na posição %td\n", trecho, texto, pos - texto)
    }
    else {
        printf("não achou [%s] em [%s]\n", trecho, texto)
    }
}

acha("dois")
acha("")
acha("nove")
acha("m")
```

```saida
achou [dois] em [um dois três] na posição 3
achou [] em [um dois três] na posição 0
não achou [nove] em [um dois três]
achou [m] em [um dois três] na posição 1
```

### `strtok`

```c
char *strtok(char * restrict s1, const char * restrict s2);
```

Uma sequência de chamadas ao `strtok` quebra o texto apontado por `s1` em
pedaços (*tokens*), separados por qualquer caractere do texto apontado por
`s2`.

- Com `s1` não nulo, a chamada é a primeira da sequência, e o texto a examinar
  é o apontado por `s1`. Com `s1` nulo, a chamada continua a sequência, e o
  texto a examinar é o que a chamada anterior deixou.
- Cada chamada procura no texto a examinar o primeiro caractere que não está
  em `s2` (que pode mudar de uma chamada pra outra). Se não acha, não há
  pedaço, e o texto a examinar continua o mesmo pra próxima chamada (um pedaço
  ainda pode sair numa chamada com outros separadores).
- Se acha, ali começa o pedaço. Dali, o `strtok` procura o primeiro caractere
  que está em `s2`. Se não acha, o pedaço vai até o fim, e o texto a examinar
  da próxima chamada é vazio. Se acha, troca esse caractere por um nulo, que
  termina o pedaço, e o texto a examinar da próxima chamada começa no
  caractere seguinte.

Cada chamada muda uma variável estática: o `strtok` não é seguro entre
threads. A função destrói o texto (escreve `'\0'` nele), e por isso um literal
de texto não pode ser o primeiro argumento. Ao contrário da maioria dos
quebradores de texto, os separadores podem mudar a cada pedaço, e até
depender dos pedaços anteriores. O comportamento é indefinido se `s1` ou `s2`
não aponta pra um texto terminado em nulo.

| Parâmetro | O que é |
|---|---|
| `s1` | o ponteiro pro texto terminado em nulo a quebrar, na primeira chamada; o ponteiro nulo nas seguintes |
| `s2` | o ponteiro pro texto terminado em nulo com os separadores |

**Retorno:** o ponteiro pro primeiro caractere do próximo pedaço, ou o
ponteiro nulo se não há mais pedaço.

Na Jinga, o `s1` é um buffer do C: o `str` ali é erro antes de rodar. Depois
da quebra, os espaços do buffer viraram nulos:

```ps
import <stdio.h>
import <string.h>

char entrada[64]
strcpy(entrada, "Uma ave desceu pelo caminho")
size_t total = strlen(entrada) + 1
char *pedaco = strtok(entrada, " ")
while (pedaco != NULL) {
    puts(pedaco)
    pedaco = strtok(NULL, " ")
}
list bytes = []
int i = 0
while (i < total) {
    if entrada[i] == 0 {
        addEnd(bytes, "\\0")
    }
    else {
        addEnd(bytes, chr(entrada[i]))
    }
    i++
}
post("".join(bytes))
```

```saida
Uma
ave
desceu
pelo
caminho
Uma\0ave\0desceu\0pelo\0caminho\0
```

## Blocos de memória

### `memchr`

```c
void *memchr(const void *s, int c, size_t n);
```

Procura a primeira ocorrência de `(unsigned char) c` nos primeiros `n` bytes
(cada um lido como `unsigned char`) do objeto apontado por `s`. A função age
como se lesse os bytes um a um e parasse no primeiro igual: se o array
apontado por `s` é menor que `n`, mas o byte está nele, o comportamento é
definido (desde o C11). O comportamento é indefinido se a leitura passa do fim
do array e se `s` é o ponteiro nulo.

| Parâmetro | O que é |
|---|---|
| `s` | o ponteiro pro objeto a examinar |
| `c` | o byte a procurar |
| `n` | o máximo de bytes a examinar |

**Retorno:** o ponteiro pro byte achado, ou o ponteiro nulo se ele não está
lá.

Na Jinga, o `void *` que volta vira o `char *` da variável:

```ps
import <stdio.h>
import <string.h>

char s[16]
strcpy(s, "ABCDEFG")
for each c in ["D", "d"] {
    char *p = memchr(s, ord(c), strlen(s))
    if p != NULL {
        printf("'%s' (%d) achado: %s\n", c, ord(c), p)
    }
    else {
        printf("'%s' (%d) não achado\n", c, ord(c))
    }
}
```

```saida
'D' (68) achado: DEFG
'd' (100) não achado
```

### `memcmp`

```c
int memcmp(const void *s1, const void *s2, size_t n);
```

Compara os primeiros `n` bytes dos objetos apontados por `s1` e `s2`, na ordem
lexicográfica. O sinal do resultado é o sinal da diferença entre os valores do
primeiro par de bytes diferentes (os dois lidos como `unsigned char`). O
comportamento é indefinido se a leitura passa do fim de um dos objetos e se
`s1` ou `s2` é o ponteiro nulo.

O `memcmp` lê a representação dos objetos, não o valor, e em geral só faz
sentido em arrays de bytes: um `struct` pode ter bytes de enchimento com valor
indeterminado; os bytes de uma `union` depois do último membro guardado têm
valor indeterminado; e um tipo pode ter mais de uma representação pro mesmo
valor (o `+0.0` e o `-0.0`, por exemplo).

| Parâmetro | O que é |
|---|---|
| `s1`, `s2` | os ponteiros pros objetos a comparar |
| `n` | o número de bytes a examinar |

**Retorno:** um valor negativo se `s1` vem antes de `s2` na ordem
lexicográfica; zero se são iguais ou se `n` é zero; um valor positivo se `s1`
vem depois.

Na Jinga, o `str` entra também onde o C pede `const void *`:

```ps
import <stdio.h>
import <string.h>

Null funct mostra(str a, str b) {
    int rc = memcmp(a, b, 3)
    str rel = "é igual a"
    if rc < 0 {
        rel = "vem antes de"
    }
    elif rc > 0 {
        rel = "vem depois de"
    }
    printf("%s %s %s na ordem dos bytes\n", a, rel, b)
}

mostra("abc", "abd")
mostra("abd", "abc")
mostra("abc", "abc")
```

```saida
abc vem antes de abd na ordem dos bytes
abd vem depois de abc na ordem dos bytes
abc é igual a abc na ordem dos bytes
```

### `memset`

```c
void *memset(void *s, int c, size_t n);
```

Copia o valor `(unsigned char) c` em cada um dos primeiros `n` bytes do objeto
apontado por `s`. O comportamento é indefinido se a escrita passa do fim do
array e se `s` é o ponteiro nulo.

No C, o compilador pode tirar o `memset` (pela regra do "como se") quando o
objeto não é mais lido depois; por isso ele não serve pra apagar dado
sensível, como uma senha (a cppreference cita o `explicit_bzero` do FreeBSD e
o `SecureZeroMemory` da Microsoft). Na Jinga, a chamada vai direto pra função
do C e não some.

| Parâmetro | O que é |
|---|---|
| `s` | o ponteiro pro objeto a preencher |
| `c` | o byte de preenchimento |
| `n` | o número de bytes a preencher |

**Retorno:** uma cópia de `s`.

```ps
import <stdio.h>
import <string.h>

char s[32]
strcpy(s, "ghghghghghghghghghghgh")
puts(s)
memset(s, ord("a"), 5)
puts(s)
```

```saida
ghghghghghghghghghghgh
aaaaahghghghghghghghgh
```

### `memcpy`

```c
void *memcpy(void * restrict s1, const void * restrict s2, size_t n);
```

Copia `n` caracteres do objeto apontado por `s2` pro objeto apontado por `s1`,
os dois lidos como arrays de `unsigned char`. O comportamento é indefinido se a
escrita passa do fim do array `s1`, se os objetos se sobrepõem (o que fere o
`restrict`; pra isso há o [`memmove`](#memmove)) e se `s1` ou `s2` é um
ponteiro inválido ou nulo.

O `memcpy` é a função mais rápida da biblioteca pra copiar memória: em geral,
mais que o `strcpy`, que tem que examinar o que copia, e que o `memmove`, que
tem que cuidar da sobreposição. Ele também dá o tipo efetivo à memória que
veio de uma função de reserva, e serve pra ler os bytes de um valor como
outro tipo, onde a regra do *strict aliasing* proíbe o acesso direto.

| Parâmetro | O que é |
|---|---|
| `s1` | o ponteiro pro objeto a escrever |
| `s2` | o ponteiro pro objeto a copiar |
| `n` | o número de bytes a copiar |

**Retorno:** uma cópia de `s1`.

Na Jinga, `&d` leva o endereço de uma variável do programa, que passa a morar
na memória do C; o exemplo lê os bits do `0.1` como inteiro:

```ps
import <stdio.h>
import <stdlib.h>
import <string.h>

int origem[3] = {1, 2, 3}
int *p = malloc(3 * sizeof(int))
memcpy(p, origem, 3 * sizeof(int))
post(p[0], p[1], p[2])
free(p)
double d = 0.1
long long n = 0
memcpy(&n, &d, sizeof(d))
printf("os bits do 0.1: %llx\n", n)
```

```saida
1 2 3
os bits do 0.1: 3fb999999999999a
```

### `memmove`

```c
void *memmove(void *s1, const void *s2, size_t n);
```

Copia `n` caracteres do objeto apontado por `s2` pro objeto apontado por `s1`,
os dois lidos como arrays de `unsigned char`. Os objetos podem se sobrepor: a
cópia acontece como se os caracteres fossem antes pra um array temporário e
dele pro `s1`. O comportamento é indefinido se a escrita passa do fim do array
`s1` e se `s1` ou `s2` é um ponteiro inválido ou nulo.

O array temporário é só o modelo: as implementações não copiam duas vezes nem
gastam memória a mais. O jeito comum (o da glibc e o da libc do BSD) é copiar
pra frente quando o destino começa antes da origem, pra trás no caso
contrário, e cair no `memcpy` quando não há sobreposição nenhuma. Como o
`memcpy`, ele dá o tipo efetivo à memória reservada e serve pra ler os bytes
de um valor como outro tipo.

| Parâmetro | O que é |
|---|---|
| `s1` | o ponteiro pro objeto a escrever |
| `s2` | o ponteiro pro objeto a copiar |
| `n` | o número de bytes a copiar |

**Retorno:** uma cópia de `s1`.

Na Jinga, a soma de um buffer com um número (`s + 4`) dá o ponteiro pro
quinto byte, como no C:

```ps
import <string.h>

char s[16]
strcpy(s, "1234567890")
memmove(s + 4, s + 3, 3)
post(str(s))
```

```saida
1234456890
```

## Erros

### `strerror`

```c
char *strerror(int errnum);
```

Devolve o ponteiro pra descrição em texto do código de erro do sistema
`errnum`, a mesma que o `perror` escreveria. O `errnum` costuma vir do
`errno`, mas a função aceita qualquer `int`. O texto depende do locale (no
POSIX, da categoria `LC_MESSAGES`). O programa não pode mudar o texto
devolvido, e uma chamada seguinte ao `strerror` pode sobrescrevê-lo: as
implementações podem devolver ponteiros diferentes pra textos estáticos só de
leitura, ou o mesmo ponteiro pra um buffer estático que cada chamada
reescreve. O `strerror` não precisa ser seguro entre threads.

| Parâmetro | O que é |
|---|---|
| `errnum` | o código de erro, um inteiro |

**Retorno:** o ponteiro pro texto terminado em nulo do código de erro
`errnum`.

Na Jinga, o `str()` copia o texto, que então não muda com a chamada seguinte.
O texto é o do sistema, em inglês no locale "C", e muda de um sistema pro
outro (o do `ERANGE` é `Numerical result out of range` no Linux e
`Result too large` no macOS); o do código 2, o `ENOENT` do POSIX, é o mesmo
nos dois:

```ps
import <errno.h>
import <stdio.h>
import <string.h>

FILE *f = fopen("nao_existe.txt", "r")
if f == NULL {
    printf("errno %d: %s\n", errno, strerror(errno))
}
post(str(strerror(2)))
```

```saida
errno 2: No such file or directory
No such file or directory
```

## Fica de fora

Estes nomes do `string.h` não estão na Jinga. Usar um deles é `NameError`
antes de rodar, com o motivo:

| Nome | Motivo |
|---|---|
| `memcpy_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `memmove_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `memset_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `strcat_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `strcpy_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `strerror_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `strerrorlen_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `strncat_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `strncpy_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `strnlen_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `strtok_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |

```ps
import <string.h>

char buf[8]
strcpy_s(buf, 8, "oi")
# NameError: strcpy_s fica de fora: é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS (doc: o C na Jinga)
```
