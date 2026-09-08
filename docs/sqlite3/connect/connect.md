# `sqlite3.connect(database)`

Abre conexão com banco SQLite. Cria o arquivo se não existir.

## Parâmetros

| nome | default |
|---|---|
| `database` | (obrigatório) |

## Retorno

[`PoolConnection`](../../objetos-internos/PoolConnection.md) — dela sai o
cursor (`.cursor()`), e nela ficam `.commit()`, `.rollback()` e `.close()`.

## Exemplo

```
import sqlite3
con = sqlite3.connect("dados.db")
cur = con.cursor()
cur.execute("CREATE TABLE IF NOT EXISTS t (n TEXT)")
con.commit()
con.close()
```

[← índice](../sqlite3.md)
