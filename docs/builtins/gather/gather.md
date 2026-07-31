# `gather(a, b, ...)`

Devolve os argumentos como lista.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `a, b...` | qualquer | — |  |

## Retorno

list

## Exemplos

```ps
post(gather(1, "a", true))
```

```saida
[1, 'a', True]
```

## Bordas

- no interpretador espera PoolFutures de `async action`; com valores comuns devolve a lista como está — na VM (sem async) é sempre isso

[← índice](../builtins.md)
