# Referência da Linguagem — 6. Funções

Uma função agrupa um trecho de código sob um nome, recebe parâmetros e devolve
um valor. Na PoolScript ela se declara com **`action`** (ou o sinônimo
**`reaction`**). Esta seção cobre a definição, os parâmetros, o retorno, as
formas tipadas (`int action`/`bool action`), funções como valores, recursão e
geradores.

Como sempre, cada comportamento foi verificado rodando o fonte de verdade.

---

## 6.1. Definição — `action` e `reaction`

```ps
action soma(a, b) {
    return a + b
}

post(soma(2, 3))     # 5
```

`reaction` é **sinônimo exato** de `action` — mesma sintaxe, mesmo
comportamento, e `type()` de qualquer uma devolve `"action"`. A escolha entre as
duas é só de intenção na leitura (por exemplo, `action` para um procedimento,
`reaction` para um callback/handler); mecanicamente não há diferença.

```ps
reaction ao_clicar(x) {
    post("clicou em", x)
}
```

O corpo é um bloco `{ }`, como todo bloco da linguagem (seção 1.3).

---

## 6.2. Parâmetros

### 6.2.1. Posicionais e valores padrão

Parâmetros podem ter **valor padrão** (uma expressão, avaliada quando falta o
argumento):

```ps
action g(a, b = 10) {
    return a + b
}

post(g(5))       # 15   (b usa o padrão)
post(g(5, 1))    # 6
```

O padrão pode ser qualquer expressão: `action f(a, b = 5 * 2)` → `b` vale `10`.

### 6.2.2. Argumentos nomeados (na chamada)

Na chamada, um argumento pode ser passado pelo nome do parâmetro, em qualquer
ordem, e misturado com posicionais:

```ps
action f(a, b, c) {
    return str(a) + str(b) + str(c)
}

post(f(c=3, a=1, b=2))    # "123"
post(f(1, c=3, b=2))      # "123"  (posicional + nomeado)
```

### 6.2.3. Parâmetros não têm tipo

Os parâmetros são **sempre nomes simples** — não se declara tipo neles. Nem
`action f(int x)` (o `int` é palavra reservada) nem `action f(x: int)` são
válidos. A tipagem estática da linguagem fica nas **declarações de variável**
(seção 4.1) e nas formas tipadas de action abaixo.

Também **não há parâmetro variádico** (`*args` / `**kwargs` não existem). O
número de parâmetros é fixo (fora os que têm padrão).

### 6.2.4. Aridade

Passar argumentos **de menos** (sem cobrir um parâmetro sem padrão) ou **de
mais** é erro em tempo de execução.

---

## 6.3. Retorno

`return <expr>` devolve um valor e encerra a função. Um `return` **sem valor**,
ou uma função que **termina sem `return`**, devolve `null`:

```ps
action nada() {
    return
}
post(nada())     # null

action semret() {
    x = 1
}
post(semret())   # null
```

---

## 6.4. Actions tipadas — `int action` / `bool action`

Prefixar a action com `int` ou `bool` muda o **contrato de retorno**: a função
passa a **nunca propagar erro** (o corpo vira um `try` implícito) e a garantir
um resultado do feitio pedido. É pensado para handlers que precisam sempre
devolver algo (por exemplo, um status).

`int action`:

| Situação | Devolve |
|---|---|
| `return <int>` | o próprio inteiro |
| `return null` / sem return | `0` |
| erro/exceção no corpo | `500` |
| `return <outro tipo>` | passa como está (não é coagido) |

```ps
int action status() {
    return null
}
post(status())        # 0

int action quebra() {
    raise Boom("x")
}
post(quebra())        # 500  (erro engolido)
```

`bool action`: o retorno vira `bool` por *truthiness* — `return 0` → `False`,
`return 5` → `True`; **erro no corpo → `False`**; `return null`/sem return →
`True`.

> Diferente de um cast: `int action f(): return "7"` devolve a **string**
> `"7"`, não o inteiro `7`. O `int`/`bool` aqui rege o tratamento de
> ausência/erro, não uma conversão do valor retornado.

### Ordem dos modificadores é livre

Os prefixos de uma action/reaction — tipo de retorno (`int`/`bool`/`str`/`flo`),
`async` e visibilidade (`public`/`private`) — podem vir em **qualquer ordem**.
Todos abaixo são equivalentes e válidos:

```ps
int async reaction f() { }
async int reaction f() { }
public async reaction f() { }
private int action f() { }
```

Vale igual com um [decorador](14-decoradores.md) em cima: `@app.post("/x")`
seguido de `int async action h()` registra `h` como qualquer outra ordem.

Esquecer o `action` (ou `reaction`) é erro de sintaxe, e a mensagem devolve a
linha montada: `int async f(x)` dá
`faltou 'action' (ou 'reaction') antes de 'f': int async action f(...)`.

---

## 6.5. Funções são valores (first-class)

Uma action pode ser guardada em variável, passada como argumento e devolvida —
sem os parênteses, o nome é a própria função:

```ps
action dobro(n) {
    return n * 2
}

g = dobro
post(g(21))          # 42

action aplica(fn, x) {
    return fn(x)
}
post(aplica(dobro, 21))   # 42
```

`type()` de uma função é `"action"`.

---

## 6.6. Recursão

Uma action pode chamar a si mesma:

```ps
action fatorial(n) {
    if n <= 1 {
        return 1
    }
    return n * fatorial(n - 1)
}

post(fatorial(5))    # 120
```

---

## 6.7. Geradores — `yield`

Uma action que usa `yield` (em vez de `return`) é um **gerador**: cada `yield`
entrega um valor e a execução pausa ali até o próximo pedido. O resultado é uma
sequência preguiçosa, consumível por `for each` ou materializável com `list(...)`:

```ps
action conta() {
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

Uma action marcada `async` (`async action`, `async reaction`) **não roda na
chamada**: devolve um **future** (uma promessa do resultado). O valor sai com
`await` (espera um future) ou `gather` (espera vários). As tasks correm
**concorrentes** — enquanto uma espera I/O, as outras andam.

```ps
async action dobro(n) {
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

Roda sobre **fibras** (*green-threads*): cada `async action` vira uma fibra e o
escalonador as revessa; `sleep`, banco e requisições de saída cedem sozinhos. O
modelo é *stackful* (cada task tem pilha própria): escala bem até a casa das
**centenas** de tasks concorrentes, onde ganha do Node em tempo e memória.

---

## 6.9. Resumo

- **`action` / `reaction`** definem funções e são sinônimos (`type()` →
  `"action"`).
- Parâmetros: posicionais + **padrão** (`b=10`); **nomeados** na chamada; **sem
  tipo**, **sem variádico**; aridade errada é erro.
- `return` sem valor / ausência de `return` → `null`.
- **`int action` / `bool action`**: nunca propagam erro (int→`500`, bool→`False`
  no erro) e tratam `null` (int→`0`, bool→`True`); não coagem o valor retornado.
- Funções são **valores** (first-class); há **recursão** e **geradores**
  (`yield`).
- **`async`/`await`/`gather`** funcionam sobre fibras; as tasks correm
  concorrentes. `await` de uma **lista** resolve os futures de dentro dela.
