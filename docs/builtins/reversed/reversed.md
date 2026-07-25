# `reversed(lista)`

Devolve uma **nova lista invertida** (a original não muda).

```
reversed(lista) -> list
```

---

## Uso

```
nums = [1, 2, 3]
post(reversed(nums))        // [3, 2, 1]
```

Inverte a **ordem** — não ordena. Pra ordenar, use [`sorted`](../sorted/sorted.md).

```
sorted([3, 1, 2])    // [1, 2, 3]   (ordena)
reversed([3, 1, 2])  // [2, 1, 3]   (só inverte)
```

---

## Relacionados

- [`sorted()`](../sorted/sorted.md) — ordenar
