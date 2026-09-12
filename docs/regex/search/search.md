# `regex.search(pattern, string)`

Procura o padrão em **qualquer posição** do texto. Devolve **`bool`**: `true`
se achou em algum lugar, `false` se não.

```
regex.search(pattern: str, string: str) -> bool
```

Pra pegar **o que** casou (e não só se casou), use
[`findall`](../findall/findall.md).

---

## Uso

```
import regex

if (regex.search("erro", "log: um erro aconteceu")) {
    post("achou 'erro' no texto")
}

# tem algum número no texto?
if (regex.search(r"\d", "abc123")) {
    post("tem dígito")
}
```

---

## `search` vs `match`

- **`search`** — acha em **qualquer lugar** do texto.
- **[`match`](../match/match.md)** — só casa no **começo**.

Use `search` pra "existe isso em algum ponto?"; use `match` pra "o texto tem
esse formato desde o início?".

---

## Relacionados

- [`regex.match()`](../match/match.md) — só no início
- [`regex.findall()`](../findall/findall.md) — todas as ocorrências, não só a 1ª
