# Libs Utilitárias

---

## date — Data e Hora

```ps
import date
```

| Função | Retorno | Exemplo |
|---|---|---|
| `date.time()` | `"HH:MM:SS"` | `"14:30:00"` |
| `date.today()` | `"DD/MM/YYYY"` | `"03/05/2026"` |
| `date.datahora()` | `"DD/MM/YYYY HH:MM:SS"` | `"03/05/2026 14:30:00"` |
| `date.now()` | ISO 8601 | `"2026-05-03 14:30:00"` |
| `date.timestamp()` | Unix timestamp | `1746281400` |
| `date.hora(hours, minutes, days)` | Segundos | `86400` |

### Exemplos

```ps
import date

post(date.today())       # 03/05/2026
post(date.datahora())    # 03/05/2026 14:30:00
post(date.timestamp())   # 1746281400

# Calcular expiração pra JWT
exp_24h = date.timestamp() + date.hora(hours=24)
exp_30min = date.timestamp() + date.hora(minutes=30)
exp_7dias = date.timestamp() + date.hora(days=7)
```

---

## mail — Envio de Email

```ps
import mail
```

Os exemplos desta seção pressupõem a lib importada e um objeto de cada classe:

```ps contexto
import mail
MailServer s = mail.MailServer()
MailMessage m = mail.MailMessage()
MailReader r = mail.MailReader()
```

### Configuração

```ps
s = mail.MailServer()
s.conn("smtp.meuservidor.com")          # porta 587; outra: s.conn(host, 2525)
s.login(user="seu@gmail.com", password="sua_senha_app")
```

`.conn()` recebe o host do servidor SMTP (está nas configurações da sua conta
de e-mail) e usa como está escrito — a lib não traduz domínio em servidor.
Muitos servidores exigem uma **senha de app** — não a senha normal da conta.

### Montando e enviando

```ps sem-rodar: precisa do servidor de e-mail conectado (o .conn() é de outro bloco)
m = mail.MailMessage()
m.fromAddress("seu@gmail.com")
m.toAddress("destino@email.com")
m.subject("Assunto aqui")
m.body("Corpo do email em texto")

s.send(m)
s.quit()
```

Body em HTML:

```ps
m.body("<h1>Olá!</h1><p>Bem vindo.</p>", True)
```

### Lendo e-mails (`MailReader`)

```ps
r = mail.MailReader()
r.conn("imap.meuservidor.com")          # porta 993
r.login(user="seu@gmail.com", password="sua_senha_app")
```

`.conn()` recebe o host do servidor IMAP, com TLS, e porta 993 quando não se
passa outra: `r.conn("127.0.0.1", 1143)` pra um servidor local.

`.select()` escolhe a pasta e retorna o próprio objeto, então dá pra encadear direto com `.search()`:

```ps sem-rodar: precisa do servidor de e-mail conectado (o .conn() é de outro bloco)
emails = r.select("INBOX", True).search("UNSEEN")   # True = readonly, não marca como lida
```

`.search(criterion_type, term, limit)` aceita:

| `criterion_type` | `term` | O que faz |
|---|---|---|
| `"ALL"` | — | Todas as mensagens da pasta |
| `"UNSEEN"` | — | Só as não lidas |
| `"SUBJECT"` | obrigatório | Filtra por palavra no assunto |
| `"FROM"` | obrigatório | Filtra pelo remetente |
| `"SINCE"` | obrigatório, `"DD-Mon-YYYY"` | Mensagens recebidas depois da data |

`limit` (opcional) corta o resultado pras N mensagens mais recentes.

Cada mensagem volta como `{"id": ..., "from": ..., "subject": ..., "date": ...}` — assunto e remetente já vêm decodificados (sem `=?UTF-8?B?...?=` cru), mesmo que o servidor tenha mandado em base64/quoted-printable. `SUBJECT`/`FROM` são case-insensitive (o protocolo IMAP já garante isso) e o termo pode ter vírgula, espaço, acento — tudo bem.

```ps sem-rodar: precisa do servidor de e-mail conectado (o .conn() é de outro bloco)
emails = r.select("INBOX").search("SUBJECT", "fatura", limit=5)
for each e in emails {
    post(e["from"] " - " e["subject"] " (" e["date"] ")")
}

r.close()
```

### Corpo do e-mail (v0.6.1)

`.search()` por padrão só traz os headers (`id`/`from`/`subject`/`date`) — não baixa o corpo, que é o dado mais pesado. Duas formas de pegar:

```ps sem-rodar: precisa do servidor de e-mail conectado (o .conn() é de outro bloco)
# (a) sob demanda, só do e-mail que interessa
emails = r.select("INBOX").search("SUBJECT", "fatura")
corpo = r.body(emails[0]["id"])

# (b) já embutido em cada resultado da busca
emails = r.select("INBOX").search("SUBJECT", "fatura", include_body=True)
post(emails[0]["body"])
```

Use `(a)` quando a busca pode trazer muitos resultados e você só vai abrir alguns; use `(b)` quando já sabe que vai precisar do corpo de todos. `.body()` prefere a parte `text/plain`; se o e-mail só tiver HTML, cai pro HTML cru (sem strip de tags). Veja [mailreader_body.md](mailreader_body.md) para os detalhes.

### Exemplo completo — leitura

```ps
import mail
from dotenv import load
import os

load()

funct verificar_caixa() {
    r = mail.MailReader()
    r.conn(os.getenv("MAIL_IMAP"))
    r.login(user=os.getenv("MAIL_SYSTEM"), password=os.getenv("PASSWORD_SYSTEM"))

    nao_lidos = r.select("INBOX", True).search("UNSEEN")
    post(f"{len(nao_lidos)} email(s) não lido(s)")

    for each e in nao_lidos {
        post(f'De: {e["from"]} | Assunto: {e["subject"]} | {e["date"]}')
    }

    r.close()
}
verificar_caixa()
```

### Exemplo completo — envio

```ps
import mail
import date
import os
from dotenv import load

load()

funct enviar_email(nome, email_destino) {
    try {
        s = mail.MailServer()
        s.conn(os.getenv("MAIL_SMTP"))
        s.login(
            user=os.getenv("MAIL_SYSTEM"),
            password=os.getenv("PASSWORD_SYSTEM")
        )

        m = mail.MailMessage()
        m.fromAddress(os.getenv("MAIL_SYSTEM"))
        m.toAddress(email_destino)
        m.subject(f"Olá {nome}!")
        m.body(f"Login realizado em: {date.datahora()}")

        s.send(m)
        s.quit()
        post(f"Email enviado para {email_destino}")
        return Null
    } catch (e) {
        post(f"Erro no envio: {e}")
        return Null
    }
}
```

---

## os — Sistema Operacional

```ps
import os
```

### Variáveis de ambiente

| Função | O que faz | Exemplo |
|---|---|---|
| `os.getenv("CHAVE")` | Lê variável de ambiente (chama `dotenv.load()` sozinho) | `os.getenv("DB_PATH")` |
| `os.getenv("CHAVE", "default")` | Lê com valor padrão | `os.getenv("PORT", "8000")` |
| `os.environ(key=Null)` | Lê 1 variável (sem `dotenv.load()` automático), ou o dict inteiro se `key` for omitido | `os.environ("PATH")`, `os.environ()` |

Os exemplos de variável de ambiente desta página pressupõem o `.env` da seção
do dotenv, abaixo, na pasta do programa:

```ps contexto
import os
os.writeFile(".env", "DB_PATH=database.db\nSECRET_KEY=minha_chave_super_secreta_aqui\nMAIL_SMTP=smtp.meuservidor.com\nMAIL_IMAP=imap.meuservidor.com\nMAIL_SYSTEM=seu@gmail.com\nPASSWORD_SYSTEM=senha_app_gmail\nPORT=7700\n")
```

```ps
import os
from dotenv import load

load()

str db = os.getenv("DB_PATH")
str secret = os.getenv("SECRET_KEY")
str porta = os.getenv("PORT", "7700")

post(db)      # database.db
post(porta)   # 7700
```

```saida
database.db
7700
```

### Caminhos e arquivos

`pathFile`/`pathFolder` buscam pelo nome a partir do diretório do `.pr` em
execução e depois do `cwd` (inclusive em subpastas) — não é preciso montar o
caminho relativo à mão.

| Função | O que faz | Exemplo |
|---|---|---|
| `os.pathFile(nome)` | Caminho absoluto de um arquivo (erro se não achar) | `os.pathFile("dados.json")` |
| `os.pathFolder(nome)` | Caminho absoluto de uma pasta (erro se não achar) | `os.pathFolder("uploads")` |
| `os.exists(caminho)` | Verifica se existe (arquivo ou pasta) | `os.exists("banco.db")` |
| `os.isfile(caminho)` | É um arquivo? | `os.isfile("banco.db")` |
| `os.isdir(caminho)` | É uma pasta? | `os.isdir("uploads")` |
| `os.size(caminho)` | Tamanho em bytes | `os.size("relatorio.pdf")` |
| `os.rename(src, dst)` | Renomeia arquivo ou pasta | `os.rename("a.txt", "b.txt")` |
| `os.copy(src, dst)` | Copia arquivo | `os.copy("a.txt", "backup/a.txt")` |
| `os.move(src, dst)` | Move arquivo ou pasta | `os.move("a.txt", "arquivo/a.txt")` |
| `os.loadFile(nome, encoding=Null)` | Carrega conteúdo — binário (`.pdf/.jpg/.png/.docx`...) vira `PoolFile`, texto (`.txt/.json/.csv/.html`...) vira `str`/`dict`/`list` | `os.loadFile("dados.json")` |

`loadFile` detecta o tipo pela extensão automaticamente. Pra forçar, passe
`encoding="rb"` (só extensões binárias) ou um encoding de texto tipo `"utf-8"`
(só extensões texto) — misturar dá erro claro (`TypeError`).

`PoolFile` (o que `loadFile` devolve pra binários) tem `.name`, `.ext`,
`.size`, `.bytes()`, `.path()`, `.save(path=Null)`, `.move(destino)`,
`.copy(destino)`, `.delete()`. O `save` grava o conteúdo em disco: sem `path`
salva na pasta do script em execução com o próprio nome.

### Diretórios

| Função | O que faz | Exemplo |
|---|---|---|
| `os.cwd()` | Diretório atual | `os.cwd()` |
| `os.chdir(caminho)` | Muda o diretório atual | `os.chdir("uploads")` |
| `os.mkdir(caminho, exist_ok=False)` | Cria diretório (cria pais também) | `os.mkdir("uploads/2026", exist_ok=True)` |
| `os.rmdir(caminho, force=False)` | Remove diretório — `force=True` remove mesmo com conteúdo | `os.rmdir("tmp", force=True)` |
| `os.ls(caminho=".")` | Lista arquivos/pastas com `name`/`type`/`size` | `os.ls("./uploads")` |

### Sistema / terminal

| Função | O que faz | Exemplo |
|---|---|---|
| `os.cmd(comando, capture=False)` | Roda comando no shell; `capture=True` devolve a saída como `str` em vez de imprimir | `os.cmd("jinga --version", capture=True)` |
| `os.code(caminho=".")` | Abre o editor de código instalado (VS Code, Cursor, Zed, nano, vim, nessa ordem) no caminho | `os.code("./meu_projeto")` |
| `os.warn(texto="", color="yellow")` | Mensagem colorida no terminal (`red/green/yellow/blue/magenta/cyan/white`) | `os.warn("Pasta criada", color="blue")` |
| `os.ipmach()` | Descobre e imprime o IP da máquina, devolve como `str` | `ip = os.ipmach()` |

```ps
import os

os.mkdir("uploads", exist_ok=True)
os.warn("Pasta criada!", color="green")

if (os.isfile("uploads/antigo.txt")) {
    os.rename("uploads/antigo.txt", "uploads/backup.txt")
}

for each item in os.ls("uploads") {
    post(item["name"] " - " item["type"])
}

versao = os.cmd("jinga --version", capture=True)
post(versao)
```

---

## dotenv — Variáveis de Ambiente

```ps
from dotenv import load

load()
```

Estrutura do `.env`:

```texto
DB_PATH=database.db
SECRET_KEY=minha_chave_super_secreta_aqui
MAIL_SMTP=smtp.meuservidor.com
MAIL_IMAP=imap.meuservidor.com
MAIL_SYSTEM=seu@gmail.com
PASSWORD_SYSTEM=senha_app_gmail
PORT=7700
```

Depois de `load()`, acesse com `os.getenv()`:

```ps
from dotenv import load
import os

load()

str banco = os.getenv("DB_PATH")
str chave = os.getenv("SECRET_KEY")
str email = os.getenv("MAIL_SYSTEM")
```

---

## request — Requisições HTTP

```ps
import request
```

| Função | O que faz |
|---|---|
| `request.get(url)` | Requisição GET |
| `request.post(url, body)` | Requisição POST |
| `request.put(url, body)` | Requisição PUT |
| `request.delete(url)` | Requisição DELETE |
| `request.ws_connect(url)` | Conecta WebSocket |

`ws_connect(url)` devolve um `WsConnection`:

| Método | O que faz |
|---|---|
| `conn.send(data)` | Manda uma mensagem (dict vira JSON automaticamente) |
| `conn.on_message(callback)` | Registra a função chamada a cada mensagem recebida do servidor |
| `conn.close()` | Fecha a conexão |

**`on_message` é obrigatório se você quer ver o que o servidor manda de volta** — sem ele, mensagens recebidas (inclusive broadcasts de outros clientes) chegam na thread de leitura e são descartadas silenciosamente, sem nenhum aviso ou erro.

### Requisições HTTP

```ps sem-rodar: faz pedido HTTP a um servidor de fora
import request

resp = request.get("https://api.exemplo.com/users")
data = resp.get_json()
post(data)

resp2 = request.post(
    "https://api.exemplo.com/users",
    body={"nome": "ana", "email": "ana@email.com"}
)
post(resp2.get_json())
```

### WebSocket — cliente

```ps sem-rodar: conecta num servidor WebSocket de fora
import request

conn = request.ws_connect("ws://chat.example.com/chat")
post(conn)  # <WsConnection ws://chat.example.com/chat [conectado]>

conn.on_message(funct(msg) { post(msg) })
conn.send({"user_name": "joao", "body_msg": "Oi!"})
```

**Exemplo — chat no terminal:**

```ps sem-rodar: lê o teclado num laço e conecta num servidor WebSocket
import request

str nome = input("Seu nome: ")

conn = request.ws_connect("ws://chat.example.com/chat")
conn.on_message(funct(msg) { post(f"\n{msg}") })
post(f"Conectado como {nome}!")

while (True) {
    str msg = input("")
    if (msg == "sair") {
        conn.close()
        break
    }
    conn.send({"user_name": nome, "body_msg": msg})
}
```

Sem o `conn.on_message(...)`, dois terminais conectados no mesmo chat **não vão trocar mensagens visivelmente**: o broadcast chega em cada cliente, mas fica preso na thread de leitura porque nenhum callback foi registrado pra exibi-lo.

---

## Lambda, map e filter

### Lambda — função anônima

```ps
dobro = funct(n) { return n * 2 }
post(dobro(5))   # 10

quadrado = funct(n) { return n * n }
post(quadrado(4))  # 16
```

Os exemplos abaixo pressupõem o `dobro` e o `quadrado` de cima:

```ps contexto
Object dobro = int funct(int n) { return n * 2 }
Object quadrado = int funct(int n) { return n * n }
```

Passando lambda como argumento:

```ps
funct aplicar(func, valor) {
    return func(valor)
}

post(aplicar(dobro, 7))     # 14
post(aplicar(quadrado, 3))  # 9
```

### map()

Aplica uma função em cada item da lista:

```ps
list nums = [1, 2, 3, 4, 5]

dobrados = map(nums, funct(n) { return n * 2 })
post(dobrados)  # [2, 4, 6, 8, 10]

quadrados = map(nums, funct(n) { return n * n })
post(quadrados)  # [1, 4, 9, 16, 25]

list nomes = ["ana", "leo", "bia"]
iniciais = map(nomes, funct(n) { return n[0:1] })
post(iniciais)  # ["a", "l", "b"]
```

### filter()

Filtra itens da lista que passam na condição:

```ps
list nums = [1, 2, 3, 4, 5, 6]

pares = filter(nums, funct(n) { return n % 2 == 0 })
post(pares)  # [2, 4, 6]

maiores = filter(nums, funct(n) { return n > 3 })
post(maiores)  # [4, 5, 6]
```

Combinando map e filter:

```ps
list nums = [1, 2, 3, 4, 5, 6]

# Pega os pares e dobra
pares = filter(nums, funct(n) { return n % 2 == 0 })
resultado = map(pares, funct(n) { return n * 2 })
post(resultado)  # [4, 8, 12]
```
