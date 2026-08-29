# `s.get(chave=Null)`

Interpreta como JSON de objeto e devolve o valor da chave.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `chave` | str | Null | sem chave, devolve o JSON inteiro |

## Retorno

valor | Null

## Erros

- **TypeError** — a `str` não contém um JSON válido

## Exemplos

```ps
post("{\"a\": 1}".get("a"))
```

```saida
1
```

[← índice](../string.md)
