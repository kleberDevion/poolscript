# `MailReader.login(user, senha)`

Autentica na caixa de e-mail (IMAP). Chame depois de
[`.conn()`](../conn/conn.md).

```
r.login(user: str, senha: str) -> None
```

---

## Uso

```
r = mail.MailReader()
r.conn("gmail.com")
r.login(os.getenv("MAIL_USER"), os.getenv("MAIL_PASS"))
```

Assim como no envio, o Gmail/Outlook exigem **senha de aplicativo**, não a
senha normal — ver [`MailServer.login`](../../MailServer/login/login.md).

---

## Relacionados

- [`.select()`](../select/select.md) — escolher a pasta depois
- [`.conn()`](../conn/conn.md) — conectar antes
