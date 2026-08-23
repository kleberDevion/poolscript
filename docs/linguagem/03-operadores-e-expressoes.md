# Referência da Linguagem — 3. Operadores e expressões

Uma **expressão** é qualquer trecho que o motor avalia até um valor: um literal,
um nome, uma chamada, ou combinações disso por **operadores**. Esta seção
especifica todos os operadores da linguagem — o que cada um faz, sobre que
tipos, o que devolve, quando dá erro — e as regras que governam como uma
expressão maior é montada a partir das menores: **precedência**,
**associatividade** e **curto-circuito**.

Tudo aqui vale igual nos dois motores (interpretador em Python e VM em C): cada
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
| 5 | Comparação, pertinência, tipo, contagem | `==` `!=` `===` `!==` `<` `>` `<=` `>=` &nbsp; `is` `is not` &nbsp; `in` `not in` &nbsp; `count` | à esquerda |
| 6 | OU bit a bit | `\|` | à esquerda |
| 7 | XOR bit a bit | `^` | à esquerda |
| 8 | E bit a bit | `&` | à esquerda |
| 9 | Deslocamento | `<<` &nbsp; `>>` | à esquerda |
| 10 | Adição / subtração | `+` &nbsp; `-` | à esquerda |
| 11 | Multiplicação / divisão / módulo | `*` &nbsp; `/` &nbsp; `%` | à esquerda |
| 12 | Unários | `+` &nbsp; `-` &nbsp; `~` &nbsp; `await` | prefixa (à direita) |
| 13 | Pós-fixados | chamada `()` &nbsp; membro `.x` &nbsp; índice/fatia `[…]` &nbsp; `++` `--` | à esquerda |
| 14 | Primários | literais, nomes, `(…)`, `[…]`, `{…}` | — |

Exemplos verificados:

![exemplo 1](../assets/linguagem__03-operadores-e-expressoes_ex1.png)

<details><summary>código</summary>

```ps
post(2 + 3 * 4)      // 14   — `*` (11) antes de `+` (10)
post(1 | 2 & 3)      // 3    — `&` (8) antes de `|` (6): 1 | (2 & 3)
post(1 + 2 << 3)     // 24   — `+` (10) antes de `<<` (9): (1 + 2) << 3
post(not 1 == 1)     // False — `==` (5) antes de `not` (4): not (1 == 1)
```

</details>

> **`not`/`!` é mais fraco que a comparação** (nível 4 < nível 5), como no
> Python. `not a == b` é `not (a == b)`, nunca `(not a) == b`. Para negar só o
> operando, use parênteses: `(not a) == b`.

Use **parênteses** `(…)` sempre que quiser forçar uma ordem diferente da tabela
— eles são o nível primário (14) e vencem tudo.

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
| `*` | multiplicação / repetição | números; **também** `list`*`int` |
| `/` | divisão | números — **sempre** verdadeira (resultado `flo`) |
| `%` | módulo (resto) | números |

### 3.2.1. Divisão é sempre real

`/` **nunca** trunca: o resultado é `flo`, mesmo quando divide exato.

![exemplo 2](../assets/linguagem__03-operadores-e-expressoes_ex2.png)

<details><summary>código</summary>

```ps
post(7 / 2)     // 3.5
post(10 / 5)    // 2.0   — não é 2 (int); é flo
```

</details>

Não existe operador de **divisão inteira** (`//` do Python não existe aqui).
Para o quociente inteiro, converta: `int(10 / 3)` → `3`.

### 3.2.2. Módulo segue o sinal do divisor

`%` usa a semântica de piso (a mesma do Python): o resto tem o **sinal do
divisor**, não o do dividendo.

![exemplo 3](../assets/linguagem__03-operadores-e-expressoes_ex3.png)

<details><summary>código</summary>

```ps
post(-7 % 3)    // 2    (não -1)
post(7 % -3)    // -2
```

</details>

### 3.2.3. Não há exponenciação

A linguagem **não tem operador de potência** — `**` não existe (é erro de
sintaxe) e **não há builtin `pow`**. Se precisar de potência, implemente com
multiplicação/laço ou use a lib de matemática (quando aplicável).

### 3.2.4. Repetição de sequência

`*` entre uma **lista** e um **int** repete a lista. `list` * `int` funciona;
`str` * `int` **não** (é erro — para repetir texto use a lib de string):

![exemplo 4](../assets/linguagem__03-operadores-e-expressoes_ex4.png)

<details><summary>código</summary>

```ps
post([0] * 3)        // [0, 0, 0]
post("ab" * 3)       // ERRO — operação matemática inválida entre str e int
```

</details>

### 3.2.5. `+` concatena, mas NÃO faz coerção

`+` soma números **ou** concatena duas sequências do mesmo tipo. O que ele
**não** faz é misturar tipos: `str` + número é **erro**, nos dois sentidos.
Para montar texto com números, converta com `str()` (ou use uma f-string).

![exemplo 5](../assets/linguagem__03-operadores-e-expressoes_ex5.png)

<details><summary>código</summary>

```ps
post("a" + "b")          // "ab"
post([1] + [2])          // [1, 2]
post((1, 2) + (3, 4))    // (1, 2, 3, 4)
post("a" + 1)            // ERRO — operação matemática inválida entre str e int
post("n = " + str(5))    // "n = 5"   
post(f"n = {5}")         // "n = 5"    (idiomático)
```

</details>

`-`, `*`, `/`, `%` com qualquer `str` envolvida também são erro.

### 3.2.6. Divisão / módulo por zero

Dividir ou tirar módulo por zero levanta erro em tempo de execução,
capturável com `try`/`catch` (os nomes dos tipos de erro estão na seção de
exceptions):

![exemplo 6](../assets/linguagem__03-operadores-e-expressoes_ex6.png)

<details><summary>código</summary>

```ps
try:
    x = 1 / 0
catch (e):
    post("erro:", e)     // erro: divisão por zero: division by zero
```

</details>

---

## 3.3. Operadores de comparação

Devolvem sempre `bool`.

| Op | Significado |
|---|---|
| `==` &nbsp; `===` | igual |
| `!=` &nbsp; `!==` | diferente |
| `<` `>` `<=` `>=` | ordem (magnitude) |

### 3.3.1. `===` é apelido de `==` (não é "igualdade estrita")

Ao contrário do JavaScript, **`===` e `==` são idênticos** aqui — mesma
semântica. `!==` idem a `!=`. As formas com três caracteres existem só por
conforto visual de quem vem de outra linguagem; não há diferença de
comportamento.

### 3.3.2. Igualdade compara VALOR, entre tipos numéricos e por estrutura

- Números de tipos diferentes se comparam pelo valor: `1 == 1.0` é `True`.
- `bool` é subtipo de `int`: `1 == true` e `0 == false` são `True`
  (`2 == true` é `False`).
- Listas, tuplas e **dicts** comparam por **conteúdo** (igualdade estrutural),
  não por identidade. Em dict a ordem das chaves não importa.

![exemplo 7](../assets/linguagem__03-operadores-e-expressoes_ex7.png)

<details><summary>código</summary>

```ps
post(1 == 1.0)                          // True
post(1 == true)                         // True
post([1, 2] == [1, 2])                  // True
post({"a": 1, "b": 2} == {"b": 2, "a": 1})   // True
post({"a": 1} == {"a": 2})              // False
```

</details>

Tipos diferentes que não sejam numéricos nunca são iguais: `5 == "5"` é `False`.

### 3.3.3. `null` na comparação

`null == null` é `True`. Na **igualdade**, `null` equivale a zero numérico
(`null == 0` é `True`). Nas comparações de **ordem** (`<`, `>`, `<=`, `>=`),
qualquer lado `null` resulta sempre `False` — `null` não tem magnitude.

![exemplo 8](../assets/linguagem__03-operadores-e-expressoes_ex8.png)

<details><summary>código</summary>

```ps
post(null == null)   // True
post(null == 0)      // True
post(null < 5)       // False
post(null >= 0)      // False
```

</details>

### 3.3.4. Comparações são associativas à ESQUERDA (não encadeiam)

Este é um ponto onde a PoolScript difere do Python. `a < b < c` **não** é o
encadeamento matemático `(a < b) and (b < c)`; é a avaliação normal à esquerda
`(a < b) < c` — e como `a < b` é um `bool` (0/1), o segundo `<` compara esse
bool com `c`.

![exemplo 9](../assets/linguagem__03-operadores-e-expressoes_ex9.png)

<details><summary>código</summary>

```ps
post(1 < 2 < 3)   // True   → (1<2)=True, True<3 → 1<3 → True  (coincidência)
post(3 > 2 > 1)   // False  → (3>2)=True, True>1 → 1>1 → False
```

</details>

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

![exemplo 10](../assets/linguagem__03-operadores-e-expressoes_ex10.png)

<details><summary>código</summary>

```ps
action f():
    post("  f() rodou")
    return true

r1 = true or f()      // f() NÃO roda
r2 = false and f()    // f() NÃO roda
r3 = false or f()     // f() roda
```

</details>

### 3.4.2. Resultado é sempre `bool` (diferente do Python)

Em Python, `0 or "x"` devolve `"x"` (o operando). Aqui **não**: os operadores
lógicos sempre devolvem um `bool`, resultado da avaliação de verdade dos
operandos (ver *truthiness* na seção 2).

![exemplo 11](../assets/linguagem__03-operadores-e-expressoes_ex11.png)

<details><summary>código</summary>

```ps
post(0 or "x")       // True    (não "x")
post("a" and "b")    // True    (não "b")
post(1 and 0)        // False
post(not 0)          // True
```

</details>

> Consequência prática: o idioma "valor padrão" do Python
> (`nome = entrada or "anônimo"`) **não** funciona aqui — `or` devolveria
> `True`, não o texto. Use um ternário: `nome = entrada if entrada else "anônimo"`.

---

## 3.5. Identidade de tipo — `is`, `is not`

Aqui `is` **não** é a identidade de objeto do Python. É o **operador de
verificação de tipo**: `valor is Tipo` pergunta se `valor` é daquele tipo, e
`Tipo is Tipo` compara dois tipos. O lado direito costuma ser um nome de tipo
(`int`, `str`, `flo`, `bool`, `list`, `dict`, `tup`, `json`).

![exemplo 12](../assets/linguagem__03-operadores-e-expressoes_ex12.png)

<details><summary>código</summary>

```ps
post(5 is int)        // True
post("x" is str)      // True
post(5 is str)        // False
post(3.0 is flo)      // True
x = [1, 2]
post(x is list)       // True
post(x is not dict)   // True
```

</details>

`json` e `dict` são o mesmo tipo, então `d is json` e `d is dict` coincidem
(ver seção 2). Para obter o nome do tipo como texto, use `type(x)`.

---

## 3.6. Pertinência — `in`, `not in`

`x in c` testa se `x` está no contêiner `c`; devolve `bool`.

- **lista / tupla:** se algum elemento é igual a `x` (usa a igualdade de 3.3).
- **string:** se `x` (um `str`) é **substring** de `c`.
- **dict / json:** se `x` é uma **chave** (comparada por tipo exato).

![exemplo 13](../assets/linguagem__03-operadores-e-expressoes_ex13.png)

<details><summary>código</summary>

```ps
post(2 in [1, 2, 3])        // True
post("ab" in "xabz")        // True    (substring)
post("k" in {"k": 1})       // True    (chave)
post(5 in {"5": 1})         // False   (a chave é o texto "5", não o int 5)
post(9 not in [1, 2, 3])    // True
```

</details>

---

## 3.7. Operadores bit a bit

`&` (E), `|` (OU), `^` (XOR), `<<` / `>>` (deslocamento) e o unário `~` (NÃO
bit a bit) operam **apenas entre `int`**. `bool` é rejeitado de propósito (mesmo
sendo 0/1) — o operando precisa ser `int` de verdade; `flo`/`str` também são
erro. Deslocamento por valor negativo é erro.

![exemplo 14](../assets/linguagem__03-operadores-e-expressoes_ex14.png)

<details><summary>código</summary>

```ps
post(5 & 3)     // 1
post(5 | 2)     // 7
post(5 ^ 1)     // 4
post(1 << 4)    // 16
post(~5)        // -6      (~x == -x-1)
post(true & 1)  // ERRO — bitwise só entre int
```

</details>

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

![exemplo 15](../assets/linguagem__03-operadores-e-expressoes_ex15.png)

<details><summary>código</summary>

```ps
post(-5)    // -5
post(+5)    // 5
post(~0)    // -1
```

</details>

---

## 3.9. Expressão condicional (ternário)

Forma `then if cond else else` — o valor à esquerda do `if` quando a condição é
verdadeira, senão o da direita do `else`. Associa à direita, então dá pra
encadear:

![exemplo 16](../assets/linguagem__03-operadores-e-expressoes_ex16.png)

<details><summary>código</summary>

```ps
sinal = "positivo" if n > 0 else "não-positivo"

faixa = "alto" if n > 100 else "médio" if n > 10 else "baixo"
// lê-se: "alto" if n>100 else ("médio" if n>10 else "baixo")
```

</details>

O ramo não escolhido **não** é avaliado (curto-circuito, como o `and`/`or`).

---

## 3.10. Contagem — `count`

`count` conta ocorrências dentro de um contêiner. Tem duas formas, com
significados diferentes:

- **Infixa — conta um VALOR:** `Tipo(valor) count in contêiner` devolve quantas
  vezes aquele valor aparece.
- **Sufixa — conta por TIPO:** `Tipo in contêiner count` devolve quantos
  elementos são daquele tipo.

![exemplo 17](../assets/linguagem__03-operadores-e-expressoes_ex17.png)

<details><summary>código</summary>

```ps
post(int(2) count in [2, 2, 3, 2])   // 3   (quantas vezes o valor 2 aparece)
post(int in [2, 2, 3, 2] count)      // 4   (quantos elementos são int)
```

</details>

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
- **`==`/`===`** são iguais; comparam por valor (numérico entre tipos) e por
  estrutura (list/tup/dict). Comparações são **associativas à esquerda**, não
  encadeiam.
- **`and`/`or`** curto-circuitam e devolvem **`bool`** (não o operando).
- **`is`** é verificação de **tipo**; **`in`** é pertinência (substring em
  `str`, chave em `dict`).
- **Bitwise** só entre `int` (bool não conta).
```
