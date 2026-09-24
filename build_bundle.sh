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
