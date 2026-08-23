# `len(x)`

Tamanho de string (em caracteres), lista, tupla, dict ou bytes.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | str | list | tup | dict | bytes | — |  |

## Retorno

int

## Erros

- **SomeValueUnexpected** — tipo sem tamanho (int, float, bool, Null)

## Exemplos

![exemplo 1](../../assets/builtins__len__len_ex1.png)

<details><summary>código</summary>

```ps
post(len("olá"), len([1, 2]), len({"a": 1}))
```

</details>

```saida
3 2 1
```

## Bordas

- string conta CARACTERES unicode, não bytes — `len("ç")` é 1

[← índice](../builtins.md)
