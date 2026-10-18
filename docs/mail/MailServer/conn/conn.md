# `MailServer.conn(host, port=Null)`

Conecta ao servidor SMTP (envio) no `host` dado. Sem `port`, usa a 587 (a porta
de envio com STARTTLS).

```assinatura
s.conn(host: str, port: int = Null) -> bool
```

Devolve `True` quando conectou; falha de rede, de TLS ou de host inexistente é
erro (não devolve `False`).

O exemplo desta página pressupõe:

```ps contexto
import mail
MailServer s = mail.MailServer()
```

---

## Exemplo

```ps sem-rodar: precisa do servidor de e-mail
s.conn("smtp.meuservidor.com")          # porta 587
s.conn("smtp.meuservidor.com", 2525)    # outra porta
```

O `host` é usado como está escrito: a lib não traduz domínio de e-mail em
servidor. `s.conn("meudominio.com")` tenta `meudominio.com:587` — o endereço do
servidor SMTP vem das configurações da sua conta de e-mail.

---

## Cuidado com o host errado

Um host digitado errado pode te conectar a um **servidor de terceiros** que
aceita a conexão e recebe seu usuário/senha no `login`. Confira o host antes
de rodar.

---

## Relacionados

- [`.login()`](../login/login.md) — o próximo passo
- [visão geral do MailServer](../MailServer.md)
