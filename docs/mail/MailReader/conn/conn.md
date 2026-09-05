# `MailReader.conn(provider_or_host, port=None)`

Conecta ao servidor **IMAP** (leitura). Igual ao envio, aceita o nome do
provedor (auto-configura) ou host manual.

```
r.conn(provider_or_host: str, port: int = None) -> None
```

---

## Provedores auto-configurados

```
r.conn("gmail.com")       # → imap.gmail.com:993
r.conn("outlook.com")     # → outlook.office365.com:993
r.conn("yahoo.com")       # → imap.mail.yahoo.com:993
```

## Host manual

```
r.conn("imap.meuservidor.com", 993)
```

---

## Relacionados

- [`.login()`](../login/login.md) — o próximo passo
- [`MailServer.conn()`](../../MailServer/conn/conn.md) — o equivalente pra envio (SMTP)
