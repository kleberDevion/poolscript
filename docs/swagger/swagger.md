# swagger — Gerador de OpenAPI, cliente e Swagger UI

A lib `swagger` é um **builder**: você encadeia a config (título, descrição,
versão, contato, saída) e no `SwaggerGEN()` ela gera, a partir das rotas do seu
app jinker:

- **`openapi.json`** — o spec OpenAPI 3.0;
- **`index.html`** — Swagger UI pra abrir no **browser** e explorar a API;
- **`client.ts`** — um cliente da API (uma função por rota), se você chamar `bundle()`.

```
import swagger
```

---

## Exemplo completo

```ps
import swagger

int async funct main() {
    d = swagger.infos(target="app.ps")
               .title("API de Pedidos")
               .description("API de app de pedidos e entrega")
               .version("1.0.0")
               .authorContact()
                   .email("ex@mail.com")
                   .name("Kleber Santana")
                   .number("(27) 00000-0000")
    d.newDoc(folder_name="docs", contracts=["contratos", "json"])
    d.bundle(lang="typescript", router="react_router")
    d.metaData(file_type=".json", name="openapi")
    d.SwaggerGEN()
}

main()
```

Isso escreve `docs/openapi.json`, `docs/index.html` e `docs/client.ts`.

---

## O builder

### `swagger.infos(target="app.ps")`
Começa o builder. `target` é o **arquivo .ps do seu app** — o swagger lê as
rotas (`@app.route(...)`) dele **estaticamente** (não roda o app) pra montar os
`paths`. Devolve o builder (tudo abaixo é encadeável).

### Info da API (encadeável)
| Método | O que seta |
|---|---|
| `.title(txt)` | título da API |
| `.description(txt)` | descrição |
| `.version(txt)` | versão (ex: `"1.0.0"`) |
| `.authorContact()` | inicia o contato (os próximos caem nele) |
| `.email(txt)` / `.name(txt)` / `.number(txt)` | contato do autor |

### Saída
| Método | O que faz |
|---|---|
| `.newDoc(folder_name, contracts=Null)` | pasta de saída; `contracts` opcional (ver abaixo) |
| `.bundle(lang, router)` | **gera o cliente** da API. Sem args → default (`typescript` + `react_router`) |
| `.metaData(file_type, name)` | nome/extensão do spec (ex: `.json`, `openapi` → `openapi.json`) |
| `.SwaggerGEN()` | **gera tudo** e devolve o caminho do spec |

---

## Contratos (opcional) — o body que a rota espera

Um **contrato** diz qual body uma rota espera; vira o `requestBody` dela no spec.
É opcional. Você aponta a pasta em `newDoc(contracts=["pasta", "json"])`
(`[pasta, tipo]`; o tipo é **`json`**).

Cada contrato é um **JSON** cuja **primeira chave é o nome da rota** com `@`
(pra o gerador saber onde encaixar), e o valor é o body esperado:

```json
{ "@login": { "nome": "kleber", "senha": "1234" } }
```

O gerador casa `@login` com a rota `/login` e põe esse exemplo no `requestBody`.

---

## O que sai

- **`openapi.json`** — `info` (título/descrição/versão/contato) + `paths` (as
  rotas do target, com `{id}` pros parâmetros e `requestBody` dos contratos).
- **`index.html`** — Swagger UI; abra no navegador pra ver/testar a API.
- **`client.ts`** (se `bundle`) — uma função `async` por rota (`postLogin(body)`,
  `getUsersId(id)`…) + um `routes` pronto pro react_router.
