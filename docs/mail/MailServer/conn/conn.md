# `MailServer.conn(provider_or_host, port=None)`

Conecta ao servidor SMTP (envio). Aceita o **nome do provedor** (auto-configura
host e porta) ou um host manual.

```
s.conn(provider_or_host: str, port: int = None) -> None
```

---

## Provedores auto-configurados

Passe só o domínio — a lib sabe o resto:

```
s.conn("gmail.com")       // → smtp.gmail.com:587
s.conn("outlook.com")     // → smtp.office365.com:587
s.conn("hotmail.com")     // → smtp.office365.com:587
s.conn("yahoo.com")       // → smtp.mail.yahoo.com:587
s.conn("proton.me")       // → smtp.protonmail.ch:587
```

## Host manual

Pra um servidor próprio, passe host e porta:

```
s.conn("smtp.meuservidor.com", 587)
```

---

## ⚠️ Cuidado com o host errado

Digitar o host errado (ex: `"smtp.gamil.com"` — "gamil" em vez de "gmail") pode
te conectar a um **servidor de terceiros** que aceita a conexão e recebe seu
usuário/senha. Prefira sempre o nome do provedor (`"gmail.com"`) em vez de
digitar o host SMTP na mão.

---

## Relacionados

- [`.login()`](../login/login.md) — o próximo passo
- [visão geral do MailServer](../MailServer.md)
