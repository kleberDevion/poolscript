# `psodbc.query(base="", cmd=Null, table="")`

Atalho **só pra SQLite** (arquivo local). Faz a consulta e já devolve os dados
prontos, sem você abrir cursor manualmente.

```assinatura
psodbc.query(base: str, cmd=Null, table: str = "") -> list | Null | DbConnection
```

| Parâmetro | O que é |
|---|---|
| `base` | caminho do arquivo `.db` |
| `cmd` | o SQL (string, ou tupla `(sql, params)`) |
| `table` | nome que substitui `@t` no SQL (ver abaixo) |

> **Só SQLite.** Pra Postgres/MySQL/SQL Server/Mongo, use
> [`connect()`](../connect/connect.md) — `query` chama `sqlite3` fixo.

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
# as tabelas dos exemplos, com algumas linhas
psodbc.query(base="loja.db", cmd="CREATE TABLE produtos (id INTEGER PRIMARY KEY, nome TEXT, preco REAL)")
psodbc.query(base="loja.db", cmd="INSERT INTO produtos (nome, preco) VALUES ('caneta', 2.5), ('caderno', 15.0), ('mochila', 120.0)")
psodbc.query(base="loja.db", cmd="CREATE TABLE clientes (id INTEGER PRIMARY KEY, nome TEXT, ativo INT)")
psodbc.query(base="loja.db", cmd="INSERT INTO clientes (nome, ativo) VALUES ('Loja A', 1), ('Loja B', 0)")
```

---

## Modo curto — já devolve os dados

```ps
import psodbc

# SELECT → devolve list[dict] direto
resultado = psodbc.query(
    base="loja.db",
    cmd="SELECT * FROM produtos"
)
post(resultado)      # [{"id": 1, "nome": "café"}, ...]
```

Se o comando não for `SELECT` (INSERT/UPDATE/DELETE), executa e devolve `Null`.

O `cmd` vai inteiro, do tamanho que for. Duas coisas não rodam, e nada do SQL
é executado, como no [`execute`](../DbCursor/execute/execute.md):

- texto com o caractere NUL (`\x00`), no `cmd` ou no `table`:
  `DatabaseError: the query contains a null character`;
- mais de um comando: `DatabaseError: You can only execute one statement at a
  time.` O `;` no fim e um comentário depois dele valem.

```ps
import psodbc
try {
    psodbc.query(base="loja.db", cmd="DELETE FROM produtos WHERE id = 1; DELETE FROM produtos")
} catch (DatabaseError e) {
    post(e)
}
post(psodbc.query(base="loja.db", cmd="SELECT COUNT(*) AS n FROM produtos"))
```

```saida
You can only execute one statement at a time. (linha 3)
[{'n': 3}]
```

---

## Com parâmetros (tupla)

Pra valores dinâmicos, passe uma **tupla** `(sql, params)` com `?` no lugar —
nunca concatene na string:

```ps
resultado = psodbc.query(
    base="loja.db",
    cmd=("SELECT * FROM produtos WHERE preco > ?", (100,))
)
```

---

## `@t` — nome da tabela dinâmico

O `table` substitui `@t` no SQL (útil quando a tabela varia):

```ps
psodbc.query(
    base="loja.db",
    cmd=("SELECT * FROM @t WHERE ativo = ?", (True,)),
    table="clientes"
)
# vira: SELECT * FROM clientes WHERE ativo = ?
```

---

## `query` vs `connect`

- **`query`** — atalho SQLite pra uma consulta rápida.
- **[`connect`](../connect/connect.md)** — qualquer banco, e quando você faz
  **várias** operações na mesma conexão (mais eficiente que abrir a cada query).

---

## Relacionados

- [`connect()`](../connect/connect.md) — conexão completa, qualquer banco
- [`DbCursor.execute()`](../DbCursor/execute/execute.md) — o modo convencional
