# `os.environ(chave=None)`

Devolve uma **variável de ambiente**, ou **todas** de uma vez se você não
passar chave.

```
os.environ(chave: str = None) -> str | dict
```

| Argumento | Devolve |
|---|---|
| `os.environ("PATH")` | o valor daquela variável |
| `os.environ()` | um dict com **todas** as variáveis de ambiente |

---

## Uso

```
import os

path = os.environ("PATH")           // uma variável

todas = os.environ()                // dict com tudo
for each chave in todas.keys() {
    post(chave)
}
```

---

## `environ` vs `getenv`

- **[`os.getenv(chave, default)`](../getenv/getenv.md)** — uma variável, com
  valor padrão se não existir. É o que você usa no dia a dia.
- **`os.environ()`** — pega **todas** de uma vez (útil pra debug/inspeção).

Pra ler uma config específica, prefira `getenv` (tem o `default`).

---

## Relacionados

- [`os.getenv()`](../getenv/getenv.md) — uma variável com valor padrão
