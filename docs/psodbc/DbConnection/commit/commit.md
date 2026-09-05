# `DbConnection.commit()`

Confirma (salva no disco) as alterações feitas na conexão — INSERT, UPDATE,
DELETE. **Sem `commit`, as mudanças se perdem** ao fechar.

```
conn.commit() -> None
```

---

## Uso

```
cursor.execute("INSERT INTO produtos (nome, preco) VALUES (?, ?)", ("café", 15))
conn.commit()          # agora sim a inserção está salva
```

Precisa de `commit` depois de **cada** operação que altera dados (ou de um
grupo delas). `SELECT` não precisa — só leitura não muda nada.

> **SQL Server** já abre com `autocommit` ligado (ver
> [connect](../../connect/connect.md)), então lá o `commit` costuma ser
> dispensável. Nos outros bancos, é necessário.

---

## Relacionados

- [`DbCursor.execute()`](../../DbCursor/execute/execute.md) — o que precisa ser confirmado
- [`.close()`](../close/close.md) — fechar depois
