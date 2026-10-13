# `round(n, casas=0)`

Arredonda um número, opcionalmente com casas decimais.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `n` | int | flo | — |  |
| `casas` | int | 0 | negativo arredonda à esquerda da vírgula (dezena, centena...) |

## Retorno

int | flo

## Erros

- **TypeError** — `n` não é `int` nem `flo`: `type str doesn't define
  __round__ method`; `casas` que não é inteiro: `'flo' object cannot be
  interpreted as an integer`

## Exemplos

```ps
post(round(3.567, 2), round(2.5), round(1250, -2), round(1234.5, -2))
```

```saida
3.57 2 1200 1200.0
```

## Bordas

- arredondamento de banco (half-to-even): `round(2.5)` → 2, `round(3.5)` → 4,
  `round(1250, -2)` → 1200, `round(1350, -2)` → 1400
- a conta é sobre o valor binário exato do `flo`: `round(2.675, 2)` é 2.67
  (o 2.675 guardado é um pouco menor que 2.675)
- inteiro com `casas` >= 0 volta igual; com `casas` negativas continua
  inteiro (o grande inclusive): `round(2 ** 70 + 7, -1)` é
  `1180591620717411303430`

[← índice](../builtins.md)
