# `s.center(width, fillchar)`

Centraliza preenchendo dos dois lados.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `width` | int | — |  |
| `fillchar` | str de 1 char | espaço | omitir preenche com espaço |

Os nomes são estes: `"ab".center(largura=6)` é
`TypeError: 'largura' is an invalid keyword argument for center()`.

## Retorno

str

## Exemplos

```ps
post("ab".center(6, "*"))
```

```saida
**ab**
```

[← índice](../string.md)
