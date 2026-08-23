# `s.format_map(dict)`

Como format, buscando os {nome} num dict.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `dict` | dict | — |  |

## Retorno

str

## Erros

- **SomeValueUnexpected** — chave ausente

## Exemplos

![exemplo 1](../../assets/string__format_map__format_map_ex1.png)

<details><summary>código</summary>

```ps
post("{a}!".format_map({"a": "oi"}))
```

</details>

```saida
oi!
```

[← índice](../string.md)
