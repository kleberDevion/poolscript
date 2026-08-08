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

## Extensão VS Code: cópia do parser

`psl-poolscript-vsix/bridge/poolscript_pkg/` é uma **cópia** de
`lexer.py`/`parser.py`/`ps_errors.py` — o bridge de análise estática nunca
importa o pacote `poolscript` inteiro, pra garantir que não executa código do
usuário. Sempre que mexer nesses três arquivos, sincronize:

```bash
python3 psl-poolscript-vsix/bridge/sync_parser.py
```

Sem isso o editor acusa erro de sintaxe em código válido (a cópia fica parada
numa versão antiga da linguagem).

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
