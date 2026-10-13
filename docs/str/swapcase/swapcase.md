# `s.swapcase()`

Inverte a caixa de cada letra.

## Retorno

str

## Exemplos

```ps
post("AbC".swapcase())
```

```saida
aBc
```

## Bordas

- Maiúscula desce e minúscula sobe, pela caixa completa do Unicode:
  `"Straße".swapcase()` é `sTRASSE` (o `ß` vira `SS`). A letra de título
  (`ǅ`) fica como está, e o `Σ` que desce no fim de palavra vira `ς`.

[← índice](../str.md)
