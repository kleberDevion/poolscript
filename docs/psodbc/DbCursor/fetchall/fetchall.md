# `DbCursor.fetchall()`

Devolve **todas** as linhas do resultado do último `SELECT`, como uma
**lista de dicts** (cada linha = um dict com as colunas).

```
cursor.fetchall() -> list
```

---

## Uso

```
cursor.execute("SELECT id, nome FROM produtos")
todos = cursor.fetchall()
// [{"id": 1, "nome": "café"}, {"id": 2, "nome": "chá"}]

for each p in todos {
    post(p["id"], p["nome"])
}
```

Se o SELECT não retornou nada, devolve uma **lista vazia** `[]`.

---

## `fetchall` vs `fetchone` vs `fetchmany`

- **`fetchall`** — todas as linhas de uma vez (bom pra resultados pequenos).
- **[`fetchone`](../fetchone/fetchone.md)** — só a próxima linha.
- **[`fetchmany`](../fetchmany/fetchmany.md)** — N linhas por vez (resultados grandes).

---

## Relacionados

- [`.execute()`](../execute/execute.md) — rodar o SELECT antes
- [`.fetchone()`](../fetchone/fetchone.md) — uma linha só
