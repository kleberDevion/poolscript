# `b.expandtabs(tabsize=8)`

Troca cada tabulação pelos espaços que faltam até a próxima parada.

## Parâmetros

| nome | default |
|---|---|
| `tabsize` | `8` |

## Retorno

byte

## Exemplos

```ps
import bytes
post("a\tbc\td".encode().expandtabs(4))
```

```saida
b'a   bc  d'
```

[← índice](../byte.md)
