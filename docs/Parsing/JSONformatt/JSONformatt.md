# `Parsing.JSONformatt(value, to_type="json")`

Converte um valor pra **dict/lista** (JSON). String vira JSON parseado; dict/
lista voltam como estão.

```
Parsing.JSONformatt(value, to_type="json") -> dict | list
```

`to_type` existe por uniformidade com as outras funções do `Parsing` — aqui
**não muda nada**.

---

## Uso

```
Parsing.JSONformatt('{"a": 1}')      # {"a": 1}   (parseia a string)
Parsing.JSONformatt({"a": 1})        # {"a": 1}   (já é dict)
Parsing.JSONformatt("texto inválido") # {}        (não deu → dict vazio)
```

Diferente de [`json.parse`](../../json/parse/parse.md), não quebra com JSON
inválido — devolve um dict vazio.

---

## Relacionados

- [`json.parse()`](../../json/parse/parse.md) — parse estrito (erro se inválido)
- [`Parsing.Arrayformatt()`](../Arrayformatt/Arrayformatt.md) — pra lista
