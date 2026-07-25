# `map(lista, funcao)`

Aplica uma função em **cada item** de uma lista e devolve uma lista nova com os
resultados. A lista original não muda.

```
map(lista, funcao) -> list
```

> Repare na ordem: **lista primeiro, função depois** — `map(lista, fn)`.

---

## Uso

```
nums = [1, 2, 3, 4]

dobrados = map(nums, action(n) { return n * 2 })
post(dobrados)          // [2, 4, 6, 8]

nomes = ["ana", "leo"]
maiusculos = map(nomes, action(n) { return n.upper() })
```

A função recebe um item por vez e devolve o valor transformado.

---

## `map` vs `filter`

- **`map`** — **transforma** cada item (mesma quantidade de itens).
- **[`filter`](../filter/filter.md)** — **seleciona** itens (menos itens).

---

## Relacionados

- [`filter()`](../filter/filter.md) — selecionar em vez de transformar
- Lambdas (`action(x) { ... }`) — ver `LANGUAGE.md`
