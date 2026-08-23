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

![exemplo 1](../../assets/string__maketrans__maketrans_ex1.png)

<details><summary>código</summary>

```ps
post("abc".translate("abc".maketrans("abc", "xyz")))
```

</details>

```saida
xyz
```

[← índice](../string.md)
