# `bytes.hex(b)`

Devolve os bytes como uma string **hexadecimal** minúscula, sem separador.

```
bytes.hex(b: bytes) -> str
```

---

## Uso

```
import bytes

bytes.hex(bytes.new("Hello"))   // "48656c6c6f"
bytes.hex(bytes.new([222, 173, 190, 239]))   // "deadbeef"
```

---

## Erros

- **TypeError** — o argumento não é bytes.

---

## Relacionados

- [`bytes.fromhex()`](../fromhex/fromhex.md) — o caminho de volta (hex → bytes)
- [`bytes.base64()`](../base64/base64.md) — a mesma ideia, em base64
- [visão geral do bytes](../bytes.md)
