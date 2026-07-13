# PoolScript v8.0.0 — Referência da linguagem

Este documento descreve, de forma completa, tudo que existe atualmente na linguagem
PoolScript: sintaxe, tipos, controle de fluxo, funções, classes, tratamento de
erros, bibliotecas padrão e comportamentos/limitações conhecidas. Substitui
`docs/PoolScript.md` (que documentava a v1.0.8 e está bastante desatualizado —
não cobre `async`/`await`, `Entity`, `match`/`case`, `count`, decorators nem
unpacking).

> Arquitetura (para quem for mexer no código): lexer (`lexer.py`) → parser
> recursive-descent que produz uma AST de nodes `@dataclass(slots=True)`
> (`parser.py`) → interpretador tree-walking (`interpreter.py`). Sem bytecode,
> sem VM — cada `Node` é executado diretamente andando na árvore.

---

## Índice

1. [Rodando código](#rodando-código)
2. [Comentários](#comentários)
3. [Indentação: `{}` vs `:`](#indentação--vs-)
4. [Tipos e variáveis](#tipos-e-variáveis)
5. [Null](#null)
6. [Strings](#strings)
7. [Listas, tuplas e dicionários](#listas-tuplas-e-dicionários)
8. [Unpacking (desempacotamento de tuplas)](#unpacking-desempacotamento-de-tuplas)
9. [Operadores](#operadores)
10. [Controle de fluxo](#controle-de-fluxo)
11. [Funções: `action` / `reaction`](#funções-action--reaction)
12. [`async` / `await`](#async--await)
13. [Geradores (`yield`)](#geradores-yield)
14. [`try` / `catch` / `finally`](#try--catch--finally)
15. [`match` / `case`](#match--case)
16. [`model`](#model)
17. [`Entity` (classes)](#entity-classes)
18. [`count` / `count each`](#count--count-each)
19. [`using`](#using)
20. [Imports e bibliotecas padrão](#imports-e-bibliotecas-padrão)
21. [Builtins globais](#builtins-globais)
22. [Erros nomeados](#erros-nomeados)
23. [Limitações e comportamentos conhecidos](#limitações-e-comportamentos-conhecidos)

---

## Rodando código

```bash
pool arquivo.ps       # roda um arquivo
pool repl             # REPL interativo
pool build            # roda todos os .ps da pasta atual
pool --version        # versão + runtime (Python/PyPy)
pool --help           # ajuda
```

Chamar `pool` **sem nenhum argumento** abre o REPL interativo (igual ao `python`
sem argumentos) — não imprime a ajuda.

---

## Comentários

```
// comentário de linha
# comentário de linha (alternativa)

"""
comentário de bloco,
pode ter várias linhas
"""
```

---

## Indentação: `{}` vs `:`

PoolScript aceita dois estilos de bloco, e você pode misturá-los no mesmo
arquivo (gera um aviso não-fatal no stderr, ver abaixo):

**Chaves** — sem regras de indentação, mas `elif`/`else` precisam ficar
"colados" ao `}` anterior **na mesma linha**:

```
if (nota >= 9) {
    post("Excelente!")
} elif (nota >= 7) {
    post("Bom!")
} else {
    post("Reprovado")
}
```

Colocar `elif`/`else` numa linha nova após o `}` **não funciona** — o parser
não reconhece a continuação da cadeia e dá erro de sintaxe.

**Dois-pontos, estilo Python** — regras estritas de indentação:

- Só espaços; **TAB é proibido** (`SyntaxError`).
- Cada nível deve ter **exatamente 4 espaços** (2, 3, 5... são erro).
- Só se pode avançar **1 nível por vez** (+4 espaços). Pular de 0 para 8 é erro.
- Dedent precisa bater exatamente com algum nível já aberto na pilha, senão é
  "indentação inconsistente".

```
if (nota >= 7):
    post("Aprovado")
else:
    post("Reprovado")
```

Misturar os dois estilos no mesmo arquivo emite (stderr, não interrompe a
execução):

```
[poolscript:warn] mistura blocos com chaves {} e dois-pontos (:) no mesmo arquivo — padronize um único estilo
```

Dentro de um bloco `{}`, blocos filhos **também precisam usar `{}`**
(indentação é ignorada enquanto há `{` aberto).

---

## Tipos e variáveis

```
x = 10                 # não-tipada — dinâmica, tipo pode mudar depois
str nome = "Pool"       # tipada — só aceita valor desse tipo (ou conversível)
int idade = 20
flo altura = 1.75
bool ativo = true
```

Tipos primitivos: `str`, `int`, `flo`, `bool`. Redeclarar uma variável **já
tipada** no mesmo escopo é erro (`OutputUnexpectedValues`). Atribuir um valor
incompatível a uma variável tipada é erro (`AtributtedValueError`, ou
`ConversionError` quando a conversão automática — ex.: string → int — falha).

`input()` sempre devolve `str`; ao declarar com tipo (`int n = input()`) o
valor é convertido automaticamente (e falha com erro claro se não for
conversível).

---

## Null

Quatro grafias equivalentes, todas o mesmo valor:

```
Null   null   None   none
```

`Null == 0` é `True` (igualdade "nullish", só para `==`; comparações de
magnitude `<`/`>` com `Null` são sempre `False`). `post(Null)` imprime `null`
— igual a qualquer valor `None`/ausente vindo de índice fora do limite, de
builtins sem resultado, ou de `return` explícito/implícito.

---

## Strings

```
"aspas duplas"
'aspas simples'
```

Escapes reconhecidos: `\n \t \r \\ \" \'`. **Qualquer outro** `\X` (ex.: `\U`,
`\l`, `\A`) **perde o backslash silenciosamente** e mantém só a letra — não é
erro. Isso é comum em outras linguagens C-like, mas costuma pegar quem embute
caminhos do Windows direto numa string (`"C:\Users\..."` vira `"C:Users..."`,
com `\t` virando um TAB literal). Para caminhos/regex, use **string raw**:

```
r"C:\Users\nome"     # preserva tudo literalmente, sem processar escapes
r'C:\Users\nome'
```

**f-string** (interpolação com chaves):

```
grau = 25
post(f"Clima: {grau} graus")
```

**Concatenação com interpolação** (sem `f`, útil em `post(...)` com vários
argumentos espaçados — chamadas em PoolScript aceitam argumentos separados só
por espaço, sem vírgula):

```
post("Clima: " {grau} " graus")
```

**String colorida** (ANSI 24-bit), hex de **exatamente 3 ou 6 dígitos** ou
nome conhecido:

```
post(<red>"alerta")
post(<2196f3>"azul")
post(<fff>"branco")
```
Nomes disponíveis: `red, green, blue, yellow, cyan, magenta, white, black,
purple, orange, pink, gray, grey, lime, teal`. Um hex de 4 ou 5 dígitos, ou um
nome desconhecido, **não** é tratado como cor — o `<` volta a ser o operador
"menor que".

---

## Listas, tuplas e dicionários

```
lista = [1, 2, 3]
tupla = (1, 2, 3)            # imutável (implementada como tuple do Python)
dicionario = {"a": 1, "b": 2}
```

Slices: `lista[1:3]`, `lista[::-1]`, etc. Índice fora do intervalo **não
quebra** — emite `IndexOutOfBoundsWarning` no stderr e o valor vira `Null`.

Builtins de lista: `addEnd(l, v)`, `removeEnd(l)`, `addStart(l, v)`,
`removeStart(l)` (em lista vazia, `removeEnd`/`removeStart` retornam `Null`).

---

## Unpacking (desempacotamento de tuplas)

Funciona como no Python — atribuição múltipla a partir de uma lista/tupla/
string do lado direito:

```
a, b = 1, 2                  # múltiplo direto
a, b = b, a                  # swap — o lado direito é avaliado por INTEIRO
                              # antes de qualquer variável ser escrita
lista = [1, 2, 3]
a, b, c = lista               # a partir de uma variável
a, (b, c) = 1, (2, 3)         # aninhado
a, b = "hi"                   # string também é iterável: a="h", b="i"
```

**Star / rest**, igual ao Python:

```
a, *resto = [1, 2, 3, 4]      # a=1, resto=[2, 3, 4]  (resto é sempre list)
*inicio, z = [1, 2, 3, 4]     # inicio=[1, 2, 3], z=4
a, *meio, z = [1, 2, 3, 4, 5] # a=1, meio=[2, 3, 4], z=5
a, = [5]                      # vírgula final — desempacota 1 elemento
```

Regras (idênticas ao Python):
- No máximo **um** `*alvo` por nível de aninhamento — dois `*` no mesmo nível
  é erro de sintaxe.
- `*resto = [...]` **sem vírgula nenhuma** é erro de sintaxe — precisa de
  `*resto, = [...]`.
- Aridade errada sem `*` (`a, b = [1, 2, 3]` ou `a, b, c = [1, 2]`) levanta
  `PoolRuntimeError` (`OutputUnexpectedValues`) com mensagem estilo Python
  ("valores insuficientes"/"valores demais para desempacotar").
- Lado direito que não é lista/tupla/string (`a, b = 5`) levanta
  `SomeValueUnexpected`.
- Cada alvo (novo ou já existente no escopo) segue a mesma semântica da
  atribuição simples: cria se não existir, sobrescreve se já existir.

**Fora do escopo (não suportado):** `self.x`/`obj.y` como alvo de unpacking;
unpacking em declaração tipada (`str a, b = ...`); unpacking no `for each`
(que continua aceitando só 1 nome: `for each x in lista`).

---

## Operadores

```
+  -  *  /  %          # aritméticos
+= -= *= /= %=          # atribuição composta
++  --                  # incremento/decremento (só em variáveis)

==  !=  ===  !==        # igualdade (=== é comparação "estrita")
<  >  <=  >=            # magnitude

and  or  not            # lógicos (também aceitam && / || como sinônimos)
is  is not  not is      # comparação de tipo/identidade (`x is int`, `x is not Null`)
in  not in              # pertencimento (`v in lista`, `v not in lista`)
```

---

## Controle de fluxo

```
if (cond) { ... } elif (cond2) { ... } else { ... }

while (cond) {
    ...
    break
    continue
}

for each item in iteravel {
    ...
}
```

`for each` aceita **só um nome** (sem unpacking — `for each k, v in ...` não
existe). Itera listas, tuplas, strings (char a char) e geradores.
`break`/`continue` funcionam normalmente mesmo dentro de um `if` aninhado no
corpo do loop.

---

## Funções: `action` / `reaction`

`action` e `reaction` são sinônimos na declaração de função:

```
action soma(a, b) {
    return a + b
}

action saudacao(nome="Visitante") {   // parâmetro com default
    return "Olá, " nome
}
```

**Tipo de retorno opcional** (`int`/`bool`) muda o comportamento em caso de
erro dentro da função — em vez de propagar a exceção, devolve um valor
"sentinela" (útil para handlers HTTP-like):

```
int reaction f() { return 1 / 0 }
post(f())     # 500 (em vez de propagar o erro)

bool reaction g() { return 1 / 0 }
post(g())     # False
```

Sem tipo de retorno declarado, o erro propaga normalmente (pode ser pego com
`try/catch`).

**Lambda / função anônima**: `action(params) { ... }` sem nome, atribuível a
uma variável.

**Decorator `@NonNull`**: valida que nenhum argumento passado é `Null`,
levanta erro se for:

```
@NonNull
action precisa(v) {
    return v
}
precisa(Null)   // erro
```

**`global`**: dentro de uma `action`/`reaction`, declara que um nome se
refere à variável do escopo global — leituras e escritas passam a atingir
direto o global, em vez de criar/usar uma variável local (igual ao `global`
do Python):

```
contador = 0

action incrementar() {
    global contador
    contador = contador + 1
}

incrementar()
incrementar()
post(contador)   // 2
```

Também funciona para criar uma variável global que ainda não existe:

```
action registrar() {
    global total_visitas
    total_visitas = 1
}

registrar()
post(total_visitas)   // 1 — visível fora da action também
```

---

## `async` / `await`

`async action`/`async reaction` executam em uma thread separada
(`ThreadPoolExecutor`) e devolvem imediatamente um **future** (`PoolFuture`) —
a chamada em si nunca bloqueia:

```
async action lenta(n) {
    sleep(0.3)
    return n
}

f = lenta(1)          // não bloqueia — devolve um future na hora
post(await f)          // só aqui bloqueia até o resultado
```

Semântica (igual à de qualquer linguagem com async/await real, verificada com
testes de concorrência por tempo de parede):

- Duas ou mais chamadas `async` disparadas antes de qualquer `await` rodam
  **em paralelo** (não seriado).
- Exceção dentro de uma `async action` **não aparece na chamada** — só
  aparece quando você dá `await` (ou `.result()`), convertida em
  `PoolRuntimeError`.
- `await` numa **lista** funciona como um `gather`: aguarda todos os
  futures da lista (misturados com valores já prontos) e devolve a lista de
  resultados; se algum tiver dado erro, o erro propaga no `await`.
- `await` encadeado (uma `async action` chamando/aguardando outra `async
  action`) e recursão assíncrona funcionam normalmente.
- Métodos `async` dentro de `Entity` funcionam e acessam/mutam `self`
  normalmente.
- `await valor_comum` (não-future) é pass-through — devolve o valor como
  está, não é erro.
- API de baixo nível do future: `f.done()` (bool), `f.result(timeout=None)`
  (mesmo que `await`, mas você escolhe o timeout — estoura
  `PoolRuntimeError` se não terminar a tempo).
- `async int reaction`/`async bool reaction` aplicam a mesma conversão de
  erro→sentinela (500/False) que a versão síncrona, só que dentro da thread.

---

## Geradores (`yield`)

Qualquer `action` que contenha `yield` em qualquer lugar do corpo (inclusive
dentro de `if`/`while` aninhados) vira um gerador automaticamente — chamá-la
não executa o corpo na hora, devolve um objeto iterável:

```
action contar(n) {
    i = 0
    while (i < n) {
        yield i
        i = i + 1
    }
}

for each v in contar(3) {
    post(v)      // 0, 1, 2
}
```

`break` no `for each` consumidor para de puxar valores do gerador (não
executa o resto do corpo da função geradora).

---

## `try` / `catch` / `finally`

**Precisa de pelo menos um `catch`** — `try { ... } finally { ... }` sem
nenhum `catch` **não é sintaxe válida** (diferente do Python).

```
try {
    x = 1 / 0
} catch (SomeValueUnexpected e) {
    post("erro tipado: " e)
} catch (e) {
    post("qualquer outro erro: " e)
} finally {
    post("sempre roda")
}
```

- `catch (Tipo nome)` só entra se o `code` do erro bater com `Tipo` (ver
  [Erros nomeados](#erros-nomeados)).
- `catch (nome)` sem tipo captura qualquer erro.
- `finally` sempre executa, com ou sem erro, e mesmo se nenhum `catch` bateu
  (nesse caso o erro original é relançado depois do `finally`).
- `try`/`catch` podem ser aninhados; um erro não capturado por um `catch`
  interno propaga para um `try` externo.

`raise "mensagem"` levanta um erro genérico com essa mensagem.

---

## `match` / `case`

```
match valor:
    case 200:
        post("ok")
    case "Sabado" | "Domingo":
        post("fim de semana")
    case v if v < 50:
        post("barato")
    case _:
        post("outro")
```

- `case A | B | C:` casa qualquer um dos valores.
- `case nome if condicao:` faz *binding* do valor em `nome` e só casa se a
  guarda for verdadeira.
- `case _:` é o coringa (default).
- Se nenhum `case` bater, o `match` simplesmente não faz nada (não é erro).

---

## `model`

Valida a estrutura de um dict (tipicamente um JSON recebido):

```
model Usuario() {
    nome: str(length=60)
    idade: int(length=3)
    ativo: bool
}

data = {"nome": "ana", "idade": 20, "ativo": true}

if (data == Usuario) {
    post("Dados válidos!")
} else {
    post("Dados inválidos!")
}
```

Campo ausente, tipo errado, ou string/int que excede o `length` declarado →
inválido. Tipos aceitos: `str(length=N)`, `int(length=N)`, `flo`, `bool`.

---

## `Entity` (classes)

```
Entity Animal():
    action __init__(self, nome):
        self.nome = nome
    action falar(self):
        return "..."

Entity Cachorro(Animal):
    action __init__(self, nome):
        base(nome)             // chama __init__ do pai
    action falar(self):
        return "Au!"

c = Cachorro("Rex")
post(c.nome)      // Rex
post(c.falar())   // Au!
```

- Herança múltipla é suportada; `base(NomePai, args...)` escolhe o pai
  explicitamente quando há mais de um.
- Herança encadeada (A → B → C) funciona normalmente, cada nível chamando
  `base(...)` para o `__init__` do pai imediato.
- `@static` — método de classe, chamado sem instância: `Util.metodo(...)`.
- `@dataentity` (via `from datasentity import dataentity, asdict, astuple,
  aslist, asjson`) — gera `__init__` automático a partir de campos tipados
  (`nome: str`, `idade: int`, ...) e helpers de conversão:

```
from datasentity import dataentity, asdict, astuple, aslist, asjson
@dataentity
Entity P():
    nome: str
    idade: int

p = P(nome="Ana", idade=30)
post(asdict(p)["nome"])    // Ana
post(astuple(p)[1])         // 30
post(aslist(p)[0])          // Ana
```

---

## `count` / `count each`

Operador multifuncional para contar/checar ocorrências em coleções, strings e
números. Três formas equivalentes:

```
count int(7) in nums          // prefixo
int(7) count in nums          // infixo
count int in nums             // sem valor — conta todos do tipo
```

`count` devolve `int` (`0` é falsy, funciona direto em `if`).

**Bloco contador** — itera as ocorrências, definindo `_match`, `_index`,
`_count` a cada uma:

```
count each int(7) in nums {
    post(_index)
}
```

Dentro do bloco, `return;` (vazio) **acumula** e devolve o total ao final
para a `action` que envolve o `count each`; `return valor` interrompe
imediatamente (curto-circuito) e devolve `valor`.

---

## `using`

Gerenciador de contexto — fecha o recurso automaticamente ao sair do bloco
(mesmo em caso de erro):

```
using open("arquivo.txt", "w") as f {
    f.write("linha 1\n")
}
// arquivo já está fechado aqui
```

---

## Imports e bibliotecas padrão

```
import os
from os import getenv
PUSH os GET getenv        // forma alternativa
import os as sistema      // alias
```

Libs embutidas (lazy-loaded, só carregam quando importadas):
`os, json, dotenv, mail, date, jinker, db, hash, jwt, request/requests,
manpu/mp, regex, sqlite3, qrcode/qr, sys, datasentity/dataentity`.

Referência completa de cada lib (todo membro acessível, com exemplos) em
`docs/`:

| Lib | Doc |
|---|---|
| `os`, `dotenv`, `mail`, `date`, `request`/`requests` (+ lambda/map/filter) | [`docs/libs_utilitarias.md`](docs/libs_utilitarias.md) |
| `jinker` (HTTP + WebSocket com salas) | [`docs/jinker.md`](docs/jinker.md) |
| `db` (SQLite/Postgres/MySQL/Mongo) + `sqlite3` (acesso direto) | [`docs/db.md`](docs/db.md) |
| `hash`, `jwt` | [`docs/hash_jwt.md`](docs/hash_jwt.md) |
| `manpu`/`mp` | [`docs/manpu.md`](docs/manpu.md) |
| `json` | [`docs/json.md`](docs/json.md) |
| `regex` | [`docs/regex.md`](docs/regex.md) |
| `qrcode`/`qr` | [`docs/qrcode.md`](docs/qrcode.md) |
| `sys` | [`docs/sys.md`](docs/sys.md) |
| `datasentity`/`dataentity` | [`docs/datasentity.md`](docs/datasentity.md) |
| `Parsing` (builtin global, sem import) | [`docs/parsing.md`](docs/parsing.md) |

`Parsing` (conversões de tipo seguras) é builtin global, não precisa de
`import` — ver `docs/parsing.md`.

Libs "stub" (existem mas não implementadas nesta versão — erro claro e
catchable ao chamar, não ao importar): `sqlite, smtplib, mimetext,
multipart, flask`.

Também é possível importar arquivos `.ps` próprios, resolvidos relativos à
raiz do projeto (igual a um pacote Python).

---

## Builtins globais

```
post(...)            // imprime (join dos argumentos com espaço) e guarda no output
post.flush(texto, delay=0.05)   // efeito de digitação, char a char
input(prompt="")     // sempre devolve str

open(path, modo)     // FileHandle
len(x)  range(...)  type(x)
str(x)  int(x)  flo(x)  bool(x)   // conversores

id(x)  hex(n)  bin(n)  oct(n)  ord(c)  chr(n)
abs(n)  round(n)  sum(lista)  min(...)  max(...)
sorted(lista)  reversed(lista)  enumerate(lista)  zip(a, b, ...)

addEnd(l, v)  removeEnd(l)  addStart(l, v)  removeStart(l)
map(lista, fn)  filter(lista, fn)   // atenção: lista vem PRIMEIRO, depois a função

sleep(segundos)      // pausa a execução (útil dentro de async action)
gather(...)          // agrega múltiplos futures
```

Métodos de string/número (chamados com `.`): `.isdigit()`, `.isalpha()`,
`.upper()`, `.lower()`, `.get_json(...)`, etc. — inteiros/floats (exceto
`bool`) ganham os métodos de string automaticamente via conversão implícita
(`x = 150; x.isdigit()` → `True`), já que `input()` costuma devolver dígitos
como string convertida.

---

## Erros nomeados

Os erros de runtime da PoolScript têm um `code` estável (usado por
`catch (Tipo nome)` e disponível como `.code` no objeto de exceção Python
`PoolRuntimeError`):

| `code` | Quando ocorre |
|---|---|
| `AtributtedValueError` | Valor incompatível atribuído a variável tipada (ex.: `str x = 10`) |
| `OutputUnexpectedValues` | Redeclaração no mesmo escopo; aridade errada em unpacking |
| `SomeValueUnexpected` | Operação inválida entre tipos, divisão por zero, valor inválido, RHS não-iterável em unpacking |
| `IndexOutOfBoundsWarning` | Índice fora do intervalo (não-fatal — vira warning + `Null`) |
| `ConversionError` | Conversão automática de tipo falhou (ex.: `int x = "abc"`) |
| `NotImplemented` | Lib "stub" chamada (existe mas função ainda não implementada) |
| `KeyError` / `IndexError` / `TimeoutError` / `NetworkError` / `IOError` / etc. | Espelham exceções nativas do Python, "blindadas" (sem expor stack trace interno) |

Todo erro de runtime nativo do Python (`ZeroDivisionError`, `TypeError`,
`FileNotFoundError`, ...) passa por um "shield" que o converte na mesma
classe pública `PoolRuntimeError` — então `except PoolRuntimeError` (Python)
ou `catch (e)` (PoolScript) sempre pegam qualquer erro de runtime,
independente da causa interna.

---

## Limitações e comportamentos conhecidos

Encontradas e documentadas durante uma auditoria profunda desta versão
(testes + correções, ver `CHANGELOG.md`):

- **String bruta imediatamente seguida de `{` em estilo chave é ambígua**:
  `for each c in "abc" { ... }` tenta interpretar o `{` como início de
  interpolação (`"texto" {expr}`) em vez de abrir o bloco do loop. Contorne
  usando o estilo `:` (`for each c in "abc":`) ou uma variável em vez do
  literal direto.
- **`elif`/`else` no estilo chave precisam ficar na mesma linha** do `}`
  anterior (`} elif (...) {`), não numa linha nova.
- **`try`/`finally` exige pelo menos um `catch`** — não existe `try {...}
  finally {...}` puro.
- **Escapes de string desconhecidos perdem o backslash silenciosamente**
  (não é erro) — cuidado ao embutir caminhos do Windows sem usar string raw
  (`r"..."`).
- Unpacking não se aplica a `for each` (continua aceitando só 1 nome), a
  declarações tipadas, nem a alvos de membro (`self.x`/`obj.y`).
