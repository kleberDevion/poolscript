# `MongoCollection.insert_many(documents)`

Insere **vários** documentos de uma vez, a partir de uma lista.

```
col.insert_many(documents: list) -> None
```

---

## Uso

```
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
