# `s.get(chave=Null)`

Interpreta como JSON de objeto e devolve o valor da chave.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `chave` | str | Null | sem chave, devolve o JSON inteiro |

## Retorno

valor | Null

## Erros

Nenhum. `str` que não contém JSON válido devolve `null`, com rc 0 — não
levanta. A seção dizia `TypeError`, e nunca houve.

## Exemplos

```ps
post("{\"a\": 1}".get("a"))
```

```saida
1
```

[← índice](../string.md)
