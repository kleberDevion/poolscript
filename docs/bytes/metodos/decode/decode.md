# `b.decode(encoding="utf-8", errors="strict")`

Volta pra texto, decodificando com o codec pedido.

## Parâmetros

| nome | default |
|---|---|
| `encoding` | `"utf-8"` |
| `errors` | `"strict"` |

## Retorno

str

## Exemplos

```ps
import bytes
b = "ção".encode()
post(b.decode())
post(b.len(), "bytes ->", len(b.decode()), "caracteres")
```

```saida
ção
5 bytes -> 3 caracteres
```

[← índice](../../bytes.md)
