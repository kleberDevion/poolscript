# jinker — Servidor HTTP

Lib de servidor HTTP da PoolScript. Zero dependências externas para HTTP básico.
Para WebSocket instale: `pip install websockets`.

## O que é e como funciona (leia isto primeiro)

Um servidor HTTP fica **esperando requisições** (um navegador ou app pedindo
uma URL) e, pra cada uma, decide o que responder. No jinker você descreve
esse "o que responder" em pedaços chamados **rotas**.

Uma **rota** é a ligação entre:

- um **caminho** de URL (ex: `/api/hello`), e
- uma **`action`** que roda quando alguém acessa esse caminho e devolve a
  resposta.

O ciclo de uma requisição é sempre este:

```
navegador pede  GET /api/hello
        │
        ▼
jinker acha a rota "/api/hello"  ──► roda a sua action  ──► você faz `return ...`
        │
        ▼
jinker transforma seu return em resposta HTTP  ──►  navegador recebe
```

Dentro da action você tem dois objetos de graça (não precisa declarar):
`request` (o que chegou) e o `return` (o que você manda de volta). O resto
deste doc é só detalhar cada parte desse ciclo.

## Primeiro servidor (exemplo completo, rode e veja)

Crie um arquivo `app.ps` com isto — é um servidor inteiro, funcional:

```
from jinker import Jinker, cors, jsonify
import os

app = Jinker(__name__)              // 1. cria a aplicação

// 2. uma rota: quem acessar GET /  recebe o JSON abaixo
@app.route("/", methods=cors.options(["GET"]))
action inicio() {
    return jsonify({"msg": "meu primeiro servidor jinker!"})
}

// 3. uma rota que lê algo de quem chamou
@app.route("/somar", methods=cors.options(["POST"]))
action somar() {
    a = request.get("a")            // pega do corpo JSON ou da query string
    b = request.get("b")
    return jsonify({"resultado": a + b})
}

// 4. sobe o servidor quando o arquivo é executado direto
run_selfwith_("main") {
    porta = int(os.getenv("PORT", "8080"))
    app(debug=true, host="0.0.0.0", port=porta)
}
```

Rode com `pool app.ps` e teste no navegador `http://localhost:8080/` — você vê
o JSON. Entendendo cada bloco:

1. **`Jinker(__name__)`** cria a aplicação. Tudo (rotas, sockets) pendura nela.
2. **`@app.route("/", ...)`** registra a `action` logo abaixo como a resposta
   pra `GET /`. Você **não** chama `inicio()` — o jinker chama por você quando
   a URL é acessada.
3. Dentro da action, **`request`** já existe. `return jsonify(...)` vira a
   resposta HTTP (mais sobre retornos adiante).
4. **`run_selfwith_("main")`** é o ponto de entrada (só roda quando você
   executa o arquivo direto, não quando ele é importado). `app(...)` liga o
   servidor de verdade.

> Repare: **não** teve `cors(...)` nem `auth=`. Sem configurar origens, o
> jinker libera qualquer origem — perfeito pra começar. Restrições de acesso
> são o próximo assunto, e são opcionais.

---

## Setup completo (com CORS)

Quando você for pro sério, adiciona a configuração de acesso global:

```
from jinker import Jinker, cors, jsonify

app = Jinker(__name__)
cors(options=["GET", "POST"], origins=["https://meusite.com"])
```

`cors(...)` é opcional (sem ele, tudo é liberado). A próxima seção explica.

---

## cors()

Define as configurações globais de acesso. Dois parâmetros:

```
cors(options=["POST", "GET", "DELETE"], origins=["https://meusite.com"])
```

**options** — métodos HTTP aceitos globalmente: `GET`, `POST`, `PUT`,
`PATCH`, `DELETE`. Cada rota pode restringir esse conjunto com
`cors.options([...])`.

**origins** — lista de origens (domínios) permitidas a acessar a API.
São checadas contra o header `Origin`/`Referer` da requisição:

| `origins=` | Efeito |
|---|---|
| `[]` (ou omitido) | **Libera qualquer origem** (sem restrição) |
| `["https://meusite.com"]` | Só requisições vindas desse domínio |
| `["https://a.com", "https://b.com"]` | Vários domínios permitidos |

Regras automáticas (não precisa configurar):
- **Origens locais** (`localhost`, `127.x.x.x`, `0.0.0.0`, `::1`) são sempre
  permitidas, mesmo com uma lista de `origins` restrita — pra você testar
  local sem liberar o mundo.
- **Cliente sem `Origin`** (Insomnia, Postman, `curl`, outro backend) passa —
  a checagem de origem é uma proteção de *browser*, não bloqueia ferramenta.

> **Legado:** o parâmetro `permiser=` e o método `cors.permiser()` ainda
> existem por compatibilidade (não vão ser removidos), mas `permiser=` na
> config é **silenciosamente ignorado** (não configura nada) e
> `cors.permiser()` é só um apelido de `cors.origins()`. Prefira `origins=` e
> `cors.origins()`. Formatos antigos como `"*/api"` ou
> `"allowed.all/Users-Agent"` **não existem** — eram fictícios.

---

## Rotas

```
@app.route("/api/hello", auth=cors.origins(), methods=cors.options(["GET"]))
action handler() {
    return jsonify({"msg": "olá!"}), 200
}
```

Não precisa envolver a `action` em chaves extras — o `@app.route(...)` já captura a próxima `action` como handler da rota. A `action` não recebe parâmetros — `request` já está disponível automaticamente dentro dela.

---

## request

Disponível automaticamente dentro de qualquer rota:

```
@app.route("/api/dados", auth=cors.origins(), methods=cors.options(["POST"]))
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
@app.route("/user:id", auth=cors.origins(), methods=cors.options(["GET"]))
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

### `jsonify` e `JinkerResponse` são a MESMA coisa

`jsonify(dados)` **não é um tipo diferente** — é literalmente um atalho de uma
linha que constrói um `JinkerResponse` e chama `.json(dados)` nele. Ou seja,
estas três formas produzem exatamente a mesma resposta:

```
return jsonify({"msg": "ok"})                      // atalho
return JinkerResponse().json({"msg": "ok"})        // idêntico ao de cima
return {"msg": "ok"}                                // dict puro: a rota converte sozinha
```

Use `jsonify` por ser mais curto; use `JinkerResponse` direto quando quiser
montar a resposta em etapas (ou quando não for JSON — veja `.send()` abaixo).

**Métodos do `JinkerResponse`** (todos devolvem o próprio objeto, então
encadeiam):

| Método | Faz |
|---|---|
| `.json(dados, status=200)` | corpo JSON (`application/json`) — é o que `jsonify` chama |
| `.send(texto, status=200)` | corpo texto puro (`text/plain`) — **não** tem atalho tipo `jsonify` |
| `.status(codigo)` | troca só o status, sem mexer no corpo |
| `.header(chave, valor)` | adiciona um cabeçalho |

```
return jsonify({"msg": "ok"}).header("X-Request-Id", "abc123"), 200
return jsonify({"msg": "criado"}).status(201)   // status() dispensa a tupla
return JinkerResponse().send("texto puro", 200)  // resposta não-JSON
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
@app.route("/api/login", auth=cors.origins(), methods=cors.options(["POST"]))
action login() {
    return jsonify({"msg": "ok"}), 200
}

# rota protegida — middleware roda primeiro
@app.route("/api/dados", auth=cors.origins(), methods=cors.options(["GET"]), middleware=app.middleware)
action dados() {
    return jsonify({"msg": "área protegida"}), 200
}
```

---

## render — Servir um arquivo (HTML, CSS, imagem, qualquer coisa)

`render(caminho)` lê um arquivo do disco e devolve um `JinkerResponse` com o
**MIME type correto detectado pela extensão** (`.html` → `text/html`, `.png` →
`image/png`, `.pdf` → `application/pdf`, etc.). Arquivos de texto vão como
texto; binários (imagem/pdf/…) vão como bytes automaticamente.

### Assinatura e as duas formas de passar o caminho

```
render(caminho)                 # um argumento só
render(pasta, arquivo)          # dois argumentos — junta os dois com "/"
```

As duas formas abaixo são **exatamente equivalentes** — escolha a que ficar
mais legível:

```
return render("paginas/index.html")     // caminho completo num argumento só
return render("paginas", "index.html")  // pasta + arquivo separados
```

Internamente `render("paginas", "index.html")` só faz `"paginas" / "index.html"`
= `"paginas/index.html"`, então dá no mesmo. A forma de dois argumentos é útil
quando a pasta é fixa e o arquivo é variável:

```
action pagina() {
    nome = request.path_param("nome")
    return render("paginas", f"{nome}.html")   // paginas/<nome>.html
}
```

### Onde `render()` procura o arquivo

O caminho é resolvido, nesta ordem, relativo a:

1. **A pasta do arquivo `.ps` em execução** (onde está o seu `app.ps`);
2. **O diretório atual** (`cwd`, de onde você rodou `pool`).

O primeiro que tiver o arquivo vence. Um caminho absoluto
(`render("C:/algo/x.html")`) é usado como está, sem busca.

Não existe pasta obrigatória: `render("index.html")`, `render("web/index.html")`
e `render("qualquer/pasta/que/voce/quiser/pag.html")` funcionam igual — a
"pasta" é só o começo do caminho que você escreveu.

Se o arquivo **não existir**, `render()` devolve uma resposta **404** com uma
página de erro (não estoura exceção) — então dá pra usar direto no `return`.

```
from jinker import Jinker, cors, render

@app.route("/", auth=cors.origins(), methods=cors.options(["GET"]))
action index() {
    return render("web/index.html")
}
```

---

## Arquivos estáticos

Há **duas** formas de servir CSS/JS/imagens automaticamente — uma fixa
(`/static/`) e uma configurável (`static_folder`).

### 1. Pasta `/static/` (fixa, sem configuração)

Qualquer URL que comece com `/static/` é servida a partir de uma pasta
`static/` no diretório onde você rodou o servidor. Não precisa configurar nada,
mas **o nome `static` é fixo** — não dá pra renomear.

```
/static/style.css   →  serve  ./static/style.css
/static/script.js   →  serve  ./static/script.js
/static/logo.png    →  serve  ./static/logo.png
```

```html
<link rel="stylesheet" href="/static/style.css">
<script src="/static/script.js"></script>
<img src="/static/logo.png">
```

### 2. `static_folder` (configurável, modo SPA)

No construtor do `Jinker` você aponta uma pasta com o build do front (React,
Vue, etc.). Aí **qualquer** URL que não bata numa rota tenta servir o arquivo
físico correspondente dentro dessa pasta; se não achar, cai no `index.html`
dela (o roteamento client-side de SPA assume dali).

```
app = Jinker(__name__, static_folder="frontend/dist")
```

Fluxo de uma requisição, na ordem em que o servidor decide o que responder:

1. Bateu numa **rota** `@app.route(...)`? → executa o handler.
2. Começa com **`/static/`** e o arquivo existe em `static/`? → serve o arquivo.
3. `static_folder` está definido e a URL corresponde a um **arquivo físico**
   dentro dele (ex: `/app.js` → `frontend/dist/app.js`)? → serve o arquivo.
4. `static_folder` definido mas a URL não é um arquivo? → serve o
   **`index.html`** da pasta (fallback de SPA).
5. Nada disso? → **404**.

`static_folder` é buscado nas mesmas raízes do `render()` (pasta do `.ps` e
`cwd`), então `"frontend/dist"` pode estar ao lado do seu `app.ps`.

> **Nota:** o construtor também aceita `static_url="/"`, mas hoje esse valor é
> **guardado e não usado** na hora de servir — a correspondência é feita contra
> a URL inteira. Ou seja, não dá (ainda) pra "montar" o SPA num prefixo tipo
> `/app`. Isso é justamente o que dá pra melhorar (ver a conversa sobre
> flexibilizar a sintaxe).

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
| `reload` | (dev) Reinicia o servidor quando o `.ps` muda. Só com `workers=1`. |
| `workers` | Nº de processos que dividem a porta (paralelismo real entre núcleos). Padrão `1`. |

---

## Concorrência e escala

Esta seção descreve o motor `pool` (binário em C). O interpretador
(`python -m poolscript`) é o runtime de referência/dev e roda sempre em um
processo com uma thread por requisição — o **observável** (o que a rota devolve)
é idêntico; o que muda é a capacidade sob carga.

### Como as requisições rodam

Cada requisição HTTP é servida numa **fibra** (green-thread): um fluxo de
execução leve e próprio, com a sua pilha. Você **não escreve nada de diferente** —
a `action` continua síncrona, sem `async`/`await`. A diferença aparece quando um
handler **espera**: se ele chama `sleep(...)`, a fibra **devolve o controle ao
servidor** e outras requisições são atendidas enquanto essa dorme; quando o tempo
passa, ela continua de onde parou. Ou seja, um handler que espera **não trava**
os outros no mesmo worker.

```
@app.route("/lento")
action lento() {
    sleep(1)                 // 100 clientes aqui NÃO viram 100s de fila:
    return jsonify({"ok": true})   // todos dormem juntos e respondem em ~1s
}
```

Medido (1 worker, handler com `sleep(0.1)`): **100 clientes simultâneos → ~570
req/s** (antes, serializado, eram ~10 req/s). O pool de fibras **cresce sob
demanda** (não trava num teto fixo), com uma rede de segurança lá no alto; na
prática um worker roda **centenas de handlers ao mesmo tempo** sem travar nem
estourar a memória.

**Consulta de banco também não trava.** Um `SELECT` lento roda dentro do driver
em C (libpq/mysql/odbc) fazendo `recv` bloqueante — isso travaria o worker. Por
isso a consulta é jogada numa **thread** enquanto a fibra cede: várias queries
correm em paralelo e o worker segue atendendo. Medido (Postgres, `pg_sleep(0.2)`):
10 requisições concorrentes em ~0,4s, não 2s. Você não muda nada no código — é
automático pros drivers de rede (postgres/mysql/sqlserver). SQLite roda direto
(é local e rápido, não bloqueia em rede).

Abrir a conexão (`connect()`) também é async pelos mesmos motivos — o handshake
de rede vai pra thread e a fibra cede.

**Requisição de saída também não trava.** Um `request.get/post/...` (chamar uma
API, disparar um webhook) e o `ws_connect(...)` (WebSocket de saída) fazem
DNS+connect+TLS+envio+recepção — rede bloqueante que travaria o worker. Igual ao
banco, isso vai pra uma **thread** e a fibra cede: dá pra chamar várias APIs em
paralelo sem congelar o servidor. Medido: 3 webhooks a um alvo de 0.5s em ~0.78s,
não 1.5s. (Dentro de um handler, `request` é a requisição de **entrada**; o
módulo de saída entra como `import request as web` e você usa `web.get(...)`.)

> **O que ainda trava a fibra:** um cálculo pesado **puro de CPU** (não tem I/O
> pra ceder — só termina ocupando o núcleo; use `workers` pra espalhar). O
> `commit()`/`BEGIN` do banco seguem inline (são controle rápido, um round-trip).
> Cedem hoje: `sleep`, a consulta ao banco (postgres/mysql), o `connect()`, as
> requisições de saída (`request.*`) e o `ws_connect()`.

### Muitas conexões ao mesmo tempo

O servidor usa `epoll`: uma conexão **parada** (keep-alive esperando a próxima
requisição) praticamente **não custa** — nem CPU (só as conexões com dados são
processadas) nem memória (o buffer de leitura de 16 KB é liberado enquanto a
conexão está ociosa, ~275 bytes cada). Medido: **16 mil conexões ociosas
consomem ~16 MB** e o throughput não cai. Extrapolando, ~100 mil conexões ociosas
ficam na casa de **~40 MB**.

### Usando todos os núcleos (`workers`)

Uma fibra é concorrência **dentro de um núcleo** (uma coisa roda por vez, elas se
revezam na espera). Pra usar os vários núcleos da máquina, suba mais processos:

```
app(host="0.0.0.0", port=2000, workers=4)   // 4 processos dividem a porta
```

Cada worker é um processo próprio (o kernel balanceia as conexões entre eles),
com a sua VM e as suas fibras — sem corrida, sem estado compartilhado. Regra
prática: `workers` ≈ número de núcleos; cada worker soma a sua concorrência de
fibras. (WebSocket com salas/broadcast roda só no worker 0.)

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
cors(options=["POST", "GET"], origins=["https://meusite.com"])

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
@app.route("/", auth=cors.origins(), methods=cors.options(["GET"]))
action index() {
    return render("index.html")
}

# API de login
@app.route("/api/login", auth=cors.origins(), methods=cors.options(["POST"]))
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
