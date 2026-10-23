# `DbCursor.fetchall()`

Devolve **todas** as linhas do resultado do último `SELECT`, como uma
**lista de dicts** (cada linha = um dict com as colunas).

```assinatura
cursor.fetchall() -> list
```

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
# a tabela dos exemplos, com algumas linhas
psodbc.query(base="loja.db", cmd="CREATE TABLE produtos (id INTEGER PRIMARY KEY, nome TEXT, preco REAL)")
psodbc.query(base="loja.db", cmd="INSERT INTO produtos (nome, preco) VALUES ('caneta', 2.5), ('caderno', 15.0), ('mochila', 120.0)")
DbConnection conn = psodbc.connect(driver="sqlite", base="loja.db")
DbCursor cursor = conn.cursor()
```

---

## Uso

```ps
cursor.execute("SELECT id, nome FROM produtos")
todos = cursor.fetchall()
# [{"id": 1, "nome": "café"}, {"id": 2, "nome": "chá"}]

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
