# Cookie — `resp.cookie(...)` e `request.cookie(nome)`

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
action entrar()
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
action eu()
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

**Os padrões são os seguros, não os permissivos.** `httponly=true` e
`samesite="Lax"` vêm ligados porque cookie de sessão legível por JavaScript
transforma qualquer XSS em roubo de sessão, e quem escreve
`resp.cookie("sid", token)` sem pensar nos atributos merece o padrão que não o
machuca. Passe `httponly=false` quando o cliente PRECISA ler.

`secure=false` é o padrão porque em desenvolvimento se roda em `http://
localhost` — com `secure=true` o navegador descarta o cookie e o login não
funciona, sem dizer por quê. **Em produção, ligue.**

---

## Apagar

Não há `delete`: um cookie se apaga sobrescrevendo com vida zero.

```ps
return jinker.JinkerResponse().send({"ok": true}).cookie("sid", "", max_age=0)
```

O `path` tem que ser o MESMO usado ao gravar — cookie é identificado por
`(nome, domínio, path)`, e apagar `/` não apaga o que foi gravado em `/x`.

---

## Sessão assinada

Não há objeto de sessão: o que existe é cookie e o módulo `jwt`, e juntos dão
sessão sem estado no servidor.

```ps
import jwt

SEGREDO = os.getenv("APP_SECRET")

@app.route("/entrar", methods=["POST"])
action entrar()
{
    t = jwt.encode({"uid": 7, "exp": date.timestamp() + 3600}, SEGREDO)
    return jinker.JinkerResponse().send({"ok": true}).cookie("sid", t, max_age=3600)
}

@app.route("/eu")
action eu()
{
    try
    {
        dados = jwt.decode(request.cookie("sid"), SEGREDO)
        return {"uid": dados["uid"]}
    }
    catch (e)
    {
        return jinker.JinkerResponse().send({"erro": "sessao invalida"}).status(401)
    }
}
```

Assinar importa: sem isso o cliente edita o cookie e vira outro usuário.

---

## Recusas

| escrito | resposta |
|---|---|
| `cookie("a\nb", "x")` | `ValueError` — quebra de linha no nome ou no valor **injeta cabeçalho**: o valor fecharia a linha e escreveria outra |
| `cookie("sid", 123)` | `TypeError` — nome e valor são `str`; converta com `str(...)` |
| cookie acima de 1023 bytes | `ValueError` dizendo o limite, em vez de truncar calado |

---

## Relacionados

- [`request.header()`](../header/header.md) — o cabeçalho cru, quando o cookie
  não basta
- [`JinkerResponse.header()`](../header/header.md) — cabeçalho de resposta que
  **não** se repete
- [`jwt`](../../jwt/jwt.md) — assinar e verificar o conteúdo da sessão

[← jinker](../jinker.md)
