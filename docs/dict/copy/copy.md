# `d.copy()`

Cópia RASA: dict novo, valores compartilhados.

## Retorno

dict — a cópia

## Exemplos

```ps
d = { "a": 1 }
c = d.copy()
c["b"] = 2
post(d, c)
```

```saida
{'a': 1} {'a': 1, 'b': 2}
```

```ps
dentro = [1]
d = { "l": dentro }
c = d.copy()
dentro.append(2)
post(c)
```

```saida
{'l': [1, 2]}
```

## Bordas

- RASA: mexer num valor aninhado aparece nos dois dicts (2º exemplo)

[← índice](../dict.md)
