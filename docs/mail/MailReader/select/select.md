# `MailReader.select(folder="INBOX", readonly=True)`

Escolhe qual **pasta** da caixa você vai ler (entrada, enviados, etc.). Precisa
ser chamado antes de [`.search()`](../search/search.md).

```assinatura
r.select(folder: str = "INBOX", readonly: bool = True) -> self
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `folder` | `"INBOX"` | nome da pasta (caixa de entrada por padrão) |
| `readonly` | `True` | `True` = só leitura (não marca e-mails como lidos) |

Os exemplos desta página pressupõem:

```ps contexto
import mail
MailReader r = mail.MailReader()
```

---

## Uso

```ps sem-rodar: precisa do servidor de e-mail conectado (o .conn() é de outro bloco)
r.select("INBOX")            # caixa de entrada
r.select("[Gmail]/Enviados") # outra folder (nome depende do provedor)
```

`readonly=True` (padrão) garante que abrir a caixa **não** marca os e-mails
como lidos — bom pra só inspecionar.

Devolve o próprio reader, então dá pra encadear:

```ps sem-rodar: precisa do servidor de e-mail conectado (o .conn() é de outro bloco)
emails = r.select("INBOX").search("ALL", limit=5)
```

---

## Relacionados

- [`.search()`](../search/search.md) — buscar depois de selecionar
- [`.login()`](../login/login.md) — logar antes
