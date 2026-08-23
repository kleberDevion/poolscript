# `sep.join(lista)`

Junta os itens da lista com o separador.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list de str | — |  |

## Retorno

str

## Erros

- **SomeValueUnexpected** — item não-string

## Exemplos

![exemplo 1](../../assets/string__join__join_ex1.png)

<details><summary>código</summary>

```ps
post("-".join(["a", "b", "c"]))
```

</details>

```saida
a-b-c
```

[← índice](../string.md)
