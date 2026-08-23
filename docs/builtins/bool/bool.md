# `bool(x)`

Verdade do valor: vazio/zero/Null são falsos, o resto é verdadeiro.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | qualquer | — |  |

## Retorno

bool

## Exemplos

![exemplo 1](../../assets/builtins__bool__bool_ex1.png)

<details><summary>código</summary>

```ps
post(bool(0), bool(""), bool([1]), bool(Null))
```

</details>

```saida
False False True False
```

[← índice](../builtins.md)
