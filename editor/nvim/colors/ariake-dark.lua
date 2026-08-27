-- Ariake Dark para Neovim.
--
-- Não é um tema "parecido": as cores foram tiradas do MESMO tema que ele usa
-- no VS Code — `wart.ariake-dark-0.2.2`. As da interface saíram de
-- `Ariake Dark-color-theme.json` e as do código de `Ariake Dark-color-theme
-- .tmTheme`, escopo por escopo. Um tema reescrito de olho fica parecido e
-- diverge no detalhe, que é justamente o que incomoda ao trocar de editor.
--
-- Correspondência dos escopos (TextMate -> Treesitter/Vim):
--
--   comment                  #555C77   Comment
--   keyword / storage        #7E7EDD   Keyword, Statement, Type qualifier
--   entity.name.type/class   #A571F4   Type, Structure
--   string                   #9AEFEA   String
--   constant / numeric       #DDA2F6   Number, Boolean, Constant
--   variable                 #85B1E0   Identifier
--   entity.name.function     #F5FAFF   Function
--   support.function/type    #93DDFB   Builtin, escape, regexp
--   (global)                 #B9BED5   Normal, operador, pontuação
--
-- Instalado por `make nvim` no repositório da PoolScript.

local c = {
  -- interface (do color-theme.json)
  bg          = "#2a2d37",
  bg_escuro   = "#1c1f26",   -- barra lateral, abas, status
  bg_widget   = "#232830",   -- popup, hover
  bg_sel      = "#4f5b66",
  bg_linha    = "#343d46",   -- linha atual / seleção de lista
  cursor      = "#c0c5ce",
  guia        = "#3b5364",
  branco_esp  = "#65737e",

  -- código (do tmTheme)
  fg          = "#b9bed5",
  comentario  = "#555c77",
  keyword     = "#7e7edd",
  tipo        = "#a571f4",
  string      = "#9aefea",
  constante   = "#dda2f6",
  variavel    = "#85b1e0",
  funcao      = "#f5faff",
  suporte     = "#93ddfb",
  interp      = "#4d8acb",

  -- diff (do color-theme.json)
  add         = "#43d08a",
  del         = "#e05252",
}

vim.cmd("highlight clear")
if vim.fn.exists("syntax_on") == 1 then
  vim.cmd("syntax reset")
end
vim.o.background = "dark"
vim.g.colors_name = "ariake-dark"

local function hi(grupo, spec)
  vim.api.nvim_set_hl(0, grupo, spec)
end

-- ── interface ───────────────────────────────────────────────────────────────
hi("Normal",         { fg = c.fg, bg = c.bg })
hi("NormalFloat",    { fg = c.fg, bg = c.bg_widget })
hi("FloatBorder",    { fg = c.bg_linha, bg = c.bg_widget })
hi("Cursor",         { fg = c.bg, bg = c.cursor })
hi("CursorLine",     { bg = "#31353f" })      -- lineHighlight sobre o fundo
hi("CursorLineNr",   { fg = c.fg })
hi("LineNr",         { fg = c.comentario })
hi("SignColumn",     { bg = c.bg })
hi("Visual",         { bg = c.bg_sel })
hi("ColorColumn",    { bg = c.bg_escuro })
hi("VertSplit",      { fg = c.bg_escuro })
hi("WinSeparator",   { fg = c.bg_escuro })
hi("StatusLine",     { fg = "#4f5b66", bg = c.bg_escuro })
hi("StatusLineNC",   { fg = c.comentario, bg = c.bg_escuro })
hi("TabLine",        { fg = "#65737f", bg = c.bg_escuro })
hi("TabLineSel",     { fg = c.fg, bg = c.bg })
hi("TabLineFill",    { bg = c.bg_escuro })
hi("Pmenu",          { fg = c.fg, bg = c.bg_widget })
hi("PmenuSel",       { fg = c.funcao, bg = c.bg_linha })
hi("PmenuSbar",      { bg = c.bg_widget })
hi("PmenuThumb",     { bg = c.bg_sel })
hi("Search",         { fg = c.bg, bg = c.constante })
hi("IncSearch",      { fg = c.bg, bg = c.suporte })
hi("MatchParen",     { fg = c.suporte, bold = true })
hi("Whitespace",     { fg = c.branco_esp })
hi("NonText",        { fg = c.branco_esp })
hi("Directory",      { fg = c.variavel })
hi("Title",          { fg = c.funcao, bold = true })
hi("Folded",         { fg = c.comentario, bg = c.bg_escuro })

-- ── sintaxe clássica ────────────────────────────────────────────────────────
hi("Comment",        { fg = c.comentario, italic = true })
hi("Constant",       { fg = c.constante })
hi("String",         { fg = c.string })
hi("Character",      { fg = c.string })
hi("Number",         { fg = c.constante })
hi("Boolean",        { fg = c.constante })
hi("Float",          { fg = c.constante })
hi("Identifier",     { fg = c.variavel })
hi("Function",       { fg = c.funcao })
hi("Statement",      { fg = c.keyword })
hi("Conditional",    { fg = c.keyword })
hi("Repeat",         { fg = c.keyword })
hi("Label",          { fg = c.keyword })
hi("Operator",       { fg = c.fg })
hi("Keyword",        { fg = c.keyword })
hi("Exception",      { fg = c.keyword })
hi("PreProc",        { fg = c.keyword })
hi("Include",        { fg = c.fg })
hi("Define",         { fg = c.keyword })
hi("Macro",          { fg = c.suporte })
hi("Type",           { fg = c.tipo })
hi("StorageClass",   { fg = c.keyword })
hi("Structure",      { fg = c.tipo })
hi("Typedef",        { fg = c.tipo })
hi("Special",        { fg = c.suporte })
hi("SpecialChar",    { fg = c.suporte })
hi("Delimiter",      { fg = c.fg })
hi("Underlined",     { underline = true })
hi("Error",          { fg = "#ffffff", bg = c.del })
hi("Todo",           { fg = c.constante, bold = true })

-- ── treesitter ──────────────────────────────────────────────────────────────
hi("@comment",              { link = "Comment" })
hi("@keyword",              { fg = c.keyword })
hi("@keyword.function",     { fg = c.keyword })
hi("@keyword.return",       { fg = c.keyword })
hi("@keyword.operator",     { fg = c.keyword })
hi("@conditional",          { fg = c.keyword })
hi("@repeat",               { fg = c.keyword })
hi("@exception",            { fg = c.keyword })
hi("@string",               { fg = c.string })
hi("@string.escape",        { fg = c.suporte })
hi("@string.regex",         { fg = c.suporte })
hi("@character",            { fg = c.string })
hi("@number",               { fg = c.constante })
hi("@boolean",              { fg = c.constante })
hi("@float",                { fg = c.constante })
hi("@constant",             { fg = c.constante })
hi("@constant.builtin",     { fg = c.constante })
hi("@variable",             { fg = c.variavel })
hi("@variable.builtin",     { fg = c.variavel })
hi("@parameter",            { fg = c.fg })
hi("@field",                { fg = c.variavel })
hi("@property",             { fg = c.variavel })
hi("@function",             { fg = c.funcao })
hi("@function.call",        { fg = c.funcao })
hi("@function.builtin",     { fg = c.suporte })
hi("@method",               { fg = c.funcao })
hi("@method.call",          { fg = c.funcao })
hi("@constructor",          { fg = c.tipo })
hi("@type",                 { fg = c.tipo })
hi("@type.builtin",         { fg = c.suporte })
hi("@namespace",            { fg = c.fg })
hi("@operator",             { fg = c.fg })
hi("@punctuation.delimiter",{ fg = c.fg })
hi("@punctuation.bracket",  { fg = c.fg })
hi("@punctuation.special",  { fg = c.interp })
hi("@tag",                  { fg = c.variavel })
hi("@tag.attribute",        { fg = c.constante })
hi("@text.title",           { fg = c.variavel, bold = true })
hi("@text.literal",         { fg = c.string })
hi("@text.uri",             { fg = c.keyword, underline = true })
hi("@text.strong",          { fg = c.constante, bold = true })
hi("@text.emphasis",        { fg = c.keyword, italic = true })

-- ── LSP e diagnóstico ───────────────────────────────────────────────────────
hi("DiagnosticError",       { fg = c.del })
hi("DiagnosticWarn",        { fg = c.constante })
hi("DiagnosticInfo",        { fg = c.suporte })
hi("DiagnosticHint",        { fg = c.string })
hi("DiagnosticUnderlineError", { undercurl = true, sp = c.del })
hi("DiagnosticUnderlineWarn",  { undercurl = true, sp = c.constante })
hi("DiagnosticUnderlineInfo",  { undercurl = true, sp = c.suporte })
hi("DiagnosticUnderlineHint",  { undercurl = true, sp = c.string })
hi("LspReferenceText",      { bg = c.bg_linha })
hi("LspReferenceRead",      { bg = c.bg_linha })
hi("LspReferenceWrite",     { bg = c.bg_linha })

-- Realce semântico do LSP. O servidor da PoolScript manda estes tipos, então
-- eles apontam pras mesmas cores dos escopos equivalentes do tema original.
hi("@lsp.type.keyword",     { fg = c.keyword })
hi("@lsp.type.string",      { fg = c.string })
hi("@lsp.type.number",      { fg = c.constante })
hi("@lsp.type.comment",     { link = "Comment" })
hi("@lsp.type.function",    { fg = c.funcao })
hi("@lsp.type.variable",    { fg = c.variavel })
hi("@lsp.type.type",        { fg = c.tipo })
hi("@lsp.type.operator",    { fg = c.fg })

-- ── diff / git ──────────────────────────────────────────────────────────────
hi("DiffAdd",     { fg = c.add, bg = "#1e2b26" })
hi("DiffDelete",  { fg = c.del, bg = "#2b1e1e" })
hi("DiffChange",  { fg = c.keyword, bg = c.bg_widget })
hi("DiffText",    { fg = c.funcao, bg = c.bg_linha })
hi("Added",       { fg = c.add })
hi("Removed",     { fg = c.del })
hi("Changed",     { fg = c.keyword })

-- ── fundo transparente ──────────────────────────────────────────────────────
-- Tira o fundo dos grupos que pintam a "chapa" do editor, pro fundo do terminal
-- aparecer atrás do texto. Mexe SÓ no fundo: cada grupo é relido e regravado com
-- a MESMA cor de frente e os mesmos atributos, então nenhuma cor de sintaxe muda.
--
-- Popup, hover e a linha do cursor continuam opacos de propósito — sem fundo eles
-- se misturam com o texto de baixo e ficam ilegíveis.
--
-- Para voltar ao fundo sólido: `vim.g.ariake_transparente = false` ANTES do
-- `colorscheme("ariake-dark")` no init.lua.
if vim.g.ariake_transparente ~= false then
  local sem_fundo = {
    "Normal", "NormalNC", "SignColumn", "FoldColumn", "EndOfBuffer",
    "MsgArea", "MsgSeparator", "LineNr", "CursorLineNr", "Folded",
    "StatusLine", "StatusLineNC", "TabLine", "TabLineSel", "TabLineFill",
  }
  for _, grupo in ipairs(sem_fundo) do
    local atual = vim.api.nvim_get_hl(0, { name = grupo })
    atual.bg = nil
    atual.ctermbg = nil
    vim.api.nvim_set_hl(0, grupo, atual)
  end
end
