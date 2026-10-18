# `request.file(field, allowed=Null)`

Pega **um arquivo enviado** por upload (formulário `multipart/form-data`).
Devolve um `PoolFileUpload` ou `Null` se o campo não tiver arquivo.

```assinatura
request.file(field: str, allowed: list = Null) -> PoolFileUpload | Null
```

| Parâmetro | O que é |
|---|---|
| `field` | nome do campo do formulário que contém o arquivo |
| `allowed` | lista de extensões permitidas (ex: `[".jpg", ".png"]`); `Null` = qualquer extensão *segura* |

Os exemplos desta página pressupõem:

```ps contexto
from jinker import Jinker, cors, jsonify
Jinker app = Jinker(__name__)
```

---

## Uso

```ps
@app.route("/upload", methods=cors.options(["POST"]))
funct upload() {
    foto = request.file("foto", allowed=[".jpg", ".png"])
    if (foto is Null) {
        return jsonify({"erro": "nenhum arquivo enviado"}), 400
    }
    foto.save("uploads/perfil.jpg")      # grava em disco
    return jsonify({"salvo": foto.name, "tamanho": foto.size})
}
```

---

## O objeto `PoolFileUpload`

O arquivo vive **em memória** até você salvar. Propriedades e métodos:

| Acesso | O que é |
|---|---|
| `.name` | nome original do arquivo (`"perfil.jpg"`) |
| `.content_type` | MIME informado pelo cliente (`"image/jpeg"`) |
| `.size` | tamanho em bytes |
| `.ext` | extensão em minúsculo (`".jpg"`) |
| `.save(destino)` | grava os bytes no caminho (cria pastas se preciso) |
| `.move(destino)` | apelido de `.save()` |
| `.bytes()` | devolve os bytes crus |

Quando o `.save()` não consegue gravar, o erro é o do sistema, com o
caminho: `FileNotFoundError`, `PermissionError`, `NotADirectoryError`, e
`OSError: [Errno 28] No space left on device` com o disco cheio (a gravação
é conferida até o fim, e o arquivo pela metade não passa por salvo). O
destino com o byte nulo (`\x00`) é `ValueError`
([exceções](../../../exceptions/exceptions.md)).

---

## Segurança de extensão (embutida)

Duas camadas automáticas:

1. **Extensões sempre bloqueadas** — scripts e executáveis
   (`.exe`, `.bat`, `.sh`, `.ps1`, `.py`, `.js`, `.php`, `.dll`, …) **sempre**
   levantam erro, mesmo que você não passe `allowed`. É pra evitar upload de
   código malicioso.
2. **Sua lista `allowed`** — se passada, só as extensões dela são aceitas;
   qualquer outra levanta erro com a lista permitida na mensagem.

```ps
# só imagens; um .pdf aqui levanta erro
foto = request.file("foto", allowed=[".jpg", ".jpeg", ".png"])

# sem allowed: aceita qualquer extensão que NÃO esteja na lista de bloqueadas
doc = request.file("documento")
```

Envolva em `try/catch` se quiser responder o erro de extensão em vez de deixar
virar 500:

```ps
funct upload() {
    try {
        foto = request.file("foto", allowed=[".png"])
    } catch (e) {
        return jsonify({"erro": f"{e}"}), 400
    }
}
```

---

## Relacionados

- [`.files()`](../files/files.md) — vários arquivos no mesmo campo
- [`request`](../request.md) — visão geral
