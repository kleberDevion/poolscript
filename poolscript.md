# PoolScript

Linguagem de programação **híbrida (dinâmica/estática)** — a legibilidade do
Python com a estrutura de blocos do JS/C. O bloco é `{ }`; a indentação é
estética, não sintaxe.

Extensões reconhecidas em tudo (rodar, importar, `psl install`, `pool build`,
editor): **`.ps`**, **`.psl`**, **`.p`**.

---

## Como funciona — a PSVM

PoolScript roda numa **máquina virtual em C**: lexer → parser → compilador →
bytecode → VM. Tudo vive em `vm/` e vira um binário só, o `pool`.

| Etapa | Arquivo |
|---|---|
| lexer | `vm/ps_lexer.c` |
| parser (recursive-descent, produz AST) | `vm/ps_parser.c` |
| compilador pra bytecode | `vm/ps_compiler.c` |
| máquina virtual + stdlib | `vm/poolscript_vm.c` e `vm/ps_*.c` |

A suíte de testes também é em C (`teste/`): cada caso roda o `pool` de VERDADE
num subprocesso, então caso que mata a VM (segfault, SIGFPE) vira falha
relatada em vez de derrubar a bateria. Roda com
`make check`.

Dois comandos, o **mesmo** binário/pacote:
- **`pool`** — RODA (`pool arquivo.ps`, `pool build`, `pool repl`, `pool --version`)
- **`psl`** — GERENCIA PACOTES (`psl install/uninstall/list`, `psl registry ...`)

---

## Instalação

Um comando instala tudo — o binário (como `pool` e `psl`), o servidor LSP, e o
tipo MIME + o ícone do `.ps`:

```bash
sudo ./instalar.sh              # não precisa de make nem de compilador
sudo ./instalar.sh --remover
```

Do repositório com o fonte, `sudo make install` faz o mesmo.

Depois disso o `.ps` é **`text/poolscript`** e aparece com a logo da linguagem
no gerenciador de arquivos. (O `.ps` era do PostScript; aqui ele é da
linguagem. `.eps` e `.ai` continuam do PostScript.)

### Compilando do fonte

Precisa de `gcc` e das libs de dev: postgresql, mysql, mongoc, openssl.

```bash
make pool      # gera ./pool na raiz
make check     # compila e roda a suíte em C
make verifica  # dependências dinâmicas e tamanho do ELF
```

### Testar
```bash
pool --version
pool examples/01_hello.ps
```

### Editor (opcional)
Servidor LSP em PoolScript: `pool lsp/servidor.ps`. Vale pra VS Code, Neovim,
Helix e JetBrains — como ligar em cada um está em [`docs/lsp.md`](docs/lsp.md).

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
  decorators, enum, f-string, private/public…): [`docs/LANGUAGE.md`](docs/LANGUAGE.md)
- **Referência das libs** (uma pasta por lib, uma página por método) e os
  **objetos internos** (tipos que as libs devolvem): [`docs/INDEX.md`](docs/INDEX.md)
- **Editor / LSP** (VS Code, IntelliJ, Neovim): [`docs/lsp.md`](docs/lsp.md)
- **Limites conhecidos e notas de projeto**: pasta [`notas/`](notas/)
- **Como buildar / versionar**: [`notas/CLAUDE.md`](notas/CLAUDE.md)

## Import — como um nome é resolvido

`import x` procura nesta ordem:

1. **lib da linguagem** (stdlib: `os`, `json`, `request`, `jinker`…)
2. **lib instalada** via `psl install ... -asLib` (`~/.poolscript/libs/`)
3. **arquivo `.ps/.psl/.p` do projeto** (relativo à raiz)

Ou seja: **uma lib SEMPRE ganha de um arquivo local de mesmo nome** — o nome
que você dá aos seus arquivos nunca ofusca uma lib. Um `random.psl` na pasta
não atrapalha `import random` achar a lib `random` (é o inverso do Python, de
propósito). Import de arquivo local do projeto usa caminho pontuado
(`from pkg.modulo import x`) ou relativo (`from .vizinho import y`).

## Versionamento

Versão mais recente — **8.3.66** (a fonte é `vm/ps_versao.h`; `pool --version`
mostra a do binário).
