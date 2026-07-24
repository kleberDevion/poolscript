# `MailReader.close()`

Encerra a conexão IMAP. Chame ao terminar de ler.

```
r.close() -> None
```

---

## Uso

```
r = mail.MailReader()
r.conn("gmail.com")
r.login(user, senha)
r.select("INBOX")
emails = r.search("ALL", limit=10)
// ... processa ...
r.close()          // fecha
```

Seguro chamar mesmo se a conexão já caiu — não quebra.

---

## Relacionados

- [`.search()`](../search/search.md) — buscar antes de fechar
- [visão geral do MailReader](../MailReader.md)
