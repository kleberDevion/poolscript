# `s.isdigit()`

True se só tem dígitos.

## Retorno

bool

## Exemplos

```ps
post("123".isdigit(), "1.5".isdigit())
```

```saida
True False
```

## Bordas

- Dígito, pela tabela do Unicode: os decimais de qualquer escrita (os do
  [`isdecimal`](../isdecimal/isdecimal.md)) e os dígitos sobrescritos e
  circulados: `"²".isdigit()` e `"①".isdigit()` são True, `"½".isdigit()`
  é False (fração é do [`isnumeric`](../isnumeric/isnumeric.md)).
- É método de `str`: `(150).isdigit()` é `AttributeError: 'int' object has no
  method 'isdigit'` (não há conversão implícita; `str(150).isdigit()` vale).

[← índice](../str.md)
