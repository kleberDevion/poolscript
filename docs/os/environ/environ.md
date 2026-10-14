# `os.environ(key=Null)`

Devolve uma **variável de ambiente**, ou **todas** de uma vez se você não
passar chave.

```
os.environ(key: str = Null) -> str | dict | Null
```

| Argumento | Devolve |
|---|---|
| `os.environ("PATH")` | o valor daquela variável, ou `Null` se ela não existe |
| `os.environ()` | um dict com **todas** as variáveis de ambiente |

---

## Uso

```
import os

path = os.environ("PATH")           # uma variável

todas = os.environ()                # dict com tudo
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

Os dois veem o `.env` do mesmo jeito: antes de ler, carregam o `.env` que
acharem subindo a partir da pasta atual, sem passar por cima de variável que
já está no ambiente. Antes só o `getenv` carregava, e uma variável do `.env`
era `Null` no `environ` até alguém chamar o `getenv`.

---

## Relacionados

- [`os.getenv()`](../getenv/getenv.md) — uma variável com valor padrão
