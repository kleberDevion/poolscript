# `QRImage.to_file()`

Converte a imagem do QR Code num arquivo (bytes em memória), sem gravar no
disco. Útil pra enviar por email ou rede sem criar um arquivo temporário.

Os exemplos desta página pressupõem:

```ps contexto
import qrcode
QRImage img = qrcode.make("https://meusite.com")
```

```ps
img.to_file()
```

---

## Uso

```ps
import qrcode
import mail

img = qrcode.make("https://meusite.com")
arquivo = img.to_file()

# anexar num email sem salvar no disco
m = mail.MailMessage()
m.attach(arquivo)
```

---

## Relacionados

- [`QRImage.save()`](../save/save.md) — gravar no disco (o caminho comum)
- [mail attach](../../../mail/MailMessage/attach/attach.md) — anexar em email
