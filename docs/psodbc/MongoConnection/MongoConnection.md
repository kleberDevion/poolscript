# `MongoConnection` — conexão MongoDB

O objeto que [`connect(driver="mongo")`](../connect/connect.md) devolve.
MongoDB não usa SQL nem cursor — você trabalha com **coleções** e
**documentos** (dicts).

---

## Métodos

| Método | O que faz | Página |
|---|---|---|
| `.collection(nome)` | pega uma [`MongoCollection`](../MongoCollection/MongoCollection.md) | [collection/collection.md](collection/collection.md) |
| `.close()` | fecha a conexão | [close/close.md](close/close.md) |

---

## Fluxo

```
import psodbc

conn = psodbc.connect(driver="mongo", host="localhost", port=27017, database="loja")
col = conn.collection("produtos")      // escolhe a coleção

col.insert({"nome": "café", "preco": 15})
achados = col.find({"preco": 15})
post(achados)

conn.close()
```

---

## SQL vs Mongo

| | SQL (`DbConnection`) | Mongo (`MongoConnection`) |
|---|---|---|
| unidade | tabela + linhas | coleção + documentos |
| executar | `.cursor().execute("SQL")` | `.collection("x").find({...})` |
| dados | dicts com colunas fixas | dicts livres (documentos) |

---

## Relacionados

- [`MongoCollection`](../MongoCollection/MongoCollection.md) — onde os dados vivem
- [`connect()`](../connect/connect.md) — conectar com `driver="mongo"`
