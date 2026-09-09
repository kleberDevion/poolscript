# `Jinker.post(path, auth=None, middleware=None, model=None)`

Registra uma rota que responde **só a POST**. É o
[`@app.route(...)`](../route/route.md) com o método já decidido pelo nome —
por isso não existe `methods=` aqui: **o verbo é o membro**.

É um membro do objeto [`Jinker`](../Jinker/Jinker.md), então na prática se
escreve `@app.post(...)`, com `app` sendo a sua aplicação.

```ps
from jinker import Jinker, jsonify, request, cors

app = Jinker(__name__)

model Login() {
    email: str(length=60)
    senha: str
}

@app.post("/login", model=Login, auth=cors.origins())
funct entrar() {
    # chegou aqui: é POST, a origem foi checada, e email/senha EXISTEM
    return jsonify({"ok": true, "email": request.get("email")})
}
```

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `path` | `str` | — | o path da URL (`"/login"`, `"/user/<id>"`); **o único obrigatório** |
| `auth` | lista de `str` | origens do `cors` | checagem de origem — [`cors.origins()`](../cors/origins/origins.md) |
| `middleware` | funct | nenhum | roda antes da funct — [middleware](../middleware/middleware.md) |
| `model` | `model` | nenhum | valida o corpo JSON **antes** da funct; torto vira `422` — a mesma regra do [`route`](../route/route.md#model--o-corpo-validado-antes-da-funct) |

Os nomes são estes quatro, e nesta ordem quando passados sem nome. `model=`
que não seja um `model` é `TypeError`.

São cinco atalhos, e são a mesma coisa com outro verbo:
[`get`](../get/get.md), `post`, [`put`](../put/put.md),
[`patch`](../patch/patch.md), [`delete`](../delete/delete.md). Tudo que está
nesta página vale pros cinco.

## Retorno

**`_RouteRegistrar`** — o registrador da rota, o mesmo objeto que
[`route`](../route/route.md) devolve.

```ps
reg = app.post("/login")
post(type(reg))     # _RouteRegistrar
post(reg)           # <RouteRegistrar ['POST'] /login>
```

Ele tem **um membro só**, `.register(handler)`, e é o decorador que o chama:
`@app.post("/login")` avalia `app.post("/login")`, define a funct de baixo e
chama `.register(funct)`. Detalhe em
[`RouteRegistrar`](../RouteRegistrar/RouteRegistrar.md).

---

## Método errado é `405`, não `404`

É a razão de o verbo estar no nome. Com `route(methods=["POST"])`, o front
escrevia `fetch("/login")` — que é GET — e recebia **404**: "a URL não existe".
A URL existia; o **método** é que não. O servidor agora separa os dois casos:

| situação | resposta |
|---|---|
| path e método batem | a funct roda |
| path **existe**, método não | **`405 Method Not Allowed`**, com `Allow:` |
| path não existe | `404` |

```
GET /login
→ 405
  Allow: POST
  {"error": true, "code": 405, "message": "método inválido: GET /login — a rota aceita POST"}
```

Se o mesmo path tem mais de um verbo (`@app.get("/perfil")` e
`@app.put("/perfil")`), o `Allow` lista todos, na ordem em que foram
registrados: `Allow: GET, PUT`.

O `OPTIONS` (preflight de CORS) não passa por isso — é respondido antes, com
`204`, como sempre foi. E `HEAD` numa rota `get` é atendido por ela (RFC 9110:
mesmos cabeçalhos, sem corpo).

---

## `route()` continua existindo

`@app.route(path, methods=[...])` é o jeito de uma funct responder a **mais
de um** método, e de deixar o método em aberto (sem `methods=`, ela aceita
todos os do `cors`). O atalho é pra quando a rota tem um verbo só — que é a
maioria.

---

## Relacionados

- [`@app.route(...)`](../route/route.md) — a forma geral, `methods=` e o `model=` em detalhe
- [`RouteRegistrar`](../RouteRegistrar/RouteRegistrar.md) — o objeto que os cinco verbos devolvem
- [`cors.origins()`](../cors/origins/origins.md) — o que vai em `auth=`
- [`request`](../request/request.md) — ler o corpo, o path param, o cabeçalho
- [visão geral do jinker](../jinker.md)
