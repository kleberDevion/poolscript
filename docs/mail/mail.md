# mail — Enviar e ler e-mails

Lib pra **enviar** e-mails (via SMTP) e **ler** a caixa de entrada (via IMAP).
Você passa o host do servidor; sem porta, a lib usa 587 no envio e 993 na
leitura.

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
m.fromAddress(os.getenv("MAIL_USER"))
m.toAddress("destino@email.com")
m.subject("Olá!")
m.body("Corpo do e-mail")

# 2. conecta, loga e envia
s = mail.MailServer()
s.conn(os.getenv("MAIL_SMTP"))                   # porta 587
s.login(os.getenv("MAIL_USER"), os.getenv("MAIL_PASS"))
s.send(m)
s.quit()
```

> **Senha de app:** muitos servidores não aceitam a senha normal da conta —
> você gera uma **senha de aplicativo** nas configurações da conta e usa ela
> no `login`. E confira o **host**: um host digitado errado pode entregar seu
> usuário e senha a um servidor de terceiros.

---

## Ler a caixa de entrada

```
r = mail.MailReader()
r.conn(os.getenv("MAIL_IMAP"))                   # porta 993
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
