# `JinkerResponse.status(code)`

Troca **só o código HTTP** da resposta, sem mexer no corpo nem no
`Content-Type`. Devolve o próprio `JinkerResponse` (encadeável).

```
status(code: int) -> JinkerResponse
```

| Parâmetro | Tipo | O que é |
|---|---|---|
| `code` | `int` | código HTTP (ex: `200`, `201`, `404`, `500`) |

---

## Pra que serve

Às vezes você monta o corpo com `jsonify(...)` (que assume `200`) mas quer um
código diferente. `.status(...)` ajusta sem precisar da tupla `(resp, codigo)`:

```
# com tupla:
return jsonify({"criado": true}), 201

# com .status() — dispensa a tupla:
return jsonify({"criado": true}).status(201)
```

As duas produzem `201 Created`. Escolha a que ficar mais legível — quando já
está encadeando `.header(...)`, `.status()` costuma ler melhor.

---

## Códigos HTTP mais comuns

| Código | Significado | Quando usar |
|---|---|---|
| `200` | OK | sucesso padrão (é o default) |
| `201` | Created | criou um recurso (POST que insere) |
| `202` | Accepted | recebido, vai processar depois |
| `204` | No Content | sucesso sem corpo (é o que `return None` gera) |
| `400` | Bad Request | dados inválidos que o cliente mandou |
| `401` | Unauthorized | falta autenticação/token |
| `403` | Forbidden | autenticado, mas sem permissão |
| `404` | Not Found | recurso não existe |
| `409` | Conflict | o estado atual não deixa (duplicado, versão velha) |
| `422` | Unprocessable Content | bem formado, mas não faz sentido |
| `500` | Internal Server Error | erro no servidor |
| `502` | Bad Gateway | o serviço atrás respondeu mal |
| `503` | Service Unavailable | fora do ar / manutenção |
| `504` | Gateway Timeout | o serviço atrás não respondeu a tempo |

Código sem frase própria recebe a da classe na linha de status
(`Success`, `Redirection`, `Client Error`, `Server Error`): `418` sai
`418 Client Error`, nunca `418 OK`.

---

## Encadeando

```
return jsonify({"erro": "sem permissão"})
    .status(403)
    .header("X-Reason", "token-expirado")
```

---

## Relacionados

- [`.json(dados, status)`](../json/json.md) — define corpo JSON (aceita status direto)
- [`.send(texto, status)`](../send/send.md) — define corpo texto (aceita status direto)
- [`.header(chave, valor)`](../header/header.md) — adiciona cabeçalhos
