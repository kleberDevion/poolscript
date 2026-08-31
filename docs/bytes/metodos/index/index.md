# `b.index(sub, inicio=0, fim=Null)`

Como `find`, mas levanta `ValueError` quando não acha, em vez de devolver -1.

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
post(b.index("l".encode()))
try { b.index("z".encode()) } catch (e) { post(e) }
```

```saida
2
subsection not found (linha 4)
```

[← índice](../../bytes.md)
