# `b.removesuffix(p)`

Tira o sufixo se ele estiver lá; senão devolve igual.

## Parâmetros

| nome | default |
|---|---|
| `p` | — |

## Retorno

bytes

## Exemplos

```ps
import bytes
post("Hello".encode().removesuffix("lo".encode()))
```

```saida
b'Hel'
```

[← índice](../../bytes.md)
