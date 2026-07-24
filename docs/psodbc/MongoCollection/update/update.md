# `MongoCollection.update(query, novos_valores)`

Atualiza os documentos que batem com o filtro, aplicando os novos valores.

```
col.update(query: dict, novos_valores: dict) -> None
```

| Parâmetro | O que é |
|---|---|
| `query` | filtro — quais documentos atualizar |
| `novos_valores` | os campos a mudar (e seus novos valores) |

---

## Uso

```
// muda o preço de todos os cafés
col.update({"nome": "café"}, {"preco": 18})

// marca como inativo quem tem estoque 0
col.update({"estoque": 0}, {"ativo": false})
```

---

## Relacionados

- [`.find()`](../find/find.md) — conferir o que vai ser afetado
- [`.remove()`](../remove/remove.md) — remover em vez de atualizar
