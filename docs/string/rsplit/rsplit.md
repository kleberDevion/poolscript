# `s.rsplit(sep=Null, max=-1)`

Como split, mas conta as divisões da DIREITA.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sep` | str | Null |  |
| `max` | int | -1 |  |

## Retorno

list

## Exemplos

![exemplo 1](../../assets/string__rsplit__rsplit_ex1.png)

<details><summary>código</summary>

```ps
post("a,b,c".rsplit(",", 1))
```

</details>

```saida
['a,b', 'c']
```

[← índice](../string.md)
