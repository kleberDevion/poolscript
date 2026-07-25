# `enumerate(lista)`

Transforma uma lista em pares `(índice, item)` — útil pra iterar sabendo a
posição de cada elemento.

```
enumerate(lista) -> list
```

---

## Uso

```
nomes = ["ana", "leo", "bia"]

for each par in enumerate(nomes) {
    post(par[0], "-", par[1])
}
// 0 - ana
// 1 - leo
// 2 - bia
```

Cada `par` é `(índice, item)` — `par[0]` é a posição, `par[1]` é o valor.

---

## Relacionados

- [`zip()`](../zip/zip.md) — juntar duas listas item a item
- [`range()`](../range/range.md) — só os números
