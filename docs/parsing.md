# Parsing — Dados Transientes e Conversão de Tipo

Conversões de tipo "seguras" — não quebram com lixo na string, e mantém o
valor original intacto (não muta a variável de entrada).

`Parsing` é um **builtin global** — não precisa de `import`, já está
disponível em qualquer script:

```
post(Parsing.integer("R$ 1.234"))   # 1234
```

---

## Parsing.string(value, to_type="str")

Remove caracteres não textuais e colapsa espaços. Com `to_type="int"` ou
`"flo"`, delega pra `integer()`/`floating()`.

```
Parsing.string("  ana   souza  ")   # "ana souza"
```

---

## Parsing.integer(value, to_type="int")

Converte pra inteiro. Float **trunca** (não arredonda): `123.7 → 123`.
Em string, extrai só os dígitos antes do primeiro ponto.

```
Parsing.integer(123.7)        # 123
Parsing.integer("R$ 1.234")   # 1234
```

---

## Parsing.floating(value, to_type="flo")

Converte pra float. Suporta separador decimal brasileiro:

```
Parsing.floating("1.299,90")   # 1299.9
Parsing.floating("29,90")      # 29.9
```

---

## Parsing.boolean(value, to_type="bool")

```
Parsing.boolean("")        # False
Parsing.boolean("0")       # False
Parsing.boolean("false")   # False
Parsing.boolean("null")    # False
Parsing.boolean("sim")     # True — qualquer outra string não-vazia
```

---

## Parsing.TransientValue(value, to_type="str")

Conversão genérica preservando o valor original sem distorção — escolhe
entre `integer()`/`floating()`/string conforme `to_type`.

```
v = Parsing.TransientValue("29,90", "flo")
post(v + 10)   # 39.9 — aritmética funciona direto no TransientValue
```

---

## Parsing.JSONformatt(value, to_type="json")

Converte pra `dict`/`list`. String tenta parse JSON (devolve `{}` se
falhar); `dict`/`list` retornam como estão.

```
Parsing.JSONformatt('{"a": 1}')   # {"a": 1}
```

---

## Parsing.Arrayformatt(value, to_type="list")

Converte pra lista: `tuple`/`set` → `list`; `str` → lista de caracteres;
`dict` → lista das chaves; qualquer outro valor vira `[value]`.

```
Parsing.Arrayformatt("abc")         # ["a", "b", "c"]
Parsing.Arrayformatt({"a": 1})      # ["a"]
```

---

## Parsing.Tuplasformatt(value, to_type="tup")

Converte pra tupla, mesma lógica do `Arrayformatt`.

---

## TransientValue — o objeto devolvido pelas conversões numéricas

`integer()`, `floating()` e `TransientValue()` devolvem um `TransientValue`,
não um `int`/`float` puro. Ele se comporta como o número original em
aritmética, comparação e `str()`/`int()`/`float()`/`bool()`, mas guarda o
`origin_type` (útil pra `.type()`):

```
v = Parsing.integer("42")
post(v + 8)        # 50
post(v.type())      # "int"
post(v == 42)        # True
```
