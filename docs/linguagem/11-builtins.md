# Referência da Linguagem — 11. Builtins

Os **builtins** são as funções sempre disponíveis, **sem `import`**. São **35**
no total. Esta seção é a visão geral; cada builtin tem uma página detalhada em
[`docs/builtins/`](../builtins/builtins.md), e os exemplos de lá **rodam de
verdade** na suíte em C (`make check`) — doc errada quebra o teste.

Tudo aqui foi verificado rodando o fonte na VM em C.

---

## 11.1. Entrada e saída

| Builtin | Assinatura | O que faz |
|---|---|---|
| `post` | `post(v1, v2, …)` | imprime os valores no stdout, separados por espaço, com quebra de linha; `post()` sem args é uma linha em branco vazia (na verdade nem imprime a quebra). `null` sai como `null`, dict como `{'k': v}`, bool como `True`/`False`. Devolve `null`. |
| `input` | `input(prompt=null)` | lê uma linha do stdin; o `prompt` (opcional) é impresso antes, sem quebra. **Devolve sempre `str`** — pra número, use `int(...)` ou declare o tipo. |
| `open` | `open(caminho, modo="r")` | abre um arquivo e devolve o handle (`read`/`readline`/`readlines`/`write`/`writelines`/`close`). Combine com `using` pra fechar sozinho. Arquivo inexistente em leitura → `IOError`. |
| `load` | `load(caminho=null)` | carrega variáveis de um `.env` pro ambiente e **devolve um dict** com o que leu (`{ "CHAVE": "valor" }`); sem `.env`, dict vazio. Não sobrescreve variável já definida. Atalho de `dotenv.load`. |

---

## 11.2. Núcleo

| Builtin | Assinatura | O que faz |
|---|---|---|
| `len` | `len(x)` | tamanho de `str` (em **caracteres**, não bytes: `len("olá")`→3), `list`, `tup`, `dict` ou `bytes`. `len(null)` → `0`. |
| `type` | `type(x)` | nome do tipo como `str` (`"int"`, `"str"`, `"list"`, `"Null"`, nome da Entity, `"action"`, `"type"`, `"generator"`). Igual ao método `x.type()`. |
| `range` | `range(fim)` / `range(início, fim, passo=1)` | **lista** concreta de inteiros, `início` (inclusive) a `fim` (exclusive), de `passo` em `passo` (negativo conta pra trás). Aceita string numérica (`range("3")`) e trunca float. `passo=0` → erro. |

---

## 11.3. Conversão de tipo

| Builtin | Assinatura | O que faz |
|---|---|---|
| `str` | `str(x)` | qualquer valor → texto de renderização (o mesmo que `post` imprime): `null`→`"null"`, `true`→`"True"`; listas/dicts recursivos. |
| `int` | `int(x)` | → inteiro: string decimal, `flo` (**trunca** pra zero: `int(3.9)`→3), `bool`. `int()` sem argumento → `0`. String não-numérica → **`ValueError`** (`invalid literal for int() with base 10: 'abc'`) — o tipo está certo, o valor é que não serve; o `TypeError` que esta linha dizia nunca aconteceu. Não confundir com o `ConversionError` da **declaração** (`int z = "abc"`), que é outro caminho. |
| `flo` | `flo(x)` | → ponto flutuante: string numérica, `int` (`3`→`3.0`), `bool`. Lixo no fim da string é recusado (não converte "meio"). |
| `bool` | `bool(x)` | verdade do valor: `0`, `0.0`, `""`, `[]`, `{}`, `null` são `False`; o resto `True`. |
| `list` | `list(x)` | materializa em lista: `str`→caracteres (por codepoint), `dict`→chaves, `tup`→lista, `range`/gerador→drenados. `list()` sem arg → `[]`. |

(As regras completas de coerção, e a diferença entre conversão e declaração
tipada, estão na seção 2.)

---

## 11.4. Números

| Builtin | Assinatura | O que faz |
|---|---|---|
| `abs` | `abs(n)` | valor absoluto (aceita `int`/`flo`/`bool`). |
| `round` | `round(n, casas=0)` | arredonda (meio-para-par: `round(2.5)`→2). 1 arg sobre `flo` → `int`; com `casas` → `flo`. `casas` negativas são **clampadas a 0**. |
| `hex` | `hex(n)` | inteiro → `"0xff"` (sinal antes do prefixo: `hex(-255)`→`"-0xff"`). |
| `bin` | `bin(n)` | inteiro → `"0b101"`. |
| `oct` | `oct(n)` | inteiro → `"0o17"`. |
| `ord` | `ord(c)` | 1 caractere → codepoint Unicode (`ord("ç")`→231, por codepoint, não byte). |
| `chr` | `chr(n)` | codepoint (0..0x10FFFF) → caractere; aceita `bool` como int. |
| `sum` | `sum(lista, start=0)` | soma os números de uma lista/tupla; `start` opcional. Vazia → `0`. |
| `min` | `min(lista)` / `min(a, b, …)` | menor valor (1 iterável, ou vários argumentos). |
| `max` | `max(lista)` / `max(a, b, …)` | maior valor. |

---

## 11.5. Sequências

| Builtin | Assinatura | O que faz |
|---|---|---|
| `sorted` | `sorted(lista)` | nova lista em ordem crescente (a original não muda); estável. |
| `reversed` | `reversed(lista)` | nova lista na ordem inversa (devolve **lista**, não iterador). |
| `enumerate` | `enumerate(lista)` | lista de tuplas `(índice, item)`, índice a partir de 0. |
| `zip` | `zip(a, b, …)` | lista de tuplas pareando os iteráveis posição a posição; **para no menor**. |
| `map` | `map(lista, fn)` | nova lista com `fn` aplicada a cada item. **A lista vem primeiro** (ao contrário do Python). |
| `filter` | `filter(lista, fn)` | nova lista só com os itens em que `fn` é verdadeiro. **A lista vem primeiro.** |

```ps
action dobro(x) {
    return x * 2
}
action par(x) {
    return x % 2 == 0
}

post(map([1, 2, 3], dobro))        # [2, 4, 6]
post(filter([1, 2, 3, 4], par))    # [2, 4]
```

---

## 11.6. Mutação de lista (in-place)

Estes **alteram a própria lista** e por isso não encadeiam (devolvem o item ou `null`):

| Builtin | Assinatura | O que faz |
|---|---|---|
| `addEnd` | `addEnd(lista, item)` | anexa `item` no fim; devolve `null`. |
| `addStart` | `addStart(lista, item)` | insere `item` no início (desloca o resto); devolve `null`. |
| `removeEnd` | `removeEnd(lista)` | remove e devolve o **último** item; lista vazia → `null` (não é erro). |
| `removeStart` | `removeStart(lista)` | remove e devolve o **primeiro** item; lista vazia → `null`. |

```ps
l = [1, 2, 3]
addEnd(l, 4)          # l == [1, 2, 3, 4]
post(removeStart(l))  # 1   (e l == [2, 3, 4])
```

---

## 11.7. Avançados

| Builtin | Assinatura | O que faz |
|---|---|---|
| `gather` | `gather(a, b, …)` | espera vários `async action` **concorrentes** e devolve os valores numa lista (na ordem). Valor comum passa direto. Aceita lista de futures. |
| `sleep` | `sleep(segundos)` | pausa a execução pelo tempo dado (aceita fração). Devolve `null`. |
| `id` | `id(x)` | identidade do valor como `int`. Para objetos, o endereço; para imediatos, o conteúdo bruto. O número em si não é estável entre execuções — use só pra comparar identidade. |

---

## 11.8. Resumo

- **35 builtins**, sempre disponíveis, sem `import`. Página detalhada de cada um
  em `docs/builtins/`.
- **I/O**: `post`, `input`, `open`, `load`.
- **Núcleo**: `len` (conta caracteres; `len(null)`→0), `type`, `range`.
- **Conversão**: `str`, `int`, `flo`, `bool`, `list` (ver seção 2).
- **Número**: `abs`, `round`, `hex`, `bin`, `oct`, `ord`, `chr`, `sum`, `min`,
  `max`.
- **Sequência**: `sorted`, `reversed`, `enumerate`, `zip`, `map`/`filter` (**a
  lista vem primeiro**).
- **Mutação de lista**: `addEnd`, `addStart`, `removeEnd`, `removeStart`.
- **Avançado**: `gather`, `sleep`, `id`.
