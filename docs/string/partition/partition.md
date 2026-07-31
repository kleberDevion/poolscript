# `s.partition(sep)`

Tupla (antes, sep, depois) na PRIMEIRA ocorrência.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sep` | str | — |  |

## Retorno

tup de 3

## Exemplos

```ps
post("a-b-c".partition("-"))
```

```saida
('a', '-', 'b-c')
```

## Bordas

- sem o separador: `(s, "", "")`

[← índice](../string.md)
