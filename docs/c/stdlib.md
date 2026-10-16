# stdlib.h — utilidades gerais

As funções do `stdlib.h` reservam e soltam memória do C, convertem texto em
número, sorteiam números, ordenam e buscam em arrays, fazem conta com
inteiros, conversam com o ambiente e encerram o programa. Como toda a
biblioteca padrão do C, elas valem com o cabeçalho importado no topo do
arquivo, igual ao `#include <stdlib.h>` do C ([o C na Jinga](c.md)):

```ps
import <stdlib.h>

post(atoi("42") + 1)
```

```saida
43
```

> Esta página segue a cppreference
> ([memória](https://en.cppreference.com/w/c/memory),
> [texto em número](https://en.cppreference.com/w/c/string/byte),
> [números pseudoaleatórios](https://en.cppreference.com/w/c/numeric/random),
> [ordenação e busca](https://en.cppreference.com/w/c/algorithm),
> [aritmética inteira](https://en.cppreference.com/w/c/numeric/math),
> [fim do programa e ambiente](https://en.cppreference.com/w/c/program),
> [multibyte](https://en.cppreference.com/w/c/string/multibyte);
> licença [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)),
> traduzida e adaptada à Jinga. Os protótipos são os do C11 (Anexo B do
> N1570), conferidos contra a tabela do motor. Cada exemplo foi rodado.

| Função | O que faz |
|---|---|
| [`malloc`](#malloc) | reserva memória |
| [`calloc`](#calloc) | reserva memória zerada |
| [`realloc`](#realloc) | muda o tamanho de um bloco reservado |
| [`free`](#free) | solta a memória reservada |
| [`aligned_alloc`](#aligned_alloc) | reserva memória alinhada |
| [`atof`](#atof) | texto em `double` |
| [`atoi`](#atoi-atol-atoll), [`atol`](#atoi-atol-atoll), [`atoll`](#atoi-atol-atoll) | texto em inteiro |
| [`strtol`](#strtol-strtoll), [`strtoll`](#strtol-strtoll) | texto em inteiro, com a base e o fim da leitura |
| [`strtoul`](#strtoul-strtoull), [`strtoull`](#strtoul-strtoull) | texto em inteiro sem sinal |
| [`strtof`](#strtof-strtod-strtold), [`strtod`](#strtof-strtod-strtold), [`strtold`](#strtof-strtod-strtold) | texto em ponto flutuante, com o fim da leitura |
| [`rand`](#rand) | número pseudoaleatório |
| [`srand`](#srand) | semente do `rand` |
| [`qsort`](#qsort) | ordena um array |
| [`bsearch`](#bsearch) | busca num array ordenado |
| [`abs`](#abs-labs-llabs), [`labs`](#abs-labs-llabs), [`llabs`](#abs-labs-llabs) | valor absoluto |
| [`div`](#div-ldiv-lldiv), [`ldiv`](#div-ldiv-lldiv), [`lldiv`](#div-ldiv-lldiv) | quociente e resto de uma vez |
| [`exit`](#exit) | encerra o programa, com a limpeza |
| [`atexit`](#atexit) | registra uma funct pro fim do programa |
| [`quick_exit`](#quick_exit) | encerra sem a limpeza completa |
| [`at_quick_exit`](#at_quick_exit) | registra uma funct pro `quick_exit` |
| [`_Exit`](#_exit) | encerra sem limpeza |
| [`abort`](#abort) | fim anormal |
| [`getenv`](#getenv) | lê uma variável de ambiente |
| [`system`](#system) | roda um comando no interpretador do sistema |
| [`mblen`](#mblen) | os bytes do próximo caractere multibyte |
| [`mbtowc`](#mbtowc) | caractere multibyte em caractere largo |
| [`wctomb`](#wctomb) | caractere largo em multibyte |
| [`mbstowcs`](#mbstowcs) | texto multibyte em texto largo |
| [`wcstombs`](#wcstombs) | texto largo em multibyte |

| Tipo ou constante | O que é |
|---|---|
| [`div_t`, `ldiv_t`, `lldiv_t`](#div_t-ldiv_t-lldiv_t) | o resultado do `div`, do `ldiv` e do `lldiv` |
| [`EXIT_SUCCESS`, `EXIT_FAILURE`](#exit_success-exit_failure) | o status de saída do programa |
| [`RAND_MAX`](#rand_max) | o maior número do `rand` |
| [`MB_CUR_MAX`](#mb_cur_max) | os bytes do maior caractere multibyte do locale |

## Na Jinga

- Sem o `import <stdlib.h>`, o nome é `NameError` com a dica do import que
  falta: `NameError: name 'malloc' is not defined. Did you forget to import
  <stdlib.h>?`. Os exemplos que usam nome de outro cabeçalho importam ele
  também, um por linha (o `printf` é do `<stdio.h>`, o `errno` e o `ERANGE`
  são do `<errno.h>`, o `strcpy` é do `<string.h>`).
- O `import <stdlib.h>` traz também o `NULL`, o `size_t` e o `wchar_t`, que o
  C declara em mais de um cabeçalho. As palavras do C (`int *p`, `unsigned`,
  `const`, `struct`, `sizeof`, o cast) não pedem import.
- `abs` é o da Jinga, mesmo com o `<stdlib.h>` importado; o valor absoluto do
  C fica pelos outros nomes que o C tem, `labs` e `llabs`
  ([`abs`](#abs-labs-llabs)).
- A memória do `malloc`, do `calloc`, do `realloc` e do `aligned_alloc` é do
  C: o coletor da Jinga não a vê e não a solta. Cada bloco pede o seu `free`,
  e o ponteiro depois do `free` aponta pra memória que não é mais do
  programa: ler ou escrever por ele é comportamento indefinido, como no C, e
  pode derrubar o programa.
- O número que volta do C vira número da Jinga: os inteiros do C viram `int`
  (acima de 2^63, o inteiro grande), e `float`, `double` e `long double`
  viram `flo` (o `long double` perde o que passa do `double`). A conta depois
  é a da Jinga: `rand() / RAND_MAX` já dá `flo`, sem o cast `(double)` do C,
  e `RAND_MAX + 1` não estoura.
- Onde o C pede `const char *`, entra o `str` da Jinga (`atoi("42")`). O
  `char *` que o C devolve (o do `getenv`) vira `str` pelo `str(p)`.
- Onde o C pede um ponteiro de função (`qsort`, `bsearch`, `atexit`,
  `at_quick_exit`), entra uma funct da Jinga, e o `--check` confere antes de
  rodar quantos argumentos ela recebe.
- A família do `strto` avisa o número fora da faixa pelo `errno` (o
  `ERANGE`): zere com `errno = 0` antes da chamada e confira depois.
- O `locale.h` ainda não está na Jinga: vale o locale "C" (o ponto decimal do
  `strtod` é o `.`, e o `MB_CUR_MAX` é 1).

## Memória

### `malloc`

```c
void *malloc(size_t size);
```

Reserva `size` bytes de memória sem inicializar. Se der certo, devolve o
ponteiro pro primeiro byte do bloco, alinhado pra qualquer tipo de objeto com
alinhamento fundamental. Se `size` é zero, o que acontece fica a cargo da
implementação: pode voltar o ponteiro nulo, ou um ponteiro não nulo que não
serve pra acessar memória mas tem que ir pro `free`.

| Parâmetro | O que é |
|---|---|
| `size` | o número de bytes a reservar |

**Retorno:** o ponteiro pro começo da memória nova, que um dia tem que ir pro
`free` ou pro `realloc` (senão vaza); o ponteiro nulo (`NULL`, que na Jinga é
o `Null`) se falhar.

O `void *` que volta vira o ponteiro do tipo da variável, como no C, e o
`sizeof` dá o tamanho do tipo do C:

```ps
import <stdio.h>
import <stdlib.h>

int *p = malloc(4 * sizeof(int))
if p != NULL {
    int n = 0
    while (n < 4) {
        p[n] = n * n
        n++
    }
    n = 0
    while (n < 4) {
        printf("p[%d] == %d\n", n, p[n])
        n++
    }
}
free(p)
```

```saida
p[0] == 0
p[1] == 1
p[2] == 4
p[3] == 9
```

### `calloc`

```c
void *calloc(size_t nmemb, size_t size);
```

Reserva memória pra um array de `nmemb` objetos de `size` bytes cada e põe
zero em todos os bytes. Se der certo, devolve o ponteiro pro primeiro byte do
bloco, alinhado pra qualquer tipo de objeto com alinhamento fundamental. Se
`size` é zero, o que acontece fica a cargo da implementação.

Por causa do alinhamento, o número de bytes reservado pode passar de
`nmemb * size`. Pelo padrão, todos os bits zero não garantem o `0.0` do ponto
flutuante nem o ponteiro nulo; nas plataformas comuns, garantem.

| Parâmetro | O que é |
|---|---|
| `nmemb` | o número de objetos |
| `size` | o tamanho de cada objeto |

**Retorno:** o ponteiro pra memória nova, que um dia tem que ir pro `free` ou
pro `realloc`; o ponteiro nulo se falhar.

```ps
import <stdlib.h>

double *v = calloc(4, sizeof(double))
post(v[0], v[1], v[2], v[3])
free(v)
```

```saida
0.0 0.0 0.0 0.0
```

### `realloc`

```c
void *realloc(void *ptr, size_t size);
```

Muda o tamanho do bloco apontado por `ptr`, que veio do `malloc`, do `calloc`
ou do `realloc` e ainda não foi solto (senão, o comportamento é indefinido).
Faz isso de um de dois jeitos:

- aumentando ou diminuindo o próprio bloco, quando dá: o conteúdo fica igual
  até o menor dos dois tamanhos, e os bytes novos ficam com valor indefinido;
- reservando um bloco novo de `size` bytes, copiando a parte comum (até o
  menor dos dois tamanhos) e soltando o bloco velho.

Sem memória, o bloco velho não é solto e volta o ponteiro nulo. Com `ptr`
nulo, o `realloc` faz o mesmo que `malloc(size)`. Com `size` zero, o que
acontece fica a cargo da implementação (no C23, passou a ser comportamento
indefinido).

| Parâmetro | O que é |
|---|---|
| `ptr` | o ponteiro pro bloco a mudar |
| `size` | o tamanho novo, em bytes |

**Retorno:** o ponteiro pro começo do bloco novo, que um dia tem que ir pro
`free` ou pro `realloc`; o ponteiro velho deixa de valer, mesmo quando o bloco
mudou no lugar. Se falhar, o ponteiro nulo, e o ponteiro velho continua
valendo (e continua pedindo o `free`).

Por isso o resultado vai pra outra variável, e o bloco velho é solto à mão
quando o `realloc` falha:

```ps
import <stdlib.h>

int *v = malloc(3 * sizeof(int))
v[0] = 10
v[1] = 20
v[2] = 30
int *maior = realloc(v, 5 * sizeof(int))
if maior == NULL {
    free(v)
}
else {
    maior[3] = 40
    maior[4] = 50
    post(maior[0], maior[1], maior[2], maior[3], maior[4])
    free(maior)
}
```

```saida
10 20 30 40 50
```

### `free`

```c
void free(void *ptr);
```

Solta a memória reservada antes pelo `malloc`, pelo `calloc`, pelo
`aligned_alloc` ou pelo `realloc`. Com `ptr` nulo, não faz nada: por isso o
resultado de qualquer função de reserva, tenha dado certo ou não, pode ir pro
`free`. O comportamento é indefinido se `ptr` não é um valor que uma dessas
funções devolveu, se a memória já foi solta (pelo `free` ou pelo `realloc`,
sem uma reserva nova que tenha devolvido o mesmo ponteiro depois) e se o
programa acessa a memória pelo `ptr` depois do `free`.

| Parâmetro | O que é |
|---|---|
| `ptr` | o ponteiro pra memória a soltar |

**Retorno:** nenhum.

Na Jinga, o `free` é o único jeito de soltar a memória do C: o coletor não a
solta. Pôr `NULL` no ponteiro depois do `free` deixa um segundo `free` sem
efeito:

```ps
import <stdlib.h>

int *p = malloc(4 * sizeof(int))
free(p)
p = NULL
free(p)
post(p == NULL)
```

```saida
True
```

### `aligned_alloc`

```c
void *aligned_alloc(size_t alignment, size_t size);
```

Reserva `size` bytes de memória sem inicializar, com o alinhamento
`alignment`. O `size` tem que ser múltiplo de `alignment`. Pelo padrão, um
tamanho inválido ou um alinhamento que a implementação não suporta fazem a
chamada falhar e voltar o ponteiro nulo (o C11 publicado dizia comportamento
indefinido, e o DR 460 corrigiu). No POSIX, os alinhamentos suportados
costumam ser as potências de dois múltiplas de `sizeof(void *)`. A função
serve pro alinhamento acima do fundamental: SIMD, linha de cache, página.

| Parâmetro | O que é |
|---|---|
| `alignment` | o alinhamento, um que a implementação suporte |
| `size` | o número de bytes a reservar, múltiplo de `alignment` |

**Retorno:** o ponteiro pra memória nova, que um dia tem que ir pro `free` ou
pro `realloc`; o ponteiro nulo se falhar.

Na Jinga, o cast `(size_t) p` dá o endereço como número:

```ps
import <stdlib.h>

int *p = aligned_alloc(64, 128)
if p != NULL {
    post((size_t) p % 64)
    free(p)
}
```

```saida
0
```

## Texto em número

### `atof`

```c
double atof(const char *nptr);
```

Interpreta um número de ponto flutuante no texto apontado por `nptr`. Pula os
brancos do começo (os que o `isspace` reconhece) até o primeiro caractere que
não é branco; dali, pega o máximo de caracteres que formam um número de ponto
flutuante válido e o converte. O número válido é um destes:

- **decimal:** sinal opcional (`+` ou `-`); uma sequência não vazia de dígitos
  decimais, com um ponto decimal opcional (o do locale atual; no "C", o `.`),
  que forma a mantissa; e um expoente opcional, `e` ou `E` seguido de sinal
  opcional e de uma sequência não vazia de dígitos decimais (expoente na base
  10);
- **hexadecimal:** sinal opcional; `0x` ou `0X`; uma sequência não vazia de
  dígitos hexadecimais, com um ponto decimal opcional, que forma a mantissa; e
  um expoente opcional, `p` ou `P` seguido de sinal opcional e de uma
  sequência não vazia de dígitos decimais (expoente na base 2);
- **infinito:** sinal opcional e `INF` ou `INFINITY`, sem diferença de caixa;
- **não-número:** sinal opcional e `NAN` ou `NAN(`*sequência*`)`, sem
  diferença de caixa na parte `NAN`; a sequência só tem dígitos, letras
  latinas e `_`. O resultado é um NaN silencioso;
- qualquer outra forma que o locale atual aceite.

| Parâmetro | O que é |
|---|---|
| `nptr` | o texto terminado em nulo a interpretar |

**Retorno:** o `double` correspondente ao texto (na Jinga, um `flo`); `0.0`
quando nenhuma conversão é possível. Com o valor fora da faixa do `double`, o
retorno é indefinido. O `atof` não avisa erro: pra saber onde a leitura parou
ou se o número passou da faixa, o [`strtod`](#strtof-strtod-strtold). O nome
vem de *ASCII to float*.

```ps
import <stdio.h>
import <stdlib.h>

printf("%g\n", atof("  -0.0000000123lixo"))
printf("%g\n", atof("0.012"))
printf("%g\n", atof("15e16"))
printf("%g\n", atof("-0x1afp-2"))
printf("%g\n", atof("inF"))
printf("%g\n", atof("Nan"))
printf("%g\n", atof("0.0"))
printf("%g\n", atof("lixo"))
```

```saida
-1.23e-08
0.012
1.5e+17
-107.75
inf
nan
0
0
```

### `atoi`, `atol`, `atoll`

```c
int atoi(const char *nptr);
long int atol(const char *nptr);
long long int atoll(const char *nptr);
```

Interpretam um número inteiro no texto apontado por `nptr`. A base é sempre
10. Pulam os brancos do começo (os que o `isspace` reconhece) até o primeiro
caractere que não é branco e pegam o máximo de caracteres que formam um
inteiro válido: um sinal opcional (`+` ou `-`) seguido de dígitos. Se o
resultado não cabe no tipo do retorno, o comportamento é indefinido.

| Parâmetro | O que é |
|---|---|
| `nptr` | o texto terminado em nulo a interpretar |

**Retorno:** o inteiro correspondente ao texto (na Jinga, um `int`); `0`
quando nenhuma conversão é possível. Não avisam erro: pra isso, o
[`strtol`](#strtol-strtoll). O nome vem de *ASCII to integer*.

```ps
import <stdlib.h>

post(atoi(" -123lixo"))
post(atoi(" +321poeira"))
post(atoi("0"))
post(atoi("0042"))      # decimal, com os zeros na frente
post(atoi("0x2A"))      # só o "0" do começo entra
post(atoi("lixo"))      # nenhuma conversão
post(atol("2147483648"), atoll("9000000000000000000"))
```

```saida
-123
321
0
42
0
0
2147483648 9000000000000000000
```

### `strtol`, `strtoll`

```c
long int strtol(const char * restrict nptr, char ** restrict endptr, int base);
long long int strtoll(const char * restrict nptr, char ** restrict endptr, int base);
```

Interpretam um número inteiro no texto apontado por `nptr`. Pulam os brancos
do começo (os que o `isspace` reconhece) até o primeiro caractere que não é
branco e pegam o máximo de caracteres que formam um inteiro válido na base
`base`, que é:

- um sinal opcional (`+` ou `-`);
- um prefixo `0` opcional, que diz octal (só com a base 8 ou 0);
- um prefixo `0x` ou `0X` opcional, que diz hexadecimal (só com a base 16 ou
  0);
- uma sequência de dígitos.

As bases válidas são 0 e de 2 a 36. Os dígitos da base 2 são `0` e `1`, os da
base 3 são `0`, `1` e `2`, e assim por diante; acima da base 10, as letras
também são dígitos, de `Aa` (base 11) a `Zz` (base 36), sem diferença de
caixa. O locale atual pode aceitar outras formas. Com a base 0, a base sai do
prefixo: `0` é octal, `0x` ou `0X` é hexadecimal, e sem prefixo é decimal. Se
o sinal de menos fazia parte da entrada, o valor dos dígitos é negado como
pelo menos unário no tipo do resultado.

As funções põem em `*endptr` o ponteiro pro caractere logo depois do último
que interpretaram; com `endptr` nulo, ele é ignorado. Se o texto é vazio ou
não tem a forma esperada, nenhuma conversão acontece, e o próprio `nptr` vai
pra `*endptr` (quando ele não é nulo).

| Parâmetro | O que é |
|---|---|
| `nptr` | o texto terminado em nulo a interpretar |
| `endptr` | o endereço de um `char *` que recebe o ponteiro pro primeiro caractere depois do número, ou `NULL` |
| `base` | a base do número: 0, ou de 2 a 36 |

**Retorno:** o inteiro correspondente ao texto (na Jinga, um `int`). Se o
valor passa da faixa do tipo do retorno, é um erro de faixa: o `errno` vira
`ERANGE` e a volta é `LONG_MAX`, `LONG_MIN`, `LLONG_MAX` ou `LLONG_MIN`,
conforme o sinal e o tipo. Se nenhuma conversão é possível, a volta é `0`.

Na Jinga, o `endptr` aceita o `NULL`, quando não interessa onde a leitura
parou, ou o endereço de um `char *` do programa: `char *fim = NULL` e `&fim`.
Depois da chamada, o `fim` aponta pro resto do texto, e `str(fim)` o traz como
`str`. O `fim` aponta pra dentro do texto que entrou: pra seguir lendo por
ele, ponha o texto num buffer do C (`char buf[64]` com o `strcpy`), porque o
`str` da Jinga entra pelo endereço do próprio texto, que o motor só garante
durante a chamada. O `errno` guarda o que as chamadas anteriores deixaram (a
biblioteca do C pode pôr nele um valor diferente de zero mesmo sem erro), e
por isso é zerado antes de cada `strtol`:

```ps
import <errno.h>
import <stdio.h>
import <stdlib.h>
import <string.h>

char buf[64]
strcpy(buf, "10 200000000000000000000000000000 30 -40 lixo")
char *p = buf
char *fim = NULL
while (True) {
    errno = 0
    long v = strtol(p, &fim, 10)
    if fim == p {
        break
    }
    printf("'%.*s' -> ", (int) (fim - p), p)
    if errno == ERANGE {
        printf("fora da faixa, deu ")
    }
    printf("%ld\n", v)
    p = fim
}
printf("sobrou: '%s'\n", p)
```

```saida
'10' -> 10
' 200000000000000000000000000000' -> fora da faixa, deu 9223372036854775807
' 30' -> 30
' -40' -> -40
sobrou: ' lixo'
```

As bases, com o `NULL` no `endptr`:

```ps
import <stdlib.h>

post(strtol("1010", NULL, 2))
post(strtol("12", NULL, 8))
post(strtol("A", NULL, 16))
post(strtol("lixo", NULL, 36))
post(strtol("012", NULL, 0))
post(strtol("0xA", NULL, 0))
post(strtol("lixo", NULL, 0))
post(strtoll("-9223372036854775808", NULL, 10))
```

```saida
10
10
10
1004316
10
10
0
-9223372036854775808
```

### `strtoul`, `strtoull`

```c
unsigned long int strtoul(const char * restrict nptr, char ** restrict endptr, int base);
unsigned long long int strtoull(const char * restrict nptr, char ** restrict endptr, int base);
```

Interpretam um número inteiro sem sinal no texto apontado por `nptr`, com as
regras do [`strtol`](#strtol-strtoll): os brancos do começo, o sinal
opcional, os prefixos `0` e `0x`, as bases 0 e de 2 a 36 e o `*endptr`. Se o
sinal de menos fazia parte da entrada, o valor dos dígitos é negado como pelo
menos unário no tipo do resultado, que não tem sinal: a conta dá a volta.

| Parâmetro | O que é |
|---|---|
| `nptr` | o texto terminado em nulo a interpretar |
| `endptr` | o endereço de um `char *` que recebe o ponteiro pro primeiro caractere depois do número, ou `NULL` |
| `base` | a base do número: 0, ou de 2 a 36 |

**Retorno:** o inteiro correspondente ao texto (na Jinga, um `int`; acima de
2^63, o inteiro grande). Se o valor passa da faixa, o `errno` vira `ERANGE` e
a volta é `ULONG_MAX` ou `ULLONG_MAX`. Se nenhuma conversão é possível, a
volta é `0`.

O `-40` dá a volta (2^64 - 40), e o laço para no `- 42`: um sinal seguido de
branco não é número, e o `fim` fica onde estava:

```ps
import <errno.h>
import <stdio.h>
import <stdlib.h>
import <string.h>

char buf[64]
strcpy(buf, "10 200000000000000000000000000000 30 -40 - 42")
char *p = buf
char *fim = NULL
while (True) {
    errno = 0
    unsigned long v = strtoul(p, &fim, 10)
    if fim == p {
        break
    }
    printf("'%.*s' -> ", (int) (fim - p), p)
    if errno == ERANGE {
        printf("fora da faixa, deu ")
    }
    printf("%lu\n", v)
    p = fim
}
printf("sobrou: '%s'\n", p)
```

```saida
'10' -> 10
' 200000000000000000000000000000' -> fora da faixa, deu 18446744073709551615
' 30' -> 30
' -40' -> 18446744073709551576
sobrou: ' - 42'
```

### `strtof`, `strtod`, `strtold`

```c
float strtof(const char * restrict nptr, char ** restrict endptr);
double strtod(const char * restrict nptr, char ** restrict endptr);
long double strtold(const char * restrict nptr, char ** restrict endptr);
```

Interpretam um número de ponto flutuante no texto apontado por `nptr`, nas
formas do [`atof`](#atof) (decimal, hexadecimal, infinito, não-número e o que
o locale aceitar), depois de pular os brancos do começo. Põem em `*endptr` o
ponteiro pro caractere logo depois do último que interpretaram; com `endptr`
nulo, ele é ignorado.

| Parâmetro | O que é |
|---|---|
| `nptr` | o texto terminado em nulo a interpretar |
| `endptr` | o endereço de um `char *` que recebe o ponteiro pro primeiro caractere depois do número, ou `NULL` |

**Retorno:** o número correspondente ao texto. Se o valor passa da faixa do
tipo, o `errno` vira `ERANGE` e a volta é `HUGE_VAL`, `HUGE_VALF` ou
`HUGE_VALL`. Se nenhuma conversão é possível, a volta é `0`.

```ps
import <errno.h>
import <stdio.h>
import <stdlib.h>
import <string.h>

char buf[96]
strcpy(buf, "111.11 -2.22 Nan nan(2) inF 0X1.BC70A3D70A3D7P+6  1.18973e+4932zzz")
char *p = buf
char *fim = NULL
while (True) {
    errno = 0
    double f = strtod(p, &fim)
    if fim == p {
        break
    }
    printf("'%.*s' -> ", (int) (fim - p), p)
    if errno == ERANGE {
        printf("fora da faixa, deu ")
    }
    printf("%f\n", f)
    p = fim
}
printf("sobrou: '%s'\n", p)
```

```saida
'111.11' -> 111.110000
' -2.22' -> -2.220000
' Nan' -> nan
' nan(2)' -> nan
' inF' -> inf
' 0X1.BC70A3D70A3D7P+6' -> 111.110000
'  1.18973e+4932' -> fora da faixa, deu inf
sobrou: 'zzz'
```

Na Jinga, os três voltam como `flo`. O do `strtof` mostra o valor que o
`float` guarda de verdade (o 0,1 mais perto que cabe em 32 bits), e o do
`strtold` perde o que passa do `double`:

```ps
import <stdlib.h>

float f = strtof("0.1", NULL)
double d = strtod("0.1", NULL)
long double ld = strtold("0.1", NULL)
post(f, d, ld)
post(strtod("  -0.0000000123lixo", NULL), strtod("lixo", NULL))
```

```saida
0.10000000149011612 0.1 0.1
-1.23e-08 0.0
```

## Números pseudoaleatórios

### `rand`

```c
int rand(void);
```

Devolve um inteiro pseudoaleatório entre `0` e [`RAND_MAX`](#rand_max), os
dois inclusive. O [`srand`](#srand) semeia o gerador que o `rand` usa. Se o
`rand` é chamado antes de qualquer `srand`, ele age como se a semente fosse
`srand(1)`. Cada vez que o gerador é semeado com o mesmo valor, ele produz a
mesma sequência.

Não há garantia de qualidade da sequência. Algumas implementações do passado
tinham falhas sérias de aleatoriedade, distribuição e período (um exemplo
conhecido: o bit mais baixo alternando entre 1 e 0 a cada chamada). O `rand`
não serve pra uso sério, como criptografia. O POSIX exige que o período do
gerador seja de pelo menos 2^32.

Sem parâmetros.

**Retorno:** um inteiro pseudoaleatório entre `0` e `RAND_MAX`, inclusive.

O exemplo joga um dado. O `1 + rand() % 6`, comum, é enviesado; a
cppreference descarta o que sobra da divisão. Na Jinga, `RAND_MAX + 1` não
estoura, e o `//` é a divisão inteira:

```ps
import <stdlib.h>
import <time.h>

srand(time(NULL))
int dado = 7
while (dado > 6) {
    dado = 1 + rand() // ((RAND_MAX + 1) // 6)
}
post(dado >= 1 and dado <= 6)
int r = rand()
post(r >= 0 and r <= RAND_MAX)
```

```saida
True
True
```

### `srand`

```c
void srand(unsigned int seed);
```

Semeia o gerador de números pseudoaleatórios do [`rand`](#rand) com o valor
`seed`. Se o `rand` é chamado antes de qualquer `srand`, ele age como se a
semente fosse `srand(1)`. Cada vez que o `rand` é semeado com o mesmo `seed`,
ele produz a mesma sequência.

Em geral, o gerador é semeado uma vez só, no começo do programa, antes de
qualquer `rand`, e não de novo a cada lote de números. O costume é usar o
`time(0)` como semente: o padrão não garante que o `time_t` seja inteiro, mas
ele é em toda implementação grande, e o POSIX exige.

| Parâmetro | O que é |
|---|---|
| `seed` | a semente |

**Retorno:** nenhum.

Na Jinga, o número entra como `unsigned int` do C e, fora da faixa, dá a
volta, como na atribuição do C (`srand(-1)` é o mesmo que
`srand(4294967295)`). A mesma semente repete a sequência:

```ps
import <stdlib.h>

srand(42)
int a = rand()
int b = rand()
srand(42)
post(rand() == a, rand() == b)
```

```saida
True True
```

## Ordenação e busca

### `qsort`

```c
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
```

Ordena em ordem crescente o array apontado por `base`, de `nmemb` elementos
de `size` bytes cada. A função apontada por `compar` compara os objetos. Se o
`compar` diz que dois elementos são equivalentes, a ordem deles no resultado
não é especificada.

Apesar do nome, nem o C nem o POSIX exigem que o `qsort` seja um quicksort, e
nenhum dos dois garante complexidade ou estabilidade.

| Parâmetro | O que é |
|---|---|
| `base` | o ponteiro pro array a ordenar |
| `nmemb` | o número de elementos do array |
| `size` | o tamanho de cada elemento, em bytes |
| `compar` | a função de comparação: devolve um inteiro negativo se o primeiro argumento é menor que o segundo, positivo se é maior, e zero se são equivalentes. A assinatura equivale a `int cmp(const void *a, const void *b)`; ela não muda os objetos que recebe e dá o mesmo resultado pros mesmos objetos, onde quer que eles estejam no array |

**Retorno:** nenhum.

Na Jinga, o comparador é uma funct com o protótipo do C,
`int funct compara(const void *a, const void *b)`, e o cast
`*(const int *) a` lê o elemento. O array do C entra como o ponteiro pro
primeiro elemento:

```ps
import <limits.h>
import <stdlib.h>

int v[7] = {-2, 99, 0, -743, 2, INT_MIN, 4}

int funct compara(const void *a, const void *b) {
    int x = *(const int *) a
    int y = *(const int *) b
    if x < y {
        return -1
    }
    if x > y {
        return 1
    }
    return 0
}

qsort(v, 7, sizeof(int), compara)
list ordem = []
for each x in v {
    addEnd(ordem, str(x))
}
post(" ".join(ordem))
```

```saida
-2147483648 -743 -2 0 2 4 99
```

O atalho `return *(const int *) a - *(const int *) b` só vale quando a
diferença cabe no `int` do C. A conta é da Jinga, mas o valor que a funct
devolve vira o `int` do C dando a volta: com o `INT_MIN` do exemplo,
`INT_MIN - 99` sai positivo, e a ordem sai errada. No C, o mesmo atalho é estouro de inteiro,
comportamento indefinido, e a cppreference faz o mesmo aviso.

Erro dentro do comparador não atravessa o C. Numa `int funct`, vale o
sentinela da Jinga ([6.4 Functs tipadas](../linguagem/06-funcoes.md)): o erro
sai no stderr, a funct devolve `500`, e o `qsort` segue com esse número, sem
ordem garantida. Numa funct sem o sentinela (sem tipo de retorno, ou com um
tipo que não é `int` nem `bool`), o primeiro erro fica guardado, as chamadas
seguintes devolvem zero sem rodar a funct, e o erro sobe quando o `qsort`
volta: um `catch` em volta da chamada o pega.

```ps
import <stdlib.h>

int v[2] = {2, 1}

int funct compara(const void *a, const void *b) {
    raise ValueError("deu ruim no comparador")
}

qsort(v, 2, sizeof(int), compara)
post("o programa segue")
```

```saida
o programa segue
```

```erro
int funct compara() devolveu 500 no lugar do erro
```

### `bsearch`

```c
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
```

Procura um elemento igual ao apontado por `key` no array apontado por `base`,
de `nmemb` elementos de `size` bytes cada. O array tem que estar particionado
em relação a `key`: todos os elementos menores que `key` vêm antes dos
iguais, e os iguais antes dos maiores (um array ordenado cumpre isso). A
função apontada por `compar` compara os elementos. O comportamento é
indefinido se o array não está particionado em ordem crescente pelo mesmo
critério do `compar`. Se vários elementos são iguais ao procurado, não é
especificado qual deles volta.

Apesar do nome, nem o C nem o POSIX exigem que o `bsearch` seja uma busca
binária, e nenhum dos dois garante complexidade.

| Parâmetro | O que é |
|---|---|
| `key` | o ponteiro pro elemento procurado |
| `base` | o ponteiro pro array |
| `nmemb` | o número de elementos do array |
| `size` | o tamanho de cada elemento, em bytes |
| `compar` | a função de comparação, como a do [`qsort`](#qsort): o `key` vai como primeiro argumento, e um elemento do array como segundo |

**Retorno:** o ponteiro pro elemento do array igual a `*key`, ou o ponteiro
nulo se não acha.

Na Jinga, `&chave` leva o endereço de uma variável do programa, que passa a
morar na memória do C com o tipo do C, e a diferença entre dois ponteiros
(`achou - v`) dá a posição:

```ps
import <stdio.h>
import <stdlib.h>

int v[6] = {1, 2, 3, 5, 7, 9}

int funct compara(const void *a, const void *b) {
    int x = *(const int *) a
    int y = *(const int *) b
    if x < y {
        return -1
    }
    if x > y {
        return 1
    }
    return 0
}

int chave = 7
int *achou = bsearch(&chave, v, 6, sizeof(int), compara)
if achou != NULL {
    printf("achou o %d na posição %td\n", *achou, achou - v)
}
chave = 4
achou = bsearch(&chave, v, 6, sizeof(int), compara)
if achou == NULL {
    printf("o %d não está no array\n", chave)
}
```

```saida
achou o 7 na posição 4
o 4 não está no array
```

## Aritmética inteira

### `abs`, `labs`, `llabs`

```c
int abs(int j);
long int labs(long int j);
long long int llabs(long long int j);
```

Calculam o valor absoluto de um inteiro. O comportamento é indefinido se o
resultado não cabe no tipo do retorno: no complemento de dois, o valor
absoluto do número mais negativo não cabe (com o `int` de 32 bits, o
`INT_MIN` é `-2147483648`, e `2147483648` passa do `INT_MAX`).

| Parâmetro | O que é |
|---|---|
| `j` | um inteiro |

**Retorno:** o valor absoluto de `j`, quando ele cabe no tipo.

Na Jinga, o nome `abs` é o embutido da Jinga
([`abs(n)`](../builtins/abs/abs.md)), mesmo com o `import <stdlib.h>`: ele
aceita `int` e `flo` e não tem o limite do `int` do C. O valor absoluto do C
fica pelos outros nomes que o C tem, `labs` e `llabs`:

```ps
import <stdlib.h>

post(abs(-3), abs(-2.5), abs(-2147483648))
post(labs(-3), llabs(-9000000000))
```

```saida
3 2.5 2147483648
3 9000000000
```

### `div`, `ldiv`, `lldiv`

```c
div_t div(int numer, int denom);
ldiv_t ldiv(long int numer, long int denom);
lldiv_t lldiv(long long int numer, long long int denom);
```

Calculam ao mesmo tempo o quociente (o campo `quot`) e o resto (o campo
`rem`) da divisão de `numer` por `denom`. O quociente é o quociente algébrico
sem a parte fracionária, cortado na direção do zero, e o resto satisfaz
`quot * denom + rem == numer`. Desde o C99, o quociente é o resultado de
`numer / denom` e o resto o de `numer % denom`, calculados juntos.

Antes do C99, a direção do arredondamento e o sinal do resto do `/` e do `%`
com operando negativo ficavam a cargo da implementação, e no `div` e no
`ldiv` já eram definidos. Em muitas plataformas, uma instrução só da CPU dá o
quociente e o resto, e a função pode usá-la; os compiladores juntam `/` e `%`
vizinhos, então o `div` nem sempre é preciso pelo desempenho.

| Parâmetro | O que é |
|---|---|
| `numer` | o numerador |
| `denom` | o denominador |

**Retorno:** o quociente e o resto num [`div_t`, `ldiv_t` ou
`lldiv_t`](#div_t-ldiv_t-lldiv_t), quando os dois cabem no tipo (`int`,
`long` ou `long long`); se não cabem, o comportamento é indefinido.

Na Jinga, o `//` e o `%` arredondam pra baixo, como no Python, e com número
negativo dão outra coisa que o `div` do C:

```ps
import <stdlib.h>

div_t d = div(17, 5)
post(d.quot, d.rem)
div_t n = div(-17, 5)
post(n.quot, n.rem, n.quot * 5 + n.rem == -17)
post(-17 // 5, -17 % 5)
ldiv_t l = ldiv(100000000000, 7)
post(l.quot, l.rem)
```

```saida
3 2
-3 -2 True
-4 3
14285714285 5
```

## Fim do programa

### `exit`

```c
_Noreturn void exit(int status);
```

Encerra o programa normalmente, com esta limpeza, nesta ordem:

1. as funções passadas ao [`atexit`](#atexit) são chamadas, na ordem inversa
   do registro;
2. todos os fluxos do C são esvaziados e fechados;
3. os arquivos criados pelo `tmpfile` são apagados;
4. o controle volta pro ambiente: com `status` zero ou
   [`EXIT_SUCCESS`](#exit_success-exit_failure), um status de sucesso; com
   `EXIT_FAILURE`, um de falha; com outro valor, um status que fica a cargo da
   implementação.

As funções do [`at_quick_exit`](#at_quick_exit) não são chamadas. Chamar o
`exit` mais de uma vez, ou o `exit` e o `quick_exit`, é comportamento
indefinido no C. Voltar do `main` faz o mesmo que o `exit`.

| Parâmetro | O que é |
|---|---|
| `status` | o código de saída do programa |

**Retorno:** nenhum; a função não volta.

Na Jinga, o `exit` do C é o [`sys.exit`](../sys/exit/exit.md): o programa
desenrola antes de sair, os `finally` (e os `using`) em volta rodam, de dentro
pra fora, e nenhum `catch` o pega. Depois, as functs do `atexit` rodam com a
VM viva, na ordem inversa do registro; um `exit` dentro de uma delas não para
as outras nem muda o código. O fim do arquivo faz o papel da volta do `main`.
O sistema guarda 8 bits do código (o resto por 256).

```ps
import <stdlib.h>

Null funct no_fim() {
    post("3. a funct do atexit")
}

atexit(no_fim)
try {
    post("1. dentro do try")
    exit(EXIT_SUCCESS)
    post("esta linha não roda")
}
catch (e) {
    post("o catch não pega o exit")
}
finally {
    post("2. o finally")
}
post("nem esta")
```

```saida
1. dentro do try
2. o finally
3. a funct do atexit
```

### `atexit`

```c
int atexit(void (*func)(void));
```

Registra a função apontada por `func` pra ser chamada no fim normal do
programa (pelo `exit` ou pela volta do `main`). As funções são chamadas na
ordem inversa do registro: a última registrada roda primeiro. A mesma função
pode ser registrada mais de uma vez. A implementação garante pelo menos 32
registros; o limite exato fica a cargo dela.

| Parâmetro | O que é |
|---|---|
| `func` | o ponteiro pra função a chamar no fim normal do programa |

**Retorno:** `0` se o registro deu certo; diferente de zero se não.

Na Jinga, a `func` é uma funct sem parâmetros (`Null funct tchau()`), e a
lista não tem o limite de 32: cresce com a memória. As functs rodam com a VM
viva no fim do arquivo, no `exit`, no `sys.exit` e também depois de um erro
que ninguém pegou, antes da mensagem dele. Erro dentro de uma delas sai no
stderr, as outras rodam, e o programa sai com 1.

```ps
import <stdlib.h>

Null funct f1() {
    post("f1")
}

Null funct f2() {
    post("f2")
}

if atexit(f1) == 0 and atexit(f2) == 0 and atexit(f2) == 0 {
    post("registradas: f1, f2 e f2 de novo")
}
```

```saida
registradas: f1, f2 e f2 de novo
f2
f2
f1
```

### `quick_exit`

```c
_Noreturn void quick_exit(int status);
```

Encerra o programa normalmente, sem a limpeza completa dos recursos. As
funções passadas ao [`at_quick_exit`](#at_quick_exit) são chamadas, na ordem
inversa do registro; depois, o `quick_exit` chama o `_Exit(status)`. As
funções do `atexit` não são chamadas, nem os tratadores de sinal instalados
pelo `signal`.

| Parâmetro | O que é |
|---|---|
| `status` | o código de saída do programa |

**Retorno:** nenhum; a função não volta.

Na Jinga, os `finally` em volta não rodam, nem as functs do `atexit`; as do
`at_quick_exit` rodam com a VM viva, e o que o programa escreveu no stdout sai
antes do fim (o `fflush` que o exemplo da cppreference faz à mão não é
preciso):

```ps
import <stdlib.h>

Null funct primeira() {
    post("registrada primeiro")
}

Null funct segunda() {
    post("registrada depois")
}

Null funct do_atexit() {
    post("não roda")
}

at_quick_exit(primeira)
at_quick_exit(segunda)
atexit(do_atexit)
try {
    quick_exit(0)
}
finally {
    post("não roda")
}
```

```saida
registrada depois
registrada primeiro
```

### `at_quick_exit`

```c
int at_quick_exit(void (*func)(void));
```

Registra a função apontada por `func` pra ser chamada no fim rápido do
programa, pelo [`quick_exit`](#quick_exit). A implementação garante pelo menos
32 registros; o limite exato fica a cargo dela. As funções registradas não
são chamadas no fim normal (`exit`): o que tem que rodar nos dois casos vai
também pro `atexit`.

| Parâmetro | O que é |
|---|---|
| `func` | o ponteiro pra função a chamar no fim rápido do programa |

**Retorno:** `0` se o registro deu certo; diferente de zero se não.

Na Jinga, a `func` é uma funct sem parâmetros. No fim normal do programa, ela
não roda:

```ps
import <stdlib.h>

Null funct rapida() {
    post("só no quick_exit")
}

post(at_quick_exit(rapida))
post("fim normal: a funct do at_quick_exit não roda")
```

```saida
0
fim normal: a funct do at_quick_exit não roda
```

### `_Exit`

```c
_Noreturn void _Exit(int status);
```

Encerra o programa normalmente, sem limpar os recursos. As funções do
`atexit` e do `at_quick_exit` não são chamadas. Se os fluxos abertos com dados
ainda no buffer são esvaziados, se eles são fechados e se os arquivos
temporários são apagados, fica a cargo da implementação. O status volta como
no [`exit`](#exit): zero ou `EXIT_SUCCESS` dão sucesso, `EXIT_FAILURE` dá
falha, e outro valor dá um status que fica a cargo da implementação.

| Parâmetro | O que é |
|---|---|
| `status` | o código de saída do programa |

**Retorno:** nenhum; a função não volta.

Na Jinga, o processo sai na hora: nenhum `finally` roda, nenhuma funct do
`atexit` nem do `at_quick_exit`. O que ainda está no buffer do stdout se
perde (com a saída num cano ou num arquivo, o buffer guarda tudo o que não
passou por um `fflush`); por isso o `fflush(stdout)` antes, como no exemplo da
cppreference:

```ps
import <stdio.h>
import <stdlib.h>

Null funct tchau() {
    post("não roda")
}

atexit(tchau)
try {
    post("antes do _Exit")
    fflush(stdout)
    _Exit(0)
}
finally {
    post("não roda")
}
```

```saida
antes do _Exit
```

### `abort`

```c
_Noreturn void abort(void);
```

Causa o fim anormal do programa, a não ser que o sinal `SIGABRT` esteja sendo
pego por um tratador passado ao `signal` e esse tratador não volte. As funções
do `atexit` não são chamadas. Se os recursos abertos, como os arquivos, são
fechados fica a cargo da implementação. Um status de falha, que fica a cargo
da implementação, volta pro ambiente. O POSIX diz que o `abort` passa por cima
do bloqueio e do descarte do `SIGABRT`.

Sem parâmetros.

**Retorno:** nenhum; a função não volta.

Na Jinga, o processo morre pelo `SIGABRT`: nenhum `finally` roda, nenhuma
funct do `atexit`, e o shell dá o código 134 (128 mais o 6 do `SIGABRT`). O
`signal.h` ainda não está na Jinga, então não há como pegar o `SIGABRT`. Se o
que está no buffer do stdout sai antes de o processo morrer fica a cargo do
sistema, como os arquivos abertos. Este exemplo não tem saída pra mostrar:
rodando, ele morre sem escrever nada.

```ps
import <stdlib.h>

Null funct no_fim() {
    post("não roda")
}

atexit(no_fim)
try {
    abort()
}
finally {
    post("não roda")
}
```

## O ambiente

### `getenv`

```c
char *getenv(const char *name);
```

Procura, na lista de ambiente que o sistema dá, a variável de nome `name` e
devolve um ponteiro pro texto associado a ela. O conjunto de variáveis e os
jeitos de mudá-lo ficam a cargo da implementação. Outra chamada ao `getenv`,
ou ao `setenv`, ao `unsetenv` ou ao `putenv` do POSIX, pode invalidar o
ponteiro de uma chamada anterior ou mudar o texto dela. Mudar o texto
devolvido é comportamento indefinido.

| Parâmetro | O que é |
|---|---|
| `name` | o nome da variável de ambiente, um texto terminado em nulo |

**Retorno:** o ponteiro pro valor da variável, ou o ponteiro nulo se ela não
existe.

Na Jinga, o `str(p)` copia o valor pra um `str`, que continua valendo depois
de outra chamada. Confira o `NULL` antes: o `str` de um ponteiro nulo dá o
texto `"Null"`. O [`os.getenv`](../os/getenv/getenv.md) da Jinga lê a mesma
lista:

```ps
import os
import <stdlib.h>

char *caminho = getenv("PATH")
if caminho != NULL {
    post("o mesmo do os.getenv:", str(caminho) == os.getenv("PATH"))
}
char *nada = getenv("JINGA_VARIAVEL_QUE_NAO_EXISTE")
post("a que não existe dá NULL:", nada == NULL)
```

```saida
o mesmo do os.getenv: True
a que não existe dá NULL: True
```

### `system`

```c
int system(const char *string);
```

Chama o interpretador de comandos do ambiente com o comando `string` e
devolve um valor que fica a cargo da implementação, em geral o que o programa
chamado devolve. Com `string` nulo, não roda nada: só confere se o ambiente
tem um interpretador de comandos e devolve diferente de zero se, e só se, ele
existe.

| Parâmetro | O que é |
|---|---|
| `string` | o comando a rodar no interpretador, ou `NULL` pra só conferir se ele existe |

**Retorno:** um valor que fica a cargo da implementação; com `string` nulo,
diferente de zero se o interpretador existe, e zero se não. No POSIX, o valor
se decompõe com o `WEXITSTATUS` e o `WSTOPSIG`, e o `popen` entrega a quem
chama a saída do comando, que o `system` não entrega.

Na Jinga, no Linux e no macOS, o interpretador é o `sh`, e o valor é o status
do `wait` do POSIX: o código de saída do comando fica no segundo byte
(`r >> 8`). O comando escreve direto no stdout do processo: o que o programa
já escreveu e ainda está no buffer sai depois do que o comando escreve, a não
ser que passe por um `fflush(stdout)` antes. Pra pegar a saída do comando
como texto, o [`os.run`](../os/run/run.md) da Jinga.

```ps
import <stdio.h>
import <stdlib.h>

post("antes do comando")
fflush(stdout)
int r = system("echo oi")
post("o comando devolveu", r)
post("com exit 3:", system("exit 3") >> 8)
post("tem interpretador de comandos:", system(NULL) != 0)
```

```saida
antes do comando
oi
o comando devolveu 0
com exit 3: 3
tem interpretador de comandos: True
```

## Caracteres multibyte

O `locale.h` ainda não está na Jinga: vale o locale "C", em que o
[`MB_CUR_MAX`](#mb_cur_max) é 1 e cada caractere ocupa um byte. O texto da
Jinga é UTF-8, e o locale "C" não lê UTF-8: pra contar os caracteres de um
`str`, o `len()` da Jinga. O caractere largo é o `wchar_t`, que o
`import <stdlib.h>` já traz.

### `mblen`

```c
int mblen(const char *s, size_t n);
```

Determina o tamanho, em bytes, do caractere multibyte cujo primeiro byte é
apontado por `s`. Com `s` nulo, zera o estado global de conversão e diz se a
codificação usa sequências de troca (*shift*). Equivale a
`mbtowc((wchar_t *) 0, s, n)`, sem mexer no estado de conversão do `mbtowc`.

Cada chamada atualiza o estado global de conversão, um objeto estático do
tipo `mbstate_t` que só a função conhece. Se a codificação usa estados de
troca, é preciso cuidado pra não voltar atrás nem varrer o texto mais de uma
vez.

| Parâmetro | O que é |
|---|---|
| `s` | o ponteiro pro caractere multibyte |
| `n` | o limite de bytes de `s` que podem ser examinados |

**Retorno:** com `s` não nulo, o número de bytes do caractere multibyte; `-1`
se os primeiros bytes não formam um caractere multibyte válido; `0` se `s`
aponta pro caractere nulo. Com `s` nulo, zera o estado e devolve `0` se a
codificação não depende de estado (não usa sequências de troca), ou diferente
de zero se depende.

O exemplo conta os caracteres de um texto ASCII, andando pelo ponteiro:

```ps
import <stdlib.h>
import <string.h>

char buf[32]
strcpy(buf, "Jinga e C")
char *p = buf
int caracteres = 0
mblen(NULL, 0)
while (*p != 0) {
    int k = mblen(p, MB_CUR_MAX)
    if k <= 0 {
        break
    }
    p = p + k
    caracteres++
}
post(caracteres, strlen(buf))
post(mblen("", 1))
```

```saida
9 9
0
```

### `mbtowc`

```c
int mbtowc(wchar_t * restrict pwc, const char * restrict s, size_t n);
```

Converte o caractere multibyte cujo primeiro byte é apontado por `s` num
caractere largo, escrito em `*pwc` se `pwc` não é nulo. Com `s` nulo, zera o
estado global de conversão e diz se a codificação usa sequências de troca.

Cada chamada atualiza o estado global de conversão, um objeto estático do
tipo `mbstate_t` que só a função conhece. Se a codificação usa estados de
troca, é preciso cuidado pra não voltar atrás nem varrer o texto mais de uma
vez.

| Parâmetro | O que é |
|---|---|
| `pwc` | o ponteiro pro caractere largo de saída |
| `s` | o ponteiro pro caractere multibyte |
| `n` | o limite de bytes de `s` que podem ser examinados |

**Retorno:** com `s` não nulo, o número de bytes do caractere multibyte; `-1`
se os primeiros bytes não formam um caractere multibyte válido; `0` se `s`
aponta pro caractere nulo. Com `s` nulo, zera o estado e devolve `0` se a
codificação não depende de estado, ou diferente de zero se depende.

Na Jinga, `&wc` leva o endereço de uma variável `wchar_t` do programa:

```ps
import <stdlib.h>

wchar_t wc = 0
mbtowc(NULL, NULL, 0)
int n = mbtowc(&wc, "Jinga", 5)
post(n, wc, chr(wc))
post(mbtowc(&wc, "", 1))
```

```saida
1 74 J
0
```

### `wctomb`

```c
int wctomb(char *s, wchar_t wchar);
```

Converte o caractere largo `wchar` na representação multibyte e a guarda, com
as sequências de troca que precisar, no array de char cujo primeiro elemento é
apontado por `s`. No máximo `MB_CUR_MAX` caracteres são guardados. A conversão
depende da categoria `LC_CTYPE` do locale. Se `wchar` é o caractere nulo, o
byte nulo é escrito em `s`, depois das sequências de troca que levam de volta
ao estado inicial. Com `s` nulo, zera o estado global de conversão e diz se a
codificação usa sequências de troca.

Cada chamada atualiza o estado global de conversão, um objeto estático do
tipo `mbstate_t` que só a função conhece; se a codificação usa estados de
troca, a função não é reentrante.

| Parâmetro | O que é |
|---|---|
| `s` | o ponteiro pro array de char de saída |
| `wchar` | o caractere largo a converter |

**Retorno:** com `s` não nulo, o número de bytes da representação multibyte
de `wchar`, ou `-1` se `wchar` não é um caractere válido. Com `s` nulo, zera o
estado e devolve `0` se a codificação não depende de estado, ou diferente de
zero se depende.

```ps
import <stdlib.h>

char mb[8]
post(wctomb(NULL, 0))
int n = wctomb(mb, 74)
post(n, mb[0], chr(mb[0]))
```

```saida
0
1 74 J
```

### `mbstowcs`

```c
size_t mbstowcs(wchar_t * restrict pwcs, const char * restrict s, size_t n);
```

Converte o texto multibyte que começa no array apontado por `s` em caracteres
largos, guardados em elementos seguidos do array apontado por `pwcs`. No
máximo `n` caracteres largos são escritos. Cada caractere é convertido como
pelo `mbtowc`, sem mexer no estado de conversão dele. A conversão para
quando:

1. o caractere nulo multibyte foi convertido e guardado;
2. aparece um caractere multibyte inválido no locale atual;
3. o próximo caractere largo a guardar passaria de `n`.

Se os dois arrays se sobrepõem, o comportamento é indefinido. Na maioria das
implementações, o `mbstowcs` atualiza um `mbstate_t` estático global enquanto
anda pelo texto. O POSIX tem uma extensão comum: com `pwcs` nulo, a função
devolve quantos caracteres largos seriam escritos.

| Parâmetro | O que é |
|---|---|
| `pwcs` | o ponteiro pro array de caracteres largos de saída |
| `s` | o ponteiro pro primeiro elemento do texto multibyte terminado em nulo |
| `n` | o número de caracteres largos que cabem em `pwcs` |

**Retorno:** o número de caracteres largos escritos, sem contar o `L'\0'` do
fim; `(size_t) -1` se apareceu um caractere multibyte inválido.

Na Jinga, o array do C `wchar_t larga[8]` recebe o texto, e cada elemento é o
código do caractere:

```ps
import <stdlib.h>

post(mbstowcs(NULL, "texto", 0))
wchar_t larga[8]
size_t n = mbstowcs(larga, "texto", 8)
post(n, larga[0], chr(larga[0]), larga[5])
```

```saida
5
5 116 t 0
```

### `wcstombs`

```c
size_t wcstombs(char * restrict s, const wchar_t * restrict pwcs, size_t n);
```

Converte os caracteres largos do array apontado por `pwcs` na representação
multibyte, começando no estado inicial de troca, e guarda os bytes em
elementos seguidos do array de char apontado por `s`. No máximo `n` bytes são
escritos. Cada caractere é convertido como pelo `wctomb`, sem mexer no estado
de conversão dele. A conversão para quando:

1. o caractere nulo `L'\0'` foi convertido e guardado (os bytes guardados são
   a sequência que volta ao estado inicial, se precisar, e o `'\0'`);
2. aparece um `wchar_t` que não é caractere válido no locale atual;
3. o próximo caractere multibyte a guardar passaria de `n`.

Se os dois arrays se sobrepõem, o comportamento não é especificado. Na maioria
das implementações, a função atualiza um `mbstate_t` estático global. O POSIX
tem uma extensão comum: com `s` nulo, a função devolve quantos bytes seriam
escritos.

| Parâmetro | O que é |
|---|---|
| `s` | o ponteiro pro array de char de saída |
| `pwcs` | o ponteiro pro primeiro elemento do texto largo terminado em nulo |
| `n` | o número de bytes que cabem em `s` |

**Retorno:** o número de bytes escritos, contando as sequências de troca e
sem contar o `'\0'` do fim; `(size_t) -1` se apareceu um caractere largo
inválido.

```ps
import <stdlib.h>

wchar_t larga[8]
mbstowcs(larga, "texto", 8)
post(wcstombs(NULL, larga, 0))
char volta[8]
size_t n = wcstombs(volta, larga, 8)
post(n, str(volta))
```

```saida
5
5 texto
```

## Tipos

### `div_t`, `ldiv_t`, `lldiv_t`

```c
typedef struct { int quot; int rem; } div_t;
typedef struct { long quot; long rem; } ldiv_t;
typedef struct { long long quot; long long rem; } lldiv_t;
```

As estruturas que o `div`, o `ldiv` e o `lldiv` devolvem: o quociente no
campo `quot` e o resto no `rem`. O padrão não fixa a ordem dos dois campos (o
`rem` pode vir antes do `quot`): o código portável usa os nomes.

| Tipo | Campos | Quem devolve |
|---|---|---|
| `div_t` | `int quot`, `int rem` | [`div`](#div-ldiv-lldiv) |
| `ldiv_t` | `long quot`, `long rem` | [`ldiv`](#div-ldiv-lldiv) |
| `lldiv_t` | `long long quot`, `long long rem` | [`lldiv`](#div-ldiv-lldiv) |

Na Jinga, o valor é um objeto do C: os campos se leem com o ponto (`d.quot`),
e a variável declarada sem `=` nasce zerada:

```ps
import <stdlib.h>

div_t d = div(7, 2)
post(d.quot, d.rem, type(d))
lldiv_t z
post(z.quot, z.rem, type(z))
```

```saida
3 1 div_t
0 0 lldiv_t
```

## Constantes

### `EXIT_SUCCESS`, `EXIT_FAILURE`

Expandem pra expressões constantes inteiras que podem ir pro [`exit`](#exit)
(e, no C, ser a volta do `main`) e dizem o status da execução do programa. O
`EXIT_SUCCESS` e o zero dizem sucesso; o padrão não exige que o
`EXIT_SUCCESS` seja zero.

| Constante | O que diz | Valor no Linux e no macOS |
|---|---|---|
| `EXIT_SUCCESS` | a execução deu certo | `0` |
| `EXIT_FAILURE` | a execução falhou | `1` |

O exemplo da cppreference: o arquivo não abre (a pasta do exemplo está
vazia), o motivo vai pro stderr, e o programa sai com `EXIT_FAILURE`:

```ps
import <stdio.h>
import <stdlib.h>

post("abrindo dados.txt")
FILE *f = fopen("dados.txt", "r")
if f == NULL {
    fprintf(stderr, "não deu pra abrir dados.txt\n")
    exit(EXIT_FAILURE)
}
fclose(f)
post("leu")
```

```saida
abrindo dados.txt
```

```erro
não deu pra abrir dados.txt
```

### `RAND_MAX`

Expande pra uma expressão constante inteira igual ao maior valor que o
[`rand`](#rand) devolve. O valor fica a cargo da implementação; o padrão
garante pelo menos `32767`.

| Constante | Valor no Linux e no macOS |
|---|---|
| `RAND_MAX` | `2147483647`, o mesmo do `INT_MAX` |

Na Jinga, a conta é da Jinga: `rand() / RAND_MAX` já dá um `flo` entre 0 e 1,
sem o cast `(double)` do C:

```ps
import <limits.h>
import <stdlib.h>

post(RAND_MAX >= 32767, RAND_MAX <= INT_MAX)
flo x = rand() / RAND_MAX
post(type(x), x >= 0.0 and x <= 1.0)
```

```saida
True True
flo True
```

### `MB_CUR_MAX`

O número máximo de bytes de um caractere multibyte no locale atual. Não é
constante: no C, é uma macro que pergunta ao locale, e na Jinga é lida na
hora. A Jinga não muda o locale (o `locale.h` ainda não está na Jinga), e no
locale "C" ele vale 1.

| Nome | Tipo | Valor na Jinga |
|---|---|---|
| `MB_CUR_MAX` | `size_t` | `1` (o locale "C") |

```ps
import <stdlib.h>

post(MB_CUR_MAX)
```

```saida
1
```

## Fica de fora

Estes nomes do `stdlib.h` não estão na Jinga. Usar um deles é `NameError`
antes de rodar, com o motivo:

| Nome | Motivo |
|---|---|
| `abort_handler_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `bsearch_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `constraint_handler_t` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `getenv_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `ignore_handler_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `mbstowcs_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `qsort_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `set_constraint_handler_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `wcstombs_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |
| `wctomb_s` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |

```ps
import <stdlib.h>

int v[2] = {2, 1}
qsort_s(v, 2, sizeof(int), NULL, NULL)
# NameError: qsort_s fica de fora: é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS (doc: o C na Jinga)
```
