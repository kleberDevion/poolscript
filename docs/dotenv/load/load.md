# `dotenv.load(path=None)`

Carrega um arquivo `.env` (linhas `CHAVE=VALOR`) para as variáveis de ambiente.
Depois disso, [`os.getenv`](../../os/getenv/getenv.md) lê os valores.

```
load(path: str = None) -> dict
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `path` | `None` | caminho do `.env`; `None` = busca automática subindo os diretórios |

---

## Uso

```
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

```
load("config/producao.env")
```

Sem argumento, `load()` sobe nos diretórios a partir do atual até achar um
`.env` — igual ao comportamento do python-dotenv.

---

## Formato do `.env`

```
# comentários com # são ignorados
DB_PATH="loja.db"
PORT=8080
SECRET_KEY='pode usar aspas simples ou duplas'
```

As aspas ao redor do valor são removidas. Valor sem aspas também funciona.

---

## Relacionados

- [`os.getenv()`](../../os/getenv/getenv.md) — ler as variáveis carregadas
- [`os.environ()`](../../os/environ/environ.md) — ver todas as variáveis
