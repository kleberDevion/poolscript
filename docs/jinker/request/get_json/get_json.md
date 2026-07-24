# `request.get_json()`

Devolve o **corpo inteiro** da requisição parseado de JSON — normalmente um
dict (objeto) ou uma lista.

```
request.get_json() -> dict | list | Null
```

---

## Uso

`POST` com corpo `{"nome": "ana", "idade": 30}`:

```
@app.route("/usuario", methods=cors.options(["POST"]))
action criar() {
    data = request.get_json()        // {"nome": "ana", "idade": 30}
    return jsonify({"criado": data["nome"]})
}
```

---

## Corpo inválido ou vazio → `Null`

Se o corpo não for JSON válido (ou estiver vazio), devolve `Null`. Cheque antes
de indexar:

```
data = request.get_json()
if (data is Null) {
    return jsonify({"erro": "corpo JSON inválido"}), 400
}
```

---

## `.get_json()` vs `.get()`

- `.get_json()` — o objeto **inteiro** (bom quando você precisa de vários campos).
- [`.get("nome")`](../get/get.md) — só **um** campo (query string ou JSON).

```
// preciso de vários → get_json
data = request.get_json()
nome = data["nome"]
email = data["email"]

// preciso de um → get
nome = request.get("nome")
```

---

## Relacionados

- [`.get()`](../get/get.md) — um campo só
- [`.text()`](../text/text.md) — corpo cru, sem parsear
