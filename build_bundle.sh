#!/usr/bin/env bash
# Monta `dist/pool-portable/`: o binário + TODAS as .so de que ele depende +
# o servidor LSP + o tipo MIME e o ícone + o instalador. Roda em qualquer
# x86-64 com glibc compatível, SEM apt install.
#
# O núcleo do glibc (libc, libm, libpthread, libdl, librt, libresolv e o
# loader) NÃO vai junto de propósito: esse vem do alvo, senão o getaddrinfo
# (NSS) quebra. glibc é retrocompatível, então o único requisito da máquina
# alvo é ter glibc >= a do build.
set -e
cd "$(dirname "$0")"

[ -f pool ] || { echo "compile antes: make pool" >&2; exit 1; }

SAIDA=dist/pool-portable
rm -rf "$SAIDA"
mkdir -p "$SAIDA/lib" "$SAIDA/lsp" "$SAIDA/dados/icones"

cp pool "$SAIDA/pool.bin"
# O servidor LSP não é um arquivo só: `server.js` carrega `analise.js`. Levar
# apenas o server.js fazia o bundle instalar um LSP que morre ao subir.
for j in editor/vscode/*.js; do
    case "$(basename "$j")" in
        extension.js|teste_servidor.js) continue ;;
    esac
    cp "$j" "$SAIDA/lsp/"
done
cp -r editor/vscode/node_modules "$SAIDA/lsp/" 2>/dev/null || true
cp dados/zz-poolscript.xml "$SAIDA/dados/"
cp dados/icones/text-poolscript.svg "$SAIDA/dados/icones/"
cp instalar.sh "$SAIDA/"
chmod +x "$SAIDA/instalar.sh"

# as .so de que o binário precisa, menos o núcleo do glibc
NUCLEO='libc\.so|libm\.so|libpthread\.so|libdl\.so|librt\.so|libresolv\.so|ld-linux|linux-vdso'
ldd pool | awk '{print $3}' | grep '^/' | sort -u | while read -r so; do
    echo "$so" | grep -Eq "$NUCLEO" && continue
    cp -L "$so" "$SAIDA/lib/" 2>/dev/null || true
done

# wrapper: aponta o loader pras .so que foram junto
cat > "$SAIDA/pool" <<'WRAP'
#!/bin/sh
# Wrapper do bundle portátil: usa as .so que vieram na pasta lib/.
AQUI=$(dirname "$(readlink -f "$0")")
LIB="$AQUI/lib"
[ -d /usr/local/lib/poolscript ] && LIB="/usr/local/lib/poolscript"
LD_LIBRARY_PATH="$LIB:$LD_LIBRARY_PATH" exec "$AQUI/pool.bin" "$@"
WRAP
chmod +x "$SAIDA/pool"

cat > "$SAIDA/LEIAME.txt" <<'TXT'
PoolScript — bundle portátil

    ./pool programa.ps        roda sem instalar nada
    sudo ./instalar.sh        instala no sistema (binário, LSP, MIME e ícone)
    sudo ./instalar.sh --remover

Depois de instalar, `.ps` passa a ser `text/poolscript` e aparece com a logo
no gerenciador de arquivos. O servidor LSP fica em `poolscript-lsp` — aponte
o editor pra ele (ver docs/lsp.md).
TXT

tar czf dist/pool-portable.tar.gz -C dist pool-portable
echo "dist/pool-portable/  ($(du -sh "$SAIDA" | cut -f1))"
echo "dist/pool-portable.tar.gz  ($(du -h dist/pool-portable.tar.gz | cut -f1))"
