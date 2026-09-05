# `MailServer.quit()`

Encerra a conexão com o servidor SMTP. Chame ao terminar de enviar.

```
s.quit() -> None
```

---

## Uso

```
s = mail.MailServer()
s.conn("gmail.com")
s.login(user, senha)
s.send(m)
s.quit()          # fecha a conexão
```

Não quebra se a conexão já estiver fechada — é seguro chamar.

---

## Relacionados

- [`.send()`](../send/send.md) — enviar antes de fechar
- [visão geral do MailServer](../MailServer.md)
