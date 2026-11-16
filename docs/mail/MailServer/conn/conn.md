# `MailServer.conn(host, port=Null, tls=False)`

Conecta ao servidor SMTP (envio) no `host` dado. O padrão é **texto puro**:
sem `tls=True`, a lib não manda `STARTTLS`. Com `tls=True`, exige STARTTLS.

```assinatura
s.conn(host: str, port: int = Null, tls: bool = False) -> bool
```

Sem `port`, a porta depende do modo:

- `tls=False` (padrão): **25**, texto puro;
- `tls=True`: **587**, com STARTTLS.

Porta explícita sempre manda. Devolve `True` quando conectou; falha de rede, de
TLS ou de host inexistente é erro (não devolve `False`).

O exemplo desta página pressupõe:

```ps contexto
import mail
MailServer s = mail.MailServer()
```

---

## Exemplo

```ps sem-rodar: precisa do servidor de e-mail
s.conn("smtp.meuservidor.com")              # 25, texto puro
s.conn("smtp.meuservidor.com", 2525)        # outra porta, texto puro
s.conn("smtp.meuservidor.com", Null, True)  # 587, com STARTTLS
s.conn("smtp.meuservidor.com", 465, True)   # porta explícita, STARTTLS
```

No modo `tls=True`, se o servidor não anunciar `STARTTLS` no EHLO, a conexão
falha. No modo de texto puro, o `STARTTLS` não é tentado.

O `host` é usado como está escrito: a lib não traduz domínio de e-mail em
servidor. `s.conn("meudominio.com")` tenta `meudominio.com:25` — o endereço do
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
