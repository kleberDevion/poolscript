# `sleep(segundos)`

Pausa a execução por um tempo. Útil pra dar intervalo entre operações ou
dentro de `async action`.

```
sleep(segundos) -> None
```

---

## Uso

```
post("começando")
sleep(2)                // espera 2 segundos
post("depois de 2s")

sleep(0.5)              // aceita frações de segundo
```

---

## Relacionados

- [`gather()`](../gather/gather.md) — aguardar várias tarefas async
- `async action` — ver `LANGUAGE.md`
