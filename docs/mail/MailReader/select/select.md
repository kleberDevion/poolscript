# `MailReader.select(pasta="INBOX", readonly=true)`

Escolhe qual **pasta** da caixa você vai ler (entrada, enviados, etc.). Precisa
ser chamado antes de [`.search()`](../search/search.md).

```
r.select(pasta: str = "INBOX", readonly: bool = true) -> self
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `pasta` | `"INBOX"` | nome da pasta (caixa de entrada por padrão) |
| `readonly` | `true` | `true` = só leitura (não marca e-mails como lidos) |

---

## Uso

```
r.select("INBOX")            // caixa de entrada
r.select("[Gmail]/Enviados") // outra pasta (nome depende do provedor)
```

`readonly=true` (padrão) garante que abrir a caixa **não** marca os e-mails
como lidos — bom pra só inspecionar.

Devolve o próprio reader, então dá pra encadear:

```
emails = r.select("INBOX").search("ALL", limit=5)
```

---

## Relacionados

- [`.search()`](../search/search.md) — buscar depois de selecionar
- [`.login()`](../login/login.md) — logar antes
