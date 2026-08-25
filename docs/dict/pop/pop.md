# `d.pop(chave, default=Null)`

Remove a chave e DEVOLVE o valor dela (muta).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `chave` | qualquer | — | a chave a remover |

## Retorno

o valor removido

## Erros

- **KeyError** — a chave não existe

## Exemplos

```ps
d = { "a": 1, "b": 2 }
post(d.pop("a"))
post(d)
```

```saida
1
{'b': 2}
```

[← índice](../dict.md)
