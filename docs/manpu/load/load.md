# `manpu.load(filepath)`

Carrega o arquivo em **bytes** — o conteúdo cru, sem parsear. Útil quando você
vai **repassar** o arquivo (anexar num email, enviar por rede) em vez de ler o
conteúdo.

```
manpu.load(filepath: str) -> bytes
```

---

## Uso típico: anexar num email

```
import manpu as mp
import mail

ld = mp.load("relatorio.pdf")      # bytes do PDF

m = mail.MailMessage()
m.body(ld)                          # usa os bytes
```

---

## `load` vs `read`

- **`load`** — bytes crus (pra repassar/anexar).
- **[`read`](../read/read.md)** — conteúdo estruturado (dicts, texto) pra usar.

Se você quer *ler os dados* de uma planilha, use `read`. Se quer *pegar o
arquivo inteiro* pra mandar adiante, use `load`.

---

## Relacionados

- [`manpu.read()`](../read/read.md) — conteúdo estruturado
- lib `mail` — anexar arquivos num email
