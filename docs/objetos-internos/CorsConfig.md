# `CorsConfig`

> **Objeto interno da linguagem** — você não cria `CorsConfig` na mão:
> é o TIPO de um objeto que a lib `jinker` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

Configuração global de CORS. Instância única por aplicação.

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.options(subset=None)` | cors.options()           → todos os métodos configurados |
| `.origins()` | cors.origins() → lista de origens configuradas (usado em auth=). |
| `.permiser()` | apelido legado de `.origins()` |
| `.local()` | cors.local() → `false` quando foi configurado `cors(app, ..., local=false)` (modo estrito: só a lista de origens vale, nem localhost nem cliente sem `Origin`). |

### `.options(...)`

cors.options()           → todos os métodos configurados
cors.options(["POST"])   → filtra/sobrescreve para a rota

[← índice](objetos-internos.md)
