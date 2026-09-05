# `date.today()`

Devolve a **data atual** como string no formato brasileiro `DD/MM/YYYY`.

```
date.today() -> str
```

---

## Uso

```
import date

post(date.today())         # "25/07/2026"

nome_arquivo = f"relatorio_{date.today()}.csv"   # relatorio_25/07/2026.csv
```

> Repare que o formato tem `/` — se for usar em nome de arquivo, troque por `-`
> (barra não é permitida em nomes): `date.today().replace("/", "-")`.

---

## Relacionados

- [`date.time()`](../time/time.md) — só a hora
- [`date.datahora()`](../datahora/datahora.md) — data + hora
- [`date.now()`](../now/now.md) — data + hora no formato ISO
