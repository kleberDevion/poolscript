# `bytes.toint(b, byteorder="big")`

Desempacota bytes num inteiro (>= 0). É o caminho de volta do
[`bytes.fromint()`](../fromint/fromint.md).

```
bytes.toint(b: bytes, byteorder: str = "big") -> int
```

- `byteorder` — `"big"` (padrão) ou `"little"`. Precisa ser o **mesmo** usado pra empacotar.

---

## Uso

```
import bytes

bytes.toint(bytes.fromint(258, 4))          // 258
bytes.toint(bytes.fromhex("deadbeef"))      // 3735928559
bytes.toint(bytes.fromint(258, 4, "little"), "little")   // 258
```

---

## Erros

- **AttributedValueError** — o argumento não é bytes.
- **TypeError** — `byteorder` inválido.

> Nota: o inteiro da PoolScript é de 64 bits — use pra sequências de até 8 bytes.

---

## Relacionados

- [`bytes.fromint()`](../fromint/fromint.md) — o caminho de ida (inteiro → bytes)
- [visão geral do bytes](../bytes.md)
