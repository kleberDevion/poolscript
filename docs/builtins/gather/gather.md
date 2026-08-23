# `gather(a, b, ...)`

Espera vários `async action` de uma vez, **rodando-os concorrentes**, e devolve os resultados numa lista, na ordem dos argumentos. Argumento que não é future passa direto. Aceita também uma **lista** de futures (`gather(fs)`).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `a, b...` | future/qualquer | — | futures de `async action`; valor comum passa direto |

## Retorno

list — os valores resolvidos, na ordem dos argumentos.

## Exemplos

```ps
async action dobro(n):
    sleep(0.2)
    return n * 2

post(gather(dobro(1), dobro(2), dobro(3)))
```

```saida
[2, 4, 6]
```

```ps
async action dobro(n):
    sleep(0.2)
    return n * 2

fs = []
for each i in range(3):
    addEnd(fs, dobro(i))
post(gather(fs))
```

```saida
[[0, 2, 4]]
```

## Bordas

- Funciona **nos dois motores** (interpretador e VM em C): resolve os futures rodando as tasks concorrentes. Valor comum (não-future) volta como está.
- `gather(fs)` com uma lista devolve **lista dentro de lista** (um resultado por argumento; o argumento-lista vira a sub-lista dos seus valores).

[← índice](../builtins.md)
