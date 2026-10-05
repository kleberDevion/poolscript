# `regex.match(pattern, string)`

Testa se o texto **inteiro** casa com o padrão. Devolve **`bool`** — `True` se
o padrão consome a string do começo ao fim, `False` caso contrário.

```
regex.match(pattern: str, string: str) -> bool
```

> **Atenção:** `match` casa a string **toda**, não só o começo.
> O nome `fullmatch` também existe e faz exatamente o mesmo:
> [`regex.fullmatch()`](../fullmatch/fullmatch.md). Pra "casa em algum lugar",
> use [`search`](../search/search.md).

---

## Uso

```
import regex

if (regex.match("\\d{3}-\\d{4}", "123-4567")) {
    post("formato válido")           # casa a string inteira
}
```

```
regex.match("Olá", "Olá mundo")      # False — sobrou " mundo"
regex.match("Olá.*", "Olá mundo")    # True  — o .* consome o resto
regex.match("mundo", "Olá mundo")    # False
```

---

## `match` vs `search`

| | `match` | `search` |
|---|---|---|
| onde casa | a string **inteira** | **qualquer** posição |
| devolve | `bool` | `bool` |
| serve pra | **validar** um formato | **encontrar** algo dentro |

```
regex.match("mundo", "Olá mundo")     # False
regex.search("mundo", "Olá mundo")    # True
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

- [`regex.fullmatch()`](../fullmatch/fullmatch.md) — o mesmo, com o nome explícito
- [`regex.search()`](../search/search.md) — achar em qualquer posição
- [`regex.findall()`](../findall/findall.md) — todas as ocorrências
- [`regex.compile()`](../compile/compile.md) — padrão compilado e reusável
