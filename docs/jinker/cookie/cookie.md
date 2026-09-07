# Cookie — `resp.cookie(...)` e `request.cookie(nome)`

Gravar (`JinkerResponse.cookie`) e ler (`request.cookie`) ficam nesta página
porque são as duas metades da mesma operação.

```
JinkerResponse.cookie(nome, valor, path="/", max_age=Null,
                      httponly=true, secure=false,
                      samesite="Lax", domain=Null) -> JinkerResponse
request.cookie(nome) -> str | Null
```

---

## Gravar

```ps
import jinker
from jinker import request

app = jinker.Jinker("auth")

@app.route("/entrar", methods=["POST"])
funct entrar()
{
    token = gera_token(request.get("usuario"))
    return jinker.JinkerResponse()
           .send({"ok": true})
           .cookie("sid", token, max_age=3600)
}
```

Sai na resposta:

```
Set-Cookie: sid=<token>; Path=/; Max-Age=3600; SameSite=Lax; HttpOnly
```

Devolve a própria resposta, então **encadeia** — e cada chamada acrescenta um
cookie, não substitui o anterior:

```ps
return jinker.JinkerResponse()
       .send({"ok": true})
       .cookie("sid", token, max_age=3600)
       .cookie("tema", "escuro", httponly=false)
```

> `Set-Cookie` é o único cabeçalho de resposta que **se repete**: um por
> cookie. Por isso ele não passa pelo `.header()`, que é um dicionário — lá o
> segundo cookie sobrescreveria o primeiro e sumiria sem erro.

## Ler

```ps
@app.route("/eu")
funct eu()
{
    sid = request.cookie("sid")
    if sid == null
    {
        return jinker.JinkerResponse().send({"erro": "sem sessao"}).status(401)
    }
    return {"usuario": quem_e(sid)}
}
```

Cookie ausente devolve `Null` — não string vazia, que se confundiria com um
cookie apagado.

---

## Os atributos

| nome | padrão | o que faz |
|---|---|---|
| `path` | `"/"` | caminho em que o navegador manda o cookie de volta |
| `max_age` | `Null` | vida em SEGUNDOS. Sem ele, o cookie morre quando o navegador fecha |
| `httponly` | `true` | JavaScript **não** enxerga (`document.cookie` não vê) |
| `secure` | `false` | só viaja em HTTPS |
| `samesite` | `"Lax"` | `"Strict"`, `"Lax"` ou `"None"` — quando o cookie acompanha requisição vinda de outro site |
| `domain` | `Null` | domínio; sem ele, vale só pro host que respondeu |

Os padrões são os seguros: `httponly=true` e `samesite="Lax"` vêm ligados
porque cookie de sessão legível por JavaScript transforma qualquer XSS em
roubo de sessão. Passe `httponly=false` só quando o cliente precisa ler o
cookie.

`secure=false` é o padrão porque em desenvolvimento se roda em
`http://localhost`, e com `secure=true` o navegador descarta o cookie. Em
produção, `secure=true`.

---

## Apagar

Não há `delete`: um cookie se apaga sobrescrevendo com vida zero.

```ps
return jinker.JinkerResponse().send({"ok": true}).cookie("sid", "", max_age=0)
```

O `path` tem que ser o MESMO usado ao gravar — cookie é identificado por
`(nome, domínio, path)`, e apagar `/` não apaga o que foi gravado em `/x`.

---

## Sessão assinada: login, rota protegida e logout

Não há objeto de sessão no servidor. A sessão é um **token assinado** dentro
do cookie: o servidor não guarda nada; cada requisição traz o token, e o
servidor confere a assinatura. As peças são o cookie desta página e o módulo
[`jwt`](../../jwt/jwt.md) — `jwt.gen(payload, secret)` cria, `jwt.check(token,
secret)` confere e devolve o conteúdo, ou **`Null`** quando não presta.

```ps
import jinker
import jwt
import os
import date
from jinker import request

app = jinker.Jinker("auth")
SEGREDO = os.getenv("APP_SECRET")       # nunca no código: no .env

# LOGIN — valida a senha do seu jeito, emite o token, grava no cookie
@app.post("/entrar")
funct entrar()
{
    uid = valida_senha(request.get("usuario"), request.get("senha"))
    if uid is Null
    {
        return jinker.JinkerResponse().send({"erro": "usuario ou senha"}).status(401)
    }
    t = jwt.gen({"uid": uid, "exp": date.timestamp() + 3600}, SEGREDO)
    return jinker.JinkerResponse().send({"ok": true}).cookie("sid", t, max_age=3600)
}

# ROTA PROTEGIDA — o cookie chega sozinho; `check` devolve Null pra token
# adulterado, assinado com outro segredo, expirado ou ausente
@app.get("/eu")
funct eu()
{
    dados = jwt.check(request.cookie("sid"), SEGREDO)
    if dados is Null
    {
        return jinker.JinkerResponse().send({"erro": "sessao invalida"}).status(401)
    }
    return {"uid": dados["uid"]}
}

# LOGOUT — apaga o cookie: mesmo nome, mesmo path, vida zero
@app.post("/sair")
action sair()
{
    return jinker.JinkerResponse().send({"ok": true}).cookie("sid", "", max_age=0)
}
```

O que sai em cada passo:

| passo | resposta |
|---|---|
| `POST /entrar` certo | `Set-Cookie: sid=<token>; Path=/; Max-Age=3600; SameSite=Lax; HttpOnly` |
| `GET /eu` com o cookie | `{"uid": 7}` |
| `GET /eu` sem cookie, ou com token mexido, expirado ou de outro segredo | `401` |
| `POST /sair` | `Set-Cookie: sid=; Path=/; Max-Age=0; SameSite=Lax; HttpOnly` — o navegador descarta |

### Como a assinatura é feita

O token tem três partes em base64 URL-safe sem `=`, separadas por ponto:

```
header . payload . assinatura
{"alg":"HS256","typ":"JWT"} . {"uid":7,"exp":1788720489} . HMAC-SHA256(segredo, "header.payload")
```

- A assinatura é **HMAC** com o segredo sobre o texto exato `header.payload`.
  `jwt.gen(payload, secret, algorithm="HS256")` aceita `HS256`, `HS384` e
  `HS512`; qualquer outro nome (`RS256`, `none`…) é recusado com erro.
- Quem confere **não confia no `alg` do token**: `jwt.check` recalcula o HMAC
  com o segredo dele e só aceita a família HS. Token com `"alg":"none"`, sem
  assinatura, ou com um byte do payload trocado devolve `Null` — a
  comparação da assinatura é em tempo constante.
- `exp` é opcional e vale em **segundos desde a época** (o mesmo relógio de
  `date.timestamp()`); token com `exp` no passado devolve `Null`. Token sem
  `exp` nunca expira.
- O JSON entra compacto (sem espaços) e a ordem das chaves do header é fixa,
  porque a assinatura cobre os bytes — um espaço a mais gera um token que não
  valida.

Sem o segredo não se forja: quem tem o cookie pode ler o payload (é só
base64, **não é cifrado** — não ponha senha nem dado sensível nele), mas não
consegue produzir outro token que passe no `check`.

### O que XSS pode e não pode com esses padrões

- Com `httponly=true` (padrão), um script injetado na página **não lê** o
  cookie: `document.cookie` não o enxerga, então o token não sai da máquina
  por aí.
- Um script rodando na sua própria origem **ainda usa** a sessão: toda
  requisição que ele dispara leva o cookie junto. Isso não se resolve com
  atributo de cookie; resolve-se não tendo XSS — escapar tudo que vem do
  usuário antes de devolver como HTML.
- `samesite="Lax"` (padrão) impede que **outro site** dispare `POST` com o
  seu cookie (CSRF). `secure=true` impede que ele viaje em `http://` — ligue
  em produção.

### Logout num token sem estado

`/sair` apaga o cookie do navegador, mas um token já emitido **continua
válido até o `exp`** para quem o tiver copiado — o servidor não guarda nada
pra "esquecer". Duas saídas, escolha pelo que a aplicação precisa:

- **`exp` curto** (minutos) e reemissão nas rotas que importam: a janela de
  um token vazado é a duração dele;
- **lista de revogados** no servidor: no `/sair`, grave `dados["uid"]` (ou um
  `jti` que você põe no payload) com o `exp` num dicionário ou banco, e o
  `/eu` recusa o que estiver na lista. É estado, mas é só o dos que saíram,
  até o `exp` deles.

---

## Recusas

| escrito | resposta |
|---|---|
| `cookie("a\nb", "x")` | `ValueError` — quebra de linha no nome ou no valor **injeta cabeçalho**: o valor fecharia a linha e escreveria outra |
| `cookie("sid", 123)` | `TypeError` — nome e valor são `str`; converta com `str(...)` |
| cookie acima de 1023 bytes | `ValueError` dizendo o limite, em vez de truncar |

---

## Relacionados

- [`request.header()`](../header/header.md) — o cabeçalho cru, quando o cookie
  não basta
- [`JinkerResponse.header()`](../header/header.md) — cabeçalho de resposta que
  **não** se repete
- [`jwt`](../../jwt/jwt.md) — assinar e verificar o conteúdo da sessão

[← jinker](../jinker.md)
