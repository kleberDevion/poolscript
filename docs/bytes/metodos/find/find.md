# `b.find(sub, inicio=0, fim=Null)`

A posição da primeira ocorrência, ou -1. A agulha pode ser bytes OU um inteiro de 0 a 255.

## Parâmetros

| nome | default |
|---|---|
| `sub` | — |
| `inicio` | `0` |
| `fim` | `Null` |

## Retorno

int

## Exemplos

```ps
import bytes
b = "Hello, World".encode()
post(b.find("o".encode()))
post(b.find(111))
post(b.find("zz".encode()))
```

```saida
4
4
-1
```

[← índice](../../bytes.md)
