# `t.len()`

Quantidade de itens da tupla — forma de método do builtin `len`.

## Parâmetros

Nenhum.

## Retorno

int — a quantidade de itens do primeiro nível.

## Erros

- **TypeError** — qualquer argumento: `len() takes no arguments (1 given)`.

## Exemplos

```ps
t = (10, 20, 30, 20)
post(t.len(), len(t))
post((7,).len())
post(().len())
```

```saida
4 4
1
0
```

## Bordas

- `t.len()` e `len(t)` devolvem o mesmo `int`
- conta itens, não profundidade: `((1, 2), (3, 4)).len()` é `2`
- a tupla de um item é `(7,)`, com a vírgula — `(7)` é só o número entre
  parênteses

[← índice](../tup.md)
