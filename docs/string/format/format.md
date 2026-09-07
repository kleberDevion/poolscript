# `s.format(a, b)`

Preenche `{}` e `{0}` no template, **por posição**.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `a` | qualquer | Null | posicional |
| `b` | qualquer | Null | posicional |

Chamar sem argumento nenhum funciona (`"x".format()` devolve `"x"`).

## Retorno

str

## Erros

- **IndexError** — o índice citado no formato não existe:
  `"{2}".format(1)` → `Replacement index 2 out of range for positional args tuple`
- **KeyError** — o formato cita um **nome**: `"{x}".format(1)` → `'x'`
- **TypeError** — argumento nomeado: `"{nome}".format(nome="ana")` →
  `'nome' is an invalid keyword argument for format()`

## Exemplos

```ps
post("{} e {}".format(1, "x"))
```

```saida
1 e x
```

## Bordas

- **`{nome}` não se preenche por aqui.** Keyword é recusada e dict posicional
  dá `KeyError`. Para nomeados existe [`format_map`](../format_map/format_map.md):
  `"{nome}".format_map({"nome": "ana"})`.
- `Null` formata como `Null`: `"{}".format(Null)` é `"Null"`.

[← índice](../string.md)
