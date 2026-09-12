# `regex.findall(pattern, string)`

Acha **todas** as ocorrências do padrão e devolve uma **lista** com elas
(lista vazia se não achar nada).

```
regex.findall(pattern: str, string: str) -> list
```

---

## Uso

```
import regex

# todos os números
nums = regex.findall(r"\d+", "tenho 3 gatos, 2 cães e 10 peixes")
post(nums)          # ["3", "2", "10"]

# todas as palavras
palavras = regex.findall(r"\w+", "olá, mundo!")
post(palavras)      # ["olá", "mundo"]

# todos os e-mails de um texto
emails = regex.findall(r"[^@\s]+@[^@\s]+\.[^@\s]+", string)
```

---

## `findall` vs `search`

- **[`search`](../search/search.md)** — acha a **primeira** ocorrência (ou nada).
- **`findall`** — acha **todas**, numa lista.

Use `findall` pra extrair vários pedaços de uma vez.

---

## Relacionados

- [`regex.search()`](../search/search.md) — só a primeira
- [`regex.sub()`](../sub/sub.md) — substituir as ocorrências
