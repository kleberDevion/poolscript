# Referência da Linguagem — 1. Estrutura léxica

Esta seção especifica como o **lexer** (o primeiro estágio do compilador/
transforma o texto-fonte de um `.ps`/`.psl`/`.p` numa sequência
de *tokens*. É o nível mais baixo da linguagem: o que conta como espaço,
comentário, número, string, operador, e como blocos são delimitados. As seções
seguintes (tipos, expressões, statements) assumem estas regras.

O
mesmo fonte produz os mesmos tokens.

---

## 1.1. Modelo de código-fonte

- O fonte é **texto UTF-8**. Fora de strings, a linguagem usa apenas ASCII para
  palavras-chave, operadores e pontuação; dentro de strings qualquer caractere
  Unicode é válido.
- A varredura é **caractere a caractere**, mantendo `linha` e `coluna` (ambas
  1-based) para as mensagens de erro no estilo Python (com o indicador `^^^`).
- Quebras de linha `\n` e `\r\n` são reconhecidas; o `\r` isolado é ignorado.

---

## 1.2. Comentários

Há **duas** formas de comentário, ambas **descartadas** na tokenização (não
viram tokens, não afetam o programa):

| Forma | Sintaxe | Alcance |
|---|---|---|
| Linha | `# texto` | do `#` até o fim da linha |
| Bloco | `""" ... """` | de `"""` até o próximo `"""`, podendo cruzar linhas |

> **`//` NÃO é mais comentário.** Ele virou o operador de **divisão inteira**
> (seção 3.2.3). Eram três formas; hoje são duas. Não dava pra ter as duas
> coisas: `x = a // b` teria que ser divisão num contexto e comentário no
> outro, e nenhuma regra de desambiguação sobrevive a `a //b` contra `a  // b`.
>
> Código antigo com `// comentário` agora é erro de sintaxe, e o erro diz isso
> com todas as letras: `'//' e divisao inteira, nao comentario — comentario e
> '#' (ou bloco entre tres aspas)`. A troca é `#`.

```ps
# isto é um comentário de linha
x = 10   # também é comentário de linha

"""
comentário de bloco:
pode ocupar várias linhas
"""
```

> **Atenção — `"""` é comentário, não string.** Aspas duplas triplas iniciam um
> **comentário de bloco**, nunca uma string multi-linha. Para uma string que
> ocupa várias linhas, use **aspas simples triplas** `''' ... '''` (ver 1.6.3).
> Um `"""` que nunca fecha é erro de sintaxe (`bloco de comentario """ nao foi
> fechado`).

---

## 1.3. Espaço em branco e delimitação de blocos

O bloco da PoolScript é **`{ }`**, e só. Quem delimita é a chave; a
**indentação não tem significado** nenhum pro compilador:

```ps
funct soma(a, b) {
    return a + b
}
```

A chave de abertura vale na mesma linha do cabeçalho ou na linha seguinte, e o
`}` de fechamento pode vir colado à continuação (`} else {`) ou sozinho:

```ps
funct soma(a, b)
{
    return a + b
}
```

### 1.3.1. Não existe bloco por `:`

Um `:` no fim da linha **não** abre bloco. A tentativa é recusada com uma
mensagem que diz o que usar:

```
SyntaxError: bloco com ':' nao existe mais — use '{ }'
```

Já foi diferente: a linguagem aceitava `:` + indentação (estilo Python) e
chaves, misturados no mesmo arquivo. Manter os dois saiu caro — praticamente
toda regressão de parser vinha da interação entre indentação e chave — e o `:`
saiu de vez.

O `:` continua com os outros três papéis, que **não** abrem bloco:

```ps
d = { "a": 1 }              # separador de dicionário
s = "abcdef"[1:3]           # fatia
Entity P() { nome: str }    # tipo de campo
```

### 1.3.2. Indentação e quebra de linha

Sem bloco por indentação, o recuo é só estética: indente como quiser (o
repositório usa 4 espaços por nível, por costume). Dentro de `(`, `[` e `{` as
quebras de linha também não geram token estrutural, então uma expressão pode
se espalhar por várias linhas à vontade.

Linhas em branco e linhas só com comentário não significam nada.

### 1.3.3. Continuação de linha por `.membro`

Quando a próxima linha (ignorando espaços) começa com `.` seguido de letra ou
`_`, ela é tratada como **continuação da expressão anterior** — não gera
NEWLINE nem mexe na indentação. Isto habilita *method chaining* em várias
linhas:

```ps
resposta = request.get(url=u)
                  .json()
                  .get("dados")
```

Um `.` isolado **não** dispara essa regra — segue o fluxo normal. E `.5` não é
float: o número da linguagem exige dígito antes do ponto (`0.5`), então `.5`
sozinho é `SyntaxError: expressao invalida`.

---

## 1.4. Identificadores

Um identificador nomeia variáveis, funções, campos, parâmetros, etc.

- Deve começar com **letra ou `_`** e seguir com letras, dígitos ou `_`
  (regex: `[A-Za-z_][A-Za-z0-9_]*`).
- A **caixa da primeira letra é semântica** e o lexer distingue dois tokens:
  - **minúscula ou `_`** → `IDENT` — variáveis, funções, parâmetros comuns.
  - **MAIÚSCULA** → `IDENT_UPPER` — por **convenção** usado para **libs,
    classes/Entity e tipos de erro**. Não é uma reserva rígida: um nome
    maiúsculo também pode ser uma variável comum (`str NOME = "ana"` ou
    `MAX = 100`). Mas o parser usa a caixa em alguns pontos — por exemplo,
    `catch (Tipo e)` e `raise Tipo(...)` só reconhecem o tipo quando ele começa
    com maiúscula (ver a seção de exceptions).

```ps
nome      = "ana"     # IDENT
_cache    = []        # IDENT
Usuario   = ...       # IDENT_UPPER (uma Entity/classe)
```

---

## 1.5. Palavras reservadas (keywords)

Estas são as palavras **da linguagem** — reservadas, não podem nomear variáveis.
Agrupadas por papel:

| Grupo | Palavras |
|---|---|
| Fluxo | `if` `elif` `else` `while` `for` `each` `in` `is` `match` `case` `break` `continue` `pass` `return` |
| Lógicos | `and` `or` `not` `Not` |
| Funções | `funct` `async` `await` `yield` |
| Tipos (em declaração / cast / `count`) | `str` `int` `long` `flo` `bool` `list` `dict` `tup` `json` `char` `Object` `object` |
| Classes / OO | `Entity` `class` `Class` `self` `base` `model` `enum` |
| Encapsulamento | `private` `public` |
| Módulos | `import` `from` `as` `PUSH` `GET` |
| Exceptions | `try` `catch` `finally` `raise` |
| Escopo / contexto | `global` `using` |
| Conversão / operador | `to` `count` |

`dict` é apelido de `json`; `tup` nomeia a tupla; `base` é reconhecida
contextualmente dentro de `Entity` (chama o construtor do pai). `action` e
`reaction` **saíram**: não são mais reservadas, e o parser as recusa com a
palavra nova na mensagem (seção 6.1).

Duas coisas **não** estão nesta tabela, e não é esquecimento:

- Os **apelidos de tipo** (`string`/`String`, `integer`/`Integer`,
  `tuple`/`Tuple`, `dictionary`/`Dictionary`) valem por **posição**, só onde um
  tipo vale. Fora dali são nome comum: `string = "a"` continua valendo.
- Os **modificadores colados** `static` e `nonnull` (seção 6), pela mesma
  razão: só valem na cabeça de uma `funct`.

A lista viva sai do próprio motor, com `pool --metadata` (campo `keywords`).

Os **builtins** (`post`, `input`, `len`, `range`, `addEnd`, `chr`, `ord`, ...)
também são nomes reservados, mas são **funções**, não palavras de controle —
documentadas na seção *Builtins*. O lexer ainda reserva um punhado de palavras
**históricas ou de lib** (sem gramática nem builtin ativo) que não fazem parte
da linguagem e portanto não são listadas aqui.

---

## 1.6. Literais

### 1.6.1. Inteiros

Sequência de dígitos decimais (`\d+`). Sem limite de tamanho: um literal maior
que 64 bits é promovido automaticamente a **inteiro de precisão arbitrária**
(bignum) — `type()` continua devolvendo `"int"`.

```ps
x = 42
gigante = 99999999999999999999999999999999999999   # ainda é int
```

### 1.6.2. Ponto flutuante (`flo`)

Dígitos com um ponto decimal (`\d+\.\d+`) — o dígito antes do ponto é
obrigatório, então `0.5` vale e `.5` é erro de sintaxe.

Há **notação científica**, com `e` ou `E`, e o resultado é sempre `flo`:

```ps
post(1E5, 1.5e3, 2e-2, type(1e5))    # 100000.0 1500.0 0.02 flo
```

```ps
pi = 3.14159
```

### 1.6.3. Strings

Aspas **simples e duplas são equivalentes** — escolha uma; não há diferença de
semântica. Uma string não pode cruzar a quebra de linha (erro `string nao
fechada antes da quebra de linha`), a menos que seja multi-linha.

| Forma | Exemplo | Observação |
|---|---|---|
| Simples/dupla | `"oi"` / `'oi'` | equivalentes |
| Multi-linha | `''' ... '''` | aspas **simples** triplas (as duplas triplas são comentário) |
| f-string | `f"olá {nome}"` | interpola expressões entre `{ }` |
| f-string multi-linha | `f''' ... {x} ... '''` | |
| raw | `r"\n literal"` | não processa escapes |
| raw multi-linha | `r''' ... '''` | |

**Escapes** (processados fora de raw strings):

| Escape | Resultado | | Escape | Resultado |
|---|---|---|---|---|
| `\n` | nova linha | | `\\` | `\` |
| `\t` | tab | | `\"` `\'` | aspas |
| `\r` | retorno | | `\a \b \f \v` | controle |
| `\e` | ESC (`\x1b`, p/ ANSI) | | `\033` | octal (1–3 díg.) |
| `\xHH` | hex (2 díg.) | | `\uXXXX` / `\UXXXXXXXX` | Unicode |

Um escape **desconhecido mantém a barra E o caractere**, e sai um aviso:

```
arquivo.ps:1: SyntaxWarning: sequencia de escape invalida '\p' — a barra fica
no texto; use '\\p' se ela e mesmo pra estar ali
```

`"C:\pasta"` tem 8 caracteres e vale `C:\pasta`, como no Python. Isso já foi
diferente e era pior: a barra sumia **calada**, o valor virava `C:pasta` (7), e
quem escrevia um caminho do Windows perdia um byte sem ficar sabendo. O aviso
vai pro **stderr** (não suja a saída do programa) e o `pool --check` o devolve
no JSON, então o editor sublinha.

Aviso não é erro: o programa roda e sai com 0.

O valor de `\033`, `\x1b` e `\e` é o **mesmo byte** ESC.

```ps
post("linha1\nlinha2")
post("\e[1mnegrito\e[0m")        # ANSI
post(r"C:\temp\nome")            # raw: a \n fica literal
nome = "mundo"
post(f"olá, {nome}!")            # f-string
```

### 1.6.4. Booleanos e nulo

- **Booleano:** `True`/`true` e `False`/`false` (as quatro formas valem).
- **Nulo:** `Null`/`null`/`None`/`none` (as quatro grafias valem na escrita; o
  valor **imprime** como `Null`, com inicial maiúscula, igual a `True` e
  `False`).

### 1.6.5. Literal de cor — `<cor>"texto"`

Uma sintaxe própria: `<hex>` ou `<nome>` colado numa string produz um token de
cor (aplicado como sequência ANSI na saída).

- `<hex>` = **3 ou 6** dígitos hexadecimais (como no CSS). 4 ou 5 dígitos **não**
  são cor válida e o `<...>` volta a ser tratado como operador.
- `<nome>` = uma das cores nomeadas: `red green blue yellow cyan magenta white
  black purple orange pink gray/grey lime teal`.

```ps
post(<red>"erro!")
post(<2196F3>"azul")
```

### 1.6.6. Coleções

Sintaxe reconhecida no parser (detalhada na seção de tipos):

```ps
lista = [1, 2, 3]                 # list
mapa  = { "a": 1, "b": 2 }        # dict/json
tupla = (1, 2, 3)                 # tup (imutável)
```

---

## 1.7. Operadores e pontuação

**Operadores de múltiplos caracteres** (reconhecidos do mais longo para o mais
curto — é o que faz `2 ** 3` não virar `2 * (*3)`): `==` `!=` `<=` `>=` `&&`
`||` `<<` `>>` `**` `//` `+=` `-=` `*=` `/=` `%=` `++` `--`.

`**` é potência e `//` é divisão inteira (seção 3). Não existe `**=` nem `//=`:
`x **= 2` é `SyntaxError: expressao invalida`.

O lexer também reconhece `===` e `!==`, mas **só para recusar com nome**:
`` `===` nao existe nesta linguagem; use `==` ``. Sem isso o erro sairia como
uma expressão inválida qualquer, sem dizer o que estava errado.

**Operadores de um caractere:** `+ - * / % = < > ! . , @ | ^ & ~`.

**Pontuação estrutural** (cada uma é seu próprio token): `( )` `[ ]` `{ }` `:`
`;` `,` `.` `@`.

Os operadores lógicos existem em duas grafias equivalentes: `&&`/`and`,
`||`/`or`, `!`/`not`. A semântica (precedência, curto-circuito, coerção) é
definida na seção de expressões.

---

## 1.8. Resumo dos tipos de token

O lexer emite: `IDENT`, `IDENT_UPPER`, `KW` (keyword), `INT`, `FLO`, `STR`,
`FSTRING`, `BOOL`, `NULL`, `COLOR`, `OP` (operador), os tokens de pontuação
(`LPAREN`, `RBRACE`, `COLON`, ...), `NEWLINE`, `INDENT`, `DEDENT` e `EOF`. As
próximas seções descrevem como o parser combina esses tokens em expressões e
statements.
