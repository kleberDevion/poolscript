# `flo(x)`

Converte para número de ponto flutuante.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | str | int | bool | flo | — |  |

## Retorno

flo

## Erros

- **SomeValueUnexpected** — string que não é número

## Exemplos

![exemplo 1](../../assets/builtins__flo__flo_ex1.png)

<details><summary>código</summary>

```ps
post(flo("2.5"), flo(3))
```

</details>

```saida
2.5 3.0
```

## Bordas

- o nome é `flo`, não `float` — `float` não existe na linguagem

[← índice](../builtins.md)
