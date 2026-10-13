# `s.isdecimal()`

True se só tem dígitos decimais.

## Retorno

bool

## Exemplos

```ps
post("123".isdecimal())
```

```saida
True
```

## Bordas

- Decimal de qualquer escrita, pela tabela do Unicode (os dígitos que formam
  número na base 10): `"٣٤".isdecimal()` (árabe-índico) e
  `"４２".isdecimal()` (largura plena) são True; `"²".isdecimal()` e
  `"½".isdecimal()` são False.

[← índice](../str.md)
