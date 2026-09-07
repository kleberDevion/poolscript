# `str(x)`

Converte qualquer valor para texto, na renderização da PoolScript.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | qualquer | — |  |

## Retorno

str

## Exemplos

```ps
post(str(12) + "!", str(Null), str(true))
```

```saida
12! null True
```

## Bordas

- `str(Null)` é `"Null"` (nunca `None` nem `null`); bool vira `True`/`False`

[← índice](../builtins.md)
