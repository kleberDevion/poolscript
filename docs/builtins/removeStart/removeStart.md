# `removeStart(lista)`

Remove e devolve o PRIMEIRO item da lista.

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
post(removeStart(l), l)
```

```saida
1 [2]
```

[← índice](../builtins.md)
