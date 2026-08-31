# `b.isascii()`

True se todo byte é menor que 0x80. Diferente dos outros predicados, o VAZIO é True.

## Parâmetros

Nenhum.

## Retorno

bool

## Exemplos

```ps
import bytes
post("abc".encode().isascii())
post(bytes.fromhex("80").isascii())
post("".encode().isascii())
```

```saida
True
False
True
```

[← índice](../../bytes.md)
