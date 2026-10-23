# `DbCursor.close()`

Fecha o cursor. Chame ao terminar de usá-lo (a conexão continua aberta até você
fechar com [`DbConnection.close()`](../../DbConnection/close/close.md)).

```assinatura
cursor.close() -> Null
```

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
# a tabela dos exemplos, com algumas linhas
psodbc.query(base="loja.db", cmd="CREATE TABLE produtos (id INTEGER PRIMARY KEY, nome TEXT, preco REAL)")
psodbc.query(base="loja.db", cmd="INSERT INTO produtos (nome, preco) VALUES ('caneta', 2.5), ('caderno', 15.0), ('mochila', 120.0)")
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
