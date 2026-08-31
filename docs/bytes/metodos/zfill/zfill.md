# `b.zfill(largura)`

Enche com zeros à esquerda. Um `+` ou `-` inicial fica NA FRENTE dos zeros.

## Parâmetros

| nome | default |
|---|---|
| `largura` | — |

## Retorno

bytes

## Exemplos

```ps
import bytes
post("42".encode().zfill(8))
post("-42".encode().zfill(8))
```

```saida
b'00000042'
b'-0000042'
```

[← índice](../../bytes.md)
