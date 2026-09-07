# `s.findall(padrao)`

Lista com todas as ocorrências do padrão.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `padrao` | str regex | — |  |

## Retorno

list

## Erros

- **ValueError** — o padrão não é uma expressão regular válida
  (`unterminated character set at position 0`). Não é `TypeError`.

## Exemplos

```ps
post("a1b2".findall("[0-9]"))
```

```saida
['1', '2']
```

[← índice](../string.md)
