# `removeStart(lista)`

Remove e devolve o PRIMEIRO item da lista.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | — | mutada in-place |

## Retorno

o item removido

## Erros

- **TypeError** — o argumento não é `list`: `removeStart() argument 1 must be list, not int`

Lista vazia **não** é erro: devolve `null`.

## Exemplos

```ps
l = [1, 2]
post(removeStart(l), l)
```

```saida
1 [2]
```

[← índice](../builtins.md)
