# `b.contains(sub)`

True se contém. É `find(...) >= 0` com nome.

## Parâmetros

| nome | default |
|---|---|
| `sub` | — |

## Retorno

bool

## Exemplos

```ps
import bytes
b = "Hello".encode()
post(b.contains("ell".encode()))
post(b.contains(0))
```

```saida
True
False
```

[← índice](../../bytes.md)
