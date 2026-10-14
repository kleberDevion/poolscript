# `Parsing.TransientValue(value, to_type="str")`

Converte um valor **preservando o original sem distorção** — trunca float pra
int sem arredondar, acha o número dentro do texto, entende o formato
brasileiro.

```
Parsing.TransientValue(value, to_type="str")
```

| `to_type` | Converte pra |
|---|---|
| `"int"` | inteiro (o [`integer`](../integer/integer.md)) |
| `"flo"` / `"float"` | decimal (o [`floating`](../floating/floating.md)) |
| `"str"` e qualquer outro | texto |

## Retorno

`str`, `int` ou `flo`, conforme a tabela acima. Um alvo desconhecido devolve o
texto do valor (antes devolvia o próprio valor: a lista, num `str q = ...`).

---

## Uso

```ps
import Parsing

post(Parsing.TransientValue(123.7, to int), Parsing.TransientValue("R$ 10", to flo), Parsing.TransientValue(42, to str))
post(type(Parsing.TransientValue([1, 2], to list)))
```

```saida
123 10.0 42
str
```

É a base que os outros conversores (`integer`, `floating`) usam por dentro.
No dia a dia, prefira os específicos ([`integer`](../integer/integer.md),
[`floating`](../floating/floating.md)) — são mais claros.

---

## Relacionados

- [`Parsing.integer()`](../integer/integer.md) · [`Parsing.floating()`](../floating/floating.md)
