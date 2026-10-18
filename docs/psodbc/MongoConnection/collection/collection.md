# `MongoConnection.collection(name)`

Devolve uma [`MongoCollection`](../../MongoCollection/MongoCollection.md) — o
equivalente Mongo de uma "tabela", onde os documentos ficam.

```assinatura
conn.collection(name: str) -> MongoCollection
```

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
```

---

## Uso

```ps sem-rodar: precisa do servidor MongoDB
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
