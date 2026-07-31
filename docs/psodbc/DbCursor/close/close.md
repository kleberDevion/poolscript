# `DbCursor.close()`

Fecha o cursor. Chame ao terminar de usá-lo (a conexão continua aberta até você
fechar com [`DbConnection.close()`](../../DbConnection/close/close.md)).

```
cursor.close() -> None
```

---

## Uso

```
cursor = conn.cursor()
cursor.execute("SELECT * FROM produtos")
dados = cursor.fetchall()
cursor.close()         // fecha o cursor
conn.close()           // fecha a conexão
```

---

## Relacionados

- [`DbConnection.close()`](../../DbConnection/close/close.md) — fecha a conexão inteira
