# `MailReader.search(criterion_type="ALL", term=None, limit=None, include_body=false)`

Busca e-mails na pasta selecionada. Devolve uma **lista de dicts**, um por
e-mail encontrado.

```
r.search(criterion_type="ALL", term=None, limit=None, include_body=false) -> list
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `criterion_type` | `"ALL"` | o filtro (ver tabela abaixo) |
| `term` | `None` | o termo de busca (exigido por alguns critérios) |
| `limit` | `None` | máximo de e-mails a trazer (os mais recentes) |

> **Use `limit`.** Cada e-mail do resultado custa um `FETCH` ao servidor (é de
> lá que saem `from`, `subject` e `date`). Numa caixa com milhares de
> mensagens, `search("ALL")` sem limite faz milhares de idas e vindas e
> demora minutos. `search("ALL", limit=20)` traz os 20 mais recentes e pronto.
| `include_body` | `false` | `true` = já traz o corpo de cada e-mail |

---

## Critérios

| Critério | Precisa de `term`? | Traz |
|---|---|---|
| `"ALL"` | não | todos os e-mails |
| `"UNSEEN"` | não | só os não lidos |
| `"SUBJECT"` | **sim** | e-mails com o termo no assunto |
| `"FROM"` | **sim** | e-mails de um remetente |
| `"SINCE"` | **sim** | e-mails a partir de uma data |

---

## Formato do retorno

Cada item é um dict:

```
{"id": "42", "from": "ana@x.com", "subject": "Olá", "date": "Mon, 25 Jul..."}
```

Com `include_body=true`, ganha também a chave `"body"` com o texto do e-mail.

---

## Exemplos

```
// os 10 mais recentes
emails = r.search("ALL", limit=10)

// só não lidos
novos = r.search("UNSEEN")

// por assunto (precisa de term)
faturas = r.search("SUBJECT", term="fatura")

// de um remetente, já com o corpo
de_ana = r.search("FROM", term="ana@x.com", include_body=true)
for each e in de_ana {
    post(e["subject"])
    post(e["body"])
}
```

---

## Corpo separado (mais leve)

Se você não passou `include_body`, pegue o corpo só dos e-mails que interessam
com [`.body(id)`](../body/body.md) — evita baixar o conteúdo de todos:

```
for each e in r.search("ALL", limit=20) {
    if (e["subject"] == "importante") {
        texto = r.body(e["id"])
        post(texto)
    }
}
```

---

## Relacionados

- [`.body()`](../body/body.md) — corpo de um e-mail específico
- [`.select()`](../select/select.md) — escolher a pasta antes
