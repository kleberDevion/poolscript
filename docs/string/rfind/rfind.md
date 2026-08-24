# `s.rfind(sub, inicio=0, fim=null)`

Índice da ÚLTIMA ocorrência, ou -1.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sub` | str | — |  |
| `inicio` | int | 0 | posição (em caracteres) onde a faixa de busca começa |
| `fim` | int | null | onde a faixa termina (exclusivo); `null` = até o fim |

## Retorno

int

## Exemplos

```ps
post("banana".rfind("na"))
```

```saida
4
```

```ps
post("banana".rfind("na", 0, 4), "banana".rfind("na", 5))
```

```saida
2 -1
```

## Bordas

- a faixa `inicio`/`fim` limita ONDE procurar; a posição devolvida é sempre absoluta (do começo da string)

[← índice](../string.md)
