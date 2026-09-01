#!/bin/sh
# Confere o Neovim de verdade: a config carrega, os pares fecham sozinhos, o
# LSP anexa e o completion devolve o que o motor sabe.
#
# POR QUE ISTO EXISTE: o `~/.config/nvim/init.lua` da máquina ficou DIAS numa
# versão truncada — sem o fechamento automático de `()`/`[]`/`{}` e sem o modo
# de edição permanente — e nada acusava. O alvo `make nvim` via que o arquivo
# diferia e RECUSAVA sobrescrever, o que se lê como "está tudo certo".
#
# Roda contra o arquivo do REPOSITÓRIO, não contra a config da máquina: o que
# se testa é o que se entrega.
#
#     editor/nvim/teste_nvim.sh [caminho-do-pool]
#
# PULA sem `nvim` — nunca finge que passou.
set -u

REPO=$(cd "$(dirname "$0")/../.." && pwd)
CFG="$REPO/editor/nvim/poolscript.lua"
POOL=${1:-pool}

if ! command -v nvim >/dev/null 2>&1; then
  echo "PULOU o teste do Neovim — nvim nao esta instalado"
  exit 0
fi
if ! command -v poolscript-lsp >/dev/null 2>&1; then
  echo "PULOU o teste do Neovim — falta \`poolscript-lsp\` (rode \`sudo make install\`)"
  exit 0
fi

# Config isolada: o teste NAO pode depender do que ha em ~/.config/nvim, nem
# mexer nele. `XDG_CONFIG_HOME` reaponta a raiz inteira.
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/nvim/colors"
cp "$CFG" "$TMP/nvim/init.lua"
cp "$REPO/editor/nvim/colors/ariake-dark.lua" "$TMP/nvim/colors/" 2>/dev/null || true

printf 'import regex\nx = regex.\n' > "$TMP/alvo.ps"

falhas=0
ok()   { echo "  ok     $1"; }
falha() { echo "  FALHOU $1"; [ $# -gt 1 ] && echo "         $2"; falhas=$((falhas + 1)); }

SAIDA="$TMP/saida.txt"
SAIDA_TESTE="$SAIDA" XDG_CONFIG_HOME="$TMP" timeout 60 nvim --headless "$TMP/alvo.ps" \
  -c 'lua
local out = {}
local function diz(k, v) out[#out+1] = k .. "=" .. tostring(v) end

-- 1. os pares fecham sozinhos: o mapa de insercao tem que existir
diz("mapa_abre_paren", vim.fn.maparg("(", "i") ~= "")
diz("mapa_abre_colch", vim.fn.maparg("[", "i") ~= "")
diz("mapa_abre_chave", vim.fn.maparg("{", "i") ~= "")
diz("mapa_fecha_paren", vim.fn.maparg(")", "i") ~= "")
diz("mapa_backspace", vim.fn.maparg("<BS>", "i") ~= "")

-- 2. o menu de sugestao abre sozinho e nao escreve por conta propria
local co = table.concat(vim.opt.completeopt:get(), ",")
diz("completeopt_menuone", co:find("menuone") ~= nil)
diz("completeopt_noinsert", co:find("noinsert") ~= nil)
diz("autotrigger", #vim.api.nvim_get_autocmds({event = "TextChangedI"}) > 0)

-- 3. o modo de edicao permanente
diz("ctrl_s_salva", vim.fn.maparg("<C-s>", "i") ~= "")
diz("ctrl_z_sai", vim.fn.maparg("<C-z>", "i") ~= "")

-- 4. o LSP anexa e RESPONDE
local anexou = vim.wait(15000, function()
  return #vim.lsp.get_active_clients({ bufnr = 0 }) > 0
end, 100)
diz("lsp_anexou", anexou)

local n = 0
if anexou then
  vim.api.nvim_win_set_cursor(0, { 2, 10 })
  local r = vim.lsp.buf_request_sync(0, "textDocument/completion",
                                     vim.lsp.util.make_position_params(), 8000)
  for _, res in pairs(r or {}) do
    local its = res.result and (res.result.items or res.result) or {}
    n = math.max(n, #its)
  end
end
diz("sugestoes_de_regex", n)

vim.fn.writefile(out, vim.env.SAIDA_TESTE)' \
  -c 'qa!' >/dev/null 2>&1

if [ ! -f "$SAIDA" ]; then
  echo "  FALHOU o nvim nao produziu saida — a config nao carregou?"
  exit 1
fi

val() { grep "^$1=" "$SAIDA" | cut -d= -f2; }

for k in mapa_abre_paren mapa_abre_colch mapa_abre_chave mapa_fecha_paren mapa_backspace; do
  [ "$(val $k)" = "true" ] && ok "$k" || falha "$k" "o fechamento automatico de () [] {} sumiu da config"
done
for k in completeopt_menuone completeopt_noinsert autotrigger; do
  [ "$(val $k)" = "true" ] && ok "$k" || falha "$k" "o menu de sugestao nao abre sozinho"
done
for k in ctrl_s_salva ctrl_z_sai; do
  [ "$(val $k)" = "true" ] && ok "$k" || falha "$k" "o modo de edicao permanente sumiu"
done
[ "$(val lsp_anexou)" = "true" ] && ok "lsp_anexou" || falha "lsp_anexou" "o servidor nao subiu no nvim"
sug=$(val sugestoes_de_regex)
[ "${sug:-0}" -ge 5 ] && ok "completion do LSP responde ($sug sugestoes)" \
                      || falha "completion do LSP" "veio $sug sugestao(oes) pra \`regex.\`"

echo
if [ "$falhas" -gt 0 ]; then
  echo "nvim: $falhas checagem(ns) FALHARAM"
  exit 1
fi
echo "nvim: config carrega, pares fecham, LSP anexa e completa"
