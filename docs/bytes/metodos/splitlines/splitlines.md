# `b.splitlines(keepends=false)`

Parte em linhas. Quebra em `\n`, `\r` e `\r\n` — e SÓ. É a mesma regra do
`str`: nenhum dos dois quebra em `\v` ou `\f` (`"a\vb".splitlines()` devolve
`['a\x0bb']`, um elemento só).

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
