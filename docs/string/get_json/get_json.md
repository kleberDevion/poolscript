# `s.get_json()`

Interpreta a string como JSON e devolve o valor.

## Retorno

dict | list | valor

## Erros

- **SomeValueUnexpected** — JSON inválido

## Exemplos

![exemplo 1](../../assets/string__get_json__get_json_ex1.png)

<details><summary>código</summary>

```ps
post("{\"a\": 1}".get_json())
```

</details>

```saida
{'a': 1}
```

## Bordas

- JSON estrito, como o módulo json

[← índice](../string.md)
