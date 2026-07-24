# `psodbc.connect(...)`

Conecta a um banco de dados. O mesmo `connect` serve pra todos os bancos —
muda só o `driver` e os parâmetros que cada um usa.

```
connect(driver="sqlite", host="localhost", port=0, user="", password="",
        database="", base="", url="", odbc_driver="", trust_server_cert=true)
```

Devolve um [`DbConnection`](../DbConnection/DbConnection.md) (bancos SQL) ou um
[`MongoConnection`](../MongoConnection/MongoConnection.md) (mongo).

---

## Qual parâmetro cada driver usa

| Parâmetro | sqlite | postgres | mysql | mssql | mongo |
|---|---|---|---|---|---|
| `base` (arquivo) | **sim** | — | — | — | — |
| `host` | — | sim | sim | sim | sim |
| `port` (default) | — | 5432 | 3306 | 1433 | 27017 |
| `user`/`password` | — | sim | sim | sim¹ | sim |
| `database` | — | sim | sim | sim | sim |
| `url` | `sqlite:///x.db` | `postgres://…` | `mysql://…` | `sqlserver://…` | `mongodb://…` |

¹ SQL Server: `user` **e** `password` juntos = autenticação SQL; qualquer um
vazio = autenticação do Windows.

---

## Exemplos por banco

**SQLite** (arquivo local — use `base`):

```
conn = psodbc.connect(driver="sqlite", base="loja.db")
```

**PostgreSQL / MySQL** (host + credenciais):

```
conn = psodbc.connect(
    driver="postgres",
    host="localhost", port=5432,
    user="admin", password="senha", database="loja"
)
```

**SQL Server** (ver detalhes abaixo):

```
conn = psodbc.connect(
    driver="sqlserver",
    host="localhost", database="loja",
    user="sa", password="Senha@123"
)
```

**MongoDB**:

```
conn = psodbc.connect(driver="mongo", host="localhost", port=27017, database="loja")
```

**Por URL** (qualquer driver, escolhido pelo prefixo):

```
conn = psodbc.connect(url="postgres://admin:senha@localhost:5432/loja")
conn = psodbc.connect(url="sqlserver://sa:senha@10.0.0.5:1433/loja")
conn = psodbc.connect(url="mongodb://localhost:27017/loja")
```

---

## SQL Server em detalhe

- **Autenticação:** `user` **e** `password` preenchidos → login SQL. Qualquer um
  vazio → **autenticação do Windows** (`Trusted_Connection`), útil pra SQL
  Server local logado na sua conta.
- **Host com instância nomeada:** escreva `r"localhost\SQLEXPRESS"` (string
  **raw**) — numa string normal o `\S` some.
- **`odbc_driver`:** vazio auto-detecta o driver ODBC instalado (18 → 17 → …).
  Passe o nome exato só pra forçar um.
- **`trust_server_cert`** (padrão `true`): manda `TrustServerCertificate=yes` —
  necessário porque o ODBC Driver 18+ valida o certificado e derruba conexão
  local com certificado autoassinado. Use `false` só com certificado de CA real.
- **Automático:** a conexão abre com `autocommit=true` (senão `CREATE
  DATABASE`/`DROP DATABASE` são rejeitados).

---

## Depois de conectar

```
cursor = conn.cursor()                       // SQL
col = conn.collection("nome")                // Mongo
```

---

## Relacionados

- [`DbConnection`](../DbConnection/DbConnection.md) — o objeto SQL devolvido
- [`MongoConnection`](../MongoConnection/MongoConnection.md) — o objeto Mongo devolvido
- [`psodbc.query()`](../query/query.md) — atalho pra SQLite
