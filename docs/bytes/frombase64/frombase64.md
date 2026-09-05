# `bytes.frombase64(s)`

Cria bytes a partir de uma string **base64**. É o caminho de volta do
[`bytes.base64()`](../base64/base64.md).

```
bytes.frombase64(s: str) -> bytes
```

---

## Uso

```
import bytes

bytes.frombase64("SGVsbG8=")   # b'Hello'
bytes.frombase64("3q2+7w==")   # b'\xde\xad\xbe\xef'
```

---

## Erros

- **ValueError** — a `str` não é base64 válido.

---

## Relacionados

- [`bytes.base64()`](../base64/base64.md) — o caminho de ida (bytes → base64)
- [`bytes.fromhex()`](../fromhex/fromhex.md) — o mesmo, mas em hex
- [visão geral do bytes](../bytes.md)
