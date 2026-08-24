# `JinkerResponse.header(key, value)`

Adiciona **um cabeçalho HTTP** (header) à resposta. Devolve o próprio
`JinkerResponse`, então você pode encadear (`.header(...).header(...)`) e
juntar com `.json()`/`.send()`/`.status()`.

```
header(key: str, value: str) -> JinkerResponse
```

| Parâmetro | Tipo | O que é |
|---|---|---|
| `key` | `str` | nome do header (ex: `"X-Request-Id"`, `"Cache-Control"`) |
| `value` | `str` | valor do header (ex: `"abc123"`, `"no-cache"`) |

---

## O que é um "header" (pra quem nunca mexeu)

Toda resposta HTTP tem duas partes: o **corpo** (o conteúdo — seu JSON, HTML,
etc.) e os **cabeçalhos** (informações *sobre* a resposta, que o corpo não
carrega). O navegador lê os cabeçalhos antes do corpo pra decidir como tratar
o conteúdo.

Exemplos de coisas que vivem em headers, não no corpo:

- por quanto tempo o navegador pode guardar (cache) a resposta;
- um identificador da requisição pra você rastrear em logs;
- se o conteúdo deve ser baixado como arquivo em vez de exibido;
- um token, uma versão de API, etc.

`.header(key, value)` é como você adiciona qualquer um desses.

---

## Exemplo mínimo

```
@app.route("/api/dado", methods=cors.options(["GET"]))
action dado() {
    return jsonify({"ok": true}).header("X-Request-Id", "req-42")
}
```

A resposta sai com o corpo `{"ok": true}` **e** um cabeçalho
`X-Request-Id: req-42`. O cliente recebe os dois.

---

## Encadeando vários headers

Como cada `.header(...)` devolve a própria resposta, é só continuar chamando:

```
return jsonify({"itens": lista})
    .header("X-Total-Count", "3")
    .header("Cache-Control", "no-cache")
    .header("X-Api-Version", "2")
```

A ordem não importa — todos entram na resposta.

---

## Combinando com status code

`.header(...)` não mexe no status. Pra mudar o código, use a tupla
`(resposta, codigo)` **ou** `.status(codigo)`:

```
// forma 1 — tupla no return
return jsonify({"criado": true}).header("Location", "/itens/10"), 201

// forma 2 — .status() dispensa a tupla
return jsonify({"criado": true}).header("Location", "/itens/10").status(201)
```

As duas produzem uma resposta `201 Created` com o header `Location`.

---

## Casos de uso reais

**Forçar download de um arquivo do disco** (em vez de abrir no navegador) —
quem lê o arquivo é o `render()`; o header `Content-Disposition: attachment` é
o que manda o navegador **baixar**:

```
action baixar() {
    return render("relatorios/vendas.csv")
        .header("Content-Disposition", "attachment; filename=\"vendas.csv\"")
}
```

> Se em vez de um arquivo você **gerou** o conteúdo na hora (uma string
> montada no código), use `.send(texto)` no lugar do `render(...)`:
> `JinkerResponse().send(meu_texto).header("Content-Disposition", "attachment; filename=\"x.csv\"")`.

**Rastrear requisições com um ID nos logs:**

```
action processar() {
    rid = request.get("request_id")
    // ... faz o trabalho ...
    return jsonify({"status": "ok"}).header("X-Request-Id", rid)
}
```

**Controlar cache de uma resposta que muda toda hora:**

```
return jsonify({"agora": date.now()}).header("Cache-Control", "no-store")
```

---

## Detalhes de comportamento (verificados no código)

- **O valor precisa ser string.** Se você tem um número, converta: `str(total)`
  — o header espera texto (`"3"`, não `3`).
- **Chamar `.header(...)` duas vezes com a MESMA chave sobrescreve** — fica só
  o último valor (os headers são guardados num dict interno).
- **Alguns headers o jinker já define sozinho** e você normalmente não precisa
  tocar: `Content-Type` (definido por `.json()` = JSON, `.send()` = texto),
  `Content-Length` e os de CORS (`Access-Control-Allow-*`). Você *pode*
  sobrescrever o `Content-Type` com `.header("Content-Type", ...)` se precisar
  (como no exemplo do CSV acima).

---

## Relacionados

- [`.json(dados, status)`](../json/json.md) — corpo JSON
- [`.send(texto, status)`](../send/send.md) — corpo texto puro
- [`.status(codigo)`](../status/status.md) — muda só o status
- [`JinkerResponse`](../JinkerResponse.md) — visão geral do objeto de resposta
