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
bytes.get(b, 0)     # 65
bytes.get(b, -1)    # 67   — o último
```

---

## Erros

- **TypeError** — `b` não é bytes, ou `i` não é inteiro.
- **IndexError** — o índice está fora da faixa: `index out of range` (sem
  nomear o tipo, diferente de list/tup/str). **Não** é `ValueError`.

```
bytes.get(bytes.new("ab"), 9)   # IndexError: index out of range
```

---

## Relacionados

- [`bytes.slice()`](../slice/slice.md) — um pedaço (como bytes)
- [`bytes.tolist()`](../tolist/tolist.md) — todos os bytes como lista de inteiros
- [visão geral do bytes](../bytes.md)
