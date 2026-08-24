# `l.contains(item)`

O item está na lista? (o mesmo que `item in l`)

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `item` | qualquer | — | o valor a procurar |

## Retorno

bool

## Exemplos

```ps
l = [1, 2]
post(l.contains(2), l.contains(9), 2 in l)
```

```saida
True False True
```

[← índice](../list.md)
