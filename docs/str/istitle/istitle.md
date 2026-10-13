# `s.istitle()`

True se cada palavra começa com maiúscula.

## Retorno

bool

## Exemplos

```ps
post("Ola Mundo".istitle())
```

```saida
True
```

## Bordas

- Maiúscula (ou letra de título, `ǅ`) só logo depois de um caractere sem
  caixa; minúscula só depois de um com caixa; e pelo menos uma letra:
  `"OLA".istitle()` é False, `"ǈubljana".istitle()` é True,
  `"漢 Ola".istitle()` é True.

[← índice](../str.md)
