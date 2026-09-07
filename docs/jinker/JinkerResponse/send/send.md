# `JinkerResponse.send(text, status=200)`

Define o corpo da resposta como **texto puro** e o `Content-Type` como
`text/plain`. Devolve o próprio `JinkerResponse` (encadeável).

```
send(text: str, status: int = 200) -> JinkerResponse
```

| Parâmetro | Tipo | Padrão | O que é |
|---|---|---|---|
| `text` | `str` | — | conteúdo textual do corpo |
| `status` | `int` | `200` | código HTTP da resposta |

---

## Quando usar `.send()` em vez de `.json()`

Use `.send()` quando a resposta **não é JSON**: texto simples, um CSV, um
trecho de HTML montado na mão, um "pong", etc.

```
@app.route("/ping", methods=cors.options(["GET"]))
funct ping() {
    return JinkerResponse().send("pong")
}
```

Não existe atalho global tipo `jsonify` pra texto — você usa
`JinkerResponse().send(...)` direto (ou `return "pong"`, que a rota já trata
como texto puro automaticamente).

---

## Trocando o Content-Type

`.send()` marca `text/plain`. Se o texto for outro formato (CSV, HTML, XML),
sobrescreva o header depois:

```
return JinkerResponse()
    .send("<h1>Olá</h1>", 200)
    .header("Content-Type", "text/html; charset=utf-8")
```

---

## Status code junto

```
return JinkerResponse().send("acesso negado", 403)
```

---

## `.send()` manda o TEXTO que você passa — não lê arquivo

Cuidado com uma confusão comum: `.send(x)` envia **exatamente `x` como texto**.
Se `x` for o caminho de um arquivo, o cliente recebe o **caminho** (a string),
**não o conteúdo do arquivo**:

```
# ERRADO — envia a string "C:\projeto\dados.csv", não o arquivo:
path = os.pathFile("dados.csv")
return JinkerResponse().send(path)      # cliente recebe: C:\projeto\dados.csv
```

Pra mandar o **conteúdo de um arquivo**, quem lê o disco é o
[`render()`](../../render/render.md), não o `send()`:

```
# CERTO — render() lê o arquivo e manda o conteúdo (com MIME certo):
return render("dados.csv")
```

Resumindo:

| Quero mandar… | Uso |
|---|---|
| um texto que eu montei na hora | `.send(text)` |
| o conteúdo de um arquivo do disco | `render("arquivo")` |

`.send()` é pra texto que você **já tem na mão** (uma mensagem, um resultado
calculado). `render()` é "vai no disco, lê o arquivo e manda".

---

## Diferença pra devolver uma string direto

Estas duas fazem a mesma coisa (texto puro, 200):

```
return "pong"
return JinkerResponse().send("pong")
```

Use a forma longa quando precisar encadear `.status()`/`.header()`.

---

## Relacionados

- [`.json(dados, status)`](../json/json.md) — corpo JSON
- [`.header(chave, valor)`](../header/header.md) — adiciona cabeçalhos
- [`.status(codigo)`](../status/status.md) — muda só o status
