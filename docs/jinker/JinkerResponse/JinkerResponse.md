# `JinkerResponse` — o objeto de resposta

`JinkerResponse` é o que uma rota devolve pro cliente. Toda vez que você faz
`return jsonify(...)`, `return render(...)` ou `return "texto"`, o que vai por
baixo é um `JinkerResponse`.

Você raramente precisa criá-lo na mão — os atalhos (`jsonify`, `render`, ou só
devolver um dict/lista/string) cobrem quase tudo. Use direto (`JinkerResponse()`)
quando quiser montar a resposta em etapas ou combinar corpo + status + headers
com controle total.

```
from jinker import JinkerResponse
```

---

## Como a rota interpreta o que você retorna

Você nem sempre precisa de um `JinkerResponse` explícito. O jinker converte
automaticamente:

| Você faz `return ...` | Vira |
|---|---|
| `{"a": 1}` (dict) ou `[1,2,3]` (lista) | JSON, status `200` |
| `"texto"` (string) | texto puro, status `200` |
| `None` / `Null` | resposta vazia, status `204` |
| `jsonify(dados)` | JSON (é um `JinkerResponse`) |
| `render("arquivo")` | conteúdo do arquivo com MIME correto |
| `(qualquer_um, 201)` (tupla) | o mesmo, mas com o status da tupla |

Ou seja: pra JSON simples, `return {"ok": true}` basta. `JinkerResponse` entra
quando você quer **encadear** ajustes.

---

## Métodos (todos devolvem o próprio objeto — encadeáveis)

| Método | O que faz | Doc |
|---|---|---|
| `.json(dados, status=200)` | corpo JSON (`application/json`) | [json/json.md](json/json.md) |
| `.send(texto, status=200)` | corpo texto puro (`text/plain`) | [send/send.md](send/send.md) |
| `.status(codigo)` | troca só o status | [status/status.md](status/status.md) |
| `.header(chave, valor)` | adiciona um cabeçalho HTTP | [header/header.md](header/header.md) |

Como cada um devolve o próprio `JinkerResponse`, dá pra encaixar em cadeia:

```
return JinkerResponse()
    .json({"criado": true})
    .status(201)
    .header("Location", "/itens/10")
```

---

## Montando na mão vs. atalhos

```
// atalho (dia a dia):
return jsonify({"ok": true})

// equivalente, na mão:
return JinkerResponse().json({"ok": true})

// na mão faz sentido quando você combina várias coisas:
return JinkerResponse()
    .send("relatorio,linha1\n", 200)
    .header("Content-Type", "text/csv")
    .header("Content-Disposition", "attachment; filename=\"r.csv\"")
```

---

## Relacionados

- [`jsonify`](../jsonify/jsonify.md) — atalho pra resposta JSON
- [`render`](../render/render.md) — resposta a partir de um arquivo
- [`request`](../request/request.md) — o lado de entrada (o que chegou)
