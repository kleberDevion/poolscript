-- Configuração do Neovim.
--
-- Duas coisas: o tema Ariake Dark (o MESMO do VS Code, convertido escopo por
-- escopo em colors/ariake-dark.lua) e o servidor LSP da PoolScript.
--
-- O LSP aqui é NATIVO: não há extensão, não há empacotamento, não há cliente
-- de terceiro no meio. Se o completion funciona aqui e não no VS Code, o
-- problema está do lado do VS Code — o servidor é o mesmo binário.

vim.opt.number = true
vim.opt.termguicolors = true      -- sem isto o tema cai pra 256 cores e
                                  -- as cores saem TODAS erradas
vim.opt.expandtab = true
vim.opt.shiftwidth = 4
vim.opt.tabstop = 4
vim.opt.signcolumn = "yes"        -- diagnóstico não empurra o texto
vim.opt.updatetime = 300
vim.opt.mouse = "a"

vim.cmd.colorscheme("ariake-dark")

-- ── PoolScript ──────────────────────────────────────────────────────────────

-- O Neovim não conhece a extensão `.ps` (ele chuta PostScript, que é o mesmo
-- engano do MIME do desktop). Aqui ela é declarada como `poolscript`.
vim.filetype.add({
  extension = {
    ps  = "poolscript",
    psl = "poolscript",
    p   = "poolscript",
  },
})

-- Comentário e indentação da linguagem, pro `gc` e o `>>` funcionarem.
vim.api.nvim_create_autocmd("FileType", {
  pattern = "poolscript",
  callback = function()
    vim.bo.commentstring = "# %s"
    vim.bo.expandtab = true
    vim.bo.shiftwidth = 4
    vim.bo.tabstop = 4
  end,
})

-- Sobe o servidor ao abrir um arquivo da linguagem. `vim.lsp.start` reaproveita
-- o mesmo processo pros arquivos do mesmo projeto (a chave é `name` + `root_dir`).
vim.api.nvim_create_autocmd("FileType", {
  pattern = "poolscript",
  callback = function(args)
    -- Prefere o `poolscript-lsp` do PATH (o que o `make install` põe). Se não
    -- houver, roda o servidor direto de um repositório clonado — útil enquanto
    -- se mexe nele.
    local cmd
    if vim.fn.executable("poolscript-lsp") == 1 then
      cmd = { "poolscript-lsp" }
    else
      local repo = vim.fn.expand("~/poolscript-lang/lsp/servidor.ps")
      if vim.fn.filereadable(repo) == 1 and vim.fn.executable("pool") == 1 then
        cmd = { "pool", repo }
      else
        vim.notify(
          "PoolScript: nao achei `poolscript-lsp` no PATH. Rode `sudo make install` no repositorio.",
          vim.log.levels.WARN)
        return
      end
    end

    vim.lsp.start({
      name = "poolscript",
      cmd = cmd,
      root_dir = vim.fs.dirname(vim.fs.find({ ".git" }, {
        upward = true, path = vim.fn.expand("%:p:h"),
      })[1]) or vim.fn.getcwd(),
    })
  end,
})

-- Atalhos, só quando há servidor ligado no buffer.
vim.api.nvim_create_autocmd("LspAttach", {
  callback = function(ev)
    local opts = { buffer = ev.buf, silent = true }
    -- completion: <C-x><C-o> é o omni do Vim; o LSP passa a alimentá-lo
    vim.bo[ev.buf].omnifunc = "v:lua.vim.lsp.omnifunc"
    vim.keymap.set("n", "K",  vim.lsp.buf.hover, opts)
    vim.keymap.set("n", "gd", vim.lsp.buf.definition, opts)
    vim.keymap.set("n", "[d", vim.diagnostic.goto_prev, opts)
    vim.keymap.set("n", "]d", vim.diagnostic.goto_next, opts)
    -- <C-espaco> pede completion, como no VS Code
    vim.keymap.set("i", "<C-Space>", "<C-x><C-o>", opts)
  end,
})

-- O diagnóstico aparece na linha, não só na coluna de sinais.
vim.diagnostic.config({
  virtual_text = { prefix = "●" },
  severity_sort = true,
})

-- ── edição direta: Ctrl+S salva, o arquivo abre pronto pra digitar ──────────
--
-- O padrão do Vim é o que confunde: o arquivo abre em modo normal, onde as
-- teclas são comandos e não texto, e gravar é `:w`. Aqui o arquivo já abre em
-- inserção e o Ctrl+S grava, como em qualquer outro editor.
--
-- Os comandos do Vim continuam todos lá: `<Esc>` sai da inserção e devolve o
-- modo normal (`:q` pra sair, `u` pra desfazer, `dd` pra apagar a linha).

local function salvar()
  -- Buffer sem nome não pode ser gravado — em vez de estourar o `E32: Nenhum
  -- nome de arquivo`, pergunta o caminho.
  if vim.api.nvim_buf_get_name(0) == "" then
    local nome = vim.fn.input("Salvar como: ", vim.fn.getcwd() .. "/", "file")
    if nome == "" then
      return
    end
    vim.cmd("write " .. vim.fn.fnameescape(nome))
    return
  end
  vim.cmd("write")
end

-- O mapeamento é a função direto: nada de trocar de modo, o Ctrl+S grava no
-- meio da digitação e o cursor fica onde estava.
vim.keymap.set({ "n", "i", "v" }, "<C-s>", salvar, { desc = "salva o arquivo (Ctrl+S)" })

-- Abre digitando. Fica de fora o que não é arquivo de texto — ajuda, quickfix,
-- lista de arquivos, terminal, buffer só-leitura: neles as teclas são atalhos,
-- e entrar em inserção só atrapalharia.
vim.api.nvim_create_autocmd("BufWinEnter", {
  desc = "abre o arquivo já em modo de edição",
  callback = function(args)
    local bo = vim.bo[args.buf]
    if bo.buftype == "" and bo.modifiable and not bo.readonly then
      vim.cmd("startinsert")
    end
  end,
})

-- ── fechamento automático de ( ) [ ] { } ────────────────────────────────────
--
-- O Neovim não fecha delimitador sozinho e aqui não há gerenciador de plugin,
-- então o comportamento é escrito na mão. É o mesmo que a extensão do VS Code
-- já dá pela `autoClosingPairs` do `language-configuration.json`.
--
--   `(`   insere `()` com o cursor no meio — mas só quando depois do cursor vem
--         fim de linha, espaço ou um fechamento. Antes de texto (`|foo`) digita
--         só `(`, senão envolver um trecho existente ficaria impossível.
--   `)`   se o próximo caractere JÁ é `)`, o cursor anda por cima em vez de
--         duplicar o fechamento.
--   BS    dentro de um par vazio `(|)`, apaga os dois de uma vez.
--   CR    dentro de um par vazio, abre o bloco: o fechamento desce e o cursor
--         fica numa linha indentada no meio.

local pares = { ["("] = ")", ["["] = "]", ["{"] = "}" }

-- Caracteres imediatamente antes e depois do cursor. `col` vem 0-based (é a
-- contagem de bytes à esquerda), então o anterior é `col` e o próximo `col+1`
-- na indexação 1-based do Lua.
local function ao_redor()
  local linha = vim.api.nvim_get_current_line()
  local col = vim.api.nvim_win_get_cursor(0)[2]
  return linha:sub(col, col), linha:sub(col + 1, col + 1)
end

local function par_vazio()
  local ant, prox = ao_redor()
  return prox ~= "" and pares[ant] == prox
end

for abre, fecha in pairs(pares) do
  vim.keymap.set("i", abre, function()
    local _, prox = ao_redor()
    if prox == "" or prox:match("[%s%)%]%},;]") then
      return abre .. fecha .. "<Left>"
    end
    return abre
  end, { expr = true, desc = "fecha " .. abre .. fecha .. " sozinho" })

  vim.keymap.set("i", fecha, function()
    local _, prox = ao_redor()
    return prox == fecha and "<Right>" or fecha
  end, { expr = true, desc = "anda por cima do " .. fecha .. " já fechado" })
end

vim.keymap.set("i", "<BS>", function()
  return par_vazio() and "<BS><Del>" or "<BS>"
end, { expr = true, desc = "apaga o par vazio inteiro" })

-- Com o menu de completion aberto o <CR> é dele: mexer aqui trocaria o aceite
-- do item por uma quebra de linha.
vim.keymap.set("i", "<CR>", function()
  if vim.fn.pumvisible() == 1 then return "<CR>" end
  return par_vazio() and "<CR><Esc>O" or "<CR>"
end, { expr = true, desc = "abre bloco ao dar enter dentro do par" })
