#!/usr/bin/env bash
# Instala a PoolScript numa máquina: o binário (`pool`/`psl`), o servidor LSP,
# o tipo MIME do `.ps` e o ícone.
#
# Numa máquina onde não há NADA — um WSL Debian recém-criado, por exemplo —
# esta linha faz tudo sozinha, buscando o que faltar:
#
#   curl -fsSL https://raw.githubusercontent.com/kleberDevion/poolscript/main/instalar.sh | sudo bash
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

REPO="${REPO:-https://github.com/kleberDevion/poolscript}"
# `PACOTE` é a URL do bundle. Sai do release por padrão, mas aceita qualquer
# outra origem — o repositório pode estar privado, e aí o release não responde
# sem credencial. Servindo o `dist/` de outra máquina da rede, por exemplo:
#   PACOTE=http://192.168.0.10:8080/pool-portable.tar.gz sudo -E bash instalar.sh
PACOTE="${PACOTE:-$REPO/releases/latest/download/pool-portable.tar.gz}"

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
    INCOMPLETO=1
fi

echo "== tirando PoolScript antiga do PATH"
# Uma instalação velha em `~/.local/bin` vem ANTES de `/usr/local/bin` e
# SEQUESTRA o comando: o `pool` respondia o traceback de um pacote Python que
# não existe mais, com a instalação nova intacta logo atrás e invisível. O
# instalador dizia "pronto" e o comando estava quebrado.
#
# Rodando por `sudo`, o `$PATH` é o do root e o `~/.local/bin` de quem chamou
# não aparece nele — por isso a casa do `SUDO_USER` entra na busca à mão.
VARRER=$(echo "$PATH" | tr ':' '\n')
if [ -n "${SUDO_USER:-}" ]; then
    CASA=$(getent passwd "$SUDO_USER" | cut -d: -f6)
    if [ -n "$CASA" ]; then
        VARRER="$VARRER
$CASA/.local/bin
$CASA/bin"
    fi
    # O PATH do usuário tem pastas que o do root não tem. No WSL isso inclui as
    # do Windows (`/mnt/c/.../AppData/Roaming/npm`), onde um `pool` de instalação
    # antiga também responde. Sem ler o PATH dele, essas ficavam de fora.
    # Tem que ser um shell INTERATIVO: o `.bashrc` do Debian começa com um
    # `case $- in *i*) ;; *) return;; esac`, então tudo que veio depois — o
    # PATH do Windows incluído — não é lido por `su -c` comum.
    PATH_USUARIO=$(su - "$SUDO_USER" -s /bin/bash -c 'bash -ic "printf %s \$PATH"' 2>/dev/null || true)
    if [ -n "$PATH_USUARIO" ]; then
        VARRER="$VARRER
$(echo "$PATH_USUARIO" | tr ':' '\n')"
    fi
fi
REMOVIDOS=$(echo "$VARRER" | sort -u | while read -r d; do
    # Só caminho absoluto: o shell interativo lido acima pode ter escrito algo
    # além do PATH, e nada disso vira alvo de `rm`.
    case "$d" in /*) ;; *) continue ;; esac
    if [ "$d" = "$PREFIXO/bin" ]; then
        continue
    fi
    # Nas pastas do Windows o comando é um punhado de arquivos irmãos (o shim
    # sh, o `.cmd`, o `.ps1`, o `.exe`) — apagar só o sem extensão deixaria o
    # comando de pé no lado de lá.
    for c in pool psl poolscript-lsp; do
        for e in "" .cmd .ps1 .exe .bat; do
            if [ -e "$d/$c$e" ] || [ -L "$d/$c$e" ]; then
                rm -f "$d/$c$e" && echo "$d/$c$e"
            fi
        done
    done
done)
if [ -n "$REMOVIDOS" ]; then
    echo "$REMOVIDOS" | sed 's/^/   removido: /'
else
    echo "   nada no PATH"
fi

# Apagar o arquivo não basta: um `alias pool='...'` no `.bashrc` aponta pro
# caminho velho DIRETO, sem passar pelo PATH, e sobrevive ao `hash -r`. Depois
# da varredura acima o comando virava `No such file or directory` apontando pro
# que acabara de ser removido. Tira o alias dos arquivos de shell do usuário.
if [ -n "${SUDO_USER:-}" ] && [ -n "${CASA:-}" ]; then
    ALIASES=""
    for rc in .bashrc .bash_aliases .bash_profile .profile .zshrc; do
        [ -f "$CASA/$rc" ] || continue
        # Só linhas que DEFINEM o alias dos nossos três comandos; qualquer
        # outra menção a "pool" no arquivo fica onde está.
        if grep -qE "^[[:space:]]*alias[[:space:]]+(pool|psl|poolscript-lsp)=" "$CASA/$rc"; then
            cp -a "$CASA/$rc" "$CASA/$rc.antes-da-poolscript"
            grep -vE "^[[:space:]]*alias[[:space:]]+(pool|psl|poolscript-lsp)=" \
                 "$CASA/$rc.antes-da-poolscript" > "$CASA/$rc"
            ALIASES="$ALIASES $rc"
        fi
    done
    if [ -n "$ALIASES" ]; then
        echo "   alias removido de:$ALIASES (cópia em <arquivo>.antes-da-poolscript)"
        REMOVIDOS="$REMOVIDOS
alias"
    fi
fi

echo "== tipo MIME e ícone do .ps"
# Um pacote sem a pasta `dados/` fazia o `install` falhar e, com o `set -e`,
# derrubava o script AQUI — depois do binário já instalado. Ficava uma
# instalação pela metade que dizia "erro" sem dizer o que sobrou funcionando.
if [ -f dados/zz-poolscript.xml ]; then
    install -d "$DADOS/mime/packages" "$DADOS/icons/hicolor/scalable/mimetypes"
    install -m644 dados/zz-poolscript.xml "$DADOS/mime/packages/"
    install -m644 dados/icones/text-poolscript.svg \
            "$DADOS/icons/hicolor/scalable/mimetypes/"
    update-mime-database "$DADOS/mime" 2>/dev/null || true
    gtk-update-icon-cache -f -t "$DADOS/icons/hicolor" 2>/dev/null || true
else
    echo "   (não veio o tipo MIME neste pacote)"
    INCOMPLETO=1
fi

echo
echo "pronto:"
echo "  pool             $("$PREFIXO/bin/pool" --version 2>/dev/null)"
echo "  psl              mesmo binário"
if [ -x "$PREFIXO/bin/poolscript-lsp" ]; then
    echo "  poolscript-lsp   servidor LSP (aponte o editor pra ele)"
fi
if [ -f "$DADOS/mime/packages/zz-poolscript.xml" ]; then
    echo "  .ps              text/poolscript, com a logo"
fi
if [ -n "$REMOVIDOS" ]; then
    echo
    echo "havia PoolScript antiga na máquina e ela foi removida. O shell que já"
    echo "estava aberto ainda lembra dela — nele, rode:"
    echo "    unalias pool psl 2>/dev/null; hash -r"
    echo "(ou abra um terminal novo, que já nasce limpo)."
fi
if [ -n "${INCOMPLETO:-}" ]; then
    echo
    echo "o pacote instalado não trazia tudo: o que faltou está marcado acima." >&2
    echo "gere um completo com \`make bundle\` (ele leva lsp/ e dados/)." >&2
fi
