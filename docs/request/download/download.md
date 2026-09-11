# Baixar arquivo com `request` (binário)

O `request` também **baixa** arquivos — não só texto/JSON. Pra binário (`.exe`,
imagem, zip, pdf) você usa os **bytes crus**, não o `.text` (que decodificaria e
corromperia).

```
resp = request.get(url)
resp.save(destino)  -> PoolFile # move o arquivo do corpo pro destino
resp.content        -> bytes    # o corpo, lido do arquivo quando você pede
resp.decode(enc)    -> str      # texto num encoding específico
```

---

## Onde o corpo fica

O motor **nunca guarda o corpo na memória**. Toda resposta desce da rede direto
pra um arquivo, pedaço a pedaço:

- com `save="caminho"`, o arquivo é **esse**, e é **seu** — o motor não o apaga;
- sem `save=`, é um arquivo do motor (`.ps_resposta_…` na pasta corrente), que
  **some junto com o `Response`**.

`.size` diz quantos bytes chegaram sem abrir nada. `.save()` **move** o arquivo
(`rename`, sem cópia). O corpo só ocupa memória se o **programa pedir** —
`.content`, `.text`, `.json()` — e aí ele é lido do arquivo naquela hora.

Medido no mesmo download de 237 MB:

| forma | pico de memória do processo |
|---|---|
| `request.get(url).save("x")` ou `request.get(url, save="x")` | 12,7 MB |
| `request.get(url).content` | 254,8 MB — o corpo uma vez, mais o processo |
| antes desta versão, `request.get(url)` | 480 MB — o corpo duas vezes |

Por isso dá pra baixar arquivo maior que a memória da máquina, desde que você
não peça `.content` dele.

```ps
import request
import os

r = request.get("https://exemplo.com/pacote.deb", save="pacote.deb")
post(r.status_code, r.size, os.size("pacote.deb"))
```

`.content` com `save=` **lê do arquivo salvo** — `save=` decide só onde o corpo
está, não se você pode lê-lo.

> **`stream=true` não faz nada disso.** Apesar do nome, a única coisa que
> `stream` liga é o teto do `max_size` (abaixo). O corpo nunca é carregado por
> conta do motor, com ou sem `stream`.

---

## Jeito simples — `.save()`

`.save()` move o arquivo do corpo pro destino e devolve um
[`PoolFile`](../../os/PoolFile/PoolFile.md) **fechado**: você tem `.name`,
`.size`, `.path()`, `.move()`, `.copy()`, `.delete()`, e `.bytes()` lê do disco
só se for chamado. Pra ler em pedaços, `open(f.path(), "rb")`.

```
import request

funct baixar() {
    try {
        url = "https://exemplo.com/PoolScript-Setup.exe"

        # content_type: garante que veio mesmo um binário (senão dá raise).
        f = request.get(url)
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

Um segundo `.save()` no mesmo `Response` **copia**: o primeiro arquivo fica. O
mesmo vale quando o corpo veio de `save=` — mover apagaria o que você pediu.

---

## Limitando o tamanho — `stream` e `max_size`

`max_size` corta o download quando o corpo passa do teto, levantando erro em vez
de continuar baixando. Ele **só vale com `stream=true`**:

```ps
request.get(url, stream=true)                    # teto padrão: 100 MB
request.get(url, stream=true, max_size="500mb")  # teto maior
request.get(url, stream=true, max_size=2000000)  # em bytes também vale
```

Combina com `save=`: `max_size` continua sendo o teto, e o que couber nele vai
pro arquivo.

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
binário direto. Repare que aqui **você** trouxe o corpo pra memória com
`.content`; o `.save()` acima não traz.

```
resp = request.get(url)
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
