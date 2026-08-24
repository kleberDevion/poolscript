# `s.find(sub, inicio=0, fim=null)`

Índice da primeira ocorrência, ou -1.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `sub` | str | — |  |
| `inicio` | int | 0 | posição (em caracteres) onde começa a procurar; negativo conta do fim |
| `fim` | int | null | posição onde para (exclusivo); `null` = até o fim |

## Retorno

int

## Exemplos

```ps
post("banana".find("na"), "banana".find("xyz"))
```

```saida
2 -1
```

```ps
post("banana".find("na", 3), "banana".find("na", 0, 3))
```

```saida
4 -1
```

```ps
post("pão de mel".find("o"), "pão de mel".find("e", -3))
```

```saida
2 8
```

## Bordas

- posições são em CARACTERES, não bytes — acento conta como 1 (`"pão".find("o")` → 2)
- `inicio` além do tamanho devolve -1 (não erra)

[← índice](../string.md)
