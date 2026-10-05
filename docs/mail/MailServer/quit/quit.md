# `MailServer.quit()`

Encerra a conexão com o servidor SMTP. Chame ao terminar de enviar.

```
s.quit() -> Null
```

---

## Uso

```
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
