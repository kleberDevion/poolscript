# `DbCursor.rowcount()`

Devolve quantas linhas foram **afetadas** pelo último comando (INSERT, UPDATE,
DELETE).

```
cursor.rowcount() -> int
```

---

## Uso

```
cursor.execute("DELETE FROM produtos WHERE preco = ?", (0,))
conn.commit()
post(cursor.rowcount(), "produtos removidos")
```

Serve pra confirmar quantos registros um UPDATE/DELETE mexeu.

---

## Relacionados

- [`.execute()`](../execute/execute.md) — o comando que afeta as linhas
