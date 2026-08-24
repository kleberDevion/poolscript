# `d.update(outro)`

Mescla os pares de outro dict; chave repetida é sobrescrita (muta).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `outro` | dict | — | os pares a mesclar |

## Retorno

null — muta o dict

## Exemplos

```ps
d = { "a": 1 }
d.update({ "b": 2 })
post(d)
```

```saida
{'a': 1, 'b': 2}
```

```ps
d = { "a": 1 }
d.update({ "a": 9 })
post(d)
```

```saida
{'a': 9}
```

[← índice](../dict.md)
