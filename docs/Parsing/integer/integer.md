# `Parsing.integer(valor)`

Converte um valor pra **inteiro**, de forma tolerante — extrai os dígitos e
trunca decimais (não arredonda).

```
Parsing.integer(valor) -> int
```

---

## Uso

```
Parsing.integer("12 unidades")   // 12    (extrai o número)
Parsing.integer("R$ 1.234")      // 1234
Parsing.integer(123.7)           // 123   (trunca, não arredonda)
Parsing.integer("abc")           // 0     (nada de número → 0)
```

---

## `Parsing.integer` vs `int`

- **`int("abc")`** — erro (texto não é número limpo).
- **`Parsing.integer("abc")`** — `0` (tolerante, não quebra).

Use `int()` quando quer o erro; `Parsing.integer` quando quer o melhor esforço
sem quebrar.

---

## Relacionados

- [`Parsing.floating()`](../floating/floating.md) — pra decimais
- builtin `int()` — conversão estrita
