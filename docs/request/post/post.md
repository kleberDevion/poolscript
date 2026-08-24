# `request.post(url, headers=None, body=None, timeout=30, stream=false, max_size=None, fields=None, file=None)`

Faz uma requisição HTTP **POST** — usada pra **enviar dados** (criar recursos,
fazer login, subir arquivo). Devolve um [`Response`](../Response/Response.md).

```
request.post(url, headers=None, body=None, timeout=30, stream=false, max_size=None,
             fields=None, file=None) -> Response
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `url` | — | endereço a chamar |
| `headers` | `None` | dict de cabeçalhos |
| `body` | `None` | o que enviar — **dict/lista viram JSON automaticamente** |
| `timeout` | `30` | segundos até desistir |
| `stream` | `false` | `true` = lê a resposta em pedaços de 64 KB com **teto de memória** — passa do teto, levanta erro em vez de engolir a RAM |
| `max_size` | `None` | o teto quando `stream=true`: bytes ou texto com unidade (`"500mb"`); `None` = 100 MB. Ignorado sem `stream` |
| `fields` | `None` | **multipart**: dict com os campos simples do formulário — ver a seção abaixo |
| `file` | `None` | **multipart**: `{campo: {"name": caminho}}` — a lib lê o arquivo do disco e envia como parte binária |

---

## Multipart — enviar arquivo (`fields=` + `file=`)

Pra APIs que recebem **arquivo** (upload, transcrição de áudio...), use
`fields=`/`file=` em vez de `body=` — a lib monta o `multipart/form-data`
inteiro sozinha (boundary, cabeçalhos de cada parte, o arquivo em binário):

```
import request

resp = request.post("https://api.exemplo.com/audio/transcriptions",
    headers={"Authorization": "Bearer TOKEN"},
    fields={
        "model": "whisper-large-v3",
        "response_format": "json",
        "language": "pt"
    },
    file={
        "file": { "name": "gravacao.mp3" }
    }
)
post(resp.get_json())
```

- **`fields=`** — os campos "de texto" do formulário (`{nome: valor}`; valor
  que não é string vira string).
- **`file=`** — os arquivos: a **chave** é o nome do campo no formulário, e
  `"name"` é o **caminho** do arquivo, de onde você quiser — absoluto,
  relativo à pasta do script em execução, ou ao diretório atual (a mesma
  regra da lib `os`). O nome enviado é o do arquivo (basename), como
  `application/octet-stream`.
- O `Content-Type` da requisição é **da lib** (carrega o boundary gerado) —
  um `Content-Type` manual nos `headers=` é descartado, senão o boundary não
  bateria. O `Authorization` e os demais headers passam normal.
- `body=` **ou** `fields=`/`file=` — os dois juntos é erro
  (`use body= OU fields=/file= (multipart) — não os dois juntos`).
- Arquivo que não existe: `FileNotFoundError` com o caminho pedido.

---

## Enviando JSON (o caso comum)

Passe um dict em `body` — ele é convertido pra JSON e o `Content-Type:
application/json` é definido sozinho:

```
import request

resp = request.post(
    "https://api.x.com/users",
    body={"nome": "ana", "email": "ana@email.com"}
)
post(resp.status)            // 201
post(resp.get_json())        // resposta do servidor
```

---

## Enviando texto ou outro formato

Se `body` for uma **string**, é enviada como está (sem virar JSON):

```
resp = request.post("https://api.x.com/webhook", body="texto cru")
```

---

## Com autenticação

```
resp = request.post(
    "https://api.x.com/protegido",
    headers={"Authorization": "Bearer token"},
    body={"acao": "salvar"}
)
```

---

## Os outros métodos que enviam dados

`put`, `patch` e `delete` têm a **mesma assinatura** e o mesmo comportamento de
`body` (dict → JSON). Muda só o método HTTP:

- [`put`](../put/put.md) — substituir um recurso inteiro
- [`patch`](../patch/patch.md) — atualizar parte de um recurso
- [`delete`](../delete/delete.md) — remover

---

## Relacionados

- [`Response`](../Response/Response.md) — o objeto devolvido
- [`request.get()`](../get/get.md) — buscar dados (detalha `.ok`, erros, headers)
