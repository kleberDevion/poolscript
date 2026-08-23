# `s.removeprefix(p)`

Remove o prefixo exato, se presente.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `p` | str | — |  |

## Retorno

str

## Exemplos

![exemplo 1](../../assets/string__removeprefix__removeprefix_ex1.png)

<details><summary>código</summary>

```ps
post("api_nome".removeprefix("api_"))
```

</details>

```saida
nome
```

## Bordas

- sem o prefixo, devolve a string intacta

[← índice](../string.md)
