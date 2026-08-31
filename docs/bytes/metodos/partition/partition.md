# `b.partition(sep)`

Corta na PRIMEIRA ocorrência e devolve `(antes, separador, depois)`. Sem ocorrência, tudo vai no primeiro.

## Parâmetros

| nome | default |
|---|---|
| `sep` | — |

## Retorno

tup

## Exemplos

```ps
import bytes
post("a=b=c".encode().partition("=".encode()))
post("abc".encode().partition("=".encode()))
```

```saida
(b'a', b'=', b'b=c')
(b'abc', b'', b'')
```

[← índice](../../bytes.md)
