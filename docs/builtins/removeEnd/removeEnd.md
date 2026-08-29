# `removeEnd(lista)`

Remove e devolve o ÚLTIMO item da lista.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | — | mutada in-place |

## Retorno

o item removido

## Erros

- **TypeError** — o argumento não é `list`: `removeEnd() argument 1 must be list, not int`

Lista vazia **não** é erro: devolve `null`.

## Exemplos

```ps
l = [1, 2]
post(removeEnd(l), l)
```

```saida
2 [1]
```

[← índice](../builtins.md)
