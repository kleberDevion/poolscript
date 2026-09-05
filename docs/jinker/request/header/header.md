# `request.header(key)`

Devolve **um** cabeçalho da requisição pelo nome, ou `Null` se não existir. A
busca é **case-insensitive** — headers HTTP não distinguem maiúsculas de
minúsculas, então `"Content-Type"`, `"content-type"` e `"CONTENT-TYPE"` acham a
mesma coisa.

```
request.header(key: str) -> str | Null
```

---

## Uso

```
@app.route("/api", methods=["GET"]) {
    action api() {
        token = request.header("Authorization")   # "Bearer abc..." ou Null
        tipo  = request.header("content-type")     # mesma coisa que "Content-Type"
        return jsonify({"tem_token": token is not Null})
    }
}
```

---

## `.header(key)` vs `.headers`

- `.header("X")` — **um** header, sem se preocupar com a caixa. `Null` se faltar.
- `.headers` — o **dict inteiro** dos cabeçalhos, com as chaves como o cliente
  mandou (aí a caixa importa, e `.get()` também é case-sensitive).

```
# um header, jeito recomendado
ua = request.header("User-Agent")

# o dict inteiro (ex: logar tudo)
todos = request.headers            # {"Host": "...", "User-Agent": "...", ...}
```

---

## Ausente devolve `Null`

```
x = request.header("X-Nao-Existe")   # Null
if (x is Null) {
    return jsonify({"erro": "header obrigatório faltando"}), 400
}
```

---

## Setar header na RESPOSTA é outra coisa

`request.header()` **lê** o que o cliente mandou. Pra **escrever** um header na
resposta (inclusive o `Content-Type`/mime), use o `JinkerResponse`:

```
r = JinkerResponse()
r.send("<nota>oi</nota>")
r.header("Content-Type", "application/xml")   # seta na saída
return r
```

---

## Relacionados

- [`request`](../request.md) — tudo que chegou na requisição
- [`JinkerResponse`](../../JinkerResponse/JinkerResponse.md) — setar headers/mime na saída
