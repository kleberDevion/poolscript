# `chr(n)`

Caractere do codepoint unicode.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `n` | int | — |  |

## Retorno

str

## Erros

- **ValueError** — o número está fora da faixa Unicode: `chr() arg not in range(0x110000)`
- **TypeError** — o argumento não é inteiro: `'str' object cannot be interpreted as an integer`

## Exemplos

```ps
post(chr(66), chr(231))
```

```saida
B ç
```

[← índice](../builtins.md)
