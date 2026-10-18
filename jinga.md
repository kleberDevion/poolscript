# Jinga

Linguagem de programação de **tipagem estática** — a legibilidade do Python,
a estrutura de blocos do JS/C e a conferência de tipos do Java, antes de rodar.
O bloco é `{ }`; a indentação é estética, não sintaxe.

Extensão reconhecida em tudo (rodar, importar, `jpkg install`, `jinga build`,
editor): **`.pr`**. Era `.ps`, `.psl` e `.p`; quem tem arquivo com uma delas
converte a pasta inteira com `jinga scripts/migra_pr.pr <pasta> --aplica
--libs`.

---

## Como funciona — a PSVM

Jinga roda numa **máquina virtual em C**: lexer → parser → compilador →
bytecode → VM, e no x86-64 o bytecode de cada funct vira código de máquina
na primeira vez que ela roda (`vm/ps_jit_x64.h`; o que o nativo não faz em
linha volta pro interpretador instrução a instrução). Tudo vive em `vm/` e
vira um binário só, o `jinga` (no repositório, `make pool` o gera como
`./pool`).

| Etapa | Arquivo |
|---|---|
| lexer | `vm/ps_lexer.c` |
| parser (recursive-descent, produz AST) | `vm/ps_parser.c` |
| compilador pra bytecode | `vm/ps_compiler.c` |
| máquina virtual + stdlib | `vm/jinga_vm.c` e `vm/ps_*.c` |
| camada de sistema (laço de eventos, aviso entre threads, pilha de fibra, memória do JIT, o próprio executável) | `vm/ps_so.h`, `vm/ps_so.c` |

O motor é o mesmo C em todo sistema (Linux, macOS Intel e Windows x64: o JIT
gera x86-64 nos três); o que muda de um sistema pro outro mora na camada de
sistema: `epoll`/`eventfd`/`/proc` no Linux, `kqueue`/pipe/
`_NSGetExecutablePath` no macOS, e no Windows `WSAPoll`, Fibers,
`CreateProcess`/pseudoconsole (ConPTY) e o repositório de certificados do
sistema.

A suíte de testes também é em C (`teste/`): cada caso roda o `jinga` de VERDADE
num subprocesso, então caso que mata a VM (segfault, SIGFPE) vira falha
relatada em vez de derrubar a bateria. Roda com
`make check`.

Dois comandos, o **mesmo** binário/pacote:
- **`jinga`** — RODA (`jinga arquivo.pr`, `jinga build`, `jinga repl`, `jinga --version`)
- **`jpkg`** — GERENCIA PACOTES (`jpkg install/uninstall/list`, `jpkg registry ...`)

`pool` e `psl` são os nomes de antes do rename e continuam instalados como
atalhos dos mesmos comandos (o servidor do editor também atende por
`poolscript-lsp`); `POOLSCRIPT_HOME` ainda é lida, `import poolscript.libs.X`
ainda importa, e uma `~/.poolscript` existente vira `~/.jinga` na primeira
chamada do `jpkg`, com aviso.

---

## Instalação

Um comando instala tudo — o binário (como `jinga` e `jpkg`), o servidor LSP, e o
tipo MIME + o ícone do `.pr`:

```bash
sudo ./instalar.sh              # não precisa de make nem de compilador
sudo ./instalar.sh --remover
```

Do repositório com o fonte, `sudo make install` faz o mesmo.

O instalador também **apaga qualquer Jinga antiga que responda pelo
comando**. Uma instalação velha em `~/.local/bin` vem antes de `/usr/local/bin`
e sequestra o `pool`: era o caso do shim em Python, que respondia
`ModuleNotFoundError` com a instalação nova
intacta e invisível logo atrás. São três frentes, porque tirar só uma não
resolve:

- **PATH** — `jinga`, `jpkg`, `jinga-lsp` e os nomes antigos (`pool`, `psl`,
  `poolscript-lsp`) em toda pasta que não seja a do
  prefixo. Inclui o PATH do usuário que chamou o `sudo`, lido de um shell
  *interativo*: o `.bashrc` do Debian retorna cedo quando não é interativo, e
  sem isso as pastas do Windows no WSL (`/mnt/c/.../npm`) não apareceriam. Lá
  o comando é um punhado de irmãos (`pool`, `pool.cmd`, `pool.ps1`, `pool.exe`),
  e todos saem.
- **alias** — `alias pool='...'` no `.bashrc` aponta pro caminho velho direto,
  sem passar pelo PATH, e sobrevive ao `hash -r`. As linhas que *definem* o
  alias dos comandos são removidas; o resto do arquivo fica, e uma cópia
  vai pra `<arquivo>.antes-da-jinga`.
- **cache do shell aberto** — esse o instalador não alcança de fora. No
  terminal que já estava aberto: `unalias jinga jpkg pool psl 2>/dev/null; hash -r`, ou
  abra um novo.

### Numa máquina onde não há nada

Num WSL Debian recém-criado, por exemplo, não há nem `curl` nem compilador —
e mesmo assim é uma linha só:

```bash
curl -fsSL https://raw.githubusercontent.com/kleberDevion/poolscript/main/instalar.sh | sudo bash
```

O instalador busca o pacote pronto do último release; não havendo release
publicado, ele clona o fonte, instala as dependências de compilação e compila.
Instala também o `node` se faltar, porque o servidor LSP precisa dele em tempo
de execução. Nos dois caminhos não sobra passo manual.

Depois disso o `.pr` é **`text/jinga`** e aparece com a logo da linguagem
no gerenciador de arquivos. (A extensão já foi `.ps`, e aí era preciso tomá-la
do PostScript. Com `.pr` não há disputa: o PostScript fica com o `.ps` dele.)

### macOS (Intel)

O pacote do Mac (`jinga-macos-x86_64.tar.gz`) já traz as bibliotecas ao lado
do binário e roda sem Homebrew:

```bash
tar xzf jinga-macos-x86_64.tar.gz && cd jinga-macos-x86_64
sudo ./instalar.sh              # /usr/local: jinga, jpkg, jinga-lsp
```

Compilando no próprio Mac (Homebrew; no Mac Intel ele compila as
dependências do fonte na primeira vez):

```bash
./pool scripts/compila.pr mac --instala   # dependências, make, suíte, pacote e instalação
```

### Windows (x64)

O pacote do Windows (`jinga-windows-x86_64.zip`) traz o `jinga.exe`, as DLLs
dos clientes de banco, o servidor do editor e a doc. Instala na pasta do
usuário, sem administrador, e põe o `bin` no PATH:

```powershell
powershell -ExecutionPolicy Bypass -File instalar.ps1            # %LOCALAPPDATA%\Programs\Jinga
powershell -ExecutionPolicy Bypass -File instalar.ps1 -Remover
```

`jpkg`, `pool`, `psl` e `jinga-lsp` são `.cmd` ao lado do `jinga.exe`. As
libs instaladas pelo `jpkg` ficam em `%USERPROFILE%\.jinga`. Compilar é pelo
MSYS2 (ambiente UCRT64, com gcc: o laço da VM usa goto calculado, que o MSVC
não tem):

```bash
./pool scripts/compila.pr windows --instala   # dentro do terminal MSYS2 UCRT64
```

O que muda no Windows é o que o sistema muda:

- `os.cmd` roda pelo `cmd.exe` (não pelo `sh`), e `os.run` acha programas
  `.exe` (um `.cmd`, como o `npm`, só pelo `os.cmd`).
- `Process.kill(sinal)` encerra o processo (o Windows não tem sinal); o
  `wait()` devolve `-sinal`, como no Linux.
- Com `pty=True` o processo roda num pseudoconsole (ConPTY): a saída vem com
  as sequências de terminal do console (a sessão abre com os pedidos dele ao
  terminal, antes do texto do programa: leia até `read()` devolver `""`), e o
  fim da entrada (`close()`) é Ctrl+Z e Enter, o do teclado do Windows.
- `os.rename` e `os.move` substituem o destino que já existe, como no Linux
  e no macOS (o `rename` do sistema recusava).
- `create_server(..., reuse_port=True)` é `ValueError` (não existe
  `SO_REUSEPORT`).
- O jinker com `workers>1` roda o programa de novo em cada worker (não há
  `fork`), que recebe o socket da porta já aberto: o que o programa faz antes
  do `run()` roda também em cada worker. Com `reload=True`, o processo que o
  terminal esperou supervisiona e roda o programa de novo a cada mudança.

### Compilar pros três sistemas

```bash
./pool scripts/compila.pr               # o sistema desta máquina
./pool scripts/compila.pr todos         # os três: os outros dois compilam no GitHub
```

Pro sistema desta máquina ela confere as dependências, compila, roda a suíte
(com e sem o código de máquina) e monta o pacote em `dist/`. Pros outros, quem
compila é o próprio sistema, no GitHub Actions (`build.yml`): a ferramenta
dispara, acompanha e baixa o pacote. Precisa do `gh` logado
(`gh auth login`). Uma tag `v*` publica o release com os três pacotes.

### Compilando do fonte

Precisa de `gcc` e das libs de dev: postgresql, mysql, mongoc, openssl. Do lado
do PostgreSQL são **dois** pacotes: `libpq-dev` e `postgresql-server-dev-all` —
a libpq entra estática, e a `libpq.a` referencia símbolos que moram na
`libpgcommon.a`/`libpgport.a`, que só o segundo instala.

A lista completa, que é a mesma que o `instalar.sh` usa quando compila sozinho,
está em `DEPS_BUILD` no [`instalar.sh`](instalar.sh).

```bash
make pool      # gera ./pool na raiz
make check     # compila e roda a suíte em C
make verifica  # dependências dinâmicas e tamanho do ELF
```

### Testar
```bash
jinga --version
jinga examples/01_hello.pr
```

### Editor (opcional)
Servidor LSP: `jinga-lsp` (instalado pelo `make install`; sobre
`vscode-languageserver`, precisa de node). Vale pra VS Code, Neovim,
Helix e JetBrains — como ligar em cada um está em [`docs/lsp.md`](docs/lsp.md).

### Depurar

No VS Code: breakpoint na canaleta, **F5**. Sem `launch.json`. O motor fala
Debug Adapter Protocol direto (`jinga --debug <porta> <arquivo.pr>`), e no fim
mostra o **gráfico de execução** com a linha que quebrou em vermelho — ver
[`docs/debugger.md`](docs/debugger.md).

---

## Guia de deploy (rodar num servidor/VPS)

O binário **PSVM** (`jinga`) já traz **embutido** (não precisa instalar nada):

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
`dist/jinga-portable/` = binário + pasta `lib/` com todas as `.so`, e o wrapper
carrega de lá. É só copiar a pasta pro VPS e rodar.

O núcleo do glibc NÃO vai junto (levá-lo quebra o `getaddrinfo`), então o alvo
precisa ser x86-64 com **glibc ≥ 2.38** — o piso vem dos símbolos que o binário
referencia (`strlcpy`, `strlcat`, `__isoc23_strtol`, `fmod`). Isso cobre Debian
13, Ubuntu 24.04 e mais novos; Debian 12 (glibc 2.36) não roda o bundle, e
nessa máquina o instalador compila do fonte.

---

## Aprofundar

- **Sintaxe completa da linguagem** (tipos, `if`/`while`/`for`, `Entity`, `match`,
  decorators, enum, f-string, private/public…): [`docs/LANGUAGE.md`](docs/LANGUAGE.md)
- **Referência das libs** (uma pasta por lib, uma página por método) e os
  **objetos internos** (tipos que as libs devolvem): [`docs/INDEX.md`](docs/INDEX.md)
- **Editor / LSP** (VS Code, IntelliJ, Neovim): [`docs/lsp.md`](docs/lsp.md)
- **Depurador** (breakpoint, passo a passo, variáveis, gráfico de execução):
  [`docs/debugger.md`](docs/debugger.md)
- **Limites conhecidos e notas de projeto**: pasta [`notas/`](notas/)
- **Como buildar / versionar**: [`notas/CLAUDE.md`](notas/CLAUDE.md)

## Import — como um nome é resolvido

`import x` procura nesta ordem:

1. **lib da linguagem** (stdlib: `os`, `json`, `request`, `jinker`…)
2. **lib instalada** via `jpkg install ... -asLib` (`~/.jinga/libs/`)
3. **arquivo `.pr` do projeto** (relativo à raiz)

Ou seja: **uma lib SEMPRE ganha de um arquivo local de mesmo nome** — o nome
que você dá aos seus arquivos nunca ofusca uma lib. Um `random.pr` na pasta
não atrapalha `import random` achar a lib `random` (é o inverso do Python, de
propósito). Import de arquivo local do projeto usa caminho pontuado
(`from pkg.modulo import x`) ou relativo (`from .vizinho import y`).

A lib instalada também atende pelo nome qualificado: `import
jinga.libs.minhalib` liga `minhalib` e é o mesmo módulo do `import
minhalib`, onde quer que as libs estejam (`JINGA_HOME` incluso).

## Versionamento

Versão mais recente — **17.0.20** (a fonte é `vm/ps_versao.h`; `jinga --version`
mostra a do binário).
