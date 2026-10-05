# `MongoCollection.find_one(query=Null)`

Busca o **primeiro** documento que bate com o filtro. Devolve um dict, ou
`Null` se nenhum bater.

```
col.find_one(query: dict = Null) -> dict | Null
```

---

## Uso

```
user = col.find_one({"email": "ana@x.com"})

if (user is Null) {
    post("não encontrado")
} else {
    post(user["nome"])
}
```

Ideal quando você espera **um** resultado (busca por um campo único, como
email ou id).

---

## Relacionados

- [`.find()`](../find/find.md) — todos os que batem (lista)
