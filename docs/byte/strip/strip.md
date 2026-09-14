# `b.strip(chars=Null)`

Tira das duas pontas. `chars` é um CONJUNTO de bytes, não um prefixo. Sem argumento, tira branco ASCII.

## Parâmetros

| nome | default |
|---|---|
| `chars` | `Null` |

## Retorno

byte

## Exemplos

```ps
import bytes
post("  a b \n".encode().strip())
post("xyaXbyx".encode().strip("xy".encode()))
```

```saida
b'a b'
b'aXb'
```

[← índice](../byte.md)
