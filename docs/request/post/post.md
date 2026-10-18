# `request.post(url, headers=Null, body=Null, timeout=30, stream=False, max_size=Null, fields=Null, file=Null, save=Null)`

Faz uma requisição HTTP **POST** — usada pra **enviar dados** (criar recursos,
fazer login, subir arquivo). Devolve um [`Response`](../Response/Response.md).

```assinatura
request.post(url, headers=Null, body=Null, timeout=30, stream=False, max_size=Null,
             fields=Null, file=Null, save=Null) -> Response
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `url` | — | endereço a chamar |
| `headers` | `Null` | dict de cabeçalhos |
| `body` | `Null` | o que enviar — **dict/lista viram JSON automaticamente** |
| `timeout` | `30` | segundos até desistir |
| `stream` | `False` | liga o teto do `max_size`, e só isso. O corpo nunca é carregado por conta do motor — ver [baixar arquivo](../download/download.md) |
| `max_size` | `Null` | o teto quando `stream=True`: bytes ou texto com unidade (`"500mb"`); `Null` = 100 MB. Ignorado sem `stream` |
| `fields` | `Null` | **multipart**: dict com os campos simples do formulário — ver a seção abaixo |
| `file` | `Null` | **multipart**: `{campo: {"name": caminho}}` — a lib lê o arquivo do disco e envia como parte binária |
| `save` | `Null` | caminho de arquivo: o corpo da resposta vai direto pra ele, e o arquivo é **seu**. Sem `save=` vai pra um arquivo do motor, que some com o `Response` — ver [baixar arquivo](../download/download.md) |

Os exemplos desta página pressupõem:

```ps contexto
import request
```

---

## Multipart — enviar arquivo (`fields=` + `file=`)

Pra APIs que recebem **arquivo** (upload, transcrição de áudio...), use
`fields=`/`file=` em vez de `body=` — a lib monta o `multipart/form-data`
inteiro sozinha (boundary, cabeçalhos de cada parte, o arquivo em binário):

```ps sem-rodar: envia um arquivo da máquina pra um servidor de fora
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

```ps sem-rodar: faz pedido HTTP a um servidor de fora
import request

resp = request.post(
    "https://api.x.com/users",
    body={"nome": "ana", "email": "ana@email.com"}
)
post(resp.status)            # 201
post(resp.get_json())        # resposta do servidor
```

---

## Enviando texto ou outro formato

Se `body` for uma **string**, é enviada como está (sem virar JSON):

```ps sem-rodar: faz pedido HTTP a um servidor de fora
resp = request.post("https://api.x.com/webhook", body="texto cru")
```

---

## Com autenticação

```ps sem-rodar: faz pedido HTTP a um servidor de fora
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
