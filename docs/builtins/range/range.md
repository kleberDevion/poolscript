# `range(fim) | range(inicio, fim, passo=1)`

Lista de inteiros de inicio (inclusive) a fim (exclusive).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `inicio` | int | 0 |  |
| `fim` | int | — | exclusive |
| `passo` | int | 1 |  |

## Retorno

list

## Erros

- **TypeError** — o argumento não é `int`

## Exemplos

```ps
post(range(3), range(1, 7, 2))
```

```saida
[0, 1, 2] [1, 3, 5]
```

## Bordas

- devolve LISTA concreta, não um iterador preguiçoso

[← índice](../builtins.md)
