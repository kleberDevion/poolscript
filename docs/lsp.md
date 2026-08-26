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
  e coluna do erro;
- **realce** (*semantic tokens*) — palavra da linguagem, string, número,
  comentário, tipo, identificador e operador, vindos do **lexer de verdade**
  (`pool --tokens`). Não existe gramática paralela pra divergir do motor:
  token novo na linguagem já nasce pintado.

O modelo de tipos vem do **próprio binário** (`pool --metadata`, lido das
tabelas do VM). Nada é digitado à mão, então o completion não tem como
divergir do motor.

## VS Code

A extensão **psl-poolscript** é o cliente. Ela não tem cérebro nenhum: são ~70
linhas de JavaScript cuja única função é levantar o `poolscript-lsp` e falar
LSP com ele — o VS Code só carrega extensão com ponto de entrada JS, e essa é
a regra do editor, não uma escolha do projeto. Completion, hover, diagnóstico
e realce vêm todos do servidor em PoolScript.

Com o `make install` (ou o `instalar.sh`) feito, não há o que configurar: a
extensão acha o `poolscript-lsp` no PATH sozinha.

Duas configurações existem, pra quando se está mexendo no servidor:

```json
{
  // roda o servidor direto do repositório, sem instalar
  "poolscript.lsp.comando": ["pool", "/caminho/do/repo/lsp/servidor.ps"],

  // desliga o servidor; sobra o realce da gramática, que é declarativo
  "poolscript.lsp.ativo": false
}
```

> Depois de trocar o `extension.js`, o VS Code **precisa recarregar a janela**
> (`Ctrl+Shift+P` → *Developer: Reload Window*) — ele mantém a extensão antiga
> em memória. Enquanto não recarrega, o que aparece no completion é a sugestão
> genérica do editor (nomes de arquivo da pasta), não a do servidor.

O painel **Saída → PoolScript** mostra a conversa com o servidor; é o primeiro
lugar a olhar quando o completion não vem.

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
| `lsp/servidor.ps` | os métodos do LSP: completion, hover, diagnóstico, realce |
| `lsp/teste_lsp.ps` | dirige o servidor como um editor faria e confere as respostas |

O teste entra no `make check` — não é varredura à parte.

Notas de implementação pra quem for mexer:

- o **corpo** de uma mensagem é lido com `sys.stdin.read(n)` (bytes exatos),
  nunca por linha: as mensagens LSP vêm coladas, sem `\n` entre elas, e ler
  por linha invade a mensagem seguinte;
- todo log do servidor vai pro **stderr**. `stdout` é o canal do protocolo, e
  um `post()` solto ali corrompe a conversa com o editor;
- o modelo de tipos vem de `sys.executable --metadata`, ou seja, do binário que
  está rodando o servidor — **não** do `pool` do PATH. Um `pool` instalado mais
  velho descreveria um motor que não é o que o usuário está usando.

Dois comandos do binário existem pra servir o editor:

| Comando | Devolve |
|---|---|
| `pool --metadata` | módulos, tipos e métodos, das tabelas do VM |
| `pool --tokens` | tokens do lexer (fonte pelo stdin), com posição e tamanho no fonte |
