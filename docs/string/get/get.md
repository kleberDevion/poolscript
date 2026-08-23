# `s.get(chave)`

Interpreta como JSON de objeto e devolve o valor da chave.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `chave` | str | — |  |

## Retorno

valor | Null

## Erros

- **SomeValueUnexpected** — JSON inválido

## Exemplos

![exemplo 1](../../assets/string__get__get_ex1.png)

<details><summary>código</summary>

```ps
post("{\"a\": 1}".get("a"))
```

</details>

```saida
1
```

[← índice](../string.md)
