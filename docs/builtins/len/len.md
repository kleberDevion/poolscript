# `len(x)`

Tamanho de string (em caracteres), lista, tupla, dict ou bytes.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | str | list | tup | dict | bytes | — |  |

## Retorno

int

## Erros

- **TypeError** — `int`, `flo` e `bool` não têm tamanho: `object of type 'int' has no len()`

`len(null)` **não** é erro: devolve `0`.

## Exemplos

```ps
post(len("olá"), len([1, 2]), len({"a": 1}))
```

```saida
3 2 1
```

## Bordas

- string conta CARACTERES unicode, não bytes — `len("ç")` é 1

[← índice](../builtins.md)
