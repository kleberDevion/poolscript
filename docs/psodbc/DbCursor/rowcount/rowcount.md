# `DbCursor.rowcount`

Quantas linhas foram **afetadas** pelo último comando (INSERT, UPDATE,
DELETE). É uma **propriedade** — lê-se **sem parênteses**; `cursor.rowcount()`
dá erro (`tentativa de chamar algo que não é função`).

```assinatura
cursor.rowcount -> int
```

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
DbConnection conn = psodbc.connect(driver="sqlite", base="loja.db")
DbCursor cursor = conn.cursor()
```

---

## Uso

```ps
cursor.execute("DELETE FROM produtos WHERE preco = ?", (0,))
conn.commit()
post(cursor.rowcount, "produtos removidos")
```

Serve pra confirmar quantos registros um UPDATE/DELETE mexeu.

---

## Relacionados

- [`.execute()`](../execute/execute.md) — o comando que afeta as linhas
