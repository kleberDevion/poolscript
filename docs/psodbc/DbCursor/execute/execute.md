# `DbCursor.execute(sql, params=())`

Executa um comando SQL. Para valores dinâmicos, use `?` e passe uma tupla em
`params` — **nunca** concatene valores na string (risco de SQL injection).

```assinatura
cursor.execute(sql: str, params: tuple = ()) -> DbCursor
```

Os exemplos desta página pressupõem (o `request` é o da
[jinker](../../../jinker/request/request.md), de onde vem o valor digitado por
quem usa):

```ps contexto
import psodbc
from jinker import request
# as tabelas dos exemplos, com algumas linhas
psodbc.query(base="loja.db", cmd="CREATE TABLE produtos (id INTEGER PRIMARY KEY, nome TEXT, preco REAL)")
psodbc.query(base="loja.db", cmd="INSERT INTO produtos (nome, preco) VALUES ('caneta', 2.5), ('caderno', 15.0), ('mochila', 120.0)")
psodbc.query(base="loja.db", cmd="CREATE TABLE users (id INTEGER PRIMARY KEY, nome TEXT, idade INT, ativo INT)")
psodbc.query(base="loja.db", cmd="INSERT INTO users (nome, idade, ativo) VALUES ('Ana', 30, 1), ('Beto', 25, 1), ('Caio', 17, 0)")
psodbc.query(base="loja.db", cmd="CREATE TABLE logs (id INTEGER PRIMARY KEY, mensagem TEXT)")
psodbc.query(base="loja.db", cmd="INSERT INTO logs (mensagem) VALUES ('inicio'), ('login'), ('compra'), ('logout')")
psodbc.query(base="loja.db", cmd="CREATE TABLE clientes (id INTEGER PRIMARY KEY, nome TEXT, ativo INT)")
psodbc.query(base="loja.db", cmd="INSERT INTO clientes (nome, ativo) VALUES ('Loja A', 1), ('Loja B', 0)")
DbConnection conn = psodbc.connect(driver="sqlite", base="loja.db")
DbCursor cursor = conn.cursor()
```

---

## Uso

```ps
cursor = conn.cursor()

# SELECT
cursor.execute("SELECT * FROM produtos")
dados = cursor.fetchall()

# com parâmetros (o jeito seguro)
cursor.execute("SELECT * FROM produtos WHERE preco > ?", (100,))
caros = cursor.fetchall()

# INSERT (lembre do commit)
cursor.execute("INSERT INTO produtos (nome, preco) VALUES (?, ?)", ("café", 15))
conn.commit()
```

---

## Sempre use `?` para valores

```ps
nome = request.get("nome")

# CERTO — parâmetro:
cursor.execute("SELECT * FROM users WHERE nome = ?", (nome,))

# ERRADO — concatenar permite SQL injection:
cursor.execute("SELECT * FROM users WHERE nome = '" + nome + "'")
```

O `?` deixa o driver escapar o valor com segurança. Repare que `params` é uma
**tupla** — pra um único valor use `(valor,)` (com a vírgula). Lista também
serve.

## O parâmetro vai com o tipo dele

`int`, `flo`, `bool`, texto e `Null` chegam ao banco como número, número, booleano,
texto e NULL — em sqlite, postgres e mysql:

```ps
cursor.execute("SELECT * FROM logs ORDER BY id LIMIT ? OFFSET ?", (20, 40))
cursor.execute("SELECT * FROM clientes WHERE ativo = ?", (True,))
```

No postgres, quem decide o tipo final é o servidor, pelo contexto: um `flo`
comparado com coluna `INTEGER` é recusado (`invalid input syntax for type
integer`), e `? IS NULL` sem outro uso do parâmetro não tem tipo de onde tirar.

---

## Relacionados

- [`.fetchall()`](../fetchall/fetchall.md) — ler o resultado de um SELECT
- [`DbConnection.commit()`](../../DbConnection/commit/commit.md) — salvar INSERT/UPDATE/DELETE
