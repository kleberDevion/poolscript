# Referência da Linguagem — 5. Controle de fluxo

Statements que decidem **o que roda e quantas vezes**: condicionais (`if`),
laços (`while`, `for each`, `count each`), o casamento de padrões (`match`), os
desvios `break`/`continue`/`pass` e o guard de entrada `if __name__ == "main"`.

O bloco da linguagem é `{ }` (seção 1.3) — e só. Toda regra desta seção foi
verificada rodando o fonte na VM em C.

Lembrete de escopo (seção 4): cada bloco aqui é um **escopo próprio** —
variável nova dentro dele não vaza pra fora, e o corpo de um laço reinicia a
cada iteração.

---

## 5.1. Condicional — `if` / `elif` / `else`

```ps
if nota >= 7 {
    post("aprovado")
} elif nota >= 5 {
    post("recuperação")
} else {
    post("reprovado")
}
```

- A condição é avaliada pela **verdade** do valor (*truthiness*, seção 2.3), não
  precisa ser `bool`: `0`, `""`, `[]`, `{}` e `null` são falsos; o resto é
  verdadeiro.
- `elif` encadeia quantas vezes quiser; `else` é opcional e vem por último.
- A palavra é **`elif`** — não existe `else if` (é erro de sintaxe).

```ps
if 5       { post("entra") }      # int não-zero é verdadeiro
if []      { post("não entra") }  # lista vazia é falsa
if "texto" { post("entra") }      # string não-vazia é verdadeira
```

Para escolher um **valor** (em vez de statements), use a expressão condicional
`A if cond else B` (seção 3.9).

---

## 5.2. Laço `while`

Repete o corpo enquanto a condição for verdadeira (mesma regra de verdade do
`if`).

```ps
n = 0
while n < 3 {
    post(n)
    n += 1
}
```

Não existe `while ... else` (é erro de sintaxe). A variável de controle
(`n` acima) precisa existir **fora** do laço para sobreviver entre as iterações
— uma variável criada só no corpo é reiniciada a cada volta (seção 4.6.3).

---

## 5.3. Laço `for each`

Itera sobre os elementos de uma sequência, ligando **uma** variável por
elemento:

```ps
for each x in [10, 20, 30] {
    post(x)
}

for each ch in "abc" {       # string: um caractere por vez
    post(ch)
}
```

Regras e limites (verificados):

- Aceita **lista, tupla e string**. **Não** itera `dict` diretamente
  (`TypeError: 'dict' object is not iterable`) — para percorrer um dict, use
  `d.keys()`, `d.values()` ou `d.items()`:

  ```ps
  d = { "a": 1, "b": 2 }
  for each k in d.keys() {
      post(k, d[k])
  }
  ```

- A variável do laço é **um único nome**. Não há forma com índice embutido nem
  desempacotamento no cabeçalho: `for each i, x in ...` é erro. Se cada elemento
  é uma tupla, ele chega inteiro na variável (desempacote no corpo, ou itere
  índices com `range`).
- A variável do laço **não existe depois** do laço (seção 4.6.3).

### 5.3.1. `range` — sequência de inteiros

`range` gera os inteiros para contar num `for each`. Tem três formas (iguais às
do Python; o fim é **exclusivo**):

| Forma | Gera |
|---|---|
| `range(fim)` | `0, 1, …, fim-1` |
| `range(início, fim)` | `início, …, fim-1` |
| `range(início, fim, passo)` | de `passo` em `passo`; `passo` negativo conta pra trás |

```ps
for each i in range(3) {          # 0 1 2
    post(i)
}
for each i in range(2, 5) {       # 2 3 4
    post(i)
}
for each i in range(10, 0, -2) {  # 10 8 6 4 2
    post(i)
}
```

---

### 5.3.2. Compreensão de lista

Montar uma lista a partir de outra sem escrever o laço:

```ps
nums = [1, 2, 3, 4]
post([n * 2 for each n in nums])        # [2, 4, 6, 8]
```

A forma é a do Python, escrita com o `for each` da linguagem:

```
[ <expressão> for each <nome> in <iterável> ]
[ <expressão> for each <nome> in <iterável> if <condição> ]
```

Com filtro, só entra na lista o elemento cuja condição é verdadeira:

```ps
post([x for each x in [1, 2, 3, 4] if x % 2 == 0])   # [2, 4]
```

Vale sobre qualquer coisa que o `for each` aceita — lista, tupla, string e
`range`:

```ps
post([c.upper() for each c in "abc"])   # ['A', 'B', 'C']
post([i * i for each i in range(5)])    # [0, 1, 4, 9, 16]
```

E pode aninhar:

```ps
post([[y for each y in range(2)] for each z in range(3)])
# [[0, 1], [0, 1], [0, 1]]
```

A variável da compreensão **sombreia**, como a do `for each`: uma de mesmo
nome que exista fora continua valendo depois (seção 4.6.3).

```ps
n = "de fora"
post([n for each n in [1, 2]], n)       # [1, 2] de fora
```

Como **argumento único** de uma chamada, os colchetes são dispensáveis — é a
forma curta do Python, e vale em qualquer função:

```ps
post(n * 2 for each n in nums)          # [2, 4, 6, 8]
post(sum(v for each v in nums))         # 10
post(max(v for each v in nums))         # 4
```

Com mais de um argumento fica ambíguo (não dá pra saber se a compreensão é um
argumento ou se falta um `)`), então aí os colchetes voltam a ser obrigatórios
— e o erro diz isso:

```ps
post("a", n for each n in [1])      # SyntaxError: compreensao so vale como argumento unico — ponha entre colchetes: [x for each ...]
post("a", [n for each n in [1]])    # a [1]
```

Só existe a de **lista**: não há compreensão de dict nem de conjunto.

---

## 5.4. `break` e `continue`

Dentro de um laço (`while`, `for each`, `count each`):

- **`break`** encerra o laço imediatamente.
- **`continue`** pula pro próximo elemento/iteração.

Ambos agem sobre o laço **mais interno**. Fora de um laço são erro de
compilação (`'break' fora de laco`).

```ps
for each n in range(100) {
    if n == 5 {
        break            # para no 5
    }
    if n % 2 == 0 {
        continue         # pula os pares
    }
    post(n)              # 1 3
}
```

---

## 5.4.1. `pass` — o statement que não faz nada

`pass` é um **no-op**, igual ao do Python: existe só pra ocupar o lugar de um
statement onde a linguagem exige um corpo, mas você não tem nada a fazer ali.
Não gera bytecode nenhum.

```ps
action ainda_nao() {
    pass                 # corpo vazio, sem erro
}

if x < 0 {
    pass                 # esse caso é ignorado de propósito
} else {
    post("positivo")
}

for each n in range(3) {
    pass
}
```

Vale em **qualquer** posição de statement — corpo de `action`, `if`/`else`,
`while`, `for each`, `try`/`catch`, corpo de classe. Diferente de `break` e
`continue`, não depende de estar dentro de um laço.

Uma action cujo corpo é só `pass` devolve `null`, igual a uma que termina sem
`return` (seção 6.3).

> Uso típico fora de corpo vazio: no **middleware** do `jinker`, chegar no
> `pass` significa "não barrei" e a requisição segue pra rota
> (`docs/jinker/middleware/middleware.md`).

---

## 5.5. `match` / `case`

Compara um valor (o *sujeito*) contra uma série de **padrões**, na ordem, e roda
o bloco do primeiro que casar.

```ps
match comando {
    case "oi" {
        post("olá")
    }
    case "sair" {
        post("tchau")
    }
    case _ {
        post("comando desconhecido:", comando)
    }
}
```

Padrões suportados (verificados):

| Padrão | Casa quando | Liga |
|---|---|---|
| Literal — `case 2 { }` / `case "x" { }` | o sujeito é igual àquele valor | — |
| Captura — `case x { }` | sempre (pega qualquer valor) | `x` = o sujeito |
| Curinga — `case _ { }` | sempre (não liga nome) | — |
| Alternativa — `case 1 \| 2 \| 3 { }` | casa com qualquer um dos valores | — |
| Lista — `case [a, b] { }` | o sujeito é uma lista com essa forma | `a`, `b` = os elementos |
| Guarda — `case x if x > 5 { }` | o padrão casa **e** a condição é verdadeira | conforme o padrão |

```ps
match ponto {
    case [0, 0] {
        post("origem")
    }
    case [x, 0] {
        post("no eixo X, em", x)
    }
    case [x, y] if x == y {
        post("na diagonal")
    }
    case _ {
        post("outro lugar")
    }
}
```

- **Nenhum casou:** o `match` simplesmente não faz nada (não é erro). Um
  `case _ { }` no fim funciona como "senão".
- Os nomes ligados por um `case` (`x`, `a`, `b`…) são **do bloco daquele case** —
  não existem depois do `match` (seção 4.6.1).

---

## 5.6. `count each` — laço sobre os elementos de um tipo

`count each <Tipo> in <container> { }` percorre **apenas os elementos daquele tipo**
dentro do container, e já entrega a contagem total. Dentro do bloco:

| Nome | Valor |
|---|---|
| `_match` | o elemento atual (que é do tipo pedido) |
| `_index` | a posição dele **no container original** |
| `self` / `_count` | o total de elementos daquele tipo (não muda durante o laço) |

```ps
count each int in [10, "x", 20, 30] {
    post("achei", _match, "na posição", _index, "de", _count)
}
# achei 10 na posição 0 de 3
# achei 20 na posição 2 de 3
# achei 30 na posição 3 de 3
```

Os elementos que não são do tipo (o `"x"` acima) são pulados. `self`, `_count`,
`_index` e `_match` são do escopo do laço — não existem depois dele.

(Existe também a forma de **expressão** do `count`, que só devolve a contagem
sem laço — `int(2) count in xs` e `int in xs count` — na seção 3.10.)

---

## 5.7. `if __name__ == "main"` — código só quando é o principal

O bloco `if __name__ == "main"` roda **apenas quando o arquivo é executado
direto**, e é pulado quando ele é **importado** por outro. É o
`if __name__ == "__main__":` do Python — o lugar do ponto de entrada.

Ele é reconhecido pela **forma**, não avaliando a condição: `__name__` vale o
caminho do arquivo (é o que se passa pro `Jinker`, por exemplo), então comparar
com `"main"` nunca daria verdadeiro por conta própria. O compilador vê o
desenho e emite o guard.

É também o **único** lugar da linguagem onde `:` ainda abre bloco — `{ }` vale
igual, e a chave pode ficar na linha seguinte:

```ps
if __name__ == "main":
    principal()
```

```ps
action principal() {
    post("rodando o app")
}

if __name__ == "main" {
    principal()
}
```

Assim, `import` desse arquivo traz a `action principal` sem disparar o
`principal()`. Como os outros blocos, é um escopo próprio (variáveis criadas
dentro não vazam).

---

## 5.8. Resumo

- **`if` / `elif` / `else`** — condição por *truthiness*; `elif` (não `else if`).
- **`while`** — repete enquanto verdadeiro; sem `while/else`.
- **`for each x in seq`** — lista/tupla/string (não dict direto: use `.keys()`
  etc.); uma variável só; `range(...)` pra contar.
- **`break` / `continue`** — no laço mais interno.
- **`pass`** — no-op; ocupa o lugar de um corpo vazio, em qualquer posição de
  statement (não precisa de laço).
- **`match` / `case`** — literal, captura, `_`, `|`, lista, guarda; sem casar =
  no-op; ligações são do case.
- **`count each Tipo in c`** — laço sobre os elementos do tipo (`_match`,
  `_index`, `self`/`_count`).
- **`if __name__ == "main"`** — só quando principal, pulado no import.
