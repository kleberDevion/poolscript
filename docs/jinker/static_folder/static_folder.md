# `static_folder` — servir um front-end (SPA)

Parâmetro do construtor [`Jinker(...)`](../Jinker/Jinker.md) que aponta uma
pasta com o **build de um front-end** (React, Vue, ou só HTML/CSS/JS). Com ele,
qualquer URL que **não** bata numa rota tenta servir o arquivo correspondente
dessa pasta — e, se não achar, cai no `index.html` dela (o roteamento do SPA
assume dali).

```
app = Jinker(__name__, static_folder="frontend/dist")
```

---

## Diferença pra pasta `/static/`

São dois mecanismos separados:

| | `/static/` (fixo) | `static_folder` (SPA) |
|---|---|---|
| Configura? | não | `Jinker(static_folder="...")` |
| Nome da pasta | **`static` fixo** | qualquer caminho |
| Qual URL serve | só as que começam com `/static/` | **qualquer** URL não-roteada |
| Fallback | não tem | serve `index.html` se não achar o arquivo |

Ou seja: `/static/` é pra assets soltos; `static_folder` é pra hospedar um
app front-end inteiro junto com a API.

---

## O fluxo completo de uma requisição

Quando uma requisição chega, o jinker decide o que responder **nesta ordem**:

1. Bateu numa **rota** `@app.route(...)`? → roda o handler. **Fim.**
2. Começa com **`/static/`** e o arquivo existe em `static/`? → serve o arquivo.
3. `static_folder` definido e a URL é um **arquivo físico** dentro dele
   (ex: `/app.js` → `frontend/dist/app.js`)? → serve o arquivo.
4. `static_folder` definido mas a URL **não** é um arquivo (ex: `/perfil`,
   `/login` — rotas do front)? → serve o **`index.html`** da pasta.
5. Nada disso? → **404**.

O passo 4 é o que faz um SPA funcionar: o usuário acessa `/perfil` direto (ou dá
F5 lá), o servidor devolve o `index.html`, e o roteador do front (React Router,
etc.) mostra a tela certa.

---

## Onde a pasta é procurada

`static_folder` é resolvido nas mesmas raízes do `render()`:

1. a pasta do `.ps` em execução;
2. o diretório atual (`cwd`).

Então `"frontend/dist"` pode ficar ao lado do seu `app.ps`.

---

## Exemplo: API + front no mesmo servidor

```
from jinker import Jinker, cors, jsonify

app = Jinker(__name__, static_folder="frontend/dist")

# a API responde em /api/...
@app.route("/api/usuarios", methods=cors.options(["GET"]))
action usuarios() {
    return jsonify([{"nome": "ana"}, {"nome": "leo"}])
}

# qualquer outra URL (/, /perfil, /app.js, /style.css) é servida
# do frontend/dist automaticamente — não precisa de rota pra cada uma

if __name__ == "main" {
    app(debug=false, host="0.0.0.0", port=8080)
}
```

Estrutura:

```
meu_projeto/
  app.ps
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

```
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
