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
#   espalha_icone.sh <dir-de-dados> instalar|remover
set -e
DADOS="${1:-/usr/share}"
ACAO="${2:-instalar}"
AQUI=$(dirname "$(readlink -f "$0")")
SVG="$AQUI/icones/text-poolscript.svg"

for tema in "$DADOS"/icons/*/; do
    [ -f "$tema/index.theme" ] || continue
    # `scalable/mimetypes` é o diretório certo pra SVG; o hicolor já declara
    # essa seção, e nos temas que não declaram o arquivo fica inerte (não
    # atrapalha) — por isso a cópia extra em 48x48, que todo tema tem.
    for sub in scalable/mimetypes mimetypes/48 48x48/mimetypes; do
        alvo="$tema$sub"
        case "$ACAO" in
            instalar)
                # só cria onde o tema JÁ tem aquela estrutura, pra não inventar
                # diretório que o index.theme dele não declara
                [ -d "$alvo" ] || continue
                cp -f "$SVG" "$alvo/text-poolscript.svg" 2>/dev/null || true
                ;;
            remover)
                rm -f "$alvo/text-poolscript.svg" 2>/dev/null || true
                ;;
        esac
    done
    gtk-update-icon-cache -f -t "$tema" >/dev/null 2>&1 || true
done

# o hicolor recebe de qualquer jeito: é o fallback portátil
if [ "$ACAO" = instalar ]; then
    mkdir -p "$DADOS/icons/hicolor/scalable/mimetypes"
    cp -f "$SVG" "$DADOS/icons/hicolor/scalable/mimetypes/text-poolscript.svg"
else
    rm -f "$DADOS/icons/hicolor/scalable/mimetypes/text-poolscript.svg"
fi
gtk-update-icon-cache -f -t "$DADOS/icons/hicolor" >/dev/null 2>&1 || true
