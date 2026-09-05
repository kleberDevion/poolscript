# `date.timestamp()`

Devolve o **timestamp Unix** atual — o número de segundos desde 1º de janeiro
de 1970. É um **inteiro**, ideal pra cálculos com tempo.

```
date.timestamp() -> int
```

---

## Uso

```
import date

agora = date.timestamp()   # ex: 1785000000
```

---

## O uso mais comum: expiração de token JWT

Timestamp + [`date.hora(...)`](../hora/hora.md) dá "o instante daqui a X tempo",
que é o que um token JWT usa no campo `exp`:

```
import date
import jwt

payload = {
    "user_id": 42,
    "exp": date.timestamp() + date.hora(hours=24)   # expira em 24h
}
token = jwt.gen(payload, "minha_chave")
```

---

## Medindo duração

Como é número, dá pra subtrair pra saber quanto tempo passou:

```
inicio = date.timestamp()
# ... trabalho demorado ...
fim = date.timestamp()
post(f"levou {fim - inicio} segundos")
```

---

## Relacionados

- [`date.hora()`](../hora/hora.md) — converter horas/minutos/dias em segundos
- [`date.now()`](../now/now.md) — data/hora como texto legível
- lib `jwt` — onde `exp` é usado
