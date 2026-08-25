# `teste/e2e` — a linguagem exercitada contra recurso de VERDADE

Aqui não tem mock. Cada script usa o recurso real: arquivo no disco, banco de
pé, servidor HTTP servido pela própria linguagem. É o que pega o defeito que
só aparece quando a coisa atravessa — e foi assim que apareceram o `status()`
apagado pelo `json()`, o `Null` virando 400 em vez de 204, e o `?` que nunca
virava `$1` no PostgreSQL.

| script | precisa de | roda com |
|---|---|---|
| `arquivo.ps` | nada (usa `/tmp`) | `./pool teste/e2e/arquivo.ps` |
| `sqlite.ps` | nada (sqlite é embutido) | `./pool teste/e2e/sqlite.ps` |
| `socket.ps` | nada (loopback) | `./pool teste/e2e/socket.ps` |
| `guzer.ps` | nada | `GUZER_HEADLESS=1 ./pool teste/e2e/guzer.ps` |
| `jinker_srv.ps` + `jinker_cli.ps` | nada (loopback) | ver abaixo |
| `db.ps` | PostgreSQL e/ou MySQL | ver abaixo |
| `mongo.ps` | mongod local | ver abaixo |

Script que não encontra o recurso imprime `PULOU` e o motivo — **não** passa
calado nem falha a suíte por ausência de servidor.

## jinker (servidor + cliente)

```bash
setsid ./pool teste/e2e/jinker_srv.ps > /tmp/ps_srv.log 2>&1 < /dev/null &
sleep 2
./pool teste/e2e/jinker_cli.ps
pkill -f jinker_srv.ps
```

Cobre HTTP (rotas, status, tupla, 204, upload), **upload multipart**
(`file()`/`files(campo)`, `PoolFileUpload` inteiro) e **WebSocket**:

- `/sala` (`channel=true`) — dois clientes, um só ouvindo num `sleep`. A
  mensagem tem que chegar **durante** a espera; era aqui que o cliente fora de
  fibra não drenava a conexão e só entregava no `close()`.
- `/ws` (sem canal) — o handler roda, mas o `return` dele **não** volta pro
  cliente: quem envia é o `emit`, e o `emit` só alcança conexão com
  `channel=true`.

## guzer

`GUZER_HEADLESS=1` monta a árvore inteira **sem abrir janela**. Sem essa
variável o script trava esperando você fechar a janela — não é defeito, é o
`guzer.UI()` sendo exibido ao fim do script por definição.

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

## mongo

Sem tocar na instalação do sistema — sobe num diretório temporário e numa
porta própria:

```bash
mkdir -p /tmp/ps_mongo_db
mongod --dbpath /tmp/ps_mongo_db --port 27099 --bind_ip 127.0.0.1 --quiet &
./pool teste/e2e/mongo.ps
pkill -f "port 27099"
```
