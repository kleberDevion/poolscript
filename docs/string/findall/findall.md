# `s.findall(padrao)`

Lista com todas as ocorrências do padrão.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `padrao` | str regex | — |  |

## Retorno

list

## Erros

- **TypeError** — o padrão não é uma expressão regular válida

## Exemplos

```ps
post("a1b2".findall("[0-9]"))
```

```saida
['1', '2']
```

[← índice](../string.md)
