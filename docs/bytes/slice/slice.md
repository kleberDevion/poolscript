# `bytes.slice(b, ini=0, fim=None)`

Recorta uma **fatia** dos bytes, de `ini` (inclusive) até `fim` (exclusive) —
mesma semântica de fatia do Python. Índices negativos contam a partir do fim.

```
bytes.slice(b: bytes, ini: int = 0, fim: int = None) -> bytes
```

---

## Uso

```
import bytes

b = bytes.new("Hello")
bytes.slice(b, 1, 3)    # b'el'
bytes.slice(b, 2)       # b'llo'   — do índice 2 até o fim
bytes.slice(b, -2)      # b'lo'    — os 2 últimos
bytes.slice(b, 0, -1)   # b'Hell'  — tudo menos o último
```

Índices fora do range são recortados (não dão erro) — igual às fatias do Python.

---

## Erros

- **TypeError** — `b` não é bytes, ou `ini`/`fim` não são inteiros nem `Null`.

`Null` (e `None`, que é apelido dele) vale **omitido** nos dois: `slice(b, 1,
Null)` é o mesmo que `slice(b, 1)`, e `slice(b, Null, 3)` o mesmo que
`slice(b, 0, 3)`.

---

## Relacionados

- [`bytes.get()`](../get/get.md) — um byte só (como inteiro)
- [`bytes.concat()`](../concat/concat.md) — a operação inversa (juntar)
- [visão geral do bytes](../bytes.md)
