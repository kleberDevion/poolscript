# `s.casefold()`

Minúsculas agressivas para comparação sem caixa.

## Retorno

str

## Exemplos

```ps
post("Straße".casefold())
```

```saida
strasse
```

## Bordas

- ß vira ss — mais forte que lower()
- É a dobra completa do Unicode (CaseFolding): `"ﬁ".casefold()` é `fi`, o
  sigma final vira `σ` (`"ΣΑΣ".casefold()` é `σασ`), e no cherokee a dobra
  é a maiúscula (`"ꭰ".casefold()` é `Ꭰ`).

[← índice](../str.md)
