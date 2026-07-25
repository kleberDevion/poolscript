# `type(x)`

Devolve o **tipo** de um valor, como texto.

```
type(x) -> str
```

---

## Uso

```
type(42)           // "int"
type(3.14)         // "flo"
type("texto")      // "str"
type(true)         // "bool"
type([1, 2])       // "list"
type({"a": 1})     // "dict"
type((1, 2))       // "tup"
type(Null)         // "Null"
```

---

## Uso comum: checar antes de usar

```
if (type(valor) == "list") {
    for each x in valor {
        post(x)
    }
}
```

---

## Relacionados

- [conversores](../conversores/conversores.md) — `str`/`int`/`flo`/`bool`
- `is` (operador) — checar tipo/identidade, ver `LANGUAGE.md`
