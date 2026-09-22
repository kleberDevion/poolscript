# `date.monotonic()`

Devolve o relógio **monotônico** em segundos, com fração — um **flo**. Serve
pra **medir duração**: ele nunca anda pra trás, mesmo que o relógio do sistema
seja acertado no meio da medição.

```
date.monotonic() -> flo
```

O valor em si não significa nada (não é data, não é hora): o que vale é a
**diferença** entre duas leituras.

---

## Uso

```
import date

inicio = date.monotonic()
sleep(0.35)
levou = date.monotonic() - inicio

post("levou", round(levou, 3), "s")     # levou 0.35 s
```

Em milissegundos, pra mostrar como um build mostra:

```
ms = int((date.monotonic() - inicio) * 1000)
post("terminou em " + str(ms) + "ms")
```

---

## Por que não usar o `timestamp()` pra isso

[`date.timestamp()`](../timestamp/timestamp.md) é o relógio do **sistema**, em
segundos inteiros: uma operação de 350 ms mede `0` ou `1`, e se o NTP acertar a
hora no meio, a conta pode até dar negativa. O monotônico não tem nenhum dos
dois problemas — é a mesma base que o motor usa por dentro pro escalonador e
pros prazos do jinker.

---

## Relacionados

- [`date.timestamp_ms()`](../timestamp_ms/timestamp_ms.md) — o relógio do sistema, em milissegundos
- [`date.timestamp()`](../timestamp/timestamp.md) — o relógio do sistema, em segundos
