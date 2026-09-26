# Jinga v16.1.3

---

Versão:

```bash
jinga --version
# Jinga 16.1.3 [PSVM] (2026-09-24) Runtime standalone
#                       ^ a data da compilação DESTE binário: duas
#                         cópias da mesma versão se distinguem por ela
```

Os nomes de antes do rename continuam valendo: `pool` e `psl` seguem
instalados como atalhos do mesmo binário (e `poolscript-lsp` do servidor do
editor), `POOLSCRIPT_HOME` ainda é lida, `import poolscript.libs.X` ainda
importa, e uma `~/.poolscript` existente vira `~/.jinga` na primeira chamada
do `jpkg` (com aviso).

---

## Rodando arquivos

```bash
jinga meu_arquivo.pr
jinga build          # roda todos os .pr da pasta atual
```

O arquivo pode ter o tamanho que for. O motor não guarda o arquivo inteiro
na memória: lê uma declaração de topo por vez, fica só com o que outra
declaração precisa dela (o cabeçalho da funct, os campos da Entity) e com o
código gerado, e solta o resto antes de ler a seguinte. O que cresce com o
tamanho do programa é o código compilado, não a leitura dele. Medido com
`/usr/bin/time` em 2026-09-23: 200 mil functs de uma linha rodavam com
612 MB de pico e passaram a 353 MB; 20 mil, de 63 MB a 38 MB. O tempo caiu
junto (200 mil linhas: 16 s para 5,9 s).

---

## Desempenho

O programa vira bytecode e roda numa máquina virtual em C. O laço da VM
despacha por goto calculado, só confere o coletor de lixo onde um laço volta
ou uma funct é chamada, e o compilador funde o que sabe: `while a < b` é uma
instrução (comparação e desvio), `n++` numa variável `int` declarada é uma
instrução, `for each i in range(...)` decodifica os limites uma vez, e a
chamada com argumentos posicionais na aridade exata copia direto pros slots.
`jinga --bytecode arquivo.pr` mostra o que cada laço virou.

**Código de máquina.** No x86-64, cada funct (e o módulo) vira código de
máquina na primeira vez que roda: carregar e guardar variáveis, `int` com
`int`, comparação com desvio, `n++`, `range`, o teste de tipo da variável
declarada — tudo em linha, sem despacho. O que o código nativo não faz em
linha (uma chamada, um `return`, uma string, um estouro de 64 bits, um
`import`, uma coleta de lixo devida) ele devolve ao interpretador, que
executa exatamente aquela instrução com a regra de sempre e volta pro
nativo na seguinte. Por isso não existe uma segunda semântica: erro,
`try`, gerador, fibra e depurador continuam sendo os do interpretador, e a
mensagem de erro é a mesma. `jinga --sem-jit arquivo.pr` (ou
`JINGA_JIT=0`) desliga; `JINGA_JIT_LOG=1` diz no stderr o que compilou. Sob
`--debug` nada é compilado. Se o sistema recusar memória executável, o
programa segue interpretado. Fora do x86-64 é só o interpretador.

Medido com contadores de hardware (`perf stat`, instruções / ciclos), que
não dependem do clock da máquina, da versão 15.93.1 até esta:

| programa | interpretador antes | interpretador depois | código de máquina |
|---|---|---|---|
| `int n = 0; while n < 10000000 { n++ }` | 5,19 G / 1,59 G (12 bytecodes por volta) | 1,12 G / 0,46 G (5 por volta) | 0,33 G / 0,087 G |
| `fib(27)` com `int n` (317.811 chamadas) | 605 M / 203 M | 349 M / 116 M | 253 M / 63 M |
| lista: 1 milhão de `l[i % 200000]` | 1,14 G / 334 M | 731 M / 268 M | |
| dict: 300 mil `d[str(i)]` | 1,07 G / 345 M | 935 M / 329 M | |
| objeto: 300 mil `p.mais()` | 798 M / 211 M | 628 M / 163 M | |
| 60 mil `s = s + "abc"` | 19,6 s | 0,31 s | |

Sair pro interpretador e voltar custa o mesmo que três a oito instruções
interpretadas, então um proto em que mais de 15% das instruções dentro dos
laços (ou do proto inteiro, sem laço) sempre saem fica com o interpretador —
em nativo ele seria mais lento (medido: `p.mais()` num laço ficava 215 M
ciclos contra 163 M). É o caso de código que vive de membro de objeto, de
coleção e de método. `JINGA_JIT_LOG=1` mostra o que compilou.

**Chamada direta.** Uma funct nativa que chama outra funct (`V_FUNC`, sem
gerador, `async` ou `@static`, aridade exata, sem valor padrão nem
`*args`/`**kwarg`) entra nela sem passar pelo interpretador: o registro do
frame é empilhado exatamente como o interpretador empilha, os argumentos
são conferidos pelos mesmos tipos, e o `return` volta pro chamador nativo.
Qualquer outra forma de chamada — método, builtin, padrão, nomeado — e
qualquer coisa que o chamado não faça em linha voltam pro interpretador, que
adota o frame em que parou e segue com as regras de sempre (traceback com
todos os frames, `try` do chamador, `RecursionError` nos mesmos limites).
`fib(30)`: 549 M ciclos só no interpretador, 267 M com código de máquina.

O laço tipado passou de 165 ciclos por volta pra 8,7. Em relógio de parede,
o mesmo laço a 1 bilhão de voltas: 2,7 s como executável `-o`, 17,5 s com
`--sem-jit`. A concatenação era quadrática porque toda string nascia com o
hash pronto: `s = s + "abc"` hashava a string inteira a cada volta. O hash
agora é calculado na primeira vez que alguém precisa dele (chave de dict,
`==`).

Nesta máquina (notebook, governor `powersave`) o relógio de parede varia até
2x entre duas execuções iguais conforme a temperatura; `teste/bench.pr`
(`make bench`) mede em rodízio e compara razões, e a comparação entre dois
binários se faz com `perf stat`.

---

## Só checar (sem rodar)

`--check` analisa o arquivo (lexer, parser e a **tipagem estática** inteira:
tipo, nome, aridade, membro) e **não executa nada** — é o que um editor/LSP
usa pra sublinhar erro enquanto você digita. Erro de sintaxe para na primeira
linha errada; erro de tipo sai **completo**, com todos os erros do arquivo em
`erros` (o primeiro também vai nos campos de cima):

```bash
jinga --check meu_arquivo.pr
# {"ok":true}

jinga --check com_erro.pr
# {"ok":false,"tipo":"SyntaxError","msg":"faltou ')' na declaracao da funct","linha":1,"coluna":11}

jinga --check tipos_errados.pr
# {"ok":false,"tipo":"AttributedValueError","msg":"variável s esperava str, recebeu int","linha":1,"coluna":9,
#  "erros":[{"tipo":"AttributedValueError","msg":"variável s esperava str, recebeu int","linha":1,"coluna":9},
#           {"tipo":"NameError","msg":"name 'zzz' is not defined","linha":2,"coluna":6}]}

cat meu_arquivo.pr | jinga --check     # sem arquivo, lê da entrada padrão
```

O mesmo veredito vale ao rodar: `jinga arquivo.pr` não executa um programa com
erro de tipo — lista os erros e sai com código 2.

O campo `ok` diz o veredito, e o **código de saída acompanha**: `0` com
`{"ok":true}`, `1` com `{"ok":false}`. Quem chama pode olhar qualquer um dos
dois.

---

## REPL — não existe

`jinga repl` responde que o REPL interativo ainda não está no binário C (ele
precisa de estado persistente na VM), e `jinga` sem argumento imprime a ajuda.
Para rodar código sem criar arquivo, use `jinga -e "<codigo>"`.

---

## Comentários

```
# comentário de linha

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
str x = 'texto'
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
str nome = "jinga"

post(nome[0:4])    # jing
post(nome[4:])     # a
post(nome[:4])     # jing
post(nome[::-1])   # agnij — invertido
post(nome[::2])    # jna — de 2 em 2

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

Bitwise (só entre `int`; `bool` entra como 0/1 — `flo` e `str`
são recusados com `TypeError: unsupported operand type(s) for &: 'flo' and 'int'`):

```
post(5 ^ 3)     # 6   — xor
post(5 | 2)     # 7   — or
post(6 & 3)     # 2   — and
post(~5)        # -6  — complemento (unário)
post(1 << 4)    # 16  — deslocamento à esquerda
post(256 >> 4)  # 16  — deslocamento à direita
```

Precedência, do mais fraco pro mais forte:
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

Como **expressão**, `if`/`else` viram o condicional inline:
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
| `AttributeError` | membro que o objeto não tem (`"abc".m`, `json.naoexiste`); nome parecido é apontado: `'str' object has no attribute 'raplace'. Did you mean: 'replace'?` |
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
| `ConversionError` | `char c = -1` — inteiro que não é um codepoint válido. (`int z = "abc"` é `AttributedValueError`: declaração não converte) |
| `NotImplemented` | construção reconhecida e ainda não executada — o nome é sem `Error` |
| `MemoryError` | sem memória |
| `RuntimeError` | `raise "texto"`, e o que só existe aqui: `private` e `nonnull` (`for each` sobre tipo que não itera é `TypeError`) |
| `e` (sem tipo) | qualquer erro |

> Esta tabela já teve `PermissionError` e `ConnectionError`, que o motor **nunca
> levanta**. Quem seguisse a doc escrevia um `catch` que jamais dispara — e aí o
> erro escapa e o programa morre com rc=1. Hoje `scripts/audita_doc.pr` confere
> cada nome desta lista contra o motor e reprova se algum não existir.
>
> **"Não existe" tem uma resposta só:** ler ou escrever índice fora da faixa
> levanta `IndexError`, e chave ausente levanta `KeyError`, as duas dizendo o
> que faltou e em quê. Ver `docs/exceptions/exceptions.md`.

---

## funct

```
funct somar(a, b) {
    return a + b
}

post(somar(3, 4))   # 7
```

Funct chamando outra:

```
funct dobro(n) {
    return n * 2
}

funct quadruplo(n) {
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
| `list`, `dict` | `tags: list(of=str)` · `extras: dict` | lista (cada item do tipo `of`) · objeto JSON |
| outro model | `endereco: Endereco` | dict que passa naquele model (aninhado) |

Parâmetros que validam o **dado** (qualquer ordem, valores literais):
`regex="…"` (str), `in=[…]` e `not_in=[…]` (valores aceitos/proibidos),
`min=N`/`max=N` (int, flo), `optional=true` (pode faltar ou vir null),
`of=T` (item da lista). Na rota do jinker com `model=`, o 422 diz o campo e o
motivo — `campo 'idade': no minimo 18, veio 12`. Detalhe em
[08-model-e-enum](linguagem/08-model-e-enum.md).

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
existe é erro: `AttributeError: type object 'Cor' has no attribute 'ROXO'`.
`Cor.type()` devolve `"enum"`.

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

`using <expr> as <nome>` — igual `if`/`while`/`funct`, aceita bloco com
chaves `{ }` — o único estilo de bloco da linguagem.

Funciona com qualquer objeto que tenha `__enter__`/`__exit__` — o builtin
global `open()` (sem import, sempre disponível) é o caso mais comum:

```
using open("log.txt", "a", encoding="utf-8") as f {
    f.write("nova linha\n")
}
# arquivo fechado automaticamente, mesmo se f.write() der erro
```

`open(path, mode="r", encoding="utf-8")` — `mode` aceita `"r"`, `"w"`, `"a"` e
as variantes binárias (`"rb"`, `"wb"`, `"ab"`); `encoding` é ignorado em modo
binário (`"b"` no mode).

E `manpu.open()` (lib separada, pensada pra CSV/XLSX estruturado — ver
[manpu.md](manpu.md)):

```
import manpu as mp

lista = mp.load("compras.txt")

using mp.open(target="planilha.xlsx") as arq {
    arq.write(column=0, celula="A1", content=lista)
}
# arquivo salvo e fechado automaticamente
```

O nome do parâmetro é `celula`, não `cell`, e o módulo importado `as mp` se
chama `mp` — usar `manpu.` depois do apelido é `NameError`.

```
using mp.open(target="planilha.xlsx", encoding="latin-1") as arq {
    arq.write(column=0, celula="A1", content=lista)
}
# arquivo salvo e fechado automaticamente
```

`manpu.open()` aceita `encoding=` (default `"utf-8"`) pra CSV/texto puro —
veja [manpu.md](manpu.md).

---

## Ponto de entrada

```
funct iniciar() {
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

Importar de outro arquivo `.pr`:

```
# utils.pr
funct somar(a, b) {
    return a + b
}
str VERSAO = "1.0"

# main.pr
from utils import somar, VERSAO
post(somar(3, 4))  # 7
post(VERSAO)       # 1.0
```

Pelo caminho, entre aspas — relativo à pasta do arquivo que importa:

```
from './utils.pr' import somar
import '../lib/texto.pr'         # liga `texto`
import 'ferramentas/kit.pr' as k
```

---

## Builtins globais

| Função | O que faz |
|---|---|
| `post(valor)` | Imprime no terminal |
| `input(msg)` | Lê entrada do usuário |
| `len(x)` | Tamanho de lista, string ou dict |
| `range(n)` | Lista de 0 até n-1 |
| `type(x)` | Nome do tipo do valor, em texto — ver [Testar o tipo](#testar-o-tipo) |
| `open(path, mode="r", encoding="utf-8")` | Abre arquivo — devolve **`PoolFile`** (`.read()`, `.readlines()`, `.readline()`, `.write(t)`→`int`, `.close()`, `.path()`, `.copy()`, `.move()`, `.delete()`, `.name`, `.ext`, `.size`). Use com `using` pra fechar automático |
| `load()` | Carrega o .env |

### Testar o tipo

A forma canônica é o operador **`is`**, o mesmo que se usa pra testar nulo:

```jinga
x = 1
if x is int {
    post("é inteiro")
}
if x not is none {
    post("tem valor")
}
```

`type(x)` devolve o **nome** do tipo em texto — serve pra mostrar e pra
registrar, não é o tipo em si:

```jinga
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

```jinga
post(type(200) == "int")   # True
post(type(200) == int)     # True
```

> As duas escrevem `int` na tela, então elas se comparam pelo nome. Sem isso,
> `type(200) == int` daria **falso sem erro**: o `if` nunca entrava e nada era
> avisado.

Para instância de Entity, `is` compara a **classe exata** — ele não sobe pela
herança:

```jinga
Entity Animal { }
Entity Gato(Animal) { }
g = Gato()

post(type(g))            # Gato
post(g is Gato)          # True
post(g is Animal)        # False — classe exata, herança não conta
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
funct verificar() {
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
funct cadastrar() {
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
funct logar() {
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
funct dados() {
    return jsonify({"msg": "área protegida"}), 200
}

if __name__ == "main":
    app(debug=False, host="0.0.0.0", port=7700, reload=true)
```