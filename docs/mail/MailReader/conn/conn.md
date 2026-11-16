# `MailReader.conn(host, port=Null, tls=False)`

Conecta ao servidor **IMAP** (leitura) no `host` dado. O padrão é **texto
puro**: sem `tls=True`, a lib não abre TLS. Com `tls=True`, usa TLS direto na
porta (sem STARTTLS).

```assinatura
r.conn(host: str, port: int = Null, tls: bool = False) -> bool
```

Sem `port`, a porta depende do modo:

- `tls=False` (padrão): **143**, texto puro;
- `tls=True`: **993**, com TLS direto.

Porta explícita sempre manda. Devolve `True` quando conectou; falha de rede, de
TLS ou de host inexistente é erro (não devolve `False`).

O exemplo desta página pressupõe:

```ps contexto
import mail
MailReader r = mail.MailReader()
```

---

## Exemplo

```ps sem-rodar: precisa do servidor de e-mail
r.conn("imap.meuservidor.com")              # 143, texto puro
r.conn("127.0.0.1", 1143)                   # outra porta, texto puro
r.conn("imap.meuservidor.com", Null, True)  # 993, com TLS direto
r.conn("imap.meuservidor.com", 1993, True)  # porta explícita, TLS direto
```

O `host` é usado como está escrito: a lib não traduz domínio de e-mail em
servidor. O endereço do servidor IMAP vem das configurações da sua conta de
e-mail.

---

## Relacionados

- [`.login()`](../login/login.md) — o próximo passo
- [`MailServer.conn()`](../../MailServer/conn/conn.md) — o equivalente pra envio (SMTP)
