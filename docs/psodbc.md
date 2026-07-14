# psodbc

Referência de API — o que é acessível via `import psodbc` num script `.ps` e
o que é interno/privado do módulo Python (não aparece do lado de fora).

---

## Acessível — `import psodbc`

O interpretador só expõe o que está no dict `EXPORTS` de `psodbc_lib.py`.
Qualquer outra coisa do arquivo **não existe** em `psodbc.*` dentro de um
script.

| Nome | Assinatura | Retorno |
|---|---|---|
| `psodbc.connect` | `connect(driver="sqlite", host="localhost", port=0, user="", password="", database="", base="", url="", odbc_driver="", trust_server_cert=True)` | `DbConnection` ou `MongoConnection` |
| `psodbc.query` | `query(base="", cmd=None, table="")` | `DbConnection`, `list[dict]` ou `None` |

`driver` aceita: `sqlite`, `postgres`/`postgresql`/`pg`, `mysql`/`mariadb`,
`mssql`/`sqlserver`, `mongo`/`mongodb`. `url`/o próprio `driver` também aceitam
uma string de conexão (`sqlserver://user:senha@host:porta/banco`, etc.).

`trust_server_cert` (só `mssql`): manda `TrustServerCertificate=yes` pro
driver — necessário porque o ODBC Driver 18+ passou a validar o certificado
por padrão e derruba a conexão com certificado autoassinado (erro `08001`
"cadeia de certificação... não é de confiança"). Já vem `True`; só usa `False`
se o servidor tiver certificado de CA confiável de verdade.

Conexão `mssql` já abre com `autocommit=True` — sem isso comandos como
`CREATE DATABASE`/`DROP DATABASE` são rejeitados pelo SQL Server por rodarem
dentro de uma transação implícita.

---

## Acessível — nos objetos retornados

Não são módulos, são instâncias comuns — todo método público (sem `_` na
frente) responde normalmente por `.`.

### `DbConnection` — sqlite, postgres, mysql, mssql

| Método | Retorno |
|---|---|
| `.cursor()` | `DbCursor` |
| `.commit()` | — |
| `.close()` | — |

### `DbCursor`

| Método | Retorno |
|---|---|
| `.execute(sql, params=())` | `DbCursor` (encadeável) |
| `.fetchall()` | `list[dict]` |
| `.fetchone()` | `dict` ou `None` |
| `.fetchmany(size=1)` | `list[dict]` |
| `.rowcount` | `int` |
| `.close()` | — |

### `MongoConnection`

| Método | Retorno |
|---|---|
| `.collection(name)` | `MongoCollection` |
| `.close()` | — |

### `MongoCollection`

| Método | Retorno |
|---|---|
| `.find(query=None)` | `list` ou `None` |
| `.find_one(query=None)` | `dict` ou `None` |
| `.insert(document)` | `None` |
| `.insert_many(documents)` | `None` |
| `.update(query, new_values)` | `None` |
| `.remove(query)` | `None` |
| `.count(query=None)` | `int` |

---

## Não acessível

### Bloqueado pelo interpretador (nem aparece em `psodbc.*`)

Tudo em `psodbc_lib.py` que não está em `EXPORTS`:

- `_parse_dsn(dsn)` — parser de URL de conexão, uso interno de `connect()`
- `_pick_mssql_odbc_driver(preferred)` — escolhe o driver ODBC do SQL Server instalado
- `_DRIVER_ALIASES` — dict de apelidos de driver (`pg`→`postgres`, `mariadb`→`mysql`, etc.)
- As classes `DbConnection`, `DbCursor`, `MongoConnection`, `MongoCollection` em si — só existem como retorno de `connect()`/`query()`, não dá pra importar/instanciar direto

### Privado por convenção (tecnicamente alcançável, não é API)

Atributos com `_` na frente nos objetos retornados — guardam a conexão/cursor
real da lib de banco por trás (`sqlite3`, `psycopg2`, `mysql.connector`,
`pyodbc`, `pymongo`). Não fazem parte da API e podem mudar sem aviso:

- `DbConnection._conn`
- `DbCursor._cursor`, `DbCursor._db_type`
- `MongoConnection._client`, `MongoConnection._db`
- `MongoCollection._col`

---

## Requisitos por driver

| driver | pacote Python necessário |
|---|---|
| `sqlite` | nenhum (stdlib) |
| `postgres` | `psycopg2-binary` |
| `mysql`/`mariadb` | `mysql-connector-python` |
| `mssql`/`sqlserver` | `pyodbc` + driver ODBC do SQL Server instalado no sistema |
| `mongo` | `pymongo` |
