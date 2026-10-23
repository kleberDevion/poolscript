# `DbConnection.cursor()`

Abre um [`DbCursor`](../../DbCursor/DbCursor.md) — o objeto por onde você executa
SQL e lê os resultados.

```assinatura
conn.cursor() -> DbCursor
```

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
# a tabela dos exemplos, com algumas linhas
psodbc.query(base="loja.db", cmd="CREATE TABLE produtos (id INTEGER PRIMARY KEY, nome TEXT, preco REAL)")
psodbc.query(base="loja.db", cmd="INSERT INTO produtos (nome, preco) VALUES ('caneta', 2.5), ('caderno', 15.0), ('mochila', 120.0)")
```

---

## Uso

```ps
conn = psodbc.connect(driver="sqlite", base="loja.db")
cursor = conn.cursor()
cursor.execute("SELECT * FROM produtos")
post(cursor.fetchall())
```

Você pode abrir mais de um cursor na mesma conexão se precisar.

---

## Relacionados

- [`DbCursor`](../../DbCursor/DbCursor.md) — o objeto devolvido (`execute`/`fetchall`/…)
- [`DbConnection`](../DbConnection.md) — visão geral
