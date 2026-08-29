# `s.get_json()`

Interpreta a string como JSON e devolve o valor.

## Retorno

dict | list | valor

## Erros

- **TypeError** — a `str` não contém um JSON válido

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
