# `filter(lista, funcao)`

Devolve uma lista nova só com os itens que **passam** no teste — a função
devolve `true`/`false` pra cada item.

```
filter(lista, funcao) -> list
```

> Ordem: **lista primeiro, função depois** — `filter(lista, fn)`.

---

## Uso

```
nums = [1, 2, 3, 4, 5, 6]

pares = filter(nums, action(n) { return n % 2 == 0 })
post(pares)             // [2, 4, 6]

maiores = filter(nums, action(n) { return n > 3 })
post(maiores)           // [4, 5, 6]
```

A função devolve `true` pra manter o item, `false` pra descartar.

---

## `filter` vs `map`

- **`filter`** — **seleciona** (o resultado tem menos ou igual itens).
- **[`map`](../map/map.md)** — **transforma** (mesma quantidade).

Dá pra combinar: filtrar e depois transformar.

---

## Relacionados

- [`map()`](../map/map.md) — transformar cada item
- Lambdas — ver `LANGUAGE.md`
