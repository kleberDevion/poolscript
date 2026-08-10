# `PoolIp`

Gerencia rate limit e ban de IPs por instância Jinker.

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.banned_list()` |  |
| `.check(ip)` | Verifica se o IP pode fazer requisição. |
| `.is_banned(ip)` |  |
| `.unban(ip)` |  |
| `.bloq` | atributo |
| `.rate` | atributo |
| `.window` | atributo |

### `.check(...)`

Verifica se o IP pode fazer requisição.
Retorna (allowed: bool, reason: str)

[← índice](../jinker.md)
