# `d.has(chave)`

A chave existe? (o mesmo que `chave in d`)

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `chave` | qualquer | — | a chave a testar |

## Retorno

bool

## Exemplos

```ps
d = { "a": 1 }
post(d.has("a"), d.has("z"), "a" in d)
```

```saida
True False True
```

## Bordas

- testa CHAVE — pro valor, `x in d.value()`

[← índice](../dict.md)
