# `flo(x)`

Converte para número de ponto flutuante.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | str | int | bool | flo | — |  |

## Retorno

flo

## Erros

- **ValueError** — a `str` não contém um número válido: `flo("x")`

## Exemplos

```ps
post(flo("2.5"), flo(3))
```

```saida
2.5 3.0
```

## Bordas

- o nome é `flo`, não `float` — `float` não existe na linguagem

[← índice](../builtins.md)
