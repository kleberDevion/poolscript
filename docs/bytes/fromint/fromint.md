# `bytes.fromint(n, length=0, byteorder="big")`

Empacota um inteiro (>= 0) em bytes. Útil pra montar formatos binários com
campos de largura fixa (cabeçalhos, protocolos).

```
bytes.fromint(n: int, length: int = 0, byteorder: str = "big") -> bytes
```

- `length` — largura fixa em bytes (zero-pad à esquerda em big-endian). Omitido (`0`), usa o mínimo necessário.
- `byteorder` — `"big"` (padrão, byte mais significativo primeiro) ou `"little"`.

---

## Uso

```
import bytes

bytes.fromint(258)             // b'\x01\x02'  — mínimo (2 bytes)
bytes.fromint(258, 4)          // b'\x00\x00\x01\x02'  — largura fixa 4
bytes.fromint(258, 4, "little") // b'\x02\x01\x00\x00'  — little-endian
```

---

## Erros

- **AtributtedValueError** — `n` não é inteiro, ou `length` não é inteiro.
- **SomeValueUnexpected** — `n` negativo, `byteorder` inválido, ou `n` não cabe em `length` bytes.

```
bytes.fromint(70000, 1)   // erro: 70000 não cabe em 1 byte(s)
```

---

## Relacionados

- [`bytes.toint()`](../toint/toint.md) — o caminho de volta (bytes → inteiro)
- [visão geral do bytes](../bytes.md)
