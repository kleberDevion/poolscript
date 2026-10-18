# `MongoConnection.close()`

Fecha a conexão com o MongoDB.

```assinatura
conn.close() -> Null
```

---

## Uso

```ps
conn = psodbc.connect(driver="mongo", host="localhost", database="loja")
col = conn.collection("produtos")
# ... operações ...
conn.close()
```

---

## Relacionados

- [`MongoConnection`](../MongoConnection.md) — visão geral
