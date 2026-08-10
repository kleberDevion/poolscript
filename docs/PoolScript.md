# PoolScript v8.2.49

Linguagem de programação híbrida — dinâmica e estática ao mesmo tempo.
Criada por Kleber Santana de Oliveira.

---

## Instalação

```bash
pip install -e .
```

Verificando:

```bash
pool --version
# PoolScript  v8.2.49
```

---

## Rodando arquivos

```bash
pool meu_arquivo.ps
pool build          # roda todos os .ps da pasta atual
```

---

## REPL

```bash
pool
pool repl
```

```
PoolScript v8.2.49 — REPL
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
# comentário de linha
// comentário de linha também

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

## Dicionários

```
data = {"nome": "ana", "idade": 20, "ativo": true}
post(data["nome"])    # ana
post(data["idade"])   # 20
post(len(data))       # 3
```

---

## Tuplas

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

Bitwise (só entre `int` — `bool`, `flo` e `str` são recusados com erro):

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

Estilo Python com `:` também funciona:

```
if (nota >= 7):
    post("Aprovado")
else:
    post("Reprovado")
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
| `ZeroDivisionError` | Divisão por zero |
| `KeyError` | Chave não existe no dict |
| `TypeError` | Tipo errado |
| `ValueError` | Valor inválido |
| `IndexError` | Índice fora do tamanho |
| `FileNotFoundError` | Arquivo não encontrado |
| `PermissionError` | Sem permissão de acesso |
| `ConnectionError` | Falha de conexão |
| `TimeoutError` | Timeout |
| `e` (sem tipo) | Qualquer erro |

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
chaves `{ }` ou estilo Python com `:` e indentação (testado, os dois funcionam).

Funciona com qualquer objeto que tenha `__enter__`/`__exit__` — o builtin
global `open()` (sem import, sempre disponível) é o caso mais comum:

```
using open("log.txt", "a", encoding="utf-8") as f:
    f.write("nova linha\n")
# arquivo fechado automaticamente, mesmo se f.write() der erro
```

`open(path, mode="r", encoding="utf-8")` — `mode` aceita os mesmos valores do
Python (`"r"`, `"w"`, `"a"`, `"rb"`, `"wb"`...); `encoding` é ignorado em modo
binário (`"b"` no mode).

E `manpu.open()` (lib separada, pensada pra CSV/XLSX estruturado — ver
[manpu.md](manpu.md)):

```
import manpu as mp

lista = mp.load("compras.txt")

using mp.open(target="planilha.xlsx") as arq {
    arq.write(column=0, cell=full, content=lista)
}
# arquivo salvo e fechado automaticamente
```

```
using mp.open(target="planilha.xlsx", encoding="latin-1") as arq:
    arq.write(column=0, cell=full, content=lista)
# arquivo salvo e fechado automaticamente
```

`mp.open()` aceita `encoding=` (default `"utf-8"`) pra CSV/texto puro —
veja [manpu.md](manpu.md).

---

## Ponto de entrada

```
action iniciar() {
    post("Servidor iniciando...")
}

run_selfwith_("main") {
    iniciar()
}
```

---

## Imports

```
import os
import db
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
| `type(x)` | Tipo do valor |
| `open(path, mode="r", encoding="utf-8")` | Abre arquivo — devolve `FileHandle` (`.read()`, `.readlines()`, `.readline()`, `.write(texto)`, `.writelines(lista)`, `.close()`). Use com `using` pra fechar automático |
| `load()` | Carrega o .env |

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
| `db` | `import db` | SQLite, PostgreSQL, MySQL, MongoDB |
| `mail` | `import mail` | Envio (`MailServer`/`MailMessage`) e leitura (`MailReader`, IMAP) de email |
| `request` | `import request` | Requisições HTTP |
| `hash` | `import hash` | Hash de senhas |
| `jwt` | `import jwt` | Tokens JWT |
| `jinker` | `from jinker import Jinker, cors, jsonify` | Servidor HTTP |
| `manpu` | `import manpu as mp` | Manipulação de arquivos |

---

## Exemplo completo — Backend com auth

```
import db
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
        db.query(
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

    result = db.query(
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

run_selfwith_("main") {
    app(debug=False, host="0.0.0.0", port=7700)
}
```
