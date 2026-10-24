# `JinkerResponse` — o objeto de resposta

`JinkerResponse` é o que uma rota devolve pro cliente. Toda vez que você faz
`return jsonify(...)`, `return render(...)` ou `return "texto"`, o que vai por
baixo é um `JinkerResponse`.

Você raramente precisa criá-lo na mão — os atalhos (`jsonify`, `render`, ou só
devolver um dict/lista/string) cobrem quase tudo. Use direto (`JinkerResponse()`)
quando quiser montar a resposta em etapas ou combinar corpo + status + headers
com controle total.

```ps
from jinker import JinkerResponse
```

Os exemplos desta página pressupõem:

```ps contexto
from jinker import JinkerResponse, jsonify
str token = "t-1"
```

---

## Como a rota interpreta o que você retorna

Você nem sempre precisa de um `JinkerResponse` explícito. O jinker converte
automaticamente:

| Você faz `return ...` | Vira |
|---|---|
| `{"a": 1}` (dict) ou `[1,2,3]` (lista) | JSON, status `200` |
| `"texto"` (string) | texto puro, status `200` |
| `Null` | resposta vazia, status `204` |
| `jsonify(dados)` | JSON (é um `JinkerResponse`) |
| `render("arquivo")` | conteúdo do arquivo com MIME correto |
| `(qualquer_um, 201)` (tupla) | o mesmo, mas com o status da tupla |

Ou seja: pra JSON simples, `return {"ok": True}` basta. `JinkerResponse` entra
quando você quer **encadear** ajustes.

O que não vira resposta é erro do handler, `500` com a frase no terminal do
servidor: o dict ou a lista que não vira JSON, e a tupla cujo status não é
inteiro de 100 a 999 (a regra do [`.status()`](status/status.md)). Ver a
[rota](../route/route.md#o-que-a-funct-pode-retornar).

---

## Métodos (todos devolvem o próprio objeto — encadeáveis)

| Método | O que faz | Doc |
|---|---|---|
| `.json(dados, status=200)` | corpo JSON (`application/json`) | [json/json.md](json/json.md) |
| `.send(texto, status=200)` | corpo texto puro (`text/plain`) | [send/send.md](send/send.md) |
| `.status(codigo)` | troca só o status | [status/status.md](status/status.md) |
| `.header(chave, valor)` | adiciona um cabeçalho HTTP | [header/header.md](header/header.md) |
| `.cookie(nome, valor, ...)` | acrescenta um `Set-Cookie` (um por chamada) | [cookie/cookie.md](cookie/cookie.md) |

Como cada um devolve o próprio `JinkerResponse`, dá pra encaixar em cadeia:

```ps
funct cria() {
    return JinkerResponse()
        .json({"criado": True})
        .status(201)
        .header("Location", "/itens/10")
        .cookie("sid", token, max_age=3600)
}
```

---

## Campo (sem parênteses)

| Campo | Tipo | O que é | Doc |
|---|---|---|---|
| `status_code` | `int` | o código HTTP; **lê e escreve** (`resp.status_code = 201`) | [status_code/status_code.md](status_code/status_code.md) |

Ele e o método `.status(codigo)` mexem no mesmo número. A diferença é o
retorno: `.status(201)` devolve a resposta e encadeia; `status_code = 201` é
atribuição e não encadeia.

---

## Montando na mão vs. atalhos

```ps
funct rota() {
    # atalho (dia a dia):
    return jsonify({"ok": True})

    # equivalente, na mão:
    return JinkerResponse().json({"ok": True})

    # na mão faz sentido quando você combina várias coisas:
    return JinkerResponse()
        .send("relatorio,linha1\n", 200)
        .header("Content-Type", "text/csv")
        .header("Content-Disposition", "attachment; filename=\"r.csv\"")
}
```

---

## Relacionados

- [`jsonify`](../jsonify/jsonify.md) — atalho pra resposta JSON
- [`render`](../render/render.md) — resposta a partir de um arquivo
- [`request`](../request/request.md) — o lado de entrada (o que chegou)
- [cookie e sessão assinada](../cookie/cookie.md) — o `.cookie()` daqui junto
  com o `request.cookie()` e o `jwt`
