# `s.capitalize()`

Só a primeira letra da string em maiúscula; o resto minúsculo.

## Retorno

str

## Exemplos

```ps
post("OLA mundo".capitalize())
```

```saida
Ola mundo
```

## Bordas

- O primeiro caractere vai pra forma de título do Unicode, que nas letras
  duplas e nas ligaduras não é a maiúscula: `"ǆemal".capitalize()` é
  `ǅemal`, `"ßa".capitalize()` é `Ssa`. O resto desce, com o sigma final
  (`Σ` no fim de palavra vira `ς`).

[← índice](../str.md)
