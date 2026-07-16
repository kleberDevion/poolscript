# jinker — Servidor HTTP

Lib de servidor HTTP da PoolScript. Zero dependências externas para HTTP básico.
Para WebSocket instale: `pip install websockets`

---

## Setup

```
from jinker import Jinker, cors, jsonify

app = Jinker(__name__)
cors(options=["GET", "POST"], permiser=["*/api", "allowed.all/Users-Agent"])
```

---

## cors()

Define as configurações globais de acesso:

```
cors(options=["POST", "GET", "DELETE"], permiser=["*/api", "allowed.all/Users-Agent"])
```

**options** — métodos HTTP aceitos: `GET`, `POST`, `PUT`, `PATCH`, `DELETE`

**permiser** — regras de acesso:

| Regra | Significado |
|---|---|
| `"*/api"` | Qualquer origem para rotas com prefixo `/api` |
| `"://meusite.com/api"` | Só de um domínio específico |
| `"allowed.all/Users-Agent"` | Libera qualquer User-Agent |

---

## Rotas

```
@app.route("/api/hello", auth=cors.permiser(), methods=cors.options(["GET"]))
action handler() {
    return jsonify({"msg": "olá!"}), 200
}
```

Não precisa envolver a `action` em chaves extras — o `@app.route(...)` já captura a próxima `action` como handler da rota. A `action` não recebe parâmetros — `request` já está disponível automaticamente dentro dela.

---

## request

Disponível automaticamente dentro de qualquer rota:

```
@app.route("/api/dados", auth=cors.permiser(), methods=cors.options(["POST"]))
action receber() {
    data = request.get_json()       # body como dict
    nome = request.get("nome")      # campo específico do JSON ou query string
    metodo = request.method         # "POST"
    caminho = request.path          # "/api/dados"
    texto = request.text()          # body como texto puro
    return jsonify({"recebido": data}), 200
}
```

Em rotas ou sockets com parâmetro dinâmico no path (`/user:id`, `/user/<id>`),
pegue o valor com `request.path_param()`:

```
@app.route("/user:id", auth=cors.permiser(), methods=cors.options(["GET"]))
action perfil() {
    id = request.path_param("id")
    return jsonify({"user_id": id}), 200
}
```

---

## Retornos da rota

```
# dict ou lista → JSON automático
return {"msg": "ok"}
return [1, 2, 3]

# jsonify com status code
return jsonify({"msg": "criado"}), 201
return jsonify({"msg": "não encontrado"}), 404
return jsonify({"msg": "erro interno"}), 500

# texto puro
return "olá mundo"

# sem body
return None    # responde 204
```

`jsonify(...)` devolve um objeto `JinkerResponse`, encadeável com `.header()`
pra mandar cabeçalhos customizados e `.status()` pra trocar o código depois:

```
return jsonify({"msg": "ok"}).header("X-Request-Id", "abc123"), 200
return jsonify({"msg": "criado"}).status(201)   // status() dispensa a tupla
```

`request` (o parâmetro implícito de toda rota) é uma instância de
`JinkerRequest`; o objeto de retorno é `JinkerResponse` — ambos exportados
pela lib (`from jinker import JinkerRequest, JinkerResponse`) caso precise
checar o tipo (`request is JinkerRequest`) ou construir uma resposta na mão
em vez de usar `jsonify`/`render`.

---

## Middleware

Define verificação que roda antes de rotas protegidas:

```
@app.middleware()
action verificar() {
    token = request.get("token")
    if (not token) {
        return jsonify({
            "status": "erro",
            "msg": "Sem permissão"
        }), 401
    }
    continue   # libera pra entrar na rota
}
```

Aplica em rotas específicas com `middleware=app.middleware`:

```
# rota livre — sem middleware
@app.route("/api/login", auth=cors.permiser(), methods=cors.options(["POST"]))
action login() {
    return jsonify({"msg": "ok"}), 200
}

# rota protegida — middleware roda primeiro
@app.route("/api/dados", auth=cors.permiser(), methods=cors.options(["GET"]), middleware=app.middleware)
action dados() {
    return jsonify({"msg": "área protegida"}), 200
}
```

---

## render — Servir páginas HTML

```
from jinker import Jinker, cors, render

@app.route("/", auth=cors.permiser(), methods=cors.options(["GET"]))
action index() {
    return render("index.html")
}

@app.route("/login", auth=cors.permiser(), methods=cors.options(["GET"]))
action login() {
    return render("login.html")
}
```

O `render()` busca o arquivo na pasta `templates/` do projeto.

---

## Arquivos estáticos

Coloque CSS, JS e imagens na pasta `static/`. São servidos automaticamente:

```
/static/style.css   →  http://localhost:2000/static/style.css
/static/script.js   →  http://localhost:2000/static/script.js
/static/logo.png    →  http://localhost:2000/static/logo.png
```

No HTML:

```html
<link rel="stylesheet" href="/static/style.css">
<script src="/static/script.js"></script>
<img src="/static/logo.png">
```

---

## Estrutura de pastas

```
meu_projeto/
  app.ps
  .env
  templates/
    index.html
    login.html
    dashboard.html
  static/
    style.css
    script.js
    logo.png
```

---

## WebSocket

Precisa instalar: `pip install websockets`

O WebSocket sobe automaticamente na porta `HTTP + 1`. Se o servidor HTTP for na porta `7700`, o WebSocket fica na `7701`.

```
@app.socket("/chat", channel=True)
action bora_msg() {
    msg = request.get_json()
    app.channel(forAll=msg)

    if (app.channel.status == "Success") {
        post("Mensagem enviada!")
    } else {
        post("Erro no envio")
    }
}
```

**channel=True** — canal aberto, todos os conectados recebem as mensagens.

Conectando no frontend:

```javascript
const ws = new WebSocket("ws://localhost:7701/chat")

ws.onmessage = (event) => {
    const msg = JSON.parse(event.data)
    console.log(msg)
}

ws.send(JSON.stringify({
    user_name: "joao",
    body_msg: "Olá pessoal!"
}))
```

### Salas (rooms)

Quando o path do socket tem parâmetro dinâmico, cada conexão entra
automaticamente numa "sala" = valor do parâmetro. Um `emit(room_id=...)`
alcança só quem está conectado naquela sala — as outras salas não recebem:

```
@app.socket("/sala:id", channel=True)
action mensagem() {
    msg = request.get_json()
    id  = request.path_param("id")

    send = app.socket()             # emissor em runtime — com instância
    send.emit(payload=msg, room_id=id)
    post("status: " {send.status_send()})
}
```

`app.socket.emit(...)` faz a mesma coisa sem precisar instanciar:

```
app.socket.emit(payload=msg, room_id=id)
```

`app.socket()` chamado **com** `path` continua funcionando como decorator
de registro (`@app.socket("/chat", channel=True)`); chamado **sem** args,
em runtime, devolve um emissor.

**Quem enviou a mensagem não recebe o próprio broadcast de volta por
padrão** — evita a mensagem aparecer duplicada em UIs de chat. Pra ecoar
de volta também pro remetente, passe `exclude_self=false`:

```
send.emit(payload=msg, room_id=id, exclude_self=false)
```

Sem `room_id`, `emit()`/`app.socket.emit()` faz broadcast pra todo mundo
conectado no socket (equivalente a `app.channel(forAll=...)`, também
sujeito ao `exclude_self`).

| Membro | Uso |
|---|---|
| `app.socket(path, channel=True)` | decorator — registra o handler do socket |
| `app.socket()` | instancia um emissor (`SocketEmitter`) pra usar em runtime |
| `app.socket.emit(payload, room_id=None, exclude_self=True)` | emite direto, sem instanciar |
| `send = app.socket(); send.emit(...)` | emite via instância |
| `send.status_send()` | status do último emit feito por essa instância |
| `app.channel(forAll=msg)` | broadcast geral (todas as salas), API antiga |
| `app.channel.status` | status do último `app.channel(forAll=...)` |

---

## Iniciando o servidor

```
run_selfwith_("main") {
    app(debug=True, host="0.0.0.0", port=2000)
}
```

| Parâmetro | Descrição |
|---|---|
| `debug` | Mostra erros detalhados no terminal |
| `host` | `"0.0.0.0"` aceita qualquer conexão, `"127.0.0.1"` só local |
| `port` | Porta do servidor |

---

## Exemplo completo com WebSocket e rotas

```
import db
import hash
import jwt
import date
import os
from dotenv import load
from jinker import Jinker, cors, jsonify, render

load()

str DB_PATH = os.getenv("DB_PATH")
str SECRET = os.getenv("SECRET_KEY")

app = Jinker(__name__)
cors(options=["POST", "GET"], permiser=["*/api", "allowed.all/Users-Agent"])

# Middleware de autenticação
@app.middleware()
action auth() {
    token = request.get("token")
    if (not token) {
        return jsonify({"msg": "não autorizado"}), 401
    }
    payload = jwt.check(token, SECRET)
    if (not payload) {
        return jsonify({"msg": "token inválido"}), 401
    }
    continue
}

# Página principal
@app.route("/", auth=cors.permiser(), methods=cors.options(["GET"]))
action index() {
    return render("index.html")
}

# API de login
@app.route("/api/login", auth=cors.permiser(), methods=cors.options(["POST"]))
action login() {
    data = request.get_json()
    email = data.get("email")
    senha = data.get("senha")

    result = db.query(
        base=DB_PATH,
        cmd=("SELECT * FROM @t WHERE email = ?", (email,)),
        table="usuarios"
    )

    if (result and hash.check(result[0]["senha"], senha)) {
        payload = {
            "user_id": result[0]["id"],
            "exp": date.timestamp() + date.hora(hours=24)
        }
        token = jwt.gen(payload, SECRET, algorithm="HS256")
        return jsonify({"token": token}), 200
    } else {
        return jsonify({"msg": "credenciais inválidas"}), 401
    }
}

# Chat em tempo real
@app.socket("/chat", channel=True)
action mensagem() {
    msg = request.get_json()
    author = msg.get("user_name")
    content = msg.get("body_msg")

    try {
        db.query(
            base=DB_PATH,
            cmd=("INSERT INTO @t (author, corpo, data) VALUES (?, ?, ?)",
                 (author, content, date.datahora())),
            table="chat"
        )
    } catch (e) {
        post(f"Erro ao salvar msg: {e}")
    }

    app.channel(forAll=msg)
}

run_selfwith_("main") {
    app(debug=False, host="0.0.0.0", port=7700)
}
```
