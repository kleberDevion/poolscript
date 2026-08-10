<!-- gerado: gera_doc_gaps.py — pode regenerar -->
# `PoolIp`

> **Objeto interno da linguagem** — você não cria `PoolIp` na mão:
> é o TIPO de um objeto que a lib `jinker` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

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
