# `s.isupper()`

True se as letras são todas maiúsculas.

## Retorno

bool

## Exemplos

```ps
post("ABC".isupper())
```

```saida
True
```

## Bordas

- Pela propriedade Uppercase do Unicode: o que não tem caixa (dígito,
  espaço, `漢`) não atrapalha, mas tem que haver pelo menos um caractere
  com caixa: `"ABC 1".isupper()` é True, `"漢".isupper()` é False.
- `"Ⅷ".isupper()` é True (numeral romano tem caixa). A letra de título
  (`ǅ`, maiúscula e minúscula juntas) faz o `isupper` e o
  [`islower`](../islower/islower.md) darem False.

[← índice](../str.md)
