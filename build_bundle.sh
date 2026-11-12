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

# ── Windows (MSYS2) ──────────────────────────────────────────────────────
# `bin\jinga.exe` já é estático (só pede DLL do próprio Windows). Os clientes
# de banco são abertos no primeiro `connect` (vm/ps_dl.h) pelo LoadLibrary, que
# procura primeiro na pasta do executável: as DLLs deles e o que elas arrastam
# vão pra `bin\` ao lado. O jpkg e os nomes antigos são `.cmd` que chamam o
# jinga.exe (o jpkg é o mesmo binário, pelos subcomandos). Sai um .zip, que o
# instalar.ps1 instala.
case "$(uname -s)" in *_NT*)
    [ -f pool.exe ] || { echo "compile antes: make pool" >&2; exit 1; }
    SAIDA=dist/jinga-windows-x86_64
    rm -rf "$SAIDA"
    mkdir -p "$SAIDA/bin" "$SAIDA/share/jinga/lsp"
    cp pool.exe "$SAIDA/bin/jinga.exe"
    PREF="${MSYSTEM_PREFIX:-/ucrt64}"
    # dependências de uma DLL que NÃO são do Windows
    deps_de() { ldd "$1" 2>/dev/null | awk '{print $3}' | grep -iv '^/c/windows' | grep -i '\.dll$' || true; }
    junta() {
        local dll="$1" nome
        nome=$(basename "$dll")
        [ -f "$SAIDA/bin/$nome" ] && return 0
        cp "$dll" "$SAIDA/bin/$nome"
        for d in $(deps_de "$dll"); do junta "$d"; done
    }
    # o jinga.exe não pode depender de nada fora do Windows
    fora=$(deps_de pool.exe)
    [ -z "$fora" ] || { echo "pool.exe depende de DLL fora do Windows: $fora" >&2; exit 1; }
    for dll in "$PREF/bin/libpq.dll" "$PREF/bin/libmariadb.dll" "$PREF/bin/libmongoc2.dll"; do
        [ -f "$dll" ] || { echo "falta $dll nesta maquina: o pacote sairia sem esse driver" >&2; exit 1; }
        junta "$dll"
    done
    # os comandos, com o fim de linha do Windows
    for c in jpkg pool psl; do
        printf '@"%%~dp0jinga.exe" %%*\r\n' > "$SAIDA/bin/$c.cmd"
    done
    for c in jinga-lsp poolscript-lsp; do
        printf '@echo off\r\nrem Servidor LSP da Jinga. Ver docs/lsp.md.\r\nwhere node >nul 2>nul || (echo jinga-lsp precisa do node 1>&2 & exit /b 1)\r\nif "%%~1"=="" (node "%%~dp0..\\share\\jinga\\lsp\\server.js" --stdio) else (node "%%~dp0..\\share\\jinga\\lsp\\server.js" %%*)\r\n' \
            > "$SAIDA/bin/$c.cmd"
    done
    for j in editor/vscode/*.js; do
        case "$(basename "$j")" in
            extension.js|teste_servidor.js) continue ;;
        esac
        cp "$j" "$SAIDA/share/jinga/lsp/"
    done
    cp -r editor/vscode/node_modules "$SAIDA/share/jinga/lsp/" 2>/dev/null || true
    cp -r docs "$SAIDA/share/jinga/docs"
    mkdir -p "$SAIDA/share/jinga/scripts"
    cp scripts/migra_pr.pr "$SAIDA/share/jinga/scripts/"   # a frase do `.ps` manda rodá-lo
    cp instalar.ps1 "$SAIDA/"
    printf 'Jinga - pacote para Windows (x86-64)\r\n\r\n    bin\\jinga.exe programa.pr     roda sem instalar nada\r\n    powershell -ExecutionPolicy Bypass -File instalar.ps1\r\n                                   instala em %%LOCALAPPDATA%%\\Programs\\Jinga e poe no PATH\r\n    powershell -ExecutionPolicy Bypass -File instalar.ps1 -Remover\r\n\r\nO servidor do editor (jinga-lsp) usa o node: nodejs.org ou `winget install OpenJS.NodeJS`.\r\n' \
        > "$SAIDA/LEIAME.txt"
    rm -f dist/jinga-windows-x86_64.zip
    (cd dist && zip -qr jinga-windows-x86_64.zip jinga-windows-x86_64)
    echo "dist/jinga-windows-x86_64/  ($(du -sh "$SAIDA" | cut -f1))"
    echo "dist/jinga-windows-x86_64.zip  ($(du -h dist/jinga-windows-x86_64.zip | cut -f1))"
    exit 0
    ;;
esac

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
    mkdir -p "$SAIDA/share/jinga/scripts"
    cp scripts/migra_pr.pr "$SAIDA/share/jinga/scripts/"   # a frase do `.ps` manda rodá-lo
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
mkdir -p "$SAIDA/scripts"
cp scripts/migra_pr.pr "$SAIDA/scripts/"   # a frase do `.ps` manda rodá-lo
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
# O do Mongo muda com a série do driver com que o `pool` compilou
# (libmongoc-1.0.so.0 no 1.x, libmongoc2.so.2 no 2.x do Debian forky): sai do
# próprio binário, que guarda o MG_NOMES da série dele. Fixo no 1.x, o bundle
# parava no forky dizendo que faltava a libmongoc-1.0.so.0.
MONGO_SO=$(grep -aoE 'libmongoc(-1\.0|2)\.so\.[0-9]+' pool | head -1)
[ -n "$MONGO_SO" ] || { echo "o pool nao cita a libmongoc (MG_NOMES, vm/ps_mongo.c)" >&2; exit 1; }
# Quem diz onde cada uma mora é o ldconfig, e no Debian ele fica em /usr/sbin,
# fora do PATH de quem não é root: sem achá-lo, o `ldconfig -p` vinha vazio e o
# bundle dizia que faltava a libpq.so.5, instalada (motor-40). O /sbin/ldconfig
# é do glibc, em todo Linux.
LDCONFIG=$(command -v ldconfig 2>/dev/null || echo /sbin/ldconfig)
for nome in libpq.so.5 libmariadb.so.3 libodbc.so.2 "$MONGO_SO"; do
    so=$("$LDCONFIG" -p | awk -v n="$nome" '$1 == n {print $NF; exit}')
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
