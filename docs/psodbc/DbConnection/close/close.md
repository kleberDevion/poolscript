# `DbConnection.close()`

Fecha a conexão com o banco. Chame ao terminar — libera o arquivo/rede.

```
conn.close() -> None
```

---

## Uso

```
conn = psodbc.connect(driver="sqlite", base="loja.db")
cursor = conn.cursor()
cursor.execute("SELECT * FROM produtos")
dados = cursor.fetchall()
conn.close()           # encerra
```

> Se você fez alterações (INSERT/UPDATE/DELETE), chame
> [`.commit()`](../commit/commit.md) **antes** de fechar — senão as mudanças
> não salvas são descartadas.

---

## Relacionados

- [`.commit()`](../commit/commit.md) — salvar antes de fechar
- [`DbConnection`](../DbConnection.md) — visão geral
