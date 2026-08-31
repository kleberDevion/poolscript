# `b.rjust(width, fillbyte)`

Enche à esquerda até a largura.

## Parâmetros

| nome | default |
|---|---|
| `width` | — |
| `fillbyte` | — |

## Retorno

bytes

## Exemplos

```ps
import bytes
post("ab".encode().rjust(5, ".".encode()))
```

```saida
b'...ab'
```

[← índice](../../bytes.md)
