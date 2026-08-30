# psodbc — Banco de dados (SQLite, Postgres, MySQL, SQL Server, Mongo)

Lib pra conectar e consultar bancos de dados. Um único `connect()` cobre
**SQLite, PostgreSQL, MySQL, SQL Server e MongoDB** — muda só o `driver`.

```
import psodbc
// ou: import psodbc   (apelido)
```

---

## Referência

| Membro | O que faz | Página |
|---|---|---|
| `psodbc.connect(...)` | conecta a **qualquer** banco suportado | [connect/connect.md](connect/connect.md) |
| `psodbc.query(...)` | atalho **só pra SQLite** (arquivo local) | [query/query.md](query/query.md) |

### Objetos devolvidos

| Classe | De onde vem | Página |
|---|---|---|
| `DbConnection` | `connect()` em SQL (sqlite/postgres/mysql/mssql) | [DbConnection/DbConnection.md](DbConnection/DbConnection.md) |
| `DbCursor` | `DbConnection.cursor()` | [DbCursor/DbCursor.md](DbCursor/DbCursor.md) |
| `MongoConnection` | `connect(driver="mongo")` | [MongoConnection/MongoConnection.md](MongoConnection/MongoConnection.md) |
| `MongoCollection` | `MongoConnection.collection()` | [MongoCollection/MongoCollection.md](MongoCollection/MongoCollection.md) |

---

## Fluxo padrão (bancos SQL)

Sempre o mesmo, independente do driver: **conecta → cursor → executa → lê →
fecha**.

```
import psodbc

conn = psodbc.connect(driver="sqlite", base="loja.db")
cursor = conn.cursor()
cursor.execute("SELECT * FROM produtos WHERE preco > ?", (100,))
resultado = cursor.fetchall()      // lista de dicts
post(resultado)
conn.close()
```

O placeholder de parâmetros é `?` — **sempre use ele** pra valores dinâmicos
(protege contra SQL injection), nunca concatene na string SQL.

---

## Mongo é diferente

MongoDB não usa cursor/SQL — usa coleções e documentos:

```
conn = psodbc.connect(driver="mongo", host="localhost", port=27017, database="loja")
col = conn.collection("produtos")
col.insert({"nome": "café", "preco": 15})
achados = col.find({"preco": 15})
conn.close()
```

---

## Escolhendo o driver

`driver` aceita: `sqlite`, `postgres`/`pg`, `mysql`/`mariadb`,
`mssql`/`sqlserver`, `mongo`/`mongodb`. Cada um usa parâmetros diferentes —
ver [connect](connect/connect.md) pra a tabela completa.
