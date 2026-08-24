# `os.getenv(key, default=None)`

Lê uma **variável de ambiente**. É como você pega segredos e configuração
(senhas, caminhos, chaves de API) sem escrever no código — normalmente vindas
de um arquivo `.env`.

```
os.getenv(key: str, default: any = None) -> str | default
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `key` | — | nome da variável (ex: `"DB_PATH"`) |
| `default` | `None` | valor devolvido se a variável não existir |

---

## Uso típico com `.env`

`.env`:

```
DB_PATH="dados.db"
SECRET_KEY="abc123"
PORT="8080"
```

No código, carregue o `.env` uma vez e leia:

```
import os
from dotenv import load

load()                                   // carrega o .env pro ambiente

str banco = os.getenv("DB_PATH")         // "dados.db"
str key = os.getenv("SECRET_KEY")      // "abc123"
porta = int(os.getenv("PORT", "3000"))   // "8080" → 8080
```

---

## O `default` protege contra ausência

Se a variável não existe e você **não** passou `default`, devolve `Null`. Com
`default`, devolve o valor que você deu:

```
modo = os.getenv("MODO", "producao")     // "producao" se MODO não existir
```

Isso evita `Null` inesperado quando a variável é opcional.

---

## Sempre volta como string

Variáveis de ambiente são sempre texto. Pra usar como número, converta:

```
porta   = int(os.getenv("PORT", "8080"))
timeout = flo(os.getenv("TIMEOUT", "1.5"))
```

---

## Relacionados

- `dotenv.load()` — carrega o `.env` (necessário antes de `getenv`)
- [`os.environ()`](../environ/environ.md) — todas as variáveis de uma vez
