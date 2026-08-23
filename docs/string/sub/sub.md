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

- **SomeValueUnexpected** — padrão inválido

## Exemplos

![exemplo 1](../../assets/string__sub__sub_ex1.png)

<details><summary>código</summary>

```ps
post("a1b2".sub("[0-9]", "#"))
```

</details>

```saida
a#b#
```

[← índice](../string.md)
