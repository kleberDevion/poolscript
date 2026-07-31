# `regex.match(padrão, texto, flags=0)`

Testa se o texto **começa** com o padrão. Devolve um objeto de correspondência
(truthy) se casar no início, ou `Null` se não.

```
regex.match(padrão: str, texto: str, flags=0)
```

---

## Uso

```
import regex

if (regex.match("Olá", "Olá mundo")) {
    post("começa com Olá")
}

// validar formato (o padrão precisa casar desde o início)
if (regex.match(r"\d{3}-\d{4}", "123-4567")) {
    post("formato válido")
}
```

---

## `match` vs `search`

- **`match`** — só casa se o padrão estiver **no começo** do texto.
- **[`search`](../search/search.md)** — casa em **qualquer lugar**.

```
regex.match("mundo", "Olá mundo")     // Null (não começa com "mundo")
regex.search("mundo", "Olá mundo")    // casa (achou no meio)
```

Pra **validar** algo (o texto inteiro tem que ter o formato), `match` costuma
ser o certo. Pra **encontrar** algo dentro, use `search`.

---

## Relacionados

- [`regex.search()`](../search/search.md) — achar em qualquer posição
- [`regex.findall()`](../findall/findall.md) — todas as ocorrências
