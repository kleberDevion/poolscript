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

![exemplo 1](../../assets/string__replace__replace_ex1.png)

<details><summary>código</summary>

```ps
post("banana".replace("na", "NA", 1))
```

</details>

```saida
baNAna
```

![exemplo 2](../../assets/string__replace__replace_ex2.png)

<details><summary>código</summary>

```ps
post("a-b_c".replace(["-", "_"], "."))
```

</details>

```saida
a.b.c
```

## Bordas

- a forma com lista é extensão da PoolScript

[← índice](../string.md)
