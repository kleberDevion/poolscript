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

- **TypeError** — o argumento não é `int` nem `flo`

## Exemplos

```ps
post(round(3.567, 2), round(2.5))
```

```saida
3.57 2
```

## Bordas

- arredondamento de banco (half-to-even): `round(2.5)` → 2, `round(3.5)` → 4

[← índice](../builtins.md)
