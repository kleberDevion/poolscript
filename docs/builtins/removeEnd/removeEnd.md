# `removeEnd(lista)`

Remove e devolve o ÚLTIMO item da lista.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | — | mutada in-place |

## Retorno

o item removido

## Erros

- **SomeValueUnexpected** — lista vazia ou não-lista

## Exemplos

![exemplo 1](../../assets/builtins__removeEnd__removeEnd_ex1.png)

<details><summary>código</summary>

```ps
l = [1, 2]
post(removeEnd(l), l)
```

</details>

```saida
2 [1]
```

[← índice](../builtins.md)
