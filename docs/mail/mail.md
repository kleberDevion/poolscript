# mail — Enviar e ler e-mails

Lib pra **enviar** e-mails (via SMTP) e **ler** a caixa de entrada (via IMAP).
Auto-configura os servidores dos provedores comuns (gmail, outlook, yahoo…) —
você só passa `"gmail.com"` e a lib sabe host e porta.

```
import mail
```

---

## As três classes

| Classe | Serve pra | Página |
|---|---|---|
| `MailServer` | **enviar** — conectar, logar, mandar | [MailServer/MailServer.md](MailServer/MailServer.md) |
| `MailMessage` | **montar** o e-mail (remetente, assunto, corpo, anexos) | [MailMessage/MailMessage.md](MailMessage/MailMessage.md) |
| `MailReader` | **ler** a caixa de entrada | [MailReader/MailReader.md](MailReader/MailReader.md) |

---

## Enviar um e-mail (fluxo completo)

```
import mail
import os
from dotenv import load

load()

# 1. monta a mensagem
m = mail.MailMessage()
m.from_address(os.getenv("MAIL_USER"))
m.to("destino@email.com")
m.subject("Olá!")
m.body("Corpo do e-mail")

# 2. conecta, loga e envia
s = mail.MailServer()
s.conn("gmail.com")                              # host/porta automáticos
s.login(os.getenv("MAIL_USER"), os.getenv("MAIL_PASS"))
s.send(m)
s.quit()
```

> **Gmail e senha de app:** provedores como o Gmail não aceitam sua senha
> normal — você gera uma **senha de aplicativo** nas configurações da conta e
> usa ela no `login`. E confira o **host**: é `"gmail.com"` (a lib mapeia pra
> `smtp.gmail.com`), não digite o host errado.

---

## Ler a caixa de entrada

```
r = mail.MailReader()
r.conn("gmail.com")
r.login(os.getenv("MAIL_USER"), os.getenv("MAIL_PASS"))
r.select("INBOX")
emails = r.search("ALL", limit=10)
for each e in emails {
    post(e["from"], "-", e["subject"])
}
r.close()
```

---

## Relacionados

- lib `manpu` / `os` — carregar arquivos pra anexar
- `.env` + `os.getenv` — guardar usuário e senha fora do código
