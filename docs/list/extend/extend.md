# `l.extend(outra)`

Anexa TODOS os itens de outra sequência no fim (muta).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `outra` | list | tup | — | a sequência cujos itens entram |

## Retorno

Null — muta a lista

## Exemplos

```ps
l = [1]
l.extend([2, 3])
post(l)
```

```saida
[1, 2, 3]
```

```ps
l = [1]
l.extend((2, 3))
post(l)
```

```saida
[1, 2, 3]
```

[← índice](../list.md)
