# `QRImage.to_file()`

Converte a imagem do QR Code num arquivo (bytes em memória), sem gravar no
disco. Útil pra enviar por email ou rede sem criar um arquivo temporário.

```
img.to_file()
```

---

## Uso

```
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
