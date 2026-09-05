# `MongoCollection.count(query=None)`

Conta quantos documentos batem com o filtro. Devolve um int.

```
col.count(query: dict = None) -> int
```

---

## Uso

```
total = col.count()                    # todos
ativos = col.count({"ativo": true})    # só os que batem
post(ativos, "produtos ativos")
```

Mais leve que `len(col.find(...))` — o banco conta sem trazer os dados.

---

## Relacionados

- [`.find()`](../find/find.md) — trazer os documentos (quando precisa dos dados)
