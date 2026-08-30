# Referência da Linguagem — 3. Operadores e expressões

Uma **expressão** é qualquer trecho que o motor avalia até um valor: um literal,
um nome, uma chamada, ou combinações disso por **operadores**. Esta seção
especifica todos os operadores da linguagem — o que cada um faz, sobre que
tipos, o que devolve, quando dá erro — e as regras que governam como uma
expressão maior é montada a partir das menores: **precedência**,
**associatividade** e **curto-circuito**.

Cada
comportamento desta seção foi verificado rodando o mesmo fonte nos dois e
comparando a saída.

---

## 3.1. Precedência e associatividade

A tabela abaixo vai do **mais fraco** (liga por último, no topo da árvore) ao
**mais forte** (liga primeiro). Operadores na mesma linha têm a mesma
precedência e são resolvidos pela associatividade indicada.

| # | Categoria | Operadores | Assoc. |
|--:|---|---|---|
| 1 | Condicional (ternário) | `A if C else B` | à direita |
| 2 | OU lógico | `or` &nbsp; `\|\|` | à esquerda |
| 3 | E lógico | `and` &nbsp; `&&` | à esquerda |
| 4 | Negação lógica | `not` &nbsp; `Not` &nbsp; `!` | prefixa (à direita) |
| 5 | Comparação, pertinência, tipo, contagem | `==` `!=` `<` `>` `<=` `>=` &nbsp; `is` `is not` &nbsp; `in` `not in` &nbsp; `count` | à esquerda |
| 6 | OU bit a bit | `\|` | à esquerda |
| 7 | XOR bit a bit | `^` | à esquerda |
| 8 | E bit a bit | `&` | à esquerda |
| 9 | Deslocamento | `<<` &nbsp; `>>` | à esquerda |
| 10 | Adição / subtração | `+` &nbsp; `-` | à esquerda |
| 11 | Multiplicação / divisão / módulo | `*` &nbsp; `/` &nbsp; `%` | à esquerda |
| 12 | Unários | `+` &nbsp; `-` &nbsp; `~` &nbsp; `await` | prefixa (à direita) |
| 13 | Potência | `**` | **à direita** |
| 14 | Pós-fixados | chamada `()` &nbsp; membro `.x` &nbsp; índice/fatia `[…]` &nbsp; `++` `--` | à esquerda |
| 15 | Primários | literais, nomes, `(…)`, `[…]`, `{…}` | — |

Exemplos verificados:

```ps
post(2 + 3 * 4)      # 14   — `*` (11) antes de `+` (10)
post(1 | 2 & 3)      # 3    — `&` (8) antes de `|` (6): 1 | (2 & 3)
post(1 + 2 << 3)     # 24   — `+` (10) antes de `<<` (9): (1 + 2) << 3
post(not 1 == 1)     # False — `==` (5) antes de `not` (4): not (1 == 1)
```

> **`not`/`!` é mais fraco que a comparação** (nível 4 < nível 5), como no
> Python. `not a == b` é `not (a == b)`, nunca `(not a) == b`. Para negar só o
> operando, use parênteses: `(not a) == b`.

Use **parênteses** `(…)` sempre que quiser forçar uma ordem diferente da tabela
— eles são o nível primário (15) e vencem tudo.

---

## 3.2. Operadores aritméticos

Operam sobre `int`, `flo` e `bool` (bool conta como `0`/`1`). O resultado segue
a **torre numérica** (ver seção 2): se qualquer operando é `flo`, o resultado é
`flo`; caso contrário é `int` (com promoção automática a bignum se estourar 64
bits).

| Op | Nome | Sobre |
|---|---|---|
| `+` | soma / concatenação | números; **também** `str`+`str`, `list`+`list`, `tup`+`tup` |
| `-` | subtração | números |
| `*` | multiplicação / repetição | números; **também** sequência*`int` (`list`, `tup`, `str`) |
| `/` | divisão | números — **sempre** verdadeira (resultado `flo`) |
| `//` | divisão inteira | números — quociente com **piso** (resultado `int` entre inteiros) |
| `%` | módulo (resto) | números |
| `**` | potência | números — associa à **direita** |

### 3.2.1. Divisão é sempre real

`/` **nunca** trunca: o resultado é `flo`, mesmo quando divide exato.

```ps
post(7 / 2)     # 3.5
post(10 / 5)    # 2.0   — não é 2 (int); é flo
```

Para o quociente **inteiro** existe `//` — ver 3.2.3.

### 3.2.2. Módulo segue o sinal do divisor

`%` usa a semântica de piso (a mesma do Python): o resto tem o **sinal do
divisor**, não o do dividendo.

```ps
post(-7 % 3)    # 2    (não -1)
post(7 % -3)    # -2
```

### 3.2.3. `//` — divisão inteira

`//` devolve o quociente com **piso** (arredonda para baixo, não trunca para
zero), como no Python. Entre inteiros o resultado é `int`; com qualquer `flo`
envolvido é `flo`.

```ps
post(7 // 2)      # 3
post(-7 // 2)     # -4    piso, não -3
post(7 // -2)     # -4
post(7.0 // 2)    # 3.0
```

**Por que ele existe.** `/` é sempre real, então o único jeito de tirar
quociente inteiro era `int(a / b)` — que passa por `double` e perde precisão
exatamente onde a linguagem não deveria perder, já que o `int` dela é de
precisão arbitrária:

```ps
post(int(10000000000000001 / 1))   # 10000000000000000   ← perdeu 1
post(10000000000000001 // 1)       # 10000000000000001   ← exato
```

`//` por zero levanta `ZeroDivisionError: integer division or modulo by zero`.

> **`//` não é mais comentário.** Era comentário de linha até esta mudança. O
> comentário de linha é `#`, que a linguagem sempre aceitou. Não dava pra ter
> os dois: `x = a // b` teria que ser divisão num contexto e comentário no
> outro, e nenhuma regra de desambiguação sobrevive a `a //b` contra `a  // b`.

### 3.2.3b. `**` — potência

```ps
post(2 ** 10)        # 1024
post(2 ** 0)         # 1
post(2 ** -1)        # 0.5     expoente negativo cai pra flo
post(2 ** 100)       # 1267650600228229401496703205376   (bignum)
post(1.5 ** 2)       # 2.25
```

A precedência dele tem três regras, todas as do Python:

```ps
post(-2 ** 2)        # -4      liga mais FORTE que o unário à esquerda
post(2 ** -1)        # 0.5     e mais FRACO à direita
post(2 ** 3 ** 2)    # 512     associa à DIREITA: 2 ** (3 ** 2)
```

`0 ** -1` levanta `ZeroDivisionError: 0.0 cannot be raised to a negative power`.

O builtin **`pow(base, expo)`** faz o mesmo, e tem um terceiro argumento que o
operador não tem: `pow(base, expo, mod)` calcula `(base ** expo) % mod` **sem
materializar a potência inteira** — o único jeito viável com expoente grande.

```ps
post(pow(2, 10))          # 1024
post(pow(3, 200, 1000))   # 1
```

### 3.2.4. Repetição de sequência

`*` entre uma **sequência** e um **int** repete a sequência — vale para `list`,
`tup` e `str`, igual ao Python:

```ps
post([0] * 3)        # [0, 0, 0]
post("ab" * 3)       # ababab
post("-" * 40)       # uma régua de 40 traços
```

Multiplicar sequência por algo que não é `int` é erro, e a mensagem nomeia o
lado errado:

```ps
post("ab" * 1.5)     # TypeError: can't multiply sequence by non-int of type 'flo'
```

> Esta seção dizia que `str * int` **não** funciona e mandava usar a lib de
> string. Funciona desde que a repetição de string entrou no motor — o exemplo
> ensinava exatamente o contrário do que a linguagem faz.

### 3.2.5. `+` concatena, mas NÃO faz coerção

`+` soma números **ou** concatena duas sequências do mesmo tipo. O que ele
**não** faz é misturar tipos: `str` + número é **erro**, nos dois sentidos.
Para montar texto com números, converta com `str()` (ou use uma f-string).

```ps
post("a" + "b")          # "ab"
post([1] + [2])          # [1, 2]
post((1, 2) + (3, 4))    # (1, 2, 3, 4)
post("a" + 1)            # TypeError: can only concatenate str (not "int") to str
post(1 + "a")            # TypeError: unsupported operand type(s) for +: 'int' and 'str'
post("n = " + str(5))    # "n = 5"   
post(f"n = {5}")         # "n = 5"    (idiomático)
```

O texto muda conforme quem está à **esquerda**: com sequência à esquerda o erro
diz qual dos dois lados é o estranho (`not "int"`); nos outros casos ele lista
os dois. É a mesma distinção do CPython.

`-`, `/`, `%` com qualquer `str` envolvida também são erro
(`unsupported operand type(s) for -: 'str' and 'int'`). `*` é a exceção: com
`int` do outro lado ele **repete** — ver 3.2.4.

### 3.2.6. Divisão / módulo por zero

Dividir ou tirar módulo por zero levanta erro em tempo de execução,
capturável com `try`/`catch` (os nomes dos tipos de erro estão na seção de
exceptions):

```ps
try {
    x = 1 / 0
} catch (e) {
    post("erro:", e)     # erro: division by zero (linha 3)
}
```

---

## 3.3. Operadores de comparação

Devolvem sempre `bool`.

| Op | Significado |
|---|---|
| `==` | igual |
| `!=` | diferente |
| `<` `>` `<=` `>=` | ordem (magnitude) |

### 3.3.1. `===` e `!==` NÃO existem

Foram removidos em 29/08. Escrever qualquer um dos dois é erro de sintaxe, com
a mensagem dizendo o que usar no lugar:

```
SyntaxError: `===` nao existe nesta linguagem; use `==`
```

**Por que saíram:** eles nunca foram igualdade estrita. Compilavam para o
**mesmo opcode** do `==`, então `false === Null` respondia igualzinho a
`false == Null`. Quem escrevia `x === Null` acreditando estar protegido da
comparação frouxa estava rodando exatamente a comparação frouxa — o operador
tinha nome de uma coisa e comportamento de outra, e isso custou tempo de
depuração real.

Se você quer comparar **valor e tipo**, compare o tipo junto:

```ps
if x == 0 and type(x) == "int" {
    # só entra com int 0, não com 0.0 nem false
}
```

### 3.3.2. Igualdade compara VALOR, entre tipos numéricos e por estrutura

- Números de tipos diferentes se comparam pelo valor: `1 == 1.0` é `True`.
- `bool` é subtipo de `int`: `1 == true` e `0 == false` são `True`
  (`2 == true` é `False`).
- Listas, tuplas e **dicts** comparam por **conteúdo** (igualdade estrutural),
  não por identidade. Em dict a ordem das chaves não importa.

```ps
post(1 == 1.0)                          # True
post(1 == true)                         # True
post([1, 2] == [1, 2])                  # True
post({"a": 1, "b": 2} == {"b": 2, "a": 1})   # True
post({"a": 1} == {"a": 2})              # False
```

Tipos diferentes que não sejam numéricos nunca são iguais: `5 == "5"` é `False`.

### 3.3.3. `null` na comparação

`null` só é igual a `null`. Na **igualdade** ele não equivale a zero, a `false`
nem a string vazia — `null == 0` é `False`. Nas comparações de **ordem** (`<`,
`>`, `<=`, `>=`), qualquer lado `null` **levanta** `TypeError`: `null` não tem
magnitude, e devolver `False` calado escondia o erro.

```ps
post(null == null)   # True
post(null == 0)      # False
post(null < 5)       # TypeError: '<' not supported between instances of 'Null' and 'int'
post(null >= 0)      # TypeError: '>=' not supported between instances of 'Null' and 'int'
```

Para testar sem levantar, compare com `null` antes:

```ps
x = null
if x != null and x > 0 {
    post("positivo")
}
```

> Esta seção dizia que `null == 0` é `True` e que ordem com `null` devolve
> `False`. As duas coisas mudaram: comparar `null` com `<` era o pior caso —
> `if x > 0` com `x` nulo caía no `else` **sem avisar**, enquanto `"abc" < 5`
> levantava. Duas políticas para o mesmo erro.

### 3.3.4. Comparações são associativas à ESQUERDA (não encadeiam)

Este é um ponto onde a PoolScript difere do Python. `a < b < c` **não** é o
encadeamento matemático `(a < b) and (b < c)`; é a avaliação normal à esquerda
`(a < b) < c` — e como `a < b` é um `bool` (0/1), o segundo `<` compara esse
bool com `c`.

```ps
post(1 < 2 < 3)   # True   → (1<2)=True, True<3 → 1<3 → True  (coincidência)
post(3 > 2 > 1)   # False  → (3>2)=True, True>1 → 1>1 → False
```

Para a intenção de "está no intervalo", escreva explicitamente:
`a < b and b < c`.

---

## 3.4. Operadores lógicos

Duas grafias equivalentes: `and`/`&&`, `or`/`||`, `not`/`!` (e `Not`).
Precedência: `not` (4) é mais forte que `and` (3), que é mais forte que
`or` (2).

### 3.4.1. Curto-circuito

`and` e `or` avaliam o operando da direita **só quando necessário**:

- `A and B` — se `A` é falso, `B` **não** é avaliado.
- `A or B` — se `A` é verdadeiro, `B` **não** é avaliado.

Isso vale para efeitos colaterais: no exemplo, `f()` só roda no último caso.

```ps
action f() {
    post("  f() rodou")
    return true
}

r1 = true or f()      # f() NÃO roda
r2 = false and f()    # f() NÃO roda
r3 = false or f()     # f() roda
```

### 3.4.2. Resultado é sempre `bool` (diferente do Python)

Em Python, `0 or "x"` devolve `"x"` (o operando). Aqui **não**: os operadores
lógicos sempre devolvem um `bool`, resultado da avaliação de verdade dos
operandos (ver *truthiness* na seção 2).

```ps
post(0 or "x")       # True    (não "x")
post("a" and "b")    # True    (não "b")
post(1 and 0)        # False
post(not 0)          # True
```

> Consequência prática: o idioma "valor padrão" do Python
> (`nome = entrada or "anônimo"`) **não** funciona aqui — `or` devolveria
> `True`, não o texto. Use um ternário: `nome = entrada if entrada else "anônimo"`.

---

## 3.5. Identidade de tipo — `is`, `is not`

Aqui `is` **não** é a identidade de objeto do Python. É o **operador de
verificação de tipo**: `valor is Tipo` pergunta se `valor` é daquele tipo, e
`Tipo is Tipo` compara dois tipos. O lado direito costuma ser um nome de tipo
(`int`, `str`, `flo`, `bool`, `list`, `dict`, `tup`, `json`).

```ps
post(5 is int)        # True
post("x" is str)      # True
post(5 is str)        # False
post(3.0 is flo)      # True
x = [1, 2]
post(x is list)       # True
post(x is not dict)   # True
```

`json` e `dict` são o mesmo tipo, então `d is json` e `d is dict` coincidem
(ver seção 2). Para obter o nome do tipo como texto, use `type(x)`.

---

## 3.6. Pertinência — `in`, `not in`

`x in c` testa se `x` está no contêiner `c`; devolve `bool`.

- **lista / tupla:** se algum elemento é igual a `x` (usa a igualdade de 3.3).
- **string:** se `x` (um `str`) é **substring** de `c`.
- **dict / json:** se `x` é uma **chave** (comparada por tipo exato).

```ps
post(2 in [1, 2, 3])        # True
post("ab" in "xabz")        # True    (substring)
post("k" in {"k": 1})       # True    (chave)
post(5 in {"5": 1})         # False   (a chave é o texto "5", não o int 5)
post(9 not in [1, 2, 3])    # True
```

---

## 3.7. Operadores bit a bit

`&` (E), `|` (OU), `^` (XOR), `<<` / `>>` (deslocamento) e o unário `~` (NÃO
bit a bit) operam **apenas entre `int`**. `bool` é rejeitado de propósito (mesmo
sendo 0/1) — o operando precisa ser `int` de verdade; `flo`/`str` também são
erro. Deslocamento por valor negativo é erro.

```ps
post(5 & 3)     # 1
post(5 | 2)     # 7
post(5 ^ 1)     # 4
post(1 << 4)    # 16
post(~5)        # -6      (~x == -x-1)
post(true & 1)  # ERRO — bitwise só entre int
```

Precedência entre eles (do mais forte pro mais fraco): `<<`/`>>` (9), `&` (8),
`^` (7), `|` (6) — todos **mais fortes** que a comparação e **mais fracos** que
`+`/`-`, exatamente como no Python.

---

## 3.8. Operadores unários

Prefixos, nível 12:

| Op | Efeito |
|---|---|
| `-` | negação numérica (`-x`) |
| `+` | identidade numérica (`+x`, devolve `x`) |
| `~` | complemento bit a bit (`int`) |
| `not` / `!` | negação lógica (nível 4, ver 3.4) |
| `await` | aguarda uma corotina (ver seção de assíncrono) |

```ps
post(-5)    # -5
post(+5)    # 5
post(~0)    # -1
```

---

## 3.9. Expressão condicional (ternário)

Forma `then if cond else else` — o valor à esquerda do `if` quando a condição é
verdadeira, senão o da direita do `else`. Associa à direita, então dá pra
encadear:

```ps
sinal = "positivo" if n > 0 else "não-positivo"

faixa = "alto" if n > 100 else "médio" if n > 10 else "baixo"
# lê-se: "alto" if n>100 else ("médio" if n>10 else "baixo")
```

O ramo não escolhido **não** é avaliado (curto-circuito, como o `and`/`or`).

---

## 3.10. Contagem — `count`

`count` conta ocorrências dentro de um contêiner. Tem duas formas, com
significados diferentes:

- **Infixa — conta um VALOR:** `Tipo(valor) count in contêiner` devolve quantas
  vezes aquele valor aparece.
- **Sufixa — conta por TIPO:** `Tipo in contêiner count` devolve quantos
  elementos são daquele tipo.

```ps
post(int(2) count in [2, 2, 3, 2])   # 3   (quantas vezes o valor 2 aparece)
post(int in [2, 2, 3, 2] count)      # 4   (quantos elementos são int)
```

---

## 3.11. Operadores pós-fixados

Ligam mais forte que qualquer operador binário (nível 13) e encadeiam à
esquerda. São detalhados em suas próprias seções; aqui fica só o resumo de
precedência:

| Forma | Significado | Seção |
|---|---|---|
| `f(args)` | chamada de função/método | Funções |
| `obj.membro` | acesso a membro / método / chave de dict | Entity, Dict |
| `seq[i]` | indexação | Tipos / Coleções |
| `seq[a:b:c]` | fatiamento | Tipos / Coleções |
| `x++` &nbsp; `x--` | incremento / decremento | Variáveis e atribuição |

Como ligam mais forte que os binários, `-a.b` é `-(a.b)` e `a + b[0]` é
`a + (b[0])`.

---

## 3.12. Atribuição

A atribuição é um **statement**, não uma expressão (não devolve valor e não pode
aparecer no meio de outra expressão). O operador `=` e suas formas aumentadas
(`+=`, `-=`, `*=`, `/=`, `%=`) e os `++`/`--` são cobertos na seção **Variáveis,
escopo e atribuição**. A forma aumentada reaplica o operador binário
correspondente, herdando as regras de tipo desta seção — `x += 1` segue as
mesmas regras de `x + 1`.

---

## 3.13. Resumo

- **Precedência**: ternário < `or` < `and` < `not` < comparação < bitwise
  (`|`<`^`<`&`) < deslocamento < `+`/`-` < `*`/`/`/`%` < unários < pós-fixados.
- **`/` é sempre real** (dá `flo`); não há `//` nem `**`/`pow`.
- **`%`** segue o sinal do divisor.
- **`+` concatena mas não coage** — `str` + número é erro; use `str()`/f-string.
- **`==`** compara por valor (numérico entre tipos) e por estrutura
  (list/tup/dict). Comparações são **associativas à esquerda**, não encadeiam.
  `===` e `!==` **não existem** — ver §3.3.1.
- **`and`/`or`** curto-circuitam e devolvem **`bool`** (não o operando).
- **`is`** é verificação de **tipo**; **`in`** é pertinência (substring em
  `str`, chave em `dict`).
- **Bitwise** só entre `int` (bool não conta).
```