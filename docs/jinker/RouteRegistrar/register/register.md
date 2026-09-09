# `RouteRegistrar.register(handler)`

Grava o `handler` no app: é o passo que transforma o registrador descartável
numa rota de verdade. É o **único** membro do
[`RouteRegistrar`](../RouteRegistrar.md), e quem o chama, no uso normal, é o
próprio decorador.

```
register(handler: funct) -> Null
```

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `handler` | funct | — | a funct que vai atender; **obrigatório**, e é o único |

O nome é este: `register(hendler=...)` é
`'hendler' is an invalid keyword argument for register()`. Zero argumentos é
`TypeError: register() takes exactly one argument (0 given)`, e dois é
`register() takes at most 1 argument (2 given)`.

O `handler` **não é conferido no registro**: `register(42)` passa sem erro. O
que ele precisa ser é uma funct que o servidor possa chamar quando a
requisição chegar.

## Retorno

**`Null`** — sempre, nos três feitios de registrador. Ele grava e não devolve
nada: nem o app, nem a rota, nem o próprio registrador.

```ps
vol = app.get("/x").register(trata)
post(type(vol))     # Null
post(vol)           # Null
```

Por isso **não encadeia**: `app.get("/x").register(f).register(g)` é
`'Null' object has no attribute 'register'`. Pra registrar duas rotas, são
dois registradores.

## O que ele grava, por feitio

| registrador | o que `register` faz |
|---|---|
| rota (`route`, `get`, `post`, `put`, `patch`, `delete`) | acrescenta a rota à lista do app, com o path, os métodos, o `auth`, o `middleware` e o `model` que estavam no registrador |
| socket (`app.socket("/ws")`) | acrescenta o handler daquele WebSocket |
| middleware (`app.middleware()`) | grava o middleware do app — é **um campo único**, então um segundo `register` substitui o anterior |

O `route_prefix` do [`Jinker(...)`](../../Jinker/Jinker.md) é aplicado **aqui**,
na hora de gravar: o path que entra na lista já vem com o prefixo colado.

## Chamar na mão

Dá, e é exatamente o que o decorador faz:

```ps
from jinker import Jinker, jsonify

app = Jinker(__name__)

funct trata() {
    return jsonify({"ok": true})
}

app.get("/manual").register(trata)      # idêntico a @app.get("/manual")
```

Serve pra registrar uma funct que já existe, ou pra registrar em laço:

```ps
for each rota in [["/a", trata_a], ["/b", trata_b]] {
    app.get(rota[0]).register(rota[1])
}
```

## Relacionados

- [`RouteRegistrar`](../RouteRegistrar.md) — o objeto e os três feitios
- [`@app.route(...)`](../../route/route.md) — o que devolve o registrador
- [decoradores](../../../linguagem/14-decoradores.md) — o protocolo
  `@objeto.metodo(...)` nas três posições
