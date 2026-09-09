# `t.count(item)`

Quantas vezes o item aparece na tupla.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `item` | qualquer | — | o valor a contar |

## Retorno

int — a contagem. Nunca erra: item ausente devolve `0`.

## Erros

- **TypeError** — argumento a mais ou a menos:
  `count() takes exactly one argument (0 given)`.

## Exemplos

```ps
t = (10, 20, 30, 20)
post(t.count(20), t.count(99))
post((1, 1.0, True).count(1))
post(t.count("20"))
```

```saida
2 0
3
0
```

## Bordas

- a comparação é por **valor**, e número é número: `1`, `1.0` e `True` contam
  como o mesmo item — daí o `3` do exemplo
- texto e número **não** se misturam: `"20"` não conta o `20`
- varre a tupla inteira, do começo ao fim; não há `inicio`/`fim` como no
  [`index`](../index/index.md)

[← índice](../tup.md)
