# `static_folder` — servir um front-end (SPA)

Parâmetro do construtor [`Jinker(...)`](../Jinker/Jinker.md) que aponta uma
pasta com o **build de um front-end** (React, Vue, ou só HTML/CSS/JS). Com ele,
qualquer URL que **não** bata numa rota tenta servir o arquivo correspondente
dessa pasta — e, se não achar, cai no `index.html` dela (o roteamento do SPA
assume dali).

Os exemplos desta página pressupõem:

```ps contexto
from jinker import Jinker
```

```ps
app = Jinker(__name__, static_folder="frontend/dist")
```

---

## Diferença pra pasta `/static/`

São dois mecanismos separados:

| | `/static/` (fixo) | `static_folder` (SPA) |
|---|---|---|
| Configura? | não | `Jinker(static_folder="...")` |
| Nome da pasta | **`static` fixo** | qualquer caminho |
| Onde fica | **ao lado do `.pr`** | onde você apontar (ver abaixo) |
| Qual URL serve | só as que começam com `/static/` | **qualquer** URL não-roteada |
| Fallback | não tem | serve `index.html` se não achar o arquivo |

Ou seja: `/static/` é pra assets soltos; `static_folder` é pra hospedar um
app front-end inteiro junto com a API.

---

## O fluxo completo de uma requisição

Quando uma requisição chega, o jinker decide o que responder **nesta ordem**:

1. Bateu numa **rota** (path **e** método)? → roda o handler. **Fim.**
2. O path de uma rota bateu, mas o **método** não? → **`405`** com `Allow:`.
   **Fim.** (ver [post/post.md](../post/post.md))
3. Começa com **`/static/`** e o arquivo existe na `static/` ao lado do `.pr`?
   → serve o arquivo. Se for uma pasta com `index.html`, serve o `index.html`
   dela (ver [Pasta](#pasta-tipo-do-arquivo-pedaço-e-cache)).
4. `static_folder` definido e a URL é um **arquivo físico** dentro dele
   (ex: `/app.js` → `frontend/dist/app.js`)? → serve o arquivo. Pasta com
   `index.html` serve o `index.html` dela.
5. `static_folder` definido mas a URL **não** é um arquivo (ex: `/perfil`,
   `/login` — rotas do front)? → serve o **`index.html`** da pasta.
6. Nada disso? → **404**.

O passo 5 é o que faz um SPA funcionar: o usuário acessa `/perfil` direto (ou dá
F5 lá), o servidor devolve o `index.html`, e o roteador do front (React Router,
etc.) mostra a tela certa.

---

## Pasta, tipo do arquivo, pedaço e cache

Valem pros dois mecanismos (`/static/` e `static_folder`), do jeito que os
servidores de arquivo consolidados fazem: o `FileServer` do Go (`net/http`), o
`http.server` do Python e a RFC 9110 (HTTP).

**Pasta.** A pasta pedida serve o `index.html` **dela**, como o Go ("use
contents of index.html for directory, if present") e o Python ("the directory
is checked for an index page"):

| pedido | resposta |
|---|---|
| `/docs/`, com `docs/index.html` | o `docs/index.html` |
| `/docs` (pasta, sem a barra) | `301` com `Location: docs/` (a query vai junto) |
| `/vazia/`, sem `index.html` | no `static_folder`, o `index.html` da raiz (SPA); na `/static/`, `404` |
| `/app` com `static_url="/app"` (a raiz da montagem) | o `index.html`, sem `301` |

O `301` existe porque, sem a barra no fim, os caminhos relativos do
`index.html` da pasta apontariam pra pasta de cima. O `Location` é relativo
(o nome da pasta com a barra), como o do Go. A pasta **nunca é listada**, e o
`index.html` achado passa pelas mesmas regras de [O que NÃO sai](#o-que-não-sai).

**Tipo do arquivo.** O `Content-Type` sai da extensão, pela tabela padrão do
`mimetypes` do Python 3.15 (a mesma do `http.server`, 213 extensões).

| extensão | tipo |
|---|---|
| `.html`, `.htm` | `text/html` |
| `.js`, `.mjs` | `text/javascript` |
| `.css` | `text/css` |
| `.json` | `application/json` |
| `.wasm` | `application/wasm` (o navegador recusa outro tipo no `WebAssembly.instantiateStreaming`) |
| `.avif`, `.webp`, `.svg` | `image/avif`, `image/webp`, `image/svg+xml` |
| `.webmanifest` | `application/manifest+json` |
| desconhecida | `application/octet-stream` |

**Pedido por pedaço (`Range`).** É o que o player de vídeo e o navegador
usam pra pular pro meio do arquivo; sem ele o Safari não toca vídeo. Vale em
`GET` e `HEAD`, um intervalo por pedido (RFC 9110, seção 14):

| `Range:` do pedido (arquivo de 100 bytes) | resposta |
|---|---|
| `bytes=0-9` | `206 Partial Content`, `Content-Range: bytes 0-9/100`, os 10 primeiros bytes |
| `bytes=90-` | `206`, do byte 90 até o fim |
| `bytes=-5` | `206`, os 5 últimos bytes |
| `bytes=200-` | `416 Range Not Satisfiable`, `Content-Range: bytes */100` |
| `bytes=0-1,5-6` (vários intervalos) | `200` com o arquivo inteiro (a RFC deixa o servidor ignorar o `Range`) |

**Cache.** Toda resposta de arquivo leva `Accept-Ranges: bytes`,
`Last-Modified` (a data do arquivo) e `ETag` (montado do tamanho e da data).
O navegador manda de volta, e:

- `If-None-Match` com o `ETag` do arquivo → `304 Not Modified`, sem corpo;
- sem `If-None-Match`, `If-Modified-Since` igual ou depois da data do arquivo
  → `304`;
- `If-Range` que não bate com o `ETag` nem com a data → o `Range` é ignorado e
  sai o arquivo inteiro (`200`).

**Memória.** O arquivo sai em blocos de 64 KB: servir um vídeo de 2 GB não põe
2 GB na memória do servidor.

---

## Onde a pasta é procurada

`static_folder` é resolvido nas mesmas raízes do `render()`:

1. a pasta do `.pr` em execução;
2. o diretório atual (`cwd`).

Então `"frontend/dist"` pode ficar ao lado do seu `app.pr`.

---

## O que NÃO sai

Duas regras, e as duas valem pros dois mecanismos (`/static/` e
`static_folder`):

| pedido | resposta | por quê |
|---|---|---|
| `/static/../app.pr`, `/%2e%2e/.env`, symlink que aponta pra fora | `404` | o caminho é **resolvido** (`realpath`) e tem que terminar **dentro** da pasta. Não é filtro de `..` na URL: qualquer codificação que vire `..` resolve, e a que não vira é um nome que não existe |
| `/.env`, `/.git/config`, `/static/.htaccess` | `404` | nome começando com `.` não sai. A **única** exceção é `.well-known/`, que existe pra ser público — é por onde o Let's Encrypt valida o domínio |

No SPA, o que é barrado cai no `index.html` (passo 5) como qualquer URL
desconhecida — o cliente não distingue "não existe" de "existe e não sai".

**Todo o resto que está na pasta sai — inclusive `.pr`.** O motivo: o
`psl install` baixa pacotes `.pr` de um registry que pode ser um
`static_folder` do próprio jinker. O `.pr` da *sua aplicação* não sai porque
ele não está na pasta publicada: a `/static/` é uma **subpasta** ao lado dele,
e travessia pra fora dela é barrada.

Por isso, **não aponte `static_folder` pra raiz do projeto** (`"."`). Isso
publica o projeto — o `.pr` da aplicação junto. O `.git` e o `.env` ficam
protegidos pela regra do ponto, mas o código não, porque publicar código é
uso legítimo e o servidor não tem como saber qual `.pr` é o seu.

---

## Exemplo: API + front no mesmo servidor

```ps
from jinker import Jinker, cors, jsonify

app = Jinker(__name__, static_folder="frontend/dist")

# a API responde em /api/...
@app.route("/api/usuarios", methods=cors.options(["GET"]))
funct usuarios() {
    return jsonify([{"nome": "ana"}, {"nome": "leo"}])
}

# qualquer outra URL (/, /perfil, /app.js, /style.css) é servida
# do frontend/dist automaticamente — não precisa de rota pra cada uma

if __name__ == "main" {
    app(debug=False, host="0.0.0.0", port=8080)
}
```

Estrutura:

```texto
meu_projeto/
  app.pr
  frontend/
    dist/
      index.html
      app.js
      style.css
```

---

## Montando num prefixo: `static_url`

Por padrão (`static_url="/"`) o `static_folder` é servido a partir da **raiz** —
é o caso acima e você não precisa mexer em nada. Se quiser **montar os arquivos
sob um prefixo** (pra não colidir com as rotas de API), passe `static_url`:

```ps
app = Jinker(__name__, static_folder="dist", static_url="/app")

# dist/app.js  →  GET /app/app.js
# fora do prefixo (ex: /users) o static_folder NÃO responde → sobra pra API
```

Detalhes na [página do `static_url`](../static_url/static_url.md). Um aviso: o
prefixo **`/static`** é reservado pela pasta física `/static/` (mecanismo fixo,
ver abaixo), então escolha outro (`/app`, `/assets`…) pra montar o SPA.

---

## Relacionados

- [`Jinker(...)`](../Jinker/Jinker.md) — o construtor onde `static_folder` é passado
- [`render`](../render/render.md) — servir **um** arquivo específico numa rota
- [visão geral do jinker](../jinker.md) — resumo dos dois mecanismos de estático
