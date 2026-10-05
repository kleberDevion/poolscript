# `s.count(sub, inicio=0, fim=Null)`

Quantas ocorrências (sem sobreposição) da substring.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sub` | str | — |  |
| `inicio` | int | 0 | posição (em caracteres) onde começa a contar |
| `fim` | int | `Null` | onde para (exclusivo); `Null` = até o fim |

## Retorno

int

## Exemplos

```ps
post("banana".count("na"))
```

```saida
2
```

```ps
post("banana".count("na", 3), "banana".count("a", 1, 4))
```

```saida
1 2
```

## Bordas

- substring vazia conta `len + 1` — uma "posição" entre cada caractere

[← índice](../str.md)
