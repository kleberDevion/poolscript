# `s.split(sep=Null, max=-1)`

Divide em lista; sem separador, divide por espaços.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sep` | str | Null | Null = qualquer branco, colapsando |
| `max` | int | -1 | máximo de divisões |

## Retorno

list

## Exemplos

![exemplo 1](../../assets/string__split__split_ex1.png)

<details><summary>código</summary>

```ps
post("a,b,c".split(","), " a  b ".split())
```

</details>

```saida
['a', 'b', 'c'] ['a', 'b']
```

[← índice](../string.md)
