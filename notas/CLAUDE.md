# PoolScript

## Versionamento

O projeto **não usa semver**. Os dígitos 2 e 3 vão de `0` a `99`; ao chegar em
100 eles zeram e somam 1 no dígito à esquerda (base 100):

```
8.2.18  -> 8.2.19
8.2.99  -> 8.3.0     (100 nunca aparece)
8.99.99 -> 9.0.0
```

A versão da **linguagem** tem uma fonte só, e é constante de C:

- `vm/ps_versao.h` — `#define PS_VERSAO "..."`, que o compilador embute no
  binário (nada de scrape de arquivo no build);
- `docs/PoolScript.md` cita a mesma versão no título, no exemplo do
  `pool --version` e no banner do REPL — sobe junto, pra doc não defasar.

Nunca edite na mão — use o script, que aplica o rollover e mantém tudo em
sincronia:

```bash
./pool bump_version.ps        # só valida, não altera
./pool bump_version.ps lang   # +1 na linguagem
```

Mudar a versão **força o relink**: o alvo `pool` depende de `ps_versao.h`, senão
o `pool --version` ficaria preso no valor antigo (as fontes `.c` não mudaram).

## Build

Tudo é C. Makefile, fontes e binário na raiz:

```bash
make pool       # gera ./pool
make check      # compila o binário + a suíte e roda
make verifica   # ldd, símbolos de Python (tem que ser vazio), tamanho
make bundle     # dist/pool-portable/ (VPS sem apt install)
```

Precisa de `gcc` e das libs de dev: postgresql, mysql, mongoc, openssl.

Entram **estáticos** no ELF: sqlite, libpq, mysqlclient, odbc, openssl, png,
expat, z. Ficam **dinâmicos**: `libmongoc`/`libbson` (não têm `.a` no sistema),
a cauda de auth do libpq (ldap/gssapi/gnutls/krb5), libX11, libgmp e o glibc.

### Bundle portátil (rodar em VPS sem apt install)

`make bundle` roda o `build_bundle.sh` e monta
`dist/pool-portable/` = `pool` (wrapper) + `pool.bin` + `lib/` com TODAS as
`.so` (via `ldd`); o wrapper aponta `LD_LIBRARY_PATH` pra esse `lib/`. NÃO
empacota o núcleo do glibc (libc/m/pthread/dl/rt/resolv + loader) — esse vem do
alvo, senão o getaddrinfo/DNS (NSS) quebra. glibc é retrocompatível, então o
único requisito do VPS é ter glibc ≥ a do build. Gera também
`dist/pool-portable.tar.gz`; no VPS:
`tar xzf … && ./pool-portable/pool app.ps`.

## Testes: em C, rodando o binário de verdade

A suíte vive em `teste/`. Cada caso é um programa `.ps` + o que se espera dele
(stdout exato, trecho do stderr, código de saída), e o runner faz **fork/exec do
`./pool`** — um subprocesso por caso. Isso é deliberado: boa parte dos casos
existe porque a VM MORREU (segfault, SIGFPE, OOM) rodando aquele fonte, e teste
que morre junto não relata nada. Com subprocesso, morte vira resultado
(`WIFSIGNALED`), e trava vira `SIGALRM`.

```bash
make check    # tudo
./testar linguagem                # só um grupo
./testar -v "closure"             # filtra pelo nome do caso
```

Grupos: `crash`, `inteiros`, `erros`, `linguagem`, `pendentes`, `cobertura`,
`diferencial`, `equivalencia`, `oraculo`, `robustez`. O grupo
**`pendentes`** é a fila de trabalho: cada caso lá dentro codifica o
comportamento CORRETO de algo que ainda não funciona — ele falha de propósito
até a correção entrar. Nada de skip, nada de teste que passa escondendo erro.

**Um número só.** Não existe varredura "por fora" que rode à parte e dê outro
placar — tudo o que se mede entra em `make check`. Três grupos são **gerados**
(não se edita na mão; se o comportamento mudou, regera e o diff mostra o que
mudou):

| Grupo | Gerador | O que trava |
|---|---|---|
| `diferencial` | — (colhido de um commit) | a saída de ontem, caso a caso |
| `equivalencia` | `teste/geradores/equivalencia.ps` | formas redundantes têm que concordar entre si |
| `oraculo` | `teste/geradores/oraculo.ps` | 4484 expressões contra o **Python** como oráculo |
| `robustez` | `teste/geradores/robustez.ps` | ~11,9 mil chamadas com aridade/tipo errados |

Os geradores são PoolScript rodando no `./pool` — não há Python nem JavaScript
no projeto. O `oraculo` consulta o CPython como ferramenta **externa** (escreve
um driver em `/tmp` na hora e o chama), do mesmo jeito que outro teste consulta
um banco de dados: nenhum `.py` fica versionado aqui.

O `oraculo` guarda o valor que a linguagem produz hoje e marca `DIVERGE` nos
casos em que o Python daria outra coisa, com o valor dele no comentário — a
diferença fica no arquivo, visível, em vez de sumir. As 64 divergências de hoje
são decisão de projeto (índice fora da faixa → `null`, `.len()` como método,
`.keys()` devolvendo lista, `type()` com nome curto).

O `robustez` é um caso por TIPO, não por chamada — 11,9 mil subprocessos
levariam mais de um minuto. Cada programa roda as chamadas erradas do seu tipo
em `try/catch` e imprime quantas passaram **caladas** e em quais métodos.
Crash e trava viram falha pelo `WIFSIGNALED`/`SIGALRM` do runner.

## Scripts de apoio: em PoolScript

Automação, auditoria e smoke tests de apoio são escritos em `.ps` e rodados no
`./pool`. Todo tropeço escrevendo `.ps` é bug ou limitação candidata — anotar em
`notas/LIMITACOES.md` ou corrigir na hora.

## Editor: servidor LSP, em PoolScript

O suporte a editor é um **servidor LSP escrito em PoolScript** (`lsp/`), rodado
pelo `pool`. Não há JavaScript no projeto, e não há extensão VS Code
versionada aqui — qualquer editor que fale LSP conversa com esse servidor.

O modelo de tipos vem do próprio binário: `pool --metadata` lista módulos,
membros, tipos e métodos lidos das tabelas do VM. É o que alimenta o hover —
a assinatura real e o tipo de retorno, sem nada digitado à mão.

**Não há completion**, por decisão de projeto: o que ele oferecia sem receptor
era a lista de palavras da linguagem, cada uma rotulada "palavra da
linguagem". O servidor não anuncia `completionProvider`.

## Doc: assinatura vem do CÓDIGO, nunca digitada

O título `# \`lib.fn(a, b=1)\`` de cada página `docs/<lib>/<m>/<m>.md` (e
`docs/<lib>/<Classe>/<m>/<m>.md`) tem que bater com a assinatura real que o
motor expõe — nomes E defaults. A fonte é `pool --metadata`, não a memória de
ninguém.

Todo método de `str`/`list`/`dict`/`tup` do motor precisa ter página: as tabelas
`METODOS_*` de `vm/poolscript_vm.c` são a lista de verdade.

## Regra que não se negocia

Nada de falso verde. Skip, `try` que engole erro, teste ajustado pro bug —
nenhum dos três. Se não passa, ou corrige o motor, ou o caso vai pra
`teste/casos_pendentes.c` falhando de propósito, com o comportamento certo
escrito.
