# `MailReader.conn(host, port=None)`

Conecta ao servidor **IMAP** (leitura) no `host` dado, com TLS. Sem `port`, usa
a 993.

```
r.conn(host: str, port: int = None) -> bool
```

Devolve `true` quando conectou; falha de rede, de TLS ou de host inexistente é
erro (não devolve `false`).

---

## Exemplo

```
r.conn("imap.meuservidor.com")          # porta 993
r.conn("127.0.0.1", 1143)               # servidor local em outra porta
```

O `host` é usado como está escrito: a lib não traduz domínio de e-mail em
servidor. O endereço do servidor IMAP vem das configurações da sua conta de
e-mail.

---

## Relacionados

- [`.login()`](../login/login.md) — o próximo passo
- [`MailServer.conn()`](../../MailServer/conn/conn.md) — o equivalente pra envio (SMTP)
