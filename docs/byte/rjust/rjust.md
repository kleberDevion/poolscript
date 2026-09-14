# `b.rjust(width, fillbyte)`

Enche à esquerda até a largura.

## Parâmetros

| nome | default |
|---|---|
| `width` | — |
| `fillbyte` | — |

## Retorno

byte

## Exemplos

```ps
import bytes
post("ab".encode().rjust(5, ".".encode()))
```

```saida
b'...ab'
```

[← índice](../byte.md)
