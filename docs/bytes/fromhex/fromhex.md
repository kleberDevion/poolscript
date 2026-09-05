# `bytes.fromhex(s)`

Cria bytes a partir de uma string **hexadecimal** (dois dígitos hex por byte).
Espaços em branco são ignorados, então dá pra escrever agrupado.

```
bytes.fromhex(s: str) -> bytes
```

---

## Uso

```
import bytes

bytes.fromhex("48656c6c6f")       # b'Hello'
bytes.fromhex("48 65 6c 6c 6f")   # b'Hello'  (espaços ignorados)
bytes.fromhex("deadbeef")         # b'\xde\xad\xbe\xef'
```

---

## Erros

- **ValueError** — hex inválido: dígito fora de `0-9a-f` ou quantidade ímpar de dígitos.

```
bytes.fromhex("zz")    # erro: hex inválido: 'zz'
bytes.fromhex("abc")   # erro: hex inválido (ímpar)
```

---

## Relacionados

- [`bytes.hex()`](../hex/hex.md) — o caminho de volta (bytes → hex)
- [`bytes.frombase64()`](../frombase64/frombase64.md) — o mesmo, mas em base64
- [visão geral do bytes](../bytes.md)
