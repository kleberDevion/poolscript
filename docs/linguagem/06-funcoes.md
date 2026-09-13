# Referência da Linguagem — 6. Funções

Uma função agrupa um trecho de código sob um nome, recebe parâmetros e devolve
um valor. Na PoolScript ela se declara com **`funct`**. Esta seção cobre a
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
saudacao(5, 3)
# AttributedValueError: parâmetro nome de saudacao() esperava str, recebeu int
```

A mesma regra vale para os `int`/`flo` que se parecem: `int` não aceita `flo`,
`flo` não aceita `int`, `bool` não aceita `int`. Para passar outro tipo,
converta você: `saudacao(str(5), 3)`.

Numa **Entity**, o tipo aceita subclasse onde a mãe foi pedida:

```ps
Entity Animal() { str nome }
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

Também **não há parâmetro variádico** (`*args` / `**kwargs` não existem). O
número de parâmetros é fixo (fora os que têm padrão).

### 6.2.4. Aridade

Passar argumentos **de menos** (sem cobrir um parâmetro sem padrão) ou **de
mais** é erro em tempo de execução.

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

Prefixar a funct com `int` ou `bool` muda o **contrato de retorno**: a função
passa a **nunca propagar erro** (o corpo vira um `try` implícito) e a garantir
um resultado do feitio pedido. É pensado para handlers que precisam sempre
devolver algo (por exemplo, um status).

`int funct`:

| Situação | Devolve |
|---|---|
| `return <int>` | o próprio inteiro |
| `return null` / sem return | `0` |
| erro/exceção no corpo | `500` |
| `return <outro tipo>` | passa como está (não é coagido) |

```ps
int funct status() {
    return null
}
post(status())        # 0

int funct quebra() {
    raise Boom("x")
}
post(quebra())        # 500  (erro engolido)
```

`bool funct`: o retorno vira `bool` por *truthiness* — `return 0` → `False`,
`return 5` → `True`; **erro no corpo → `False`**; `return null`/sem return →
`True`.

> Diferente de um cast: `int funct f()` com `return "7"` devolve a **string**
> `"7"`, não o inteiro `7`. O `int`/`bool` aqui rege o tratamento de
> ausência/erro, não uma conversão do valor retornado.

### Qualquer tipo vale como retorno

Não há lista branca. O que vem colado antes do `funct`, depois dos
modificadores, **é** o tipo de retorno — qualquer tipo da linguagem
(`str`, `int`, `long`, `flo`, `bool`, `char`, `list`, `dict`, `tup`, `json`,
`Object`) e também o nome de uma classe sua:

```ps
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

> **Só `int` e `bool` mudam o comportamento** — o contrato de erro descrito
> acima. Os outros hoje são só a declaração: a funct devolve o que devolver,
> sem conversão e sem tratamento de erro.

Isto vale em qualquer posição — solta, dentro de `Entity`/`class` e em lambda —
e os modificadores vêm em qualquer ordem:

```ps
public class C() {
    public static string funct main() {
        sys.stdout.writeln("Ola mundo!")
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
Entity Mat() {
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

---

## 6.8. Assíncrono — `async` / `await` / `gather`

Uma funct marcada `async` (`async funct`) **não roda na chamada**: devolve um
**future** (uma promessa do resultado). O valor sai com
`await` (espera um future) ou `gather` (espera vários). As tasks correm
**concorrentes** — enquanto uma espera I/O, as outras andam.

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
fs = [dobro(1), dobro(2), dobro(3)]
post(await fs)              # [2, 4, 6]
post(await [dobro(1), 99])  # [2, 99]
post(await 5)               # 5 — valor comum devolve ele mesmo
```

`gather(fs)` com uma lista devolve **lista dentro de lista** (`[[2, 4, 6]]`):
cada argumento vira um elemento do resultado, e o argumento-lista vira a
sub-lista dos valores dele. Pra achatar, use `await fs`.

Roda sobre **fibras** (*green-threads*): cada `async funct` vira uma fibra e o
escalonador as revessa; `sleep`, banco e requisições de saída cedem sozinhos. O
modelo é *stackful* (cada task tem pilha própria): escala bem até a casa das
**centenas** de tasks concorrentes, onde ganha do Node em tempo e memória.

---

## 6.9. Resumo

- **`funct`** define uma função (`type()` → `"funct"`). É a única palavra:
  `action` e `reaction` saíram e são erro de sintaxe.
- Parâmetros: posicionais + **padrão** (`b=10`); **nomeados** na chamada; **sem
  tipo**, **sem variádico**; aridade errada é erro.
- `return` sem valor / ausência de `return` → `null`.
- **`int funct` / `bool funct`**: nunca propagam erro (int→`500`, bool→`False`
  no erro) e tratam `null` (int→`0`, bool→`True`); não coagem o valor retornado.
- **`static`** e **`nonnull`** vêm colados na cabeça, em qualquer ordem com os
  outros modificadores, e valem só para a funct em que estão escritos.
- Funções são **valores** (first-class); há **recursão** e **geradores**
  (`yield`).
- **`async`/`await`/`gather`** funcionam sobre fibras; as tasks correm
  concorrentes. `await` de uma **lista** resolve os futures de dentro dela.
