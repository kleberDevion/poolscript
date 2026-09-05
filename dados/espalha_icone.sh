#!/bin/sh
# Põe (ou tira) a logo do `.ps` em TODOS os temas de ícone instalados.
#
# Por que em todos, e não só no hicolor: o GTK resolve o ícone tema a tema, e
# prefere um ícone achado num tema PRÓXIMO a um nome mais específico achado num
# tema distante. Como `text-x-generic` existe em qualquer tema de desktop
# (Mint-Y, Adwaita, Papirus…) e o hicolor é o último da herança, a nossa logo
# NUNCA seria escolhida se morasse só lá. Tema só mostra ícone próprio de tipo
# que ele conhece — então a instalação faz o tema conhecer.
#
# SEM ROOT TAMBÉM FUNCIONA. `~/.local/share/icons/<tema>` vem ANTES de
# `/usr/share/icons/<tema>` na busca do GTK, e os dois são o MESMO tema (o
# `index.theme` de um vale pro outro) — então basta espelhar, no diretório do
# usuário, os temas e subdiretórios que existem no sistema. É o que o laço
# `espalha` faz: ele varre uma raiz de REFERÊNCIA pra saber quais temas
# existem e com que layout, e escreve em `$DADOS`. Com `$DADOS=/usr/share` os
# dois são o mesmo lugar e o comportamento é o de sempre.
#
#   espalha_icone.sh <dir-de-dados> instalar|remover
set -e
DADOS="${1:-/usr/share}"
ACAO="${2:-instalar}"
AQUI=$(dirname "$(readlink -f "$0")")
SVG="$AQUI/icones/text-poolscript.svg"

# `scalable/mimetypes` é o diretório certo pra SVG; o hicolor já declara essa
# seção, e nos temas que não declaram o arquivo fica inerte (não atrapalha) —
# por isso as cópias extras em 48x48, que todo tema tem.
SUBS="scalable/mimetypes mimetypes/48 48x48/mimetypes"

espalha() {
    [ -d "$1" ] || return 0
    for tema in "$1"/*/; do
        [ -f "$tema/index.theme" ] || continue
        nome=$(basename "$tema")
        for sub in $SUBS; do
            # só onde o tema JÁ tem aquela estrutura, pra não inventar
            # diretório que o `index.theme` dele não declara
            [ -d "$tema$sub" ] || continue
            alvo="$DADOS/icons/$nome/$sub"
            case "$ACAO" in
                instalar)
                    mkdir -p "$alvo" 2>/dev/null || continue
                    cp -f "$SVG" "$alvo/text-poolscript.svg" 2>/dev/null || true
                    ;;
                remover)
                    rm -f "$alvo/text-poolscript.svg" 2>/dev/null || true
                    ;;
            esac
        done
        gtk-update-icon-cache -f -t "$DADOS/icons/$nome" >/dev/null 2>&1 || true
    done
}

espalha "$DADOS/icons"
# Instalando fora do /usr/share (usuário, sem root): os temas de verdade estão
# no sistema, e é o layout DELES que precisa ser espelhado aqui.
if [ "$DADOS" != /usr/share ]; then
    espalha /usr/share/icons
    espalha /usr/local/share/icons
fi

# o hicolor recebe de qualquer jeito: é o fallback portátil
if [ "$ACAO" = instalar ]; then
    mkdir -p "$DADOS/icons/hicolor/scalable/mimetypes"
    cp -f "$SVG" "$DADOS/icons/hicolor/scalable/mimetypes/text-poolscript.svg"
else
    rm -f "$DADOS/icons/hicolor/scalable/mimetypes/text-poolscript.svg"
fi
gtk-update-icon-cache -f -t "$DADOS/icons/hicolor" >/dev/null 2>&1 || true
