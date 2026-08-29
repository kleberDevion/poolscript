# `max(lista) | max(a, b, ...)`

Maior valor de uma lista ou dos argumentos.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista | a, b...` | list ou 2+ valores | — |  |

## Retorno

valor

## Erros

- **ValueError** — a lista está vazia: `max() iterable argument is empty`
- **TypeError** — os itens não se comparam entre si: `'>' not supported between instances of 'str' and 'int'`
- **TypeError** — o argumento não itera: `'int' object is not iterable`

## Exemplos

```ps
post(max([3, 1]), max(2, 9, 4))
```

```saida
3 9
```

[← índice](../builtins.md)
