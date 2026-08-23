# `list(x)`

Converte para lista: string vira caracteres, dict vira chaves, tupla vira lista.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | str | dict | tup | list | — |  |

## Retorno

list

## Erros

- **SomeValueUnexpected** — tipo não-iterável

## Exemplos

![exemplo 1](../../assets/builtins__list__list_ex1.png)

<details><summary>código</summary>

```ps
post(list("abc"), list({"a": 1}))
```

</details>

```saida
['a', 'b', 'c'] ['a']
```

[← índice](../builtins.md)
