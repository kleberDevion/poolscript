# Referência da Linguagem — 6. Funções

Uma função agrupa um trecho de código sob um nome, recebe parâmetros e devolve
um valor. Na Jinga ela se declara com **`funct`**. Esta seção cobre a
definição, os parâmetros, o retorno, as formas tipadas (`int funct`/`bool
funct`), os modificadores colados (`static`, `nonnull`), funções como valores,
recursão e geradores.

Como sempre, cada comportamento foi verificado rodando o fonte de verdade.

---

## 6.1. Definição — `funct`

```ps
funct soma(a, b) {
    return a + b
}

post(soma(2, 3))     # 5
```

O corpo é um bloco `{ }`, como todo bloco da linguagem (seção 1.3).

> **`action` e `reaction` saíram da linguagem.** Eram as grafias antigas desta
> mesma declaração. Não são mais palavra reservada, e escrever qualquer uma
> delas onde `funct` deveria estar é erro de sintaxe, com o conserto na
> mensagem:
>
> ```
> action g() { ... }
> SyntaxError: 'action' saiu da linguagem; a funcao se declara com 'funct': funct g(args) { ... }
> ```
>
> A recusa cobre as quatro posições: função solta, método de Entity, lambda
> (`x = action(y) {`) e cabeça com modificadores (`int async reaction h(a)`).

---

## 6.2. Parâmetros

### 6.2.1. Posicionais e valores padrão

Parâmetros podem ter **valor padrão** (uma expressão, avaliada quando falta o
argumento):

```ps
funct g(a, b = 10) {
    return a + b
}

post(g(5))       # 15   (b usa o padrão)
post(g(5, 1))    # 6
```

O padrão pode ser qualquer expressão: `funct f(a, b = 5 * 2)` → `b` vale `10`.

### 6.2.2. Argumentos nomeados (na chamada)

Na chamada, um argumento pode ser passado pelo nome do parâmetro, em qualquer
ordem, e misturado com posicionais:

```ps
funct f(a, b, c) {
    return str(a) + str(b) + str(c)
}

post(f(c=3, a=1, b=2))    # "123"
post(f(1, c=3, b=2))      # "123"  (posicional + nomeado)
```

### 6.2.3. Parâmetro tipado — o tipo vem ANTES do nome

Um parâmetro pode declarar o tipo, e o tipo vem **antes** do nome, como no
campo de Entity (seção 7.2) e como em Java:

```ps
funct saudacao(str nome, int vezes) {
    return nome * vezes
}

post(saudacao("oi ", 3))     # "oi oi oi "
```

Vale **qualquer tipo** da tabela da seção 2.6 (`str`, `int`, `flo`, `bool`,
`char`, `long`, `list`, `dict`, `tup`, `byte`, `PoolFile`, `Object`), os
apelidos deles (`String` é `str`, `Integer` é `int`), o nome de uma **Entity**
e o nome de um tipo de objeto do motor (`Response`, `PoolCursor`,
`MailMessage`…).

O tipo é **checado na chamada**, e checar é tudo o que ele faz — argumento de
tipo errado é recusado, **nunca convertido**:

```ps
funct saudacao(str nome, int vezes) {
    return nome * vezes
}
saudacao(5, 3)
# AttributedValueError: parâmetro nome de saudacao() esperava str, recebeu int
```

A mesma regra vale para os `int`/`flo` que se parecem: `int` não aceita `flo`,
`flo` não aceita `int`, `bool` não aceita `int`. Para passar outro tipo,
converta você: `saudacao(str(5), 3)`.

Numa **Entity**, o tipo aceita subclasse onde a mãe foi pedida:

```ps
Entity Animal { str nome }
Entity Cachorro(Animal) { }

funct fala(Animal a) { post(a.nome) }
fala(Cachorro("Rex"))        # Rex
```

Tipar é **opcional e por parâmetro** — pode misturar:

```ps
funct mist(str a, b, int c) { post(a, b, c) }
mist("a", [1], 2)            # a [1] 2
```

O tipo casa com **valor padrão** e com **argumento nomeado**:

```ps
funct pad(str a, int n = 2) { return a * n }
post(pad("x"))               # "xx"
post(pad("x", n=3))          # "xxx"
post(pad("x", n="y"))
# AttributedValueError: parâmetro n de pad() esperava int, recebeu str
```

A checagem vale em **todas** as formas de chamada: função solta, método,
método `static` chamado na Entity, lambda, gerador e `async funct`.

A forma `funct f(x: int)` **não** existe — no parâmetro só há a ordem
`tipo nome`. (Em campo de Entity as duas ordens valem; no parâmetro, não.) O
erro diz a ordem certa:

```
funct f(x: int) { post(x) }
SyntaxError: no parametro o tipo vem ANTES do nome: escreva `funct f(int x)`, nao `funct f(x: int)`
```

### 6.2.4. Variádicos — `*args` e `**kwarg`

Uma funct pode receber **qualquer quantidade** de argumentos: `*args` guarda
os posicionais que sobraram numa **tup**, e `**kwarg` guarda os nomeados que
não casam com nenhum parâmetro num **dict**, na ordem da chamada. Os nomes
`args` e `kwarg` são convenção — qualquer nome vale.

```ps
funct f(a, *args, **kwarg) {
    post(a, args, kwarg)
}

f(1, 2, x=3)     # 1 (2,) {'x': 3}
f(1)             # 1 () {}
```

A ordem é fixa: parâmetros comuns (com ou sem padrão), depois `*args`, depois
`**kwarg`. Cada um aparece **no máximo uma vez**; nenhum aceita tipo (a
estrela já decide: tup e dict) nem valor padrão (sem argumento, vêm vazios).
O erro diz o conserto:

```
funct f(*args, x) { }
SyntaxError: parametro `x` depois de `*args` nao e permitido: mova `x` pra antes do `*args`

funct f(*int args) { }
SyntaxError: parametro `*int args` nao aceita tipo: `*args` e sempre tup e `**kwarg` sempre dict — escreva `*args`
```

O tipo escrito **antes** da estrela (`int *args`, `String *args`, `dict **kw`)
recebe a mesma frase.

Um nomeado que **casa** com um parâmetro fixo fica nele; só o que não casa
cai no `kwarg`:

```ps
funct g(a, b=2, **kwarg) {
    post(a, b, kwarg)
}

g(1, c=3, b=5)   # 1 5 {'c': 3}
```

**Espalhamento na chamada.** Do outro lado, `f(*lista)` passa os itens de
uma `list`/`tup` como posicionais e `f(**dict)` passa as chaves de um `dict`
como nomeados — em qualquer chamada (funct, método, Entity, builtin,
decorador, `base().__init__()`), misturados com argumentos comuns e na ordem escrita:

```ps
funct soma(a, b, c=0) {
    return a + b + c
}

l = [1, 2]
d = {"c": 3}
post(soma(*l))             # 3
post(soma(*l, **d))        # 6
post(soma(1, *[2], c=3))   # 6
post(max(*[4, 9, 2]))      # 9
```

Vale a regra de sempre: posicional (ou `*x`) antes de nomeado (ou `**d`), e
num nome repetido o **último ganha**. `*x` exige `list` ou `tup` e `**d`
exige `dict` — não há conversão:

```
f(*5)
TypeError: argument after * must be a list or tup, not int
```

`funct meio(*args, **kwarg)` chamando `alvo(*args, **kwarg)` é o **repasse**:
uma funct que envolve outra sem conhecer a assinatura dela — é o que um
decorador faz ([capítulo 14](14-decoradores.md)).

### 6.2.5. Aridade

Passar argumentos **de menos** (sem cobrir um parâmetro sem padrão) ou **de
mais** é erro em tempo de execução.

Uma funct tem no máximo **256 parâmetros fixos** (o `self` conta; `*args` e
`**kwarg` não). Passar disso é erro **na declaração**, antes de rodar — vale
pra funct, método e lambda:

```
SyntaxError: f() tem parametros demais (maximo 256)
```

O `__init__` que uma Entity gera dos campos recebe `self` + um parâmetro por
campo, então uma Entity com mais de 255 campos é recusada do mesmo jeito:
`Entity E: o __init__ gerado dos campos tem parametros demais (maximo 256: self + 255 campos)`.

---

## 6.3. Retorno — `return`

`return <expr>` devolve um valor e encerra a função. Um `return` **sem valor**,
ou uma função que **termina sem `return`**, devolve `null`:

```ps
funct nada() {
    return
}
post(nada())     # null

funct semret() {
    x = 1
}
post(semret())   # null
```

---

## 6.4. Functs tipadas — o tipo antes do `funct`

O que vem colado antes do `funct` (depois dos modificadores) **é o tipo de
retorno**, e ele é um contrato conferido **antes de rodar**:

- todo `return` da funct devolve um valor daquele tipo;
- toda **saída** que o corpo faz — `post`, `sys.stdout`/`sys.stderr`, escrita
  em arquivo — é daquele tipo;
- chegar ao fim sem `return` é erro: o fim devolveria `Null`, que não é de
  tipo nenhum. Só `int` e `bool` escapam, pelo sentinela abaixo.

Nada é convertido: `str funct` com `return 10` é erro, não `"10"`.

```ps
str funct nome() {
    return 10
}
# AttributedValueError: retorno de nome() esperava str, recebeu int
```

```ps
int funct conta() {
    post("total:", 1)      # saída de texto numa int funct
    return 1
}
# AttributedValueError: saída de conta() esperava int, recebeu str
```

O que só se sabe rodando (o valor de `json.parse`, de `d["k"]`) é conferido
rodando, no `return` — o mesmo erro, no outro momento.

`int funct` e `bool funct` têm, além disso, o **sentinela de erro**: o corpo
vira um `try` implícito e a funct nunca propaga exceção. É pensado para
handlers que precisam sempre devolver algo (um status, um sim/não).

O erro **não some**: o sentinela sai como valor e o erro sai **junto** no
stderr, no formato do traceback — tipo, mensagem, onde aconteceu — com uma
linha dizendo o que a funct devolveu no lugar. O programa segue com o
sentinela (o código de saída não muda por isso).

```
Boom: x
  em app.pr, linha 6
  |     raise Boom("x")
  |     ^^^
  int funct quebra() devolveu 500 no lugar do erro
```

`int funct`:

| Situação | Devolve |
|---|---|
| `return <int>` | o próprio inteiro |
| `return null` / sem return | `0` |
| erro/exceção no corpo | `500` |
| `return <outro tipo>` | erro de tipo — que o `try` implícito transforma em `500` |

```ps
int funct status() {
    return null
}
post(status())        # 0

int funct quebra() {
    raise Boom("x")
}
post(quebra())        # 500 — e o erro sai no stderr
```

`bool funct`: `return <bool>` devolve ele; **erro no corpo → `False`**;
`return null`/sem return → `True`. `return 1` é erro de tipo (e vira `False`
pelo mesmo `try`) — não há conversão por verdadeiro/falso.

### Qualquer tipo vale como retorno

Não há lista branca. O que vem colado antes do `funct`, depois dos
modificadores, **é** o tipo de retorno — qualquer tipo da linguagem
(`str`, `int`, `long`, `flo`, `bool`, `char`, `list`, `dict`, `tup`, `json`,
`Object`) e também o nome de uma classe sua:

```ps
Entity Pessoa { nome: str }

str  funct nome()      { return "ana" }
list funct itens()     { return [1, 2] }
Pessoa funct criar()   { return Pessoa("ana") }
```

Não há ambiguidade a resolver: nenhuma outra construção da linguagem tem um
nome seguido de `funct`.

Os **apelidos** resolvem para o tipo apelidado, e a árvore guarda o canônico —
`string funct f()` é a mesma coisa que `str funct f()`:

| Escrita | Vale como |
|---|---|
| `string`, `String` | `str` |
| `integer`, `Integer` | `int` |
| `tuple`, `Tuple` | `tup` |
| `dictionary`, `Dictionary`, `json`, `JSON` | `dict` |
| `object` | `Object` |
| `Long` | `long` |

A tabela é uma só pra toda a linguagem: o apelido vale igual em declaração
(`JSON j = {}`), parâmetro, retorno, campo, `x is JSON` e `count JSON in l`.

> `int` e `bool` são os únicos com sentinela de erro; a conferência do
> `return` e da saída vale pra todos os tipos.

Isto vale em qualquer posição — solta, dentro de `Entity`/`class` e em lambda —
e os modificadores vêm em qualquer ordem:

```ps
import sys

public class C {
    public static string funct main() {
        sys.stdout.writeln("Ola mundo!")
        return "Ola mundo!"
    }
}
```

Antes, dentro de uma classe, `string funct` não era reconhecido como cabeça de
funct: casava com a regra de campo e nascia um **campo chamado `string`, do
tipo `static`**. O método sumia da classe e o arquivo rodava sem erro nenhum e
sem fazer nada.

### 6.4.1. Modificadores colados — `static` e `nonnull`

Além do tipo de retorno, do `async` e da visibilidade, dois modificadores vêm
**colados na declaração**:

| Modificador | O que faz |
|---|---|
| `static` | o método pertence ao **tipo**, não ao objeto: chama-se `Tipo.metodo(...)`, sem instanciar (seção [7.4](07-entity.md)) |
| `nonnull` | nenhum parâmetro pode chegar `Null` — inclusive por valor padrão |

```ps
Entity Mat {
    static funct soma(a, b) {
        return a + b
    }
}
post(Mat.soma(1, 2))        # 3

nonnull funct exige(valor) {
    return valor
}
exige(Null)                 # nonnull: parametro 'valor' em 'exige' nao pode ser Null
```

Chamar um `static` pela instância é erro:
`funct 'soma' e static: chame pela Entity (Tipo.soma(...)), nao pela instancia`.

`static` numa funct **fora de Entity** é aceito, e ela não recebe `self` —
não há instância nunca. Declarar `static funct f(self)` no módulo é erro na
declaração: `funct static f(self, ...) nao recebe self: static nao tem
instancia, tire o self`. (Num método `static` de Entity o `self` é permitido
e dropado na chamada pela classe — [14-decoradores](14-decoradores.md).)

Os dois valem só para a funct em que estão escritos — uma funct declarada
**dentro** do corpo não herda a marca.

`static` e `nonnull` **não são palavras reservadas**: valem por posição, só na
cabeça da declaração. `static = 1` continua sendo uma variável comum.

> As grafias antigas `@static` e `@NonNull`, em linha própria acima da
> declaração, continuam funcionando — seção [14](14-decoradores.md).

### 6.4.2. Ordem dos modificadores é livre

Os prefixos de uma funct — tipo de retorno (`int`/`bool`/`str`/`flo`), `async`,
visibilidade (`public`/`private`) e os colados (`static`/`nonnull`) — podem vir
em **qualquer ordem**. Todos abaixo são equivalentes e válidos:

```ps
int async funct f() { }
async int funct f() { }
public async funct f() { }
private int funct f() { }
static nonnull funct f(v) { }
private static int funct f() { }
```

Vale igual com um [decorador](14-decoradores.md) em cima: `@app.post("/x")`
seguido de `int async funct h()` registra `h` como qualquer outra ordem.

Esquecer o `funct` é erro de sintaxe, e a mensagem devolve a linha montada:
`int async f(x)` dá `faltou 'funct' antes de 'f': int async funct f(...)`.

---

## 6.5. Funções são valores (first-class)

Uma funct pode ser guardada em variável, passada como argumento e devolvida —
sem os parênteses, o nome é a própria função:

```ps
funct dobro(n) {
    return n * 2
}

g = dobro
post(g(21))          # 42

funct aplica(fn, x) {
    return fn(x)
}
post(aplica(dobro, 21))   # 42
```

`type()` de uma função é `"funct"`, e `post` de uma delas imprime
`<funct #N>`.

---

## 6.6. Recursão

Uma funct pode chamar a si mesma:

```ps
funct fatorial(n) {
    if n <= 1 {
        return 1
    }
    return n * fatorial(n - 1)
}

post(fatorial(5))    # 120
```

Recursão sem fim é `RecursionError: maximum recursion depth exceeded`
(capturável com `catch (RecursionError e)`), também quando ela passa por uma
função da linguagem que chama a sua de volta — `map`, `filter`, um decorador,
um gerador que consome outro gerador dele mesmo:

```
funct f(x) {
    return map([x], f)
}
f(1)
RecursionError: maximum recursion depth exceeded
```

Dentro de uma `async funct` a pilha de cada task é menor, e esse limite chega
antes do que no programa principal.

---

## 6.7. Geradores — `yield`

Uma funct que usa `yield` (em vez de `return`) é um **gerador**: cada `yield`
entrega um valor e a execução pausa ali até o próximo pedido. O resultado é uma
sequência preguiçosa, consumível por `for each` ou materializável com `list(...)`:

```ps
funct conta() {
    yield 1
    yield 2
    yield 3
}

for each v in conta() {
    post(v)          # 1, 2, 3
}

post(list(conta()))  # [1, 2, 3]
```

Chamar um gerador **não executa nada**: devolve o gerador, e o corpo só roda no
primeiro pedido. Vale em qualquer forma de chamada — posicional, por nome,
espalhada (`g(*lista)`), como **método** (`obj.conta(2)`, ou `f = obj.conta` e
depois `f(2)`) e passada a uma função que chama de volta (`map([2], obj.conta)`
devolve uma lista de geradores).

---

## 6.8. Assíncrono — `async` / `await` / `gather`

Uma funct marcada `async` (`async funct`) devolve um **future** (uma promessa
do resultado) e **começa a rodar na chamada**: o corpo corre até o primeiro
ponto em que ele **cede** — `sleep`, I/O (banco, `request`, `os.run`), ou um
`await` de dentro — e aí o controle volta pra quem chamou. O valor sai com
`await` (espera um future) ou `gather` (espera vários). As tasks correm
**concorrentes** — enquanto uma espera I/O, as outras andam.

```ps
async funct t() {
    post("comecei")      # sai AGORA, na linha da chamada
    sleep(0.1)
    post("terminei")
}

f = t()
post("chamei")           # "comecei" já saiu antes desta linha
await f                  # aqui o resto do corpo roda
```

**A tarefa anda sempre que o programa principal espera.** Uma tarefa que cedeu
continua quando o principal para pra esperar alguma coisa:

- o `await`, o `gather` e o `post` de um future — esperam a tarefa;
- o `sleep` do principal — enquanto ele dorme, as tarefas andam;
- a I/O do principal (`request`, banco, `os.run`, sockets, e-mail) — enquanto
  ela não volta, as tarefas andam.

É a mesma regra dentro do `jinker`, onde o laço do servidor roda as tarefas
entre uma requisição e outra: o mesmo código se comporta igual num script e
no servidor.

```ps
async funct t() {
    sleep(0.1)
    post("tarefa")
}

f = t()
sleep(0.5)               # a tarefa termina durante este sono
post("principal")        # tarefa, principal
```

Enquanto o principal **roda** (sem esperar nada), as tarefas ficam paradas —
elas só andam nos pontos de espera. No fim do programa o motor **não** espera
tarefa pendente: se o resultado importa, `await` nela.

**Erro de tarefa não some.** Se a exceção acontece antes do primeiro ponto de
cedência, ela sobe na própria chamada, como em funct comum. Se acontece
depois e ninguém aguardou aquele future, a mensagem sai no **stderr** no fim
do programa, dizendo que era de uma tarefa sem `await`, e o programa
**termina com código de saída 1** — o mesmo de uma exceção não pega no
principal — mesmo que o resto tenha corrido até o fim. Erro é erro, também
pra quem só olha o código (`&&`, `set -e`, o CI). `sys.exit(n)` explícito
mantém o `n` que você escolheu; o erro é impresso do mesmo jeito. Em
qualquer caso o traceback termina na linha do `raise`, não na do `await`.

```ps
async funct dobro(n) {
    sleep(0.2)
    return n * 2
}

post(await dobro(21))                          # 42
post(gather(dobro(1), dobro(2), dobro(3)))     # [2, 4, 6] — os três em ~0.2s, não 0.6s
```

`await` também aceita uma **lista**: resolve os futures que estiverem dentro
dela, no lugar, e item que não é future passa direto.

```ps
async funct dobro(n) { return n * 2 }

fs = [dobro(1), dobro(2), dobro(3)]
post(await fs)              # [2, 4, 6]
post(await [dobro(1), 99])  # [2, 99]
post(await 5)               # 5 — valor comum devolve ele mesmo
```

`gather(fs)` com uma lista devolve **lista dentro de lista** (`[[2, 4, 6]]`):
cada argumento vira um elemento do resultado, e o argumento-lista vira a
sub-lista dos valores dele. Pra achatar, use `await fs`.

**Toda chamada devolve o future**, em qualquer forma: posicional, por nome
(`dobro(n=21)`), espalhada (`dobro(*l)`, `dobro(**d)`), como **método**
(`obj.m(1)`, ou `f = obj.m` e depois `f(1)`), `static` pela Entity, funct
aninhada que usa variável de fora (ela leva as variáveis junto) e passada a uma
função que chama de volta (`map([1, 2], dobro)` devolve a lista de futures —
`await` dela dá `[2, 4]`). Não há limite de argumentos. Os erros de chamada
(aridade, nome, tipo) saem **na chamada**, não no `await`:

```
async funct f(a) {
    return a
}
f(b=1)
TypeError: f() got an unexpected keyword argument 'b'
```

Um `__init__` não pode ser `async` nem gerador — a instanciação vale o objeto,
e não teria onde pôr o future: `TypeError: __init__() should return None, not 'future'`.

**Handler de framework roda o corpo no lugar.** A rota, o middleware e o
socket do `jinker`, e o `on_message` de um WebSocket de saída, **esperam** o
handler: uma `async funct` ali roda na fibra que atende a conexão, e vê a
requisição dela. Um helper `async` aguardado de dentro do handler
(`v = await ler_corpo()`) roda noutra fibra, que nasce com a mesma requisição —
o `request` lá dentro é o do handler.

Roda sobre **fibras** (*green-threads*): cada `async funct` vira uma fibra e o
escalonador as revessa; `sleep`, banco e requisições de saída cedem sozinhos. O
modelo é *stackful* (cada task tem pilha própria): escala bem até a casa das
**centenas** de tasks concorrentes, onde ganha do Node em tempo e memória.

---

## 6.9. Resumo

- **`funct`** define uma função (`type()` → `"funct"`). É a única palavra:
  `action` e `reaction` saíram e são erro de sintaxe.
- Parâmetros: posicionais + **padrão** (`b=10`); **nomeados** na chamada;
  **tipo antes do nome** (opcional); **variádicos** `*args` (tup) e `**kwarg`
  (dict); `f(*lista)` / `f(**dict)` espalham na chamada; aridade errada é erro;
  no máximo 256 parâmetros fixos, conferido na declaração.
- `return` sem valor / ausência de `return` → `null`.
- **`int funct` / `bool funct`**: nunca propagam erro (int→`500`, bool→`False`
  no erro, e o erro sai junto no stderr) e tratam `null` (int→`0`,
  bool→`True`); não coagem o valor retornado.
- **`static`** e **`nonnull`** vêm colados na cabeça, em qualquer ordem com os
  outros modificadores, e valem só para a funct em que estão escritos.
- Funções são **valores** (first-class); há **recursão** (sem fim, inclusive
  passando por `map`/`filter`/decorador, é `RecursionError`) e **geradores**
  (`yield`) — chamar um gerador, em qualquer forma, devolve o gerador.
- **`async`/`await`/`gather`** funcionam sobre fibras; as tasks correm
  concorrentes. Toda chamada de `async funct` devolve o future, em qualquer
  forma; handler do jinker e `on_message` rodam o corpo no lugar. `await` de uma
  **lista** resolve os futures de dentro dela.
