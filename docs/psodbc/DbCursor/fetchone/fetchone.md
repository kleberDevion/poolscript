# `DbCursor.fetchone()`

Devolve **uma** linha do resultado (a próxima), como um dict — ou `Null` se
não houver mais linhas.

```assinatura
cursor.fetchone() -> dict | Null
```

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
DbConnection conn = psodbc.connect(driver="sqlite", base="loja.db")
DbCursor cursor = conn.cursor()
```

---

## Uso

```ps
cursor.execute("SELECT * FROM users WHERE id = ?", (1,))
user = cursor.fetchone()

if (user is Null) {
    post("não achou")
} else {
    post(user["nome"])
}
```

Ideal quando você espera **um** resultado (busca por id, por exemplo) — evita
trazer uma lista pra pegar `[0]`.

---

## Relacionados

- [`.fetchall()`](../fetchall/fetchall.md) — todas as linhas
- [`.execute()`](../execute/execute.md) — rodar o SELECT
