# `MongoCollection.count(query=Null)`

Conta quantos documentos batem com o filtro. Devolve um int.

```
col.count(query: dict = Null) -> int
```

---

## Uso

```
total = col.count()                    # todos
ativos = col.count({"ativo": True})    # só os que batem
post(ativos, "produtos ativos")
```

Mais leve que `len(col.find(...))` — o banco conta sem trazer os dados.

---

## Relacionados

- [`.find()`](../find/find.md) — trazer os documentos (quando precisa dos dados)
