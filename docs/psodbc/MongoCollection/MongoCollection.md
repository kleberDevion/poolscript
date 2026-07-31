# `MongoCollection` — documentos do MongoDB

O objeto que [`MongoConnection.collection(nome)`](../MongoConnection/collection/collection.md)
devolve. Aqui você busca, insere, atualiza e remove **documentos** (dicts).

---

## Métodos

| Método | O que faz | Página |
|---|---|---|
| `.find(query)` | busca **todos** os documentos que batem | [find/find.md](find/find.md) |
| `.find_one(query)` | busca **um** documento | [find_one/find_one.md](find_one/find_one.md) |
| `.insert(doc)` | insere **um** documento | [insert/insert.md](insert/insert.md) |
| `.insert_many(docs)` | insere **vários** | [insert_many/insert_many.md](insert_many/insert_many.md) |
| `.update(query, novo)` | atualiza os que batem | [update/update.md](update/update.md) |
| `.remove(query)` | remove os que batem | [remove/remove.md](remove/remove.md) |
| `.count(query)` | conta quantos batem | [count/count.md](count/count.md) |

---

## A "query" é um dict de filtro

Em quase todos os métodos, você descreve o que procurar com um dict:

```
col.find({"ativo": true})               // todos com ativo = true
col.find({"idade": 30})                 // todos com idade = 30
col.find({})                            // todos (filtro vazio)
```

---

## Exemplo completo

```
import psodbc

conn = psodbc.connect(driver="mongo", host="localhost", database="loja")
col = conn.collection("produtos")

col.insert({"nome": "café", "preco": 15, "ativo": true})
col.insert_many([{"nome": "chá"}, {"nome": "suco"}])

ativos = col.find({"ativo": true})
post(col.count())                       // total de documentos

col.update({"nome": "café"}, {"preco": 18})
col.remove({"ativo": false})

conn.close()
```

---

## Relacionados

- [`MongoConnection`](../MongoConnection/MongoConnection.md) — quem cria a coleção
- [`.find()`](find/find.md) — o ponto de partida
