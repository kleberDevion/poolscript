# `MailMessage.toAddress(endereco)`

Define o **destinatário** do e-mail (quem recebe).

```assinatura
m.toAddress(endereco: str) -> MailMessage
```

O exemplo desta página pressupõe:

```ps contexto
import mail
```

---

## Uso

```ps
m = mail.MailMessage()
m.toAddress("cliente@address.com")
```

---

## Relacionados

- [`.fromAddress()`](../fromAddress/fromAddress.md) — o remetente
- [`.subject()`](../subject/subject.md) — o assunto
