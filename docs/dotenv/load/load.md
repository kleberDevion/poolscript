# `dotenv.load(path=Null)`

Carrega um arquivo `.env` (linhas `CHAVE=VALOR`) para as variáveis de ambiente.
Depois disso, [`os.getenv`](../../os/getenv/getenv.md) lê os valores.

```assinatura
load(path: str = Null) -> dict
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `path` | `Null` | caminho do `.env`; `Null` = busca automática subindo os diretórios |

---

## Uso

```ps
import os
from dotenv import load

load()                          # acha e carrega o .env automaticamente

usuario = os.getenv("MAIL_USER")
senha = os.getenv("MAIL_PASS")
```

Chame `load()` **uma vez**, no início do programa, antes de qualquer `getenv`.

---

## Caminho específico

Se o `.env` não está na pasta padrão, aponte:

```ps
load("config/producao.env")
```

Sem argumento, `load()` sobe nos diretórios a partir do atual até achar um
`.env`.

---

## Formato do `.env`

```texto
# comentários com # são ignorados
DB_PATH="loja.db"
PORT=8080
SECRET_KEY='pode usar aspas simples ou duplas'
```

As aspas ao redor do valor são removidas. Valor sem aspas também funciona. A
linha pode ter qualquer tamanho (um token longo de API cabe; antes a linha
era cortada em 4.096 bytes, e o resto virava outra chave).

O [`os.getenv`](../../os/getenv/getenv.md) e o
[`os.environ`](../../os/environ/environ.md) carregam sozinhos o `.env` achado
a partir da pasta atual: o `load()` é pra apontar outro arquivo, ou pra ter o
dict do que foi lido.

---

## Relacionados

- [`os.getenv()`](../../os/getenv/getenv.md) — ler as variáveis carregadas
- [`os.environ()`](../../os/environ/environ.md) — ver todas as variáveis
