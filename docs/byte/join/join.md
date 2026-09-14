# `b.join(lista)`

Junta uma lista de bytes usando ESTE valor como separador.

## Parâmetros

| nome | default |
|---|---|
| `lista` | — |

## Retorno

byte

## Exemplos

```ps
import bytes
post("-".encode().join(["a".encode(), "b".encode(), "c".encode()]))
```

```saida
b'a-b-c'
```

[← índice](../byte.md)
