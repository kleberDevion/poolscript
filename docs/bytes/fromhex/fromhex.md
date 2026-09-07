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

- **ValueError** — dígito que não é hexadecimal, ou quantidade ímpar. A
  mensagem é a mesma nos dois casos, e aponta a **posição**:

```
bytes.fromhex("zz")    # ValueError: non-hexadecimal number found in fromhex() arg at position 0
bytes.fromhex("abc")   # ValueError: non-hexadecimal number found in fromhex() arg at position 3
```

**Maiúsculas passam**: `bytes.fromhex("DEADBEEF")` é `b'\xde\xad\xbe\xef'`. A
faixa aceita é `0-9a-fA-F`, não só a minúscula.

---

## Relacionados

- [`bytes.hex()`](../hex/hex.md) — o caminho de volta (bytes → hex)
- [`bytes.frombase64()`](../frombase64/frombase64.md) — o mesmo, mas em base64
- [visão geral do bytes](../bytes.md)
