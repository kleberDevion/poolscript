# `sorted(lista)`

Nova lista com os itens em ordem crescente; a original não muda.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | tup | — |  |

## Retorno

list

## Erros

- **TypeError** — os itens não se comparam entre si (ex.: `int` com `str`)

## Exemplos

```ps
post(sorted([3, 1, 2]))
```

```saida
[1, 2, 3]
```

[← índice](../builtins.md)
