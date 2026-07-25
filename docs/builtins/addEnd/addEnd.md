# `addEnd(lista, valor)`

Adiciona um valor ao **final** de uma lista. Modifica a lista original
(in-place).

```
addEnd(lista, valor) -> None
```

---

## Uso

```
frutas = ["maçã", "banana"]
addEnd(frutas, "uva")
post(frutas)            // ["maçã", "banana", "uva"]
```

Modifica a lista direto — não devolve uma nova.

---

## A família de edição de lista

| Função | Onde |
|---|---|
| `addEnd(l, v)` | adiciona no fim |
| [`removeEnd(l)`](../removeEnd/removeEnd.md) | remove do fim |
| [`addStart(l, v)`](../addStart/addStart.md) | adiciona no início |
| [`removeStart(l)`](../removeStart/removeStart.md) | remove do início |

---

## Relacionados

- [`removeEnd()`](../removeEnd/removeEnd.md) — o oposto
