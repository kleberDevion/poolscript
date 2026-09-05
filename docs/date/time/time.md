# `date.time()`

Devolve a **hora atual** como string no formato `HH:MM:SS` (24 horas).

```
date.time() -> str
```

---

## Uso

```
import date

post(date.time())          # "14:30:05"

# carimbar um log
post(f"[{date.time()}] servidor iniciado")
```

Usa o horário **local** da máquina.

---

## Relacionados

- [`date.today()`](../today/today.md) — só a data
- [`date.datahora()`](../datahora/datahora.md) — data + hora juntas
