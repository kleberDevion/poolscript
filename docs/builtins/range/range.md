# `range(...)`

Gera uma **lista de números**. Muito usado pra repetir algo N vezes com
`for each`.

```
range(fim)              // 0 até fim-1
range(inicio, fim)      // inicio até fim-1
range(inicio, fim, passo)
```

---

## Uso

```
range(5)               // [0, 1, 2, 3, 4]
range(2, 6)            // [2, 3, 4, 5]
range(0, 10, 2)       // [0, 2, 4, 6, 8]

// repetir 3 vezes
for each i in range(3) {
    post("linha", i)
}
```

O `fim` é **exclusivo** — `range(5)` vai até `4`, não `5`.

---

## Relacionados

- [`len()`](../len/len.md) — tamanho de uma coleção
- [`enumerate()`](../enumerate/enumerate.md) — índice + item numa lista existente
