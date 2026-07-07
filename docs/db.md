# db — Banco de Dados

Suporta SQLite, PostgreSQL, MySQL e MongoDB.

---

## SQLite

### Modo convencional

```
import db

conn = db.query("meu_banco.db")
cursor = conn.cursor()
cursor.execute("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, nome TEXT, email TEXT)")
cursor.execute("INSERT INTO users (nome, email) VALUES (?, ?)", ("ana", "ana@email.com"))
conn.commit()

cursor.execute("SELECT * FROM users")
result = getAll()
conn.close()

post(result)
# [{"id": 1, "nome": "ana", "email": "ana@email.com"}]
```

### Modo curto

```
import db

# SELECT
result = db.query(
    base="meu_banco.db",
    cmd="SELECT * FROM @t",
    table="users"
)
post(result)

# INSERT com parâmetros
db.query(
    base="meu_banco.db",
    cmd=("INSERT INTO @t (nome, email) VALUES (?, ?)", ("leo", "leo@email.com")),
    table="users"
)

# UPDATE
db.query(
    base="meu_banco.db",
    cmd=("UPDATE @t SET email = ? WHERE nome = ?", ("novo@email.com", "leo")),
    table="users"
)

# DELETE
db.query(
    base="meu_banco.db",
    cmd=("DELETE FROM @t WHERE nome = ?", ("leo",)),
    table="users"
)
```

O `@t` é substituído pelo valor de `table`. O `cmd` pode ser:
- String simples: `"SELECT * FROM users"`
- Tupla com parâmetros: `("SELECT * FROM @t WHERE nome = ?", ("joao",))`

### Retornos do modo curto

| Operação | Retorno |
|---|---|
| SELECT com resultado | Lista de dicts |
| SELECT sem resultado | `None` |
| INSERT/UPDATE/DELETE ok | `None` |
| Erro | String com mensagem de erro |

---

## getAll()

Builtin global — retorna resultado do último `cursor.execute()`:

```
cursor.execute("SELECT id, nome, email FROM users WHERE nome = ?", ("ana",))
result = getAll()

post(result[0]["id"])
post(result[0]["nome"])
post(result[0]["email"])
```

- SELECT com resultado → lista de dicts
- SELECT sem resultado → `None`
- INSERT/UPDATE/DELETE → `None`

---

## PostgreSQL

Instale: `pip install psycopg2-binary`

```
import db

conn = db.connect(
    driver="postgres",
    host="localhost",
    port=5432,
    user="admin",
    password="senha",
    database="meu_banco"
)

cursor = conn.cursor()
cursor.execute("SELECT * FROM users")
result = getAll()
conn.close()

post(result)
```

---

## MySQL / MariaDB

Instale: `pip install mysql-connector-python`

```
import db

conn = db.connect(
    driver="mysql",
    host="localhost",
    port=3306,
    user="admin",
    password="senha",
    database="meu_banco"
)

cursor = conn.cursor()
cursor.execute("INSERT INTO users (nome, email) VALUES (%s, %s)", ("bia", "bia@email.com"))
conn.commit()
conn.close()
```

---

## MongoDB

Instale: `pip install pymongo`

```
import db

conn = db.connect(
    driver="mongo",
    host="localhost",
    port=27017,
    database="meu_banco"
)

col = conn.collection("users")

# Inserir
col.insert({"nome": "joao", "email": "joao@email.com", "ativo": true})

# Buscar todos
result = col.find()
post(result)

# Buscar com filtro
result = col.find({"nome": "joao"})
post(result)

# Buscar um
user = col.find_one({"email": "joao@email.com"})
post(user)

# Atualizar
col.update({"nome": "joao"}, {"email": "novo@email.com"})

# Remover
col.remove({"nome": "joao"})

# Contar
total = col.count()
post(total)

conn.close()
```

### Com autenticação

```
conn = db.connect(
    driver="mongo",
    host="localhost",
    port=27017,
    user="admin",
    password="senha",
    database="meu_banco"
)
```

---

## db.connect() — Todos os parâmetros

```
conn = db.connect(
    driver="sqlite",    # sqlite, postgres, mysql, mongo
    host="localhost",
    port=5432,
    user="admin",
    password="senha",
    database="meu_banco",
    base="arquivo.db"  # só pra SQLite
)
```

Se não tiver a lib do banco instalada, retorna uma string de erro com o comando de instalação.

---

## Exemplo completo com try/catch

```
import db
import date

action salvar_usuario(nome, email, senha_hash) {
    try {
        conn = db.query("banco.db")
        cursor = conn.cursor()
        cursor.execute(
            "INSERT INTO users (nome, email, senha, criado_em) VALUES (?, ?, ?, ?)",
            (nome, email, senha_hash, date.datahora())
        )
        conn.commit()
        conn.close()
        return true
    } catch (e) {
        post(f"Erro ao salvar: {e}")
        return false
    }
}

action buscar_usuario(email) {
    try {
        result = db.query(
            base="banco.db",
            cmd=("SELECT * FROM @t WHERE email = ?", (email,)),
            table="users"
        )
        if (result) {
            return result[0]
        } else {
            return None
        }
    } catch (e) {
        post(f"Erro na busca: {e}")
        return None
    }
}
```
