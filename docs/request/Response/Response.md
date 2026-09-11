# `Response` — a resposta de uma requisição HTTP

`Response` é o que `request.get/post/put/patch/delete` devolvem. Guarda o
status, o corpo, os headers e a URL final, com métodos pra ler o corpo como
JSON.

---

## Propriedades

| Acesso | O que é |
|---|---|
| `.status` | código HTTP (`200`, `404`, `500`…). `-1` em falha de transporte. |
| `.status_code` | **alias** de `.status` |
| `.ok` | `true` se o status for `2xx` (sucesso) |
| `.text` | o corpo como texto UTF-8 (tolerante a bytes inválidos) |
| `.content` | o corpo em **bytes** crus, lido do arquivo **na hora em que você pede** — é aí, e só aí, que o corpo ocupa memória. Use pra binário (imagem, zip, pdf, exe); não corrompe |
| `.size` | bytes que chegaram, lido sem abrir o corpo |
| `.headers` | dict com os cabeçalhos da resposta |
| `.url` | a URL final (após redirecionamentos) |
| `.filename` | nome sugerido pelo servidor (`Content-Disposition`) ou o fim da URL |

---

## Métodos pra ler o corpo

| Método | Devolve |
|---|---|
| `.json()` | o corpo parseado como dict/lista (`Null` se não for JSON válido) |
| `.get_json(chave=None)` | o JSON inteiro, ou só uma chave dele |
| `.get(chave)` | um **header** (case-insensitive) **ou** uma chave do JSON |

---

## Binário e download

| Método | O que faz |
|---|---|
| `.decode(encoding)` | corpo como texto num encoding específico — ex: `.decode("latin-1")` (o `.text` já assume UTF-8) |
| `.content_type(esperado)` | valida o `Content-Type`: **erra** (cai no `try/catch`) se não bater. Encadeável — devolve o próprio `Response`. |
| `.save(caminho)` | **move** o arquivo do corpo pro caminho (`rename`, sem passar pela memória) e devolve um [`PoolFile`](../../os/PoolFile/PoolFile.md) fechado — `.name`, `.size`, `.path()`, `.move()`, `.copy()`, `.delete()`; `.bytes()` lê do disco só se chamado. Se o corpo veio de `save=`, ou já foi salvo antes, **copia** e os dois ficam. Se `caminho` for uma pasta (`"."`, termina em `/`, ou já existe como dir), o nome vem do `.filename`; senão usa o caminho dado. |

```
# baixar um arquivo — nome vem do servidor
resp = request.get("https://exemplo.com/relatorio.pdf")
arq = resp.save(".")                 # PoolFile
post("salvo:", arq.name, "-", arq.size, "bytes")

# validar o tipo antes de salvar (erra se não for PDF)
try {
    request.get(url).content_type("application/pdf").save("doc.pdf")
}
catch (e) {
    post("não é PDF:", e)
}

# binário na mão, sem salvar
bruto = request.get(url).content    # bytes
```

---

## Uso típico

```
import request

resp = request.get("https://api.x.com/user/1")

if (resp.ok) {                        # status 2xx?
    dados = resp.json()               # dict
    post(dados["nome"])
} else {
    post("erro", resp.status)
}
```

### `.get_json()` — inteiro ou uma chave

```
resp.get_json()             # {"nome": "ana", "idade": 30}
resp.get_json("nome")       # "ana"  (atalho pra uma chave)
```

### `.get()` — serve pra header e pra chave JSON

```
tipo = resp.get("Content-Type")   # procura primeiro nos headers
nome = resp.get("nome")           # se não for header, procura no JSON
```

### `.text` — corpo cru

Quando a resposta não é JSON (HTML, texto, XML):

```
resp = request.get("https://exemplo.com/pagina.html")
post(resp.text)             # o HTML como string
```

---

## Relacionados

- [`request.get()`](../get/get.md) / [`request.post()`](../post/post.md) — o que devolvem um `Response`
- lib `json` — parsear/gerar JSON manualmente
