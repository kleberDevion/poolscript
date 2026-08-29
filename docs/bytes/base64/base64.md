# `bytes.base64(b)`

Devolve os bytes como uma string **base64** (padrão, com padding `=`). Base64 é
o jeito comum de embutir binário em texto — JSON, e-mail, data URLs, etc.

```
bytes.base64(b: bytes) -> str
```

---

## Uso

```
import bytes

bytes.base64(bytes.new("Hello"))   // "SGVsbG8="
bytes.base64(bytes.fromhex("deadbeef"))   // "3q2+7w=="
```

---

## Erros

- **AttributedValueError** — o argumento não é bytes.

---

## Relacionados

- [`bytes.frombase64()`](../frombase64/frombase64.md) — o caminho de volta (base64 → bytes)
- [`bytes.hex()`](../hex/hex.md) — a mesma ideia, em hex
- [visão geral do bytes](../bytes.md)
