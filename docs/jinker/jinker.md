# jinker — Servidor HTTP + WebSocket

Lib de servidor HTTP da PoolScript. **Zero dependências externas** — HTTP e
WebSocket são do próprio motor.

Esta é a **página de referência** da lib. Cada membro tem sua própria página
detalhada (links na tabela abaixo). Se é sua primeira vez, leia primeiro as
duas seções seguintes — elas ensinam o modelo antes dos detalhes.

---

## O modelo (leia isto primeiro)

Um servidor HTTP fica **esperando requisições** (um navegador ou app pedindo
uma URL) e decide o que responder pra cada uma. No jinker, você descreve esse
"o que responder" em pedaços chamados **rotas**.

Uma **rota** liga:

- um **caminho** de URL (ex: `/api/hello`), a
- uma **`action`** que roda quando esse caminho é acessado e devolve a resposta.

O ciclo é sempre este:

```
navegador pede  GET /api/hello
        │
        ▼
jinker acha a rota  ──►  roda sua action  ──►  você faz `return ...`
        │
        ▼
jinker vira seu return em resposta HTTP  ──►  navegador recebe
```

Dentro da action você tem `request` (o que chegou) de graça, e o `return` (o
que volta). Todo o resto é detalhe dessas duas pontas.

---

## Primeiro servidor (completo, funciona)

`app.ps`:

```
from jinker import Jinker, cors, jsonify
import os

app = Jinker(__name__)                    # 1. cria a aplicação

@app.route("/", methods=cors.options(["GET"]))   # 2. rota GET /
action inicio() {
    return jsonify({"msg": "meu primeiro servidor!"})
}

@app.route("/somar", methods=cors.options(["POST"]))   # 3. lê dados do cliente
action somar() {
    a = request.get("a")
    b = request.get("b")
    return jsonify({"resultado": a + b})
}

if __name__ == "main" {                   # 4. sobe o servidor
    porta = int(os.getenv("PORT", "8080"))
    app(debug=true, host="0.0.0.0", port=porta)
}
```

Rode `pool app.ps` e abra `http://localhost:8080/`. Cada bloco:

1. **`Jinker(__name__)`** cria a app — tudo pendura nela.
2. **`@app.route("/", ...)`** registra a action de baixo como resposta a
   `GET /`. Você não chama `inicio()` — o jinker chama quando a URL é acessada.
3. `request` já existe dentro da action; `return jsonify(...)` vira a resposta.
4. **`if __name__ == "main"`** liga o servidor (só quando o arquivo roda direto).

Repare que não teve `cors(...)` nem `auth=` — sem configurar, tudo é liberado.
Restrições são opcionais (ver [`cors`](cors/cors.md)).

---

## Referência — membros da lib

Importe de `jinker`: `from jinker import Jinker, cors, jsonify, render, request, JinkerRequest, JinkerResponse`.

| Membro | O que é | Página |
|---|---|---|
| `Jinker(...)` | a aplicação; construtor, `oauth`, subir o servidor | [Jinker/Jinker.md](Jinker/Jinker.md) |
| `@app.route(...)` | registra uma rota HTTP | [route/route.md](route/route.md) |
| `@app.get/post/put/patch/delete(...)` | rota com o verbo no nome; método errado é `405` | [post/post.md](post/post.md) |
| `@app.socket(...)` | registra um handler de WebSocket | [socket/socket.md](socket/socket.md) |
| `app.channel` | envia mensagens pros WebSockets conectados | [channel/channel.md](channel/channel.md) |
| `@app.middleware()` | verificação que roda antes de rotas | [middleware/middleware.md](middleware/middleware.md) |
| `cors(...)` | config global de métodos + origens | [cors/cors.md](cors/cors.md) |
| `request` | o que chegou na requisição | [request/request.md](request/request.md) |
| `resp.cookie(...)` · `request.cookie(...)` | gravar e ler cookie; sessão assinada com `jwt` | [cookie/cookie.md](cookie/cookie.md) |
| `jsonify(dados)` | atalho pra resposta JSON | [jsonify/jsonify.md](jsonify/jsonify.md) |
| `render(caminho)` | resposta a partir de um arquivo | [render/render.md](render/render.md) |
| `JinkerResponse` | o objeto de resposta (`.json`/`.send`/`.status`/`.header`) | [JinkerResponse/JinkerResponse.md](JinkerResponse/JinkerResponse.md) |
| `JinkerRequest` | o tipo de `request` | [request/request.md](request/request.md) |

---

## `Jinker(...)`, segurança e subir o servidor

O construtor, o `oauth` (rate limit com `poolip`, HTTPS com `tls`) e como ligar
o servidor têm página própria: **[Jinker/Jinker.md](Jinker/Jinker.md)**.

HTTPS e o aviso **"não seguro"** do navegador (self-signed, mkcert no dev,
Let's Encrypt em produção): **[tls.md](tls.md)**.

---

## Arquivos estáticos

Duas formas:

1. **`/static/` (fixo):** qualquer URL `/static/...` serve da pasta `static/`
   **ao lado do `.ps`** — não da pasta de onde o servidor foi chamado. Nome
   `static` é fixo, não configurável. Ver a seção em [render](render/render.md).
2. **`static_folder` (SPA):** `Jinker(static_folder="frontend/dist")` — hospeda
   um front-end inteiro junto com a API, com fallback pra `index.html`. Página
   própria: **[static_folder/static_folder.md](static_folder/static_folder.md)**.

Ordem de decisão de cada requisição: rota → path existe com outro método
(`405`) → `/static/` → arquivo no `static_folder` → `index.html` do
`static_folder` → 404.

**O que nunca sai** pelos dois: caminho que resolve pra **fora** da pasta
(`..`, codificado ou não, e symlink) e nome que começa com **`.`** (`.env`,
`.git`; a exceção é `.well-known`, do Let's Encrypt). O resto sai — `.ps`
inclusive, que é como o `psl install` baixa pacote de um registry —, então
**não aponte `static_folder` pra raiz do projeto**. Detalhe em
[static_folder/static_folder.md](static_folder/static_folder.md#o-que-não-sai).

---

## Estrutura típica de projeto

```
meu_projeto/
  app.ps
  .env
  frontend/
    dist/
      index.html
      app.js
  static/
    logo.png
```
