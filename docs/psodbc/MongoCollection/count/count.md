# `MongoCollection.count(query=Null)`

Conta quantos documentos batem com o filtro. Devolve um int.

```assinatura
col.count(query: dict = Null) -> int
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
total = col.count()                    # todos
ativos = col.count({"ativo": True})    # só os que batem
post(ativos, "produtos ativos")
```

Mais leve que `len(col.find(...))` — o banco conta sem trazer os dados.

---

## Relacionados

- [`.find()`](../find/find.md) — trazer os documentos (quando precisa dos dados)
