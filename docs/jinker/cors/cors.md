# `cors(options=None, origins=None, permiser=None)`

Configuração **global** de acesso da aplicação: quais métodos HTTP são aceitos
e de quais origens (domínios). É uma instância única, importada da lib.

```
from jinker import cors

cors(options: list = None, origins: list = None) -> cors
```

Chamar `cors(...)` é **opcional** — sem ele, tudo é liberado (qualquer método
configurável por rota, qualquer origem).

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

Lista dos métodos HTTP liberados globalmente:

```
cors(options=["GET", "POST", "PUT", "PATCH", "DELETE"])
```

Cada rota pode **restringir** esse conjunto com
[`cors.options([...])`](options/options.md) no `methods=`.

---

## `origins` — domínios permitidos

```
cors(origins=["https://meusite.com"])
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

## Uso completo

```
from jinker import Jinker, cors, jsonify

app = Jinker(__name__)
cors(options=["GET", "POST"], origins=["https://meusite.com"])

@app.route("/api/dados", auth=cors.origins(), methods=cors.options(["GET"]))
action dados() {
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
