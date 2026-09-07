# `bytes.concat(lista)`

Junta uma **lista de bytes** num só, na ordem. Útil pra montar um buffer a
partir de pedaços (cabeçalho + corpo, por exemplo).

```
bytes.concat(lista: list) -> bytes
```

---

## Uso

```
import bytes

bytes.concat([bytes.new("Hi"), bytes.new("!!")])   # b'Hi!!'

cab  = bytes.fromint(2, 2)          # b'\x00\x02'
corpo = bytes.new("oi")
bytes.concat([cab, corpo])          # b'\x00\x02oi'
```

---

## Erros

- **TypeError** — o argumento não é lista, ou algum item não é bytes.

```
bytes.concat([bytes.new("a"), 5])   # TypeError: sequence item 1: expected a bytes-like object, int found
bytes.concat("abc")                 # TypeError: can only join an iterable
```

---

## Relacionados

- [`bytes.slice()`](../slice/slice.md) — a operação inversa (recortar um pedaço)
- [visão geral do bytes](../bytes.md)
