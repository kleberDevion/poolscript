# `b.rstrip(chars=Null)`

Como `strip`, só da direita.

## Parâmetros

| nome | default |
|---|---|
| `chars` | `Null` |

## Retorno

byte

## Exemplos

```ps
import bytes
post("xyaXbyx".encode().rstrip("xy".encode()))
```

```saida
b'xyaXb'
```

[← índice](../byte.md)
