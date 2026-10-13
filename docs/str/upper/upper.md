# `s.upper()`

Tudo em maiúsculas (unicode: ç→Ç, á→Á).

## Retorno

str

## Exemplos

```ps
post("ação".upper())
```

```saida
AÇÃO
```

## Bordas

- Caixa completa do Unicode, de qualquer escrita: `"łódź".upper()` é
  `ŁÓDŹ`, `"ș".upper()` é `Ș`, `"ạ".upper()` é `Ạ`, `"ა".upper()` é `Ა`.
- Um caractere pode virar dois ou três: `"ß"` vira `SS`, `"ﬁ"` vira `FI`,
  `"ŉ"` vira `ʼN`. `ı` (i sem ponto) vira `I`; `ĸ` não tem maiúscula e fica.

[← índice](../str.md)
