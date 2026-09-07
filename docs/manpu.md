# manpu — Manipulação de Arquivos

```
import manpu as mp
```

Suporta: CSV, XLSX, XML, HTML, JSON, texto puro e arquivos de código.

---

## mp.read()

Lê um arquivo e retorna o conteúdo:

```
ler = mp.read("dados.csv")
post(ler)
# [{"nome": "ana", "email": "ana@email.com"}, {"nome": "leo", ...}]
```

**Retorno por tipo:**

| Tipo | Retorno |
|---|---|
| `.csv` | Lista de dicts `[{"col": "valor"}]` |
| `.xlsx` / `.xls` | Lista de dicts |
| `.xml` | Dict com `tag`, `attrs`, `text`, `children` |
| `.html` | Texto limpo sem tags |
| `.json` | Dict ou lista |
| `.txt`, `.py`, `.ps`, outros | Texto puro |

---

## mp.load()

Carrega o arquivo em bytes — útil para enviar por email:

```
import manpu as mp
import mail

ld = mp.load("relatorio.pdf")

m = mail.MailMessage()
m.body(ld)
```

---

## mp.write()

Escreve conteúdo em um arquivo:

```
nm = mp.write(
    content="Kleber",
    column=0,
    celula=3,
    target="planilha.csv"
)

if (nm == True) {
    post(nm.status)  # Success
} else {
    post(nm.status)  # Error: ...
}
```

**Parâmetros:**

| Parâmetro | Descrição |
|---|---|
| `content` | Conteúdo a escrever |
| `column` | Número da coluna (0 = primeira) |
| `celula` | Índice da linha/célula |
| `target` | Caminho do arquivo |

Para `.txt` e outros sem estrutura, adiciona ao final do arquivo.

---

## mp.remove()

Remove conteúdo de um arquivo:

```
# Remove em CSV por coluna e célula
x = mp.remove(
    column=2,
    celula=5,
    amount=full,
    target="planilha.csv"
)

# Remove texto em HTML ou txt
x = mp.remove(
    value="Texto alvo",
    amount=full,
    target="pagina.html"
)

# Remove metade
x = mp.remove(
    value="Texto longo aqui",
    amount=mei,
    target="arquivo.txt"
)
```

**amount:**
- `full` — remove tudo que encontrar
- `mei` — remove metade do texto encontrado

---

## mp.open() com using

Abre um arquivo e mantém aberto durante as operações. Salva automaticamente ao sair do bloco:

```
import manpu as mp

lista = mp.load("compras.txt")

using mp.open(target="compras.xlsx") as arq {
    arq.write(column=0, cell=full, content=lista)
}
# arquivo salvo e fechado automaticamente
```

`using` aceita bloco com chaves `{ }`, como todo bloco da linguagem — não é
diferente de `if`/`while`/`funct` nesse sentido:

```
using mp.open(target="compras.xlsx") as arq {
    arq.write(column=0, cell=full, content=lista)
}
# arquivo salvo e fechado automaticamente
```

**Parâmetros de `mp.open()`:**

| Parâmetro | Descrição |
|---|---|
| `target` | Caminho do arquivo |
| `encoding` | Charset pra ler/escrever CSV e texto puro (default `"utf-8"`; ignorado em xlsx/xls). Use `"latin-1"`, `"cp1252"` etc. pra arquivos legados que não são utf-8 |

```
using mp.open(target="legado.csv", encoding="latin-1") as arq {
    dados = arq.read()
    post(dados)
}
```

### arq.write()

```
arq.write(
    content=lista,
    column=0,
    cell=full,
    init=0
)
```

**Parâmetros:**

| Parâmetro | Descrição |
|---|---|
| `content` | Conteúdo a escrever |
| `column` | Coluna alvo (número ou `full` pra todas) |
| `cell` / `celula` | Índice da célula, ou `full` pra descer linha a linha |
| `init` | De onde começa no conteúdo (índice, default `0`) |

### cell=full

Quando `cell=full`, o conteúdo é dividido por quebra de linha e cada parte vai numa célula descendo na coluna:

```
# compras.txt:
# Arroz
# Feijão
# Macarrão

lista = mp.load("compras.txt")

using mp.open(target="compras.csv") as arq {
    arq.write(column=0, cell=full, content=lista)
}

# resultado:
# coluna 0, linha 0 → Arroz
# coluna 0, linha 1 → Feijão
# coluna 0, linha 2 → Macarrão
```

### arq.read()

```
using mp.open(target="dados.csv") as arq {
    dados = arq.read()
    post(dados)
}
```

---

## mp.src()

Resolve o caminho absoluto de um arquivo:

```
caminho = mp.src("meu_arquivo.xlsx")
post(caminho)
# /caminho/completo/para/meu_arquivo.xlsx
```

---

## Exemplo completo

```
import manpu as mp
import mail
import os
from dotenv import load

load()

# Lê planilha de clientes
clientes = mp.read("clientes.xlsx")
post(f"Total de clientes: {len(clientes)}")

# Escreve relatório
using mp.open(target="relatorio.csv") as arq {
    for each cliente in clientes {
        arq.write(column=0, cell=full, content=cliente["nome"])
        arq.write(column=1, cell=full, content=cliente["email"])
    }
}

# Envia por email
ld = mp.load("relatorio.csv")

s = mail.MailServer()
s.conn("gmail.com")
s.login(
    user=os.getenv("MAIL_SYSTEM"),
    password=os.getenv("PASSWORD_SYSTEM")
)

m = mail.MailMessage()
m.from_address(os.getenv("MAIL_SYSTEM"))
m.to("gestor@empresa.com")
m.subject("Relatório de clientes")
m.body(ld)

s.send(m)
s.quit()
post("Relatório enviado!")
```
