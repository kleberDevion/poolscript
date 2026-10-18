# `static_url` — onde o `static_folder` é montado

Parâmetro do construtor [`Jinker(...)`](../Jinker/Jinker.md) que define o
**prefixo de URL** sob o qual os arquivos do [`static_folder`](../static_folder/static_folder.md)
são servidos. O padrão é `"/"` (raiz).

```assinatura
Jinker(name, oauth=, static_folder=, static_url="/")
```

Ele **não é obrigatório** e **não é um decorator** — não existe
`@app.static_url(...)`. É só um argumento na hora de criar o app.

Os exemplos desta página pressupõem:

```ps contexto
from jinker import Jinker
Jinker app = Jinker(__name__, static_folder="dist", static_url="/app")
```

---

## Pra que serve

Serve pra **isolar os arquivos estáticos das rotas de API**. Sem prefixo, tudo
divide a raiz: se a API tem `/users` e o `static_folder` também teria algo em
`/users`, dá ambiguidade. Com um prefixo, os assets ficam só embaixo dele e a
API fica dona do resto.

```ps
app = Jinker(__name__, static_folder="dist", static_url="/app")

# dist/app.js      →  GET /app/app.js      serve o arquivo
# dist/style.css   →  GET /app/style.css   serve o arquivo
# GET /users                               → NÃO é estático → vai pra API/404
```


---

## Default `"/"` = raiz (comportamento de sempre)

Se você **não** passar `static_url`, nada muda: o `static_folder` continua sendo
servido na raiz, como sempre foi.

```ps
app = Jinker(__name__, static_folder="dist")   # static_url = "/"

# dist/app.js  →  GET /app.js   (raiz)
```

---

## Como o prefixo casa

Com `static_url="/app"`:

| URL pedida | Resultado |
|---|---|
| `/app/app.js` | serve `dist/app.js` |
| `/app` | serve `dist/index.html` (entrada do SPA) |
| `/app/perfil` (não é arquivo) | serve `dist/index.html` (fallback SPA) |
| `/app/docs/` (pasta com `index.html`) | serve `dist/docs/index.html` |
| `/app/docs` (a mesma pasta, sem a barra) | `301` com `Location: docs/` |
| `/app.js` (fora do prefixo) | não é estático → 404 |
| `/users` | não é estático → rota/404 |

A barra final é ignorada (`"/app"` e `"/app/"` são iguais).

---

## Aviso: `/static` é reservado

O prefixo **`/static`** já é usado pelo mecanismo fixo da pasta física
`/static/` (ver [`static_folder`](../static_folder/static_folder.md) →
"Diferença pra pasta `/static/`"), que roda ANTES do `static_folder`. Então não
use `static_url="/static"` — escolha `/app`, `/assets`, `/public` etc.

---

## Ler de volta

`app.static_url` devolve o valor configurado (default `"/"`), e
`app.static_folder` a pasta (ou `Null` se não setada):

```ps
post(app.static_url)      # "/app"
post(app.static_folder)   # "dist"
```

---

## Relacionados

- [`static_folder`](../static_folder/static_folder.md) — a pasta servida
- [`Jinker(...)`](../Jinker/Jinker.md) — o construtor
- [`render`](../render/render.md) — servir **um** arquivo específico numa rota
