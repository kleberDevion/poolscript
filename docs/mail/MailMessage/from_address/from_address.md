# `MailMessage.from_address(address)`

Define o **remetente** do e-mail (quem envia).

```
m.from_address(address: str) -> None
```

---

## Uso

```
m = mail.MailMessage()
m.from_address("eu@gmail.com")
```

Normalmente é o **mesmo e-mail** que você usa no
[`MailServer.login()`](../../MailServer/login/login.md) — os provedores exigem
que o remetente bata com a conta autenticada.

```
m.from_address(os.getenv("MAIL_USER"))    # mesmo do login
```

---

## Relacionados

- [`.to()`](../to/to.md) — o destinatário
- [visão geral do MailMessage](../MailMessage.md)
