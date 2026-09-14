# `b.lstrip(chars=Null)`

Como `strip`, só da esquerda.

## Parâmetros

| nome | default |
|---|---|
| `chars` | `Null` |

## Retorno

byte

## Exemplos

```ps
import bytes
post("xyaXbyx".encode().lstrip("xy".encode()))
```

```saida
b'aXbyx'
```

[← índice](../byte.md)
