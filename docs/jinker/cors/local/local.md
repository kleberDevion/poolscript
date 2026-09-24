# `cors.local()`

Devolve o modo de origem da última chamada de `cors(app, ...)`: `true` (o
padrão) ou `false` quando foi configurado `local=false`, o modo estrito.

```
cors.local() -> bool
```

---

## O que o modo muda

```
cors(app, origins=["https://meusite.com"])                # cors.local() == true
cors(app, origins=["https://meusite.com"], local=false)   # cors.local() == false
```

| Pedido | `true` (padrão) | `false` (estrito) |
|---|---|---|
| `Origin` da lista | passa | passa |
| `Origin` fora da lista | `403` | `403` |
| origem local fora da lista (`localhost`, `127.x.x.x`, `0.0.0.0`, `::1`) | passa | `403` |
| sem `Origin` (`curl`, Postman, outro backend) | passa | `403` |
| `Origin: null` | `403` | passa só se `"null"` está na lista |

A regra é a mesma nas rotas e no handshake do WebSocket. A recusa é sempre
`403` com `{"error": true, "code": 403, "message": "Origem não autorizada:
..."}` e **sem** `Access-Control-Allow-Origin`. Sem `origins` configurado o
modo não tem efeito: lista vazia libera tudo.

---

## Uso

```
from jinker import Jinker, cors

app = Jinker(__name__)
cors(app, origins=["https://meusite.com"], local=false)

@app.route("/estado")
funct estado() {
    return {"estrito": not cors.local()}
}
```

---

## Relacionados

- [`cors`](../cors.md) — configuração global (onde `local=` é definido)
- [`cors.origins()`](../origins/origins.md) — as origens configuradas
- [`socket`](../../socket/socket.md) — a origem vale no handshake
