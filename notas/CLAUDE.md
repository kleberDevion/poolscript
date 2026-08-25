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

A da **extensão VS Code** vive em `psl-poolscript-vsix/package.json` e é
independente da versão da linguagem.

Nunca edite na mão — use o script, que aplica o rollover e mantém tudo em
sincronia:

```bash
./rebuild/bump_version.py            # só valida, não altera
./rebuild/bump_version.py lang       # +1 na linguagem
./rebuild/bump_version.py ext        # +1 na extensão
./rebuild/bump_version.py lang ext   # ambas
```

Mudar a versão **força o relink**: o alvo `pool` depende de `ps_versao.h`, senão
o `pool --version` ficaria preso no valor antigo (as fontes `.c` não mudaram).

## Build

Tudo é C. O Makefile mora em `rebuild/`, mas as fontes e o binário são da RAIZ:

```bash
make -f rebuild/Makefile pool       # gera ./pool
make -f rebuild/Makefile check      # compila o binário + a suíte e roda
make -f rebuild/Makefile verifica   # ldd, símbolos de Python (tem que ser vazio), tamanho
make -f rebuild/Makefile bundle     # dist/pool-portable/ (VPS sem apt install)
```

Precisa de `gcc` e das libs de dev: postgresql, mysql, mongoc, openssl.

Entram **estáticos** no ELF: sqlite, libpq, mysqlclient, odbc, openssl, png,
expat, z. Ficam **dinâmicos**: `libmongoc`/`libbson` (não têm `.a` no sistema),
a cauda de auth do libpq (ldap/gssapi/gnutls/krb5), libX11, libgmp e o glibc.

### Bundle portátil (rodar em VPS sem apt install)

`make -f rebuild/Makefile bundle` roda o `build_bundle.sh` e monta
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
make -f rebuild/Makefile check    # tudo
./testar linguagem                # só um grupo
./testar -v "closure"             # filtra pelo nome do caso
```

Grupos: `crash`, `inteiros`, `erros`, `linguagem`, `pendentes`. O grupo
**`pendentes`** é a fila de trabalho: cada caso lá dentro codifica o
comportamento CORRETO de algo que ainda não funciona — ele falha de propósito
até a correção entrar. Nada de skip, nada de teste que passa escondendo erro.

## Scripts de apoio: em PoolScript

Automação, auditoria e smoke tests de apoio são escritos em `.ps` e rodados no
`./pool`. Todo tropeço escrevendo `.ps` é bug ou limitação candidata — anotar em
`notas/LIMITACOES.md` ou corrigir na hora.

## Extensão VS Code

O cérebro do completion é `psl-poolscript-vsix/extension.js` e o modelo de tipos
que ele consulta é `psl-poolscript-vsix/bridge/metadata.json` (cada função de lib
com seus PARÂMETROS, cada classe com seus MEMBROS e TIPO DE RETORNO). É o que faz
o autocomplete ser type-aware — a cadeia `conn = psodbc.connect()` →
`DbConnection` → `conn.cursor()` → `DbCursor` → `fetchall/fetchone/...` — e, se o
tipo é desconhecido, NÃO sugere nada (nada de método falso).

O binário expõe a mesma informação com `pool --metadata` (módulos, membros,
tipos e métodos, lidos das tabelas do próprio VM). É a fonte pra manter o
`metadata.json` em dia sem escrever nada à mão.

Testes (harness Node que dirige o `extension.js` real, sem VS Code):

```bash
node psl-poolscript-vsix/test/completion.test.js   # cenários (cadeia DB, arg nomeado)
node psl-poolscript-vsix/test/coverage.test.js     # varre TODO o metadata (100%)
node psl-poolscript-vsix/test/hover.test.js
```

Build do `.vsix`: `cd psl-poolscript-vsix && npx @vscode/vsce package
--allow-star-activation --skip-license`.

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
