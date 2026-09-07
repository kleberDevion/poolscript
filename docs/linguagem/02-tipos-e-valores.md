# Referência da Linguagem — 2. Tipos e valores

A PoolScript tem **tipagem estática por declaração**: uma variável declarada
com tipo (`int x = 5`) **é** daquele tipo — a linguagem exige e coage esse tipo
em toda atribuição a ela, da criação em diante. Uma variável criada sem tipo
(`x = 5`) carrega o tipo do valor que recebeu e pode receber outro depois. As
duas formas convivem no mesmo programa.

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
(`funct`), módulos importados, `enum`, `model` — descritos nas suas
seções.

`json` é apenas outro **nome** para `dict` (o mesmo tipo).

### `char` — um caractere

`char` serve em dois lugares: no operador `count`
(`count each char in frase`) e como **tipo de declaração**.

```ps
char a = "x"        # um caractere
char opa = 64       # inteiro converte pelo codepoint -> "@"
char c = "ç"        # acento conta como UM caractere
char e = 128512     # 😀
```

O que ele garante é **um caractere só** — declarar com mais de um é erro, e
`1.5` também:

```ps
char c = "abc"      # AttributedValueError: variável c esperava char (um caractere), recebeu 3
char c = 1.5        # AttributedValueError: variável c esperava char
char c = -1         # ConversionError: -1 nao e um caractere valido
```

`char` é a restrição da **declaração**, não um tipo separado em runtime: o
valor guardado é uma `str` de comprimento 1, e `type()` responde `"str"`. Não
existe construtor `char()` — pra converter um número use `chr(n)`.

`char funct` não existe: só `int funct` e `bool funct` têm tipo de retorno.

---

## 2.2. Descobrindo o tipo — `type(x)`

O builtin `type(x)` devolve o **nome** do tipo como string, e todo valor também
expõe o método `.type()` (equivalente):

```ps
post(type(42))        # int
post(type(3.14))      # flo
post(type("oi"))      # str
post(type([1,2]))     # list
post((1,2).type())    # tup
```

Um número gigante (bignum) continua sendo `"int"` — a promoção é transparente:

```ps
g = 99999999999999999999999999999999999999
post(type(g))         # int
```

---

## 2.3. Veracidade (*truthiness*)

Em contexto booleano (`if`, `while`, `and`, `or`, `not`), o valor é convertido
segundo estas regras:

| Valor | Verdadeiro quando |
|---|---|
| `int`/`flo` | diferente de zero |
| `bool` | `True` |
| `str`/`bytes` | não vazio (`len > 0`) |
| `list`/`tup` | não vazia |
| `dict` | tem ao menos uma entrada |
| `Null`/`none` | **sempre falso** |
| Entity/instância/função | **sempre verdadeiro** |

```ps
if ([]) { post("não entra") }      # lista vazia é falsa
if ("x") { post("entra") }         # string não vazia é verdadeira
if (Null) { post("não entra") }    # Null é falso
```

---

## 2.4. Booleano é um inteiro

Como no Python, `bool` é subtipo de `int`: `True` vale `1` e `False` vale `0` em
qualquer operação aritmética ou de comparação.

```ps
post(True + True)     # 2
post(False < 3)       # True
post([10, 20][True])  # 20  (índice 1)
```

---

## 2.5. `Null` — ausência de valor

`Null` (e os sinônimos `null`/`None`/`none`) representa "sem valor". Regras:

- **Não se ordena.** Qualquer `<`, `>`, `<=`, `>=` com `Null` de um dos lados
  **levanta** `TypeError: '<' not supported between instances of 'Null' and
  'int'` — inclusive `Null >= Null`. Antes devolvia `False`, e a justificativa
  era "melhor que erro"; na prática `if x > 0` com `x` nulo caía no `else`
  **sem avisar**, enquanto `"abc" < 5` levantava. Eram duas políticas para o
  mesmo erro. Quem quer o teste sem levantar escreve `x != Null` antes.
- Em igualdade, `Null == Null` é `True`; com **qualquer** outra coisa é
  `False`, inclusive `Null == 0`, `Null == false` e `Null == ""`.
- **Imprime como `Null`**, com inicial maiúscula, igual a `True` e `False`.
  As quatro grafias (`Null`/`null`/`None`/`none`) valem na ESCRITA; a saída é
  uma só. Em **JSON** continua `null` minúsculo, porque ali é a especificação
  do formato, não a da linguagem: `json.stringify({"a": Null})` dá
  `{"a": null}`.

---

## 2.6. Declaração com tipo e coerção

Declarar o tipo antes do nome torna a atribuição **checada e coagida**:

```ps
int   idade = 30
str   nome  = "ana"
flo   preco = 9.90
bool  ativo = True
```

A checagem vale em **toda** escrita na variável, não só na declaração: `s = 42`
depois de `str s = "oi"` é erro (`variável s esperava str`), e `n = "7"` depois
de `int n = 1` coage para `7` — ver seção 4.6.4.

Regras de coerção na declaração, por tipo-alvo:

| Alvo | Aceita direto | Coage | Recusa |
|---|---|---|---|
| `int` | `int` | string numérica inteira (`"7"`→`7`) | `flo` (mesmo `5.0`), `bool` |
| `flo` | `flo` | `int` (`5`→`5.0`), string numérica (`"1.5"`→`1.5`) | `bool` |
| `str` | `str` | — | `int`, `flo`, `bool` (não "stringifica") |
| `bool` | `bool` | — | `int` (mesmo `1`), etc. |
| `list` `dict` `tup` `json` | o próprio tipo | — | os demais |
| `Object` / `object` | qualquer objeto que não é `str`/`list`/`dict`/`tup`/`bytes`: instância de classe, servidor, conexão, arquivo | — | `str`, `list`, `dict`, `tup`, `bytes`, `int`, `flo`, `bool`, `Null` |
| `string`/`String`, `integer`/`Integer`, `tuple`/`Tuple`, `dictionary`/`Dictionary` | apelidos de `str`, `int`, `tup`, `dict` — a linha do tipo apelidado vale igual | idem | idem |

Ou seja: a linguagem faz só as conversões que não perdem nem adivinham
informação — **int→flo** (alargamento) e **string numérica→número** (parsing).
O resto é erro, e há dois erros distintos:

- **`AttributedValueError`** — tipo incompatível que não se coage
  (`int x = 5.0`, `str s = 42`, `bool b = 1`).
- **`ConversionError`** — o valor é uma string que não representa o número
  pedido (`int x = "abc"`).

```ps
int  x = "7"      # 7    (parsing de string numérica)
flo  f = 5        # 5.0  (alargamento int→flo)
int  y = 5.0      # AttributedValueError — não trunca nem aceita float
str  s = 42       # AttributedValueError — não "stringifica" sozinho
int  z = "abc"    # ConversionError — string não vira int
```

---

## 2.7. Conversão explícita

Há três mecanismos, do mais direto ao mais tolerante:

1. **Construtores builtin** — `int(x)`, `flo(x)`, `str(x)`, `bool(x)`,
   `list(x)`. Convertem ou estouram se impossível:

   ```ps
   n = int("42")        # 42
   s = str(3.14)        # "3.14"
   l = list("abc")      # ["a", "b", "c"]
   ```

2. **Lib `Parsing` com alvo `to <tipo>`** — conversão *tolerante* (não estoura;
   devolve o que der), boa para entrada suja. O `to <tipo>` diz o tipo-alvo às
   funções do `Parsing`; **não é um cast solto** — `"7" to int` sozinho não
   converte (continua string). Detalhada na seção de bibliotecas.

   ```ps
   n = Parsing.integer("  127.abc ", to int)
   ```

---

## 2.8. Torre numérica e promoção

Em operação mista, a linguagem promove seguindo a regra do Python:

- `int OP int` → `int` (com promoção a **bignum** se estourar 64 bits).
- Qualquer operando `flo` → o resultado é `flo`.
- `bool` participa como `int` (0/1).

```ps
post(2 + 3)          # 5      (int)
post(2 + 3.0)        # 5.0    (flo)
post(9223372036854775807 + 1)   # 9223372036854775808  (bignum, sem estourar)
```

A divisão `/` é **sempre real** (resultado `flo`), inclusive entre inteiros:
`7 / 2` é `3.5`. O `%` segue o **sinal do divisor** (como o Python): `-1 % 3` é
`2`.

---

## 2.9. Mutabilidade

- **Mutáveis:** `list`, `dict` — podem ser alteradas no lugar (`addEnd`,
  `l[0] = x`, `d["k"] = v`).
- **Imutáveis:** `str`, `tup`, `bytes`, números — operações geram um novo valor.

```ps
l = [1, 2]
addEnd(l, 3)         # l vira [1, 2, 3] (mesma lista)

t = (1, 2)
# t[0] = 9         # erro — tupla é imutável
```
