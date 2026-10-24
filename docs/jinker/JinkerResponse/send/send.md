# `JinkerResponse.send(text, status=200)`

Define o corpo da resposta como **texto puro** e o `Content-Type` como
`text/plain`. Devolve o próprio `JinkerResponse` (encadeável).

```assinatura
send(text: str, status: int = 200) -> JinkerResponse
```

| Parâmetro | Tipo | Padrão | O que é |
|---|---|---|---|
| `text` | `str` | — | conteúdo textual do corpo |
| `status` | `int` | `200` | código HTTP da resposta |

Os exemplos desta página pressupõem:

```ps contexto
import os
from jinker import Jinker, JinkerResponse, cors, render
Jinker app = Jinker(__name__)
```

---

## Quando usar `.send()` em vez de `.json()`

Use `.send()` quando a resposta **não é JSON**: texto simples, um CSV, um
trecho de HTML montado na mão, um "pong", etc.

```ps
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

```ps
funct pagina() {
    return JinkerResponse()
        .send("<h1>Olá</h1>", 200)
        .header("Content-Type", "text/html; charset=utf-8")
}
```

---

## Status code junto

```ps
funct proibido() {
    return JinkerResponse().send("acesso negado", 403)
}
```

O status segue a regra do [`.status()`](../status/status.md), inteiro de 100
a 999: o texto é `TypeError`, e o número fora da faixa é `ValueError`.
`Null`, ou o segundo argumento ausente, não mexe no status que já estava.

```ps
JinkerResponse().send("acesso negado", 4030)
# ValueError: send(): codigo HTTP fora da faixa 100..999: 4030
```

---

## `.send()` manda o TEXTO que você passa — não lê arquivo

Cuidado com uma confusão comum: `.send(x)` envia **exatamente `x` como texto**.
Se `x` for o caminho de um arquivo, o cliente recebe o **caminho** (a string),
**não o conteúdo do arquivo**:

```ps
# ERRADO — envia a string "C:\projeto\dados.csv", não o arquivo:
funct baixar() {
    path = os.pathFile("dados.csv")
    return JinkerResponse().send(path)      # cliente recebe: C:\projeto\dados.csv
}
```

Pra mandar o **conteúdo de um arquivo**, quem lê o disco é o
[`render()`](../../render/render.md), não o `send()`:

```ps
# CERTO — render() lê o arquivo e manda o conteúdo (com MIME certo):
funct baixar() {
    return render("dados.csv")
}
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

```ps
funct ping() {
    return "pong"
    return JinkerResponse().send("pong")
}
```

Use a forma longa quando precisar encadear `.status()`/`.header()`.

---

## Relacionados

- [`.json(dados, status)`](../json/json.md) — corpo JSON
- [`.header(chave, valor)`](../header/header.md) — adiciona cabeçalhos
- [`.status(codigo)`](../status/status.md) — muda só o status
