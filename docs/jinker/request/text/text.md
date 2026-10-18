# `request.text()`

Devolve o **corpo cru** da requisição como texto (string), sem tentar parsear
como JSON.

```assinatura
request.text() -> str
```

Os exemplos desta página pressupõem:

```ps contexto
from jinker import Jinker, cors, jsonify
Jinker app = Jinker(__name__)
```

---

## Quando usar

Quando o corpo **não é JSON**: um XML, um texto puro, um webhook que manda
formato próprio, ou quando você quer o conteúdo exato pra processar na mão.

```ps
@app.route("/webhook", methods=cors.options(["POST"]))
funct webhook() {
    corpo = request.text()           # string com o corpo exato
    post(f"recebido: {corpo}")
    return jsonify({"ok": True})
}
```

---

## `.text()` vs `.get_json()`

- `.text()` — o corpo **como veio**, string crua.
- [`.get_json()`](../get_json/get_json.md) — o corpo **parseado** em dict/lista.

Se o corpo é JSON e você quer o objeto, use `.get_json()`. Se quer os bytes/
texto exatos (ou o formato não é JSON), use `.text()`.

---

## Relacionados

- [`.get_json()`](../get_json/get_json.md) — corpo parseado como JSON
- [`.get()`](../get/get.md) — um campo específico
