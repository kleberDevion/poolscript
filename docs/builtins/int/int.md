# `int(x)`

Converte para inteiro: string numérica, float (trunca) ou bool.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | str | flo | bool | int | — |  |

## Retorno

int

## Erros

- **ValueError** — a `str` não contém um número válido: `int("abc")`

## Exemplos

```ps
post(int("7"), int(3.9), int(true))
```

```saida
7 3 1
```

## Bordas

- float TRUNCA em direção ao zero (`int(3.9)` → 3), não arredonda

[← índice](../builtins.md)
