# `Parsing.integer(value, to_type="int")`

Converte um valor pra **inteiro**, de forma tolerante: acha o número dentro do
texto e trunca a fração (não arredonda).

```assinatura
Parsing.integer(value, to_type="int") -> int
```

`to_type` existe por uniformidade com as outras funções do `Parsing` — aqui
**não muda nada**: o resultado é sempre `int`.

---

## Uso

```ps
import Parsing

post(Parsing.integer("12 unidades"), Parsing.integer("R$ 1.234"), Parsing.integer("R$ 19,90"), Parsing.integer("12.7"))
post(Parsing.integer("-5"), Parsing.integer(123.7), Parsing.integer("abc"), Parsing.integer(True))
post(Parsing.integer("99999999999999999999"), Parsing.integer("1e5"))
```

```saida
12 1234 19 12
-5 123 0 1
99999999999999999999 100000
```

O número no texto segue a regra de [O número dentro do texto](../Parsing.md#o-número-dentro-do-texto):
o primeiro trecho de dígitos, com o sinal colado antes, o milhar e o decimal do
formato brasileiro e o expoente. Daí sai a parte inteira, de qualquer tamanho
(`"99999999999999999999"` não trava em 9223372036854775807, como travava).

Valor que não é texto:

| Valor | Resultado |
|---|---|
| `int` | ele mesmo |
| `flo` | truncado pra zero (`123.7` é 123, `-2.9` é -2), de qualquer tamanho; `nan` e infinito dão `0` |
| `bool` | `1` ou `0` |
| outro (lista, dict...) | o número do texto dele (`[7, 8]` é 7) |

---

## `Parsing.integer` vs `int`

- **`int("abc")`** — erro (texto não é número limpo).
- **`Parsing.integer("abc")`** — `0` (tolerante, não quebra).

Use `int()` quando quer o erro; `Parsing.integer` quando quer o melhor esforço
sem quebrar.

---

## Relacionados

- [`Parsing.floating()`](../floating/floating.md) — pra decimais
- builtin `int()` — conversão estrita
