# `zip(a, b, ...)`

Junta duas ou mais listas **item a item**, formando pares (ou tuplas). Para na
menor lista.

```
zip(a, b, ...) -> list
```

---

## Uso

```
nomes = ["ana", "leo"]
idades = [30, 25]

for each par in zip(nomes, idades) {
    post(par[0], "tem", par[1], "anos")
}
// ana tem 30 anos
// leo tem 25 anos
```

Cada item do resultado é uma tupla com um elemento de cada lista, na mesma
posição.

---

## Relacionados

- [`enumerate()`](../enumerate/enumerate.md) — parear item com seu índice
