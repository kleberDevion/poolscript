# `s.get_json()`

Interpreta a string como JSON e devolve o valor.

## Retorno

dict | list | valor

## Erros

Nenhum. `str` que não contém JSON válido devolve `null`, com rc 0 — não
levanta. A seção dizia `TypeError`, e nunca houve.

## Exemplos

```ps
post("{\"a\": 1}".get_json())
```

```saida
{'a': 1}
```

## Bordas

- JSON estrito, como o módulo json

[← índice](../string.md)
