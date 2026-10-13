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
- **ValueError** — o número é um substituto (U+D800 a U+DFFF, metade de um par
  do UTF-16, que não é caractere): `chr(55296)` → `chr() arg is a surrogate
  (0xd800..0xdfff), not a character`
- **TypeError** — o argumento não é inteiro: `'str' object cannot be interpreted as an integer`

## Exemplos

```ps
post(chr(66), chr(231))
```

```saida
B ç
```

[← índice](../builtins.md)
