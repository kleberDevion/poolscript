# `s.isnumeric()`

True se só tem caracteres numéricos.

## Retorno

bool

## Exemplos

```ps
post("123".isnumeric())
```

```saida
True
```

## Bordas

- Pela tabela do Unicode: além dos decimais e dígitos, frações (`½`),
  numerais romanos (`Ⅷ`) e os numerais CJK: `"½Ⅷ一二".isnumeric()` é True.

[← índice](../str.md)
