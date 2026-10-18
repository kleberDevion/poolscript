# `MailReader.login(user, password)`

Autentica na caixa de e-mail (IMAP). Chame depois de
[`.conn()`](../conn/conn.md).

```assinatura
r.login(user: str, password: str) -> Null
```

O exemplo desta página pressupõe:

```ps contexto
import mail
import os
```

---

## Uso

```ps sem-rodar: lê a conta de e-mail do ambiente (MAIL_USER...) e precisa do servidor
r = mail.MailReader()
r.conn(os.getenv("MAIL_IMAP"))
r.login(os.getenv("MAIL_USER"), os.getenv("MAIL_PASS"))
```

Assim como no envio, muitos servidores exigem **senha de aplicativo**, não a
senha normal — ver [`MailServer.login`](../../MailServer/login/login.md).

---

## Relacionados

- [`.select()`](../select/select.md) — escolher a pasta depois
- [`.conn()`](../conn/conn.md) — conectar antes
