# `MailMessage.get_as_string()`

Devolve o e-mail montado como **texto** (o formato MIME cru). Útil pra
inspecionar/depurar o que será enviado, ou pra salvar/logar a mensagem.

```
m.get_as_string() -> str
```

---

## Uso

```
m = mail.MailMessage()
m.from_address("eu@gmail.com")
m.to("destino@email.com")
m.subject("Teste")
m.body("Olá")

post(m.get_as_string())     // mostra os headers + corpo em formato MIME
```

Normalmente você não precisa disto no dia a dia — é pra debug ou pra guardar
uma cópia do e-mail enviado.

---

## Relacionados

- [`MailServer.send()`](../../MailServer/send/send.md) — enviar a mensagem
- [visão geral do MailMessage](../MailMessage.md)
