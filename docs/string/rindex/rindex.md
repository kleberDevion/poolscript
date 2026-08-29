# `s.rindex(sub, inicio=0, fim=null)`

Como rfind, mas ERRA quando não acha.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sub` | str | — |  |
| `inicio` | int | 0 | posição (em caracteres) onde a faixa de busca começa |
| `fim` | int | null | onde a faixa termina (exclusivo); `null` = até o fim |

## Retorno

int

## Erros

- **ValueError** — a subcadeia não aparece no trecho pedido

## Exemplos

```ps
post("banana".rindex("na"))
```

```saida
4
```

```ps
post("banana".rindex("na", 0, 4))
```

```saida
2
```

[← índice](../string.md)
