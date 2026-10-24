# `JinkerResponse.json(data, status=200)`

Define o corpo da resposta como **JSON** e o `Content-Type` como
`application/json`. Devolve o próprio `JinkerResponse` (encadeável).

```assinatura
json(data: any, status: int = 200) -> JinkerResponse
```

| Parâmetro | Tipo | Padrão | O que é |
|---|---|---|---|
| `data` | dict / lista / valor | — | o que vira JSON no corpo |
| `status` | `int` | `200` | código HTTP da resposta |

Os exemplos desta página pressupõem:

```ps contexto
from jinker import JinkerResponse, jsonify
```

---

## É o que o `jsonify` faz por baixo

`jsonify(data)` é literalmente `JinkerResponse().json(data)`. Então estas
duas linhas são idênticas:

```ps
funct rota() {
    return jsonify({"msg": "ok"})
    return JinkerResponse().json({"msg": "ok"})
}
```

Use `jsonify` no dia a dia (é mais curto). Use `.json()` direto quando já tem
um `JinkerResponse` na mão e quer definir/trocar o corpo.

---

## Status code junto

O segundo argumento define o código já na chamada:

```ps
funct rota() {
    return JinkerResponse().json({"erro": "não encontrado"}, 404)
}
```

Equivale a `jsonify({"erro": "não encontrado"}), 404`.

O status segue a regra do [`.status()`](../status/status.md), inteiro de 100
a 999: o texto é `TypeError`, e o número fora da faixa é `ValueError`.
`Null`, ou o segundo argumento ausente, não mexe no status que já estava (o
`.status(418).json(...)` continua 418).

```ps
JinkerResponse().json({"erro": "não encontrado"}, "404")
# TypeError: 'str' object cannot be interpreted as an integer
```

```ps
JinkerResponse().json({"ok": True}, 2000)
# ValueError: json(): codigo HTTP fora da faixa 100..999: 2000
```

---

## O que pode ir em `data`

Qualquer coisa serializável em JSON: dict, lista, string, número, bool, `Null`
(vira `Null`), e combinações aninhadas.

```ps
JinkerResponse()
    .json({"usuario": {"nome": "ana", "tags": [1, 2, 3], "ativo": True}})
```

Acentos são preservados (`ensure_ascii=False` internamente) — `"ção"` sai como
`"ção"`, não escapado.

O que não vira JSON é erro, como no `json.stringify`: `TypeError` pro valor
que não serializa, `ValueError` pro `nan` e o infinito.

```ps
JinkerResponse().json({"media": flo("nan")})
# ValueError: Out of range float values are not JSON compliant: nan
```

---

## Relacionados

- [`jsonify(dados)`](../../jsonify/jsonify.md) — o atalho
- [`.send(texto, status)`](../send/send.md) — corpo texto puro
- [`.header(chave, valor)`](../header/header.md) — adiciona cabeçalhos
- [`.status(codigo)`](../status/status.md) — muda só o status
