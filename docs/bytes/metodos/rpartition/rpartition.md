# `b.rpartition(sep)`

Corta na ÚLTIMA ocorrência. Sem ocorrência, tudo vai no ÚLTIMO — ao contrário do `partition`.

## Parâmetros

| nome | default |
|---|---|
| `sep` | — |

## Retorno

tup

## Exemplos

```ps
import bytes
post("a=b=c".encode().rpartition("=".encode()))
post("abc".encode().rpartition("=".encode()))
```

```saida
(b'a=b', b'=', b'c')
(b'', b'', b'abc')
```

[← índice](../../bytes.md)
