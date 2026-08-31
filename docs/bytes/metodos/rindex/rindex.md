# `b.rindex(sub, inicio=0, fim=Null)`

Como `rfind`, mas levanta `ValueError` quando não acha.

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
b = "Hello".encode()
post(b.rindex("l".encode()))
```

```saida
3
```

[← índice](../../bytes.md)
