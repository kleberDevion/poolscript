# `DbConnection` — conexão SQL

O objeto que [`connect()`](../connect/connect.md) devolve para bancos SQL
(SQLite, Postgres, MySQL, SQL Server). Por ele você abre cursores, confirma
transações e fecha a conexão.

---

## Métodos

| Método | O que faz | Página |
|---|---|---|
| `.cursor()` | abre um [`DbCursor`](../DbCursor/DbCursor.md) pra executar SQL | [cursor/cursor.md](cursor/cursor.md) |
| `.commit()` | confirma (salva) as alterações | [commit/commit.md](commit/commit.md) |
| `.close()` | fecha a conexão | [close/close.md](close/close.md) |

---

## Fluxo padrão

```
import psodbc

conn = psodbc.connect(driver="sqlite", base="loja.db")
cursor = conn.cursor()                     # abre cursor
cursor.execute("SELECT * FROM produtos")   # executa
dados = cursor.fetchall()                  # lê
conn.close()                               # fecha
```

Pra inserir/atualizar/apagar, chame `.commit()` depois do `execute` (senão as
mudanças não são salvas):

```
cursor.execute("INSERT INTO produtos (nome) VALUES (?)", ("café",))
conn.commit()                              # salva a inserção
```

---

## Relacionados

- [`DbCursor`](../DbCursor/DbCursor.md) — executar e ler
- [`connect()`](../connect/connect.md) — o que devolve um `DbConnection`
