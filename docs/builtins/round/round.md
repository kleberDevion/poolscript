# `round(n, casas=0)`

Arredonda um número, opcionalmente com casas decimais.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `n` | int | flo | — |  |
| `casas` | int | 0 |  |

## Retorno

int | flo

## Erros

- **SomeValueUnexpected** — não-número

## Exemplos

![exemplo 1](../../assets/builtins__round__round_ex1.png)

<details><summary>código</summary>

```ps
post(round(3.567, 2), round(2.5))
```

</details>

```saida
3.57 2
```

## Bordas

- arredondamento de banco (half-to-even): `round(2.5)` → 2, como no Python

[← índice](../builtins.md)
