# `jsonify(data)`

Atalho para criar uma resposta JSON. Devolve um
[`JinkerResponse`](../JinkerResponse/JinkerResponse.md) com o corpo já
serializado e o `Content-Type` marcado como `application/json`.

```ps
from jinker import jsonify
```

```assinatura
jsonify(data: any) -> JinkerResponse
```

| Parâmetro | Tipo | O que é |
|---|---|---|
| `data` | dict / lista / valor | o que vira JSON no corpo da resposta |

Os exemplos desta página pressupõem:

```ps contexto
from jinker import Jinker, JinkerResponse, cors, jsonify
Jinker app = Jinker(__name__)
list lista = ["a", "b", "c"]
```

---

## É literalmente um atalho

`jsonify(data)` é `JinkerResponse().json(data)` — nada mais. As duas linhas
abaixo produzem exatamente a mesma resposta:

```ps
funct rota() {
    return jsonify({"msg": "ok"})
    return JinkerResponse().json({"msg": "ok"})
}
```

Use `jsonify` por ser mais curto. Veja [`.json()`](../JinkerResponse/json/json.md)
para os detalhes do que acontece por baixo.

---

## Uso típico

```ps
@app.route("/api/user", methods=cors.options(["GET"]))
funct user() {
    return jsonify({"nome": "ana", "idade": 30})
}
```

Resposta: corpo `{"nome": "ana", "idade": 30}`, `Content-Type: application/json`,
status `200`.

---

## Com status code

`jsonify` sempre nasce `200`. Pra outro código, use a tupla ou encadeie
`.status()`:

```ps
funct rota() {
    return jsonify({"erro": "não encontrado"}), 404
    return jsonify({"erro": "não encontrado"}).status(404)
}
```

---

## Encadeando headers

Como devolve um `JinkerResponse`, dá pra continuar:

```ps
funct rota() {
    return jsonify({"itens": lista}).header("X-Total-Count", "3"), 200
}
```

---

## Sem usar `jsonify`

Se você só quer JSON simples com status 200, devolver o dict/lista direto já
funciona — a rota converte sozinha:

```ps
funct rota() {
    return {"msg": "ok"}      # idêntico a jsonify({"msg": "ok"})
}
```

`jsonify` passa a valer a pena quando você quer status diferente ou headers.

---

## Relacionados

- [`JinkerResponse`](../JinkerResponse/JinkerResponse.md) — o objeto que ele cria
- [`.json()`](../JinkerResponse/json/json.md) — o método que ele chama
- [`render`](../render/render.md) — resposta a partir de um arquivo (não-JSON)
