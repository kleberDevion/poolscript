# `s.replace(velho, novo, count=-1)`

Troca ocorrências; velho pode ser LISTA de alvos.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `velho` | str | list | — | lista troca cada um pelo mesmo novo |
| `novo` | str | — |  |
| `count` | int | -1 | máximo de trocas |

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
