# stdio.h — entrada e saída

As funções do `stdio.h` abrem e fecham arquivos, leem e escrevem bytes,
caracteres, linhas e texto formatado (`printf`, `scanf`), mudam a posição no
arquivo e dizem o que deu errado. O C chama de *stream* o canal ligado a um
arquivo, a um terminal ou a outro dispositivo, e o programa usa o stream pelo
ponteiro `FILE *`. Os nomes do `stdio.h` valem com `import <stdio.h>` no
topo do arquivo, igual ao `#include <stdio.h>` do C; sem o import, o nome é
`NameError`, com a dica do import que falta ([o C na Jinga](c.md)):

```ps
import <stdio.h>

printf("olá, %s!\n", "Jinga")
```

```saida
olá, Jinga!
```

> Esta página segue a cppreference
> ([en.cppreference.com/w/c/io](https://en.cppreference.com/w/c/io),
> licença [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)),
> traduzida e adaptada à Jinga. Os protótipos são os do C11 (Anexo B do
> N1570), conferidos contra a tabela do motor. Cada exemplo foi rodado.

| Função | O que faz |
|---|---|
| [`fopen`](#fopen) | abre um arquivo |
| [`freopen`](#freopen) | abre outro arquivo num stream que já existe |
| [`fclose`](#fclose) | fecha um arquivo |
| [`fflush`](#fflush) | manda pro sistema o que está no buffer de saída |
| [`setbuf`](#setbuf) | dá ao stream um buffer, ou desliga o buffer |
| [`setvbuf`](#setvbuf) | escolhe o buffer e o modo de buffer do stream |
| [`fread`](#fread) | lê objetos de um arquivo |
| [`fwrite`](#fwrite) | escreve objetos num arquivo |
| [`fgetc`](#fgetc-getc) | lê um caractere de um stream |
| [`getc`](#fgetc-getc) | lê um caractere de um stream (pode ser macro) |
| [`fgets`](#fgets) | lê uma linha de um stream |
| [`fputc`](#fputc-putc) | escreve um caractere num stream |
| [`putc`](#fputc-putc) | escreve um caractere num stream (pode ser macro) |
| [`fputs`](#fputs) | escreve um texto num stream |
| [`getchar`](#getchar) | lê um caractere do `stdin` |
| [`putchar`](#putchar) | escreve um caractere no `stdout` |
| [`puts`](#puts) | escreve um texto e a quebra de linha no `stdout` |
| [`ungetc`](#ungetc) | devolve um caractere ao stream |
| [`scanf`](#scanf-fscanf-sscanf) | lê texto formatado do `stdin` |
| [`fscanf`](#scanf-fscanf-sscanf) | lê texto formatado de um stream |
| [`sscanf`](#scanf-fscanf-sscanf) | lê texto formatado de um texto |
| [`printf`](#printf-fprintf-sprintf-snprintf) | escreve texto formatado no `stdout` |
| [`fprintf`](#printf-fprintf-sprintf-snprintf) | escreve texto formatado num stream |
| [`sprintf`](#printf-fprintf-sprintf-snprintf) | escreve texto formatado num buffer |
| [`snprintf`](#printf-fprintf-sprintf-snprintf) | escreve texto formatado num buffer, até um tamanho |
| [`ftell`](#ftell) | a posição no arquivo |
| [`fgetpos`](#fgetpos) | guarda a posição no arquivo |
| [`fseek`](#fseek) | vai pra uma posição do arquivo |
| [`fsetpos`](#fsetpos) | volta pra uma posição guardada pelo `fgetpos` |
| [`rewind`](#rewind) | volta pro começo do arquivo |
| [`clearerr`](#clearerr) | limpa os indicadores de erro e de fim de arquivo |
| [`feof`](#feof) | chegou ao fim do arquivo? |
| [`ferror`](#ferror) | deu erro no stream? |
| [`perror`](#perror) | escreve no `stderr` a mensagem do `errno` |
| [`remove`](#remove) | apaga um arquivo |
| [`rename`](#rename) | troca o nome de um arquivo |
| [`tmpfile`](#tmpfile) | cria e abre um arquivo temporário |
| [`tmpnam`](#tmpnam) | gera um nome de arquivo que ainda não existe |

Depois das funções vêm os tipos ([`FILE`](#file) e [`fpos_t`](#fpos_t)), os
streams padrão ([`stdin`, `stdout` e `stderr`](#stdin-stdout-stderr)), as
[constantes](#constantes) e o que a Jinga [deixa de fora](#fica-de-fora).

## Na Jinga

- **O import.** As funções, os tipos (`FILE`, `fpos_t`), as constantes
  (`EOF`, `BUFSIZ`, `SEEK_SET`...) e os streams (`stdin`, `stdout`,
  `stderr`) daqui valem com `import <stdio.h>` no topo do arquivo, igual ao
  `#include <stdio.h>` do C. Sem o import, o nome é `NameError` antes de
  rodar, com a dica do import que falta:

  ```ps
  printf("olá\n")
  # NameError: name 'printf' is not defined. Did you forget to import <stdio.h>?
  ```

  O `NULL` e o `size_t` valem com qualquer cabeçalho que o C diz que os
  define (o `stdio.h` é um deles). As palavras do C (`int *p`, `unsigned`,
  `const`, `sizeof`, o cast) não pedem import. Os exemplos usam ainda o
  `errno` (do `errno.h`), o `malloc` e o `free` (do `stdlib.h`) e o `strlen`
  e o `strerror` (do `string.h`), cada um com o import do seu cabeçalho.
- **O stream é um `FILE *`.** O `fopen` devolve um ponteiro opaco: `type(f)`
  dá `FILE *`, e o ponteiro nulo é o `Null` da Jinga (`if f == NULL`). O
  `FILE` e o `fpos_t` só existem atrás de ponteiro: `FILE f` é erro antes de
  rodar, e a memória de um `fpos_t` vem do `malloc(sizeof(fpos_t))`.
- **O coletor não fecha arquivo.** O stream é do C: fica aberto até o
  `fclose`, mesmo depois que a variável sai de uso. No fim do programa (o fim
  normal, o erro não tratado e o `sys.exit`), o C esvazia os buffers e fecha
  os streams que sobraram.
- **Texto.** O `str` da Jinga entra onde o C só lê o texto (`const char *`):
  o nome do arquivo, o modo, o formato, o texto do `fputs`, do `puts` e do
  `perror`. Onde o C escreve (`char *`: o `fgets`, o `sprintf`, o `snprintf`,
  o `tmpnam`, o `%s` do `scanf`), vai um buffer do C: `char buf[64]` nasce
  zerado, e `str(buf)` traz o texto de volta, até o primeiro byte zero. O
  `str` nesse lugar é erro antes de rodar (`TypeError: fgets() altera o texto
  (char *), e o str da Jinga só entra como const char *: copie pra um
  buffer`).
- **O tamanho do buffer.** O tamanho de um array do C é um número escrito no
  código (`char buf[64]`); `char buf[BUFSIZ]` não vale. A memória do tamanho
  de uma constante vem do `malloc` (`char *buf = malloc(BUFSIZ)`) e volta
  com o `free`.
- **As variádicas** (`printf`, `fprintf`, `sprintf`, `snprintf`, `scanf`,
  `fscanf`, `sscanf`) são guiadas pelo formato: o motor lê o formato e passa
  cada argumento no tipo que a conversão pede. No `printf`, o `int` da Jinga
  vai pro `%d` como o `int` do C (o valor fora da faixa dá a volta, como na
  atribuição do C), o `flo` (e também o `int`) vai pro `%f`, o `%c` recebe o
  código do caractere (`ord("A")`) e o `%s` recebe um `str` ou um buffer. Com
  o formato escrito no código, a conferência sai antes de rodar (no `--check`
  e no editor); com o formato numa variável, sai na chamada. O argumento do
  tipo errado é `TypeError`:

  ```ps
  import <stdio.h>

  printf("%d anos\n", "dez")
  # TypeError: printf: %d espera int, recebeu str
  ```

- **O `scanf` grava pelo ponteiro.** Cada argumento depois do formato é o
  endereço de onde guardar. A variável da Jinga entra com `&`
  (`scanf("%d", &idade)`) e passa a morar na memória do C, com o tipo do C do
  nome dela: o `int` vira o `int` do C (4 bytes), o `flo` vira o `double`. O
  tipo apontado tem que ser o que a conversão pede (o C11 diz que o outro é
  comportamento indefinido; a Jinga dá `TypeError`): `%d` pede `int`, `%u` e
  `%x` pedem `unsigned int` (`unsigned int x = 0`), `%lf` pede `double` (o
  `flo`), e `%f` pede `float`. O `%s`, o `%[` e o `%c` gravam num buffer do
  C, que vai sem o `&`.
- **O `errno`** é guardado logo depois de cada chamada, porque a própria VM
  pode mexer nele antes da linha seguinte: lido depois de uma função daqui, é
  o que ela deixou.
- **A saída sai na ordem.** O `stdin`, o `stdout` e o `stderr` são valores
  `FILE *`, e o `post` da Jinga escreve no mesmo `stdout` do C: o `post` e o
  `printf` saem na ordem em que foram chamados.
- **O comportamento indefinido continua indefinido.** Escrever além do fim do
  buffer no `sprintf`, usar o `FILE *` depois do `fclose` ou passar um modo
  fora da tabela pro `fopen` é indefinido no C, e é o mesmo na Jinga: a
  função do C é chamada direto, sem conferência no meio.

## Acesso a arquivo

### `fopen`

```c
FILE *fopen(const char * restrict filename, const char * restrict mode);
```

Abre o arquivo de nome `filename` e devolve o stream ligado a ele. O `mode`
diz o acesso:

| Modo | O que é | Se o arquivo existe | Se não existe |
|---|---|---|---|
| `"r"` | leitura | lê do começo | falha |
| `"w"` | escrita (cria o arquivo) | apaga o conteúdo | cria |
| `"a"` | acréscimo | escreve no fim | cria |
| `"r+"` | leitura e escrita | lê do começo | falha |
| `"w+"` | leitura e escrita (cria o arquivo) | apaga o conteúdo | cria |
| `"a+"` | leitura e acréscimo | escreve no fim | cria |

- O `b` no modo (`"rb"`, `"wb+"`) abre em modo binário. No Linux e no macOS
  (POSIX) não muda nada; no Windows, desliga o tratamento especial do `'\n'`
  e do `'\x1A'`.
- O `x` (C11), no fim do `"w"` ou do `"w+"` (`"wx"`, `"w+x"`), faz a
  abertura falhar quando o arquivo já existe, em vez de apagar o conteúdo.
- Nos modos de acréscimo, toda escrita vai pro fim do arquivo, seja qual for
  a posição atual.
- No modo de atualização (o `+`), o stream lê e escreve, mas a saída não
  pode vir seguida de entrada sem um `fflush`, `fseek`, `fsetpos` ou `rewind`
  no meio, e a entrada não pode vir seguida de saída sem um `fseek`,
  `fsetpos` ou `rewind` no meio, a não ser que a entrada tenha chegado ao fim
  do arquivo.
- O modo fora dessa tabela é comportamento indefinido.

O formato do nome é do sistema, e ele nem sempre é de um arquivo (pode ser o
console ou outro dispositivo que o sistema mostre como arquivo); onde o
sistema aceita, o nome traz o caminho, absoluto ou relativo.

| Parâmetro | O que é |
|---|---|
| `filename` | o nome do arquivo (um `str` da Jinga entra direto) |
| `mode` | o modo de acesso, da tabela acima |

**Retorno:** o stream do arquivo aberto, com buffer completo, a não ser que o
nome seja de um dispositivo interativo (um terminal); `NULL` (o `Null` da
Jinga) quando falha. O POSIX manda guardar o motivo no `errno` quando falha.

```ps
import <stdio.h>
import <errno.h>
import <string.h>

FILE *f = fopen("notas.txt", "w")      # cria o arquivo
fputs("primeira\n", f)
fclose(f)
f = fopen("notas.txt", "a")            # acrescenta no fim
fputs("segunda\n", f)
fclose(f)

char linha[32]
f = fopen("notas.txt", "r")
while (fgets(linha, sizeof(linha), f) != NULL) {
    printf("leu: %s", linha)
}
fclose(f)

FILE *g = fopen("notas.txt", "wx")     # o x falha: o arquivo existe
if g == NULL {
    printf("wx: %s\n", strerror(errno))
}
g = fopen("nao_existe.txt", "r")
post(g)
printf("r: %s\n", strerror(errno))
remove("notas.txt")
```

```saida
leu: primeira
leu: segunda
wx: File exists
Null
r: No such file or directory
```

### `freopen`

```c
FILE *freopen(const char * restrict filename, const char * restrict mode, FILE * restrict stream);
```

Primeiro tenta fechar o arquivo ligado ao `stream`, ignorando qualquer erro.
Depois, se `filename` não é nulo, tenta abrir esse arquivo com o `mode`, como
o `fopen`, e o liga ao mesmo `stream`. Se `filename` é nulo, tenta abrir de
novo o arquivo que já estava ligado ao `stream`, e quais trocas de modo valem
nesse caso é do sistema. É o jeito de mandar o `stdin`, o `stdout` ou o
`stderr` pra um arquivo.

O `freopen` é também o único jeito de trocar a orientação (estreita ou larga)
de um stream depois que uma operação de entrada e saída a fixou. A biblioteca
do C da Microsoft não aceita troca nenhuma de modo com `filename` nulo, e
trata isso como erro.

| Parâmetro | O que é |
|---|---|
| `filename` | o arquivo a ligar ao stream; `NULL` abre de novo o que já estava ligado |
| `mode` | o modo de acesso, o mesmo do `fopen` |
| `stream` | o stream a mudar |

**Retorno:** o próprio `stream` quando dá certo; `NULL` quando falha.

O exemplo põe o `stdin` pra ler de um arquivo, e o `scanf` passa a ler dele:

```ps
import <stdio.h>

FILE *f = fopen("numeros.txt", "w")
fputs("10 20 30\n", f)
fclose(f)

FILE *r = freopen("numeros.txt", "r", stdin)
post(r == stdin)
int x = 0
int soma = 0
while (scanf("%d", &x) == 1) {
    soma = soma + x
}
printf("soma: %d\n", soma)
remove("numeros.txt")
```

```saida
True
soma: 60
```

### `fclose`

```c
int fclose(FILE *stream);
```

Fecha o stream. O que estava no buffer de saída e ainda não tinha sido
escrito vai pro sistema; o que estava no buffer de entrada e ainda não tinha
sido lido é descartado. Dando certo ou não, o stream deixa de estar ligado ao
arquivo, e o buffer dado pelo `setbuf` ou pelo `setvbuf`, se havia, também
se desliga dele (e é liberado, se foi alocado sozinho). Usar o ponteiro
depois do `fclose` é comportamento indefinido.

Na Jinga, o coletor não fecha o stream: ele fica aberto até o `fclose`, ou
até o fim do programa.

| Parâmetro | O que é |
|---|---|
| `stream` | o stream a fechar |

**Retorno:** `0` quando dá certo; `EOF` quando falha.

O exemplo lê o arquivo por um segundo stream antes e depois do `fclose` do
primeiro:

```ps
import <stdio.h>

FILE *f = fopen("saida.txt", "w")
fputs("guardado no buffer", f)

FILE *leitor = fopen("saida.txt", "r")
char lido[32]
post(fgets(lido, sizeof(lido), leitor) == NULL)    # o arquivo ainda está vazio
post(fclose(f))                                     # o fclose manda o buffer
clearerr(leitor)
fgets(lido, sizeof(lido), leitor)
post(str(lido))
fclose(leitor)
remove("saida.txt")
```

```saida
True
0
guardado no buffer
```

### `fflush`

```c
int fflush(FILE *stream);
```

No stream de saída (e no de atualização cuja última operação foi uma saída),
escreve no dispositivo o que está no buffer e ainda não tinha sido escrito.
No stream de entrada (e no de atualização cuja última operação foi uma
entrada), o comportamento é indefinido. Com `stream` nulo, esvazia todos os
streams de saída abertos, inclusive os que o programa não alcança.

O POSIX define o `fflush` também no stream de entrada ligado a um arquivo ou
a outro dispositivo em que se pode mudar a posição: a posição do arquivo
volta pra posição do stream (o que desfaz a leitura antecipada do buffer), e
os caracteres devolvidos pelo `ungetc` e ainda não lidos são descartados.

| Parâmetro | O que é |
|---|---|
| `stream` | o stream a esvaziar; `NULL` esvazia todos os de saída |

**Retorno:** `0` quando dá certo; `EOF` quando falha, e aí o indicador de
erro do stream fica ligado.

Um uso comum é o `fflush(stdout)` depois de uma pergunta sem quebra de
linha, pra ela aparecer antes de o programa esperar a resposta. O exemplo
esvazia dois arquivos de uma vez:

```ps
import <stdio.h>

FILE *a = fopen("a.txt", "w")
FILE *b = fopen("b.txt", "w")
fputs("um", a)
fputs("dois", b)
post(fflush(NULL))                  # todos os streams de saída

char lido[16]
FILE *ra = fopen("a.txt", "r")
FILE *rb = fopen("b.txt", "r")
fgets(lido, sizeof(lido), ra)
printf("%s ", lido)
fgets(lido, sizeof(lido), rb)
printf("%s\n", lido)
fclose(ra)
fclose(rb)
fclose(a)
fclose(b)
remove("a.txt")
remove("b.txt")
```

```saida
0
um dois
```

### `setbuf`

```c
void setbuf(FILE * restrict stream, char * restrict buf);
```

Dá ao stream o buffer que as operações dele vão usar. O buffer precisa de
pelo menos `BUFSIZ` bytes. Com `buf` não nulo, é o mesmo que
`setvbuf(stream, buf, _IOFBF, BUFSIZ)`, o buffer completo; com `buf` nulo, é
o mesmo que `setvbuf(stream, NULL, _IONBF, 0)`, que desliga o buffer.

Só vale depois que o stream foi ligado a um arquivo aberto e antes de
qualquer outra operação nele (a não ser uma chamada do `setbuf` ou do
`setvbuf` que falhou). O `setbuf` não diz se deu certo; o `setvbuf` diz. O
erro comum é dar ao `stdin` ou ao `stdout` um buffer que deixa de existir
antes do fim do programa.

Na Jinga, o buffer de `BUFSIZ` bytes vem do `malloc` (o tamanho de um array
do C é um número escrito no código), e o `free` só vem depois do `fclose`.

| Parâmetro | O que é |
|---|---|
| `stream` | o stream |
| `buf` | o buffer, com pelo menos `BUFSIZ` bytes; `NULL` desliga o buffer |

**Retorno:** nenhum (`void`).

```ps
import <stdio.h>
import <stdlib.h>

FILE *f = fopen("direto.txt", "w")
setbuf(f, NULL)                    # sem buffer: cada escrita vai direto
fputs("sem buffer", f)
FILE *g = fopen("direto.txt", "r")
char lido[32]
fgets(lido, sizeof(lido), g)
post(str(lido))
fclose(g)
fclose(f)

char *meu = malloc(BUFSIZ)
f = fopen("direto.txt", "w")
setbuf(f, meu)                     # buffer completo, no buffer do programa
fputs("no buffer", f)
g = fopen("direto.txt", "r")
post(fgets(lido, sizeof(lido), g) == NULL)
fclose(f)                          # o fclose antes do free
free(meu)
fclose(g)
remove("direto.txt")
```

```saida
sem buffer
True
```

### `setvbuf`

```c
int setvbuf(FILE * restrict stream, char * restrict buf, int mode, size_t size);
```

Troca o modo de buffer do stream pelo `mode`: `_IOFBF` (buffer completo),
`_IOLBF` (buffer de linha) ou `_IONBF` (sem buffer). Com `buf` nulo, muda o
tamanho do buffer interno pra `size`; com `buf` não nulo, o stream passa a
usar o buffer do programa, de `size` bytes a partir de `buf`. O stream tem
que ser fechado (`fclose`) antes que esse buffer deixe de existir, e o
conteúdo dele depois da chamada é indeterminado: usá-lo é comportamento
indefinido.

Só vale depois que o stream foi ligado a um arquivo aberto e antes de
qualquer outra operação nele (a não ser uma chamada do `setbuf` ou do
`setvbuf` que falhou). Nem todos os `size` bytes são usados sempre: o sistema
costuma arredondar o tamanho pra baixo (um múltiplo de 2, do tamanho da
página...). Em muitos sistemas, o buffer de linha na entrada só vale pro
terminal. O `BUFSIZ` deve ser o tamanho mais eficiente pro sistema, mas o
`fstat` do POSIX costuma dar uma estimativa melhor.

| Parâmetro | O que é |
|---|---|
| `stream` | o stream |
| `buf` | o buffer do programa, ou `NULL` pra só mudar o modo e o tamanho |
| `mode` | o modo de buffer: `_IOFBF`, `_IOLBF` ou `_IONBF` |
| `size` | o tamanho do buffer |

**Retorno:** `0` quando dá certo; diferente de zero quando falha.

Com o buffer de linha, cada quebra de linha esvazia o buffer:

```ps
import <stdio.h>

FILE *f = fopen("linhas.txt", "w")
post(setvbuf(f, NULL, _IOLBF, 0))
fputs("linha inteira\nmeia", f)    # a linha inteira vai; a meia fica
FILE *g = fopen("linhas.txt", "r")
char lido[32]
while (fgets(lido, sizeof(lido), g) != NULL) {
    printf("leu: %s", lido)
}
fclose(g)
fclose(f)
post(setvbuf(stdin, NULL, 99, 0) != 0)     # modo que não existe
remove("linhas.txt")
```

```saida
0
leu: linha inteira
True
```

## Entrada e saída direta

### `fread`

```c
size_t fread(void * restrict ptr, size_t size, size_t nmemb, FILE * restrict stream);
```

Lê até `nmemb` objetos do stream e os guarda no array `ptr`, como se
chamasse o `fgetc` `size` vezes por objeto e guardasse cada byte, na ordem,
nas posições seguidas de `ptr` (lido como array de `unsigned char`). A
posição do arquivo avança o número de bytes lidos. Se dá erro, a posição fica
indeterminada; se um objeto é lido pela metade, o valor dele fica
indeterminado.

Na Jinga, `ptr` é um array do C (`double b[8]`), um ponteiro do `malloc` ou o
endereço de uma variável (`&d`).

| Parâmetro | O que é |
|---|---|
| `ptr` | onde guardar os objetos lidos |
| `size` | o tamanho de cada objeto, em bytes |
| `nmemb` | quantos objetos ler |
| `stream` | o stream de onde ler |

**Retorno:** quantos objetos foram lidos inteiros, que é menos que `nmemb`
quando dá erro ou o arquivo acaba. Com `size` ou `nmemb` zero, devolve zero e
não faz mais nada. O `fread` não diferencia o fim do arquivo do erro: quem
chama confere com o `feof` e o `ferror`.

```ps
import <stdio.h>

double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0}
FILE *f = fopen("dados.bin", "wb")      # o b: arquivo binário
fwrite(a, sizeof(double), 5, f)
fclose(f)

double b[8]
f = fopen("dados.bin", "rb")
size_t lidos = fread(b, sizeof(double), 8, f)    # pede 8, o arquivo tem 5
post(lidos, feof(f) != 0, ferror(f) != 0)
post(b[0], b[1], b[2], b[3], b[4])
fclose(f)
remove("dados.bin")
```

```saida
5 True False
1.0 2.0 3.0 4.0 5.0
```

### `fwrite`

```c
size_t fwrite(const void * restrict ptr, size_t size, size_t nmemb, FILE * restrict stream);
```

Escreve no stream `nmemb` objetos do array `ptr`, como se lesse cada objeto
como um array de `unsigned char` e chamasse o `fputc` `size` vezes por
objeto. A posição do arquivo avança o número de bytes escritos. Se dá erro, a
posição fica indeterminada.

Na Jinga, `ptr` é um array do C, um ponteiro, o endereço de uma variável ou um
`str` (que o C só lê: os bytes do texto em UTF-8).

| Parâmetro | O que é |
|---|---|
| `ptr` | o primeiro objeto a escrever |
| `size` | o tamanho de cada objeto, em bytes |
| `nmemb` | quantos objetos escrever |
| `stream` | o stream onde escrever |

**Retorno:** quantos objetos foram escritos inteiros, que só é menos que
`nmemb` quando dá erro. Com `size` ou `nmemb` zero, devolve zero e não faz
mais nada.

```ps
import <stdio.h>

int v[3] = {7, 8, 9}
FILE *f = fopen("ints.bin", "wb")
size_t n = fwrite(v, sizeof(int), 3, f)
printf("escreveu %zu de 3, %ld bytes\n", n, ftell(f))
fwrite("fim", 1, 3, f)
fclose(f)

int w[3]
char resto[4]
f = fopen("ints.bin", "rb")
fread(w, sizeof(int), 3, f)
fread(resto, 1, 3, f)
post(w[0], w[1], w[2], str(resto))
fclose(f)
remove("ints.bin")
```

```saida
escreveu 3 de 3, 12 bytes
7 8 9 fim
```

## Entrada e saída sem formato

### `fgetc`, `getc`

```c
int fgetc(FILE *stream);
int getc(FILE *stream);
```

Leem o próximo caractere do stream. O `getc` é o mesmo que o `fgetc`, mas
pode ser uma macro que avalia o `stream` mais de uma vez; no C, o argumento
dele não deve ter efeito colateral. Na Jinga, os dois são a função do C.

O caractere volta como `int`, e o `EOF` (`-1`) fica fora da faixa de todo
`unsigned char`: a variável que recebe o caractere é `int`, pro `EOF` não se
confundir com um byte.

| Parâmetro | O que é |
|---|---|
| `stream` | o stream de onde ler |

**Retorno:** o caractere lido, como `unsigned char` convertido pra `int`;
`EOF` quando falha. Se a falha é o fim do arquivo, liga o indicador de fim
do stream (`feof`); se é outro erro, liga o indicador de erro (`ferror`).

```ps
import <stdio.h>

FILE *f = fopen("ola.txt", "w+")
fputs("Olá, mundo!\n", f)
rewind(f)
int n = 0
int c = fgetc(f)
while (c != EOF) {
    putchar(c)
    n++
    c = getc(f)
}
if ferror(f) != 0 {
    puts("erro de leitura")
}
elif feof(f) != 0 {
    printf("fim do arquivo depois de %d bytes\n", n)
}
fclose(f)
remove("ola.txt")
```

O `á` ocupa dois bytes em UTF-8: são 12 caracteres e 13 bytes.

```saida
Olá, mundo!
fim do arquivo depois de 13 bytes
```

### `fgets`

```c
char *fgets(char * restrict s, int n, FILE * restrict stream);
```

Lê do stream no máximo `n - 1` caracteres e os guarda no array `s`. Para na
quebra de linha (que fica em `s`) ou no fim do arquivo. Se leu algum byte
sem erro, escreve o byte zero logo depois do último caractere.

O padrão não deixa claro o que acontece com `n` menor que 2. Com `n` menor
que 1, as bibliotecas não fazem nada e dão erro; com `n` igual a 1, umas
fazem o mesmo, e outras não leem nada, guardam o zero em `s[0]` e dão certo.

Na Jinga, `s` é um buffer do C (`char linha[80]`), e `str(linha)` traz o
texto de volta. O `str` da Jinga não entra: o `fgets` escreve em `s`.

| Parâmetro | O que é |
|---|---|
| `s` | o buffer onde guardar a linha |
| `n` | o tamanho do buffer: lê até `n - 1` caracteres e deixa lugar pro zero |
| `stream` | o stream de onde ler |

**Retorno:** `s` quando dá certo; `NULL` quando falha. No fim do arquivo,
liga o indicador de fim do stream; só é falha se nenhum byte foi lido, e aí
`s` fica como estava. Se a falha é outro erro, liga o indicador de erro e o
conteúdo de `s` fica indeterminado (pode nem terminar em zero). O POSIX manda
guardar o motivo no `errno` no erro de leitura.

Com um buffer de 8 bytes, cada chamada lê até 7 caracteres, e a linha mais
comprida sai em pedaços:

```ps
import <stdio.h>

FILE *tmpf = tmpfile()
fputs("Alan Turing\n", tmpf)
fputs("John von Neumann\n", tmpf)
fputs("Alonzo Church\n", tmpf)
rewind(tmpf)

char buf[8]
while (fgets(buf, sizeof(buf), tmpf) != NULL) {
    printf("\"%s\"\n", buf)
}
if feof(tmpf) != 0 {
    puts("Fim do arquivo")
}
fclose(tmpf)
```

```saida
"Alan Tu"
"ring
"
"John vo"
"n Neuma"
"nn
"
"Alonzo "
"Church
"
Fim do arquivo
```

### `fputc`, `putc`

```c
int fputc(int c, FILE *stream);
int putc(int c, FILE *stream);
```

Escrevem o caractere `c` no stream. O caractere vira `unsigned char` logo
antes de ser escrito. O `putc` pode ser uma macro que avalia o `stream` mais
de uma vez; no C, o argumento dele não deve ter efeito colateral. Na Jinga,
os dois são a função do C.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a escrever: o código (`ord("a")`), não o `str` |
| `stream` | o stream onde escrever |

**Retorno:** o caractere escrito quando dá certo; `EOF` quando falha, e aí
liga o indicador de erro do stream.

```ps
import <stdio.h>

int r = 0
int c = ord("a")
while (r != EOF and c < ord("z")) {
    r = putc(c, stdout)
    c++
}
if r == EOF and ferror(stdout) != 0 {
    perror("putc()")
}
putc(ord("\n"), stdout)

FILE *f = fopen("letra.txt", "w+")
post(fputc(ord("J"), f))
rewind(f)
post(chr(fgetc(f)))
fclose(f)
remove("letra.txt")
```

```saida
abcdefghijklmnopqrstuvwxy
74
J
```

### `fputs`

```c
int fputs(const char * restrict s, FILE * restrict stream);
```

Escreve no stream cada caractere do texto `s`, como se chamasse o `fputc`
pra cada um. O byte zero do fim não é escrito. Ao contrário do `puts`, não
acrescenta a quebra de linha.

| Parâmetro | O que é |
|---|---|
| `s` | o texto a escrever (um `str` da Jinga entra direto) |
| `stream` | o stream onde escrever |

**Retorno:** um número não negativo quando dá certo; `EOF` quando falha, e aí
liga o indicador de erro do stream. Qual número não negativo sai depende da
biblioteca (umas devolvem o último caractere escrito, outras quantos foram
escritos, outras zero): o certo é comparar com `EOF`. O POSIX manda guardar o
motivo no `errno` quando falha.

```ps
import <stdio.h>

int rc = fputs("Olá", stdout)
fputs(", mundo", stdout)
fputs("\n", stdout)
if rc == EOF {
    perror("fputs()")
}
post(rc != EOF)
```

```saida
Olá, mundo
True
```

### `getchar`

```c
int getchar(void);
```

Lê o próximo caractere do `stdin`. É o mesmo que `getc(stdin)`.

Sem parâmetros.

**Retorno:** o caractere lido; `EOF` quando falha. Se a falha é o fim do
arquivo, liga o indicador de fim do `stdin`; se é outro erro, liga o
indicador de erro do `stdin`.

```ps
import <stdio.h>

int linhas = 0
int bytes = 0
int c = getchar()
while (c != EOF) {
    bytes++
    if c == ord("\n") {
        linhas++
    }
    c = getchar()
}
printf("%d linhas, %d bytes\n", linhas, bytes)
if feof(stdin) != 0 {
    puts("fim da entrada")
}
```

Com a entrada:

```entrada
abc
de
```

```saida
2 linhas, 7 bytes
fim da entrada
```

### `putchar`

```c
int putchar(int c);
```

Escreve o caractere `c` no `stdout`. O caractere vira `unsigned char` logo
antes de ser escrito. É o mesmo que `putc(c, stdout)`.

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a escrever (o código, `ord("a")`) |

**Retorno:** o caractere escrito quando dá certo; `EOF` quando falha, e aí
liga o indicador de erro do `stdout`.

O retorno é o caractere já convertido, que pode ser outro número:

```ps
import <stdio.h>

int c = ord("a")
while (c < ord("z")) {
    putchar(c)
    c++
}
putchar(ord("\n"))
int r = putchar(0x1070)       # só o byte de baixo, 0x70 (o p), é escrito
printf("\n0x%x\n", r)
```

```saida
abcdefghijklmnopqrstuvwxy
p
0x70
```

### `puts`

```c
int puts(const char *s);
```

Escreve no `stdout` cada caractere do texto `s` e mais uma quebra de linha
(`'\n'`), como se chamasse o `fputc` pra cada um. O byte zero do fim não é
escrito. Ao contrário do `fputs`, acrescenta a quebra de linha.

| Parâmetro | O que é |
|---|---|
| `s` | o texto a escrever (um `str` da Jinga entra direto) |

**Retorno:** um número não negativo quando dá certo; `EOF` quando falha, e aí
liga o indicador de erro do `stdout`. Qual número não negativo sai depende da
biblioteca: o certo é comparar com `EOF`. A falha comum é o disco cheio com o
`stdout` mandado pra um arquivo; o POSIX manda guardar o motivo no `errno`.

```ps
import <stdio.h>

int rc = puts("Olá, Jinga")
if rc == EOF {
    perror("puts()")
}
puts("")
puts("o puts põe a quebra de linha; o fputs, não")
```

```saida
Olá, Jinga

o puts põe a quebra de linha; o fputs, não
```

### `ungetc`

```c
int ungetc(int c, FILE *stream);
```

Se `c` não é `EOF`, devolve o caractere `c` (como `unsigned char`) ao buffer
de entrada do stream, e a próxima leitura do stream o lê. O arquivo não muda.
O `fseek`, o `fsetpos` e o `rewind` desfazem o `ungetc`.

- Só um caractere devolvido é garantido: o `ungetc` chamado de novo, sem uma
  leitura ou uma mudança de posição no meio, pode falhar. Quando mais de um
  dá certo, a leitura os devolve na ordem inversa das chamadas.
- Com `c` igual a `EOF`, falha e não mexe no stream.
- Dando certo, desliga o indicador de fim do stream.
- No stream binário, a posição volta um byte (com a posição zero, fica
  indeterminada). No stream de texto, a posição muda de um jeito não
  especificado, mas volta ao valor de antes do `ungetc` quando todos os
  caracteres devolvidos forem lidos.

Na prática, o tamanho dessa devolução varia: de 4 KiB (Linux, macOS) a 4
bytes (Solaris) ou só o 1 garantido (HP-UX, AIX).

| Parâmetro | O que é |
|---|---|
| `c` | o caractere a devolver (o código) |
| `stream` | o stream |

**Retorno:** `c` quando dá certo; `EOF` quando falha, e aí o stream fica como
estava.

Foi pra isso que o `ungetc` nasceu: o `scanf` lê um caractere a mais pra
saber onde o número acaba, e devolve esse caractere ao stream.

```ps
import <stdio.h>

FILE *f = fopen("entrada.txt", "w+")
fputs("123x", f)
rewind(f)
int num = 0
int c = getc(f)
while (c >= ord("0") and c <= ord("9")) {
    num = num * 10 + c - ord("0")
    c = getc(f)
}
post(ungetc(c, f) == c)        # devolve o x, que não é dígito
printf("o número %d, depois o '%c'\n", num, getc(f))
post(ungetc(EOF, f) == EOF)
fclose(f)
remove("entrada.txt")
```

```saida
True
o número 123, depois o 'x'
True
```

## Entrada formatada

### `scanf`, `fscanf`, `sscanf`

```c
int scanf(const char * restrict format, ...);
int fscanf(FILE * restrict stream, const char * restrict format, ...);
int sscanf(const char * restrict s, const char * restrict format, ...);
```

Leem texto, interpretam pelo `format` e guardam os valores nos endereços
dados. O `scanf` lê do `stdin`, o `fscanf` lê do `stream`, e o `sscanf` lê do
texto `s`; pro `sscanf`, chegar ao fim do texto é o mesmo que chegar ao fim
do arquivo pro `fscanf`.

O formato tem três tipos de coisa:

- caractere que não é branco nem `%`: lê exatamente um caractere igual, ou
  a leitura falha ali;
- branco (espaço, `'\n'`, `'\t'`...): lê todos os brancos seguidos da
  entrada (`" "`, `"\n"` e `"\t\t"` fazem o mesmo);
- a conversão, que começa no `%`: `%`, o `*` opcional (lê e converte, mas
  não guarda, e não conta no retorno), a largura opcional (o máximo de
  caracteres a ler, maior que zero), o modificador de tamanho opcional e a
  letra da conversão.

| Conversão | Lê | Grava em (na Jinga) |
|---|---|---|
| `%d` | um inteiro decimal com sinal (o que o `strtol` lê na base 10) | `&x` de `int` |
| `%i` | um inteiro na base que o começo diz (`0x` hexadecimal, `0` octal; o `strtol` na base 0) | `&x` de `int` |
| `%u` | um inteiro decimal sem sinal (o `strtoul` na base 10) | `&x` de `unsigned int` |
| `%o` | um inteiro octal sem sinal (o `strtoul` na base 8) | `&x` de `unsigned int` |
| `%x`, `%X` | um inteiro hexadecimal sem sinal (o `strtoul` na base 16) | `&x` de `unsigned int` |
| `%f`, `%e`, `%g`, `%a` (e `%F`, `%E`, `%G`, `%A`) | um número de ponto flutuante (o que o `strtof` lê) | `&x` de `float`; com `l` (`%lf`), de `flo` ou `double` |
| `%s` | uma sequência sem brancos; guarda o zero no fim | buffer `char` com lugar pra largura + 1 |
| `%c` | exatamente a largura em caracteres (1 sem largura); não pula brancos e não guarda o zero | buffer `char` |
| `%[conjunto]` | a maior sequência de caracteres do conjunto (`%[^...]`: fora do conjunto); guarda o zero | buffer `char` com lugar pra largura + 1 |
| `%n` | não lê nada: guarda quantos caracteres foram lidos até ali (não conta no retorno) | `&x` de `int` |
| `%p` | um endereço, no formato que o `%p` do `printf` escreve | `&p` de `void *` |
| `%%` | um `%` | nenhum |

- Toda conversão, menos o `%[`, o `%c` e o `%n`, pula os brancos do começo
  antes de ler (e eles não contam na largura). Por isso dois `scanf("%d")`
  seguidos leem dois números na mesma linha ou em linhas diferentes. O
  `%c` não pula: `scanf(" %c", letra)`, com o espaço, pula os brancos antes.
- Cada conversão lê a maior sequência que cabe na largura e que é (ou começa)
  um valor válido; o primeiro caractere depois dela fica sem ler. Sequência
  vazia ou que não converte é uma falha de casamento; o fim do arquivo, um
  erro de codificação ou de leitura antes de ler é uma falha de entrada.
- O valor que não cabe no tipo do destino é comportamento indefinido no C.
- Pelo padrão, o `%f` em `"100er"` consome o `"100e"` e falha; o Linux
  (glibc) e o macOS consomem só o `"100"` e deixam o `"er"`.
- O `sscanf` de algumas bibliotecas mede o texto inteiro a cada chamada: o
  tempo cresce com o tamanho do texto, não só com o que é lido.
- No `%[`, o `]` logo no começo (`%[]...]` ou `%[^]...]`) entra no conjunto;
  o `-` fora do começo (`%[0-9]`) é uma faixa no Linux e no macOS, mas o
  padrão deixa isso pro sistema.
- O `%s` e o `%[` sem largura são tão perigosos quanto o `gets`: a largura
  (`%31s` num buffer de 32) é o que impede de passar do fim do buffer.
- Os modificadores de tamanho são os mesmos do `printf`: `hh` (`char`), `h`
  (`short`), `l` (`long` e `double`), `ll` (`long long`), `j` (`intmax_t`),
  `z` (`size_t`), `t` (`ptrdiff_t`) e `L` (`long double`).

| Parâmetro | O que é |
|---|---|
| `stream` | o stream de onde ler (`fscanf`) |
| `s` | o texto de onde ler (`sscanf`; um `str` da Jinga entra direto) |
| `format` | o formato |
| `...` | os endereços onde guardar, um por conversão que grava |

**Retorno:** quantos argumentos foram gravados (zero, se uma falha de
casamento veio antes do primeiro); `EOF` se uma falha de entrada veio antes
do primeiro.

`scanf` com o `&` de uma variável da Jinga:

```ps
import <stdio.h>

int idade = 0
printf("idade: ")
if scanf("%d", &idade) == 1 {
    printf("ano que vem: %d\n", idade + 1)
}
```

Com a entrada:

```entrada
41
```

```saida
idade: ano que vem: 42
```

`fscanf` lendo um arquivo até o fim, com o `%lf` pro `flo`:

```ps
import <stdio.h>

FILE *f = fopen("notas.txt", "w")
fputs("Ana 8.5\nBeto 7.0\nCaio 9.25\n", f)
fclose(f)

f = fopen("notas.txt", "r")
char nome[32]
flo nota = 0.0
while (fscanf(f, "%31s %lf", nome, &nota) == 2) {
    printf("%-5s %5.2f\n", nome, nota)
}
post(fscanf(f, "%31s", nome) == EOF)
fclose(f)
remove("notas.txt")
```

```saida
Ana    8.50
Beto   7.00
Caio   9.25
True
```

`sscanf` com largura, `*`, conjunto e branco no formato:

```ps
import <stdio.h>

int i = 0
int j = 0
float x = 0.0
float y = 0.0
char str1[10]
char str2[4]
str entrada = "25 54.32E-1 Thompson 56789 0123 56"
# %d inteiro; %f float; %9s até 9 caracteres sem branco; %2d dois dígitos
# (5 e 6); %f os dígitos 7, 8 e 9; %*d lê um inteiro e não guarda; o espaço
# pula os brancos; %3[0-9] até 3 dígitos (5 e 6)
int ret = sscanf(entrada, "%d%f%9s%2d%f%*d %3[0-9]", &i, &x, str1, &j, &y, str2)
printf("%d campos\n", ret)
printf("i = %d, x = %f, str1 = %s\n", i, x, str1)
printf("j = %d, y = %f, str2 = %s\n", j, y, str2)
post(sscanf("abc", "%d", &i), sscanf("", "%d", &i))
```

```saida
6 campos
i = 25, x = 5.432000, str1 = Thompson
j = 56, y = 789.000000, str2 = 56
0 -1
```

## Saída formatada

### `printf`, `fprintf`, `sprintf`, `snprintf`

```c
int printf(const char * restrict format, ...);
int fprintf(FILE * restrict stream, const char * restrict format, ...);
int sprintf(char * restrict s, const char * restrict format, ...);
int snprintf(char * restrict s, size_t n, const char * restrict format, ...);
```

Convertem os argumentos em texto pelo `format` e escrevem o resultado: o
`printf` no `stdout`, o `fprintf` no `stream`, o `sprintf` e o `snprintf` no
buffer `s`. O `sprintf` não sabe o tamanho de `s`: o texto mais o zero do fim
passar do tamanho do buffer é comportamento indefinido. O `snprintf` escreve
no máximo `n - 1` caracteres e o zero do fim; com `n` zero, não escreve nada
(e `s` pode ser `NULL`), mas devolve o tamanho do mesmo jeito.

O formato é texto que sai como está, e conversões que começam no `%`. Cada
conversão tem, nesta ordem: `%`, os sinais opcionais, a largura mínima
opcional, a precisão opcional, o modificador de tamanho opcional e a letra da
conversão.

| Sinal | O que faz |
|---|---|
| `-` | alinha à esquerda no campo (o padrão é à direita) |
| `+` | põe o sinal também no número não negativo (nas conversões com sinal) |
| espaço | põe um espaço antes do número não negativo, quando não há o `+` |
| `#` | a forma alternativa: `0x` (ou `0X`) no `%x` com número diferente de zero, um `0` na frente no `%o`, e o ponto decimal sempre nas conversões de ponto flutuante |
| `0` | completa com zeros à esquerda em vez de espaços, nas conversões de número; com o `-`, não vale, e nos inteiros com precisão também não |

- **Largura**: um número dá o tamanho mínimo do campo, completado com
  espaços. O `*` tira a largura de um argumento `int` a mais, antes do valor;
  a largura negativa vale como o `-` com o número positivo.
- **Precisão**: `.` e um número. No `%s`, é o máximo de caracteres; nos
  inteiros, o mínimo de dígitos (o padrão é 1, e com precisão zero o zero não
  escreve nada); no `%f`, no `%e` e no `%a`, os dígitos depois do ponto; no
  `%g`, os dígitos significativos. Só o `.` vale zero. O `.*` tira a precisão
  de um argumento `int` a mais, depois do da largura; a precisão negativa é
  ignorada.
- **Tamanho**: `hh` (`char`), `h` (`short`), `l` (`long`), `ll`
  (`long long`), `j` (`intmax_t`), `z` (`size_t`), `t` (`ptrdiff_t`) e `L`
  (`long double`). Na Jinga, o modificador diz em que tipo do C o número vai:
  `printf("%hhd", 300)` escreve `44`.

| Conversão | Escreve | Argumento (na Jinga) |
|---|---|---|
| `%d`, `%i` | o inteiro com sinal, em decimal | `int` |
| `%u` | o inteiro sem sinal, em decimal | `int` |
| `%o` | o inteiro sem sinal, em octal | `int` |
| `%x`, `%X` | o inteiro sem sinal, em hexadecimal (`abcdef` ou `ABCDEF`) | `int` |
| `%c` | um caractere (o `int` vira `unsigned char`) | `int`: o código, `ord("A")` |
| `%s` | o texto até o byte zero, ou até a precisão | `str` ou buffer do C |
| `%f`, `%F` | o número em decimal, `[-]ddd.ddd`; a precisão padrão é 6 | `flo` ou `int` |
| `%e`, `%E` | o número em notação científica, `[-]d.ddde±dd`, com pelo menos dois dígitos no expoente; a precisão padrão é 6 | `flo` ou `int` |
| `%g`, `%G` | o `%e` ou o `%f`: com a precisão P (6 sem precisão, 1 com precisão zero) e o expoente X do `%e`, sai o `%f` com precisão P - 1 - X se P > X ≥ -4, e o `%e` com precisão P - 1 se não; os zeros do fim e o ponto que sobra saem, a não ser com o `#` | `flo` ou `int` |
| `%a`, `%A` | o número em hexadecimal com expoente, `[-]0xh.hhhp±d` | `flo` ou `int` |
| `%p` | o endereço de um ponteiro, no formato do sistema | ponteiro |
| `%n` | nada: guarda quantos caracteres saíram até ali | `&x` de `int` |
| `%%` | o `%` | nenhum |

- As letras maiúsculas (`%X`, `%E`, `%G`, `%A`, `%F`) escrevem em maiúsculas.
  O infinito sai `inf` ou `infinity`, e o NaN sai `nan` ou `nan(...)`, cada
  sistema com o seu.
- O `%n` é alvo conhecido de ataque. No macOS, ele derruba o programa (o
  sinal `SIGABRT`, código 134): a biblioteca do C da Apple recusa o `%n` num
  formato guardado em memória que se pode escrever, e o formato que a Jinga
  passa sempre está nela.
- A conversão desconhecida (`%k`) é erro antes de rodar. O `%lc` e o `%ls`,
  do texto largo, são do `wchar.h`, que ainda não está na Jinga (é a segunda
  parte da biblioteca do C).
- Os inteiros de largura fixa usam as macros do `inttypes.h` (`PRId64`...).
- Os argumentos são conferidos: o tipo errado e a quantidade que não bate com
  o formato são erro (`TypeError`); o argumento a mais também, embora o C o
  avalie e ignore.

| Parâmetro | O que é |
|---|---|
| `stream` | o stream onde escrever (`fprintf`) |
| `s` | o buffer onde escrever (`sprintf`, `snprintf`) |
| `n` | o tamanho do buffer: escreve até `n - 1` caracteres e o zero (`snprintf`) |
| `format` | o formato |
| `...` | os valores a escrever, um por conversão (dois com o `*`) |

**Retorno:** o `printf` e o `fprintf` devolvem quantos caracteres (bytes)
saíram, ou um número negativo quando dá erro de saída ou de codificação. O
`sprintf` devolve quantos caracteres escreveu em `s`, sem contar o zero do
fim, ou um negativo no erro de codificação. O `snprintf` devolve quantos
caracteres escreveria (sem o zero do fim) se `n` fosse grande o bastante, ou
um negativo no erro de codificação. O POSIX manda guardar o motivo no `errno`
quando dá erro.

`printf` com largura, precisão, sinais e as notações:

```ps
import <stdio.h>

printf("Arredondamento: %f %.0f %.32f\n", 1.5, 1.5, 1.3)
printf("Preenchimento: %05.2f %.2f %5.2f\n", 1.5, 1.5, 1.5)
printf("Científica: %E %e\n", 1.5, 1.5)
printf("Hexadecimal: %a %A\n", 1.5, 1.5)
printf("[%5d] [%-5d] [%+d] [%x] [%#o] [%c] [%.3s] [%*d]\n", 42, 42, 42, 255, 8, 74, "abcdef", 4, 7)
int n = printf("olá\n")
post(n)
```

O `olá` tem 3 caracteres e 4 bytes: com a quebra de linha, o `printf`
devolve 5.

```saida
Arredondamento: 1.500000 2 1.30000000000000004440892098500626
Preenchimento: 01.50 1.50  1.50
Científica: 1.500000E+00 1.500000e+00
Hexadecimal: 0x1.8p+0 0X1.8P+0
[   42] [42   ] [+42] [ff] [010] [J] [abc] [   7]
olá
5
```

`fprintf` num arquivo e no `stderr`:

```ps
import <stdio.h>

FILE *f = fopen("relatorio.txt", "w")
int n = fprintf(f, "%s=%.1f\n", "media", 8.25)
fclose(f)
post(n)
char linha[32]
f = fopen("relatorio.txt", "r")
fgets(linha, sizeof(linha), f)
printf("no arquivo: %s", linha)
fclose(f)
fprintf(stderr, "aviso: %d linha escrita\n", 1)
remove("relatorio.txt")
```

O `8.25` sai `8.2`: o número está exatamente no meio, e o arredondamento do C
vai pro par.

```saida
10
no arquivo: media=8.2
```

```erro
aviso: 1 linha escrita
```

`sprintf` e `snprintf` num buffer, com o tamanho perguntado antes:

```ps
import <stdio.h>
import <stdlib.h>

char buf[32]
int n = sprintf(buf, "%d + %d = %d", 2, 3, 2 + 3)
post(n, str(buf))

char peq[8]
n = snprintf(peq, sizeof(peq), "%s", "abcdefghij")    # cabem 7 e o zero
post(n, str(peq))

int tam = snprintf(NULL, 0, "raiz de 2 = %.4f", 1.41421356)
char *exato = malloc(tam + 1)                           # +1 pro zero do fim
snprintf(exato, tam + 1, "raiz de 2 = %.4f", 1.41421356)
post(tam, str(exato))
free(exato)
```

```saida
9 2 + 3 = 5
10 abcdefg
18 raiz de 2 = 1.4142
```

## Posição no arquivo

### `ftell`

```c
long int ftell(FILE *stream);
```

Devolve a posição atual do stream. No stream binário, é o número de bytes
desde o começo do arquivo. No stream de texto, o valor não é especificado e
só serve pra passar ao `fseek`; no Linux e no macOS, que não diferenciam
texto de binário, é sempre o número de bytes.

| Parâmetro | O que é |
|---|---|
| `stream` | o stream |

**Retorno:** a posição quando dá certo; `-1` quando falha, e aí guarda no
`errno` um valor positivo que é do sistema.

```ps
import <stdio.h>

double a[5] = {1.1, 2.0, 3.0, 4.0, 5.0}
FILE *f = fopen("teste.bin", "wb")
fwrite(a, sizeof(double), 5, f)
fclose(f)

double b[1]
f = fopen("teste.bin", "rb")
printf("posição: %ld\n", ftell(f))
fread(b, sizeof(double), 1, f)
printf("posição: %ld\n", ftell(f))
printf("b[0]: %.1f\n", b[0])
fclose(f)
remove("teste.bin")
```

```saida
posição: 0
posição: 8
b[0]: 1.1
```

### `fgetpos`

```c
int fgetpos(FILE * restrict stream, fpos_t * restrict pos);
```

Guarda em `*pos` a posição do stream e o estado da leitura de caracteres
multibyte, se há. O valor guardado só serve pra passar ao `fsetpos`.

Na Jinga, o `fpos_t` só existe atrás de ponteiro: a memória dele vem do
`malloc(sizeof(fpos_t))` e volta com o `free`.

| Parâmetro | O que é |
|---|---|
| `stream` | o stream |
| `pos` | onde guardar a posição |

**Retorno:** `0` quando dá certo; diferente de zero quando falha. O C11 manda
guardar no `errno` um valor positivo que é do sistema quando falha.

```ps
import <stdio.h>
import <stdlib.h>

double v[4] = {1.1, 2.2, 3.3, 4.4}
FILE *fp = fopen("teste.bin", "wb")
fwrite(v, sizeof(double), 4, fp)
fclose(fp)

fp = fopen("teste.bin", "rb")
fpos_t *pos = malloc(sizeof(fpos_t))
fgetpos(fp, pos)                    # guarda o começo do arquivo
double d = 0.0
fread(&d, sizeof(d), 1, fp)         # lê o primeiro double
printf("primeiro valor: %.1f\n", d)
fsetpos(fp, pos)                    # volta pro começo
fread(&d, sizeof(d), 1, fp)         # lê o primeiro de novo
printf("de novo: %.1f\n", d)
free(pos)
fclose(fp)
remove("teste.bin")
```

```saida
primeiro valor: 1.1
de novo: 1.1
```

### `fseek`

```c
int fseek(FILE *stream, long int offset, int whence);
```

Muda a posição do stream. No stream binário, a posição nova fica a `offset`
bytes do começo do arquivo (`whence` igual a `SEEK_SET`), da posição atual
(`SEEK_CUR`) ou do fim do arquivo (`SEEK_END`); o stream binário não precisa
aceitar o `SEEK_END`. No stream de texto, os únicos `offset` que valem são o
zero (com qualquer `whence`) e um valor que o `ftell` devolveu antes, no
mesmo arquivo (só com `SEEK_SET`). No stream de texto largo, valem as
restrições dos dois.

Além de mudar a posição, desfaz o `ungetc` e desliga o indicador de fim do
arquivo. Se dá erro de leitura ou de escrita, liga o indicador de erro e a
posição não muda.

O POSIX deixa a posição ir além do fim do arquivo: a escrita depois disso
deixa um buraco, e a leitura do buraco devolve bytes zero. O POSIX manda
também esvaziar o buffer de saída antes de mudar a posição, e devolver `-1`
com o motivo no `errno` quando falha. Num stream de texto largo, a escrita
depois de ir pra uma posição que não é o fim pode estragar o resto do
arquivo.

| Parâmetro | O que é |
|---|---|
| `stream` | o stream |
| `offset` | quantos bytes andar, a partir de `whence` |
| `whence` | de onde contar: `SEEK_SET`, `SEEK_CUR` ou `SEEK_END` |

**Retorno:** `0` quando dá certo; diferente de zero quando falha.

```ps
import <stdio.h>

double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0}
FILE *fp = fopen("teste.bin", "wb")
fwrite(a, sizeof(double), 5, fp)
fclose(fp)

double b[1]
fp = fopen("teste.bin", "rb")
if fseek(fp, sizeof(double) * 2, SEEK_SET) != 0 {     # na frente do terceiro
    perror("fseek()")
}
int lidos = fread(b, sizeof(double), 1, fp)
printf("lidos: %d, b[0] = %.1f\n", lidos, b[0])
fseek(fp, -8, SEEK_END)                                # o último
fread(b, sizeof(double), 1, fp)
printf("o último: %.1f\n", b[0])
fseek(fp, -24, SEEK_CUR)                               # três pra trás
fread(b, sizeof(double), 1, fp)
printf("três antes do fim: %.1f\n", b[0])
fclose(fp)
remove("teste.bin")
```

```saida
lidos: 1, b[0] = 3.0
o último: 5.0
três antes do fim: 3.0
```

### `fsetpos`

```c
int fsetpos(FILE *stream, const fpos_t *pos);
```

Muda a posição do stream e o estado da leitura multibyte, se há, pro valor
de `*pos`, guardado antes pelo `fgetpos` num stream do mesmo arquivo. Além
disso, desfaz o `ungetc` e desliga o indicador de fim do arquivo. Se dá erro
de leitura ou de escrita, liga o indicador de erro.

Num stream de texto largo, a escrita depois de voltar pra uma posição que
não é o fim pode estragar o resto do arquivo (uma sequência multibyte de
outro tamanho, por exemplo).

| Parâmetro | O que é |
|---|---|
| `stream` | o stream |
| `pos` | a posição guardada pelo `fgetpos` |

**Retorno:** `0` quando dá certo; diferente de zero quando falha. O C11 manda
guardar no `errno` um valor positivo que é do sistema quando falha.

```ps
import <stdio.h>
import <stdlib.h>

FILE *f = fopen("texto.txt", "w+")
fputs("abc", f)
rewind(f)
fpos_t *inicio = malloc(sizeof(fpos_t))
fgetpos(f, inicio)
while (fgetc(f) != EOF) {
}
post(feof(f) != 0)
post(fsetpos(f, inicio), feof(f) != 0)    # volta e desliga o fim do arquivo
post(chr(fgetc(f)))
free(inicio)
fclose(f)
remove("texto.txt")
```

```saida
True
0 False
a
```

### `rewind`

```c
void rewind(FILE *stream);
```

Volta a posição do stream pro começo do arquivo. É o mesmo que
`fseek(stream, 0, SEEK_SET)`, mas desliga também os indicadores de fim do
arquivo e de erro, e desfaz o `ungetc`.

| Parâmetro | O que é |
|---|---|
| `stream` | o stream |

**Retorno:** nenhum (`void`).

```ps
import <stdio.h>

FILE *f = fopen("arquivo.txt", "w")
int ch = ord("0")
while (ch <= ord("9")) {
    fputc(ch, f)
    ch++
}
fclose(f)

char texto[20]
f = fopen("arquivo.txt", "r")
fread(texto, 1, 10, f)
puts(texto)
rewind(f)
fread(texto, 1, 10, f)
puts(texto)
post(fgetc(f) == EOF, feof(f) != 0)
rewind(f)
post(feof(f) != 0)
fclose(f)
remove("arquivo.txt")
```

```saida
0123456789
0123456789
True True
False
```

## Erros

### `clearerr`

```c
void clearerr(FILE *stream);
```

Desliga os indicadores de erro e de fim do arquivo do stream.

| Parâmetro | O que é |
|---|---|
| `stream` | o stream |

**Retorno:** nenhum (`void`).

```ps
import <stdio.h>

FILE *tmpf = tmpfile()
fputs("jinga\n", tmpf)
rewind(tmpf)
int ch = fgetc(tmpf)
while (ch != EOF) {
    putchar(ch)
    ch = fgetc(tmpf)
}
if feof(tmpf) != 0 {
    puts("chegou ao fim do arquivo")
}
clearerr(tmpf)
if feof(tmpf) != 0 {
    puts("o indicador de fim continua ligado")
}
else {
    puts("o indicador de fim foi desligado")
}
fclose(tmpf)
```

```saida
jinga
chegou ao fim do arquivo
o indicador de fim foi desligado
```

### `feof`

```c
int feof(FILE *stream);
```

Confere se o stream chegou ao fim do arquivo. Diz o estado que a última
operação de entrada e saída deixou, sem olhar o arquivo: depois do `fgetc`
que leu o último byte, o `feof` ainda dá zero; o `fgetc` seguinte falha e
liga o fim do arquivo, e só aí o `feof` dá diferente de zero. O uso de
sempre é parar a leitura em qualquer falha e depois diferenciar o fim do
arquivo (`feof`) do erro (`ferror`).

| Parâmetro | O que é |
|---|---|
| `stream` | o stream |

**Retorno:** diferente de zero se o stream chegou ao fim do arquivo; zero se
não chegou.

```ps
import <stdio.h>

FILE *f = fopen("um.txt", "w+")
fputc(ord("x"), f)
rewind(f)
int c = fgetc(f)                 # lê o último (e único) byte
post(chr(c), feof(f) != 0)       # ninguém tentou ler além dele ainda
c = fgetc(f)                     # esta leitura falha
post(c == EOF, feof(f) != 0)
fclose(f)
remove("um.txt")
```

```saida
x False
True True
```

### `ferror`

```c
int ferror(FILE *stream);
```

Confere se deu erro no stream: o indicador de erro fica ligado até o
`clearerr` ou o `rewind`.

| Parâmetro | O que é |
|---|---|
| `stream` | o stream |

**Retorno:** diferente de zero se deu erro no stream; zero se não deu.

Ler de um stream aberto só pra escrita falha, e liga o indicador de erro:

```ps
import <stdio.h>
import <errno.h>
import <string.h>

FILE *f = fopen("so_escrita.txt", "w")
int c = fgetc(f)
post(c == EOF, ferror(f) != 0, feof(f) != 0)
printf("errno %d: %s\n", errno, strerror(errno))
clearerr(f)
post(ferror(f) != 0)
fclose(f)
remove("so_escrita.txt")
```

```saida
True True False
errno 9: Bad file descriptor
False
```

### `perror`

```c
void perror(const char *s);
```

Escreve no `stderr` a descrição do erro guardado no `errno`: o texto `s` e
`": "` (os dois ficam de fora se `s` é nulo ou vazio), a mensagem do sistema
pro código do `errno` (a mesma do `strerror(errno)`) e a quebra de linha.

| Parâmetro | O que é |
|---|---|
| `s` | o texto que explica o erro; `NULL` ou `""` escreve só a mensagem |

**Retorno:** nenhum (`void`).

```ps
import <stdio.h>

FILE *f = fopen("nao_existe.txt", "r")
if f == NULL {
    perror("fopen() falhou")
}
post("o programa segue")
```

```saida
o programa segue
```

```erro
fopen() falhou: No such file or directory
```

## Operações com arquivos

### `remove`

```c
int remove(const char *filename);
```

Apaga o arquivo de nome `filename`. Se o arquivo está aberto por algum
processo, o que acontece é do sistema: o POSIX tira o nome do diretório, mas
o espaço no disco só volta quando nenhum processo o tem aberto e não há outro
link pra ele; o Windows não deixa apagar. O POSIX diz mais coisas sobre o
`remove` (a pasta vazia, por exemplo, ele também apaga).

| Parâmetro | O que é |
|---|---|
| `filename` | o nome do arquivo a apagar |

**Retorno:** `0` quando dá certo; diferente de zero quando falha.

```ps
import <stdio.h>

FILE *fp = fopen("arquivo1.txt", "w")
puts("criou arquivo1.txt")
fclose(fp)
if remove("arquivo1.txt") == 0 {
    puts("apagou arquivo1.txt")
}
fp = fopen("arquivo1.txt", "r")
if fp == NULL {
    perror("abrir o arquivo apagado falhou")
}
if remove("arquivo1.txt") != 0 {
    perror("apagar de novo falhou")
}
```

```saida
criou arquivo1.txt
apagou arquivo1.txt
```

```erro
abrir o arquivo apagado falhou: No such file or directory
apagar de novo falhou: No such file or directory
```

### `rename`

```c
int rename(const char *old, const char *new);
```

Troca o nome do arquivo `old` pra `new`. Se já existe um arquivo com o nome
`new`, o que acontece é do sistema; o POSIX (o Linux e o macOS) troca o
arquivo antigo pelo novo, e diz mais coisas sobre o `rename`.

| Parâmetro | O que é |
|---|---|
| `old` | o nome atual do arquivo |
| `new` | o nome novo |

**Retorno:** `0` quando dá certo; diferente de zero quando falha.

```ps
import <stdio.h>

FILE *fp = fopen("de.txt", "w")
fputc(ord("a"), fp)
fclose(fp)
if rename("de.txt", "para.txt") != 0 {
    perror("rename")
}
fp = fopen("para.txt", "r")
printf("%c\n", fgetc(fp))
fclose(fp)
post(rename("de.txt", "outro.txt") != 0)     # de.txt não existe mais
remove("para.txt")
```

```saida
a
True
```

### `tmpfile`

```c
FILE *tmpfile(void);
```

Cria e abre um arquivo temporário, com um nome que não se repete no sistema
de arquivos, em modo binário de atualização (como o `fopen` com `"wb+"`). O
arquivo some quando é fechado ou quando o programa termina normalmente; se o
programa termina de outro jeito, se ele é apagado é do sistema. O programa
pode abrir pelo menos `TMP_MAX` arquivos assim (limite que pode ser dividido
com o `tmpnam` e que o `FOPEN_MAX` pode diminuir).

Em alguns sistemas (o Linux antigo, por exemplo), o `tmpfile` cria o
arquivo, abre e já o apaga do diretório: enquanto o stream está aberto, o
arquivo existe, mas nenhum outro processo o abre pelo nome, e o espaço volta
quando o stream fecha ou o programa termina. Desde o Linux 3.11, conforme o
sistema de arquivos, esse arquivo invisível nasce num passo só, por uma opção
do `open()`. No Windows, criar o arquivo pode pedir permissão de
administrador, porque ele pode ir pra uma pasta do sistema.

Sem parâmetros.

**Retorno:** o stream do arquivo temporário; `NULL` quando falha.

```ps
import <stdio.h>

FILE *tmpf = tmpfile()
fputs("Bom dia, Jinga", tmpf)
rewind(tmpf)
char buf[4]
fgets(buf, sizeof(buf), tmpf)
printf("leu do arquivo: '%s'\n", buf)
fclose(tmpf)
```

```saida
leu do arquivo: 'Bom'
```

### `tmpnam`

```c
char *tmpnam(char *s);
```

Gera um nome de arquivo válido e único, com no máximo `L_tmpnam` caracteres,
e o guarda em `s`. Gera até `TMP_MAX` nomes diferentes, mas alguns podem já
estar em uso. O `tmpnam` mexe em estado estático e não precisa ser seguro
entre threads.

Entre o `tmpnam` devolver o nome e o programa criar o arquivo, outro processo
pode criar um arquivo com esse nome. O `tmpfile` e o `mkstemp` do POSIX não
têm esse problema; por isso o compilador ou o ligador do C (o do GCC, por
exemplo) avisa que o `tmpnam` é perigoso.

Na Jinga, o buffer de `L_tmpnam` bytes vem do `malloc` (o tamanho de um array
do C é um número escrito no código).

| Parâmetro | O que é |
|---|---|
| `s` | o buffer, com pelo menos `L_tmpnam` bytes; `NULL` usa um buffer estático da biblioteca |

**Retorno:** `s`, quando não é nulo; senão, o buffer estático. `NULL` quando
não consegue gerar um nome.

O nome muda a cada chamada e de sistema pra sistema; o exemplo mostra que ele
serve pra criar um arquivo novo:

```ps
import <stdio.h>
import <stdlib.h>
import <string.h>

char *nome = tmpnam(NULL)                 # no buffer estático
post(nome != NULL, strlen(nome) > 0)
char *meu = malloc(L_tmpnam)
post(tmpnam(meu) == meu)
FILE *f = fopen(meu, "wx")                # o x falharia se o nome existisse
post(f != NULL)
fclose(f)
remove(meu)
free(meu)
```

```saida
True True
True
True
```

## Tipos

### `FILE`

O tipo do stream do C. Cada `FILE` guarda o que o C precisa pra ler e
escrever no dispositivo (no POSIX, o descritor do arquivo) e, direto ou não:

| O quê | O que é |
|---|---|
| largura dos caracteres | sem orientação, estreita ou larga |
| estado da leitura multibyte | a conversão dos caracteres largos (um `mbstate_t`) |
| modo de buffer | sem buffer, buffer de linha ou buffer completo |
| o buffer | que o programa pode trocar pelo dele (`setbuf`, `setvbuf`) |
| modo | entrada, saída ou atualização (as duas) |
| texto ou binário | o indicador do modo |
| fim do arquivo | o indicador que o `feof` lê |
| erro | o indicador que o `ferror` lê |
| a posição | que o `fgetpos` guarda num `fpos_t` |
| a trava | contra a disputa entre threads (C11) |

- O stream novo não tem orientação; a primeira operação de entrada e saída
  diz se ele é estreito (as funções daqui) ou largo (as do `wchar.h`), e só o
  `freopen` muda isso.
- O stream de texto é uma sequência de linhas, cada uma com zero ou mais
  caracteres e o `'\n'` no fim; o sistema pode mudar caracteres na entrada e
  na saída (o Windows troca o `'\n'` por `'\r\n'` na escrita, e o contrário
  na leitura). O stream binário guarda os bytes como foram escritos. O Linux
  e o macOS (POSIX) não diferenciam os dois.
- O padrão não diz se o `FILE` pode ser copiado: o programa usa sempre o
  ponteiro que o `fopen` devolveu.

Na Jinga, o `FILE` só existe atrás de ponteiro, como no `fopen`: a variável
`FILE *f` vale, e `FILE f` é erro antes de rodar.

```ps
import <stdio.h>

FILE *f = fopen("um.txt", "w")
post(type(f), f != NULL)
fclose(f)
remove("um.txt")
```

```saida
FILE * True
```

```ps
import <stdio.h>

FILE f = Null
# TypeError: FILE só existe atrás de ponteiro: declare FILE * (variável f)
```

### `fpos_t`

O tipo que guarda a posição de um stream e o estado da leitura multibyte
(o `mbstate_t` do stream largo), pro `fgetpos` guardar e o `fsetpos`
restaurar. O que ele tem dentro é do sistema.

Na Jinga, o `fpos_t` só existe atrás de ponteiro, como o `FILE`: a memória
dele vem do `malloc(sizeof(fpos_t))`. O exemplo está no
[`fgetpos`](#fgetpos).

## Os streams padrão

### `stdin`, `stdout`, `stderr`

Os três streams de texto que o C abre no começo do programa, sem orientação:

| Stream | O que é | Buffer no começo |
|---|---|---|
| `stdin` | a entrada padrão, de onde o programa lê | completo só quando não é um dispositivo interativo |
| `stdout` | a saída padrão, onde o programa escreve | completo só quando não é um dispositivo interativo |
| `stderr` | a saída de erro, pras mensagens de diagnóstico | nunca completo |

O que é dispositivo interativo é do sistema. No Unix, o costume é o `stdin`
e o `stdout` com buffer de linha no terminal, e o `stderr` sem buffer.

Na Jinga, os três são valores `FILE *`, e o `post` escreve no mesmo `stdout`
que o `printf`, na ordem em que foram chamados.

```ps
import <stdio.h>

post(type(stdin), type(stdout), type(stderr))
fprintf(stdout, "pelo fprintf no stdout\n")
post("pelo post, no mesmo stdout")
fprintf(stderr, "no stderr\n")
```

```saida
FILE * FILE * FILE *
pelo fprintf no stdout
pelo post, no mesmo stdout
```

```erro
no stderr
```

## Constantes

### `EOF`, `BUFSIZ`, `FOPEN_MAX`, `FILENAME_MAX`, `L_tmpnam`, `TMP_MAX`

O fim do arquivo e os limites do `stdio.h`. O valor é o do C de cada sistema
(`post(BUFSIZ)` mostra o deste); a coluna do meio diz o que o C11 garante.

| Constante | O que é | Linux (glibc) | macOS |
|---|---|---|---|
| `EOF` | o `int` negativo que as funções de leitura devolvem no fim do arquivo (e no erro) | -1 | -1 |
| `BUFSIZ` | o tamanho do buffer do `setbuf`; pelo menos 256 | 8192 | 1024 |
| `FOPEN_MAX` | quantos arquivos o sistema garante abrir ao mesmo tempo; pelo menos 8, contando os três streams padrão | 16 | 20 |
| `FILENAME_MAX` | o tamanho de um array de `char` que cabe o nome de arquivo mais comprido que o sistema garante abrir | 4096 | 1024 |
| `L_tmpnam` | o tamanho de um array de `char` que cabe o nome que o `tmpnam` gera | 20 | 1024 |
| `TMP_MAX` | quantos nomes diferentes o `tmpnam` gera, no mínimo; pelo menos 25 | 238328 | 308915776 |

### `_IOFBF`, `_IOLBF`, `_IONBF`

Os modos de buffer do [`setvbuf`](#setvbuf): `_IOFBF` é o buffer completo
(a escrita vai pro sistema quando o buffer enche), `_IOLBF` é o buffer de
linha (vai também em cada quebra de linha), e `_IONBF` é sem buffer (cada
escrita vai direto). O C11 só exige que os três sejam diferentes; no Linux e
no macOS valem 0, 1 e 2.

### `SEEK_SET`, `SEEK_CUR`, `SEEK_END`

De onde o [`fseek`](#fseek) conta o deslocamento: do começo do arquivo
(`SEEK_SET`), da posição atual (`SEEK_CUR`) ou do fim (`SEEK_END`). O C11 só
exige que os três sejam diferentes; no Linux e no macOS valem 0, 1 e 2.

```ps
import <stdio.h>

post(EOF, _IOFBF, _IOLBF, _IONBF, SEEK_SET, SEEK_CUR, SEEK_END)
```

```saida
-1 0 1 2 0 1 2
```

## Fica de fora

Estes nomes do `stdio.h` do C11 não existem na Jinga, e usar um deles é
`NameError` antes de rodar, com o motivo:

| Nomes | Por quê |
|---|---|
| `vprintf`, `vfprintf`, `vsprintf`, `vsnprintf`, `vscanf`, `vfscanf`, `vsscanf` | o va_list só existe dentro de uma função variádica do C |
| `gets` | saiu do C11, porque lia sem limite e passava do fim do buffer; use fgets |
| `printf_s`, `fprintf_s`, `sprintf_s`, `snprintf_s`, `scanf_s`, `fscanf_s`, `sscanf_s`, `vprintf_s`, `vfprintf_s`, `vsprintf_s`, `vsnprintf_s`, `vscanf_s`, `vfscanf_s`, `vsscanf_s`, `fopen_s`, `freopen_s`, `gets_s`, `tmpfile_s`, `tmpnam_s`, `L_tmpnam_s`, `TMP_MAX_S` | é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |

As funções do texto largo (`fwide`, `fgetwc`, `fputws`, `fwprintf`...) são do
`wchar.h`, que ainda não está na Jinga (é a segunda parte da biblioteca do C).

```ps
import <stdio.h>

gets(1)
# NameError: gets fica de fora: saiu do C11, porque lia sem limite e passava do fim do buffer; use fgets (doc: o C na Jinga)
```
