# `s.sub(padrao, novo)`

Substitui as ocorrências do padrão regex.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `padrao` | str regex | — |  |
| `novo` | str | — |  |

## Retorno

str

## Erros

- **TypeError** — o padrão não é uma expressão regular válida

## Exemplos

```ps
post("a1b2".sub("[0-9]", "#"))
```

```saida
a#b#
```

[← índice](../string.md)
