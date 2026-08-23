# `min(lista) | min(a, b, ...)`

Menor valor de uma lista ou dos argumentos.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista | a, b...` | list ou 2+ valores | — |  |

## Retorno

valor

## Erros

- **SomeValueUnexpected** — lista vazia ou tipos não comparáveis

## Exemplos

![exemplo 1](../../assets/builtins__min__min_ex1.png)

<details><summary>código</summary>

```ps
post(min([3, 1]), min(5, 2, 8))
```

</details>

```saida
1 2
```

[← índice](../builtins.md)
