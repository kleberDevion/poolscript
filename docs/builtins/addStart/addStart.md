# `addStart(lista, item)`

Insere o item no INÍCIO da lista, mutando a própria lista.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | — | mutada in-place |
| `item` | qualquer | — |  |

## Retorno

Null

## Erros

- **SomeValueUnexpected** — primeiro argumento não é lista

## Exemplos

```ps
l = [1]
addStart(l, 0)
post(l)
```

```saida
[0, 1]
```

[← índice](../builtins.md)
