# `Parsing.string(value, to_type="str")`

Devolve o texto do valor, com os brancos normalizados: cada corrida de
espaço, tab e quebra de linha vira um espaço só, e os das pontas saem (o
`" ".join(s.split())` do Python).

```
Parsing.string(value, to_type="str")
```

---

## Uso

```ps
import Parsing

post(Parsing.string("  texto   com   espaços  "), Parsing.string(42))
post(Parsing.string("R$ 1.299,90", to flo), Parsing.string("7 itens", to int))
```

```saida
texto com espaços 42
1299.9 7
```

Com `to_type` `"int"` ou `"flo"`, o resultado é o do
[`integer`](../integer/integer.md) ou do [`floating`](../floating/floating.md),
com a mesma regra do número. Antes era outro caminho, que juntava todos os
dígitos (`"R$ 1.299,90"` dava 129990.0). O texto tem qualquer tamanho (era
cortado em 1.024 caracteres).

## Retorno

`str` — ou `int`/`flo` quando `to_type` é `int`/`flo`. Qualquer outro alvo
devolve `str`.

---

## Relacionados

- [`Parsing.integer()`](../integer/integer.md) · [`Parsing.floating()`](../floating/floating.md)
- [visão geral do Parsing](../Parsing.md)
