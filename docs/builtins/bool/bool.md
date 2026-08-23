# `bool(x)`

Verdade do valor: vazio/zero/Null são falsos, o resto é verdadeiro.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | qualquer | — |  |

## Retorno

bool

## Exemplos

```ps
post(bool(0), bool(""), bool([1]), bool(Null))
```

```saida
False False True False
```

[← índice](../builtins.md)
