# `MailServer.quit()`

Encerra a conexão com o servidor SMTP. Chame ao terminar de enviar.

```assinatura
s.quit() -> Null
```

O exemplo desta página pressupõe:

```ps contexto
import mail
str user = "eu@meuservidor.com"
str senha = "senha-de-app"
MailMessage m = mail.MailMessage()
```

---

## Uso

```ps sem-rodar: precisa do servidor de e-mail
s = mail.MailServer()
s.conn("smtp.meuservidor.com")
s.login(user, senha)
s.send(m)
s.quit()          # fecha a conexão
```

Não quebra se a conexão já estiver fechada — é seguro chamar.

---

## Relacionados

- [`.send()`](../send/send.md) — enviar antes de fechar
- [visão geral do MailServer](../MailServer.md)
