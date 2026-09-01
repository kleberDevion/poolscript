#!/usr/bin/env bash
# Instala a PoolScript numa máquina: o binário, o servidor LSP, o tipo MIME do
# `.ps` e o ícone. Roda tanto no repositório quanto dentro do bundle portátil —
# não precisa de make, compilador nem código-fonte.
#
#   sudo ./instalar.sh                 # instala em /usr/local (+ MIME em /usr/share)
#   sudo ./instalar.sh /opt/poolscript # outro prefixo
#   sudo ./instalar.sh --remover
#
# O MIME vai pra /usr/share MESMO com outro prefixo: é a base onde o
# PostScript está definido, e o `.ps` só resolve pra PoolScript se a nossa
# definição estiver na mesma base.
set -e
cd "$(dirname "$0")"

PREFIXO="${1:-/usr/local}"
DADOS=/usr/share

if [ "$1" = "--remover" ]; then
    rm -f /usr/local/bin/pool /usr/local/bin/psl /usr/local/bin/poolscript-lsp
    rm -rf /usr/local/share/poolscript
    rm -f "$DADOS/mime/packages/zz-poolscript.xml"
    rm -f "$DADOS/icons/hicolor/scalable/mimetypes/text-poolscript.svg"
    update-mime-database "$DADOS/mime" 2>/dev/null || true
    gtk-update-icon-cache -f -t "$DADOS/icons/hicolor" 2>/dev/null || true
    echo "removido."
    exit 0
fi

if [ "$(id -u)" != "0" ]; then
    echo "precisa de root: sudo ./instalar.sh" >&2
    exit 1
fi

# O binário: `pool` no repositório, `pool.bin` no bundle (lá o `pool` é o
# wrapper que aponta o LD_LIBRARY_PATH pras .so que vão junto).
BIN=pool
[ -f pool.bin ] && BIN=pool.bin
if [ ! -f "$BIN" ]; then
    echo "não achei o binário ($BIN) ao lado deste script" >&2
    exit 1
fi

echo "== binário"
install -d "$PREFIXO/bin"
if [ -f pool.bin ]; then
    install -m755 pool.bin "$PREFIXO/bin/pool.bin"
    install -m755 pool     "$PREFIXO/bin/pool"      # wrapper
    install -d "$PREFIXO/lib/poolscript"
    cp -a lib/. "$PREFIXO/lib/poolscript/" 2>/dev/null || true
else
    install -m755 pool "$PREFIXO/bin/pool"
fi
ln -sf "$PREFIXO/bin/pool" "$PREFIXO/bin/psl"

echo "== servidor LSP"
install -d "$PREFIXO/share/poolscript/lsp"
install -m644 editor/vscode/server.js \
        "$PREFIXO/share/poolscript/lsp/"
for m in vscode-languageserver vscode-languageserver-protocol \
         vscode-languageserver-types vscode-jsonrpc \
         vscode-languageserver-textdocument semver; do
    [ -d "editor/vscode/node_modules/$m" ] || continue
    mkdir -p "$PREFIXO/share/poolscript/lsp/node_modules"
    rm -rf "$PREFIXO/share/poolscript/lsp/node_modules/$m"
    cp -r "editor/vscode/node_modules/$m" "$PREFIXO/share/poolscript/lsp/node_modules/"
done
printf '#!/bin/sh\n# Servidor LSP da PoolScript. Ver docs/lsp.md.\nif ! command -v node >/dev/null 2>&1; then\n  echo "poolscript-lsp precisa do node" >&2\n  exit 1\nfi\nexec node %s/share/poolscript/lsp/server.js "${@:---stdio}"\n' \
        "$PREFIXO" > "$PREFIXO/bin/poolscript-lsp"
chmod 755 "$PREFIXO/bin/poolscript-lsp"

echo "== tipo MIME e ícone do .ps"
install -d "$DADOS/mime/packages" "$DADOS/icons/hicolor/scalable/mimetypes"
install -m644 dados/zz-poolscript.xml "$DADOS/mime/packages/"
install -m644 dados/icones/text-poolscript.svg \
        "$DADOS/icons/hicolor/scalable/mimetypes/"
update-mime-database "$DADOS/mime" 2>/dev/null || true
gtk-update-icon-cache -f -t "$DADOS/icons/hicolor" 2>/dev/null || true

echo
echo "pronto:"
echo "  pool             $("$PREFIXO/bin/pool" --version 2>/dev/null)"
echo "  psl              mesmo binário"
echo "  poolscript-lsp   servidor LSP (aponte o editor pra ele)"
echo "  .ps              text/poolscript, com a logo"
