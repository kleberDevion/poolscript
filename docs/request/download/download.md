# Baixar arquivo com `request` (binário)

O `request` também **baixa** arquivos — não só texto/JSON. Pra binário (`.exe`,
imagem, zip, pdf) você usa os **bytes crus**, não o `.text` (que decodificaria e
corromperia).

```
resp = request.get(url)
resp.content        -> bytes    # conteúdo cru, intacto
resp.save(destino)  -> PoolFile # grava em disco
resp.decode(enc)    -> str      # texto num encoding específico
```

---

## Jeito simples — `.save()`

`.save()` grava e devolve um [`PoolFile`](../../os/PoolFile/PoolFile.md), então você
já tem `.name`, `.size`, `.bytes()`, `.move()`, `.copy()`, `.delete()`.

```
import request

reaction baixar() {
    try {
        url = "https://exemplo.com/PoolScript-Setup.exe"

        # stream=true: baixa em pedaços e aborta se ficar grande demais.
        # content_type: garante que veio mesmo um binário (senão dá raise).
        f = request.get(url, stream=true)
            .content_type("application/octet-stream")
            .save("downloads/")            # pasta -> nome vem do servidor

        post("salvo:", f.name, "-", f.size, "bytes")
        return f.path()
    } catch (e) {
        post(f"Erro no download: {e}")
        return 500
    }
}
```

**O nome do arquivo**, quando você passa uma **pasta** (ex: `"downloads/"` ou
`"."`), vem do header `Content-Disposition` do servidor; se não tiver, do fim da
URL. Se você passar o caminho **com nome** (`"downloads/pool.exe"`), usa o seu.

---

## Controlando a memória — `stream` e `max_size`

`stream=true` baixa em pedaços em vez de carregar tudo de uma vez. Se o arquivo
passar do teto, ele **para e lança erro** (cai no `catch`) — protege contra baixar
um arquivo gigante sem querer.

```
request.get(url, stream=true)                    # teto padrão: 100 MB
request.get(url, stream=true, max_size="500mb")  # teto maior
request.get(url, stream=true, max_size=2000000)  # em bytes também vale
```

> **Recomendado** usar `stream=true` em qualquer download de arquivo. Pra
> respostas pequenas (JSON de API) não precisa.

---

## Validando o tipo — `content_type()`

Declara o MIME que você **espera**. Se o servidor devolver outro, dá **raise**
(cai no `catch`) — pega download errado cedo (ex: recebeu uma página de erro HTML
no lugar do arquivo).

```
request.get(url).content_type("application/octet-stream")   # ok se bater
request.get(url).content_type("image/png")                  # raise se vier outra coisa
```

---

## Jeito manual — `using open`

Se preferir controlar a escrita, `.content` são bytes e o `open(..., "wb")` grava
binário direto:

```
resp = request.get(url, stream=true)
if (resp.size <= 100) {
    return 400                     # pequeno demais, provável erro
}
using open("downloads/pool.exe", "wb") as f {
    f.write(resp.content)
}
```

---

## Texto com encoding — `.decode()`

Pra corpo de texto num encoding que não seja UTF-8:

```
resp = request.get("https://site-legado.com")
html = resp.decode("latin-1")
```

O `.text` (UTF-8) e o `.json()` continuam funcionando como sempre — isto aqui é só
pra os casos binário/encoding especial.

---

## Relacionados

- [`Response`](../Response/Response.md) — todos os campos/métodos da resposta
- [`get`](../get/get.md) — a requisição
- [`PoolFile`](../../os/PoolFile/PoolFile.md) — o que o `.save()` devolve
- [jinker `render`](../../jinker/jinker.md) — o oposto: **servir** um arquivo (lado servidor)
