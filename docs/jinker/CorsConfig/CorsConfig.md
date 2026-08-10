# `CorsConfig`

Configuração global de CORS. Instância única por aplicação.

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.options(subset=None)` | cors.options()           → todos os métodos configurados |
| `.origins()` | cors.origins() → lista de origens configuradas (usado em auth=). |
| `.permiser()` |  |

### `.options(...)`

cors.options()           → todos os métodos configurados
cors.options(["POST"])   → filtra/sobrescreve para a rota

[← índice](../jinker.md)
