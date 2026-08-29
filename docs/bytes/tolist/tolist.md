# `bytes.tolist(b)`

Devolve uma **lista** com o valor inteiro (0-255) de cada byte. É o caminho de
volta do `bytes.new([...])`.

```
bytes.tolist(b: bytes) -> list
```

---

## Uso

```
import bytes

bytes.tolist(bytes.new("ABC"))            // [65, 66, 67]
bytes.tolist(bytes.fromhex("deadbeef"))   // [222, 173, 190, 239]
```

---

## Erros

- **TypeError** — o argumento não é bytes.

---

## Relacionados

- [`bytes.new()`](../new/new.md) — o caminho de volta (lista → bytes)
- [`bytes.get()`](../get/get.md) — um byte só, por índice
- [visão geral do bytes](../bytes.md)
