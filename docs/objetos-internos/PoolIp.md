# `PoolIp` — não é um objeto

Esta página descrevia um tipo `PoolIp` com `.check(ip)`, `.is_banned(ip)`,
`.unban(ip)`, `.banned_list()`, `.bloq`, `.rate` e `.window`. **Nada disso é
alcançável**: `type()` nunca devolve `PoolIp`, o nome não aparece em
`pool --metadata`, e `app.poolip` é
`AttributeError: 'Jinker' object has no attribute 'poolip'`.

Na VM o PoolIp é **configuração**, não objeto. Você liga na criação do app:

```ps
Object app = Jinker(__name__, oauth={poolip: true, rate: 60, bloq: 1})
```

| chave | o que é |
|---|---|
| `poolip` | liga a contagem por IP |
| `rate` | quantas requisições o IP pode fazer na janela |
| `bloq` | por quantos **dias** o IP fica bloqueado ao estourar |

A **janela é fixa em 60 segundos** — não há `window` para configurar. Ao
estourar, a resposta diz `IP bloqueado por N dia(s)`.

Não há como consultar nem desbloquear um IP de dentro do programa: a lista vive
no servidor e não é exposta.

[← índice](objetos-internos.md)
