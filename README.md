# 🏊 PoolScript

PoolScript é uma linguagem de programação **híbrida (dinâmica/estática)** que mistura a legibilidade do Python com a estrutura de blocos do JS/C. Esta é a implementação de referência do interpretador, escrita em Python puro (sem dependências externas obrigatórias).

## Instalação

```bash
unzip poolscript.zip
cd poolscript
pip install -e .          # registra os comandos `pool` e `psl` no PATH
```

Ou rode direto sem instalar:

```bash
python -m poolscript examples/hello.ps
```

## Uso rápido

```bash
pool examples/hello.ps        # roda um arquivo
pool build                    # roda todos os .ps da pasta atual
pool --version                # versão
psl --help                    # ajuda
psl //doc                     # URL da spec
```

## Gerenciador de pacotes (`psl install`)

`psl install` tem três modos, escolhidos pelo alvo e por flags:

```bash
psl install arquivo.ps            # instala como comando global (rodável de qualquer lugar)
psl install arquivo.ps -asLib     # instala como lib importável (`import nome` em qualquer script)
psl install nome -py              # instala uma lib Python via pip
psl install nome [-asLib]         # nome sem .ps e sem -py: busca no registro configurado

psl uninstall nome [-asLib | -py] # remove o que foi instalado (mesmas flags do install)
psl list                          # lista comandos / libs PoolScript / libs Python instalados

psl registry set-url <url>        # aponta para o seu próprio índice (JSON {"nome": "url_do_.ps"})
psl registry show                 # mostra o registro configurado
```

Tudo fica em `~/.poolscript/` (ou `$POOLSCRIPT_HOME`, se definida): comandos e libs
instalados, o que está registrado (`installed.json`) e o registro configurado
(`config.json`). Comandos globais viram um shim em `~/.poolscript/bin`, que é
adicionado automaticamente ao PATH do usuário no Windows na primeira instalação.

Alvos terminados em `.ps` são resolvidos como arquivo local; qualquer outro nome
(sem `-py`) é procurado no índice remoto configurado via `psl registry set-url` —
não há dependência de nenhum índice de terceiros, o registro é seu.

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

// erros
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


## Validação por tipo e comparações

```pool
entrada = input("DIGITAR: ")

if (entrada is int) {
    post("E um inteiro")
} else {
    post("Nao e um inteiro")
}

nome = "Pool"
if (nome not is None) {
    post("nome existe")
}

nome1 = "A"
nome2 = "B"
nome3 = "C"
if (nome1 and nome2 and nome3 not is None) {
    post("todos existem")
}
```

Operadores suportados em `if`: `==`, `!=`, `===`, `!==`, `<`, `>`, `<=`, `>=`, `is`, `not is`, `is not`, `in`, `not in`, `and`, `or`, `not`, `!`.
Tipos validáveis: `str`, `int`, `flo`, `bool`, `list`, `json` e `None`/`Null`.

## Operador `count` (v0.5.2)

`count` é um utilitário multifuncional para validar existência e contar
ocorrências tipadas em coleções, strings ou números.

### Três formas sintáticas

```ps
list nums = [1, 7, 7, 17, 7]

// 1) Prefixo com valor
if (count int(7) in nums) {
    post("achei pelo menos um 7")
}
post(count int(7) in nums)         // 3

// 2) Forma infixa
post(int(7) count in nums)         // 3

// 3) Prefixo só com tipo (sem valor) — conta todos do tipo
post(count int in nums)            // 5
```

### Onde funciona

| Container | `count <tipo>(<valor>) in <container>` |
|---|---|
| `list` / tupla | conta itens iguais a `<valor>` que sejam do `<tipo>` |
| `json` (dict) | conta valores iguais a `<valor>` que sejam do `<tipo>` |
| `str` (texto) | conta substring `<valor>` no texto |
| `int` (número) | conta o(s) dígito(s) de `<valor>` na representação do número |

```ps
str frase = "oi mundo oi pessoal oi"
post(count str("oi") in frase)     // 3

int n = 17717
post(count int(7) in n)            // 3   ← três 7s nos dígitos

int par = 67
post(count int(7) in par)          // 1   ← rastreia o 7 dentro do par 67

json scores = {"joao": 7, "ana": 8, "leo": 7}
post(count int(7) in scores)       // 2
```

### Bloco `count each`

Executa o bloco a cada ocorrência. Dentro do bloco você tem acesso a:

- `_match` — o item encontrado.
- `_index` — posição da ocorrência.
- `_count` — total acumulado até aqui.

`return;` (sem valor) faz a action enclosing devolver o **total final**.
`return <expr>` interrompe imediatamente devolvendo `<expr>`.

```ps
list nums = [7, 7, 8, 7]

action contarSetes() {
    count each int(7) in nums {
        return;       // → devolve 3 ao final
    }
}
post(contarSetes())                // 3

action primeiroSete() {
    count each int(7) in nums {
        return _index    // → devolve 0 (curto-circuito na 1ª ocorrência)
    }
}
post(primeiroSete())               // 0
```

### Combinando em `if`

`count` retorna `int` — `0` é falsy, qualquer `> 0` é truthy. Combina com tudo:

```ps
if (count int(1) in a and count int(3) in b) { post("ambas têm") }
if (count int(9) in xs == 3) { post("exatamente 3 noves") }
if (not count str("zzz") in frase) { post("não tem zzz") }
```

## Bibliotecas padrão

Implementadas e funcionando:

| Lib | Funções |
|---|---|
| `os` | `pathFile`, `pathFolder`, `getenv` |
| `json` | `parse`, `stringify` |
| `dotenv` | `load` (carrega `.env` automaticamente) |
| `request` | `get`, `post`, `put`, `delete` (retorna objeto com `.status`, `.text`, `.get_json(chave)`) |

Stubs (declaradas mas levantam erro amigável quando chamadas): `sqlite`, `smtplib`, `mimetext`, `multipart`, `flask`.

## Tabela de erros

| Código | Quando ocorre |
|---|---|
| `AtributtedValueError` | Atribuir/somar tipos incompatíveis (ex: `str + int`) |
| `OutputUnexpectedValues` | Redeclarar variável no mesmo escopo |
| `SomeValueUnexpected` | Operação matemática inválida entre tipos |
| `IndexOutOfBoundsWarning` | Acesso a índice fora do range — **não trava**, retorna `Null` e imprime aviso |

## Regras especiais (do spec)

- `Null == 0` retorna `True` (igualdade nullish).
- Comparações de magnitude com `Null` (`Null > 0`, etc.) retornam `False`.
- `NOME` ≠ `nome` (case sensitive).
- Variáveis começam com letra minúscula ou `_`. Maiúsculas reservadas para libs/classes.

## Rodar a suíte de testes

```bash
pip install -e ".[dev]"
pytest
```

## Estrutura do projeto

```
src/poolscript/
├── lexer.py        ← tokenizer com indent estilo Python + braces estilo C
├── parser.py       ← recursivo descendente, ~25 nós de AST
├── interpreter.py  ← tree-walking, scopes, closures, imports
├── errors.py       ← códigos de erro nomeados
├── cli.py          ← comandos pool/psl
├── pkgmgr.py       ← gerenciador de pacotes do psl install/uninstall/list/registry
├── __main__.py     ← `python -m poolscript`
└── stdlib/
    ├── __init__.py     ← registry
    ├── os_lib.py
    ├── json_lib.py
    ├── dotenv_lib.py
    └── request_lib.py
examples/   ← arquivos .ps de exemplo
tests/      ← suíte pytest
```

## Limitações desta versão

- `Class` / `class` / `type` são keywords reservadas mas o parser/interpreter ainda não suportam declaração de classes.
- Decorators (`@route`, `@app`) são parseados mas ignorados (no-op).
- `psl -up release` é stub (sem auto-update).
- `async` / `await`, `listen`, `route` HTTP — reservados, sem implementação.

## Contato

📧 poolscript@proton.me

---

## 🆕 v0.3.0 — Novos built-ins

### Built-ins globais (sem import)
- `open(path, mode)` — abre arquivo. Modo: `r`, `w`, `a`, `rb`, `wb`...
- `len(x)` — comprimento de string/lista/json
- `range(n)` / `range(a, b)` — lista de inteiros
- `type(x)` — nome do tipo (`"int"`, `"str"`, `"Null"`, `"json"`...)

### Bloco `using ... as` (context manager)
Fecha o recurso automaticamente, mesmo se der erro:

```poolscript
using open("log.txt", "a") as f {
    f.write("nova linha\n")
}
```

### Lib `date`
```poolscript
import date
post(date.datahora())   // "23/04/2026 14:30:55"
post(date.today())      // "23/04/2026"
post(date.time())       // "14:30:55"
```

### Lib `mail`
SMTP com auto-detecção de provedor (gmail, outlook, yahoo, hotmail, proton):

```poolscript
import mail

server = mail.MailServer()
server.conn("gmail.com")              // porta/host automáticos
server.login("user@gmail.com", "senha-de-app")

msg = mail.MailMessage()
msg.from_address("user@gmail.com")
msg.to("destino@site.com")
msg.subject("Relatório")
msg.body("Segue <b>PDF</b>", true)    // true = HTML
msg.attach("/tmp/relatorio.pdf")

server.send(msg)
server.quit()
```

### Lib `mail` — leitura (`MailReader`, v0.6.0)
IMAP com auto-detecção de provedor (gmail, outlook, yahoo, hotmail):

```poolscript
import mail

reader = mail.MailReader()
reader.conn("gmail.com")                      // porta/host automáticos
reader.login("user@gmail.com", "senha-de-app")

reader.select("INBOX", true)                   // true = readonly (não marca como lida)
emails = reader.search("UNSEEN")
for each e in emails:
    post(e["from"] " — " e["subject"] " (" e["date"] ")")

// outros filtros: "ALL", "SUBJECT", "FROM", "SINCE"
recentes = reader.select().search("SINCE", "01-Jan-2026", 10)   // 10 últimos desde a data
do_joao   = reader.select().search("FROM", "joao@empresa.com")
sobre_nf  = reader.select().search("SUBJECT", "nota fiscal")

reader.close()
```

Cada item retornado é um dict: `{"id", "from", "subject", "date"}`.

### Lib `request` (atualizada)
- `patch()` adicionado
- Body dict/list vira JSON automaticamente
- User-Agent e Accept default (corrige 403)
- Erros de conexão/timeout retornam `{"error": "...", "message": "..."}`
