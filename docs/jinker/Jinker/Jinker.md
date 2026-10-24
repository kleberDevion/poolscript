# `Jinker(name="main", oauth=Null, static_folder=Null, static_url="/", route_prefix="")`

O construtor da aplicação. Cria o objeto `app` onde tudo se pendura — rotas,
sockets, middleware — e onde você configura segurança (rate limit, HTTPS) e
arquivos estáticos.

```ps
from jinker import Jinker

app = Jinker(__name__)
```

| Parâmetro | Tipo | Padrão | O que é |
|---|---|---|---|
| `name` | `str` | `"main"` | nome da app — passe `__name__`, que no arquivo executado vale `"main"` |
| `oauth` | dict | `Null` | segurança: rate limit (`poolip`) e HTTPS (`tls`) — ver abaixo |
| `static_folder` | `str` | `Null` | pasta com build de front a servir (SPA) — ver [static_folder](../static_folder/static_folder.md) |
| `static_url` | `str` | `"/"` | prefixo da URL sob o qual o `static_folder` é servido — ver [static_url](../static_url/static_url.md) |
| `route_prefix` | `str` | `""` | prefixo aplicado a **todas** as rotas e sockets: `"api"` ou `"/api/"` viram `/api`, e `@app.route("/users")` responde em `/api/users` (a raiz `"/"` vira só `/api`) |

Os exemplos desta página pressupõem:

```ps contexto
from jinker import Jinker
Jinker app = Jinker(__name__)
```

---

## Subindo o servidor

O objeto `app` é **chamável** — chamá-lo liga o servidor de verdade. Faça isso
dentro do ponto de entrada `if __name__ == "main"`:

```ps sem-rodar: sobe o servidor na porta 8080 e fica escutando
if __name__ == "main" {
    app(debug=False, host="0.0.0.0", port=8080)
}
```

| Argumento de `app(...)` | Tipo | Padrão | O que faz |
|---|---|---|---|
| `debug` | `bool` | `False` | imprime logs de requisição e erros detalhados no terminal |
| `host` | `str` | `"127.0.0.1"` | interface de rede (`"127.0.0.1"` = só local, como no Flask; `"0.0.0.0"` = aceita de qualquer IP) |
| `port` | `int` | `2000` | porta HTTP, de 0 a 65535 |
| `reload` | `bool` | `False` | reinicia sozinho quando o arquivo `.pr` muda (dev). No Windows o processo que o terminal esperou supervisiona e roda o programa de novo a cada mudança |
| `workers` | `int` | `1` | nº de processos (prefork), de 1 a 256. `>1` usa vários núcleos: forka N processos que dividem o socket HTTP. No Windows (sem `fork`) cada worker roda o programa de novo e recebe a porta já aberta: o que vem antes do `app(...)` roda em cada um |

O tipo e a faixa de cada argumento são conferidos antes de abrir a porta, e
o argumento a mais e o nome que não é parâmetro também, com a frase das
funções da linguagem. A porta tem 16 bits, como no
[`sockets`](../../sockets/sockets.md):

```ps
app(port="8080")
# TypeError: app() argument 'port' must be int, not str
```

```ps
app(port=70000)
# OverflowError: app(): port must be 0-65535
```

```ps
app(workers=0)
# ValueError: app(): workers fora da faixa 1..256: 0
```

Se houver algum `@app.socket(...)`, o servidor WebSocket sobe automaticamente
na porta **HTTP + 1**. Com `workers>1`, o WebSocket roda **só no worker 0**
(salas/broadcast num processo só, corretas) — o HTTP é que espalha nos N.

---

## `oauth` — segurança embutida

Um dict com chaves opcionais. Só o que você colocar é ativado.

```ps
app = Jinker(__name__, oauth={poolip: True, rate: 60, bloq: 1, tls: True})
```

### `poolip` — rate limit + ban de IP

| Chave | Padrão | O que faz |
|---|---|---|
| `poolip: True` | (desligado) | **liga** o controle de IP |
| `rate: N` | `100` | máximo de requisições por IP a cada **60 segundos** |
| `bloq: N` | `1` | dias de **ban** quando o IP passa do limite |

Como funciona: cada IP pode fazer até `rate` requisições por janela de 60s.
Passou disso → recebe **429 Too Many Requests** e é **banido por `bloq` dias**
(toda requisição durante o ban recebe 429 com `Retry-After`).

```ps
# no máximo 60 req/min por IP; quem passar fica banido 1 dia
app = Jinker(__name__, oauth={poolip: True, rate: 60, bloq: 1})
```

Sem `poolip: True`, não há limite nenhum (o `rate`/`bloq` são ignorados).

### `tls` — HTTPS

| Chave | O que faz |
|---|---|
| `tls: True` | serve em **https://** em vez de http:// |
| `cert: "caminho"` | caminho do certificado (opcional) |
| `key: "caminho"` | caminho da chave privada, quando em arquivo separado |

Se `tls: True` e nenhum `cert`, o jinker procura um `.jinkerTls` no projeto;
não achando, **gera um certificado self-signed automático** (v3, com SAN pra
localhost/127.0.0.1/::1) pra uso temporário — o navegador ainda marca "não
seguro" porque é self-signed.

Cert e chave podem vir **juntos** num PEM só, ou **separados** (é como
Let's Encrypt e mkcert entregam: `fullchain.pem` + `privkey.pem`):

```ps
# juntos (cert e chave no mesmo arquivo)
app = Jinker(__name__, oauth={tls: True, cert: ".jinkerTls"})

# separados (o normal de um cert real)
app = Jinker(__name__, oauth={tls: True, cert: "fullchain.pem", key: "privkey.pem"})
```

Sem `key`, o jinker procura a chave irmã `<base>.key` ao lado do cert (ex:
`cert.pem` → `cert.key`); não achando, assume que a chave está dentro do próprio
cert.

**Sumir com o "não seguro" do navegador** (self-signed sempre avisa): use um
cert confiável — **mkcert** no dev, **Let's Encrypt** em produção. Receita
passo a passo: **[tls.md](../tls.md)**.

---

## Exemplo completo

```ps
from jinker import Jinker, cors, jsonify
import os

app = Jinker(__name__, oauth={poolip: True, rate: 100, bloq: 1})
cors(options=["GET", "POST"], origins=["https://meusite.com"])

@app.route("/api/status", methods=cors.options(["GET"]))
funct status() {
    return jsonify({"online": True})
}

if __name__ == "main" {
    porta = int(os.getenv("PORT", "8080"))
    app(debug=False, host="0.0.0.0", port=porta)
}
```

---

## Relacionados

- [`static_folder`](../static_folder/static_folder.md) — servir um front (SPA)
- [`route`](../route/route.md) — registrar rotas
- [`cors`](../cors/cors.md) — restringir métodos e origens
- [visão geral do jinker](../jinker.md)
