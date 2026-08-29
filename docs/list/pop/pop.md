# `l.pop(i=-1)`

Remove e DEVOLVE o item da posição `i` (o último, por padrão).

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `i` | int | -1 | posição a remover; negativo conta do fim |

## Retorno

o item removido

## Exemplos

```ps
l = [1, 2, 3]
post(l.pop(), l)
```

```saida
3 [1, 2]
```

```ps
l = [1, 2, 3]
post(l.pop(0), l)
```

```saida
1 [2, 3]
```

## Erros

- **IndexError** — a lista está vazia: `pop from empty list`
- **IndexError** — o índice está fora da faixa: `pop index out of range`
- **TypeError** — o índice não é inteiro: `'str' object cannot be interpreted as an integer`

## Bordas

- é o único mutador que devolve algo útil

[← índice](../list.md)
