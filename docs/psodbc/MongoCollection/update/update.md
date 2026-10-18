# `MongoCollection.update(query, new_values)`

Atualiza os documentos que batem com o filtro, aplicando os novos valores.

```assinatura
col.update(query: dict, new_values: dict) -> Null
```

| Parâmetro | O que é |
|---|---|
| `query` | filtro — quais documentos atualizar |
| `new_values` | os campos a mudar (e seus novos valores) |

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
MongoConnection conn = psodbc.connect(driver="mongo", host="localhost", database="loja")
MongoCollection col = conn.collection("produtos")
```

---

## Uso

```ps sem-rodar: precisa do servidor MongoDB
# muda o preço de todos os cafés
col.update({"nome": "café"}, {"preco": 18})

# marca como inativo quem tem estoque 0
col.update({"estoque": 0}, {"ativo": False})
```

---

## Relacionados

- [`.find()`](../find/find.md) — conferir o que vai ser afetado
- [`.remove()`](../remove/remove.md) — remover em vez de atualizar
