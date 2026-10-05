# `str(x)`

Converte qualquer valor para texto, na renderização da Jinga.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | qualquer | — |  |

## Retorno

str

## Exemplos

```ps
post(str(12) + "!", str(Null), str(True))
```

```saida
12! Null True
```

## Bordas

- `str(Null)` é `"Null"` (nunca `"null"` nem `"None"`); bool vira `True`/`False`

[← índice](../builtins.md)
