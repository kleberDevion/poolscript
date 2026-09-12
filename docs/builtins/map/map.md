# `map(lista, fn)`

Nova lista com fn aplicada a cada item. A LISTA vem primeiro.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | tup | — |  |
| `fn` | funct | — | recebe 1 argumento |

## Retorno

list

## Erros

- **TypeError** — `fn` não é chamável: `'int' object is not callable`
- **TypeError** — o 1º argumento não é `list` nem `tup`: `map() argument 1 must be list or tup, not int`

`map(l, str)` **funciona** — `str` é chamável. O exemplo anterior citava isso como erro.

## Exemplos

```ps
funct dobro(x) { return x * 2 }
post(map([1, 2, 3], dobro))
```

```saida
[2, 4, 6]
```

## Bordas

- a ordem dos argumentos é **lista primeiro, função depois**

[← índice](../builtins.md)
