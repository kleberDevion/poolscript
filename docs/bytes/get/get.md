# `bytes.get(b, i)`

Devolve o **valor inteiro** (0-255) do byte na posição `i`. Índice negativo
conta a partir do fim.

```
bytes.get(b: bytes, i: int) -> int
```

Diferente de `bytes.slice(b, i, i+1)` (que devolve `bytes`), `get` devolve o
**número** daquele byte.

---

## Uso

```
import bytes

b = bytes.new("ABC")
bytes.get(b, 0)     // 65
bytes.get(b, -1)    // 67   — o último
```

---

## Erros

- **AttributedValueError** — `b` não é bytes, ou `i` não é inteiro.
- **SomeValueUnexpected** — índice fora do range.

```
bytes.get(bytes.new("ab"), 9)   // erro: índice 9 fora do range (0..1)
```

---

## Relacionados

- [`bytes.slice()`](../slice/slice.md) — um pedaço (como bytes)
- [`bytes.tolist()`](../tolist/tolist.md) — todos os bytes como lista de inteiros
- [visão geral do bytes](../bytes.md)
