# `s.index(sub)`

Como find, mas ERRA quando não acha.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sub` | str | — |  |

## Retorno

int

## Erros

- **SomeValueUnexpected** — substring ausente

## Exemplos

![exemplo 1](../../assets/string__index__index_ex1.png)

<details><summary>código</summary>

```ps
post("banana".index("na"))
```

</details>

```saida
2
```

[← índice](../string.md)
