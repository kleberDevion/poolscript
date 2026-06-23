#!/usr/bin/env bash
# PoolScript Uninstaller - Linux/Mac
set -e
C_YELLOW='\033[1;33m'; C_GREEN='\033[0;32m'; C_RESET='\033[0m'

echo -e "${C_YELLOW}Desinstalando PoolScript...${C_RESET}"

if command -v python3 &> /dev/null; then PY=python3; else PY=python; fi
$PY -m pip uninstall -y poolscript 2>/dev/null || true

SITE=$($PY -c "import site; print(site.getusersitepackages())" 2>/dev/null || echo "")
for dir in "$SITE/poolscript" "$SITE"/poolscript-*.dist-info "$SITE"/poolscript-*.egg-info; do
    [ -e "$dir" ] && rm -rf "$dir"
done

if command -v code &> /dev/null; then
    for ext in poolscript.poolscript poolscript.poolscript-language; do
        code --uninstall-extension "$ext" 2>/dev/null || true
    done
fi
for d in "$HOME"/.vscode/extensions/poolscript*; do
    [ -e "$d" ] && rm -rf "$d"
done

echo -e "${C_GREEN}PoolScript removido.${C_RESET}"
