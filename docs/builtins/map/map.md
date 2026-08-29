# `map(lista, fn)`

Nova lista com fn aplicada a cada item. A LISTA vem primeiro.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `lista` | list | tup | — |  |
| `fn` | action | — | recebe 1 argumento |

## Retorno

list

## Erros

- **TypeError** — fn não é chamável (ex.: `map(l, str)` — nome nu de tipo não é função)

## Exemplos

```ps
action dobro(x) { return x * 2 }
post(map([1, 2, 3], dobro))
```

```saida
[2, 4, 6]
```

## Bordas

- a ordem dos argumentos é INVERSA à do Python: lista primeiro, função depois

[← índice](../builtins.md)
