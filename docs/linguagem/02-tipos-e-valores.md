# Referência da Linguagem — 2. Tipos e valores

A PoolScript é **dinamicamente tipada com anotação estática opcional**: toda
variável carrega o tipo do seu valor em tempo de execução (como Python), mas
você *pode* declarar um tipo (`int x = 5`), e aí a linguagem passa a **exigir e
coagir** esse tipo na atribuição (mais perto de Java/TypeScript). As duas coisas
convivem.

---

## 2.1. Os tipos internos

| Tipo | `type()` devolve | Literal | Mutável? | Descrição |
|---|---|---|---|---|
| Inteiro | `"int"` | `42`, `0` | — (imutável) | 64 bits com **promoção automática a precisão arbitrária** (bignum) no estouro |
| Ponto flutuante | `"flo"` | `3.14` | — | IEEE-754 double |
| Booleano | `"bool"` | `True`/`False` | — | subtipo de inteiro (ver 2.4) |
| String | `"str"` | `"texto"` | não | sequência de caracteres Unicode (UTF-8) |
| Lista | `"list"` | `[1, 2]` | **sim** | sequência ordenada e homogênea/heterogênea |
| Tupla | `"tup"` | `(1, 2)` | não | sequência **imutável** |
| Dicionário | `"dict"` | `{ "k": v }` | **sim** | mapa chave→valor (apelido: `json`) |
| Bytes | `"bytes"` | `"x".encode()` | não | sequência de bytes crus |
| Nulo | `"Null"` | `Null`/`null`/`None`/`none` | — | ausência de valor |

Além destes, existem **valores-objeto**: instâncias de `Entity`, funções
(`action`/`reaction`), módulos importados, `enum`, `model` — descritos nas suas
seções.

`json` é apenas outro **nome** para `dict` (o mesmo tipo). `char` é um nome de
tipo usado sobretudo com o operador `count` (ex.: `count each char in frase`) e
representa um caractere isolado — **não** existe um valor `char` distinto de
`str` em runtime nem um construtor `char()`.

---

## 2.2. Descobrindo o tipo — `type(x)`

O builtin `type(x)` devolve o **nome** do tipo como string, e todo valor também
expõe o método `.type()` (equivalente):

![exemplo 1](../assets/linguagem__02-tipos-e-valores_ex1.png)

<details><summary>código</summary>

```ps
post(type(42))        // int
post(type(3.14))      // flo
post(type("oi"))      // str
post(type([1,2]))     // list
post((1,2).type())    // tup
```

</details>

Um número gigante (bignum) continua sendo `"int"` — a promoção é transparente:

![exemplo 2](../assets/linguagem__02-tipos-e-valores_ex2.png)

<details><summary>código</summary>

```ps
g = 99999999999999999999999999999999999999
post(type(g))         // int
```

</details>

---

## 2.3. Veracidade (*truthiness*)

Em contexto booleano (`if`, `while`, `and`, `or`, `not`), o valor é convertido
segundo estas regras — **iguais nos dois motores**:

| Valor | Verdadeiro quando |
|---|---|
| `int`/`flo` | diferente de zero |
| `bool` | `True` |
| `str`/`bytes` | não vazio (`len > 0`) |
| `list`/`tup` | não vazia |
| `dict` | tem ao menos uma entrada |
| `Null`/`none` | **sempre falso** |
| Entity/instância/função | **sempre verdadeiro** |

![exemplo 3](../assets/linguagem__02-tipos-e-valores_ex3.png)

<details><summary>código</summary>

```ps
if ([]) { post("não entra") }      // lista vazia é falsa
if ("x") { post("entra") }         // string não vazia é verdadeira
if (Null) { post("não entra") }    // Null é falso
```

</details>

---

## 2.4. Booleano é um inteiro

Como no Python, `bool` é subtipo de `int`: `True` vale `1` e `False` vale `0` em
qualquer operação aritmética ou de comparação.

![exemplo 4](../assets/linguagem__02-tipos-e-valores_ex4.png)

<details><summary>código</summary>

```ps
post(True + True)     // 2
post(False < 3)       // True
post([10, 20][True])  // 20  (índice 1)
```

</details>

---

## 2.5. `Null` — ausência de valor

`Null` (e os sinônimos `null`/`None`/`none`) representa "sem valor". Regras:

- **Não se ordena.** Qualquer `<`, `>`, `<=`, `>=` com `Null` de um dos lados é
  `False` — inclusive `Null >= Null`. Isso é proposital: `if x > 0` com `x`
  ainda não preenchido simplesmente não entra, em vez de estourar.
- Em igualdade, `Null == Null` é `True`; `Null == 0` e `Null == 0.0` são `True`
  (compatibilidade numérica); com o resto é `False`.
- Imprime como `null`.

---

## 2.6. Declaração com tipo e coerção

Declarar o tipo antes do nome torna a atribuição **checada e coagida**:

![exemplo 5](../assets/linguagem__02-tipos-e-valores_ex5.png)

<details><summary>código</summary>

```ps
int   idade = 30
str   nome  = "ana"
flo   preco = 9.90
bool  ativo = True
```

</details>

A checagem vale **só na declaração**. A partir daí a variável é dinâmica: uma
atribuição posterior (sem o tipo na frente) pode trocar o valor por outro tipo
livremente — ver seção 4.

Regras de coerção na declaração, por tipo-alvo:

| Alvo | Aceita direto | Coage | Recusa |
|---|---|---|---|
| `int` | `int` | string numérica inteira (`"7"`→`7`) | `flo` (mesmo `5.0`), `bool` |
| `flo` | `flo` | `int` (`5`→`5.0`), string numérica (`"1.5"`→`1.5`) | `bool` |
| `str` | `str` | — | `int`, `flo`, `bool` (não "stringifica") |
| `bool` | `bool` | — | `int` (mesmo `1`), etc. |
| `list` `dict` `tup` `json` | o próprio tipo | — | os demais |

Ou seja: a linguagem faz só as conversões que não perdem nem adivinham
informação — **int→flo** (alargamento) e **string numérica→número** (parsing).
O resto é erro, e há dois erros distintos:

- **`AtributtedValueError`** — tipo incompatível que não se coage
  (`int x = 5.0`, `str s = 42`, `bool b = 1`).
- **`ConversionError`** — o valor é uma string que não representa o número
  pedido (`int x = "abc"`).

![exemplo 6](../assets/linguagem__02-tipos-e-valores_ex6.png)

<details><summary>código</summary>

```ps
int  x = "7"      // 7    (parsing de string numérica)
flo  f = 5        // 5.0  (alargamento int→flo)
int  y = 5.0      // AtributtedValueError — não trunca nem aceita float
str  s = 42       // AtributtedValueError — não "stringifica" sozinho
int  z = "abc"    // ConversionError — string não vira int
```

</details>

---

## 2.7. Conversão explícita

Há três mecanismos, do mais direto ao mais tolerante:

1. **Construtores builtin** — `int(x)`, `flo(x)`, `str(x)`, `bool(x)`,
   `list(x)`. Convertem ou estouram se impossível:

   ![exemplo 7](../assets/linguagem__02-tipos-e-valores_ex7.png)

<details><summary>código</summary>

```ps
   n = int("42")        // 42
   s = str(3.14)        // "3.14"
   l = list("abc")      // ["a", "b", "c"]
   ```

</details>

2. **Lib `Parsing` com alvo `to <tipo>`** — conversão *tolerante* (não estoura;
   devolve o que der), boa para entrada suja. O `to <tipo>` diz o tipo-alvo às
   funções do `Parsing`; **não é um cast solto** — `"7" to int` sozinho não
   converte (continua string). Detalhada na seção de bibliotecas.

   ![exemplo 8](../assets/linguagem__02-tipos-e-valores_ex8.png)

<details><summary>código</summary>

```ps
   n = Parsing.integer("  127.abc ", to int)
   ```

</details>

---

## 2.8. Torre numérica e promoção

Em operação mista, a linguagem promove seguindo a regra do Python:

- `int OP int` → `int` (com promoção a **bignum** se estourar 64 bits).
- Qualquer operando `flo` → o resultado é `flo`.
- `bool` participa como `int` (0/1).

![exemplo 9](../assets/linguagem__02-tipos-e-valores_ex9.png)

<details><summary>código</summary>

```ps
post(2 + 3)          // 5      (int)
post(2 + 3.0)        // 5.0    (flo)
post(9223372036854775807 + 1)   // 9223372036854775808  (bignum, sem estourar)
```

</details>

A divisão `/` é **sempre real** (resultado `flo`), inclusive entre inteiros:
`7 / 2` é `3.5`. O `%` segue o **sinal do divisor** (como o Python): `-1 % 3` é
`2`.

---

## 2.9. Mutabilidade

- **Mutáveis:** `list`, `dict` — podem ser alteradas no lugar (`addEnd`,
  `l[0] = x`, `d["k"] = v`).
- **Imutáveis:** `str`, `tup`, `bytes`, números — operações geram um novo valor.

![exemplo 10](../assets/linguagem__02-tipos-e-valores_ex10.png)

<details><summary>código</summary>

```ps
l = [1, 2]
addEnd(l, 3)         // l vira [1, 2, 3] (mesma lista)

t = (1, 2)
// t[0] = 9         // erro — tupla é imutável
```

</details>
