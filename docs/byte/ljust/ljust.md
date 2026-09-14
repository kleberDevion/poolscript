# `b.ljust(width, fillbyte)`

Enche à direita até a largura. `fillbyte` tem que ter exatamente um byte.

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
post("ab".encode().ljust(5, ".".encode()))
```

```saida
b'ab...'
```

[← índice](../byte.md)
