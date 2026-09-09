# `t.contains(item)`

O item está na tupla? (o mesmo que `item in t`)

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `item` | qualquer | — | o valor a procurar |

## Retorno

bool — `True` se achou, `False` se não. Nunca erra por item ausente, ao
contrário do [`index`](../index/index.md).

## Erros

- **TypeError** — sem argumento:
  `contains() takes exactly one argument (0 given)`.

## Exemplos

```ps
t = (10, 20, 30, 20)
post(t.contains(30), t.contains(99))
post(30 in t, 99 in t)
post((10, 20).contains(10.0))
post(((1, 2), (3, 4)).contains((3, 4)))
```

```saida
True False
True False
True
True
```

## Bordas

- `contains` é o nome **canônico**; [`has`](../has/has.md) é apelido do mesmo
  método (as duas entradas da tabela `METODOS_TUPLA` apontam pra
  `met_l_contains`)
- a comparação é por **valor**, não por identidade: `10.0` acha o `10`, e uma
  tupla aninhada é achada por outra tupla de conteúdo igual
- busca **rasa**: só olha os itens do primeiro nível

[← índice](../tup.md)
