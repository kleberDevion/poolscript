# time.h — data e hora

As funções do `time.h` leem o relógio do sistema e o tempo de processador,
convertem o tempo do calendário em data e hora (em UTC ou na hora local) e
escrevem a data como texto. Os nomes valem com `import <time.h>` no topo do
arquivo, igual ao `#include` do C; sem o import, o nome é `NameError`, com a
dica do import que falta ([o C na Jinga](c.md)).

> Esta página segue a cppreference
> ([en.cppreference.com/w/c/chrono](https://en.cppreference.com/w/c/chrono),
> licença [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)),
> traduzida e adaptada à Jinga. Os protótipos são os do C11 (Anexo B do
> N1570), conferidos contra a tabela do motor. Cada exemplo foi rodado.

| Função | O que faz |
|---|---|
| [`clock`](#clock) | o tempo de processador gasto pelo programa |
| [`difftime`](#difftime) | a diferença entre dois tempos, em segundos |
| [`time`](#time) | a hora atual do calendário |
| [`timespec_get`](#timespec_get) | a hora atual em segundos e nanossegundos |
| [`asctime`](#asctime) | a data de uma `struct tm` como texto de forma fixa |
| [`ctime`](#ctime) | a data de um `time_t` como texto, na hora local |
| [`strftime`](#strftime) | a data de uma `struct tm` como texto, com formato |
| [`gmtime`](#gmtime) | o `time_t` em data e hora UTC |
| [`localtime`](#localtime) | o `time_t` em data e hora local |
| [`mktime`](#mktime) | a data e hora local em `time_t` |

| Tipo ou constante | O que é |
|---|---|
| [`time_t`](#time_t) | o tempo do calendário, desde a época |
| [`clock_t`](#clock_t) | o tempo de processador |
| [`struct tm`](#struct-tm) | a data e a hora quebradas em campos |
| [`struct timespec`](#struct-timespec) | um tempo em segundos e nanossegundos |
| [`CLOCKS_PER_SEC`](#clocks_per_sec) | os tiques do `clock` por segundo |
| [`TIME_UTC`](#time_utc) | a base de tempo UTC do `timespec_get` |

## Na Jinga

- O `import <time.h>` traz também o `NULL` e o `size_t`: como no C, os dois
  valem com qualquer cabeçalho que os define (`stddef.h`, `stdio.h`,
  `stdlib.h`, `string.h`, `time.h`). O `printf` é do `stdio.h`.
- No Linux e no macOS, o `time_t` é o `long` (8 bytes) e conta os segundos
  desde 1970-01-01 00:00:00 UTC, o tempo do POSIX. O `clock_t` é o `long` no
  Linux e o `unsigned long` no macOS. Numa variável da Jinga
  (`time_t t = 0`), o valor é um `int` da Jinga.
- A conta é a da Jinga: `(fim - inicio) / CLOCKS_PER_SEC` dá `flo`, onde o C
  faria a divisão inteira.
- `gmtime` e `localtime` devolvem um ponteiro (`struct tm *`) pra uma
  `struct tm` da biblioteca do C, que a chamada seguinte de qualquer uma das
  duas (e o `ctime`) pode reescrever. O campo se lê com `->`
  (`t->tm_year`); pra guardar a data, copie a estrutura:
  `struct tm copia = *t`. Uma `struct tm d` declarada sem `=` nasce zerada.
- `asctime` e `ctime` devolvem um `char *` pra um texto da biblioteca do C,
  também reescrito pela chamada seguinte: `str(p)` traz o texto pra Jinga. O
  `strftime` escreve num buffer do C (`char buf[64]`), e `str(buf)` lê.
- A Jinga não muda o locale (o `locale.h` é do lote C2): vale o locale "C",
  e o `strftime` escreve os nomes em inglês (`Thursday`, `January`).
- A hora local (`localtime`, `ctime`, `mktime`) segue o fuso do sistema; no
  Linux e no macOS, a variável de ambiente `TZ` manda, como no POSIX.
- Da `struct tm`, a Jinga alcança os nove campos do padrão. A memória é a do
  sistema, com os campos que ele acrescenta (`tm_gmtoff` e `tm_zone` no Linux
  e no macOS), que o `sizeof(struct tm)` conta.

Só com o `import <time.h>`, o `NULL` e o `size_t` já valem:

```ps
import <time.h>

time_t agora = time(NULL)
size_t tamanho = sizeof(time_t)
post(agora > 0, tamanho)
```

```saida
True 8
```

Sem o import, o nome do C não existe no arquivo, e o erro sai antes de rodar:

```ps
time_t t = 0    # NameError: falta o import <time.h>
```

```saida
```

```erro
NameError: name 'time_t' is not defined. Did you forget to import <time.h>?
```

## Manipulação do tempo

### `clock`

```c
clock_t clock(void);
```

Devolve o tempo de processador aproximado que o processo usou desde o começo
de uma era definida pela implementação, ligada à execução do programa. Pra
converter o resultado em segundos, divida por `CLOCKS_PER_SEC`.

Só a diferença entre dois valores de chamadas diferentes do `clock` tem
sentido: o começo da era do `clock` não precisa coincidir com o começo do
programa.

O tempo do `clock` pode andar mais rápido ou mais devagar que o relógio de
parede, conforme os recursos de execução que o sistema dá ao programa. Se o
processador é dividido com outros processos, o tempo do `clock` anda mais
devagar que o relógio de parede; se o processo tem várias threads e há mais de
um núcleo livre, anda mais rápido.

**Retorno:** o tempo de processador usado pelo programa até agora;
`(clock_t)(-1)` se esse tempo não está disponível; um valor não especificado
se ele não cabe num `clock_t`.

O valor pode dar a volta em alguns sistemas: com um `clock_t` de 32 bits com
sinal e `CLOCKS_PER_SEC` de 1000000, isso acontece depois de 2147 segundos
(uns 36 minutos). No Linux e no macOS, o `clock_t` tem 64 bits.

```ps
import <time.h>

clock_t inicio = clock()
int soma = 0
for each i in range(300000) {
    soma = soma + i
}
clock_t fim = clock()
flo segundos = (fim - inicio) / CLOCKS_PER_SEC
post(fim >= inicio, segundos >= 0.0, type(segundos))
```

```saida
True True flo
```

### `difftime`

```c
double difftime(time_t time1, time_t time0);
```

Calcula a diferença, em segundos, entre dois tempos do calendário
(`time1 - time0`). Se `time1` é anterior a `time0`, o resultado é negativo.

| Parâmetro | O que é |
|---|---|
| `time1` | o tempo do fim |
| `time0` | o tempo do começo |

**Retorno:** a diferença entre os dois tempos, em segundos, como `double` (na
Jinga, `flo`).

No POSIX, o `time_t` conta segundos e o `difftime` é a subtração; o C permite
unidades fracionárias no `time_t`.

```ps
import <time.h>

time_t inicio = 1000000000
time_t fim = inicio + 90 * 60
post(difftime(fim, inicio), difftime(inicio, fim))
```

```saida
5400.0 -5400.0
```

### `time`

```c
time_t time(time_t *timer);
```

Devolve a hora atual do calendário como um `time_t` e também a guarda no
`time_t` apontado por `timer`, quando `timer` não é um ponteiro nulo.

| Parâmetro | O que é |
|---|---|
| `timer` | ponteiro pro `time_t` que recebe a hora, ou `NULL` |

**Retorno:** a hora atual do calendário como `time_t`, ou `(time_t)(-1)` se
deu erro. Se `timer` não é nulo, o mesmo valor fica guardado no `time_t`
apontado.

A codificação do tempo no `time_t` não é especificada pelo C, mas a maioria
dos sistemas segue o POSIX e devolve um inteiro com os segundos desde a época.
Onde o `time_t` é um inteiro de 32 bits com sinal (muitos sistemas antigos),
ele estoura no ano de 2038.

```ps
import <time.h>

time_t agora = time(NULL)
post(agora > 1700000000)    # depois de 14 de novembro de 2023
time_t guardado = 0
time_t devolvido = time(&guardado)
post(guardado == devolvido)
```

```saida
True
True
```

### `timespec_get`

```c
int timespec_get(struct timespec *ts, int base);
```

Muda a `struct timespec` apontada por `ts` pra guardar a hora atual do
calendário na base de tempo `base`.

Se `base` é `TIME_UTC`, o `ts->tv_sec` recebe os segundos desde uma época
definida pela implementação, truncados num valor inteiro, e o `ts->tv_nsec`
recebe os nanossegundos, arredondados pra resolução do relógio do sistema. Os
outros valores de `base` são definidos pela implementação.

| Parâmetro | O que é |
|---|---|
| `ts` | ponteiro pra `struct timespec` que recebe a hora |
| `base` | `TIME_UTC`, ou outro valor diferente de zero que indique uma base de tempo |

**Retorno:** o valor de `base` se deu certo; zero se não.

```ps
import <time.h>

struct timespec ts
if timespec_get(&ts, TIME_UTC) == TIME_UTC {
    post(ts.tv_sec > 1700000000, ts.tv_nsec >= 0 and ts.tv_nsec < 1000000000)
}
```

```saida
True True
```

## Conversão de formato

### `asctime`

```c
char *asctime(const struct tm *timeptr);
```

Converte a data e a hora de uma `struct tm` num texto de forma fixa, com 25
caracteres: `Www Mmm dd hh:mm:ss yyyy\n`, onde

- `Www` é o dia da semana abreviado em inglês, de `timeptr->tm_wday`: `Mon`,
  `Tue`, `Wed`, `Thu`, `Fri`, `Sat` ou `Sun`;
- `Mmm` é o mês abreviado em inglês, de `timeptr->tm_mon`: `Jan`, `Feb`,
  `Mar`, `Apr`, `May`, `Jun`, `Jul`, `Aug`, `Sep`, `Oct`, `Nov` ou `Dec`;
- `dd` é o dia do mês, de `timeptr->tm_mday`, como se escrito pelo `sprintf`
  com `%2d`;
- `hh`, `mm` e `ss` são a hora, o minuto e o segundo, de `timeptr->tm_hour`,
  `timeptr->tm_min` e `timeptr->tm_sec`, como se escritos com `%.2d`;
- `yyyy` é o ano, de `timeptr->tm_year + 1900`, como se escrito com `%4d`.

O comportamento é indefinido se algum campo de `*timeptr` está fora da faixa
normal, ou se o ano tem mais de 4 dígitos ou é anterior ao ano 1000. A função
não segue o locale, e não há como tirar a quebra de linha do fim. Ela mexe em
memória estática e não é segura entre threads.

| Parâmetro | O que é |
|---|---|
| `timeptr` | ponteiro pra `struct tm` com a data a escrever |

**Retorno:** ponteiro pra um texto estático, terminado em nulo, com a data e a
hora na forma acima. O texto pode ser o mesmo do `ctime` e pode ser reescrito a
cada chamada de qualquer uma das duas.

O POSIX marca o `asctime` como obsoleto e recomenda o `strftime`; o padrão C
também recomenda o `strftime`, que é mais flexível e segue o locale.

```ps
import <time.h>

time_t t = 1000000000
str texto = str(asctime(gmtime(&t)))
post(texto.strip())
post(len(texto))
```

```saida
Sun Sep  9 01:46:40 2001
25
```

### `ctime`

```c
char *ctime(const time_t *timer);
```

Converte um tempo desde a época na hora local do calendário e depois em texto,
como se chamasse `asctime(localtime(timer))`. O texto tem a forma
`Www Mmm dd hh:mm:ss yyyy\n`, e a função não segue o locale.

| Parâmetro | O que é |
|---|---|
| `timer` | ponteiro pro `time_t` com o tempo a escrever |

**Retorno:** ponteiro pra um texto estático, terminado em nulo, com a data e a
hora. O texto pode ser o mesmo do `asctime` e pode ser reescrito a cada chamada
de qualquer uma das duas.

O `ctime` devolve um ponteiro pra memória estática e não é seguro entre
threads; ele também mexe na `struct tm` estática que o `gmtime` e o
`localtime` podem usar. O comportamento é indefinido quando o texto passaria
de 25 caracteres (o ano 10000, por exemplo). O POSIX marca o `ctime` como
obsoleto e recomenda o `strftime`.

```ps
import <time.h>

time_t t = 1000000000
str a = str(ctime(&t))
str b = str(asctime(localtime(&t)))
post(a == b, len(a))
```

```saida
True 25
```

### `strftime`

```c
size_t strftime(char * restrict s, size_t maxsize, const char * restrict format, const struct tm * restrict timeptr);
```

Converte a data e a hora de `timeptr` num texto multibyte terminado em nulo,
escrito em `s`, conforme o texto de formato `format`. No máximo `maxsize`
bytes são escritos.

| Parâmetro | O que é |
|---|---|
| `s` | ponteiro pro primeiro elemento do array de `char` que recebe o texto |
| `maxsize` | o máximo de bytes a escrever |
| `format` | o texto de formato (abaixo) |
| `timeptr` | ponteiro pra `struct tm` com a data |

**Retorno:** o número de bytes escritos em `s`, sem o `'\0'` do fim. Se o
texto não cabe em `maxsize` bytes, devolve `0`, e o conteúdo de `s` fica
indeterminado.

O formato tem zero ou mais especificações de conversão e caracteres comuns
(menos o `%`). Os caracteres comuns, inclusive o nulo do fim, vão pra saída
sem mudança. Cada especificação começa com `%`, seguido do caractere que
decide o que sai. Os exemplos são os do locale "C", o da Jinga, com a data
2001-09-09 01:46:40 (domingo):

| Conversão | Escreve | Exemplo | Campos usados |
|---|---|---|---|
| `%%` | o `%` | `%` | |
| `%n` | uma quebra de linha | | |
| `%t` | uma tabulação horizontal | | |
| `%Y` | o ano | `2001` | `tm_year` |
| `%y` | os 2 últimos dígitos do ano (`00` a `99`) | `01` | `tm_year` |
| `%C` | os 2 primeiros dígitos do ano (`00` a `99`) | `20` | `tm_year` |
| `%G` | o ano da semana da ISO 8601: o ano que contém a semana | `2001` | `tm_year`, `tm_wday`, `tm_yday` |
| `%g` | os 2 últimos dígitos do ano da semana da ISO 8601 | `01` | `tm_year`, `tm_wday`, `tm_yday` |
| `%b`, `%h` | o mês abreviado | `Sep` | `tm_mon` |
| `%B` | o mês por extenso | `September` | `tm_mon` |
| `%m` | o mês (`01` a `12`) | `09` | `tm_mon` |
| `%U` | a semana do ano, com o domingo como primeiro dia (`00` a `53`) | `36` | `tm_year`, `tm_wday`, `tm_yday` |
| `%W` | a semana do ano, com a segunda como primeiro dia (`00` a `53`) | `36` | `tm_year`, `tm_wday`, `tm_yday` |
| `%V` | a semana da ISO 8601 (`01` a `53`) | `36` | `tm_year`, `tm_wday`, `tm_yday` |
| `%j` | o dia do ano (`001` a `366`) | `252` | `tm_yday` |
| `%d` | o dia do mês (`01` a `31`) | `09` | `tm_mday` |
| `%e` | o dia do mês (`1` a `31`); um dígito só ganha um espaço antes | ` 9` | `tm_mday` |
| `%a` | o dia da semana abreviado | `Sun` | `tm_wday` |
| `%A` | o dia da semana por extenso | `Sunday` | `tm_wday` |
| `%w` | o dia da semana, com o domingo `0` (`0` a `6`) | `0` | `tm_wday` |
| `%u` | o dia da semana, com a segunda `1` (`1` a `7`, da ISO 8601) | `7` | `tm_wday` |
| `%H` | a hora no relógio de 24 horas (`00` a `23`) | `01` | `tm_hour` |
| `%I` | a hora no relógio de 12 horas (`01` a `12`) | `01` | `tm_hour` |
| `%M` | o minuto (`00` a `59`) | `46` | `tm_min` |
| `%S` | o segundo (`00` a `60`) | `40` | `tm_sec` |
| `%c` | a data e a hora do locale | `Sun Sep  9 01:46:40 2001` | todos |
| `%x` | a data do locale | `09/09/01` | todos |
| `%X` | a hora do locale | `01:46:40` | todos |
| `%D` | o mesmo que `%m/%d/%y` | `09/09/01` | `tm_mon`, `tm_mday`, `tm_year` |
| `%F` | o mesmo que `%Y-%m-%d`, a data da ISO 8601 | `2001-09-09` | `tm_mon`, `tm_mday`, `tm_year` |
| `%r` | a hora do relógio de 12 horas do locale | `01:46:40 AM` | `tm_hour`, `tm_min`, `tm_sec` |
| `%R` | o mesmo que `%H:%M` | `01:46` | `tm_hour`, `tm_min` |
| `%T` | o mesmo que `%H:%M:%S`, a hora da ISO 8601 | `01:46:40` | `tm_hour`, `tm_min`, `tm_sec` |
| `%p` | o AM ou PM do locale | `AM` | `tm_hour` |
| `%z` | a diferença pra UTC na forma da ISO 8601 (`-0430`), ou nada se o fuso não é conhecido | | `tm_isdst` |
| `%Z` | o nome ou a abreviação do fuso, ou nada se o fuso não é conhecido | | `tm_isdst` |

Na semana da ISO 8601 (`%G`, `%g`, `%V`), a semana começa na segunda, e a
primeira semana do ano é a que tem o dia 4 de janeiro (a que tem a primeira
quinta-feira do ano).

Entre o `%` e a letra pode vir o modificador `E` (`%Ec`, `%EC`, `%Ex`, `%EX`,
`%Ey`, `%EY`) ou `O` (`%Od`, `%Oe`, `%OH`, `%OI`, `%Om`, `%OM`, `%OS`, `%Ou`,
`%OU`, `%OV`, `%Ow`, `%OW`, `%Oy`): a conversão usa a representação
alternativa do locale (a era japonesa no lugar do ano, os numerais chineses no
lugar dos dígitos). No locale "C", o modificador é ignorado.

O `%z` e o `%Z` dependem do sistema: com a `struct tm` do `gmtime`, o Linux
escreve `+0000` e `GMT`, e o macOS escreve a diferença do fuso local e `UTC`.

```ps
import <time.h>

time_t t = 1000000000
struct tm *d = gmtime(&t)
char buf[64]
size_t n = strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", d)
post(str(buf), n)
strftime(buf, sizeof(buf), "%A, %d de %B de %Y, dia %j do ano", d)
post(str(buf))
char curto[8]
post(strftime(curto, sizeof(curto), "%Y-%m-%d", d))
```

```saida
2001-09-09 01:46:40 19
Sunday, 09 de September de 2001, dia 252 do ano
0
```

### `gmtime`

```c
struct tm *gmtime(const time_t *timer);
```

Converte o tempo desde a época (o `time_t` apontado por `timer`) em data e hora
do calendário, expressas no Tempo Universal Coordenado (UTC), na forma da
`struct tm`. O resultado fica em memória estática, e a função devolve um
ponteiro pra ela.

| Parâmetro | O que é |
|---|---|
| `timer` | ponteiro pro `time_t` a converter |

**Retorno:** ponteiro pra uma `struct tm` estática interna, ou ponteiro nulo
(`NULL`) se não deu certo. A estrutura pode ser a mesma do `localtime` e do
`ctime` e pode ser reescrita a cada chamada.

O `gmtime` pode não ser seguro entre threads. O POSIX manda o `gmtime` pôr o
`EOVERFLOW` no `errno` quando falha porque o argumento é grande demais.

```ps
import <time.h>

time_t zero = 0
struct tm *t = gmtime(&zero)
post(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_wday)
time_t longe = 9223372036854775807
post(gmtime(&longe) == NULL)    # o ano não cabe no int do tm_year
```

```saida
1970 1 1 0 4
True
```

### `localtime`

```c
struct tm *localtime(const time_t *timer);
```

Converte o tempo desde a época (o `time_t` apontado por `timer`) em data e hora
do calendário, expressas na hora local, na forma da `struct tm`. O resultado
fica em memória estática, e a função devolve um ponteiro pra ela.

| Parâmetro | O que é |
|---|---|
| `timer` | ponteiro pro `time_t` a converter |

**Retorno:** ponteiro pra uma `struct tm` estática interna, ou ponteiro nulo
(`NULL`) se não deu certo. A estrutura pode ser a mesma do `gmtime` e do
`ctime` e pode ser reescrita a cada chamada.

O `localtime` pode não ser seguro entre threads. O POSIX manda pôr o
`EOVERFLOW` no `errno` quando ele falha porque o argumento é grande demais, e
diz que o fuso vem como se fosse pelo `tzset`, que lê a variável de ambiente
`TZ`.

O exemplo vale em qualquer fuso: o `mktime` desfaz o `localtime`.

```ps
import <time.h>

time_t t = 1000000000
struct tm *local = localtime(&t)
post(local->tm_year + 1900, local->tm_isdst >= 0)
post(mktime(local) == t)
```

```saida
2001 True
True
```

### `mktime`

```c
time_t mktime(struct tm *timeptr);
```

Renormaliza a hora local do calendário guardada numa `struct tm` e a converte
em tempo desde a época, como `time_t`. O `timeptr->tm_wday` e o
`timeptr->tm_yday` são ignorados, e os valores dos campos não são conferidos
contra a faixa normal.

Um `timeptr->tm_isdst` negativo faz o `mktime` tentar descobrir se o horário de
verão estava em vigor na hora indicada.

Se a conversão pro `time_t` dá certo, o `mktime` muda a `struct tm`: todos os
campos voltam pra faixa normal, e o `tm_wday` e o `tm_yday` são recalculados a
partir dos outros campos.

| Parâmetro | O que é |
|---|---|
| `timeptr` | ponteiro pra `struct tm` com a hora local a converter |

**Retorno:** o tempo desde a época, como `time_t`, se deu certo; `-1` se a data
não cabe num `time_t` (o POSIX manda, nesse caso, pôr o `EOVERFLOW` no
`errno`).

O exemplo pede o 32 de janeiro, que o `mktime` vira 1º de fevereiro (um
domingo, o dia 31 do ano contado do zero):

```ps
import <time.h>

struct tm d
d.tm_year = 2026 - 1900
d.tm_mon = 0        # janeiro
d.tm_mday = 32
d.tm_hour = 12
d.tm_isdst = -1     # o sistema decide se é horário de verão
time_t t = mktime(&d)
post(t != -1)
post(d.tm_mon + 1, d.tm_mday, d.tm_wday, d.tm_yday)
```

```saida
True
2 1 0 31
```

## Tipos

### `time_t`

```c
typedef /* não especificado */ time_t;
```

Tipo real (inteiro ou de ponto flutuante) capaz de representar tempos. O
padrão C não diz o que ele conta, mas quase sempre é um inteiro com os
segundos, sem contar os segundos bissextos, desde 00:00 de 1º de janeiro de
1970 UTC: o tempo do POSIX. O padrão chama um valor do tipo `time_t` de *tempo
do calendário*.

| Sistema | `time_t` |
|---|---|
| Linux (64 bits) | `long`, 8 bytes, com sinal |
| macOS | `long`, 8 bytes, com sinal |

```ps
import <time.h>

time_t dia = 86400          # um dia, em segundos
struct tm *d = gmtime(&dia)
post(d->tm_year + 1900, d->tm_mon + 1, d->tm_mday)
post(sizeof(time_t))
```

```saida
1970 1 2
8
```

### `clock_t`

```c
typedef /* não especificado */ clock_t;
```

Tipo real capaz de representar o tempo de processador usado por um processo.
A faixa e a precisão são definidas pela implementação.

| Sistema | `clock_t` |
|---|---|
| Linux (64 bits) | `long`, 8 bytes, com sinal |
| macOS | `unsigned long`, 8 bytes, sem sinal |

### `struct tm`

```c
struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};
```

Estrutura com a data e a hora do calendário quebradas em componentes.

| Campo | O que é | Faixa |
|---|---|---|
| `tm_sec` | segundos depois do minuto | `0` a `60` |
| `tm_min` | minutos depois da hora | `0` a `59` |
| `tm_hour` | horas desde a meia-noite | `0` a `23` |
| `tm_mday` | dia do mês | `1` a `31` |
| `tm_mon` | meses desde janeiro | `0` a `11` |
| `tm_year` | anos desde 1900 | |
| `tm_wday` | dias desde domingo | `0` a `6` |
| `tm_yday` | dias desde 1º de janeiro | `0` a `365` |
| `tm_isdst` | o horário de verão: positivo se está em vigor, zero se não está, negativo se não se sabe | |

O `60` do `tm_sec` é o segundo bissexto positivo; dois no mesmo minuto não
existem (a faixa de `0` a `61` do C89 era um defeito). O padrão só manda ter
esses campos, em qualquer ordem; os sistemas costumam acrescentar outros.

```ps
import <time.h>

time_t t = 1000000000
struct tm *d = gmtime(&t)
post(d->tm_year + 1900, d->tm_mon + 1, d->tm_mday)
post(d->tm_hour, d->tm_min, d->tm_sec)
post(d->tm_wday, d->tm_yday, d->tm_isdst)
struct tm copia = *d
copia.tm_year = 0
post(copia.tm_year + 1900, d->tm_year + 1900)
```

```saida
2001 9 9
1 46 40
0 251 0
1900 2001
```

### `struct timespec`

```c
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};
```

Estrutura com um intervalo quebrado em segundos e nanossegundos.

| Campo | O que é |
|---|---|
| `tv_sec` | os segundos inteiros (valores válidos: `>= 0`) |
| `tv_nsec` | os nanossegundos (valores válidos: `0` a `999999999`) |

A ordem dos dois campos não é especificada, e a implementação pode acrescentar
outros. O `long` do `tv_nsec` é o do C (8 bytes no Linux e no macOS).

```ps
import <time.h>

struct timespec ts
ts.tv_sec = 1
ts.tv_nsec = 500000000
post(ts.tv_sec + ts.tv_nsec / 1000000000)
```

```saida
1.5
```

## Constantes

### `CLOCKS_PER_SEC`

```c
#define CLOCKS_PER_SEC /* definido pela implementação */
```

Expande pra uma expressão (não necessariamente uma constante de compilação)
do tipo `clock_t`, igual ao número de tiques por segundo do relógio que o
`clock()` lê.

O POSIX fixa o `CLOCKS_PER_SEC` em 1000000, qualquer que seja a precisão real
do relógio: é o valor no Linux e no macOS. Antes de entrar no C89 com esse
nome, a macro às vezes se chamava `CLK_TCK`, o nome do IEEE 1003.1-1988, que
não entrou no C89 e saiu do próprio POSIX em 1996.

```ps
import <time.h>

post(CLOCKS_PER_SEC)
```

```saida
1000000
```

### `TIME_UTC`

```c
#define TIME_UTC /* definido pela implementação */
```

A base de tempo UTC: um valor inteiro positivo, que é um `base` válido do
`timespec_get` (o exemplo está lá). A implementação pode definir outras macros
`TIME_` pra outras bases de tempo. No Linux e no macOS, vale `1`.

## Fica de fora

| Nome | Por quê |
|---|---|
| `asctime_s`, `ctime_s`, `gmtime_s`, `localtime_s` | são do Anexo K do C11, que é opcional e não existe no Linux nem no macOS |

Usar um deles é erro antes de rodar, com o motivo; nada sai no stdout:

```ps
import <time.h>

time_t t = 0
char buf[32]
asctime_s(buf, sizeof(buf), gmtime(&t))    # NameError: fica de fora
```

```saida
```

```erro
NameError: asctime_s fica de fora: é do Anexo K do C11, que é opcional e não existe no Linux nem no macOS
```

Os nomes que entraram no C23 (`gmtime_r`, `localtime_r`, `timegm`,
`timespec_getres`) não fazem parte do C11, que a Jinga segue.
