# dotenv — Carregar o arquivo `.env`

Lib pra carregar um arquivo `.env` (segredos e configuração) pras variáveis de
ambiente, que você lê depois com [`os.getenv`](../os/getenv/getenv.md).

```
from dotenv import load
```

| Membro | O que faz | Página |
|---|---|---|
| `load(path=None)` | carrega o `.env` pro ambiente | [load/load.md](load/load.md) |

---

## Por que usar

Você **nunca** escreve senhas, chaves de API e caminhos direto no código
(qualquer um que veja o código veria os segredos, e eles vazam pro git). Em vez
disso, põe num `.env` (que fica fora do git) e carrega com `dotenv`.

`.env`:

```
DB_PATH="loja.db"
SECRET_KEY="abc123"
MAIL_PASS="senha-de-app"
```

Código:

```
import os
from dotenv import load

load()                              // carrega o .env

banco = os.getenv("DB_PATH")        // "loja.db"
chave = os.getenv("SECRET_KEY")     // "abc123"
```

---

## Relacionados

- [`load()`](load/load.md) — os detalhes
- [`os.getenv()`](../os/getenv/getenv.md) — ler as variáveis depois de carregar
