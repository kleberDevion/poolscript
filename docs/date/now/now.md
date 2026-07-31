# `date.now()`

Devolve **data e hora** atuais no formato **ISO 8601** (`YYYY-MM-DD HH:MM:SS`).

```
date.now() -> str
```

---

## Uso

```
import date

post(date.now())           // "2026-07-25 14:30:05"
```

---

## Por que o formato ISO importa

No formato ISO (`ano-mês-dia`), datas em texto **ordenam corretamente** quando
comparadas como string — `"2026-07-25"` vem depois de `"2026-07-24"`
alfabeticamente, o que não acontece no formato BR (`25/07/2026`). Use `now()`
quando for guardar datas pra ordenar/comparar; use
[`datahora()`](../datahora/datahora.md) pra exibir pra pessoas.

> Apesar do nome, `now()` **não** é idêntico a `datahora()` — muda o formato
> (ISO vs BR). São a mesma informação apresentada diferente.

---

## Relacionados

- [`date.datahora()`](../datahora/datahora.md) — mesmo dado, formato brasileiro
- [`date.timestamp()`](../timestamp/timestamp.md) — como número (Unix)
