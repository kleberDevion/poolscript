# `Parsing.Arrayformatt(value, to_type="list")`

Converte um valor pra **lista**.

```
Parsing.Arrayformatt(value, to_type="list") -> list
```

`to_type` existe por uniformidade com as outras funções do `Parsing` — aqui
**não muda nada**: o resultado é sempre `list`.

---

## Uso

```
Parsing.Arrayformatt((1, 2, 3))      // [1, 2, 3]   (tupla → lista)
Parsing.Arrayformatt("abc")          // ["a", "b", "c"]  (string → caracteres)
Parsing.Arrayformatt({"a": 1, "b": 2}) // ["a", "b"]  (dict → chaves)
Parsing.Arrayformatt(42)             // [42]        (valor solto → lista de 1)
```

---

## Relacionados

- [`Parsing.Tuplasformatt()`](../Tuplasformatt/Tuplasformatt.md) — pra tupla
- [visão geral do Parsing](../Parsing.md)
