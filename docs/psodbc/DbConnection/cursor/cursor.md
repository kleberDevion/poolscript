# `DbConnection.cursor()`

Abre um [`DbCursor`](../../DbCursor/DbCursor.md) — o objeto por onde você executa
SQL e lê os resultados.

```assinatura
conn.cursor() -> DbCursor
```

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
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
