# `s.split(sep=Null, maxsplit=-1)`

Divide em lista; sem separador, divide por espaços.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sep` | str | Null | Null = qualquer branco, colapsando |
| `maxsplit` | int | -1 | máximo de divisões |

## Retorno

list

## Exemplos

```ps
post("a,b,c".split(","), " a  b ".split())
```

```saida
['a', 'b', 'c'] ['a', 'b']
```

[← índice](../string.md)
