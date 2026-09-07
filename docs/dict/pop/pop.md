# `d.pop(chave, default=Null)`

Remove a chave e DEVOLVE o valor dela (muta).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `chave` | qualquer | — | a chave a remover |
| `default` | qualquer | Null | devolvido quando a chave não existe |

## Retorno

o valor removido, ou o `default` quando a chave não existe

## Erros

- **KeyError** — a chave não existe **e nenhum `default` foi passado**. Com
  `default`, não há erro.

## Exemplos

```ps
d = { "a": 1, "b": 2 }
post(d.pop("a"))
post(d)
post(d.pop("z", 0))
```

```saida
1
{'b': 2}
0
```

[← índice](../dict.md)
