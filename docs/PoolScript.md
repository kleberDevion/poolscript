# PoolScript v8.3.84

Linguagem de programação híbrida — dinâmica e estática ao mesmo tempo.

---

Verificando:

```bash
pool --version
# PoolScript 8.3.84 [PSVM]
```

---

## Rodando arquivos

```bash
pool meu_arquivo.ps
pool build          # roda todos os .ps da pasta atual
```

---

## Só checar a sintaxe (sem rodar)

`--check` analisa o arquivo (lexer + parser) e **não executa nada** — é o que
um editor/LSP usa pra sublinhar erro enquanto você digita. A saída é um JSON
de uma linha:

```bash
pool --check meu_arquivo.ps
# {"ok":true}

pool --check com_erro.ps
# {"ok":false,"tipo":"SyntaxError","msg":"faltou ')' na declaracao da action","linha":1,"coluna":11}

cat meu_arquivo.ps | pool --check     # sem arquivo, lê da entrada padrão
```

O processo sai com código 0 mesmo quando o arquivo tem erro — quem chama olha
o campo `ok`.

---

## REPL

```bash
pool
pool repl
```

```
PoolScript v8.3.84 — REPL
Digite 'sair' ou Ctrl+C para sair.

>>> str nome = "joao"
>>> post(nome)
joao
>>> action dobro(n) {
...     return n * 2
... }
>>> post(dobro(5))
10
>>> sair
Até mais!
```

---

## Comentários

```
# comentário de linha  (era `//` também; o `//` virou divisão inteira)

"""
comentário
de bloco
"""
```

---

## Tipos de variável

```
str nome = "joao"
int idade = 17
flo altura = 1.75
bool ativo = true
```

Variável sem tipo declarado:

```
x = 10
dados = {"nome": "ana"}
lista = [1, 2, 3]
```

Variáveis em maiúsculo são permitidas:

```
str VERSAO = "1.0.8"
int LIMITE = 100
```

---

## Strings

```
str nome = "kleber"
str msg = f"Olá {nome}!"
post(msg)  # Olá kleber!
```

Aspas simples também funcionam:

```
str x = 'texto aqui'
```

Aspas simples **triplas** — `'''...'''` — abrem string multi-linha (útil pra
SQL, textos longos etc.). Funciona com `f'''...'''` (interpolação) e
`r'''...'''` (raw, sem escape):

```
str sql = '''SELECT *
FROM users
WHERE ativo = 1'''

str msg = f'''Olá {nome}!
Segunda linha.'''
```

Aspas **duplas** triplas (`"""`) são reservadas pra comentário de bloco (ver
[Comentários](#comentários)) — não servem de string, só `'''` multi-linha.

---

## Listas

```
list nomes = ["ana", "leo", "bia"]
post(nomes[0])      # ana
post(nomes[-1])     # bia
post(len(nomes))    # 3

nomes[0] = "joao"
post(nomes)         # ['joao', 'leo', 'bia']
```

---

## Dicionário

```
data = {"nome": "ana", "idade": 20, "ativo": true}
post(data["nome"])    # ana
post(data["idade"])   # 20
post(len(data))       # 3
```

---

## Tupla

```
cmd = ("SELECT * FROM users WHERE nome = ?", ("joao",))
post(cmd[0])  # SELECT * FROM users WHERE nome = ?
post(cmd[1])  # ('joao',)
```

---

## Slice

Acessa partes de strings e listas com `[inicio:fim:passo]`:

```
str nome = "poolscript"

post(nome[0:4])    # pool
post(nome[4:])     # script
post(nome[:4])     # pool
post(nome[::-1])   # tpircsloop — invertido
post(nome[::2])    # posrp — de 2 em 2

list nums = [1, 2, 3, 4, 5]
post(nums[1:3])    # [2, 3]
post(nums[::-1])   # [5, 4, 3, 2, 1]
post(nums[::2])    # [1, 3, 5]
post(nums[1:])     # [2, 3, 4, 5]
```

---

## Operadores

```
x = 10
x += 5    # 15
x -= 2    # 13
x *= 3    # 39
x /= 2    # 19.5
x++       # incrementa
x--       # decrementa
```

Comparação:

```
x == 10
x != 5
x > 3
x < 20
x >= 10
x <= 10
```

Lógicos:

```
if (x == 1 and y == 2) { ... }
if (x == 1 or y == 2) { ... }
if (not x) { ... }
```

Bitwise (só entre `int`; `bool` entra como 0/1, como no Python — `flo` e `str`
são recusados com `TypeError: unsupported operand type(s) for &: 'flo' and 'int'`):

```
post(5 ^ 3)     # 6   — xor
post(5 | 2)     # 7   — or
post(6 & 3)     # 2   — and
post(~5)        # -6  — complemento (unário)
post(1 << 4)    # 16  — deslocamento à esquerda
post(256 >> 4)  # 16  — deslocamento à direita
```

Precedência igual à do Python — do mais fraco pro mais forte:
`|` , `^` , `&` , `<<`/`>>`. Todos ligam mais forte que comparação e mais
fraco que `+`/`-`, então `2 + 3 << 1` é `(2 + 3) << 1` = `10`.

---

## if / elif / else

```
int nota = 8

if (nota >= 9) {
    post("Excelente!")
} elif (nota >= 7) {
    post("Bom!")
} elif (nota >= 5) {
    post("Regular")
} else {
    post("Reprovado")
}
```

Bloco é sempre `{ }` — `:` não abre bloco na linguagem. A chave pode ficar na
linha seguinte (estilo Allman), e dá no mesmo:

```
if (nota >= 7)
{
    post("Aprovado")
}
else
{
    post("Reprovado")
}
```

### Condicional inline (ternário)

Como **expressão**, `if`/`else` viram o condicional inline no estilo Python —
`A if cond else B`. Devolve `A` quando a condição é verdadeira, senão `B`, e só
avalia o ramo escolhido:

```
str situacao = "aprovado" if nota >= 7 else "reprovado"
post(situacao)

# encadeia à direita
str faixa = "A" if nota >= 9 else "B" if nota >= 7 else "C"
```

O `else` é obrigatório — é ele que separa o ternário de um `if` statement. Por
isso um `if cond { ... }` dentro de um bloco continua sendo statement normal.

---

## while

```
int i = 0
while (i < 5) {
    post(i)
    i += 1
}
# 0 1 2 3 4
```

---

## for each

```
list frutas = ["maçã", "banana", "uva"]

for each fruta in frutas {
    post(fruta)
}
# maçã
# banana
# uva
```

---

## break e continue

```
int i = 0
while (i < 10) {
    i += 1
    if (i == 3) {
        continue    # pula o 3
    }
    if (i == 6) {
        break       # para no 6
    }
    post(i)
}
# 1 2 4 5
```

---

## try / catch

Catch simples:

```
try {
    int x = 1 / 0
} catch (e) {
    post(f"Erro: {e}")
}
# Erro: division by zero
```

Múltiplos catches com tipo:

```
try {
    data = {"nome": "ana"}
    post(data["idade"])
} catch (KeyError e) {
    post("Chave não existe!")
} catch (TypeError e) {
    post("Tipo errado!")
} catch (e) {
    post(f"Erro genérico: {e}")
}
# Chave não existe!
```

Tipos disponíveis:

| Tipo | Quando ocorre |
|---|---|
| `TypeError` | o **tipo** está errado: `"a" - 1`, `len(5)`, aridade errada. O mais comum |
| `ValueError` | o tipo está certo e o **valor** não serve: `int("abc")`, `max([])` |
| `ZeroDivisionError` | divisão ou resto por zero |
| `NameError` | nome que não existe no escopo (`post(x)`) |
| `AttributeError` | membro que o objeto não tem (`"abc".m`, `json.naoexiste`) |
| `OverflowError` | número que não cabe no destino (`int(flo("inf"))`) |
| `AttributedValueError` | valor incompatível em variável **tipada** (`str x = 10`) — não cobre o `+` |
| `KeyError` | chave não existe no dict. A mensagem é só a chave: `'z'` |
| `IndexError` | índice fora da faixa, lendo ou escrevendo (`l[99]`, `l[99] = x`) |
| `FileNotFoundError` | arquivo não encontrado |
| `IOError` / `OSError` | arquivo / sistema |
| `NetworkError` | falha de conexão |
| `TimeoutError` | tempo esgotado |
| `DatabaseError` | erro de banco |
| `ImportError` | módulo não encontrado |
| `ConversionError` | coerção de declaração tipada (`int z = "abc"`) |
| `NotImplemented` | construção reconhecida e ainda não executada — o nome é sem `Error` |
| `MemoryError` | sem memória |
| `RuntimeError` | `raise "texto"`, e o que só existe aqui: `private`, `@NonNull`, `for each` sobre tipo que não itera |
| `e` (sem tipo) | qualquer erro |

> Esta tabela já teve `PermissionError` e `ConnectionError`, que o motor **nunca
> levanta**. Quem seguisse a doc escrevia um `catch` que jamais dispara — e aí o
> erro escapa e o programa morre com rc=1. Hoje `scripts/audita_doc.ps` confere
> cada nome desta lista contra o motor e reprova se algum não existir.
>
> **"Não existe" tem uma resposta só:** ler ou escrever índice fora da faixa
> levanta `IndexError`, e chave ausente levanta `KeyError`, as duas dizendo o
> que faltou e em quê. Ver `docs/exceptions/exceptions.md`.

---

## action

```
action somar(a, b) {
    return a + b
}

post(somar(3, 4))   # 7
```

Action chamando outra:

```
action dobro(n) {
    return n * 2
}

action quadruplo(n) {
    return dobro(dobro(n))
}

post(quadruplo(3))  # 12
```

---

## model

Valida estrutura de dados JSON:

```
model Usuario() {
    nome: str(length=60)
    email: str(length=100)
    idade: int(length=3)
    ativo: bool
}

data = {"nome": "ana", "email": "ana@email.com", "idade": 20, "ativo": true}

if (data == Usuario) {
    post("Dados válidos!")
} else {
    post("Dados inválidos!")
}
```

Campos disponíveis:

| Tipo | Exemplo | Valida |
|---|---|---|
| `str(length=N)` | `nome: str(length=60)` | texto com máx N caracteres |
| `int(length=N)` | `idade: int(length=3)` | inteiro com máx N dígitos |
| `flo` | `altura: flo` | número decimal |
| `bool` | `ativo: bool` | true ou false |

---

## enum

Namespace de constantes nomeadas. `Cor.RED` devolve o valor do membro:

```
enum Cor { RED, GREEN, BLUE }

post(Cor.RED)     # 0
post(Cor.GREEN)   # 1
post(Cor.BLUE)    # 2
```

A auto-numeração começa em `0`. Cada membro pode receber um valor explícito —
inclusive string:

```
enum Hex {
    RED   = "#f00",
    GREEN = "#0f0",
    BLUE  = "#00f"
}

post(Hex.RED)   # #f00
```

Auto e explícito se misturam. Um valor **int** explícito reancora a sequência
(os próximos autos continuam a partir dele); um valor não-int não mexe no
contador:

```
enum Mix { A, B = 10, C, D = "x", E }
# A=0  B=10  C=11  D="x"  E=12
```

Membros separados por vírgula (opcional no último). Acessar um membro que não
existe é erro (`enum 'Cor' não tem membro 'ROXO'`). `Cor.type()` devolve
`"enum"`.

---

## count

```
list nums = [1, 7, 2, 7, 3, 7]
post(count int(7) in nums)  # 3

if (count int(7) in nums == 3) {
    post("Tem três 7s!")
}
```

### count each com bloco

```
count each int(7) in nums {
    post(f"Encontrado no índice {_index}")
}
```

---

## using

`using <expr> as <nome>` — igual `if`/`while`/`action`, aceita bloco com
chaves `{ }` — o único estilo de bloco da linguagem.

Funciona com qualquer objeto que tenha `__enter__`/`__exit__` — o builtin
global `open()` (sem import, sempre disponível) é o caso mais comum:

```
using open("log.txt", "a", encoding="utf-8") as f {
    f.write("nova linha\n")
}
# arquivo fechado automaticamente, mesmo se f.write() der erro
```

`open(path, mode="r", encoding="utf-8")` — `mode` aceita os mesmos valores do
Python (`"r"`, `"w"`, `"a"`, `"rb"`, `"wb"`...); `encoding` é ignorado em modo
binário (`"b"` no mode).

E `manpu.open()` (lib separada, pensada pra CSV/XLSX estruturado — ver
[manpu.md](manpu.md)):

```
import manpu as mp

lista = manpu.load("compras.txt")

using manpu.open(target="planilha.xlsx") as arq {
    arq.write(column=0, cell=full, content=lista)
}
# arquivo salvo e fechado automaticamente
```

```
using manpu.open(target="planilha.xlsx", encoding="latin-1") as arq {
    arq.write(column=0, cell=full, content=lista)
}
# arquivo salvo e fechado automaticamente
```

`manpu.open()` aceita `encoding=` (default `"utf-8"`) pra CSV/texto puro —
veja [manpu.md](manpu.md).

---

## Ponto de entrada

```
action iniciar() {
    post("Servidor iniciando...")
}

if __name__ == "main" {
    iniciar()
}
```

---

## Imports

```
import os
import psodbc
import hash
import jwt
import date
import mail
import manpu as mp
from dotenv import load
from jinker import Jinker, cors, jsonify, render
```

Importar de outro arquivo `.ps`:

```
# utils.ps
action somar(a, b) {
    return a + b
}
str VERSAO = "1.0"

# main.ps
from utils import somar, VERSAO
post(somar(3, 4))  # 7
post(VERSAO)       # 1.0
```

---

## Builtins globais

| Função | O que faz |
|---|---|
| `post(valor)` | Imprime no terminal |
| `post.flush(texto, delay=N)` | Efeito de digitação |
| `input(msg)` | Lê entrada do usuário |
| `len(x)` | Tamanho de lista, string ou dict |
| `range(n)` | Lista de 0 até n-1 |
| `type(x)` | Nome do tipo do valor, em texto — ver [Testar o tipo](#testar-o-tipo) |
| `open(path, mode="r", encoding="utf-8")` | Abre arquivo — devolve `FileHandle` (`.read()`, `.readlines()`, `.readline()`, `.write(texto)`, `.writelines(lista)`, `.close()`). Use com `using` pra fechar automático |
| `load()` | Carrega o .env |

### Testar o tipo

A forma canônica é o operador **`is`**, o mesmo que se usa pra testar nulo:

```poolscript
if x is int {
    post("é inteiro")
}
if x not is none {
    post("tem valor")
}
```

`type(x)` devolve o **nome** do tipo em texto — serve pra mostrar e pra
registrar, não é o tipo em si:

```poolscript
post(type(200))     # int
post(type("a"))     # str
post(type(none))    # Null
post(type(1.5))     # flo
```

Os nomes são `str`, `int`, `flo`, `bool`, `list`, `dict`, `tup`, `Null`,
`type`, `module` e, pra instância, o nome da Entity. Não existe nome `char` em
tempo de execução: `chr(65)` é `str`.

Comparar o resultado funciona das duas maneiras — com o nome em texto ou com a
referência de tipo:

```poolscript
post(type(200) == "int")   # True
post(type(200) == int)     # True
```

> As duas escrevem `int` na tela, então elas se comparam pelo nome. Sem isso,
> `type(200) == int` daria **falso calado**: o `if` nunca entrava e ninguém era
> avisado.

Para instância de Entity, `is` compara a **classe exata** — ele não sobe pela
herança:

```poolscript
Entity Animal() { }
Entity Gato(Animal) { }
g = Gato()

post(type(g))            # Gato
post(g is Gato)          # True
post(g is Animal)        # False — classe exata, herança não conta
```

### post.flush()

```
post.flush("Carregando...", delay=0.05)
post.flush("Pronto!", delay=0.08)
```

---

## Libs disponíveis

| Lib | Import | Descrição |
|---|---|---|
| `os` | `import os` | Sistema operacional |
| `dotenv` | `from dotenv import load` | Arquivo .env |
| `date` | `import date` | Data e hora |
| `db` | `import psodbc` | SQLite, PostgreSQL, MySQL, MongoDB |
| `mail` | `import mail` | Envio (`MailServer`/`MailMessage`) e leitura (`MailReader`, IMAP) de email |
| `request` | `import request` | Requisições HTTP |
| `hash` | `import hash` | Hash de senhas |
| `jwt` | `import jwt` | Tokens JWT |
| `jinker` | `from jinker import Jinker, cors, jsonify` | Servidor HTTP |
| `manpu` | `import manpu as mp` | Manipulação de arquivos |

---

## Exemplo completo — Backend com auth

```
import psodbc
import hash
import jwt
import date
import os
from dotenv import load
from jinker import Jinker, cors, jsonify

load()

str DB_PATH = os.getenv("DB_PATH")
str SECRET = os.getenv("SECRET_KEY")

app = Jinker(__name__)
cors(options=["POST", "GET"], permiser=["*/api", "allowed.all/Users-Agent"])

model Usuario() {
    nome: str(length=60)
    email: str(length=100)
    senha: str(length=100)
}

@app.middleware()
action verificar() {
    token = request.get("token")
    if (not token) {
        return jsonify({"msg": "não autorizado"}), 401
    }
    payload = jwt.check(token, SECRET)
    if (not payload) {
        return jsonify({"msg": "token inválido"}), 401
    }
    continue
}

@app.route("/api/cadastro", auth=cors.permiser(), methods=cors.options(["POST"]))
action cadastrar() {
    data = request.get_json()

    if (data == Usuario) {
        continue
    } else {
        return jsonify({"msg": "dados inválidos"}), 400
    }

    nome = data.get("nome")
    email = data.get("email")
    senha = data.get("senha")
    senha_hash = hash.crypt(senha)

    try {
        psodbc.query(
            base=DB_PATH,
            cmd=("INSERT INTO @t (nome, email, senha) VALUES (?, ?, ?)",
                 (nome, email, senha_hash)),
            table="usuarios"
        )
        return jsonify({"msg": "cadastrado!"}), 201
    } catch (e) {
        return jsonify({"msg": "erro interno"}), 500
    }
}

@app.route("/api/login", auth=cors.permiser(), methods=cors.options(["POST"]))
action logar() {
    data = request.get_json()
    email = data.get("email")
    senha = data.get("senha")

    result = psodbc.query(
        base=DB_PATH,
        cmd=("SELECT * FROM @t WHERE email = ?", (email,)),
        table="usuarios"
    )

    if (result and hash.check(result[0]["senha"], senha)) {
        payload = {
            "user_id": result[0]["id"],
            "email": result[0]["email"],
            "exp": date.timestamp() + date.hora(hours=24)
        }
        token = jwt.gen(payload, SECRET, algorithm="HS256")
        return jsonify({"msg": "login feito", "token": token}), 200
    } else {
        return jsonify({"msg": "credenciais inválidas"}), 401
    }
}

@app.route("/api/dados", auth=cors.permiser(), methods=cors.options(["GET"]), middleware=app.middleware)
action dados() {
    return jsonify({"msg": "área protegida"}), 200
}

if __name__ == "main" {
    app(debug=False, host="0.0.0.0", port=7700, reload=true)
}
```
