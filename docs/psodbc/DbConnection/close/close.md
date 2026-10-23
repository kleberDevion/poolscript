# `DbConnection.close()`

Fecha a conexão com o banco. Chame ao terminar — libera o arquivo/rede.

```assinatura
conn.close() -> Null
```

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
# a tabela dos exemplos, com algumas linhas
psodbc.query(base="loja.db", cmd="CREATE TABLE produtos (id INTEGER PRIMARY KEY, nome TEXT, preco REAL)")
psodbc.query(base="loja.db", cmd="INSERT INTO produtos (nome, preco) VALUES ('caneta', 2.5), ('caderno', 15.0), ('mochila', 120.0)")
```

---

## Uso

```ps
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
