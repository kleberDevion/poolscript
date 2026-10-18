# `MongoCollection.insert(document)`

Insere **um** documento (dict) na coleção.

```assinatura
col.insert(document: dict) -> Null
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
col.insert({"nome": "café", "preco": 15, "ativo": True})
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
