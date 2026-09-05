# `Parsing.TransientValue(value, to_type="str")`

Converte um valor **preservando o original sem distorção** — trunca float pra
int sem arredondar, extrai dígitos de string, entende separador BR pra float.

```
Parsing.TransientValue(value, to_type="str")
```

| `to_type` | Converte pra |
|---|---|
| `"str"` | texto |
| `"int"` | inteiro (trunca) |
| `"flo"` / `"float"` | decimal |

---

## Uso

```
Parsing.TransientValue(123.7, "int")   # 123   (trunca)
Parsing.TransientValue("R$ 10", "flo") # 10.0
Parsing.TransientValue(42, "str")      # "42"
```

É a base que os outros conversores (`integer`, `floating`) usam por dentro.
No dia a dia, prefira os específicos ([`integer`](../integer/integer.md),
[`floating`](../floating/floating.md)) — são mais claros.

---

## Relacionados

- [`Parsing.integer()`](../integer/integer.md) · [`Parsing.floating()`](../floating/floating.md)
