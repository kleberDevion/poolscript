# `MailServer.send(msg)` / `.send(to, subject, body, html)`

Envia um e-mail. Duas formas: passar um [`MailMessage`](../../MailMessage/MailMessage.md)
pronto, ou os campos direto.

```
s.send(msg)                                   // objeto MailMessage
s.send(to, subject=None, body=None, html=false)   // direto
```

---

## Forma 1 — com `MailMessage` (recomendada)

Monta a mensagem separada (permite anexos, HTML, etc.) e envia:

```
m = mail.MailMessage()
m.from_address(os.getenv("MAIL_USER"))
m.to("destino@email.com")
m.subject("Relatório")
m.body("Segue em anexo.")
m.attach("relatorio.pdf")

s.send(m)
```

## Forma 2 — direto (rápida, sem anexo)

Pra um e-mail simples de texto:

```
s.send("destino@email.com", "Assunto", "Corpo do e-mail")
```

Pra corpo em HTML, passe `html=true`:

```
s.send("destino@email.com", "Oi", "<h1>Olá</h1>", html=true)
```

---

## Precisa estar logado

Chame [`.conn()`](../conn/conn.md) e [`.login()`](../login/login.md) antes.
Enviar sem conexão levanta erro.

---

## Relacionados

- [`MailMessage`](../../MailMessage/MailMessage.md) — montar mensagens ricas (anexo, HTML)
- [`.quit()`](../quit/quit.md) — fechar depois de enviar
