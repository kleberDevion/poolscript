# `zip(a, b, ...)`

Lista de tuplas pareando os iteráveis; para no menor.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `a, b...` | list | tup | — | 2+ iteráveis |

## Retorno

list de tup

## Erros

- **SomeValueUnexpected** — tipo não-iterável

## Exemplos

```ps
post(zip([1, 2], ["x", "y"]))
```

```saida
[(1, 'x'), (2, 'y')]
```

## Bordas

- comprimentos diferentes truncam no MENOR, sem erro

[← índice](../builtins.md)
