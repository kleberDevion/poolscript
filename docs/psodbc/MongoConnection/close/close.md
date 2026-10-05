# `MongoConnection.close()`

Fecha a conexão com o MongoDB.

```
conn.close() -> Null
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
