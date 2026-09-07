# `s.ljust(width, fillchar)`

Preenche à direita até a largura.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `width` | int | — |  |
| `fillchar` | str de 1 char | espaço | omitir preenche com espaço |

Os nomes são estes: `"ab".ljust(largura=5)` é
`TypeError: 'largura' is an invalid keyword argument for ljust()`.

## Retorno

str

## Exemplos

```ps
post("ab".ljust(5, "-"))
```

```saida
ab---
```

[← índice](../string.md)
