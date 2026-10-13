# `s.isalnum()`

True se só tem letras e dígitos.

## Retorno

bool

## Exemplos

```ps
post("a1".isalnum(), "a 1".isalnum())
```

```saida
True False
```

## Bordas

- Cada caractere é letra ([`isalpha`](../isalpha/isalpha.md)) ou numérico
  ([`isnumeric`](../isnumeric/isnumeric.md)), de qualquer escrita:
  `"漢٣½".isalnum()` é True.

[← índice](../str.md)
