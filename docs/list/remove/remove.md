# `l.remove(item)`

Remove a PRIMEIRA ocorrência do item (muta).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `item` | qualquer | — | o valor a remover |

## Retorno

Null — muta a lista

## Exemplos

```ps
l = [1, 2, 1]
l.remove(1)
post(l)
```

```saida
[2, 1]
```

## Bordas

- remove por VALOR, não por posição — pra posição use `pop(i)`

[← índice](../list.md)
