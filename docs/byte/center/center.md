# `b.center(width, fillbyte)`

Centraliza na largura. Com sobra ímpar e largura ímpar, o byte a mais fica à esquerda.

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
post("ab".encode().center(7, "*".encode()))
```

```saida
b'***ab**'
```

[← índice](../byte.md)
