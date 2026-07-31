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

- **SomeValueUnexpected** — tamanhos diferentes

## Exemplos

```ps
post("abc".translate("abc".maketrans("abc", "xyz")))
```

```saida
xyz
```

[← índice](../string.md)
