# `RouteRegistrar` — o registrador que o decorador consome

`RouteRegistrar` é o objeto devolvido por [`@app.route(...)`](../route/route.md)
e pelos cinco atalhos por verbo ([`get`](../get/get.md),
[`post`](../post/post.md), [`put`](../put/put.md), [`patch`](../patch/patch.md),
[`delete`](../delete/delete.md)). Ele existe por um motivo só: **segurar a
rota enquanto a `funct` de baixo ainda não foi definida**.

Você não o importa e, no uso normal, não o vê — ele nasce quando o decorador
é avaliado e morre uma linha depois, quando o decorador chama
[`.register(handler)`](register/register.md) nele.

```ps
reg = app.get("/x")
post(type(reg))     # _RouteRegistrar
post(reg)           # <RouteRegistrar ['GET'] /x>
```

O nome que `type()` devolve tem underline na frente (`_RouteRegistrar`): é
tipo interno, e o underline diz isso. Em `pool --metadata` o escopo aparece
como `RouteRegistrar`, sem o underline.

---

## O protocolo do decorador

`@app.get("/x")` em cima de uma `funct` é, em três passos:

```
1. avalia  app.get("/x")     ->  o registrador
2. define  funct de baixo
3. chama   registrador.register(a_funct)
```

O passo 3 é o que prende a `funct` na rota. É o **mesmo protocolo** nas três
posições em que um decorador desse tipo vale — funct solta, classe, e método
dentro de classe (ver [decoradores](../../linguagem/14-decoradores.md)):

```ps
@app.get("/funct")
funct f() { return {"ok": true} }        # registra f

@app.get("/classe")
class H() {
    funct handler(self) { return {"ok": true} }   # instancia H() e registra o 1º método
}
```

Como o registrador é descartável, dá pra fazer o mesmo na mão, sem decorador:

```ps
funct trata() {
    return jsonify({"ok": true})
}

app.get("/manual").register(trata)       # idêntico a @app.get("/manual")
```

---

## Membros

| membro | devolve | o que faz |
|---|---|---|
| [`.register(handler)`](register/register.md) | `Null` | grava a rota (ou o socket, ou o middleware) no app |

É **um só**. O registrador não é uma rota inspecionável: `reg.path`, `reg.methods`
e `reg.handler` não existem —
`'_RouteRegistrar' object has no attribute 'path'`. Nem há como listar as
rotas depois: `app.routes` também não existe (ver
[`Route`](../../objetos-internos/Route.md)).

---

## Os três registradores

O motor tem **um tipo** de registrador, com três feitios, e os três aceitam
`.register(handler)`. O que muda é onde o handler é gravado:

| de onde vem | `type()` | como aparece impresso | o handler vira |
|---|---|---|---|
| `app.route(...)`, `app.get/post/put/patch/delete(...)` | `_RouteRegistrar` | `<RouteRegistrar ['GET'] /x>` | uma rota |
| `app.socket("/ws")` | `_SocketRegistrar` | `<SocketRegistrar /ws>` | o handler daquele WebSocket |
| `app.middleware()` | `MiddlewareRegistrar` | `<MiddlewareRegistrar>` | o middleware do app |

---

## Relacionados

- [`.register(handler)`](register/register.md) — o único membro, em detalhe
- [`@app.route(...)`](../route/route.md) — a forma geral que devolve o registrador
- [decoradores](../../linguagem/14-decoradores.md) — o protocolo `@objeto.metodo(...)`
- [visão geral do jinker](../jinker.md)
