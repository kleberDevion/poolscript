# `bytes.new(x=0)`

Cria uma sequência de bytes a partir de uma **lista de inteiros** (0-255), de um
**texto** (codificado em utf-8), de um **tamanho** (N bytes zerados) ou de uma
**cópia** de outros bytes.

```
bytes.new(x: list | str | int | bytes = 0) -> bytes
```

---

## Uso

```
import bytes

bytes.new([72, 105])        // b'Hi'   — cada inteiro vira um byte
bytes.new("Oi")             // b'Oi'   — texto em utf-8
bytes.new(3)                // b'\x00\x00\x00'  — 3 bytes zerados
bytes.new()                 // b''     — vazio
bytes.new(bytes.new("ok"))  // b'ok'   — cópia
```

---

## Erros

- **TypeError** — tipo que não dá pra virar bytes (ex: `flo`), ou `bool` como tamanho.
- **ValueError** — lista com item fora de 0-255, ou tamanho negativo.

```
bytes.new([300])   // erro: a lista precisa conter inteiros de 0 a 255
bytes.new(3.5)     // erro: não sei criar bytes de flo
```

---

## Relacionados

- [`bytes.fromhex()`](../fromhex/fromhex.md) — criar a partir de hex
- [`bytes.fromint()`](../fromint/fromint.md) — criar a partir de um inteiro
- [`bytes.tolist()`](../tolist/tolist.md) — o caminho de volta (bytes → lista)
- [visão geral do bytes](../bytes.md)
