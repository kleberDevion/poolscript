# PoolScript

## Versionamento

O projeto **não usa semver**. Os dígitos 2 e 3 vão de `0` a `99`; ao chegar em
100 eles zeram e somam 1 no dígito à esquerda (base 100):

```
8.2.18  -> 8.2.19
8.2.99  -> 8.3.0     (100 nunca aparece)
8.99.99 -> 9.0.0
```

A versão da **linguagem** vive em quatro arquivos que precisam bater entre si:

- `pyproject.toml`
- `src/poolscript/__init__.py`
- `installer/pool_installer.iss`
- `docs/PoolScript.md` (título + exemplo do `pool --version` + banner do REPL —
  o script atualiza as três citações de uma vez, pra doc não ficar defasada)

A da **extensão VS Code** vive em `psl-poolscript-vsix/package.json` e é
independente da versão da linguagem.

Nunca edite esses arquivos na mão — use o script, que aplica o rollover e
mantém os três arquivos da linguagem sincronizados:

```bash
./bump_version.py            # só valida, não altera
./bump_version.py lang       # +1 na linguagem
./bump_version.py ext        # +1 na extensão
./bump_version.py lang ext   # ambas
```

(o sistema pode não ter o comando `python`, só `python3` — por isso o script
é executável direto; alternativamente `python3 bump_version.py ...`)

## Extensão VS Code: metadata type-aware

`psl-poolscript-vsix/bridge/gen_metadata.py` **introspecta a stdlib real** e
gera `bridge/metadata.json` — cada função de lib com seus PARÂMETROS e cada
classe com seus MEMBROS e TIPO DE RETORNO. É o que faz o autocomplete ser
type-aware (a cadeia `conn = psodbc.connect()` → `DbConnection` →
`conn.cursor()` → `DbCursor` → `fetchall/fetchone/...`, mais argumento nomeado
`connect(driver=...)`). O `extension.js` resolve tipo por essa cadeia e, se o
tipo é desconhecido, NÃO sugere nada (nada de método falso).

Sempre que mexer nas assinaturas/classes da stdlib, regenere:

```bash
PYTHONPATH=src python3 psl-poolscript-vsix/bridge/gen_metadata.py
```

Testes (harness Node que dirige o `extension.js` real, sem VS Code):

```bash
node psl-poolscript-vsix/test/completion.test.js   # cenários (cadeia DB, arg nomeado)
node psl-poolscript-vsix/test/coverage.test.js     # varre TODO o metadata (100%)
```

Build do `.vsix`: `cd psl-poolscript-vsix && npx @vscode/vsce package
--allow-star-activation --skip-license` (o `.vsix` é gitignored).

## Doc: assinatura vem do CÓDIGO, nunca digitada

O título `# \`lib.fn(a, b=1)\`` de cada página `docs/<lib>/<m>/<m>.md` (e
`docs/<lib>/<Classe>/<m>/<m>.md`) tem que bater com a assinatura real da
stdlib — nomes E defaults. Auditoria (escrita em PoolScript):

```bash
PYTHONPATH=src python3 scripts/audita_doc_sigs.py   # introspecta -> .audita_sigs.json
./pool scripts/audita_doc.ps                         # relata divergências
APLICA=1 ./pool scripts/audita_doc.ps                # reescreve os títulos
```

`docs/string` e `docs/builtins` ficam fora (são gerados de `scripts/doc_specs_*.py`
por `PYTHONPATH=.:src python3 scripts/gera_doc.py`). As páginas dos elementos
da guzer e a tabela do índice saem de `./pool scripts/gera_guzer_docs.ps`.

## Scripts de apoio: em PoolScript

Automação, auditoria e smoke tests de apoio são escritos em `.ps` e rodados no
`./pool` (e no interp, pra pegar divergência de graça). Python só pro que a PS
não alcança (introspectar a stdlib Python). Todo tropeço escrevendo `.ps` é bug
ou limitação candidata — anotar em `notas/LIMITACOES.md` ou corrigir na hora.

## Build do binário

`pool.spec` (PyInstaller) precisa que as libs opcionais estejam instaladas no
ambiente de build — `jinker`/`request` (websockets), `qrcode` (Pillow) e
`psodbc` (pyodbc/psycopg2/pymongo/mysql-connector) importam sob demanda dentro
de funções, então o PyInstaller só as empacota se conseguir importá-las:

```bash
pip install ".[all]"     # + unixodbc no sistema, pro pyodbc
pyinstaller pool.spec --noconfirm
```

O binário Linux publicado é `dist/pool-linux` (o PyInstaller gera `dist/pool`,
que é renomeado).

### Bundle portátil da VM (rodar em VPS sem apt install)

O binário `pool` da VM em C (`make pool`) já embute estático o essencial
(sqlite, libpq, mysqlclient, odbc, openssl). Mas ainda depende dinâmico de
`libmongoc`/`libbson` (mongo — não têm `.a` no sistema) e da cauda de auth do
libpq (ldap/gssapi/gnutls/krb5...), que num VPS "pelado" faltam e o binário não
sobe.

Solução sem static-linking frágil: `make bundle` (roda `build_bundle.sh`).
Ele monta `dist/pool-portable/` = `pool` (wrapper) + `pool.bin` + `lib/` com
TODAS as `.so` (via `ldd`), e o wrapper aponta `LD_LIBRARY_PATH` pra esse `lib/`.
NÃO empacota o núcleo do glibc (libc/m/pthread/dl/rt/resolv + loader) — esse vem
do alvo, senão o getaddrinfo/DNS (NSS) quebra. glibc é retrocompatível, então o
único requisito do VPS é ter glibc ≥ a do build. Gera também
`dist/pool-portable.tar.gz`; no VPS: `tar xzf … && ./pool-portable/pool app.ps`.
