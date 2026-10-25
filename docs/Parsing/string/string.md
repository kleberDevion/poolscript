# `Parsing.string(value, to_type="str")`

Devolve o texto do valor, com os brancos normalizados: cada corrida de
branco vira um espaço só, e os das pontas saem. Branco é a régua da
linguagem, a mesma do `split()` sem separador e do
[`isspace`](../../str/isspace/isspace.md): a propriedade `White_Space` do
Unicode, com o espaço sem quebra (U+00A0), o espaço ideográfico (U+3000) e
os outros. É o `" ".join(s.split())` do Python, menos os separadores U+001C
a U+001F, que o Python conta como branco e a linguagem conta como controle.

```assinatura
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

O espaço ideográfico, o `\v` e o `\f` também são branco, e o U+001C não é
(até o 20.0.0, só o espaço, o tab, o `\n` e o `\r` contavam):

```ps
import Parsing

post(Parsing.string("nome\U00003000\U00003000sobrenome\x0b\x0cfim"))
post(Parsing.string("a\x1cb") == "a\x1cb")
```

```saida
nome sobrenome fim
True
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
