# `s.index(sub, inicio=0, fim=null)`

Como find, mas ERRA quando não acha.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sub` | str | — |  |
| `inicio` | int | 0 | posição (em caracteres) onde começa a procurar |
| `fim` | int | null | onde para (exclusivo); `null` = até o fim |

## Retorno

int

## Erros

- **SomeValueUnexpected** — substring ausente (na faixa dada)

## Exemplos

```ps
post("banana".index("na"))
```

```saida
2
```

```ps
post("banana".index("na", 3))
```

```saida
4
```

[← índice](../string.md)
