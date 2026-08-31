# `b.count(sub, inicio=0, fim=Null)`

Quantas vezes aparece, sem sobreposição. Agulha vazia conta as POSIÇÕES.

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
b = "aaaa".encode()
post(b.count("aa".encode()))
post(b.count(97))
post(b.count("".encode()))
```

```saida
2
4
5
```

[← índice](../../bytes.md)
