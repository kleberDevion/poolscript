# `MailReader.body(id)`

Busca e decodifica o **corpo** de um e-mail específico, pelo `id` que veio no
[`.search()`](../search/search.md).

```
r.body(id: str) -> str
```

---

## Uso

```
emails = r.search("ALL", limit=20)     # sem corpo (leve)

for each e in emails {
    if (e["subject"] == "relatório") {
        texto = r.body(e["id"])         # pega o corpo só desse
        post(texto)
    }
}
```

---

## Por que existe (em vez de sempre trazer o corpo)

Trazer o corpo de **todos** os e-mails no `search` é pesado. O padrão é o
`search` trazer só os cabeçalhos (from/subject/date) — rápido — e você pedir o
corpo com `.body(id)` só dos que interessam.

> Se você **sabe** que vai querer o corpo de todos, passe `include_body=true`
> direto no [`.search()`](../search/search.md) e pule o `.body()`.

---

## Relacionados

- [`.search()`](../search/search.md) — de onde vem o `id`
- [`.close()`](../close/close.md) — fechar ao terminar
