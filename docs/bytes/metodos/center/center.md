# `b.center(width, fillbyte)`

Centraliza na largura. Com sobra ímpar e largura ímpar, o byte a mais fica à esquerda.

## Parâmetros

| nome | default |
|---|---|
| `width` | — |
| `fillbyte` | espaço |

Omitir o `fillbyte` preenche com espaço: `"ab".encode().center(7)` é
`b'   ab  '`.

## Retorno

bytes

## Exemplos

```ps
import bytes
post("ab".encode().center(7, "*".encode()))
```

```saida
b'***ab**'
```

[← índice](../../bytes.md)
