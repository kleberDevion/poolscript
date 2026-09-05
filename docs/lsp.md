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
- **Entity: campos, métodos, `private` e herança.** `self.` dentro de um
  método lista os campos e métodos daquela Entity **e os herdados do pai**;
  `c = Conta(...)` seguido de `c.` lista os mesmos, agora **sem** os `private`
  — a mesma regra que a VM impõe em runtime (07-entity §7.6), porque o
  completion não pode contar uma história diferente da do motor. Conta como
  campo o declarado no corpo (`saldo: int` ou `int saldo`), o declarado no
  construtor (`private str nome = n`, §7.6.1) e o criado com `self.x = …`.
  Vale também pra Entity de **outro arquivo**, pelo `import`.

  > O nome de Entity é `IDENT_UPPER` no lexer (§1.4) e o servidor exigia
  > `IDENT`. Uma linha — e com ela toda Entity, de todo arquivo, era invisível:
  > `self.` dava zero, `c.` dava zero, e o nome da classe não aparecia em lista
  > nenhuma.

- **escopo local** — parâmetro da action que contém o cursor e variável ligada
  antes dele (atribuição, `for each`, desempacotamento). Módulo que o arquivo
  **não importou** não entra na lista: digitar `f` oferecia `flask` porque o
  servidor despejava todo módulo do motor em qualquer ponto do arquivo. O
  lugar deles é depois do `import`, e é lá que estão;
- **hover** — a assinatura real do método e o tipo que ele devolve, com a
  prosa da página `docs/…` achada pelo **caminho** (`jinker/request/get`);
  numa **palavra-chave** (`if`, `for each`, `try`, `action`, `return`…) a seção
  de `docs/linguagem/` cujo título a traz em crase; numa variável, o tipo
  construído (`Jinker mapping`) e a linha; num parâmetro, a action dona; numa
  action do arquivo, `int async action f(...)` e o decorador em cima; num
  `model`, os campos;
- **outline** — a classe como um nó, com campos e métodos aninhados dentro;
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
  # roda o servidor direto do repositório, sem instalar
  "poolscript.pool": "/caminho/do/repo/pool",

  # desliga o servidor; sobra o realce da gramática, que é declarativo
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

```bash
make intellij      # compila o plugin, sincroniza a gramática e instala
```

São três peças, e as três vêm do repositório:

| peça | o que dá | onde |
|---|---|---|
| plugin | ícone por extensão (`.ps` → **PS** azul, `.p` → **P** azul, `.psl` → **&lt;PSL/&gt;** vermelho), indentação no Enter, auto-fechamento de bracket/aspas com type-over | `editor/intellij/plugin` |
| bundle TextMate | o realce — **cópia** da gramática do vsix, fonte única lá | `editor/intellij/bundle` |
| LSP4IJ | completion, hover, diagnóstico: o mesmo `poolscript-lsp` do VS Code | plugin do marketplace |

Depois do `make intellij`, **reinicie o IDEA** (plugin só carrega no boot) e:

- realce: `Settings → Editor → TextMate Bundles → +` →
  `editor/intellij/bundle/PoolScript.tmbundle`;
- LSP: `Settings → Languages & Frameworks → Language Servers → +` →
  *New Language Server*, comando `poolscript-lsp`, extensões `ps;psl;p`.

> O fonte do plugin morava em `ideia-icons/` e foi apagado junto com centenas
> de arquivos no commit `d91f2e9`. O `.jar` continuou instalado e funcionando,
> então nada acusou — mas sem o fonte ele não se reconstrói, não acompanha a
> gramática e não vai pra outra máquina. Um binário instalado não é uma
> entrega. O `build.sh` compila contra **stubs** (só as assinaturas usadas):
> não precisa de Gradle nem do SDK do IntelliJ, só de um `javac`.

## Tema

A extensão traz **PoolScript C# Dark** — a paleta do C# no VS Code:

| cor | onde |
|---|---|
| `#569CD6` | palavra da linguagem, tipo primitivo, `self`, `private`/`public` |
| `#C586C0` | controle de fluxo (`if`, `return`, `for each`) |
| `#4EC9B0` | nome de tipo — a Entity e o pai dela |
| `#DCDCAA` | nome de método, chamada e decorador |
| `#9CDCFE` | parâmetro, variável local, campo |
| `#CE9178` string · `#B5CEA8` número · `#6A9955` comentário | |

Escolha em `Ctrl+K Ctrl+T → PoolScript C# Dark`. A gramática emite os escopos
padrão que essas cores esperam (`entity.name.type.class`, `variable.parameter`,
`variable.language.self`, …), então o visual fica próximo do C# **em qualquer
tema dark** — o tema só fecha a paleta exata.

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
