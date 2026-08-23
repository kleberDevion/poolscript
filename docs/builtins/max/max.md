# `max(lista) | max(a, b, ...)`

Maior valor de uma lista ou dos argumentos.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista | a, b...` | list ou 2+ valores | — |  |

## Retorno

valor

## Erros

- **SomeValueUnexpected** — lista vazia ou tipos não comparáveis

## Exemplos

![exemplo 1](../../assets/builtins__max__max_ex1.png)

<details><summary>código</summary>

```ps
post(max([3, 1]), max(2, 9, 4))
```

</details>

```saida
3 9
```

[← índice](../builtins.md)
