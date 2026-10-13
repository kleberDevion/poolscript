# `date.hora(hours=0, minutes=0, days=0)`

Converte uma **duração** (horas, minutos, dias) no total de **segundos**.
Não é "que horas são" — é "quantos segundos tem em tanto tempo". Serve pra
somar com [`date.timestamp()`](../timestamp/timestamp.md).

```
date.hora(hours=0, minutes=0, days=0) -> int
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `hours` | `0` | quantidade de horas |
| `minutes` | `0` | quantidade de minutos |
| `days` | `0` | quantidade de dias |

---

## Uso

```
import date

date.hora(hours=1)         # 3600    (1 hora em segundos)
date.hora(minutes=30)      # 1800    (30 minutos)
date.hora(days=1)          # 86400   (1 dia)
date.hora(hours=2, minutes=30)   # 9000  (2h30 = 2*3600 + 30*60)
```

---

## O uso principal: expiração relativa

Somado ao timestamp atual, dá "daqui a X tempo" — o padrão pro `exp` de um JWT:

```
import date
import jwt

# token que expira em 7 dias
exp = date.timestamp() + date.hora(days=7)
token = jwt.gen({"user_id": 1, "exp": exp}, "chave")
```

---

## Erros

- **TypeError** — horas, minutos ou dias que não são inteiros:
  `date.hora(1.5)` → `'flo' object cannot be interpreted as an integer`
  (o `flo` era truncado calado: 1.5 hora dava 3600). Uma hora e meia é
  `date.hora(1, 30)`. `bool` vale 0/1, e a conta é exata com inteiro de
  qualquer tamanho.

---

## Relacionados

- [`date.timestamp()`](../timestamp/timestamp.md) — o "agora" em segundos, pra somar
- lib `jwt` — onde a soma vira o campo `exp`
