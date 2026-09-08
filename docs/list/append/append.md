# `l.append(item)`

Anexa UM item no fim da lista (muta).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `item` | qualquer | — | o valor a anexar; lista/dict entram como um único item |

## Retorno

Null — muta a lista

## Exemplos

```ps
l = [1, 2]
l.append(3)
post(l)
```

```saida
[1, 2, 3]
```

```ps
l = [1]
l.append([2, 3])
post(l, len(l))
```

```saida
[1, [2, 3]] 2
```

## Bordas

- anexa UM item — pra juntar outra lista item a item, use `extend`

[← índice](../list.md)
