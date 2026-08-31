# `b.startswith(prefixo, inicio=0, fim=Null)`

True se começa com o prefixo. Aceita uma tupla/lista de prefixos ("bate com qualquer um") e `inicio`/`fim`.

## Parâmetros

| nome | default |
|---|---|
| `prefixo` | — |
| `inicio` | `0` |
| `fim` | `Null` |

## Retorno

bool

## Exemplos

```ps
import bytes
b = "abcdef".encode()
post(b.startswith("abc".encode()))
post(b.startswith(("x".encode(), "ab".encode())))
post(b.startswith("cd".encode(), 2))
```

```saida
True
True
True
```

[← índice](../../bytes.md)
