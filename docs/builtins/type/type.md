# `type(x)`

Nome do tipo do valor, como string.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | qualquer | — |  |

## Retorno

str

## Exemplos

![exemplo 1](../../assets/builtins__type__type_ex1.png)

<details><summary>código</summary>

```ps
post(type(1), type("a"), type([1]), type(Null))
```

</details>

```saida
int str list Null
```

## Bordas

- igual ao método universal `x.type()` — os dois respondem o mesmo nome

[← índice](../builtins.md)
