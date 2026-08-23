# `chr(n)`

Caractere do codepoint unicode.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `n` | int | — |  |

## Retorno

str

## Erros

- **SomeValueUnexpected** — fora da faixa unicode

## Exemplos

![exemplo 1](../../assets/builtins__chr__chr_ex1.png)

<details><summary>código</summary>

```ps
post(chr(66), chr(231))
```

</details>

```saida
B ç
```

[← índice](../builtins.md)
