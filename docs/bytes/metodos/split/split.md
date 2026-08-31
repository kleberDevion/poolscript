# `b.split(sep=Null, maxsplit=-1)`

Parte no separador. Sem separador (ou com `Null`), parte em RUNS de branco e descarta os das pontas.

## Parâmetros

| nome | default |
|---|---|
| `sep` | `Null` |
| `maxsplit` | `-1` |

## Retorno

list

## Exemplos

```ps
import bytes
post("a-b-c".encode().split("-".encode()))
post("a-b-c".encode().split("-".encode(), 1))
post("  a  b \t c ".encode().split())
```

```saida
[b'a', b'b', b'c']
[b'a', b'b-c']
[b'a', b'b', b'c']
```

[← índice](../../bytes.md)
