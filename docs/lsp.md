# LSP — a PoolScript em QUALQUER editor

A linguagem tem um **language server** próprio (`poolscript-lsp`), na mesma
arquitetura do Pylance: o editor é um cliente fino e o cérebro roda num
processo da linguagem, usando o **lexer/parser reais** (o mesmo que executa
seu código) e um modelo de tipos por **introspecção viva** da stdlib.

O que o servidor entrega em qualquer editor com LSP:

- **completion type-aware** — a cadeia `conn = psodbc.connect()` →
  `DbConnection` → `conn.cursor().` → `fetchall/fetchone/...`; `self.` dentro
  de `Entity`; o `request.` do jinker (proxy) vs a lib `request` (verbos);
  argumento nomeado dentro da chamada; `from ._arquivo import <TAB>` com os
  exports reais do arquivo — e tipo desconhecido **não sugere nada** (zero
  método falso);
- **diagnóstico em tempo real** — erro de sintaxe apontado pelo parser REAL
  (a mesma mensagem do `pool`), e import/variável **não usados** marcados
  apagados (estilo Pylance);
- **hover rico** — assinatura + resumo + exemplo, da mesma fonte da doc viva.

## Instalação (uma vez)

```bash
pip install poolscript[lsp]     # ou, no repositório: pip install -e ".[lsp]"
poolscript-lsp                  # existir no PATH é o que os editores precisam
```

## VS Code

Nada a fazer: a extensão **psl-poolscript** detecta o `poolscript-lsp`
sozinha (ou `python3 -m poolscript.lsp.server`) e vira cliente dele. Se o
servidor não existir na máquina, a extensão usa o cérebro embutido dela —
você não fica sem completion nunca.

Configurações (`settings.json`):

```jsonc
"poolscript.lsp.ativo": true,                 // false = só o cérebro embutido
"poolscript.lsp.comando": []                  // ex: ["python3","-m","poolscript.lsp.server"]
```

## IntelliJ IDEA (e demais IDEs JetBrains)

Dois passos: o **cérebro** (LSP) e as **cores** (TextMate).

**1. Cérebro — plugin LSP4IJ** (Red Hat; funciona no Community e no Ultimate,
sem escrever plugin):

1. `Settings → Plugins → Marketplace` → instale **LSP4IJ**.
2. `Settings → Languages & Frameworks → Language Servers` → `+` (New server):
   - **Name:** `PoolScript`
   - **Command:** `poolscript-lsp`
     (alternativa sem instalar o pacote: `python3 -m poolscript.lsp.server`,
     com a variável `PYTHONPATH` apontando pro `src/` do repositório)
   - **Mappings → File name patterns:** `*.ps;*.psl;*.p`, language id
     `poolscript`.
3. Abra um `.ps` — completion, hover e diagnósticos passam a vir do servidor.

**2. Cores — bundle TextMate** (o IDEA lê a gramática da extensão VS Code
como está):

1. `Settings → Editor → TextMate Bundles` → `+`.
2. Selecione a pasta `psl-poolscript-vsix/` do repositório (o IDEA lê o
   `package.json` + `syntaxes/poolscript.tmLanguage.json`).
3. `.ps/.psl/.p` ganham o realce completo da linguagem.

## Neovim (de brinde)

```lua
vim.filetype.add({ extension = { ps = "poolscript", psl = "poolscript", p = "poolscript" } })
vim.api.nvim_create_autocmd("FileType", {
  pattern = "poolscript",
  callback = function()
    vim.lsp.start({ name = "poolscript", cmd = { "poolscript-lsp" } })
  end,
})
```

## Por dentro (pra quem mexe no repositório)

- Servidor: `src/poolscript/lsp/server.py` (pygls/stdio) + modelo de tipos em
  `src/poolscript/lsp/metadados.py` (introspecção viva — não defasa nunca).
- Código incompleto durante a digitação (`cur.` sem nada depois) passa por
  **parse com reparo**: a linha quebrada é neutralizada e o resto do arquivo
  segue indexado, com o erro ainda aparecendo como diagnóstico.
- Testes: `tests/test_lsp.py` fala o **protocolo de verdade** por stdio
  (initialize/didOpen/completion/hover/diagnostics), como um editor faria; e
  o E2E da extensão (`psl-poolscript-vsix/test/runTest.js`) sobe o VS Code
  REAL com a extensão como cliente deste servidor.
