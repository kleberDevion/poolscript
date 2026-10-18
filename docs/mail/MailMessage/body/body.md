# `MailMessage.body(content, is_html=False)`

Define o **corpo** do e-mail — o texto principal. Pode ser texto simples ou
HTML.

```assinatura
m.body(content, is_html: bool = False) -> Null
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `content` | — | o corpo (texto, HTML, ou bytes de um arquivo) |
| `is_html` | `False` | `True` = o conteúdo é HTML e vai ser renderizado |

Os exemplos desta página pressupõem:

```ps contexto
import mail
MailMessage m = mail.MailMessage()
```

---

## Texto simples

```ps
m.body("Olá! Segue o relatório em anexo.")
```

## HTML

```ps
m.body("<h1>Olá!</h1><p>Segue o <b>relatório</b>.</p>", is_html=True)
```

Com `is_html=True`, o cliente de e-mail renderiza as tags (negrito, links,
imagens…). Sem, o texto aparece cru.

---

## Relacionados

- [`.attach()`](../attach/attach.md) — anexar um arquivo
- [`.subject()`](../subject/subject.md) — o assunto
