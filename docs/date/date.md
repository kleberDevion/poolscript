# date — Data e hora

Lib pra pegar a **data e hora atuais** em vários formatos, o **timestamp** Unix,
e calcular **durações em segundos** (útil pra expiração de tokens JWT).

```
import date
```

Todas as funções usam o horário **local** da máquina.

---

## Referência

| Membro | Devolve | Exemplo de saída | Página |
|---|---|---|---|
| `date.time()` | hora atual | `"14:30:05"` | [time/time.md](time/time.md) |
| `date.today()` | data atual (BR) | `"25/07/2026"` | [today/today.md](today/today.md) |
| `date.datahora()` | data + hora (BR) | `"25/07/2026 14:30:05"` | [datahora/datahora.md](datahora/datahora.md) |
| `date.now()` | data + hora (ISO) | `"2026-07-25 14:30:05"` | [now/now.md](now/now.md) |
| `date.timestamp()` | timestamp Unix (int) | `1785000000` | [timestamp/timestamp.md](timestamp/timestamp.md) |
| `date.hora(h, m, d)` | duração em segundos (int) | `86400` | [hora/hora.md](hora/hora.md) |

> As três primeiras devolvem **string** (bom pra exibir/gravar). `timestamp` e
> `hora` devolvem **int** (bom pra cálculo). `datahora` e `now` mostram a mesma
> informação em formatos diferentes (BR vs ISO).

---

## Exemplo rápido

```
import date

post(date.today())        // 25/07/2026
post(date.time())         // 14:30:05
post(date.datahora())     // 25/07/2026 14:30:05

// carimbar um registro
registro = {"texto": "olá", "quando": date.datahora()}

// expiração de token daqui a 24h (ver lib jwt)
exp = date.timestamp() + date.hora(hours=24)
```
