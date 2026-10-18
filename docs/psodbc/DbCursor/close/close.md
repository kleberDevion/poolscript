# `DbCursor.close()`

Fecha o cursor. Chame ao terminar de usá-lo (a conexão continua aberta até você
fechar com [`DbConnection.close()`](../../DbConnection/close/close.md)).

```assinatura
cursor.close() -> Null
```

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
DbConnection conn = psodbc.connect(driver="sqlite", base="loja.db")
```

---

## Uso

```ps
cursor = conn.cursor()
cursor.execute("SELECT * FROM produtos")
dados = cursor.fetchall()
cursor.close()         # fecha o cursor
conn.close()           # fecha a conexão
```

---

## Relacionados

- [`DbConnection.close()`](../../DbConnection/close/close.md) — fecha a conexão inteira
