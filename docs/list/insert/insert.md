# `l.insert(i, item)`

Insere `item` NA posição `i`, empurrando o resto (muta).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `i` | int | — | posição onde o item passa a ficar |
| `item` | qualquer | — | o valor a inserir |

## Retorno

Null — muta a lista

## Exemplos

```ps
l = ["a", "c"]
l.insert(1, "b")
post(l)
```

```saida
['a', 'b', 'c']
```

```ps
l = [1, 2]
l.insert(0, 0)
post(l)
```

```saida
[0, 1, 2]
```

## Bordas

- índice além do fim não erra: o item vai pro fim

[← índice](../list.md)
