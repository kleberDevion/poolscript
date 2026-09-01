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
      -- direto do repositório clonado, útil enquanto se mexe no servidor.
      -- (Apontava pro `lsp/servidor.ps`, que era o servidor em PoolScript e
      -- não existe mais: hoje o servidor é `editor/vscode/server.js`, sobre
      -- `vscode-languageserver`.)
      local repo = vim.fn.expand("~/poolscript-lang/editor/vscode/server.js")
      if vim.fn.filereadable(repo) == 1 and vim.fn.executable("node") == 1 then
        cmd = { "node", repo, "--stdio" }
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

-- ── o menu de sugestão aparece SOZINHO ──────────────────────────────────────
--
-- No VS Code o menu abre enquanto se digita. Aqui não abria: o completion
-- existia, mas só sob `<C-x><C-o>` — quem não conhece o atalho conclui, com
-- razão, que "o LSP não funciona".
--
-- Não há gerenciador de plugin aqui, então é na mão. O `vim.lsp.completion`
-- com `autotrigger` só chegou no Neovim 0.11; nesta versão o caminho é pedir o
-- omni quando a palavra começa a tomar forma.
--
--   `menuone`   mostra o menu mesmo com UM candidato (senão ele completa
--               sozinho e você nem vê o que aconteceu)
--   `noselect`  não pré-seleciona: o <CR> continua sendo quebra de linha até
--               você escolher alguma coisa de propósito
--   `noinsert`  não escreve no buffer enquanto você navega o menu
vim.opt.completeopt = { "menu", "menuone", "noselect", "noinsert" }
vim.opt.pumheight = 12            -- menu gigante tapa o código

local pedindo = false

vim.api.nvim_create_autocmd("TextChangedI", {
  desc = "abre o menu de sugestão enquanto digita",
  callback = function(ev)
    if vim.bo[ev.buf].buftype ~= "" then return end
    if vim.fn.pumvisible() == 1 or pedindo then return end
    -- só quando há servidor NESTE buffer: sem isto o omni é o do Vim e o menu
    -- vira lista de palavras do arquivo, que atrapalha mais do que ajuda
    local clientes = vim.lsp.get_active_clients({ bufnr = ev.buf })
    if #clientes == 0 then return end

    local linha = vim.api.nvim_get_current_line()
    local col = vim.api.nvim_win_get_cursor(0)[2]
    local antes = linha:sub(1, col)
    -- dispara depois de um `.` (membro) ou de duas letras (nome). Uma letra só
    -- abriria o menu a cada tecla e a lista seria o mundo inteiro.
    if not (antes:match("%.$") or antes:match("[%w_][%w_]$")) then return end

    pedindo = true
    vim.api.nvim_feedkeys(vim.api.nvim_replace_termcodes("<C-x><C-o>", true, false, true), "n", false)
    vim.schedule(function() pedindo = false end)
  end,
})

-- O diagnóstico aparece na linha, não só na coluna de sinais.
vim.diagnostic.config({
  virtual_text = { prefix = "●" },
  severity_sort = true,
})

-- ── modo de edição permanente ───────────────────────────────────────────────
--
-- O editor fica SEMPRE em modo de edição: toda tecla escreve texto, nenhuma
-- vira comando. É daí que vinha o conflito — no Vim padrão, uma tecla solta no
-- modo normal apaga a linha, cola, desfaz, e sem avisar que fez isso.
--
--   Ctrl+S   salva
--   Ctrl+Z   sai do modo de edição; outro Ctrl+Z volta pra ele
--
-- Nenhuma outra tecla tira você da edição: o próprio `<Esc>` devolve o cursor
-- pra digitação. Os comandos do Vim não foram apagados — eles ficam do outro
-- lado do Ctrl+Z, onde você só chega de propósito. Lá valem `:w`, `:q`, `u`,
-- `dd` e o resto, como sempre.

-- Sair da inserção recua o cursor um caractere. Como aqui a saída acontece e se
-- desfaz sozinha, sem isso cada <Esc> esbarrado andaria com o ponto de
-- digitação pra trás. `onemore` deixa o cursor parar depois do último caractere.
vim.opt.virtualedit = "onemore"

local function editavel(buf)
  local bo = vim.bo[buf]
  return bo.buftype == "" and bo.modifiable and not bo.readonly
end

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
    if editavel(args.buf) then
      vim.cmd("startinsert")
    end
  end,
})

-- O Ctrl+Z é a ÚNICA porta de saída da edição. A bandeira distingue a saída que
-- ele pediu daquela que aconteceu sozinha (um <Esc> esbarrado, um comando que
-- terminou); só a primeira é respeitada.
local saindo_de_proposito = false

-- As teclas que saíam da edição sem querer ficam inertes AQUI, antes de sair —
-- deixar sair e voltar depois abre uma fresta em que o Vim recua o cursor e
-- ainda processa a tecla seguinte como comando.
-- Com o menu de completion aberto o <Esc> ainda serve pra fechá-lo.
vim.keymap.set("i", "<Esc>", function()
  return vim.fn.pumvisible() == 1 and "<C-e>" or ""
end, { expr = true, desc = "não sai da edição (fecha o completion, se houver)" })
vim.keymap.set("i", "<C-c>", "<Nop>", { desc = "não sai da edição" })

vim.keymap.set("i", "<C-z>", function()
  saindo_de_proposito = true
  vim.cmd("stopinsert")
end, { desc = "sai do modo de edição" })

-- Voltar pra edição tem um detalhe: no modo normal o cursor fica SOBRE um
-- caractere, não entre dois, então um `startinsert` seco começaria a digitar uma
-- posição à esquerda de onde você parou. O `a` insere DEPOIS do caractere sob o
-- cursor, que é exatamente o ponto de onde a edição saiu — mas só quando o
-- cursor continua lá; se você andou pelo modo normal, quem manda é a posição
-- nova, e aí é `i` mesmo.
--
-- As teclas vão pela fila (`feedkeys`) de propósito: um `:normal! gi` fecharia a
-- inserção ao terminar o comando, o InsertLeave chamaria esta função de novo e o
-- editor travava em laço — foi o que aconteceu no primeiro teste.
local function entra_na_edicao()
  local marca = vim.api.nvim_buf_get_mark(0, "^")
  local atual = vim.api.nvim_win_get_cursor(0)
  local tecla = (marca[1] == atual[1] and marca[2] == atual[2] + 1) and "a" or "i"
  vim.api.nvim_feedkeys(tecla, "n", false)
end

-- No modo normal o Ctrl+Z do Vim suspende o processo e o editor "some" da tela.
-- Aqui ele faz o contrário do de cima: devolve a digitação.
vim.keymap.set("n", "<C-z>", function()
  if editavel(0) then
    entra_na_edicao()
  end
end, { desc = "volta pro modo de edição" })

vim.api.nvim_create_autocmd("InsertLeave", {
  desc = "só o Ctrl+Z tira do modo de edição; o resto volta pra ele",
  callback = function(args)
    if saindo_de_proposito then
      saindo_de_proposito = false
      return
    end
    if editavel(args.buf) then
      -- agendado: dentro do próprio InsertLeave o `startinsert` seria descartado
      vim.schedule(function()
        if vim.api.nvim_get_mode().mode == "n" and editavel(0) then
          entra_na_edicao()
        end
      end)
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
