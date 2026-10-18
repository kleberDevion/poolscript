# `MailMessage.subject(title)`

Define o **assunto** (título) do e-mail — a linha que aparece na lista de
e-mails do destinatário.

```assinatura
m.subject(title: str) -> Null
```

O exemplo desta página pressupõe:

```ps contexto
import mail
```

---

## Uso

```ps
m = mail.MailMessage()
m.subject("Sua fatura de julho")
```

---

## Relacionados

- [`.body()`](../body/body.md) — o corpo do e-mail
- [visão geral do MailMessage](../MailMessage.md)
