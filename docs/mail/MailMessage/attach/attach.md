# `MailMessage.attach(arquivo)`

Anexa um arquivo ao e-mail. Aceita um **caminho** (string), um
[`PoolFile`](../../../os/PoolFile/PoolFile.md) (binário carregado) ou um
`PoolFileUpload` (arquivo recebido num upload).

```
m.attach(arquivo) -> None
```

---

## Anexar por caminho

```
m = mail.MailMessage()
m.attach("relatorio.pdf")
m.attach("planilha.xlsx")     // dá pra anexar vários
```

## Anexar um arquivo já carregado

```
import os
pdf = os.loadFile("nota.pdf")     // PoolFile
m.attach(pdf)
```

## Anexar um upload recebido (jinker)

```
// dentro de uma rota que recebe upload
foto = request.file("foto")
m.attach(foto)                    // PoolFileUpload
```

---

## Vários anexos

Chame `.attach()` uma vez por arquivo:

```
m.attach("doc1.pdf")
m.attach("doc2.pdf")
m.attach("imagem.png")
```

---

## Relacionados

- [`os.loadFile()`](../../../os/loadFile/loadFile.md) — carregar um arquivo binário
- [jinker request.file()](../../../jinker/request/file/file.md) — receber upload pra anexar
- [`.body()`](../body/body.md) — o corpo do e-mail
