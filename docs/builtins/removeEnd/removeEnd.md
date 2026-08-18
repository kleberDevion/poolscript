# `removeEnd(lista)`

Remove e devolve o ÚLTIMO item da lista.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | — | mutada in-place |

## Retorno

o item removido; **`null`** se a lista estiver vazia (não é erro)

## Erros

- **SomeValueUnexpected** — o argumento não é uma lista

## Exemplos

```ps
l = [1, 2]
post(removeEnd(l), l)
```

```saida
2 [1]
```

[← índice](../builtins.md)
