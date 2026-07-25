# `Parsing.floating(valor)`

Converte um valor pra **número decimal** (float), entendendo o separador
brasileiro (vírgula).

```
Parsing.floating(valor) -> flo
```

---

## Uso

```
Parsing.floating("R$ 19,90")     // 19.90   (entende vírgula BR)
Parsing.floating("3.14")         // 3.14
Parsing.floating(10)             // 10.0
Parsing.floating("abc")          // 0.0
```

---

## Vírgula BR vs ponto

Diferente do `flo()` builtin (que espera ponto), `Parsing.floating` entende
`"19,90"` como `19.90` — útil pra valores digitados no formato brasileiro.

---

## Relacionados

- [`Parsing.integer()`](../integer/integer.md) — pra inteiros
- builtin `flo()` — conversão estrita (espera ponto)
