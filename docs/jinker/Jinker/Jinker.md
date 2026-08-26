# `Jinker(name="__main__", oauth=None, static_folder=None, static_url="/", route_prefix="")`

O construtor da aplicação. Cria o objeto `app` onde tudo se pendura — rotas,
sockets, middleware — e onde você configura segurança (rate limit, HTTPS) e
arquivos estáticos.

```
from jinker import Jinker

app = Jinker(__name__)
```

| Parâmetro | Tipo | Padrão | O que é |
|---|---|---|---|
| `name` | `str` | `"__main__"` | nome da app — passe `__name__` |
| `oauth` | dict | `None` | segurança: rate limit (`poolip`) e HTTPS (`tls`) — ver abaixo |
| `static_folder` | `str` | `None` | pasta com build de front a servir (SPA) — ver [static_folder](../static_folder/static_folder.md) |
| `static_url` | `str` | `"/"` | (guardado, **ainda não usado** na correspondência de URL) |
| `route_prefix` | `str` | `""` | prefixo aplicado a **todas** as rotas e sockets: `"api"` ou `"/api/"` viram `/api`, e `@app.route("/users")` responde em `/api/users` (a raiz `"/"` vira só `/api`) |

---

## Subindo o servidor

O objeto `app` é **chamável** — chamá-lo liga o servidor de verdade. Faça isso
dentro do ponto de entrada `if __name__ == "main"`:

```
if __name__ == "main" {
    app(debug=false, host="0.0.0.0", port=8080)
}
```

| Argumento de `app(...)` | Padrão | O que faz |
|---|---|---|
| `debug` | `false` | imprime logs de requisição e erros detalhados no terminal |
| `host` | `"0.0.0.0"` | interface de rede (`"0.0.0.0"` = aceita de qualquer IP; `"127.0.0.1"` = só local) |
| `port` | `2000` | porta HTTP |
| `reload` | `false` | reinicia sozinho quando o arquivo `.ps` muda (dev) |
| `workers` | `1` | nº de processos (prefork). `>1` usa vários núcleos: forka N processos que dividem o socket HTTP. |

Se houver algum `@app.socket(...)`, o servidor WebSocket sobe automaticamente
na porta **HTTP + 1**. Com `workers>1`, o WebSocket roda **só no worker 0**
(salas/broadcast num processo só, corretas) — o HTTP é que espalha nos N.

---

## `oauth` — segurança embutida

Um dict com chaves opcionais. Só o que você colocar é ativado.

```
app = Jinker(__name__, oauth={poolip: true, rate: 60, bloq: 1, tls: true})
```

### `poolip` — rate limit + ban de IP

| Chave | Padrão | O que faz |
|---|---|---|
| `poolip: true` | (desligado) | **liga** o controle de IP |
| `rate: N` | `100` | máximo de requisições por IP a cada **60 segundos** |
| `bloq: N` | `1` | dias de **ban** quando o IP passa do limite |

Como funciona: cada IP pode fazer até `rate` requisições por janela de 60s.
Passou disso → recebe **429 Too Many Requests** e é **banido por `bloq` dias**
(toda requisição durante o ban recebe 429 com `Retry-After`).

```
// no máximo 60 req/min por IP; quem passar fica banido 1 dia
app = Jinker(__name__, oauth={poolip: true, rate: 60, bloq: 1})
```

Sem `poolip: true`, não há limite nenhum (o `rate`/`bloq` são ignorados).

### `tls` — HTTPS

| Chave | O que faz |
|---|---|
| `tls: true` | serve em **https://** em vez de http:// |
| `cert: "caminho"` | caminho do certificado (opcional) |
| `key: "caminho"` | caminho da chave privada, quando em arquivo separado |

Se `tls: true` e nenhum `cert`, o jinker procura um `.jinkerTls` no projeto;
não achando, **gera um certificado self-signed automático** (v3, com SAN pra
localhost/127.0.0.1/::1) pra uso temporário — o navegador ainda marca "não
seguro" porque é self-signed.

Cert e chave podem vir **juntos** num PEM só, ou **separados** (é como
Let's Encrypt e mkcert entregam: `fullchain.pem` + `privkey.pem`):

```
// juntos (cert e chave no mesmo arquivo)
app = Jinker(__name__, oauth={tls: true, cert: ".jinkerTls"})

// separados (o normal de um cert real)
app = Jinker(__name__, oauth={tls: true, cert: "fullchain.pem", key: "privkey.pem"})
```

Sem `key`, o jinker procura a chave irmã `<base>.key` ao lado do cert (ex:
`cert.pem` → `cert.key`); não achando, assume que a chave está dentro do próprio
cert.

**Sumir com o "não seguro" do navegador** (self-signed sempre avisa): use um
cert confiável — **mkcert** no dev, **Let's Encrypt** em produção. Receita
passo a passo: **[tls.md](../tls.md)**.

---

## Exemplo completo

```
from jinker import Jinker, cors, jsonify
import os

app = Jinker(__name__, oauth={poolip: true, rate: 100, bloq: 1})
cors(options=["GET", "POST"], origins=["https://meusite.com"])

@app.route("/api/status", methods=cors.options(["GET"]))
action status() {
    return jsonify({"online": true})
}

if __name__ == "main" {
    porta = int(os.getenv("PORT", "8080"))
    app(debug=false, host="0.0.0.0", port=porta)
}
```

---

## Relacionados

- [`static_folder`](../static_folder/static_folder.md) — servir um front (SPA)
- [`route`](../route/route.md) — registrar rotas
- [`cors`](../cors/cors.md) — restringir métodos e origens
- [visão geral do jinker](../jinker.md)
