# `MongoCollection.insert_many(documents)`

Insere **vários** documentos de uma vez, a partir de uma lista.

```assinatura
col.insert_many(documents: list) -> Null
```

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
MongoConnection conn = psodbc.connect(driver="mongo", host="localhost", database="loja")
MongoCollection col = conn.collection("produtos")
```

---

## Uso

```ps sem-rodar: precisa do servidor MongoDB
col.insert_many([
    {"nome": "café"},
    {"nome": "chá"},
    {"nome": "suco"}
])
```

Mais eficiente que chamar [`insert`](../insert/insert.md) várias vezes quando
você já tem a lista pronta.

---

## Relacionados

- [`.insert()`](../insert/insert.md) — um documento só
