# `addEnd(lista, item)`

Anexa o item no FIM da lista, mutando a própria lista.

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
addEnd(l, 2)
post(l)
```

```saida
[1, 2]
```

## Bordas

- muta e devolve Null — não encadeia (`addEnd(l,2).x` não existe)

[← índice](../builtins.md)
