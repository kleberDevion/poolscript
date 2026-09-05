# `date.datahora()`

Devolve **data e hora** atuais juntas, formato brasileiro
`DD/MM/YYYY HH:MM:SS`.

```
date.datahora() -> str
```

---

## Uso

```
import date

post(date.datahora())      # "25/07/2026 14:30:05"

# carimbar um registro no banco
msg = {"texto": "olá", "criado_em": date.datahora()}
```

---

## `datahora()` vs `now()`

As duas mostram data + hora, em **formatos diferentes**:

| Função | Formato | Exemplo |
|---|---|---|
| `datahora()` | brasileiro | `25/07/2026 14:30:05` |
| [`now()`](../now/now.md) | ISO 8601 | `2026-07-25 14:30:05` |

Use `datahora()` pra exibir pra pessoas (formato BR); use `now()` quando
precisar do formato ISO (padrão internacional, ordena/compara melhor como
string).

---

## Relacionados

- [`date.now()`](../now/now.md) — mesmo dado, formato ISO
- [`date.today()`](../today/today.md) / [`date.time()`](../time/time.md) — separados
