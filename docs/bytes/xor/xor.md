# `bytes.xor(dados, chave)`

Aplica **XOR byte a byte** de `dados` com `chave`. Se a chave for menor que os
dados, ela **repete** (cifra XOR de chave repetida).

```
bytes.xor(dados: bytes, chave: bytes) -> bytes
```

Como o XOR é reversível, aplicar a **mesma chave** duas vezes volta ao original.

---

## Uso

```
import bytes

// chave repetida
bytes.xor(bytes.new("aaaa"), bytes.new("K"))   // b'****'  (0x61 ^ 0x4B = 0x2A)

// cifra reversível
segredo  = bytes.new("mensagem")
chave    = bytes.new("K3y")
cifrado  = bytes.xor(segredo, chave)
bytes.xor(cifrado, chave)     // b'mensagem'  — de volta ao original
```

---

## Erros

- **AttributedValueError** — `dados` ou `chave` não são bytes.
- **TypeError** — a chave está vazia.

---

## Relacionados

- [`bytes.new()`](../new/new.md) — criar a chave e os dados
- [visão geral do bytes](../bytes.md)
