#!/bin/bash
# A gramática do bundle TextMate do IDEA é CÓPIA da do vsix — a fonte única é
# `editor/vscode/syntaxes/jinga.tmLanguage.json`. Rode sempre que ela
# mudar; o `make intellij` já roda.
#
# O caminho apontava pra uma pasta do vsix que não existe mais. Copiar de um
# caminho morto não dá erro visível: o `cp` falha, a gramática velha fica, e o
# IDEA colore com regras de outra época.
set -e
cd "$(dirname "$0")"
FONTE=../../vscode/syntaxes/jinga.tmLanguage.json
[ -f "$FONTE" ] || { echo "atualiza.sh: nao achei $FONTE" >&2; exit 1; }
cp "$FONTE" Jinga.tmbundle/Syntaxes/
echo "gramatica sincronizada com o vsix"
