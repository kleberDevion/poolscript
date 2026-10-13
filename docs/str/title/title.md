# `s.title()`

Primeira letra de cada palavra em maiúscula.

## Retorno

str

## Exemplos

```ps
post("ola mundo".title())
```

```saida
Ola Mundo
```

## Bordas

- Sobe (pra forma de título) a letra que vem depois de um caractere sem
  caixa e desce as outras: `"漢a".title()` é `漢A`, `"ﬁx".title()` é `Fix`.
  O apóstrofo não tem caixa: `"it's".title()` é `It'S`.
- O sigma no fim de palavra vira o sigma final, como no `lower`:
  `"ΟΔΥΣΣΕΥΣ".title()` é `Οδυσσευς`.

[← índice](../str.md)
