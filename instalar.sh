#!/usr/bin/env bash
# Instala a PoolScript numa máquina: o binário (`pool`/`psl`), o servidor LSP,
# o tipo MIME do `.ps` e o ícone.
#
# Numa máquina onde não há NADA — um WSL Debian recém-criado, por exemplo —
# esta linha faz tudo sozinha, buscando o que faltar:
#
#   curl -fsSL https://raw.githubusercontent.com/kleberDevion/poolscript-lang/main/instalar.sh | sudo bash
#
# Ela pega o pacote pronto do último release; não havendo release publicado,
# clona o fonte, instala as dependências de compilação e compila. Nos dois
# caminhos não sobra passo manual.
#
# Rodando dentro do repositório ou do bundle portátil, instala o que está ao
# lado e não busca nada:
#
#   sudo ./instalar.sh                 # instala em /usr/local (+ MIME em /usr/share)
#   sudo ./instalar.sh /opt/poolscript # outro prefixo
#   sudo ./instalar.sh --remover
#
# O MIME vai pra /usr/share MESMO com outro prefixo: é a base onde o
# PostScript está definido, e o `.ps` só resolve pra PoolScript se a nossa
# definição estiver na mesma base.
set -e

REPO="${REPO:-https://github.com/kleberDevion/poolscript-lang}"
PACOTE="$REPO/releases/latest/download/pool-portable.tar.gz"

# Vindo de `curl | bash` não existe arquivo nem pasta ao lado ($0 é "bash"), e
# o `cd` levaria pra um lugar sem relação nenhuma com a PoolScript.
if [ -f "$0" ]; then
    cd "$(dirname "$0")"
fi

PREFIXO=/usr/local
DADOS=/usr/share
REMOVER=0
for arg in "$@"; do
    case "$arg" in
        --remover) REMOVER=1 ;;
        -*)        echo "opção desconhecida: $arg" >&2; exit 1 ;;
        *)         PREFIXO="$arg" ;;
    esac
done

if [ "$(id -u)" != "0" ]; then
    echo "precisa de root: sudo ./instalar.sh" >&2
    exit 1
fi

if [ "$REMOVER" = 1 ]; then
    rm -f  "$PREFIXO/bin/pool" "$PREFIXO/bin/pool.bin" \
           "$PREFIXO/bin/psl"  "$PREFIXO/bin/poolscript-lsp"
    # `lib/poolscript` (as .so do bundle) ficava pra trás: o --remover tirava
    # o binário e deixava os 37 MB de biblioteca instalados.
    rm -rf "$PREFIXO/share/poolscript" "$PREFIXO/lib/poolscript"
    rm -f "$DADOS/mime/packages/zz-poolscript.xml"
    rm -f "$DADOS/icons/hicolor/scalable/mimetypes/text-poolscript.svg"
    update-mime-database "$DADOS/mime" 2>/dev/null || true
    gtk-update-icon-cache -f -t "$DADOS/icons/hicolor" 2>/dev/null || true
    echo "removido."
    exit 0
fi

tem() { command -v "$1" >/dev/null 2>&1; }

apt_instala() {
    tem apt-get || return 1
    if [ -z "${APT_ATUALIZADO:-}" ]; then
        DEBIAN_FRONTEND=noninteractive apt-get update -qq || true
        APT_ATUALIZADO=1
    fi
    DEBIAN_FRONTEND=noninteractive apt-get install -y -qq "$@"
}

baixa() {   # baixa <destino> <url>
    if   tem curl; then curl -fsSL --retry 3 -o "$1" "$2"
    elif tem wget; then wget -q -O "$1" "$2"
    else return 1
    fi
}

# As dependências de compilação, na ordem em que o link as pede. Os nomes são
# de Debian/Ubuntu; o pacote do OpenLDAP trocou de nome entre versões
# (`libldap2-dev` → `libldap-dev`), então as duas grafias são tentadas.
#
# `postgresql-server-dev-all` está aqui porque a libpq entra ESTÁTICA, e a
# `libpq.a` referencia 33 símbolos que moram na `libpgcommon.a`/`libpgport.a`.
# O `libpq-dev` sozinho não traz essas duas — no Debian 13 ele instala só a
# `libpq.a`, e o link morre em `cannot find -lpgcommon`.
DEPS_BUILD="build-essential git pkg-config nodejs npm
            libsqlite3-dev libpq-dev postgresql-server-dev-all
            unixodbc-dev libssl-dev libpng-dev
            libexpat1-dev zlib1g-dev libmariadb-dev libzstd-dev libltdl-dev
            libkrb5-dev libmongoc-dev libbson-dev libgmp-dev"

compila_do_fonte() {   # compila_do_fonte <pasta_temporária>
    local tmp="$1"
    if ! tem apt-get; then
        echo "sem release publicado e esta máquina não é Debian/Ubuntu:" >&2
        echo "compile do fonte à mão — $REPO" >&2
        exit 1
    fi
    echo "== instalando as dependências de compilação"
    apt_instala $DEPS_BUILD
    apt_instala libldap-dev || apt_instala libldap2-dev
    echo "== clonando o fonte"
    git clone --depth 1 "$REPO" "$tmp/fonte"
    echo "== compilando (leva alguns minutos)"
    make -C "$tmp/fonte" -j"$(nproc)" pool
    # O `node_modules` do LSP não é versionado: num clone limpo ele não existe,
    # e sem ele o servidor sobe e morre na primeira linha.
    ( cd "$tmp/fonte/editor/vscode" && npm install --omit=dev --silent ) \
        || echo "   aviso: npm install falhou — o LSP fica sem dependências"
}

# O binário é `pool` no repositório e `pool.bin` no bundle (lá o `pool` é o
# wrapper que aponta o LD_LIBRARY_PATH pras .so que vão junto). Sem nenhum dos
# dois ao lado, não há o que instalar: busca.
if [ ! -f pool ] && [ ! -f pool.bin ]; then
    TMP=$(mktemp -d)
    trap 'rm -rf "$TMP"' EXIT
    if ! tem curl && ! tem wget; then
        echo "== instalando o curl (esta máquina não tem curl nem wget)"
        apt_instala curl || {
            echo "sem curl/wget e sem apt: instale um dos dois" >&2
            exit 1
        }
    fi
    echo "== procurando o pacote pronto"
    if baixa "$TMP/pool-portable.tar.gz" "$PACOTE"; then
        tar xzf "$TMP/pool-portable.tar.gz" -C "$TMP"
        ACHADO=$(find "$TMP" -name pool.bin -type f | head -1)
        if [ -z "$ACHADO" ]; then
            echo "o pacote baixado não tem pool.bin dentro" >&2
            exit 1
        fi
        echo "   pacote pronto — nada pra compilar"
        cd "$(dirname "$ACHADO")"
    else
        echo "   não há release publicado — clonando e compilando"
        compila_do_fonte "$TMP"
        cd "$TMP/fonte"
    fi
fi

echo "== binário"
install -d "$PREFIXO/bin"
if [ -f pool.bin ]; then
    install -m755 pool.bin "$PREFIXO/bin/pool.bin"
    install -d "$PREFIXO/lib/poolscript"
    cp -a lib/. "$PREFIXO/lib/poolscript/" 2>/dev/null || true
    # O wrapper que vem no bundle aponta pra /usr/local fixo. Instalado em
    # outro prefixo ele carregaria as .so de um caminho que não existe — daí
    # ser escrito aqui, com o prefixo real.
    printf '#!/bin/sh\n# Wrapper da PoolScript: usa as .so instaladas junto do binário.\nLD_LIBRARY_PATH=%s/lib/poolscript:$LD_LIBRARY_PATH exec %s/bin/pool.bin "$@"\n' \
            "$PREFIXO" "$PREFIXO" > "$PREFIXO/bin/pool"
    chmod 755 "$PREFIXO/bin/pool"
else
    install -m755 pool "$PREFIXO/bin/pool"
fi
ln -sf "$PREFIXO/bin/pool" "$PREFIXO/bin/psl"

echo "== servidor LSP"
# `lsp/` é o layout do bundle, `editor/vscode/` o do repositório. Procurar só
# pelo segundo fazia o instalador do bundle abortar aqui (com `set -e`), logo
# depois do binário: sem LSP, sem MIME e sem ícone.
if [ -f lsp/server.js ]; then
    LSP_ORIG=lsp
else
    LSP_ORIG=editor/vscode
fi
if [ -f "$LSP_ORIG/server.js" ]; then
    install -d "$PREFIXO/share/poolscript/lsp"
    # O `server.js` foi dividido em módulos (`require('./analise.js')`), e
    # copiar só ele deixava o LSP morrendo em `Cannot find module` em TODA
    # máquina instalada. Vai tudo o que é do servidor — de fora ficam só o
    # cliente do VS Code e o teste, que não são carregados por ele.
    for j in "$LSP_ORIG"/*.js; do
        case "$(basename "$j")" in
            extension.js|teste_servidor.js) continue ;;
        esac
        install -m644 "$j" "$PREFIXO/share/poolscript/lsp/"
    done
    for m in vscode-languageserver vscode-languageserver-protocol \
             vscode-languageserver-types vscode-jsonrpc \
             vscode-languageserver-textdocument semver; do
        [ -d "$LSP_ORIG/node_modules/$m" ] || continue
        mkdir -p "$PREFIXO/share/poolscript/lsp/node_modules"
        rm -rf "$PREFIXO/share/poolscript/lsp/node_modules/$m"
        cp -r "$LSP_ORIG/node_modules/$m" "$PREFIXO/share/poolscript/lsp/node_modules/"
    done
    printf '#!/bin/sh\n# Servidor LSP da PoolScript. Ver docs/lsp.md.\nif ! command -v node >/dev/null 2>&1; then\n  echo "poolscript-lsp precisa do node" >&2\n  exit 1\nfi\nexec node %s/share/poolscript/lsp/server.js "${@:---stdio}"\n' \
            "$PREFIXO" > "$PREFIXO/bin/poolscript-lsp"
    chmod 755 "$PREFIXO/bin/poolscript-lsp"
    # O servidor precisa do node em tempo de execução; quem instala numa
    # máquina nova não deveria descobrir isso só quando o editor não completa.
    if ! tem node; then
        echo "   node não encontrado — instalando"
        apt_instala nodejs || echo "   aviso: instale o node pra usar o LSP"
    fi
else
    echo "   (não veio servidor LSP neste pacote)"
fi

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
