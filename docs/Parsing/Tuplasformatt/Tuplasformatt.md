# `Parsing.Tuplasformatt(value, to_type="tup")`

Converte um valor pra **tupla** (lista imutável).

```assinatura
Parsing.Tuplasformatt(value, to_type="tup") -> tup
```

`to_type` existe por uniformidade com as outras funções do `Parsing` — aqui
**não muda nada**: o resultado é sempre `tup`.

---

## Uso

```ps
import Parsing

Parsing.Tuplasformatt([1, 2, 3])     # (1, 2, 3)   (lista → tupla)
Parsing.Tuplasformatt("abc")         # ("a", "b", "c")
Parsing.Tuplasformatt(42)            # (42,)
```

---

## Relacionados

- [`Parsing.Arrayformatt()`](../Arrayformatt/Arrayformatt.md) — pra lista (mutável)
