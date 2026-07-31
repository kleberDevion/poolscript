# `MailMessage` — montar um e-mail

Constrói o e-mail a ser enviado: remetente, destinatário, assunto, corpo
(texto ou HTML) e anexos. Depois passe pro [`MailServer.send()`](../MailServer/send/send.md).

```
from mail import MailMessage
m = mail.MailMessage()
```

---

## Métodos

| Método | O que faz | Página |
|---|---|---|
| `.from_address(email)` | define o remetente | [from_address/from_address.md](from_address/from_address.md) |
| `.to(email)` | define o destinatário | [to/to.md](to/to.md) |
| `.subject(titulo)` | define o assunto | [subject/subject.md](subject/subject.md) |
| `.body(conteudo, is_html)` | define o corpo (texto ou HTML) | [body/body.md](body/body.md) |
| `.attach(arquivo)` | anexa um arquivo | [attach/attach.md](attach/attach.md) |
| `.get_as_string()` | devolve o e-mail montado como texto | [get_as_string/get_as_string.md](get_as_string/get_as_string.md) |

---

## Exemplo completo

```
import mail
import os

m = mail.MailMessage()
m.from_address(os.getenv("MAIL_USER"))
m.to("cliente@email.com")
m.subject("Sua fatura")
m.body("<h1>Olá!</h1><p>Segue sua fatura.</p>", is_html=true)
m.attach("fatura.pdf")

// ... envia com MailServer.send(m) ...
```

---

## Relacionados

- [`MailServer.send()`](../MailServer/send/send.md) — enviar a mensagem montada
- [visão geral do mail](../mail.md)
