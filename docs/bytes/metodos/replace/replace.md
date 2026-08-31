# `b.replace(old, new, count=-1)`

Troca ocorrências. `count` limita quantas; -1 (o padrão) troca todas.

## Parâmetros

| nome | default |
|---|---|
| `old` | — |
| `new` | — |
| `count` | `-1` |

## Retorno

bytes

## Exemplos

```ps
import bytes
b = "aaa".encode()
post(b.replace("a".encode(), "b".encode()))
post(b.replace("a".encode(), "b".encode(), 2))
```

```saida
b'bbb'
b'bba'
```

[← índice](../../bytes.md)
