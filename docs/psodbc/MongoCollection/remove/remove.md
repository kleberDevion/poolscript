# `MongoCollection.remove(query)`

Remove os documentos que batem com o filtro.

```
col.remove(query: dict) -> None
```

---

## Uso

```
col.remove({"ativo": false})      # remove todos os inativos
col.remove({"nome": "café"})      # remove os cafés
```

> **Cuidado:** `col.remove({})` (filtro vazio) removeria **todos** os
> documentos. Sempre passe um filtro específico, a não ser que seja mesmo essa
> a intenção.

---

## Relacionados

- [`.find()`](../find/find.md) — conferir o que vai ser removido antes
- [`.update()`](../update/update.md) — atualizar em vez de remover
