# `Parsing.JSONformatt(value, to_type="json")`

Converte um valor pra **dict/lista** (JSON). String vira JSON parseado; dict/
lista voltam como estão.

```assinatura
Parsing.JSONformatt(value, to_type="json") -> dict | list
```

`to_type` existe por uniformidade com as outras funções do `Parsing` — aqui
**não muda nada**.

---

## Uso

```ps
import Parsing

post(Parsing.JSONformatt("{\"a\": 1}"), Parsing.JSONformatt("[1, 2]"), Parsing.JSONformatt({"a": 1}))
post(Parsing.JSONformatt("texto inválido"), Parsing.JSONformatt("123"))
```

```saida
{'a': 1} [1, 2] {'a': 1}
{} {}
```

Diferente de [`json.parse`](../../json/parse/parse.md), não quebra com JSON
inválido — devolve um dict vazio. O JSON válido que não é objeto nem lista
(`"123"`, `"true"`) também dá o dict vazio: a função devolve `dict | list`, e
antes um `dict d = Parsing.JSONformatt("123")` passava no `--check` e quebrava
rodando.

---

## Relacionados

- [`json.parse()`](../../json/parse/parse.md) — parse estrito (erro se inválido)
- [`Parsing.Arrayformatt()`](../Arrayformatt/Arrayformatt.md) — pra lista
