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

![exemplo 1](../../assets/builtins__zip__zip_ex1.png)

<details><summary>código</summary>

```ps
post(zip([1, 2], ["x", "y"]))
```

</details>

```saida
[(1, 'x'), (2, 'y')]
```

## Bordas

- comprimentos diferentes truncam no MENOR, sem erro

[← índice](../builtins.md)
