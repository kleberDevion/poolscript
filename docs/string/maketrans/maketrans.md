# `s.maketrans(de, para)`

Tabela de tradução caractere-a-caractere para translate.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `de` | str | — |  |
| `para` | str | — | mesmo tamanho |

## Retorno

dict

## Erros

- **ValueError** — os dois argumentos têm tamanhos diferentes
  (`the first two maketrans arguments must have equal length`). Não é
  `TypeError`: um `catch (TypeError e)` não pega.

## Exemplos

```ps
post("abc".translate("abc".maketrans("abc", "xyz")))
```

```saida
xyz
```

[← índice](../string.md)
