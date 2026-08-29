# `chr(n)`

Caractere do codepoint unicode.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `n` | int | — |  |

## Retorno

str

## Erros

- **TypeError** — o número está fora da faixa Unicode (0 a 0x10FFFF)

## Exemplos

```ps
post(chr(66), chr(231))
```

```saida
B ç
```

[← índice](../builtins.md)
