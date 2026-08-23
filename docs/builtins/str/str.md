# `str(x)`

Converte qualquer valor para texto, na renderização da PoolScript.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | qualquer | — |  |

## Retorno

str

## Exemplos

![exemplo 1](../../assets/builtins__str__str_ex1.png)

<details><summary>código</summary>

```ps
post(str(12) + "!", str(Null), str(true))
```

</details>

```saida
12! null True
```

## Bordas

- `str(Null)` é `"null"` (nunca `None`); bool vira `True`/`False`

[← índice](../builtins.md)
