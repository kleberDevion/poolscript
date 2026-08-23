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

```ps
post(min([3, 1]), min(5, 2, 8))
```

```saida
1 2
```

[← índice](../builtins.md)
