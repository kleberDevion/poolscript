# `s.replace(old, new, count)`

Troca ocorrências; `old` pode ser LISTA de alvos.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `old` | str \| list | — | lista: cada alvo é trocado |
| `new` | str \| list | — | `str`: todo alvo vira ele; `list` (com `old` lista): em paralelo, o i-ésimo alvo vira o i-ésimo novo |
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

```ps
post("A e B".replace(["A", "B"], ["x", "y"]))
```

```saida
x e y
```

## Bordas

- a forma com lista é extensão da PoolScript

[← índice](../str.md)
