# `b.hex(sep=Null, bytes_per_sep=1)`

Os bytes como texto hexadecimal minúsculo. Com `sep`, um separador a cada `bytes_per_sep` bytes — positivo agrupa da direita, negativo da esquerda.

## Parâmetros

| nome | default |
|---|---|
| `sep` | `Null` |
| `bytes_per_sep` | `1` |

## Retorno

str

## Exemplos

```ps
import bytes
b = bytes.fromhex("deadbeef")
post(b.hex())
post(b.hex("-"))
post(b.hex("_", 2))
```

```saida
deadbeef
de-ad-be-ef
dead_beef
```

[← índice](../../bytes.md)
