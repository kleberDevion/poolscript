# `s.isalpha()`

True se só tem letras (e não é vazia).

## Retorno

bool

## Exemplos

```ps
post("abc".isalpha(), "a1".isalpha())
```

```saida
True False
```

## Bordas

- Letra é a categoria L do Unicode (maiúscula, minúscula, título,
  modificadora e as letras sem caixa), de qualquer escrita:
  `"ação".isalpha()`, `"Ωμέγα".isalpha()` e `"漢字".isalpha()` são True;
  dígito, espaço, pontuação e acento solto (combinante) não são letra.

[← índice](../str.md)
