# Editores — a PoolScript em VS Code, JetBrains e Neovim

O suporte a editor é um **servidor LSP escrito em PoolScript**, rodado pelo
próprio `pool`:

```bash
pool lsp/servidor.ps
```

Ele fala **Language Server Protocol** por stdin/stdout, então serve qualquer
editor que seja cliente LSP — VS Code, Neovim, Helix, Emacs, JetBrains (via
plugin LSP). Não há JavaScript no projeto e não existe extensão pra instalar:
o que se configura é o comando acima.

## O que ele entrega

- **completion type-aware** — a cadeia `conn = psodbc.connect()` →
  `DbConnection` → `cur = conn.cursor()` → `cur.` → `fetchall/fetchone/…`;
  membros de módulo (`regex.`); palavras da linguagem e as variáveis do
  arquivo quando não há receptor. Tipo desconhecido **não sugere nada** — zero
  método falso;
- **hover** — a assinatura real do método e o tipo que ele devolve;
- **diagnóstico** — `pool --check` no arquivo, ao abrir e ao salvar, com linha
  e coluna do erro.

O modelo de tipos vem do **próprio binário** (`pool --metadata`, lido das
tabelas do VM). Nada é digitado à mão, então o completion não tem como
divergir do motor.

## VS Code

O VS Code precisa de uma extensão pra saber falar com um servidor LSP, e uma
extensão de VS Code é sempre JavaScript — é regra do editor. Como aqui não há
JS, o caminho é uma extensão genérica de LSP: instale
[Generic LSP Client](https://marketplace.visualstudio.com/search?term=generic%20lsp)
(ou equivalente) e aponte pro comando:

```json
{
  "languageServerExample.command": "pool",
  "languageServerExample.args": ["/caminho/do/repo/lsp/servidor.ps"],
  "files.associations": { "*.ps": "poolscript", "*.psl": "poolscript", "*.p": "poolscript" }
}
```

## Neovim

Nativo, sem plugin nenhum:

```lua
vim.filetype.add({ extension = { ps = "poolscript", psl = "poolscript", p = "poolscript" } })

vim.api.nvim_create_autocmd("FileType", {
  pattern = "poolscript",
  callback = function()
    vim.lsp.start({
      name = "poolscript",
      cmd = { "pool", "/caminho/do/repo/lsp/servidor.ps" },
      root_dir = vim.fs.dirname(vim.fs.find({ ".git" }, { upward = true })[1]),
    })
  end,
})
```

## Helix

Em `~/.config/helix/languages.toml`:

```toml
[language-server.poolscript]
command = "pool"
args = ["/caminho/do/repo/lsp/servidor.ps"]

[[language]]
name = "poolscript"
file-types = ["ps", "psl", "p"]
language-servers = ["poolscript"]
```

## JetBrains (IntelliJ, PyCharm, …)

Pelo plugin **LSP4IJ**: `Settings → Languages & Frameworks → Language Servers`
→ `+` → *New Language Server*, comando `pool /caminho/do/repo/lsp/servidor.ps`,
extensões `ps;psl;p`.

## Por dentro (pra quem mexe no repositório)

| Arquivo | O que é |
|---|---|
| `lsp/protocolo.ps` | transporte: JSON-RPC enquadrado por `Content-Length`, sobre stdin/stdout |
| `lsp/modelo.ps` | modelo de tipos (de `pool --metadata`) e a inferência da cadeia |
| `lsp/servidor.ps` | os métodos do LSP: completion, hover, diagnóstico |
| `lsp/teste_lsp.ps` | dirige o servidor como um editor faria e confere as respostas |

O teste entra no `make check` — não é varredura à parte.

Duas notas de implementação que valem pra quem for mexer:

- o **corpo** de uma mensagem é lido com `sys.stdin.read(n)` (bytes exatos),
  nunca por linha: as mensagens LSP vêm coladas, sem `\n` entre elas, e ler
  por linha invade a mensagem seguinte;
- todo log do servidor vai pro **stderr**. `stdout` é o canal do protocolo, e
  um `post()` solto ali corrompe a conversa com o editor.
