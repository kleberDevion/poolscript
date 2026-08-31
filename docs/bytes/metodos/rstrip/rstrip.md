# `b.rstrip(chars=Null)`

Como `strip`, só da direita.

## Parâmetros

| nome | default |
|---|---|
| `chars` | `Null` |

## Retorno

bytes

## Exemplos

```ps
import bytes
post("xyaXbyx".encode().rstrip("xy".encode()))
```

```saida
b'xyaXb'
```

[← índice](../../bytes.md)
