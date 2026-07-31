# `Parsing.string(valor, to_type="str")`

Limpa um valor e devolve texto, removendo caracteres estranhos e normalizando
espaços.

```
Parsing.string(valor, to_type="str")
```

---

## Uso

```
Parsing.string("  texto   com   espaços  ")   // "texto com espaços"
Parsing.string(42)                            // "42"
```

Com `to_type`, pode já converter pra número no fim (`"int"`, `"flo"`).

---

## Relacionados

- [`Parsing.integer()`](../integer/integer.md) · [`Parsing.floating()`](../floating/floating.md)
- [visão geral do Parsing](../Parsing.md)
