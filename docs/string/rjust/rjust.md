# `s.rjust(width, fillchar)`

Preenche à esquerda até a largura.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `width` | int | — |  |
| `fillchar` | str de 1 char | espaço | omitir preenche com espaço |

Os nomes são estes: `"ab".rjust(largura=5)` é
`TypeError: 'largura' is an invalid keyword argument for rjust()`.

## Retorno

str

## Exemplos

```ps
post("ab".rjust(5, "-"))
```

```saida
---ab
```

[← índice](../string.md)
