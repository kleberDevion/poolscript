# Documentação da PoolScript — Índice

Referência completa: uma pasta por lib, e dentro dela uma página por método.
Cada página tem assinatura, parâmetros, o **porquê** e exemplos reais.

Para a **linguagem em si** (sintaxe, tipos, `if`/`while`/`for`, `Entity`,
`match`, decorators…), veja [`LANGUAGE.md`](LANGUAGE.md).

Para usar a PoolScript **em qualquer editor** (VS Code, IntelliJ IDEA,
Neovim…) com completion type-aware e diagnóstico do parser real, veja
[`lsp.md`](lsp.md) — o language server da linguagem.

---

## Builtins (sem `import`)

- **[builtins](builtins/builtins.md)** — `post`, `input`, `len`, `range`, `map`,
  `filter`, `open`, `sleep`, conversores (`str/int/flo/bool/list`), e utilitários
  de número — sempre disponíveis.
- **[string methods](string/string.md)** — métodos de texto embutidos:
  `upper`/`lower`, `strip`, `replace` (com lista!), `split`/`join`, verificações
  (`isdigit`…), e regex integrado (`match`/`findall`/`sub`).
- **[list methods](list/list.md)** — `append`/`extend`/`insert`, `pop`/`remove`,
  `sort`/`reverse`, `index`/`count`/`contains`, `copy`, `len`. A maioria **muta
  a lista** e devolve `null`.
- **[dict methods](dict/dict.md)** — `keys`/`values` (ou `value`)/`items`,
  `get` (com default), `has`, `pop`, `update`, `copy`, `len`. `x in d` testa a
  **chave**; pro **valor**, `x in d.value()`.

---

## Bibliotecas (com `import`)

### Web e rede
- **[jinker](jinker/jinker.md)** — servidor HTTP + WebSocket
- **[request](request/request.md)** — cliente HTTP + WebSocket
- **[sockets](sockets/sockets.md)** — sockets de rede crus: TCP/UDP/UNIX, cliente e servidor

### Dados e arquivos
- **[os](os/os.md)** — sistema de arquivos, ambiente, terminal
- **[manpu](manpu/manpu.md)** — arquivos CSV/XLSX/texto
- **[json](json/json.md)** — texto JSON ↔ dados
- **[bytes](bytes/bytes.md)** — criar/converter dados binários (hex, base64, inteiros)
- **[psodbc](psodbc/psodbc.md)** — banco de dados (SQLite/Postgres/MySQL/SQL Server/Mongo)

### Segurança e comunicação
- **[hash](hash/hash.md)** — senhas seguras
- **[jwt](jwt/jwt.md)** — tokens de autenticação
- **[mail](mail/mail.md)** — enviar e ler e-mails

### Interface (desktop)

### Linguagem
- **[exceptions](exceptions/exceptions.md)** — erros, `raise` (tipo livre) e `catch` (tipo/variável opcionais)

### Utilidades
- **[date](date/date.md)** — data e hora
- **[regex](regex/regex.md)** — expressões regulares
- **[dotenv](dotenv/dotenv.md)** — carregar o `.env`
- **[sys](sys/sys.md)** — runtime, saída, argumentos
- **[qrcode](qrcode/qrcode.md)** — gerar QR Codes
- **[Parsing](Parsing/Parsing.md)** — conversões de tipo tolerantes
- **[datasentity](datasentity/datasentity.md)** — `@dataentity` e conversões de Entity

---

## Apelidos (mesma lib, outro nome de import)

| Você importa | É a mesma lib de |
|---|---|
| `db` | `psodbc` |
| `mp` | `manpu` |
| `qr` | `qrcode` |
| `requests` | `request` |
| `JSON` | `json` |
| `dataentity` | `datasentity` |
