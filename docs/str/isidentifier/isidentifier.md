# `s.isidentifier()`

True se a string é um nome válido na Jinga: letra ASCII ou `_` no começo, e
depois letras ASCII, `_` ou dígitos (a regra 1.4 da linguagem).

## Retorno

bool

## Exemplos

```ps
post("nome_1".isidentifier(), "1a".isidentifier(), "ação".isidentifier())
```

```saida
True False False
```

## Bordas

- Letra acentuada ou de fora do ASCII não entra, como no código: `int ação =
  1` é SyntaxError.
- Palavra reservada passa (`"if".isidentifier()` é True): o método confere a
  forma do nome, não se ele está livre.

[← índice](../str.md)
