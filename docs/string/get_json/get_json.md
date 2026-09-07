# `s.get_json(chave=Null)`

Interpreta a string como JSON e devolve o valor. Com `chave`, devolve só o
valor dessa chave.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `chave` | str | Null | sem ela, devolve o objeto inteiro |

## Retorno

dict | list | valor

## Erros

Nenhum. `str` que não contém JSON válido devolve `Null`, com rc 0 — não
levanta. Chave inexistente também devolve `Null`.

## Exemplos

```ps
post("{\"a\": 1}".get_json())
post("{\"a\": 1}".get_json("a"))
post("{\"a\": 1}".get_json("zzz"))
```

```saida
{'a': 1}
1
Null
```

## Bordas

- JSON estrito, como o módulo json

[← índice](../string.md)
