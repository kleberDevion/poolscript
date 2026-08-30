# psodbc

Referência de API — o que é acessível via `import psodbc` num script `.ps` e
o que é interno/privado do módulo Python (não aparece do lado de fora).

---

## Acessível — `import psodbc`

```
import psodbc
// ou: import psodbc   (alias)
```

A tabela abaixo é a superfície INTEIRA de `psodbc.*`: o que não está aqui
**não existe** dentro de um script.

| Nome | Serve para | Retorno |
|---|---|---|
| `psodbc.connect` | **Qualquer** banco suportado (SQLite, Postgres, MySQL, SQL Server, Mongo) | `DbConnection` ou `MongoConnection` |
| `psodbc.query` | **SÓ SQLite** — atalho de arquivo `.db` local. Não funciona com SQL Server/Postgres/MySQL/Mongo (chama `sqlite3.connect` fixo) | `DbConnection`, `list[dict]` ou `None` |

`driver` aceita: `sqlite`, `postgres`/`postgresql`/`pg`, `mysql`/`mariadb`,
`mssql`/`sqlserver`, `mongo`/`mongodb`. `url`/o próprio `driver` também aceitam
uma string de conexão (`sqlserver://user:senha@host:porta/banco`, etc.).

---

## `connect()` — parâmetros por driver

Assinatura completa:

```
connect(driver="sqlite", host="localhost", port=0, user="", password="",
        database="", base="", url="", odbc_driver="", trust_server_cert=True)
```

Nem todo parâmetro vale pra todo driver — esta tabela diz qual usa o quê:

| Parâmetro | sqlite | postgres | mysql | **mssql/sqlserver** | mongo |
|---|---|---|---|---|---|
| `base` (caminho do arquivo) | **sim** (obrigatório) | — | — | — | — |
| `host` | — | sim | sim | **sim** (aceita instância: `r"host\INSTANCIA"`) | sim |
| `port` (default se 0) | — | 5432 | 3306 | **1433** | 27017 |
| `user` / `password` | — | sim | sim | **sim — os DOIS juntos, senão vira autenticação do Windows** | sim |
| `database` | — | sim | sim | **sim** (vazio = banco padrão do login) | sim |
| `odbc_driver` | — | — | — | **sim** (só dele) | — |
| `trust_server_cert` | — | — | — | **sim** (só dele) | — |
| `url` | `sqlite:///arq.db` | `postgres://...` | `mysql://...` | **`sqlserver://user:senha@host:1433/banco`** | `mongodb://...` |

### SQL Server em detalhe (`driver="mssql"` ou `"sqlserver"`)

**Autenticação — a regra exata do código:**
- `user` **e** `password` preenchidos → autenticação SQL (`UID`/`PWD` na
  connection string).
- Qualquer um dos dois vazio/omitido → **autenticação do Windows**
  (`Trusted_Connection=yes`) — útil pra SQL Server local logado na sua conta.

**`host` com instância nomeada — armadilha de escape:** em
`"localhost\SQLEXPRESS"` o `\S` não é escape conhecido e a PoolScript
**descarta a barra** silenciosamente (vira `localhostSQLEXPRESS`). Escreva
`r"localhost\SQLEXPRESS"` (string raw) ou `"localhost\\SQLEXPRESS"`.

**`port`:** omitido/0 deixa o driver resolver (1433). Informado, a connection
string vira `SERVER=host,porta` (vírgula — convenção do SQL Server, feito
automaticamente). Com instância nomeada normalmente NÃO se passa porta.

**`odbc_driver`:** vazio auto-detecta o primeiro instalado, nesta ordem:
`ODBC Driver 18 for SQL Server` → `17` → `13` → `SQL Server Native Client
11.0` → `FreeTDS` → `SQL Server`. Se nenhum existir, erro claro pedindo pra
instalar o 17/18. Só passe o nome exato se quiser forçar um específico.

**`trust_server_cert`** (default `true`): manda `TrustServerCertificate=yes` —
necessário porque o ODBC Driver 18+ valida o certificado por padrão e derruba
a conexão com certificado autoassinado (erro `08001` "cadeia de certificação...
não é de confiança"). Só use `false` se o servidor tiver certificado de CA
confiável de verdade.

**Automático (não é parâmetro):** a conexão abre com `autocommit=True` — sem
isso `CREATE DATABASE`/`DROP DATABASE` são rejeitados pelo SQL Server por
rodarem dentro de transação implícita. E o placeholder de parâmetros no
`execute()` é `?` (pyodbc): `cursor.execute("... WHERE id = ?", (1,))`.

**Exemplos completos:**

```
import psodbc

// 1. Autenticação SQL, servidor remoto, porta explícita
conn = psodbc.connect(
    driver="sqlserver",
    host="10.0.0.5",
    port=1433,
    user="sa",
    password="Senha@123",
    database="vendas"
)

// 2. Autenticação do Windows, instância local nomeada (repare o r"")
conn = psodbc.connect(driver="sqlserver", host=r"localhost\SQLEXPRESS", database="vendas")

// 3. Por URL — igual ao exemplo 1 (senha com @ vira %40 na URL)
conn = psodbc.connect(url="sqlserver://sa:Senha%40123@10.0.0.5:1433/vendas")

// 4. Forçando um driver ODBC específico
conn = psodbc.connect(driver="sqlserver", host="localhost", database="vendas",
                      odbc_driver="ODBC Driver 17 for SQL Server")

cursor = conn.cursor()
cursor.execute("SELECT * FROM clientes WHERE ativo = ?", (true,))
post(cursor.fetchall())
conn.close()
```

---

## Exemplos

**SQLite — modo convencional** (igual aos outros drivers: `connect()` +
`cursor()` + `execute()` + `fetchall()`):

```
import psodbc

conn = psodbc.connect(driver="sqlite", base="meu_banco.db")
cursor = conn.cursor()
cursor.execute("SELECT * FROM users")
result = cursor.fetchall()
post(result)
conn.close()
```

**SQLite — modo curto** (`query()` já devolve os dados prontos, sem passar por
`cursor()`):

```
resultado = psodbc.query(base="meu_banco.db", cmd="SELECT * FROM @t", table="users")
post(resultado)   // list[dict], ou None se não for SELECT
```

**Por parâmetros — Postgres/MySQL/SQL Server**:

```
conn = psodbc.connect(
    driver="postgres",
    host="localhost",
    port=5432,
    user="admin",
    password="senha",
    database="meu_banco"
)
cursor = conn.cursor()
cursor.execute("SELECT * FROM users WHERE ativo = ?", (true,))
post(cursor.fetchall())
conn.close()
```

**Por URL de conexão** — funciona com qualquer driver suportado, escolhido
pelo prefixo:

```
conn = psodbc.connect(url="postgres://admin:senha@localhost:5432/meu_banco")
conn = psodbc.connect(url="mysql://admin:senha@localhost:3306/meu_banco")
conn = psodbc.connect(url="sqlserver://user:senha@localhost:1433/meu_banco")
conn = psodbc.connect(url="mongodb://localhost:27017/meu_banco")
conn = psodbc.connect(url="sqlite:///meu_banco.db")
```

**SQL Server local com autenticação do Windows** (sem `user`/`password`):

```
conn = psodbc.connect(driver="sqlserver", host="localhost\\SQLEXPRESS", database="meu_banco")
```

**MongoDB** — API diferente (`.collection()` em vez de `cursor()`/`execute()`):

```
conn = psodbc.connect(driver="mongo", host="localhost", port=27017, database="meu_banco")
col = conn.collection("users")

achados = col.find({"nome": "ana"})
post(achados)

col.insert({"nome": "leo", "email": "leo@email.com"})
conn.close()
```

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

### Interno do motor (nem aparece em `psodbc.*`)

Fica em `vm/ps_db.c` e não é exposto ao script:

- o parser de URL de conexão (uso interno de `connect()`)
- a escolha do driver ODBC do SQL Server instalado
- os apelidos de driver (`pg`→`postgres`, `mariadb`→`mysql`, etc.)
- as classes `DbConnection`, `DbCursor`, `MongoConnection`, `MongoCollection` em si — só existem como retorno de `connect()`/`query()`, não dá pra importar/instanciar direto

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

Os clientes de banco entram **estáticos** no binário `pool` — não há pacote
pra instalar pra usar `sqlite`, `postgres` ou `mysql`.

| driver | precisa de algo na máquina? |
|---|---|
| `sqlite` | não — embutido |
| `postgres` | não — embutido |
| `mysql`/`mariadb` | não — embutido |
| `mssql`/`sqlserver` | o **driver ODBC do SQL Server** instalado no sistema (o gerenciador ODBC é embutido; o driver do fabricante não) |
| `mongo` | `libmongoc` no sistema |
