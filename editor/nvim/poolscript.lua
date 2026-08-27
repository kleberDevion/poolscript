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
