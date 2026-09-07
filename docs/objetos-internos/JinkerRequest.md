# `JinkerRequest` — vazio de propósito

O tipo existe e é criável (`JinkerRequest()`), mas **não tem membro nenhum**:
`pool --metadata` publica a lista vazia, e qualquer acesso é
`AttributeError: 'JinkerRequest' object has no attribute '…'`.

Esta página listava `.file`, `.files`, `.get`, `.json`, `.path_param`, `.text`,
`.body_raw`, `.headers`, `.method`, `.path` e `.query` — **os onze dão
`AttributeError`**.

Dentro da rota e do socket, o `request` **não é** um `JinkerRequest`:

```ps
@app.get("/x")
funct h() {
    post(type(request))    # RequestProxy
    return {"ok": true}
}
```

A API de requisição está em [`RequestProxy`](RequestProxy.md). Dela, dois nomes
que esta página prometia **não existem em objeto nenhum**: `body_raw` e
`query` — para o corpo cru use `request.text()`, e para a query string
`request.get(chave)`.

[← índice](objetos-internos.md)
