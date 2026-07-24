# `DbCursor.execute(sql, params=())`

Executa um comando SQL. Para valores dinâmicos, use `?` e passe uma tupla em
`params` — **nunca** concatene valores na string (risco de SQL injection).

```
cursor.execute(sql: str, params: tuple = ()) -> DbCursor
```

---

## Uso

```
cursor = conn.cursor()

// SELECT
cursor.execute("SELECT * FROM produtos")
dados = cursor.fetchall()

// com parâmetros (o jeito seguro)
cursor.execute("SELECT * FROM produtos WHERE preco > ?", (100,))
caros = cursor.fetchall()

// INSERT (lembre do commit)
cursor.execute("INSERT INTO produtos (nome, preco) VALUES (?, ?)", ("café", 15))
conn.commit()
```

---

## Sempre use `?` para valores

```
nome = request.get("nome")

// CERTO — parâmetro:
cursor.execute("SELECT * FROM users WHERE nome = ?", (nome,))

// ERRADO — concatenar permite SQL injection:
cursor.execute("SELECT * FROM users WHERE nome = '" + nome + "'")
```

O `?` deixa o driver escapar o valor com segurança. Repare que `params` é uma
**tupla** — pra um único valor use `(valor,)` (com a vírgula).

---

## Relacionados

- [`.fetchall()`](../fetchall/fetchall.md) — ler o resultado de um SELECT
- [`DbConnection.commit()`](../../DbConnection/commit/commit.md) — salvar INSERT/UPDATE/DELETE
