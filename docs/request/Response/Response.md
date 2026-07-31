# `Response` — a resposta de uma requisição HTTP

`Response` é o que `request.get/post/put/patch/delete` devolvem. Guarda o
status, o corpo, os headers e a URL final, com métodos pra ler o corpo como
JSON.

---

## Propriedades

| Acesso | O que é |
|---|---|
| `.status` | código HTTP (`200`, `404`, `500`…) |
| `.ok` | `true` se o status for `2xx` (sucesso) |
| `.text` | o corpo como texto cru (string) |
| `.headers` | dict com os cabeçalhos da resposta |
| `.url` | a URL final (após redirecionamentos) |

---

## Métodos pra ler o corpo

| Método | Devolve |
|---|---|
| `.json()` | o corpo parseado como dict/lista (`Null` se não for JSON válido) |
| `.get_json(chave=None)` | o JSON inteiro, ou só uma chave dele |
| `.get(chave)` | um **header** (case-insensitive) **ou** uma chave do JSON |

---

## Uso típico

```
import request

resp = request.get("https://api.x.com/user/1")

if (resp.ok) {                        // status 2xx?
    dados = resp.json()               // dict
    post(dados["nome"])
} else {
    post("erro", resp.status)
}
```

### `.get_json()` — inteiro ou uma chave

```
resp.get_json()             // {"nome": "ana", "idade": 30}
resp.get_json("nome")       // "ana"  (atalho pra uma chave)
```

### `.get()` — serve pra header e pra chave JSON

```
tipo = resp.get("Content-Type")   // procura primeiro nos headers
nome = resp.get("nome")           // se não for header, procura no JSON
```

### `.text` — corpo cru

Quando a resposta não é JSON (HTML, texto, XML):

```
resp = request.get("https://exemplo.com/pagina.html")
post(resp.text)             // o HTML como string
```

---

## Relacionados

- [`request.get()`](../get/get.md) / [`request.post()`](../post/post.md) — o que devolvem um `Response`
- lib `json` — parsear/gerar JSON manualmente
