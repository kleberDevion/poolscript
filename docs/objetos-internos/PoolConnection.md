# `PoolConnection`

> **Objeto interno da linguagem** — você não cria `PoolConnection` na mão:
> é o TIPO de um objeto que a lib `sqlite3` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

Wrapper da conexão SQLite — expõe métodos como atributos.

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.close()` |  |
| `.commit()` |  |
| `.cursor()` |  |
| `.execute(sql, params=())` | Atalho: conn.execute() sem precisar criar cursor. |
| `.rollback()` |  |

[← índice](objetos-internos.md)
