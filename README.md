# PoolScript

PoolScript é uma linguagem de programação **híbrida (dinâmica/estática)** que
mistura a legibilidade do Python com a estrutura de blocos do JS/C — indentação
*ou* chaves, `:` *ou* `{}`, à vontade e no mesmo arquivo.

Ela roda em **dois motores que ficam em paridade**:

- **Interpretador em Python** (`src/poolscript/`) — a autoridade semântica, tree-walking, sem dependências obrigatórias.
- **PSVM** — a VM em C (`vm/`), o runtime de produção, distribuída como o binário `pool` (e também como extensão CPython). Rápida, sem runtime Python instalado.

Os dois são testados de forma **diferencial**: o mesmo programa tem que produzir
o mesmo `stdout` **e** o mesmo texto de erro (traceback incluso) nos dois. Se
diverge, é bug.

Extensões de arquivo reconhecidas em tudo (rodar, importar, `psl install`,
`pool build`, editor): **`.ps`**, **`.psl`** e **`.p`**.

## Instalação

```bash
# a) via pip — registra os comandos `pool` e `psl` no PATH (usa o interpretador Python)
pip install -e .

# b) binário standalone (VM em C) — não precisa de Python instalado
install -m755 dist/pool-linux /usr/local/bin/pool
```

Ou rode direto sem instalar nada:

```bash
python -m poolscript examples/hello.ps
```

## Uso rápido

```bash
pool arquivo.ps               # roda um arquivo (.ps / .psl / .p)
pool build                    # roda todos os scripts da pasta atual
pool repl                     # REPL interativo
pool --version                # versão e runtime
psl --help                    # ajuda
psl //doc                     # URL da spec
```

## Sintaxe (resumo)

```pool
// declaração com tipo
str nome = "Pool"
int idade = 25
flo peso = 70.5
bool vivo = True

// dinâmica (inferida)
x = 10

// interpolação (3 formas)
post("Clima: " {grau} "°")    // chaves entre strings
post(f"Clima: {grau}°")        // f-string
juncao = ("Resultado: " {grau})

// condicionais (mistura : e {} livre)
if (x == y) {
    post("Igual")
} elif x > y:
    post("Maior")
else {
    post("Menor")
}

// listas + iteração
minha_lista = ["A", "B", "C"]
for each i in minha_lista:
    post(i)

// funções
action somar(a, b) {
    return a + b
}

// erros — com traceback completo estilo Python
try {
    risky()
} catch (e) {
    post("Erro: " {e})
}

// imports — 3 sintaxes
import os
from json import parse
PUSH os GET getenv
```

### Tipos como valores de primeira classe

Tipos são valores: dá pra guardar, passar e chamar.

```pool
f = str            // guarda o tipo
post(f(123))       // "123"  — chamar o tipo converte
mapeado = map(lista, int)   // aplica o conversor em cada item

if (x is int) { post("é inteiro") }   // is compara identidade de tipo
```

## Entities (classes)

`Entity` (aliases `class` / `Class`) declara classes com herança, métodos,
encapsulamento e métodos estáticos.

```pool
Entity Animal {
    action __init__(self, nome) {
        self.nome = nome
    }
    action falar(self) {
        return self.nome " faz um som"
    }
}

Entity Cachorro(Animal) {          // herança
    action falar(self) {
        return self.nome ": au au"
    }
}

c = Cachorro("Rex")
post(c.falar())                    // "Rex: au au"
```

- **`private` / `public`** — encapsulamento de verdade, enforçado nos dois motores.
- **`@static`** — método chamável sem instanciar (`Classe.metodo()`); sem ele, só via instância.

### `@dataentity` (estilo `@dataclass`)

```pool
from datasentity import dataentity, asdict, asjson

@dataentity
Entity Person {
    nome:  str
    idade: int
    email: str
}

p = Person("Ana", 30, "ana@x.com")   // __init__ automático
post(asjson(p))
```

## Operador `count`

`count` valida existência e conta ocorrências tipadas em listas, dicts, strings
ou números. Retorna `int` (`0` é falsy), então combina direto em `if`.

```pool
list nums = [1, 7, 7, 17, 7]

post(count int(7) in nums)         // 3   — conta os 7 (do tipo int)
post(int(7) count in nums)         // 3   — forma infixa
post(count int in nums)            // 5   — só o tipo: conta todos do tipo

str frase = "oi mundo oi pessoal oi"
post(count str("oi") in frase)     // 3   — substring

int n = 17717
post(count int(7) in n)            // 3   — dígitos no número
```

### Bloco `count each`

Executa o bloco a cada ocorrência, com `_match`, `_index` e `_count` disponíveis
dentro. `return;` (sem valor) devolve o total final; `return <expr>` corta na hora.

```pool
action contarSetes() {
    count each int(7) in [7, 7, 8, 7] {
        return;       // → 3 ao final
    }
}
post(contarSetes())                // 3
```

## Validação por tipo e comparações

```pool
entrada = input("DIGITAR: ")

if (entrada is int) {
    post("É um inteiro")
} else {
    post("Não é um inteiro")
}

if (nome not is None) { post("nome existe") }
```

Operadores em `if`: `==`, `!=`, `===`, `!==`, `<`, `>`, `<=`, `>=`, `is`,
`not is`, `is not`, `in`, `not in`, `and`, `or`, `not`, `!`.
Tipos validáveis: `str`, `int`, `flo`, `bool`, `list`, `json` e `None`/`Null`.

## Bibliotecas padrão

Todas implementadas e funcionando. As que dependem de libs externas só as
importam sob demanda (instale com `pip install ".[all]"`).

| Lib | O que faz |
|---|---|
| `os` | arquivos e sistema: `pathFile`, `pathFolder`, `ls`, `mkdir`, `rmdir`, `exists`, `cmd`, `run`, `getenv`… |
| `sys` | `sys.argv`, saída/entrada padrão |
| `json` / `JSON` | `parse`, `stringify` |
| `dotenv` | `load` (carrega `.env`) |
| `date` | `datahora`, `today`, `time`, `now`, `timestamp` |
| `request` | HTTP client: `get`, `post`, `put`, `patch`, `delete`, `ws_connect` (WebSocket) |
| `mail` | SMTP (`MailServer`) e IMAP (`MailReader`) com auto-detecção de provedor |
| `hash` | hash de senha: `crypt`, `check` |
| `jwt` | tokens JWT: `gen`, `check` (HS256…) |
| `regex` | engine própria com paridade com o `re` do Python |
| `bytes` | criar/converter bytes: `new`, `fromhex`/`hex`, `base64`/`frombase64`, `fromint`/`toint`, `tolist`, `concat`, `slice`, `get`, `xor` |
| `psodbc` / `db` | SQLite, PostgreSQL, MySQL, SQL Server e MongoDB |
| `sqlite3` | SQLite direto |
| `qrcode` / `qr` | geração de QR Code (`make`, `.save`) |
| `manpu` / `mp` | leitura/escrita de CSV, XLSX, XML, HTML e texto |
| `datasentity` | `@dataentity`, `asdict`, `astuple`, `aslist`, `asjson` |
| `jinker` | servidor HTTP + WebSocket nativo (veja abaixo) |

### `regex` — paridade com o `re` do Python

Engine escrita do zero em C (e espelhada no interpretador). Suporta grupos
nomeados `(?P<n>...)`, lookaround `(?=)(?!)(?<=)(?<!)`, backreferences, flags
inline `(?ims)` / `(?ims:...)`, âncoras `\b \A \Z`, classes Unicode e
`IGNORECASE`/`MULTILINE`/`DOTALL`.

```pool
from regex import findall, sub, match, split, escape

findall("\\d+", "a1b2c3")         // ["1", "2", "3"]
sub("\\s+", "_", "hello world")   // "hello_world"
```

## jinker — servidor HTTP + WebSocket

Servidor web nativo, zero dependências externas para o básico. Rotas por
decorator (com precedência sobre arquivos estáticos, igual Flask), CORS,
`jsonify`, arquivos estáticos com fallback de SPA, **múltiplos processos** e
**reload** em desenvolvimento.

```pool
import jinker
from jinker import Jinker, jsonify

server = Jinker(__name__, static_folder="dist")   // serve a build do React/Vite

@server.route("/api/hello", methods=["GET"]) {
    action handler() {
        return jsonify({"msg": "oi"})
    }
}

// WebSocket com salas — a conexão entra na sala = valor de :id
@server.socket("/sala:id", channel=True) {
    action main() {
        msg = request.get_json()
        id  = request.path_param("id")
        server.socket.emit(payload=msg, room_id=id)   // broadcast pra sala
    }
}

// inicia o servidor
run_selfwith_("main") {
    server(host="0.0.0.0", port=8000, workers=4, reload=True)
}
```

- **`static_folder`** — serve arquivos estáticos; `GET /` cai no `index.html`, e rotas desconhecidas também (fallback de SPA). Suas `@route` sempre vêm antes do estático.
- **`workers=N`** — prefork multi-processo no binário `pool`: N processos compartilham o socket, sem gargalo em conexões concorrentes (o interpretador roda sempre em 1 processo, aceitando `workers` só pra o mesmo `.ps` rodar nos dois motores).
- **`reload=True`** — reinicia sozinho quando o código muda (dev).
- Keep-alive multiplexado no event loop — conexões ociosas não travam o servidor.

## Gerenciador de pacotes (`psl`)

```bash
psl install arquivo.ps            # instala como comando global (rodável de qualquer lugar)
psl install arquivo.ps -asLib     # instala como lib importável (`import nome`)
psl install nome -py              # instala uma lib Python via pip
psl install nome [-asLib]         # nome sem extensão e sem -py: busca no registro configurado

psl uninstall nome [-asLib | -py] # remove (acha a categoria sozinho, ou desambigua pela flag)
psl list                          # lista comandos / libs PoolScript / libs Python instalados

psl registry set-url <url>        # aponta pro seu próprio índice (JSON {"nome": "url_do_arquivo"})
psl registry show                 # mostra o registro configurado
```

Tudo vive em `~/.poolscript/` (ou `$POOLSCRIPT_HOME`): comandos e libs
instalados, o que está registrado (`installed.json`) e o registro configurado
(`config.json`). Comandos globais viram um shim em `~/.poolscript/bin`,
adicionado automaticamente ao PATH no Windows na primeira instalação. Não há
dependência de índice de terceiros — **o registro é seu**.

## Erros e regras especiais

Erros trazem **traceback completo** (arquivo, linha e coluna, com `^^^` sob o
ponto exato), idêntico nos dois motores.

| Código | Quando ocorre |
|---|---|
| `AtributtedValueError` | Atribuir/somar tipos incompatíveis (ex: `str + int`) |
| `OutputUnexpectedValues` | Redeclarar variável no mesmo escopo |
| `SomeValueUnexpected` | Operação matemática inválida entre tipos |
| `IndexOutOfBoundsWarning` | Índice fora do range — **não trava**, retorna `Null` e avisa |

- `Null == 0` retorna `True` (igualdade nullish); magnitude com `Null` (`Null > 0`) retorna `False`.
- `bool` conta como `int` (0/1) em comparação e aritmética.
- `NOME` ≠ `nome` (case sensitive). Variáveis começam com minúscula ou `_`; maiúsculas ficam para libs/classes.

## Built-ins globais (sem import)

`open(path, mode, encoding)` (com `r/w/a/rb/wb/…`; binário lê/escreve `bytes`),
`len`, `range`, `type`, `map`, `input`, `post`, além do bloco `using ... as` que
fecha o recurso automaticamente:

```pool
using open("dados.bin", "rb") as f {
    conteudo = f.read()      // bytes
}
```

## Rodar a suíte de testes

```bash
pip install -e ".[dev]"
pytest
```

## Estrutura do projeto

```
src/poolscript/     ← interpretador de referência (Python)
  ├── lexer.py        tokenizer (indent estilo Python + braces estilo C)
  ├── parser.py       recursivo descendente
  ├── interpreter.py  tree-walking: scopes, closures, entities, imports
  ├── cli.py          comandos pool/psl
  ├── pkgmgr.py       psl install/uninstall/list/registry
  └── stdlib/         bibliotecas padrão
vm/                 ← PSVM: a VM em C (binário `pool` + extensão CPython)
  ├── poolscript_vm.c   compilador + máquina virtual
  ├── ps_regex.c        engine de regex
  ├── ps_pkg.c          resolução de módulos/pacotes
  └── main.c            CLI do binário + traceback
psl-poolscript-vsix/  ← extensão VS Code (highlight + IntelliSense via parser real)
examples/             ← exemplos .ps
tests/                ← suíte pytest (inclui testes diferenciais interp × VM)
```

* Versão atual: **8.2.31**.

## Contato

kleberdevion@proton.me
