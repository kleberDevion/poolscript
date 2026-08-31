# `b.upper()`

Maiúsculas — SÓ ASCII. Byte fora de ASCII fica como está.

## Parâmetros

Nenhum.

## Retorno

bytes

## Exemplos

```ps
import bytes
post("hello".encode().upper())
post(bytes.fromhex("c0e0").upper())
```

```saida
b'HELLO'
b'\xc0\xe0'
```

[← índice](../../bytes.md)
