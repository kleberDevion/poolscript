# `enumerate(lista)`

Lista de tuplas (indice, item), começando em 0.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | tup | str | — |  |

## Retorno

list de tup

## Erros

- **TypeError** — o valor não é percorrível (`int`, `flo`, `bool` e `Null` não são)

## Exemplos

```ps
post(enumerate(["a", "b"]))
```

```saida
[(0, 'a'), (1, 'b')]
```

[← índice](../builtins.md)
