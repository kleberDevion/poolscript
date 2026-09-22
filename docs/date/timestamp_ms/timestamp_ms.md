# `date.timestamp_ms()`

Devolve o **timestamp Unix em milissegundos** — o mesmo relógio do
[`date.timestamp()`](../timestamp/timestamp.md), com mil vezes mais resolução.
É um **inteiro**.

```
date.timestamp_ms() -> int
```

---

## Uso

```
import date

agora = date.timestamp_ms()   # ex: 1785000000123
post(agora // 1000)           # o mesmo que date.timestamp()
```

---

## Quando usar este e quando usar o `timestamp()`

| precisa de | use |
|---|---|
| carimbar um registro, `exp` de token, comparar datas | [`timestamp()`](../timestamp/timestamp.md) (segundos) |
| carimbar um evento que acontece várias vezes por segundo (log, fila, telemetria) | `timestamp_ms()` |
| **medir quanto durou** alguma coisa | [`monotonic()`](../monotonic/monotonic.md) |

Pra medir duração, prefira o `monotonic()`: o relógio do sistema pode ser
acertado no meio da medição (NTP, mudança de fuso) e andar pra trás.

---

## Relacionados

- [`date.timestamp()`](../timestamp/timestamp.md) — o mesmo instante, em segundos
- [`date.monotonic()`](../monotonic/monotonic.md) — pra medir duração
- [`date.hora()`](../hora/hora.md) — duração em segundos, pra somar ao timestamp
