# `MongoCollection.insert(documento)`

Insere **um** documento (dict) na coleção.

```
col.insert(documento: dict) -> None
```

---

## Uso

```
col.insert({"nome": "café", "preco": 15, "ativo": true})
```

O documento é um dict livre — o Mongo não exige colunas fixas, cada documento
pode ter campos diferentes.

---

## `insert` vs `insert_many`

- **`insert`** — um documento.
- **[`insert_many`](../insert_many/insert_many.md)** — uma lista de vários de uma vez.

---

## Relacionados

- [`.insert_many()`](../insert_many/insert_many.md) — vários de uma vez
- [`.find()`](../find/find.md) — buscar depois
