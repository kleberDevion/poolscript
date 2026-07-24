# `DbCursor` — executar SQL e ler resultados

O objeto que [`DbConnection.cursor()`](../DbConnection/cursor/cursor.md)
devolve. Por ele você **executa** comandos SQL e **lê** o que voltou.

---

## Métodos

| Método | O que faz | Página |
|---|---|---|
| `.execute(sql, params)` | executa um comando SQL | [execute/execute.md](execute/execute.md) |
| `.fetchall()` | pega **todas** as linhas do resultado | [fetchall/fetchall.md](fetchall/fetchall.md) |
| `.fetchone()` | pega **uma** linha | [fetchone/fetchone.md](fetchone/fetchone.md) |
| `.fetchmany(size)` | pega **N** linhas | [fetchmany/fetchmany.md](fetchmany/fetchmany.md) |
| `.rowcount()` | quantas linhas foram afetadas | [rowcount/rowcount.md](rowcount/rowcount.md) |
| `.close()` | fecha o cursor | [close/close.md](close/close.md) |

---

## Fluxo

```
cursor = conn.cursor()
cursor.execute("SELECT * FROM produtos WHERE preco > ?", (100,))
caros = cursor.fetchall()      // lista de dicts
for each p in caros {
    post(p["nome"], p["preco"])
}
```

`.execute()` roda o comando; os `.fetch*()` leem o resultado de um `SELECT`.

---

## Relacionados

- [`DbConnection`](../DbConnection/DbConnection.md) — quem cria o cursor
- [`.execute()`](execute/execute.md) — o ponto de partida
