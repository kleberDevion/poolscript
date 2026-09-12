# `hex(n)`

Inteiro em hexadecimal, com prefixo 0x. Inteiro grande também (o sinal vai
antes do prefixo: `hex(-(2 ** 70))` é `-0x400000000000000000`).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `n` | int | — |  |

## Retorno

str

## Erros

- **TypeError** — o argumento não é `int`

## Exemplos

```ps
post(hex(255))
```

```saida
0xff
```

[← índice](../builtins.md)
