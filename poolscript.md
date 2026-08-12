# PoolScript

Linguagem de programação **híbrida (dinâmica/estática)** — a legibilidade do
Python com a estrutura de blocos do JS/C: indentação **ou** chaves, `:` **ou**
`{}`, à vontade e no mesmo arquivo.

Extensões reconhecidas em tudo (rodar, importar, `psl install`, `pool build`,
editor): **`.ps`**, **`.psl`**, **`.p`**.

---

## Como funciona — dois motores em paridade

PoolScript roda em **dois motores que produzem exatamente o mesmo resultado**:

| Motor | O que é | Onde vive | Pra quê |
|---|---|---|---|
| **INTERP** | Interpretador em Python (tree-walking) — a **autoridade semântica** | `src/poolscript/` | referência; roda com Python 3.10+, sem dependência obrigatória |
| **PSVM** | Máquina virtual em **C** (lexer→parser→compilador→bytecode→VM) | `vm/` | runtime de produção; binário `pool` |

Os dois são testados de forma **diferencial**: o mesmo programa tem que dar o
mesmo `stdout` **e** o mesmo texto de erro (traceback incluso). Se diverge, é
bug. São ~2700 testes garantindo isso.

Dois comandos, o **mesmo** binário/pacote:
- **`pool`** — RODA (`pool arquivo.ps`, `pool build`, `pool repl`, `pool --version`)
- **`psl`** — GERENCIA PACOTES (`psl install/uninstall/list`, `psl registry ...`)

---

## Instalação

### Jeito fácil — `pooler` (recomendado)

Um instalador que detecta o OS, **pergunta qual motor** (INTERP ou PSVM),
**remove instalação anterior** e resolve as dependências sozinho (no PSVM, se
faltar `.so` cai no bundle portátil). É o caminho sem precisar decorar comando:

**Standalone (sem clonar nada) — baixa só o script e ele instala via curl:**
```bash
# Linux / macOS — baixa o pooler e roda (menu interativo)
curl -fsSL https://raw.githubusercontent.com/kleberDevion/poolscript/main/installer/pooler.sh -o pooler.sh && sh pooler.sh
# ou direto, sem menu:
curl -fsSL https://raw.githubusercontent.com/kleberDevion/poolscript/main/installer/pooler.sh | sh -s -- --engine psvm --yes
```
```powershell
# Windows (PowerShell)
irm https://raw.githubusercontent.com/kleberDevion/poolscript/main/installer/pooler.ps1 | iex
```
Fora do repositório ele baixa o INTERP direto do GitHub (pip) e o PSVM do
**release** (`pool-linux` / `pool-portable.tar.gz`).

**Dentro do repositório clonado:**
```bash
./installer/pooler.sh              # menu; ou: --engine psvm|interp --yes  [--system]
.\installer\pooler.ps1             # Windows; ou: -Engine interp -Yes
```

Ou instale manual, escolhendo o motor:

### Versão INTERP (Python) — dev, portátil, sem compilar

```bash
pip install -e .        # registra `pool` e `psl` no PATH (usa o interpretador)
```
Pré-requisito: **Python 3.10+**. Rodar sem instalar: `python3 -m poolscript arquivo.ps`.

### Versão PSVM (binário C) — produção, sem Python

```bash
# usar o binário já publicado:
sudo install -m755 dist/pool-linux /usr/local/bin/pool
sudo install -m755 dist/pool-linux /usr/local/bin/psl

# ou compilar do fonte (precisa gcc + libs de dev: postgresql, mysql, mongoc, openssl):
./rebuild_vm.sh         # gera ./pool e a extensão CPython em src/poolscript/vm/
```

### Testar
```bash
pool --version
pool examples/01_hello.ps
```

### Extensão de editor (opcional)
Cliente LSP (VS Code / IntelliJ / Neovim) — ver [`docs/lsp.md`](docs/lsp.md).
VS Code direto: `code --install-extension psl-poolscript-vsix/*.vsix`.

---

## Guia de deploy (rodar num servidor/VPS)

O binário **PSVM** (`pool`) já traz **embutido** (não precisa instalar nada):

> **sqlite, libpq (postgres), mysqlclient, odbc, openssl (TLS), png, expat, z.**
> Ou seja: Todas as libs que são padrão da linguagem ja estão com suas dependencias embutidas no ELF, so existem
> algumas que são estaticas e e necessario o ``.so`` dela para o ELF chamar em Runtime

O binário **chama de fora** (a `.so` precisa existir no servidor):
- **Mongo** — `libmongoc-1.0-0 libbson-1.0-0 libmongocrypt0 libsnappy1v5` (só se usar MongoDB).
- **Cauda de auth do libpq** (LDAP/Kerberos/GnuTLS — `libldap-2.5-0 libsasl2-2 libgssapi-krb5-2 libgnutls30`.
- **Sistema** — `libltdl7 libzstd1 libstdc++6 libgcc-s1`.
- **glibc** (`libc`, `libm`, `pthread`, `dl`, `resolv`) — todo Linux já tem. 

**VPS (Debian/Ubuntu), uma linha:**
```bash
sudo apt update && sudo apt install -y \
  libmongoc-1.0-0 libbson-1.0-0 libmongocrypt0 libsnappy1v5 \
  libldap-2.5-0 libsasl2-2 libgssapi-krb5-2 libgnutls30 libzstd1 libltdl7
```
Se faltar alguma: `ldd ./pool | grep "not found"` mostra o nome exato.

**Alternativa sem instalar nada** — bundle portátil (`make bundle`): gera
`dist/pool-portable/` = binário + pasta `lib/` com todas as `.so`, e o wrapper
carrega de lá. É só copiar a pasta pro VPS e rodar. Requisito único do alvo:
glibc compatível (x86-64).

---

## Aprofundar

- **Sintaxe completa da linguagem** (tipos, `if`/`while`/`for`, `Entity`, `match`,
  decorators, enum, f-string, private/public…): [`LANGUAGE.md`](LANGUAGE.md)
- **Referência das libs** (uma pasta por lib, uma página por método) e os
  **objetos internos** (tipos que as libs devolvem): [`docs/INDEX.md`](docs/INDEX.md)
- **Editor / LSP** (VS Code, IntelliJ, Neovim): [`docs/lsp.md`](docs/lsp.md)
- **Limites conhecidos e notas de projeto**: pasta [`notas/`](notas/)
- **Como buildar / versionar** (base-100, os 4 arquivos de versão, bundle):
  [`CLAUDE.md`](CLAUDE.md)

## Import — como um nome é resolvido

`import x` procura nesta ordem (nos dois motores, idêntico):

1. **lib da linguagem** (stdlib: `os`, `json`, `request`, `jinker`…)
2. **lib instalada** via `psl install ... -asLib` (`~/.poolscript/libs/`)
3. **arquivo `.ps/.psl/.p` do projeto** (relativo à raiz)

Ou seja: **uma lib SEMPRE ganha de um arquivo local de mesmo nome** — o nome
que você dá aos seus arquivos nunca ofusca uma lib. Um `random.psl` na pasta
não atrapalha `import random` achar a lib `random` (é o inverso do Python, de
propósito). Import de arquivo local do projeto usa caminho pontuado
(`from pkg.modulo import x`) ou relativo (`from .vizinho import y`).

## Versionamento

Versão mas recente - 8.0.53
