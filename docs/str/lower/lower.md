# `s.lower()`

Tudo em minúsculas.

## Retorno

str

## Exemplos

```ps
post("AÇÃO".lower())
```

```saida
ação
```

## Bordas

- Caixa completa do Unicode, de qualquer escrita: `"ŁÓDŹ".lower()` é
  `łódź`, `"Ա".lower()` é `ա`. `İ` (I com ponto) vira `i` seguido do
  ponto combinante (U+0307): `len("İ".lower())` é 2.
- O sigma maiúsculo no fim de palavra vira o sigma final:
  `"ΟΔΥΣΣΕΥΣ".lower()` é `οδυσσευς`. O mesmo vale no `title`, no
  `capitalize` e no `swapcase`; o `casefold` dá sempre `σ`.

[← índice](../str.md)
