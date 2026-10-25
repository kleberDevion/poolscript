# `teste/e2e` — a linguagem exercitada contra recurso de VERDADE

Aqui não tem mock. Cada script usa o recurso real: arquivo no disco, banco de
pé, servidor HTTP servido pela própria linguagem. É o que pega o defeito que
só aparece quando a coisa atravessa — e foi assim que apareceram o `status()`
apagado pelo `json()`, o `Null` virando 400 em vez de 204, e o `?` que nunca
virava `$1` no PostgreSQL.

| script | precisa de | roda com |
|---|---|---|
| `arquivo.pr` | nada (no Linux e no macOS usa `/tmp/ps_e2e`; no Windows a pasta `ps_e2e_arquivo` na pasta corrente) | `./pool teste/e2e/arquivo.pr` |
| `sqlite.pr` | nada (sqlite é embutido) | `./pool teste/e2e/sqlite.pr` |
| `socket.pr` | nada (loopback) | `./pool teste/e2e/socket.pr` |
| `pkg.pr`, `qrcode.pr`, `c.pr`, `d.pr`, `f.pr` | nada | `./pool teste/e2e/<script>` |
| `mail_local.pr` | `python3` e `openssl` (os servidores falsos de SMTP/IMAP) | `./pool teste/e2e/mail_local.pr` |
| `jinker_srv.pr` + `jinker_cli.pr` + `jinker_bruto.pr` | nada (loopback) | ver abaixo |
| `db.pr` | PostgreSQL e/ou MySQL | ver abaixo |
| `mongo.pr` | mongod local | ver abaixo |

Os scripts que criam arquivo trabalham numa pasta `ps_e2e_*` da pasta de
onde rodam (a raiz do repositório, pelo `make`), apagam no fim e apagam a
sobra de uma rodada interrompida no começo da seguinte.

Script que não encontra o recurso imprime `PULOU` e o motivo — **não** passa
calado nem falha a suíte por ausência de servidor.

## Rodar tudo

```bash
make check-e2e                 # todos
make check-e2e E2E=jinker      # só o que casa com o filtro
```

O driver é `teste/e2e_roda.pr`, e ele conhece o PAPEL de cada script. Isso não
é detalhe: o alvo antigo era um `for` sobre `*.pr`, o shell ordena
alfabeticamente, e `jinker_cli.pr` rodava ANTES de `jinker_srv.pr` — cliente
sem servidor morria com "Connection refused" e o servidor ficava servindo até
o timeout. A orquestração certa existia aqui embaixo, em prosa, fora de tudo
que roda; agora está no código.

Três papéis, declarados no topo do driver:

| papel | o que o driver faz |
|---|---|
| **par** (servidor + cliente) | sobe o servidor como processo filho (`os.run(..., capture="live")`, sem shell), **espera a porta aceitar conexão** (tenta conectar, com prazo de relógio; não dorme no escuro), roda os clientes em ordem, e derruba o servidor aconteça o que acontecer: `kill()` (SIGTERM no Linux e no macOS e, se não sair em 6 s, SIGKILL; no Windows, TerminateProcess). Cliente de par que diz PULOU conta como FALHA: a porta já tinha aberto, então foi o servidor que caiu |
| **sozinho** | roda direto |
| **sem par** | servidor sem cliente — bloquearia até o timeout. É RELATADO, não rodado |

Todo script tem que ser **re-executável**: `jinker_srv.pr` falhava na primeira
linha porque `os.mkdir` da pasta estática não aceitava a pasta que sobrou da
rodada anterior. É a mesma família do E1 (teste que suja o ambiente) que a
suíte principal fechou com `mkdtemp`+`chdir`.

## jinker (servidor + cliente), na mão

O jeito curto é o próprio driver com o filtro: `./pool teste/e2e_roda.pr jinker`.
Pra depurar o servidor fora dele, dois terminais na MESMA pasta (a raiz do
repositório — servidor e cliente trocam arquivos pela pasta `ps_e2e_jinker`
de lá):

```bash
./pool teste/e2e/jinker_srv.pr      # terminal 1: fica servindo na 28840
./pool teste/e2e/jinker_cli.pr      # terminal 2, com o servidor de pé
./pool teste/e2e/jinker_bruto.pr    # terminal 2: o HTTP cru
```

Ctrl+C no terminal 1 derruba o servidor.

Cobre HTTP (rotas, status, tupla, 204, upload) e **upload multipart**
(`file()`/`files(campo)`, `PoolFileUpload` inteiro). O servidor de WebSocket
saiu da jinker no 19.0.0; o cliente (`request.ws_connect`) é testado no
`teste/jinker_roda.pr`, contra o par de eco `teste/ws_par.pr`.


## Bancos

Usuário e banco dedicados, chamados `ps_teste` nos dois. Criar:

```bash
sudo -u postgres psql -c "CREATE USER ps_teste WITH PASSWORD 'ps_teste';" \
                     -c "CREATE DATABASE ps_teste OWNER ps_teste;"

sudo mysql -e "CREATE DATABASE ps_teste;
               CREATE USER 'ps_teste'@'localhost' IDENTIFIED BY 'ps_teste';
               GRANT ALL ON ps_teste.* TO 'ps_teste'@'localhost';"
```

Remover quando não quiser mais:

```bash
sudo -u postgres psql -c "DROP DATABASE ps_teste;" -c "DROP USER ps_teste;"
sudo mysql -e "DROP DATABASE ps_teste; DROP USER 'ps_teste'@'localhost';"
```

A porta vem do ambiente quando a padrão já está ocupada (por um banco que não
é de teste): `PS_E2E_PG_PORTA`, `PS_E2E_MY_PORTA` e `PS_E2E_MS_PORTA`. Com
contêineres de teste, só no loopback:

```bash
docker run -d --name ps-teste-pg -p 127.0.0.1:55432:5432 \
  -e POSTGRES_USER=ps_teste -e POSTGRES_PASSWORD=ps_teste -e POSTGRES_DB=ps_teste postgres:16-alpine
docker run -d --name ps-teste-my -p 127.0.0.1:53306:3306 \
  -e MARIADB_USER=ps_teste -e MARIADB_PASSWORD=ps_teste -e MARIADB_DATABASE=ps_teste \
  -e MARIADB_RANDOM_ROOT_PASSWORD=1 mariadb:11
PS_E2E_PG_PORTA=55432 PS_E2E_MY_PORTA=53306 ./pool teste/e2e/db.pr
```

O SQL Server entra pelo ODBC 18 da Microsoft (o `msodbcsql18`), com o usuário
`sa` e a senha de `PS_E2E_MS_SENHA`; sem ela, o script diz `PULOU`:

```bash
docker run -d --name ps-teste-ms -p 127.0.0.1:51433:1433 --memory 2g \
  -e ACCEPT_EULA=Y -e MSSQL_SA_PASSWORD='<senha forte>' mcr.microsoft.com/mssql/server:2022-latest
PS_E2E_MS_PORTA=51433 PS_E2E_MS_SENHA='<senha forte>' ./pool teste/e2e/db.pr
```

## mongo

Sem tocar na instalação do sistema — sobe numa pasta vazia e numa porta
própria, em outro terminal:

```bash
mongod --dbpath <pasta vazia> --port 27099 --bind_ip 127.0.0.1 --quiet   # terminal 1
./pool teste/e2e/mongo.pr                                                # terminal 2
```

Ctrl+C no terminal 1 derruba o mongod.
