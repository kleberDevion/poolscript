# `MailReader` — ler a caixa de entrada (IMAP)

Conecta na caixa de e-mail e **lê** mensagens: lista, filtra e pega o corpo.

```
from mail import MailReader
r = mail.MailReader()
```

---

## Métodos (na ordem de uso)

| Método | O que faz | Página |
|---|---|---|
| `.conn(provedor)` | conecta ao servidor IMAP | [conn/conn.md](conn/conn.md) |
| `.login(user, senha)` | autentica | [login/login.md](login/login.md) |
| `.select(pasta)` | escolhe a pasta (INBOX, etc.) | [select/select.md](select/select.md) |
| `.search(criterio, term, limit)` | busca e-mails | [search/search.md](search/search.md) |
| `.body(id)` | pega o corpo de um e-mail específico | [body/body.md](body/body.md) |
| `.close()` | encerra a conexão | [close/close.md](close/close.md) |

---

## Fluxo completo

```
import mail
import os
from dotenv import load

load()

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

- [`MailServer`](../MailServer/MailServer.md) — o lado de **enviar**
- [visão geral do mail](../mail.md)
