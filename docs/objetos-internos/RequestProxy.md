<!-- gerado: gera_doc_gaps.py — pode regenerar -->
# `RequestProxy`

> **Objeto interno da linguagem** — você não cria `RequestProxy` na mão:
> é o TIPO de um objeto que a lib `jinker` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

Proxy do `request` para uso com `import request` na PoolScript.
Os métodos delegam pro JinkerRequest atual injetado no escopo pela rota.
O interpretador injeta o objeto real em _current antes de chamar a action.

Usa threading.local() para garantir isolamento entre requisições concorrentes:
cada thread tem seu próprio _current, evitando race condition quando rotas
fazem chamadas internas (ex: webhook que chama outra rota do mesmo servidor).

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.file(field, allowed=None)` | request.file('campo', allowed=['.jpg', '.png']) → PoolFileUpload |
| `.files(field, allowed=None)` | request.files('campo') → lista de PoolFileUpload |
| `.get(key)` |  |
| `.get_json()` |  |
| `.header(key)` | request.header('User-Agent') → valor do header (case-insensitive), |
| `.headers` |  |
| `.json()` | Alias de get_json() — o corpo do POST como dict. O JinkerRequest usa |
| `.method` |  |
| `.path` |  |
| `.path_param(key)` | request.path_param('id') → valor do parâmetro dinâmico da rota/socket. |
| `.text()` |  |

### `.header(...)`

request.header('User-Agent') → valor do header (case-insensitive),
ou Null se não existir. Atalho pra request.headers sem precisar do dict
inteiro nem se preocupar com maiúsculas/minúsculas.

### `.json(...)`

Alias de get_json() — o corpo do POST como dict. O JinkerRequest usa
`.json()`, então o proxy aceita os dois nomes (request.json() e
request.get_json()) pra não pegar ninguém de surpresa.

### `.path_param(...)`

request.path_param('id') → valor do parâmetro dinâmico da rota/socket.
Ex: /sala:id ou /user/<id>

[← índice](objetos-internos.md)
