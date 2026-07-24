# `MailServer` — enviar e-mails (SMTP)

Conecta num servidor de e-mail, faz login e envia mensagens.

```
from mail import MailServer
s = mail.MailServer()
```

---

## Métodos (na ordem de uso)

| Método | O que faz | Página |
|---|---|---|
| `.conn(provedor, port)` | conecta ao servidor SMTP | [conn/conn.md](conn/conn.md) |
| `.login(user, senha)` | autentica | [login/login.md](login/login.md) |
| `.send(msg)` | envia a mensagem | [send/send.md](send/send.md) |
| `.quit()` | encerra a conexão | [quit/quit.md](quit/quit.md) |

---

## Fluxo completo

```
import mail
import os
from dotenv import load

load()

m = mail.MailMessage()
m.from_address(os.getenv("MAIL_USER"))
m.to("destino@email.com")
m.subject("Assunto")
m.body("Corpo")

s = mail.MailServer()
s.conn("gmail.com")                             // 1. conecta
s.login(os.getenv("MAIL_USER"), os.getenv("MAIL_PASS"))   // 2. loga
s.send(m)                                       // 3. envia
s.quit()                                        // 4. fecha
```

---

## Relacionados

- [`MailMessage`](../MailMessage/MailMessage.md) — montar o que enviar
- [`MailReader`](../MailReader/MailReader.md) — ler e-mails
