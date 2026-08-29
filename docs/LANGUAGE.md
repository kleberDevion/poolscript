# PoolScript v8.2.18 — Referência da linguagem

Este documento descreve, de forma completa, tudo que existe atualmente na linguagem
PoolScript: sintaxe, tipos, controle de fluxo, funções, classes, tratamento de
erros, bibliotecas padrão e comportamentos/limitações conhecidas. Substitui
`docs/PoolScript.md` (que documentava a v1.0.8 e está bastante desatualizado —
não cobre `async`/`await`, `Entity`, `match`/`case`, `count`, decorators nem
unpacking).

> Arquitetura (para quem for mexer no código): lexer (`vm/ps_lexer.c`) →
> parser recursive-descent que produz uma AST (`vm/ps_parser.c`) → compilador
> pra bytecode (`vm/ps_compiler.c`) → máquina virtual (`vm/poolscript_vm.c`).
> Tudo em C, sem dependência de runtime externo.

---

## Índice

1. [Rodando código](#rodando-código)
2. [Comentários](#comentários)
3. [Blocos: `{ }`](#blocos--)
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
pool --version        # versão do binário
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

## Blocos: `{ }`

O bloco da PoolScript é `{ }`. **Só.** Não existe bloco por indentação com `:`
— tentar usar dá um erro que diz o que fazer:

```
SyntaxError: bloco com ':' nao existe mais — use '{ }'
```

Isso já foi diferente: a linguagem aceitava os dois estilos e permitia
misturá-los. A convivência custou caro — quase toda regressão de parser saía da
interação entre indentação e chave — e o `:` saiu.

Dentro de `{ }` a **indentação não tem significado**: quem delimita é a chave.
Indente como quiser (o repositório usa 4 espaços por nível, por costume).

A chave de abertura pode ficar **na mesma linha** ou **na linha seguinte**, e
`elif`/`else`/`catch`/`finally` podem vir colados ao `}` ou numa linha nova —
as quatro combinações valem:

```
if (nota >= 9) {
    post("Excelente!")
} elif (nota >= 7) {
    post("Bom!")
} else {
    post("Reprovado")
}
```

```
if (nota >= 9)
{
    post("Excelente!")
}
else
{
    post("Reprovado")
}
```

O `:` continua sendo `:` onde ele **não** abre bloco — e essas formas não
mudaram:

```
d = { "a": 1, "b": 2 }     // dicionário
s = "abcdef"[1:3]          // fatia
Entity P() { nome: str }   // campo tipado
```

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
incompatível a uma variável tipada é erro (`AttributedValueError`, ou
`ConversionError` quando a conversão automática — ex.: string → int — falha).

`input()` sempre devolve `str`; ao declarar com tipo (`int n = input()`) o
valor é convertido automaticamente (e falha com erro claro se não for
conversível).

No **fim da entrada** (EOF — o outro lado fechou, ou a entrada veio de um
arquivo que acabou), `input()` devolve **`null`**, não `""`. Uma linha vazia de
verdade continua devolvendo `""`, então dá pra distinguir as duas e um laço de
leitura sabe onde parar:

```ps
while true {
    linha = input()
    if linha == Null {
        break          // acabou a entrada
    }
    post("li:", linha)
}
```

### Palavras reservadas não podem virar nome

Como no Python, uma palavra reservada é recusada onde um nome seria **ligado** —
variável, parâmetro, `action`, `Entity`, `model`, campo, variável de loop, de
`catch`, de `global` e alvo de desempacotamento:

```
if = 5                       // erro: 'if' é palavra reservada
action f(while) { ... }      // erro: 'while' é palavra reservada
for each return in xs { }    // erro: 'return' é palavra reservada
```

`base` entra na lista mesmo não sendo keyword: é a chamada da superclasse
dentro de `__init__`, então ligá-la a um nome quebrava `base(...)` — antes
`base = 5` era aceito calado e só estourava depois, como "variável não
definida".

Duas coisas que **continuam** válidas, porque não ligam nome nenhum:

```
o.base                             // acesso a membro chamado 'base'
db.query(base="x.db")              // nome de argumento é só um rótulo
regex.sub("a", "b", s, count=2)    // idem, mesmo sendo 'count'
```

`self` é reservada, mas liberada no único lugar onde faz sentido: a lista de
parâmetros de um método.

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

**String multi-linha** — aspas simples **triplas** (`'''...'''`), combina com
`f`/`r`. Aspas duplas triplas (`"""`) já são comentário de bloco (ver
[Comentários](#comentários)), por isso multi-linha usa `'''` e não `"""`:

```
str sql = '''SELECT *
FROM users
WHERE ativo = 1'''

f'''Olá {nome}!
Segunda linha.'''

r'''C:\Users\nome
sem processar escape'''
```

**f-string** (interpolação com chaves — aspas simples e duplas valem igual,
como em qualquer string):

```
grau = 25
post(f"Clima: {grau} graus")
post(f'Clima: {grau} graus')     // mesma coisa
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

|  ^  &  ~  <<  >>      # bitwise — só entre int (bool/flo/str dão erro)
```

Os bitwise seguem a precedência do Python — do mais fraco pro mais forte:
`|`, `^`, `&`, `<<`/`>>`. Ligam mais forte que comparação e mais fraco que
`+`/`-`, então `2 + 3 << 1` é `(2 + 3) << 1` = `10`. `~` é unário.

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
match valor {
    case 200 {
        post("ok")
    }
    case "Sabado" | "Domingo" {
        post("fim de semana")
    }
    case v if v < 50 {
        post("barato")
    }
    case _ {
        post("outro")
    }
}
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

`Entity`, `class` e `Class` são **a mesma keyword** — três grafias do mesmo
recurso. Podem se misturar no mesmo arquivo e herdar entre si sem restrição.
Daqui pra frente o texto usa `class`, mas tudo vale igual para os três.

### Declaração e instância

```
class Animal() {
    action __init__(self, nome) {
        self.nome = nome
    }
    action falar(self) {
        return f"{self.nome} faz um som"
    }
}

a = Animal("Bicho")     // chama __init__ automaticamente
post(a.nome)            // Bicho
post(a.falar())         // Bicho faz um som
```

Regras concretas:

- **Os parênteses após o nome são obrigatórios**, mesmo sem herança:
  `class Animal()` — `class Animal` sozinho é erro de sintaxe.
- **`__init__` é opcional.** Sem ele, a instância nasce sem atributos e você
  os cria depois (`obj.x = ...`) ou só usa os métodos.
- **Todo método declarado precisa de `self` como primeiro parâmetro** — é o
  próprio objeto. Chamar `obj.metodo(a, b)` injeta `self` automaticamente; você
  passa só `a, b`. Um método sem `self` é erro em tempo de execução.
- **Atributos vivem em `self.x`.** Não existe declaração prévia de campos (a
  não ser via `@dataentity`, abaixo) — atribuir `self.x = valor` dentro de
  qualquer método cria/atualiza o atributo.
- **`post(obj)`** imprime `<NomeDaClasse {atributos}>`.

### `private` / `public` — encapsulamento

Um campo ou método pode ser marcado `private` (ou `public`, que é o **default**).
Membro `private` só é acessível **de dentro de um método da própria classe** —
tentar ler, escrever ou chamar de fora levanta erro:

```
Entity Conta() {
    private saldo: int = 0          // só a própria classe mexe
    public dono: str = "kleber"     // público (igual a não pôr nada)

    public reaction deposita(self, v) {
        self.saldo = self.saldo + v   // OK: dentro da classe
        return self.saldo
    }
    private reaction _log(self) { return "interno" }   // só a classe chama
}

c = Conta()
post(c.deposita(100))   // 100  — via método público
post(c.dono)            // kleber — público
post(c.saldo)           // ERRO: 'saldo' é private de Conta
c._log()                // ERRO: 'private'
c.saldo = 9             // ERRO: escrita em private de fora
```

- **Default é público** — código sem modificador funciona como sempre.
- Encapsulamento é **disciplina de código**, não blindagem de segurança: não
  protege contra debugger, dump de memória ou processo externo lendo a RAM
  (nenhuma linguagem faz isso — nem o `private` do Java). Serve pra forçar o
  acesso pela API pública, não pra esconder segredo de um atacante com acesso à
  máquina.
- Campo `private` precisa ser **declarado tipado** no corpo (`private x: tipo`);
  atributos criados só por `self.x = ...` num método são públicos.

### Herança simples e `base()`

`base(...)` chama o `__init__` do **pai imediato**. Só funciona dentro de um
`__init__` (fora dele dá erro "base() só pode ser chamado dentro de um
__init__ de Entity").

```
class Cachorro(Animal) {
    action __init__(self, nome, raca) {
        base(nome)          // executa Animal.__init__(self, nome)
        self.raca = raca    // e aí adiciona o atributo próprio
    }
    action falar(self) {     // sobrescreve o falar do pai
        return f"{self.nome} ({self.raca}) late"
    }
}

c = Cachorro("Rex", "vira-lata")
post(c.falar())     // Rex (vira-lata) late
post(c.nome)        // Rex   — atributo herdado, criado pelo base()
```

Método **não sobrescrito** é herdado direto — `Gato` abaixo não define
`falar`, então usa o do `Animal`:

```
class Gato(Animal) {
    action __init__(self, nome) {
        base(nome)
    }
}

post(Gato("Felix").falar())   // Felix faz um som   (veio de Animal)
```

Cadeia de qualquer profundidade funciona; cada nível chama o `base()` do seu
pai imediato:

```
class Base() {
    action __init__(self, x) {
        self.x = x
    }
}
class Meio(Base) {
    action __init__(self, x, y) {
        base(x)          // Base.__init__
        self.y = y
    }
}
class Topo(Meio) {
    action __init__(self, x, y, z) {
        base(x, y)       // Meio.__init__
        self.z = z
    }
}

t = Topo(1, 2, 3)
post(t.x, t.y, t.z)      // 1 2 3
```

### Herança múltipla e `base(Pai, ...)`

Com mais de um pai, `base(...)` sozinho miraria só o **primeiro** pai da lista.
Para escolher um pai específico, passe o nome dele como primeiro argumento:
`base(NomePai, args...)`.

```
class Motor() {
    action __init__(self, cavalos) {
        self.cavalos = cavalos
    }
}
class Roda() {
    action __init__(self, qtd) {
        self.qtd_rodas = qtd
    }
}

class Carro(Motor, Roda) {
    action __init__(self) {
        base(Motor, 300)     // mira Motor, passa 300
        base(Roda, 4)        // mira Roda, passa 4
        self.tipo = "esportivo"
    }
}

c = Carro()
post(c.cavalos, c.qtd_rodas, c.tipo)   // 300 4 esportivo
```

> ⚠️ **Armadilha real (verificada):** o alvo do pai só é reconhecido quando há
> uma **vírgula** depois do nome. `base(Motor, 300)` funciona. Mas
> `base(Motor)` — nome sozinho, sem vírgula — **NÃO** mira o pai `Motor`: o
> parser trata `Motor` como um *argumento comum* passado para o primeiro pai, e
> você recebe um erro tipo `esperava até 0 argumento(s), recebeu 1`.
>
> Para mirar um pai que **não recebe argumentos**, use a **vírgula final**:
> `base(Motor,)` — mira `Motor` com zero args. Isso funciona:
>
> ```
> class A():
>     action __init__(self):
>         self.a = 1
> class C(A, B):
>     action __init__(self):
>         base(A,)     // vírgula final = mira A, sem argumentos
>         base(B,)
> ```

Resolução de métodos (não-`__init__`) na herança múltipla é **esquerda-para-
direita**: `class C(A, B)` procura o método primeiro em `C`, depois em `A` (e
toda a cadeia de `A`), depois em `B`. O primeiro encontrado vence.

### `@static` — método sem instância

Método marcado com `@static` é chamado direto na classe, sem criar objeto e
sem `self`:

```
class Util() {
    @static
    action dobro(n) {
        return n * 2
    }
}

post(Util.dobro(21))    // 42   — sem instanciar Util
```

### `@NonNull` — barra argumentos Null

Aplicado a um método, faz a chamada falhar (erro `@NonNull: parâmetro '...' não
pode ser Null`) se qualquer argumento recebido for `Null`/`None`.

```
class Calc() {
    action __init__(self) {
        self.total = 0
    }
    @NonNull
    action somar(self, valor) {
        self.total = self.total + valor
        return self.total
    }
}

c = Calc()
c.somar(5)        // ok
c.somar(Null)     // ERRO: @NonNull: parâmetro 'valor' em 'somar' não pode ser Null
```

**Escopo (por design):** `@NonNull` vale para chamadas de método normais
(`obj.metodo(...)`), métodos `@static` e funções soltas. Ele **não** dispara
no `__init__` durante a instanciação — isso é intencional, não uma falha:
`Null` é usado como sentinela interna de "campo sem default" (ver
`@dataentity`), então barrar `Null` no construtor conflitaria com esse
mecanismo. Da mesma forma, `@NonNull` não rejeita um valor **default** que seja
`Null`. Se quiser validar um campo obrigatório no construtor, faça a checagem à
mão dentro do `__init__` (`if dono is Null: raise ...`).

### `@dataentity` — `__init__` automático + conversões

Gera o `__init__` sozinho a partir de campos tipados declarados no corpo (no
formato `nome: tipo`, opcionalmente `nome: tipo = default`). Precisa importar
de `datasentity`:

```
from datasentity import dataentity, asdict, astuple, aslist, asjson

@dataentity
class Pessoa() {
    nome: str
    idade: int = 18        // default opcional
}

p = Pessoa(nome="Ana", idade=30)    // __init__ gerado — aceita kwargs
q = Pessoa(nome="Léo")               // idade cai no default 18

post(asdict(p))     // {"nome": "Ana", "idade": 30}
post(astuple(p))    // ("Ana", 30)
post(aslist(p))     // ["Ana", 30]
post(asjson(p))     // string JSON
post(q.idade)       // 18
```

Detalhes de `@dataentity`:

- Campos **sem default** viram argumentos obrigatórios; passar menos que o
  necessário é erro.
- Se você declarar seu **próprio** `__init__` no corpo, ele tem prioridade — o
  automático só é gerado quando não há `__init__` escrito à mão.
- `asdict`/`astuple`/`aslist`/`asjson` seguem a **ordem de declaração** dos
  campos.

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
| `os`, `dotenv`, `mail`, `date`, `request`/`requests` (+ lambda/map/filter) | [`docs/libs_utilitarias.md`](libs_utilitarias.md) |
| `jinker` (HTTP + WebSocket com salas) | [`docs/jinker.md`](jinker.md) |
| `db` (SQLite/Postgres/MySQL/Mongo, alias de `psodbc`) + `sqlite3` (acesso direto) | [`docs/psodbc.md`](psodbc.md) |
| `hash`, `jwt` | [`docs/hash_jwt.md`](hash_jwt.md) |
| `manpu`/`mp` | [`docs/manpu.md`](manpu.md) |
| `json` | [`docs/json.md`](json.md) |
| `regex` | [`docs/regex.md`](regex.md) |
| `qrcode`/`qr` | [`docs/qrcode.md`](qrcode.md) |
| `sys` | [`docs/sys.md`](sys.md) |
| `datasentity`/`dataentity` | [`docs/datasentity.md`](datasentity.md) |
| `Parsing` (builtin global, sem import) | [`docs/parsing.md`](parsing.md) |

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

open(path, mode="r", encoding="utf-8")     // FileHandle: .read() .readlines() .readline() .write(t) .writelines(l) .close()
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

---

## Métodos

Chamados com ponto. Estão listados aqui inteiros porque não há como descobrir
quais existem lendo código — e "etc." não é documentação.

### Em qualquer valor

| | Devolve |
|---|---|
| `.type()` | nome do tipo: `str` `int` `flo` `bool` `list` `dict` `tup` `Null` `action`, ou o nome da Entity |

Mesmos nomes que o builtin `type(x)` — os dois já discordaram (`json` contra
`dict`), hoje não discordam mais.

### String

Tudo trabalha em **caractere**, não em byte: `"ção".len()` é 3, e
`"ção"[0]` não parte o UTF-8 no meio.

**Caixa** — cobre acento (`ç à é õ`), não só ASCII.

| | |
|---|---|
| `.upper()` `.lower()` | caixa alta / baixa |
| `.title()` | primeira letra de cada palavra; dígito quebra palavra, então `"a1b".title()` é `"A1B"` |
| `.capitalize()` | só a primeira letra do texto |
| `.swapcase()` | inverte a caixa de cada letra |
| `.casefold()` | hoje igual a `.lower()` |

**Aparar** — o argumento é um **conjunto de caracteres**, não um prefixo:
`"xyx".strip("xy")` come qualquer um dos dois, em qualquer ordem.

| | |
|---|---|
| `.strip(chars?)` `.lstrip(chars?)` `.rstrip(chars?)` | sem argumento, apara branco |

**Buscar** — `find` devolve `-1` quando não acha; `index` levanta erro.

| | |
|---|---|
| `.startswith(p)` `.endswith(s)` | bool |
| `.contains(sub)` `.has(sub)` | bool — grafias equivalentes |
| `.find(sub)` `.rfind(sub)` | posição, ou `-1` |
| `.index(sub)` `.rindex(sub)` | posição, ou **erro** |
| `.count(sub)` | ocorrências sem sobreposição |
| `.len()` | caracteres |

**Testar conteúdo** — todos dão `False` em string vazia. `isupper`/`islower`/
`istitle` exigem pelo menos uma letra com caixa.

| | |
|---|---|
| `.isalpha()` `.isdigit()` `.isnumeric()` `.isdecimal()` `.isalnum()` | |
| `.isspace()` `.isupper()` `.islower()` `.isascii()` `.istitle()` `.isprintable()` | |

**Quebrar e juntar** — `split()` sem separador quebra em qualquer corrida de
branco e descarta as pontas vazias; **com** separador, cada ocorrência gera um
campo, inclusive vazio. São regras diferentes de propósito.

| | |
|---|---|
| `.split(sep?, max?)` `.rsplit(...)` | lista |
| `.splitlines()` | `\r\n` conta como uma quebra só |
| `.join(lista)` | o alvo é o separador: `"-".join(["a","b"])` |
| `.replace(velho, novo?, quantas?)` | alvo vazio casa em toda fronteira: `"a".replace("","x")` é `"xax"` |

**Partir e cortar borda** — `partition` devolve sempre 3 partes.

| | |
|---|---|
| `.partition(sep)` `.rpartition(sep)` | `(antes, sep, depois)`; sem o separador o texto vai pra frente no `partition` e pro fim no `rpartition` |
| `.removeprefix(p)` `.removesuffix(s)` | devolve intacto se não bate |

**Preencher** — largura em caractere.

| | |
|---|---|
| `.ljust(n, ch?)` `.rjust(n, ch?)` | |
| `.center(n, ch?)` | sobra ímpar vai pra **esquerda** quando largura e folga são ambas ímpares — `"ab".center(5)` é `"  ab "` |
| `.zfill(n)` | zeros **depois** do sinal: `"-5".zfill(4)` é `"-005"` |
| `.expandtabs(n=8)` | TAB vira espaço até a próxima parada |

**Regex** — ver [regex](#imports-e-bibliotecas-padrão) para o que o motor cobre.

| | |
|---|---|
| `.match(padrao)` | bool, casando a string **inteira** |
| `.findall(padrao)` | lista |
| `.sub(padrao, troca)` | `\1`..`\9` na troca viram os grupos |

**Número recebe método de string** por conversão automática: `(150).isdigit()`
vale sem `str()` na frente. `len` fica de fora (não significa nada em número),
e `bool` também.

### Lista

Alteram a lista **no lugar**, exceto `copy`.

| | |
|---|---|
| `.append(v)` `.extend(l)` `.insert(i, v)` | posição fora do fim gruda no fim |
| `.pop(i?)` | do fim sem argumento; erro se vazia |
| `.remove(v)` `.index(v)` | erro se não achar |
| `.count(v)` `.len()` | |
| `.reverse()` `.sort()` | `sort` é estável |
| `.clear()` `.copy()` | |

### Arquivo

O que `open()` devolve. Fechar é responsabilidade de quem abriu — ou do
[`using`](#using), que fecha mesmo se o bloco estourar.

| | |
|---|---|
| `.read(n?)` | tudo, ou no máximo `n` bytes |
| `.readline()` | uma linha, **com** a quebra no fim |
| `.readlines()` | lista de linhas, cada uma com a quebra |
| `.write(t)` | devolve quantos bytes escreveu; não-string vira texto |
| `.writelines(l)` | escreve cada item, sem inserir quebra |
| `.close()` | chamar duas vezes não é erro |

### Dicionário

`keys`/`values`/`items` saem em **ordem de inserção**.

| | |
|---|---|
| `.keys()` `.values()` `.items()` | listas; `items` dá tuplas |
| `.get(k, padrao?)` | `Null` se ausente, nunca erro |
| `.has(k)` `.contains(k)` | bool |
| `.pop(k, padrao?)` | sem padrão, chave ausente é **erro** |
| `.update(outro)` `.clear()` `.copy()` `.len()` | |

---

## Erros nomeados

Os erros de runtime da PoolScript têm um `code` estável — é o nome que o
`catch (Tipo nome)` filtra:

| `code` | Quando ocorre |
|---|---|
| `AttributedValueError` | Valor incompatível atribuído a variável tipada (ex.: `str x = 10`) |
| `OutputUnexpectedValues` | Redeclaração no mesmo escopo; aridade errada em unpacking |
| `SomeValueUnexpected` | Operação inválida entre tipos, divisão por zero, valor inválido, RHS não-iterável em unpacking |
| `IndexOutOfBoundsWarning` | Índice fora do intervalo (não-fatal — vira warning + `Null`) |
| `ConversionError` | Conversão automática de tipo falhou (ex.: `int x = "abc"`) |
| `NotImplemented` | Lib "stub" chamada (existe mas função ainda não implementada) |
| `KeyError` / `IndexError` / `TimeoutError` / `NetworkError` / `IOError` / etc. | Erro de chave, índice, tempo esgotado, rede e I/O |

`catch (e)` sem tipo pega **qualquer** erro de runtime, independente da causa.

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
