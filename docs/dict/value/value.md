# `d.value()`

Os VALORES — idêntico a `values()`, com o nome curto.

## Retorno

list

## Exemplos

```ps
d = { "nome": "ana", "idade": 30 }
post("ana" in d, "ana" in d.value())
```

```saida
False True
```

```ps
d = { "a": [1, 2], "b": (3, 4), "c": 2.5 }
post([1, 2] in d.value(), (3, 4) in d.value(), 2.5 in d.value())
```

```saida
True True True
```

## Bordas

- existe por causa do `in`: `x in d` olha a CHAVE, `x in d.value()` olha o VALOR
- vale pra qualquer tipo de valor — str, int, flo, list, tup

[← índice](../dict.md)
