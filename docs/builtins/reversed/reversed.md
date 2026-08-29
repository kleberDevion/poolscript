# `reversed(lista)`

Nova lista com os itens na ordem inversa; a original não muda.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | tup | — |  |

## Retorno

list

## Erros

- **TypeError** — o valor não é reversível: `'int' object is not reversible`

Aceita `str`, `list`, `tup`, `bytes` e também `dict` (itera as chaves).

## Exemplos

```ps
post(reversed([1, 2, 3]))
```

```saida
[3, 2, 1]
```

[← índice](../builtins.md)
