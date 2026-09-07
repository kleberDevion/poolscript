# `JinkerRequest`

> **Objeto interno da linguagem** — você não cria `JinkerRequest` na mão:
> é o TIPO de um objeto que a lib `jinker` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

Objeto de requisição disponível dentro da funct do handler.

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.file(field, allowed=None)` | Retorna um único arquivo do upload. |
| `.files(field, allowed=None)` | Retorna lista de arquivos do upload (múltiplos arquivos no mesmo campo). |
| `.get(key)` | Pega do query string ou do body JSON. |
| `.json()` |  |
| `.path_param(key)` | Retorna parâmetro dinâmico da URL. Ex: /user:<id> → request.path_param('id') |
| `.text()` |  |
| `.body_raw` | atributo |
| `.headers` | atributo |
| `.method` | atributo |
| `.path` | atributo |
| `.query` | atributo |

### `.file(...)`

Retorna um único arquivo do upload.

allowed: lista de extensões permitidas ex: [".jpg", ".png"]
         None = aceita qualquer extensão segura

[← índice](objetos-internos.md)
