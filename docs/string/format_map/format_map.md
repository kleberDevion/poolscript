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

```ps
post("{a}!".format_map({"a": "oi"}))
```

```saida
oi!
```

[← índice](../string.md)
