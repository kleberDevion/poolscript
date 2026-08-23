# `enumerate(lista)`

Lista de tuplas (indice, item), começando em 0.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | tup | str | — |  |

## Retorno

list de tup

## Erros

- **SomeValueUnexpected** — tipo não-iterável

## Exemplos

![exemplo 1](../../assets/builtins__enumerate__enumerate_ex1.png)

<details><summary>código</summary>

```ps
post(enumerate(["a", "b"]))
```

</details>

```saida
[(0, 'a'), (1, 'b')]
```

[← índice](../builtins.md)
