# `int(x)`

Converte para inteiro: string numérica, float (trunca) ou bool.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | str | flo | bool | int | — |  |

## Retorno

int

## Erros

- **SomeValueUnexpected** — string que não é número

## Exemplos

![exemplo 1](../../assets/builtins__int__int_ex1.png)

<details><summary>código</summary>

```ps
post(int("7"), int(3.9), int(true))
```

</details>

```saida
7 3 1
```

## Bordas

- float TRUNCA em direção ao zero (`int(3.9)` → 3), não arredonda

[← índice](../builtins.md)
