# `b.lstrip(chars=Null)`

Como `strip`, só da esquerda.

## Parâmetros

| nome | default |
|---|---|
| `chars` | `Null` |

## Retorno

bytes

## Exemplos

```ps
import bytes
post("xyaXbyx".encode().lstrip("xy".encode()))
```

```saida
b'aXbyx'
```

[← índice](../../bytes.md)
