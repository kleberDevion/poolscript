# Referência da Linguagem — 6. Funções

Uma função agrupa um trecho de código sob um nome, recebe parâmetros e devolve
um valor. Na PoolScript ela se declara com **`action`** (ou o sinônimo
**`reaction`**). Esta seção cobre a definição, os parâmetros, o retorno, as
formas tipadas (`int action`/`bool action`), funções como valores, recursão e
geradores.

Como sempre, cada comportamento foi verificado rodando o mesmo fonte nos dois
motores.

---

## 6.1. Definição — `action` e `reaction`

![exemplo 1](../assets/linguagem__06-funcoes_ex1.png)

<details><summary>código</summary>

```ps
action soma(a, b):
    return a + b

post(soma(2, 3))     // 5
```

</details>

`reaction` é **sinônimo exato** de `action` — mesma sintaxe, mesmo
comportamento, e `type()` de qualquer uma devolve `"action"`. A escolha entre as
duas é só de intenção na leitura (por exemplo, `action` para um procedimento,
`reaction` para um callback/handler); mecanicamente não há diferença.

![exemplo 2](../assets/linguagem__06-funcoes_ex2.png)

<details><summary>código</summary>

```ps
reaction ao_clicar(x):
    post("clicou em", x)
```

</details>

Os dois estilos de bloco valem (`:` + indentação ou `{ }`).

---

## 6.2. Parâmetros

### 6.2.1. Posicionais e valores padrão

Parâmetros podem ter **valor padrão** (uma expressão, avaliada quando falta o
argumento):

![exemplo 3](../assets/linguagem__06-funcoes_ex3.png)

<details><summary>código</summary>

```ps
action g(a, b = 10):
    return a + b

post(g(5))       // 15   (b usa o padrão)
post(g(5, 1))    // 6
```

</details>

O padrão pode ser qualquer expressão: `action f(a, b = 5 * 2)` → `b` vale `10`.

### 6.2.2. Argumentos nomeados (na chamada)

Na chamada, um argumento pode ser passado pelo nome do parâmetro, em qualquer
ordem, e misturado com posicionais:

![exemplo 4](../assets/linguagem__06-funcoes_ex4.png)

<details><summary>código</summary>

```ps
action f(a, b, c):
    return str(a) + str(b) + str(c)

post(f(c=3, a=1, b=2))    // "123"
post(f(1, c=3, b=2))      // "123"  (posicional + nomeado)
```

</details>

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

![exemplo 5](../assets/linguagem__06-funcoes_ex5.png)

<details><summary>código</summary>

```ps
action nada():
    return
post(nada())     // null

action semret():
    x = 1
post(semret())   // null
```

</details>

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

![exemplo 6](../assets/linguagem__06-funcoes_ex6.png)

<details><summary>código</summary>

```ps
int action status():
    return null
post(status())        // 0

int action quebra():
    raise Boom("x")
post(quebra())        // 500  (erro engolido)
```

</details>

`bool action`: o retorno vira `bool` por *truthiness* — `return 0` → `False`,
`return 5` → `True`; **erro no corpo → `False`**; `return null`/sem return →
`True`.

> Diferente de um cast: `int action f(): return "7"` devolve a **string**
> `"7"`, não o inteiro `7`. O `int`/`bool` aqui rege o tratamento de
> ausência/erro, não uma conversão do valor retornado.

### Ordem dos modificadores é livre

Os prefixos de uma action/reaction — tipo de retorno (`int`/`bool`/`str`/`flo`),
`async` e visibilidade (`public`/`private`) — podem vir em **qualquer ordem**.
Todos abaixo são equivalentes e válidos (nos dois motores):

![exemplo 7](../assets/linguagem__06-funcoes_ex7.png)

<details><summary>código</summary>

```ps
int async reaction f():   ...
async int reaction f():   ...
public async reaction f(): ...
private int action f():   ...
```

</details>

---

## 6.5. Funções são valores (first-class)

Uma action pode ser guardada em variável, passada como argumento e devolvida —
sem os parênteses, o nome é a própria função:

![exemplo 8](../assets/linguagem__06-funcoes_ex8.png)

<details><summary>código</summary>

```ps
action dobro(n):
    return n * 2

g = dobro
post(g(21))          // 42

action aplica(fn, x):
    return fn(x)
post(aplica(dobro, 21))   // 42
```

</details>

`type()` de uma função é `"action"`.

---

## 6.6. Recursão

Uma action pode chamar a si mesma:

![exemplo 9](../assets/linguagem__06-funcoes_ex9.png)

<details><summary>código</summary>

```ps
action fatorial(n):
    if n <= 1:
        return 1
    return n * fatorial(n - 1)

post(fatorial(5))    // 120
```

</details>

---

## 6.7. Geradores — `yield`

Uma action que usa `yield` (em vez de `return`) é um **gerador**: cada `yield`
entrega um valor e a execução pausa ali até o próximo pedido. O resultado é uma
sequência preguiçosa, consumível por `for each` ou materializável com `list(...)`:

![exemplo 10](../assets/linguagem__06-funcoes_ex10.png)

<details><summary>código</summary>

```ps
action conta():
    yield 1
    yield 2
    yield 3

for each v in conta():
    post(v)          // 1, 2, 3

post(list(conta()))  // [1, 2, 3]
```

</details>

---

## 6.8. Assíncrono — `async` / `await` / `gather`

Uma action marcada `async` (`async action`, `async reaction`) **não roda na
chamada**: devolve um **future** (uma promessa do resultado). O valor sai com
`await` (espera um future) ou `gather` (espera vários). As tasks correm
**concorrentes** — enquanto uma espera I/O, as outras andam.

![exemplo 11](../assets/linguagem__06-funcoes_ex11.png)

<details><summary>código</summary>

```ps
async action dobro(n):
    sleep(0.2)
    return n * 2

post(await dobro(21))                          # 42
post(gather(dobro(1), dobro(2), dobro(3)))     # [2, 4, 6] — os três em ~0.2s, não 0.6s
```

</details>

`gather` também aceita uma **lista** de futures (`gather(fs)` → lista com os
valores na mesma ordem). `await` de um valor comum (não-future) devolve o próprio
valor.

Roda **nos dois motores**: no interpretador (sobre um executor de threads) e na
**VM em C** (sobre fibras/*green-threads* — cada `async action` vira uma fibra e o
escalonador as revessa; `sleep`, banco e requisições de saída cedem sozinhos). O
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
- **`async`/`await`/`gather`** funcionam **nos dois motores** (interpretador e
  VM); as tasks correm concorrentes.
