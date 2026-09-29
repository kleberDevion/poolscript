#!/usr/bin/env bash
# Monta `dist/jinga-portable/`: o binário + TODAS as .so de que ele depende +
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

# ── macOS ────────────────────────────────────────────────────────────────
# O mesmo pacote, no formato do macOS: `bin/jinga` + as .dylib que não são do
# sistema em `lib/jinga/`, com os caminhos reescritos pra serem RELATIVOS ao
# executável (`@executable_path/../lib/jinga/...`). Assim ele roda da pasta
# do pacote e, instalado, de /usr/local (bin/ e lib/jinga/ lado a lado) —
# sem Homebrew na máquina de destino. Os clientes de banco (abertos só no
# primeiro `connect`, ver vm/ps_dl.h) vão junto, com o que eles arrastam.
if [ "$(uname -s)" = "Darwin" ]; then
    SAIDA=dist/jinga-macos-x86_64
    rm -rf "$SAIDA"
    mkdir -p "$SAIDA/bin" "$SAIDA/lib/jinga" "$SAIDA/share/jinga/lsp"
    cp pool "$SAIDA/bin/jinga"
    DEST="$SAIDA/lib/jinga"

    # dependências de um arquivo Mach-O que NÃO são do sistema
    deps_de() {
        otool -L "$1" | tail -n +2 | awk '{print $1}' \
          | grep -Ev '^(/usr/lib/|/System/|@)' || true
    }
    # copia `$1` e tudo que ele arrasta (recursivo, sem repetir)
    junta() {
        local so="$1" nome
        nome=$(basename "$so")
        [ -f "$DEST/$nome" ] && return 0
        cp -L "$so" "$DEST/$nome"
        chmod u+w "$DEST/$nome"
        for d in $(deps_de "$so"); do junta "$d"; done
    }
    for d in $(deps_de pool); do junta "$d"; done
    BREW=$(brew --prefix)
    for so in \
        "$(brew --prefix libpq)/lib/libpq.5.dylib" \
        "$(brew --prefix mariadb-connector-c)/lib/mariadb/libmariadb.3.dylib" \
        "$BREW/lib/libodbc.2.dylib" \
        "$(brew --prefix mongo-c-driver@1)/lib/libmongoc-1.0.0.dylib"; do
        [ -f "$so" ] || { echo "falta $so nesta maquina: o pacote sairia sem esse driver" >&2; exit 1; }
        junta "$so"
    done
    # reescreve: o binário acha as libs em ../lib/jinga; cada lib acha as
    # outras na própria pasta
    for d in $(deps_de pool); do
        install_name_tool -change "$d" "@executable_path/../lib/jinga/$(basename "$d")" "$SAIDA/bin/jinga"
    done
    for f in "$DEST"/*.dylib; do
        install_name_tool -id "@loader_path/$(basename "$f")" "$f"
        for d in $(otool -L "$f" | tail -n +2 | awk '{print $1}' | grep -Ev '^(/usr/lib/|/System/|@)' || true); do
            install_name_tool -change "$d" "@loader_path/$(basename "$d")" "$f"
        done
    done
    # mexer nos comandos de carga invalida a assinatura: assina de novo (ad hoc)
    for f in "$SAIDA/bin/jinga" "$DEST"/*.dylib; do codesign -s - -f "$f" >/dev/null 2>&1; done
    ln -sf jinga "$SAIDA/bin/jpkg"
    for j in editor/vscode/*.js; do
        case "$(basename "$j")" in
            extension.js|teste_servidor.js) continue ;;
        esac
        cp "$j" "$SAIDA/share/jinga/lsp/"
    done
    cp -R editor/vscode/node_modules "$SAIDA/share/jinga/lsp/" 2>/dev/null || true
    cp -R docs "$SAIDA/share/jinga/docs"
    cp instalar.sh "$SAIDA/"
    chmod +x "$SAIDA/instalar.sh"
    cat > "$SAIDA/LEIAME.txt" <<'TXT'
Jinga — pacote para macOS (Intel, x86-64)

    ./bin/jinga programa.pr      roda sem instalar nada
    sudo ./instalar.sh           instala em /usr/local (jinga, jpkg, jinga-lsp)
    sudo ./instalar.sh --remover

O servidor do editor (jinga-lsp) usa o node: instale-o pelo instalador
oficial (nodejs.org) ou pelo Homebrew.
TXT
    tar czf dist/jinga-macos-x86_64.tar.gz -C dist jinga-macos-x86_64
    echo "dist/jinga-macos-x86_64/  ($(du -sh "$SAIDA" | cut -f1))"
    echo "dist/jinga-macos-x86_64.tar.gz  ($(du -h dist/jinga-macos-x86_64.tar.gz | cut -f1))"
    exit 0
fi

SAIDA=dist/jinga-portable
rm -rf "$SAIDA"
mkdir -p "$SAIDA/lib" "$SAIDA/lsp" "$SAIDA/dados/icones"

cp pool "$SAIDA/jinga.bin"
# O servidor LSP não é um arquivo só: `server.js` carrega `analise.js`. Levar
# apenas o server.js fazia o bundle instalar um LSP que morre ao subir.
for j in editor/vscode/*.js; do
    case "$(basename "$j")" in
        extension.js|teste_servidor.js) continue ;;
    esac
    cp "$j" "$SAIDA/lsp/"
done
cp -r editor/vscode/node_modules "$SAIDA/lsp/" 2>/dev/null || true
# A DOC vai junto: o servidor LSP lê o hover das páginas (`docs/linguagem`,
# `docs/<lib>/...`) a partir de `<prefixo>/share/jinga/docs`. Sem ela no
# bundle, a máquina instalada por ele tinha hover de palavra-chave vazio — ou
# pior, uma cópia velha de outro `make install`, dizendo `action`.
cp -r docs "$SAIDA/docs"
cp dados/zz-jinga.xml "$SAIDA/dados/"
cp dados/icones/text-jinga.svg "$SAIDA/dados/icones/"
cp instalar.sh "$SAIDA/"
chmod +x "$SAIDA/instalar.sh"

# as .so de que o binário precisa, menos o núcleo do glibc
NUCLEO='libc\.so|libm\.so|libpthread\.so|libdl\.so|librt\.so|libresolv\.so|ld-linux|linux-vdso'
ldd pool | awk '{print $3}' | grep '^/' | sort -u | while read -r so; do
    echo "$so" | grep -Eq "$NUCLEO" && continue
    cp -L "$so" "$SAIDA/lib/" 2>/dev/null || true
done

# E os clientes de banco, que a `jinga` abre só no primeiro `connect` do driver
# (vm/ps_dl.h) — por isso o `ldd` acima não os vê. Os nomes são os primeiros
# de PG_NOMES, MY_NOMES, OD_NOMES (vm/ps_db.c) e MG_NOMES (vm/ps_mongo.c). Cada
# um vai com o que ELE arrasta (Kerberos, LDAP, gnutls...). Faltando um nesta
# máquina, o bundle sairia sem aquele driver: para aqui em vez de sair capenga.
for nome in libpq.so.5 libmariadb.so.3 libodbc.so.2 libmongoc-1.0.so.0; do
    so=$(ldconfig -p | awk -v n="$nome" '$1 == n {print $NF; exit}')
    [ -n "$so" ] || { echo "falta $nome nesta maquina: o bundle sairia sem esse driver" >&2; exit 1; }
    for dep in "$so" $(ldd "$so" | awk '{print $3}' | grep '^/'); do
        echo "$dep" | grep -Eq "$NUCLEO" && continue
        cp -L "$dep" "$SAIDA/lib/" 2>/dev/null || true
    done
done

# wrapper: aponta o loader pras .so que foram junto. `pool` é o nome de antes
# do rename e continua valendo.
cat > "$SAIDA/jinga" <<'WRAP'
#!/bin/sh
# Wrapper do bundle portátil: usa as .so que vieram na pasta lib/.
AQUI=$(dirname "$(readlink -f "$0")")
LIB="$AQUI/lib"
[ -d /usr/local/lib/jinga ] && LIB="/usr/local/lib/jinga"
LD_LIBRARY_PATH="$LIB:$LD_LIBRARY_PATH" exec "$AQUI/jinga.bin" "$@"
WRAP
chmod +x "$SAIDA/jinga"
cp "$SAIDA/jinga" "$SAIDA/pool"

cat > "$SAIDA/LEIAME.txt" <<'TXT'
Jinga — bundle portátil

    ./jinga programa.pr       roda sem instalar nada
    sudo ./instalar.sh        instala no sistema (binário, LSP, MIME e ícone)
    sudo ./instalar.sh --remover

Depois de instalar, `.pr` passa a ser `text/jinga` e aparece com a logo no
gerenciador de arquivos. O servidor LSP fica em `jinga-lsp` — aponte o editor
pra ele (ver docs/lsp.md). Os comandos `pool`, `psl` e `poolscript-lsp`, os
nomes de antes do rename, são instalados junto e continuam valendo.
TXT

tar czf dist/jinga-portable.tar.gz -C dist jinga-portable
echo "dist/jinga-portable/  ($(du -sh "$SAIDA" | cut -f1))"
echo "dist/jinga-portable.tar.gz  ($(du -h dist/jinga-portable.tar.gz | cut -f1))"
