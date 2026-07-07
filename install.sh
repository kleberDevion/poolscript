#!/usr/bin/env bash
# PoolScript Installer - Linux/Mac
# Remove versões antigas e instala a nova (Python package + extensão VSCode)
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
VSIX_FILE="$SCRIPT_DIR/vscode-poolscript/poolscript.poolscript-0.5.2.vsix"

C_GREEN='\033[0;32m'; C_YELLOW='\033[1;33m'; C_RED='\033[0;31m'; C_BLUE='\033[0;34m'; C_RESET='\033[0m'

echo -e "${C_BLUE}=========================================${C_RESET}"
echo -e "${C_BLUE}  PoolScript Installer v0.5.2${C_RESET}"
echo -e "${C_BLUE}=========================================${C_RESET}"

# ---- 1. Detecta Python ----
if command -v python3 &> /dev/null; then PY=python3
elif command -v python &> /dev/null; then PY=python
else echo -e "${C_RED}[ERRO] Python 3.10+ não encontrado.${C_RESET}"; exit 1
fi
echo -e "${C_GREEN}[OK]${C_RESET} Python: $($PY --version)"

# ---- 2. Remove versões antigas do pacote Python ----
echo -e "\n${C_YELLOW}[1/3] Removendo versões antigas do PoolScript (Python)...${C_RESET}"
$PY -m pip uninstall -y poolscript 2>/dev/null || echo "  (nenhuma versão anterior encontrada)"

# Remove pastas residuais site-packages
SITE=$($PY -c "import site; print(site.getusersitepackages())" 2>/dev/null || echo "")
for dir in "$SITE/poolscript" "$SITE"/poolscript-*.dist-info "$SITE"/poolscript-*.egg-info; do
    [ -e "$dir" ] && rm -rf "$dir" && echo "  removido: $dir"
done

# ---- 3. Instala nova versão Python ----
echo -e "\n${C_YELLOW}[2/3] Instalando PoolScript v0.5.2...${C_RESET}"
$PY -m pip install --user --upgrade "$SCRIPT_DIR"
echo -e "${C_GREEN}[OK]${C_RESET} Comandos disponíveis: pool, psl"

# ---- 4. Extensão VSCode ----
echo -e "\n${C_YELLOW}[3/3] Instalando extensão VSCode...${C_RESET}"
if command -v code &> /dev/null; then
    # Remove versões antigas da extensão
    for ext_id in poolscript.poolscript poolscript.poolscript-language; do
        if code --list-extensions 2>/dev/null | grep -qi "$ext_id"; then
            echo "  removendo extensão antiga: $ext_id"
            code --uninstall-extension "$ext_id" 2>/dev/null || true
        fi
    done
    # Limpa pasta residual ~/.vscode/extensions/poolscript*
    for d in "$HOME"/.vscode/extensions/poolscript*; do
        [ -e "$d" ] && rm -rf "$d" && echo "  removida pasta: $d"
    done
    # Instala nova
    if [ -f "$VSIX_FILE" ]; then
        code --install-extension "$VSIX_FILE" --force
        echo -e "${C_GREEN}[OK]${C_RESET} Extensão VSCode instalada."
    else
        echo -e "${C_RED}[AVISO] .vsix não encontrado em $VSIX_FILE${C_RESET}"
    fi
else
    echo -e "${C_YELLOW}[SKIP] comando 'code' (VSCode CLI) não encontrado no PATH.${C_RESET}"
    echo "  Para habilitar: VSCode > Cmd+Shift+P > 'Shell Command: Install code in PATH'"
fi

echo -e "\n${C_GREEN}=========================================${C_RESET}"
echo -e "${C_GREEN}  Instalação concluída!${C_RESET}"
echo -e "${C_GREEN}=========================================${C_RESET}"
echo "  Teste:  pool --version"
echo "          pool examples/01_hello.ps"
