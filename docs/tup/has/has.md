# `t.has(item)`

Apelido de [`contains`](../contains/contains.md) — as duas grafias existem e
fazem exatamente a mesma coisa.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `item` | qualquer | — | o valor a procurar |

## Retorno

bool — `True` se achou, `False` se não.

## Erros

- **TypeError** — sem argumento:
  `contains() takes exactly one argument (0 given)`. A mensagem cita
  **`contains`**, não `has`: `contains` é o nome canônico, e é ele que o motor
  usa ao reclamar de argumento faltando.
- **TypeError** — argumento demais: `has() takes at most 1 argument (2 given)`.
  Aqui o nome que aparece é o que você **escreveu**.

## Exemplos

```ps
t = (10, 20, 30, 20)
post(t.has(10), t.has(99))
post(t.has(10) == t.contains(10))
```

```saida
True False
True
```

## Bordas

- `has` e `contains` são a **mesma função** na tabela `METODOS_TUPLA`: as duas
  entradas apontam pra `met_l_contains`. Não há diferença de retorno, de
  desempenho nem de comparação — só de grafia
- em `dict` a relação é invertida: lá o canônico é `has` e o apelido é
  `contains`

[← índice](../tup.md)
