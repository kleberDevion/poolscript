# `sorted(lista)`

Devolve uma **nova lista ordenada** (a original não muda).

```
sorted(lista) -> list
```

---

## Uso

```
nums = [3, 1, 4, 1, 5]
post(sorted(nums))          // [1, 1, 3, 4, 5]

nomes = ["leo", "ana", "bia"]
post(sorted(nomes))         // ["ana", "bia", "leo"]  (ordem alfabética)
```

Números ordenam do menor pro maior; strings, em ordem alfabética.

---

## Relacionados

- [`reversed()`](../reversed/reversed.md) — inverter (não ordenar)
