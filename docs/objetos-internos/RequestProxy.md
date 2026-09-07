# `RequestProxy`

> **Objeto interno da linguagem** — você não cria `RequestProxy` na mão:
> é o TIPO de um objeto que a lib `jinker` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

É o que o nome `request` vale **dentro de uma rota ou de um socket**:
`type(request)` ali devolve `RequestProxy`. O motor põe a requisição corrente
no lugar antes de chamar a sua `funct`.

O isolamento entre requisições concorrentes vem do próprio desenho da VM: a
requisição corrente é um campo da VM (`vm->jk_req`), e cada fibra tem o seu
estado — não há tabela por thread nem trava. Uma rota que chama outra rota do
mesmo servidor não embaralha as duas.

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.file(field, allowed=None)` | request.file('campo', allowed=['.jpg', '.png']) → PoolFileUpload |
| `.files(field, allowed=None)` | request.files('campo') → lista de PoolFileUpload |
| `.get(key)` |  |
| `.get_json()` |  |
| `.header(key)` | request.header('User-Agent') → valor do header (case-insensitive), |
| `.cookie(nome)` | valor do cookie enviado pelo cliente — ver [cookie](../jinker/cookie/cookie.md) |
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

Alias de `get_json()` — o corpo do POST como dict. Os dois nomes valem
(`request.json()` e `request.get_json()`).

### `.path_param(...)`

request.path_param('id') → valor do parâmetro dinâmico da rota/socket.
Ex: /sala:id ou /user/<id>

[← índice](objetos-internos.md)
