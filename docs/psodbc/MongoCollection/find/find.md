# `MongoCollection.find(query=None)`

Busca **todos** os documentos que batem com o filtro. Devolve uma lista.

```
col.find(query: dict = None) -> list
```

---

## Uso

```
col.find({"ativo": true})      // todos com ativo = true
col.find({"categoria": "bebida"})
col.find()                     // todos (sem filtro)

for each doc in col.find({"preco": 15}) {
    post(doc["nome"])
}
```

---

## `find` vs `find_one`

- **`find`** — **lista** de todos que batem.
- **[`find_one`](../find_one/find_one.md)** — só o **primeiro** (um dict).

---

## Relacionados

- [`.find_one()`](../find_one/find_one.md) — um documento só
- [`.count()`](../count/count.md) — só contar, sem trazer os dados
