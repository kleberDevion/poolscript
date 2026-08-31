# `b.rfind(sub, inicio=0, fim=Null)`

Como `find`, mas a ÚLTIMA ocorrência.

## Parâmetros

| nome | default |
|---|---|
| `sub` | — |
| `inicio` | `0` |
| `fim` | `Null` |

## Retorno

int

## Exemplos

```ps
import bytes
b = "Hello, World".encode()
post(b.rfind("o".encode()))
```

```saida
8
```

[← índice](../../bytes.md)
