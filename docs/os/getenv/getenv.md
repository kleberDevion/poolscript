# `os.getenv(key, default=Null)`

Lê uma **variável de ambiente**. É como você pega segredos e configuração
(senhas, caminhos, chaves de API) sem escrever no código — normalmente vindas
de um arquivo `.env`.

```assinatura
os.getenv(key: str, default: any = Null) -> str | Null | default
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `key` | — | nome da variável (ex: `"DB_PATH"`) |
| `default` | `Null` | valor devolvido se a variável não existir |

Os exemplos desta página pressupõem:

```ps contexto
import os
```

---

## Uso típico com `.env`

`.env`:

```texto
DB_PATH="dados.db"
SECRET_KEY="abc123"
PORT="8080"
```

No código, é só ler: o `getenv` carrega sozinho o `.env` que achar subindo a
partir da pasta atual (o mesmo que o [`dotenv.load()`](../../dotenv/load/load.md)
sem argumento faz), sem passar por cima de variável que já está no ambiente.
O `load()` serve pra apontar outro arquivo ou pra pegar o dict do que foi lido.

```ps sem-rodar: lê DB_PATH e SECRET_KEY do ambiente de quem roda
import os

str banco = os.getenv("DB_PATH")         # "dados.db"
str key = os.getenv("SECRET_KEY")        # "abc123"
int porta = int(os.getenv("PORT", "3000"))   # "8080" → 8080
```

---

## O `default` protege contra ausência

Se a variável não existe e você **não** passou `default`, devolve `Null`. Com
`default`, devolve o valor que você deu:

```ps
modo = os.getenv("MODO", "producao")     # "producao" se MODO não existir
```

Isso evita `Null` inesperado quando a variável é opcional.

---

## Sempre volta como string

Variáveis de ambiente são sempre texto. Pra usar como número, converta:

```ps
porta   = int(os.getenv("PORT", "8080"))
timeout = flo(os.getenv("TIMEOUT", "1.5"))
```

Nome com o byte nulo (`\x00`) é `ValueError`: o sistema leria a variável do
pedaço de antes dele (`os.getenv("PATH\x00X")` devolvia o `PATH`). O mesmo
vale no [`os.environ(nome)`](../environ/environ.md).

---

## Relacionados

- [`dotenv.load()`](../../dotenv/load/load.md) — carrega um `.env` apontado, e devolve o dict
- [`os.environ()`](../environ/environ.md) — todas as variáveis de uma vez (também vê o `.env`)
