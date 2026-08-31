# `b.splitlines(keepends=false)`

Parte em linhas. Quebra em `\n`, `\r` e `\r\n` — e SÓ. O `\v` e o `\f`, que no `str` quebram, aqui ficam dentro da linha.

## Parâmetros

| nome | default |
|---|---|
| `keepends` | `false` |

## Retorno

list

## Exemplos

```ps
import bytes
post("a\nb\r\nc".encode().splitlines())
post("a\nb".encode().splitlines(true))
post(bytes.fromhex("610b62").splitlines())
```

```saida
[b'a', b'b', b'c']
[b'a\n', b'b']
[b'a\x0bb']
```

[← índice](../../bytes.md)
