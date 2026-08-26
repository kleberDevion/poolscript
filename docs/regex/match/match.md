# `regex.match(pattern, string, flags=0)`

Testa se o texto **inteiro** casa com o padrão. Devolve **`bool`** — `true` se
o padrão consome a string do começo ao fim, `false` caso contrário.

```
regex.match(pattern: str, string: str, flags=0) -> bool
```

> **Atenção, quem vem do Python:** aqui `match` é o **`fullmatch`** do `re`,
> não o `re.match`. O padrão precisa casar a string **toda** — não só o começo.
> O nome `fullmatch` também existe e faz exatamente o mesmo:
> [`regex.fullmatch()`](../fullmatch/fullmatch.md). Pra "casa em algum lugar",
> use [`search`](../search/search.md).

---

## Uso

```
import regex

if (regex.match("\\d{3}-\\d{4}", "123-4567")) {
    post("formato válido")           // casa a string inteira
}
```

```
regex.match("Olá", "Olá mundo")      // false — sobrou " mundo"
regex.match("Olá.*", "Olá mundo")    // true  — o .* consome o resto
regex.match("mundo", "Olá mundo")    // false
```

---

## `match` vs `search`

| | `match` | `search` |
|---|---|---|
| onde casa | a string **inteira** | **qualquer** posição |
| devolve | `bool` | `bool` |
| serve pra | **validar** um formato | **encontrar** algo dentro |

```
regex.match("mundo", "Olá mundo")     // false
regex.search("mundo", "Olá mundo")    // true
```

Pra validar (CPF, e-mail, data), `match` é o certo: garante que não sobrou
lixo nas pontas.

---

## Padrão reusado: compile

Num laço, [`regex.compile()`](../compile/compile.md) compila uma vez só:

```
p = regex.compile("\\d{3}-\\d{4}")
for each t in telefones {
    if (p.match(t)) { post(t) }
}
```

---

## Relacionados

- [`regex.fullmatch()`](../fullmatch/fullmatch.md) — o mesmo, com o nome do Python
- [`regex.search()`](../search/search.md) — achar em qualquer posição
- [`regex.findall()`](../findall/findall.md) — todas as ocorrências
- [`regex.compile()`](../compile/compile.md) — padrão compilado e reusável
