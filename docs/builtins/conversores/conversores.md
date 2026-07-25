# Conversores: `str(x)`, `int(x)`, `flo(x)`, `bool(x)`

Convertem um valor de um tipo pra outro. São **estritos** — se a conversão não
faz sentido, dá erro (diferente do [`Parsing`](../../Parsing/Parsing.md), que é
tolerante).

---

## `str(x)` — pra texto

```
str(42)            // "42"
str(3.14)          // "3.14"
str(true)          // "true"
```

## `int(x)` — pra inteiro

```
int("25")          // 25
int(3.9)           // 3     (trunca)
int("abc")         // ERRO  (não é número limpo)
```

## `flo(x)` — pra decimal

```
flo("3.14")        // 3.14
flo(10)            // 10.0
flo("3,14")        // ERRO  (espera ponto, não vírgula)
```

## `bool(x)` — pra booleano

```
bool(1)            // true
bool(0)            // false
bool("")           // false  (vazio)
bool("texto")      // true
```

---

## Conversores vs `Parsing`

- **`int("abc")`** — **erro** (estrito).
- **[`Parsing.integer("abc")`](../../Parsing/integer/integer.md)** — `0`
  (tolerante).

Use os conversores builtin quando você **espera** valor limpo e quer erro se
não for. Use `Parsing` pra entrada suja/incerta.

---

## Relacionados

- [`type()`](../type/type.md) — descobrir o tipo atual
- [`Parsing`](../../Parsing/Parsing.md) — conversão tolerante
