# `MailMessage.fromAddress(address)`

Define o **remetente** do e-mail (quem envia).

```assinatura
m.fromAddress(address: str) -> Null
```

Os exemplos desta página pressupõem:

```ps contexto
import mail
import os
MailMessage m = mail.MailMessage()
```

---

## Uso

```ps
m = mail.MailMessage()
m.fromAddress("eu@gmail.com")
```

Normalmente é o **mesmo e-mail** que você usa no
[`MailServer.login()`](../../MailServer/login/login.md) — os provedores exigem
que o remetente bata com a conta autenticada.

```ps sem-rodar: lê o remetente do ambiente (MAIL_USER)
m.fromAddress(os.getenv("MAIL_USER"))    # mesmo do login
```

---

## Relacionados

- [`.toAddress()`](../toAddress/toAddress.md) — o destinatário
- [visão geral do MailMessage](../MailMessage.md)
