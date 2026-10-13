# `s.islower()`

True se as letras são todas minúsculas.

## Retorno

bool

## Exemplos

```ps
post("abc".islower())
```

```saida
True
```

## Bordas

- Pela propriedade Lowercase do Unicode, a regra espelhada do
  [`isupper`](../isupper/isupper.md): `"abc 1".islower()` é True,
  `"漢".islower()` é False, e `"ª".islower()` é True (o ordinal tem caixa
  minúscula).

[← índice](../str.md)
