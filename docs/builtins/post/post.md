# `post(v1, v2, ...)`

Imprime os valores no stdout, separados por espaço, com quebra de linha no final.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `v...` | qualquer | — | zero ou mais valores |

## Retorno

Null

## Exemplos

![exemplo 1](../../assets/builtins__post__post_ex1.png)

<details><summary>código</summary>

```ps
post(1, "a", true)
```

</details>

```saida
1 a True
```

![exemplo 2](../../assets/builtins__post__post_ex2.png)

<details><summary>código</summary>

```ps
post([1, Null], {"k": 2})
```

</details>

```saida
[1, null] {'k': 2}
```

## Bordas

- `post()` sem argumento nenhum não imprime nada — nem a quebra de linha; linha em branco é `post("")`
- `Null` imprime `null`, inclusive aninhado (`[Null]` → `[null]`)

[← índice](../builtins.md)
