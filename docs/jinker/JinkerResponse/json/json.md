# `JinkerResponse.json(dados, status=200)`

Define o corpo da resposta como **JSON** e o `Content-Type` como
`application/json`. Devolve o próprio `JinkerResponse` (encadeável).

```
json(dados: any, status: int = 200) -> JinkerResponse
```

| Parâmetro | Tipo | Padrão | O que é |
|---|---|---|---|
| `dados` | dict / lista / valor | — | o que vira JSON no corpo |
| `status` | `int` | `200` | código HTTP da resposta |

---

## É o que o `jsonify` faz por baixo

`jsonify(dados)` é literalmente `JinkerResponse().json(dados)`. Então estas
duas linhas são idênticas:

```
return jsonify({"msg": "ok"})
return JinkerResponse().json({"msg": "ok"})
```

Use `jsonify` no dia a dia (é mais curto). Use `.json()` direto quando já tem
um `JinkerResponse` na mão e quer definir/trocar o corpo.

---

## Status code junto

O segundo argumento define o código já na chamada:

```
return JinkerResponse().json({"erro": "não encontrado"}, 404)
```

Equivale a `jsonify({"erro": "não encontrado"}), 404`.

---

## O que pode ir em `dados`

Qualquer coisa serializável em JSON: dict, lista, string, número, bool, `Null`
(vira `null`), e combinações aninhadas.

```
.json({"usuario": {"nome": "ana", "tags": [1, 2, 3], "ativo": true}})
```

Acentos são preservados (`ensure_ascii=false` internamente) — `"ção"` sai como
`"ção"`, não escapado.

---

## Relacionados

- [`jsonify(dados)`](../../jsonify/jsonify.md) — o atalho
- [`.send(texto, status)`](../send/send.md) — corpo texto puro
- [`.header(chave, valor)`](../header/header.md) — adiciona cabeçalhos
- [`.status(codigo)`](../status/status.md) — muda só o status
