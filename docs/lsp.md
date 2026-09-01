# Editores — a PoolScript em VS Code, JetBrains e Neovim

O suporte a editor é um **servidor LSP sobre `vscode-languageserver`** — a
implementação de referência do protocolo, a mesma que as extensões sérias do
VS Code usam:

```bash
poolscript-lsp          # instalado por `make install`; precisa de node
```

Ele fala **Language Server Protocol** por stdin/stdout, então serve qualquer
editor que seja cliente LSP — VS Code, Neovim, Helix, Emacs, JetBrains.

**Por que não é escrito em PoolScript.** Era, e o resultado foi ruim: o
servidor anterior implementava o protocolo à mão, anunciava QUATRO capacidades
e respondia `-32601` pra todo o resto — sem ir-pra-definição, sem outline, sem
signature help. E o pouco que fazia, fazia adivinhando com busca de string no
texto cru, o que dava:

| escrito no editor | o que acontecia |
|---|---|
| `import json as js` → `js.` | ZERO sugestão — o `as` era ignorado |
| `import random` | ZERO — lib instalada em `~/.poolscript/libs` não era catalogada |
| `regex.sub("(", ` | o parêntese DENTRO DA STRING quebrava o detector |
| `f(` com `action f(a, b)` | função LOCAL não oferecia parâmetro nenhum |
| `regex.sub("a", "b", ` | reoferecia os cinco parâmetros, inclusive os dois já dados |
| dentro de comentário | despejava a lista de módulos inteira |

Reimplementar protocolo não é onde está o valor. O que é NOSSO — e continua
sendo — é o conhecimento da linguagem, e esse vem do motor.

## O que ele entrega

- **completion type-aware** — a cadeia `conn = psodbc.connect()` →
  `DbConnection` → `cur = conn.cursor()` → `cur.` → `fetchall/fetchone/…`;
  membros de módulo (`regex.`); módulos e variáveis do arquivo quando não há
  receptor. Tipo desconhecido **não sugere nada** — zero método falso.

  Cada sugestão carrega três coisas, e é isso que a faz valer: o **nome**, a
  **assinatura real com o tipo de retorno** (`regex.compile(pattern, flags) ->
  Pattern`) e a **frase da doc** explicando o que faz. Palavra da linguagem
  **não entra**: o editor já completa palavra do próprio buffer, e uma lista
  de `for`/`while` rotulada "palavra da linguagem" só empurra a sugestão útil
  pra baixo;
- **hover** — a assinatura real do método e o tipo que ele devolve;
- **diagnóstico** — `pool --check` no arquivo, ao abrir e ao salvar, com linha
  e coluna do erro;
- **realce** (*semantic tokens*) — palavra da linguagem, string, número,
  comentário, tipo, identificador e operador, vindos do **lexer de verdade**
  (`pool --tokens`). Não existe gramática paralela pra divergir do motor:
  token novo na linguagem já nasce pintado.

O modelo de tipos vem do **próprio binário** (`pool --metadata`, lido das
tabelas do VM). Nada é digitado à mão, então nem o completion nem o hover
têm como divergir do motor.

A **prosa** vem de `docs/<escopo>/<nome>/<nome>.md` — a mesma página que o
`scripts/audita_doc.ps` confere contra o motor. Não há texto digitado no
servidor: se a doc muda, a sugestão muda junto; se a página não existe, a
sugestão vem sem prosa em vez de vir com invenção. É lido sob demanda e
memorizado, então completar `regex.` toca ~10 arquivos, não 353.

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
  "poolscript.pool": "/caminho/do/repo/pool",

  // desliga o servidor; sobra o realce da gramática, que é declarativo
  "poolscript.lsp.ativo": false
}
```

> Depois de trocar o `extension.js`, o VS Code **precisa recarregar a janela**
> (`Ctrl+Shift+P` → *Developer: Reload Window*) — ele mantém a extensão antiga
> em memória — o processo antigo do servidor continua rodando até isso.

O painel **Saída → PoolScript** mostra a conversa com o servidor; é o primeiro
lugar a olhar quando algo não vem — foi ele que mostrou o enquadramento
quebrado que derrubava o servidor a cada acento.

## Neovim

Nativo, sem plugin nenhum. A configuração pronta está em
`editor/nvim/poolscript.lua` e se instala com:

```bash
make nvim          # vai pra ~/.config/nvim/init.lua
```

Se já existir um `init.lua` diferente, ele vira `init.lua.bak-<data>` e o novo
entra. **Antes o alvo recusava sobrescrever**, e o resultado foi o pior dos
dois mundos: a config da máquina ficou dias numa versão truncada — sem o
fechamento automático de `()`/`[]`/`{}` e sem o modo de edição permanente — e o
alvo dizia "não vou sobrescrever" toda vez, o que se lê como "está tudo certo".

O que a config entrega, além do LSP:

| | |
|---|---|
| menu de sugestão | abre **sozinho** enquanto se digita, como no VS Code (depois de um `.` ou de duas letras). `<C-Space>` força |
| `()` `[]` `{}` | fecham sozinhos; `)` sobre o fechamento já existente anda por cima; `<BS>` num par vazio apaga os dois |
| modo de edição | permanente — `Ctrl+S` salva, `Ctrl+Z` alterna |
| `K` / `gd` / `[d` `]d` | hover, ir pra definição, navegar diagnóstico |

O `completeopt` usa `noselect,noinsert` de propósito: o menu aparece, mas nada
é escrito no buffer até você escolher — o `<CR>` continua sendo quebra de linha.

O mínimo, se preferir montar a sua:

```lua
vim.filetype.add({ extension = { ps = "poolscript", psl = "poolscript", p = "poolscript" } })

vim.api.nvim_create_autocmd("FileType", {
  pattern = "poolscript",
  callback = function()
    vim.lsp.start({
      name = "poolscript",
      cmd = { "poolscript-lsp" },        -- ou { "node", "<repo>/editor/vscode/server.js", "--stdio" }
      root_dir = vim.fs.dirname(vim.fs.find({ ".git" }, { upward = true })[1]),
    })
  end,
})
```

`editor/nvim/teste_nvim.sh` roda no `make check`: sobe um Neovim de verdade com
essa config, confere que os pares fecham, que o menu abre sozinho, que o
servidor anexa e que o completion responde. PULA sem `nvim`.

## Helix

Em `~/.config/helix/languages.toml`:

```toml
[language-server.poolscript]
command = "node"
args = ["/caminho/do/repo/editor/vscode/server.js"]

[[language]]
name = "poolscript"
file-types = ["ps", "psl", "p"]
language-servers = ["poolscript"]
```

## JetBrains (IntelliJ, PyCharm, …)

Pelo plugin **LSP4IJ**: `Settings → Languages & Frameworks → Language Servers`
→ `+` → *New Language Server*, comando `poolscript-lsp`,
extensões `ps;psl;p`.

## Por dentro (pra quem mexe no repositório)

| Arquivo | O que é |
|---|---|
| `editor/vscode/server.js` | o servidor, sobre `vscode-languageserver` |
| `editor/vscode/extension.js` | o cliente do VS Code — só levanta o servidor |
| `editor/vscode/teste_servidor.js` | dirige o servidor como o editor faria e confere as respostas |

O teste entra no `make check` (PULA sem node, dizendo que pulou). Cada caso
dele é uma das linhas da tabela lá em cima: são defeitos reproduzidos, não
features inventadas.

**A divisão, que é a razão do desenho:**

| camada | quem faz |
|---|---|
| protocolo | `vscode-languageserver` — sync incremental, capacidades, cancelamento |
| análise léxica | `pool --tokens`, o lexer DE VERDADE. `STR` e `COMMENT` chegam como um token cada, então parêntese dentro de string ou comentário não existe como pontuação — a família inteira de defeitos de detecção some por construção |
| o que a linguagem tem | `pool --metadata`, das tabelas do VM |
| diagnóstico | `pool --check` |
| prosa | `docs/<escopo>/<nome>/<nome>.md` |

Nenhuma lista de módulo, método ou lib é digitada no servidor. Se o motor
ganha um método, o completion ganha junto, sem ninguém tocar em nada.

O binário que o servidor consulta vem do cliente (`poolscript.pool`) ou do
PATH — apontar outro faria o completion descrever um motor diferente do que o
usuário roda.

Comandos do binário que existem pra servir o editor:

| Comando | Devolve |
|---|---|
| `pool --metadata` | módulos, tipos e métodos, das tabelas do VM |
| `pool --tokens` | tokens do lexer (fonte pelo stdin), com posição e tamanho |
| `pool --check` | o erro de compilação em JSON, com linha e coluna |
