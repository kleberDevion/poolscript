# `cors.local()`

Devolve o modo de origem da última chamada de `cors(app, ...)`: `True` (o
padrão) ou `False` quando foi configurado `local=False`, o modo estrito.

```assinatura
cors.local() -> bool
```

Os exemplos desta página pressupõem:

```ps contexto
from jinker import Jinker, cors
Jinker app = Jinker(__name__)
```

---

## O que o modo muda

```ps
cors(app, origins=["https://meusite.com"])                # cors.local() == True
cors(app, origins=["https://meusite.com"], local=False)   # cors.local() == false
```

| Pedido | `True` (padrão) | `False` (estrito) |
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

```ps
from jinker import Jinker, cors

app = Jinker(__name__)
cors(app, origins=["https://meusite.com"], local=False)

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
