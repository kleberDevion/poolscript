# `s.rfind(sub, inicio=0, fim=Null)`

Índice da ÚLTIMA ocorrência, ou -1.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sub` | str | — |  |
| `inicio` | int | 0 | posição (em caracteres) onde a faixa de busca começa |
| `fim` | int | `Null` | onde a faixa termina (exclusivo); `Null` = até o fim |

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

[← índice](../str.md)
