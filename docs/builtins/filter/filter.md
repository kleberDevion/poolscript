# `filter(lista, fn)`

Nova lista só com os itens em que fn devolve verdadeiro. A LISTA vem primeiro.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | tup | — |  |
| `fn` | funct | — | recebe 1 argumento |

## Retorno

list

## Erros

- **TypeError** — fn não é chamável

## Exemplos

```ps
funct par(x) { return x % 2 == 0 }
post(filter([1, 2, 3, 4], par))
```

```saida
[2, 4]
```

[← índice](../builtins.md)
