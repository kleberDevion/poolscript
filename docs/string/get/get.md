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

```ps
post("{\"a\": 1}".get("a"))
```

```saida
1
```

[← índice](../string.md)
