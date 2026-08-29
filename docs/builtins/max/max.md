# `max(lista) | max(a, b, ...)`

Maior valor de uma lista ou dos argumentos.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista | a, b...` | list ou 2+ valores | — |  |

## Retorno

valor

## Erros

- **ValueError** — a lista está vazia, ou os itens não se comparam entre si

## Exemplos

```ps
post(max([3, 1]), max(2, 9, 4))
```

```saida
3 9
```

[← índice](../builtins.md)
