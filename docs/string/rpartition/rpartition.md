# `s.rpartition(sep)`

Tupla (antes, sep, depois) na ÚLTIMA ocorrência.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sep` | str | — |  |

## Retorno

tup de 3

## Exemplos

![exemplo 1](../../assets/string__rpartition__rpartition_ex1.png)

<details><summary>código</summary>

```ps
post("a-b-c".rpartition("-"))
```

</details>

```saida
('a-b', '-', 'c')
```

[← índice](../string.md)
