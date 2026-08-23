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

```ps
post("banana".rindex("na"))
```

```saida
4
```

[← índice](../string.md)
