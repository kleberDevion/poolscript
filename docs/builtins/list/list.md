# `list(x)`

Converte para lista: string vira caracteres, dict vira chaves, tupla vira lista.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | str | dict | tup | list | — |  |

## Retorno

list

## Erros

- **TypeError** — o valor não é percorrível (`int`, `flo`, `bool` e `Null` não são)

## Exemplos

```ps
post(list("abc"), list({"a": 1}))
```

```saida
['a', 'b', 'c'] ['a']
```

[← índice](../builtins.md)
