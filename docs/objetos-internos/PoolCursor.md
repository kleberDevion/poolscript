# `PoolCursor`

> **Objeto interno da linguagem** — você não cria `PoolCursor` na mão:
> é o TIPO de um objeto que a lib `sqlite3` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

Wrapper do cursor SQLite — expõe métodos como atributos.

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.close()` |  |
| `.execute(sql, params=())` |  |
| `.executemany(sql, params)` | o segundo parâmetro se chama `params`, não `seq` |
| `.fetchall()` |  |
| `.fetchmany(size=1)` |  |
| `.fetchone()` |  |
| `.lastrowid` |  |
| `.rowcount` |  |

[← índice](objetos-internos.md)
