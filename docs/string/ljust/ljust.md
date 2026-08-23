# `s.ljust(largura, preenchimento=" ")`

Preenche à direita até a largura.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `largura` | int | — |  |
| `preenchimento` | str de 1 char | " " |  |

## Retorno

str

## Exemplos

![exemplo 1](../../assets/string__ljust__ljust_ex1.png)

<details><summary>código</summary>

```ps
post("ab".ljust(5, "-"))
```

</details>

```saida
ab---
```

[← índice](../string.md)
