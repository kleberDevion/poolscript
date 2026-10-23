# `MailMessage.attach(file_or_path)`

Anexa um arquivo ao e-mail. Aceita um **caminho** (string), um
[`PoolFile`](../../../os/PoolFile/PoolFile.md) (binário carregado) ou um
`PoolFileUpload` (arquivo recebido num upload).

```assinatura
m.attach(file_or_path) -> Null
```

Os exemplos desta página pressupõem:

```ps contexto
import mail
import os
import bytes
from jinker import request
MailMessage m = mail.MailMessage()
# os arquivos dos exemplos (os binários com o começo de um arquivo de verdade)
os.writeFile("relatorio.pdf", bytes.fromhex("255044462d312e340a25e2e3cfd30a0000"))
os.writeFile("nota.pdf", bytes.fromhex("255044462d312e340a25e2e3cfd30a0000"))
os.writeFile("doc1.pdf", bytes.fromhex("255044462d312e340a25e2e3cfd30a0000"))
os.writeFile("doc2.pdf", bytes.fromhex("255044462d312e340a25e2e3cfd30a0000"))
os.writeFile("planilha.xlsx", bytes.fromhex("504b030414000000080000"))
os.writeFile("imagem.png", bytes.fromhex("89504e470d0a1a0a0000000d49484452"))
```

---

## Anexar por caminho

```ps
m = mail.MailMessage()
m.attach("relatorio.pdf")
m.attach("planilha.xlsx")     # dá pra anexar vários
```

## Anexar um arquivo já carregado

```ps
import os
pdf = os.loadFile("nota.pdf")     # PoolFile
m.attach(pdf)
```

## Anexar um upload recebido (jinker)

```ps
# dentro de uma rota que recebe upload
foto = request.file("foto")
m.attach(foto)                    # PoolFileUpload
```

---

## Vários anexos

Chame `.attach()` uma vez por arquivo:

```ps
m.attach("doc1.pdf")
m.attach("doc2.pdf")
m.attach("imagem.png")
```

---

## Relacionados

- [`os.loadFile()`](../../../os/loadFile/loadFile.md) — carregar um arquivo binário
- [jinker request.file()](../../../jinker/request/file/file.md) — receber upload pra anexar
- [`.body()`](../body/body.md) — o corpo do e-mail
