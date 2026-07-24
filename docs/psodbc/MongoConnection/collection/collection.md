# `MongoConnection.collection(nome)`

Devolve uma [`MongoCollection`](../../MongoCollection/MongoCollection.md) — o
equivalente Mongo de uma "tabela", onde os documentos ficam.

```
conn.collection(nome: str) -> MongoCollection
```

---

## Uso

```
conn = psodbc.connect(driver="mongo", host="localhost", database="loja")
produtos = conn.collection("produtos")
clientes = conn.collection("clientes")

produtos.insert({"nome": "café"})
```

Se a coleção não existir, o Mongo a cria no primeiro insert.

---

## Relacionados

- [`MongoCollection`](../../MongoCollection/MongoCollection.md) — os métodos de dados (find/insert/…)
- [`MongoConnection`](../MongoConnection.md) — visão geral
