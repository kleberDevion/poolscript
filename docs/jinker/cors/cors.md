# `cors(app, ..., options=None, origins=None)`

Configuração de acesso de um servidor: quais métodos HTTP são aceitos e de
quais origens (domínios). O **primeiro argumento é o servidor** (a instância
do `Jinker`), e pode ser mais de um — a mesma configuração vale pra todos os
que forem passados.

```
from jinker import Jinker, cors

app = Jinker(__name__)
cors(app, options: list = None, origins: list = None) -> cors
```

Chamar `cors(...)` é **opcional** — servidor sem `cors(app, ...)` libera tudo
(qualquer método configurável por rota, qualquer origem).

```
cors(app, origins=["https://meusite.com"])                # um servidor
cors(api, admin, options=["GET", "POST"], origins=["https://meusite.com"])   # dois
```

Sem o servidor é erro, antes de configurar qualquer coisa:

```
cors(origins=["https://meusite.com"])
# TypeError: cors() precisa do servidor como primeiro argumento: cors(app, origins=[...])
```

---

## O que é CORS (pra quem nunca ouviu)

Quando um site em `https://meusite.com` faz uma requisição pro seu servidor
em outro domínio, o **navegador** checa se o seu servidor autoriza aquele
domínio. Se não autorizar, o navegador bloqueia — isso é CORS. `origins`
é a lista de domínios que você autoriza.

Importante: é uma checagem **de navegador**. Ferramentas como `curl`, Postman
ou outro backend não passam por CORS (por isso continuam funcionando mesmo com
`origins` restrito).

---

## `options` — métodos aceitos

Lista dos métodos HTTP liberados pro servidor:

```
cors(app, options=["GET", "POST", "PUT", "PATCH", "DELETE"])
```

É o que o preflight `OPTIONS` responde em `Access-Control-Allow-Methods`, e
o conjunto que uma rota sem `methods=` aceita. Cada rota pode **restringir**
esse conjunto com [`cors.options([...])`](options/options.md) no `methods=`.

---

## `origins` — domínios permitidos

```
cors(app, origins=["https://meusite.com"])
```

| `origins=` | Efeito |
|---|---|
| `[]` ou omitido | **libera qualquer origem** |
| `["https://meusite.com"]` | só esse domínio |
| `["https://a.com", "https://b.com"]` | vários domínios |

Regras automáticas:

- **Origens locais** (`localhost`, `127.x.x.x`, `0.0.0.0`, `::1`) sempre
  passam, mesmo com lista restrita — pra testar local sem liberar o mundo.
- **Cliente sem header `Origin`** (Postman, `curl`, backend) passa.

Ver [`cors.origins()`](origins/origins.md) para usar no `auth=` de uma rota.

---

## A config é por servidor

Cada `cors(app, ...)` cria a configuração **daquele(s)** servidor(es): é
ela que o registro das rotas e o preflight leem. Dois servidores no mesmo
programa podem ter configurações diferentes:

```
cors(api, origins=["https://app.meusite.com"])
cors(admin, origins=["https://admin.meusite.com"])
```

Os helpers `cors.options()` e `cors.origins()` (sem servidor) devolvem o que
a **última** chamada de `cors(...)` configurou — é o que se usa no
`methods=`/`auth=` das rotas escritas logo abaixo dela.

---

## Uso completo

```
from jinker import Jinker, cors, jsonify

app = Jinker(__name__)
cors(app, options=["GET", "POST"], origins=["https://meusite.com"])

@app.route("/api/dados", auth=cors.origins(), methods=cors.options(["GET"]))
funct dados() {
    return jsonify({"ok": true})
}
```

---

## Legado: `permiser`

O parâmetro `permiser=` e o método `cors.permiser()` ainda existem por
compatibilidade e **não vão ser removidos**, mas:

- `permiser=` na config é **silenciosamente ignorado** (não configura nada);
- `cors.permiser()` é só um **apelido** de `cors.origins()`.

Prefira `origins=` e `cors.origins()`. Formatos antigos como `"*/api"` ou
`"allowed.all/Users-Agent"` **nunca existiram de verdade** — eram fictícios na
doc velha.

---

## Métodos

| Método | O que faz | Doc |
|---|---|---|
| `cors.options([...])` | resolve/filtra métodos pra uma rota | [options/options.md](options/options.md) |
| `cors.origins()` | devolve as origens configuradas (usado em `auth=`) | [origins/origins.md](origins/origins.md) |
