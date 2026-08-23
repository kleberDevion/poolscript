# `s.partition(sep)`

Tupla (antes, sep, depois) na PRIMEIRA ocorrência.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sep` | str | — |  |

## Retorno

tup de 3

## Exemplos

![exemplo 1](../../assets/string__partition__partition_ex1.png)

<details><summary>código</summary>

```ps
post("a-b-c".partition("-"))
```

</details>

```saida
('a', '-', 'b-c')
```

## Bordas

- sem o separador: `(s, "", "")`

[← índice](../string.md)
