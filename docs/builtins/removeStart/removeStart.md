# `removeStart(lista)`

Remove e devolve o PRIMEIRO item da lista.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | — | mutada in-place |

## Retorno

o item removido

## Erros

- **SomeValueUnexpected** — lista vazia ou não-lista

## Exemplos

![exemplo 1](../../assets/builtins__removeStart__removeStart_ex1.png)

<details><summary>código</summary>

```ps
l = [1, 2]
post(removeStart(l), l)
```

</details>

```saida
1 [2]
```

[← índice](../builtins.md)
