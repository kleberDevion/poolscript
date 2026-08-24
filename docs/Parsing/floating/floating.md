# `Parsing.floating(value, to_type="flo")`

Converte um valor pra **número decimal** (float), entendendo o separador
brasileiro (vírgula).

```
Parsing.floating(value, to_type="flo") -> flo
```

`to_type` existe por uniformidade com as outras funções do `Parsing` — aqui
**não muda nada**: o resultado é sempre `flo`.

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
