# `gather(f1, f2, ...)`

Aguarda **vários** resultados de `async action` de uma vez, devolvendo a lista
com todos os valores quando todos terminam.

```
gather(futuro1, futuro2, ...) -> list
```

---

## Uso

```
// dispara três tarefas async ao mesmo tempo
a = buscaDados("api1")      // async action → começa já
b = buscaDados("api2")
c = buscaDados("api3")

// espera as três terminarem
resultados = gather(a, b, c)
post(resultados)            // [resultado1, resultado2, resultado3]
```

As tarefas rodam **em paralelo**; `gather` espera todas e junta os resultados —
mais rápido que esperar uma de cada vez.

---

## Relacionados

- `async action` / `await` — ver `LANGUAGE.md`
- [`sleep()`](../sleep/sleep.md) — pausar
