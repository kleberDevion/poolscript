# `s.rindex(sub)`

Como rfind, mas ERRA quando não acha.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sub` | str | — |  |

## Retorno

int

## Erros

- **SomeValueUnexpected** — substring ausente

## Exemplos

![exemplo 1](../../assets/string__rindex__rindex_ex1.png)

<details><summary>código</summary>

```ps
post("banana".rindex("na"))
```

</details>

```saida
4
```

[← índice](../string.md)
