# `MailServer.login(user, password)`

Autentica no servidor SMTP com usuário e senha. Chame **depois** de
[`.conn()`](../conn/conn.md).

```
s.login(user: str, password: str) -> None
```

---

## Uso

```
import mail
import os
from dotenv import load

load()

s = mail.MailServer()
s.conn("gmail.com")
s.login(os.getenv("MAIL_USER"), os.getenv("MAIL_PASS"))
```

Guarde usuário e senha no `.env` — nunca no código.

---

## ⚠️ Gmail/Outlook: use senha de aplicativo

Provedores modernos **não aceitam a senha normal** da sua conta por SMTP. Você
precisa gerar uma **senha de aplicativo** (App Password) nas configurações de
segurança da conta e usar essa senha no `login`. Com a senha normal, o login
falha com erro de autenticação.

Se o login falhar mesmo com a senha certa: confira se você conectou no host
correto ([`.conn`](../conn/conn.md)) — um host errado dá "falha de auth" mesmo
com credenciais válidas.

---

## Relacionados

- [`.conn()`](../conn/conn.md) — conectar antes
- [`.send()`](../send/send.md) — enviar depois
