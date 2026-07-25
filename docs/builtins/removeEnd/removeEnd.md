# `removeEnd(lista)`

Remove o **último** item da lista e o devolve. Modifica a lista original.

```
removeEnd(lista) -> valor | Null
```

---

## Uso

```
pilha = [1, 2, 3]
ultimo = removeEnd(pilha)
post(ultimo)            // 3
post(pilha)             // [1, 2]
```

Em lista **vazia**, devolve `Null` (não quebra).

---

## Relacionados

- [`addEnd()`](../addEnd/addEnd.md) — o oposto (adicionar no fim)
- [`removeStart()`](../removeStart/removeStart.md) — remover do início
