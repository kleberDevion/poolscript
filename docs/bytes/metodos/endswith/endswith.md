# `b.endswith(sufixo, inicio=0, fim=Null)`

True se termina com o sufixo. Mesmas regras do `startswith`.

## Parâmetros

| nome | default |
|---|---|
| `sufixo` | — |
| `inicio` | `0` |
| `fim` | `Null` |

## Retorno

bool

## Exemplos

```ps
import bytes
b = "abcdef".encode()
post(b.endswith("def".encode()))
post(b.endswith("cd".encode(), 0, 4))
```

```saida
True
True
```

[← índice](../../bytes.md)
