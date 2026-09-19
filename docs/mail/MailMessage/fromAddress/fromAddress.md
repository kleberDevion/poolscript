# `MailMessage.fromAddress(address)`

Define o **remetente** do e-mail (quem envia).

```
m.fromAddress(address: str) -> None
```

---

## Uso

```
m = mail.MailMessage()
m.fromAddress("eu@gmail.com")
```

Normalmente é o **mesmo e-mail** que você usa no
[`MailServer.login()`](../../MailServer/login/login.md) — os provedores exigem
que o remetente bata com a conta autenticada.

```
m.fromAddress(os.getenv("MAIL_USER"))    # mesmo do login
```

---

## Relacionados

- [`.toAddress()`](../toAddress/toAddress.md) — o destinatário
- [visão geral do MailMessage](../MailMessage.md)
