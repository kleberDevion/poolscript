# `min(lista) | min(a, b, ...)`

Menor valor de uma lista ou dos argumentos.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista | a, b...` | list ou 2+ valores | — |  |

## Retorno

valor

## Erros

- **ValueError** — a lista está vazia: `min() iterable argument is empty`
- **TypeError** — os itens não se comparam entre si: `'>' not supported between instances of 'str' and 'int'`
- **TypeError** — o argumento não itera: `'int' object is not iterable`

## Exemplos

```ps
post(min([3, 1]), min(5, 2, 8))
```

```saida
1 2
```

[← índice](../builtins.md)
