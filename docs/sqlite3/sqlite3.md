# sqlite3 — Banco SQLite embutido

Banco SQL **local, num arquivo** (ou em memória), sem servidor: o `sqlite3` já
vem embutido, sem instalar nada. O caminho é `connect` → `cursor` → `execute`
→ `fetch`.

```
import sqlite3
```

> Para Postgres/MySQL/SQL Server/Mongo (banco em rede), use [`psodbc`](../psodbc/psodbc.md).
> O `sqlite3` é pro banco que mora num arquivo do seu projeto.

---

## `sqlite3.connect(arquivo)`

Abre (e **cria, se não existir**) o banco. Devolve uma [`Connection`](#connection).

```
conn = sqlite3.connect("app.db")     # arquivo em disco
conn = sqlite3.connect(":memory:")   # banco só na memória (some ao fechar)
```

---

## Connection

O que `connect()` devolve.

| Método | Faz |
|---|---|
| `.cursor()` | cria um [`Cursor`](#cursor) novo |
| `.execute(sql, params=Null)` | atalho: roda o SQL num cursor novo e o devolve (não precisa criar cursor à mão) |
| `.commit()` | grava as mudanças pendentes (INSERT/UPDATE/DELETE). Devolve a própria conexão. |
| `.rollback()` | desfaz as mudanças desde o último commit. Devolve a própria conexão. |
| `.close()` | fecha a conexão |

> **Precisa de `commit()`** depois de INSERT/UPDATE/DELETE, senão a mudança não
> persiste no arquivo. SELECT não precisa.

### `with` — commit e close automáticos

Dentro de um `with`, ao sair a conexão faz **commit + close** sozinha:

```
with sqlite3.connect("app.db") as conn:
    conn.execute("INSERT INTO users (nome) VALUES (?)", ("Ana",))
# aqui já commitou e fechou
```

---

## Cursor

Vem de `conn.cursor()` (ou é o retorno de `conn.execute(...)`).

| Método | Faz |
|---|---|
| `.execute(sql, params=Null)` | roda um SQL. Devolve o **próprio cursor** para consultas; devolve **`Null`** para DDL (`CREATE`/`DROP`/`ALTER`/`PRAGMA`/`ATTACH`/`DETACH`/`VACUUM`). |
| `.executemany(sql, seq)` | roda o mesmo SQL para cada item da sequência (ex: vários INSERT de uma vez) |
| `.fetchall()` | **lista de dicts** (`coluna → valor`) com todas as linhas do último SELECT |
| `.fetchone()` | um dict com a próxima linha, ou `Null` se acabou |
| `.fetchmany(n=1)` | lista de até `n` linhas (dicts) |
| `.rowcount` | quantas linhas o último INSERT/UPDATE/DELETE afetou |
| `.lastrowid` | o id (rowid) da última linha inserida |
| `.close()` | fecha o cursor |

> **As linhas voltam como `dict`** (nome da coluna → valor), não como tupla: você
> acessa `linha["nome"]`. `SELECT *` traz todas as colunas nomeadas.

### Parâmetros — sempre com `?`

Nunca cole valores na string SQL (injeção). Use `?` e passe uma **tupla**:

```
cur.execute("SELECT * FROM users WHERE idade > ?", (18,))
```

---

## Exemplo completo (roda inteiro)

```ps
import sqlite3

conn = sqlite3.connect(":memory:")
conn.execute("CREATE TABLE users (id INTEGER PRIMARY KEY, nome TEXT, idade INT)")

conn.execute("INSERT INTO users (nome, idade) VALUES (?, ?)", ("Ana", 30))
conn.execute("INSERT INTO users (nome, idade) VALUES (?, ?)", ("Beto", 25))
conn.commit()

cur = conn.execute("SELECT nome, idade FROM users ORDER BY idade")
post(cur.fetchall())
conn.close()
```

```saida
[{'nome': 'Beto', 'idade': 25}, {'nome': 'Ana', 'idade': 30}]
```

### `fetchone` e `lastrowid`

```ps
import sqlite3

conn = sqlite3.connect(":memory:")
conn.execute("CREATE TABLE t (id INTEGER PRIMARY KEY, v TEXT)")
c = conn.execute("INSERT INTO t (v) VALUES (?)", ("x",))
post(c.lastrowid)
post(conn.execute("SELECT v FROM t").fetchone())
conn.close()
```

```saida
1
{'v': 'x'}
```

---

## Relacionados

- [`psodbc`](../psodbc/psodbc.md) — bancos em rede (Postgres, MySQL, SQL Server, Mongo)
- [`os`](../os/os.md) — mexer no arquivo `.db` (mover, apagar, tamanho)
