# `MiddlewareRegistrar` — não existe

Esta página descrevia um tipo `MiddlewareRegistrar` com `.register(handler)`.
**Ele não existe**: o nome não aparece em `pool --metadata`, e o que
`app.middleware` vale é uma função:

```ps
from jinker import Jinker
Object app = Jinker(__name__)
post(type(app.middleware))    # funct
```

Não há objeto registrador nem método `.register`. O middleware se declara com o
decorador, e é isso:

```ps
from jinker import Jinker, jsonify, request
app = Jinker(__name__)

@app.middleware()
funct exige_token() {
    if request.header("Authorization") == Null {
        return jsonify({"erro": "sem token"}, 401)
    }
}
```

Está documentado em [`middleware`](../jinker/middleware/middleware.md).

[← índice](objetos-internos.md)
