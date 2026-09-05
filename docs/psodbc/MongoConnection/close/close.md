# `MongoConnection.close()`

Fecha a conexão com o MongoDB.

```
conn.close() -> None
```

---

## Uso

```
conn = psodbc.connect(driver="mongo", host="localhost", database="loja")
col = conn.collection("produtos")
# ... operações ...
conn.close()
```

---

## Relacionados

- [`MongoConnection`](../MongoConnection.md) — visão geral
