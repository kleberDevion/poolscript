# `b.join(lista)`

Junta uma lista de bytes usando ESTE valor como separador.

## Parâmetros

| nome | default |
|---|---|
| `lista` | — |

## Retorno

bytes

## Exemplos

```ps
import bytes
post("-".encode().join(["a".encode(), "b".encode(), "c".encode()]))
```

```saida
b'a-b-c'
```

[← índice](../../bytes.md)
