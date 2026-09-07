# `s.replace(old, new, count)`

Troca ocorrências; `old` pode ser LISTA de alvos.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `old` | str \| list | — | lista troca cada um pelo mesmo `new` |
| `new` | str | — |  |
| `count` | int | todas | máximo de trocas; omitir troca todas |

Os nomes são estes: `"banana".replace(velho="na", novo="NA")` é
`TypeError: 'velho' is an invalid keyword argument for replace()`.

## Retorno

str

## Exemplos

```ps
post("banana".replace("na", "NA", 1))
```

```saida
baNAna
```

```ps
post("a-b_c".replace(["-", "_"], "."))
```

```saida
a.b.c
```

## Bordas

- a forma com lista é extensão da PoolScript

[← índice](../string.md)
