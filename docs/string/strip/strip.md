# `s.strip(chars=Null)`

Remove espaços (ou os chars dados) das duas pontas.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `chars` | str | Null | conjunto de caracteres a remover |

## Retorno

str

## Exemplos

![exemplo 1](../../assets/string__strip__strip_ex1.png)

<details><summary>código</summary>

```ps
post("  oi  ".strip() + "!")
```

</details>

```saida
oi!
```

![exemplo 2](../../assets/string__strip__strip_ex2.png)

<details><summary>código</summary>

```ps
post("xxoixx".strip("x"))
```

</details>

```saida
oi
```

[← índice](../string.md)
