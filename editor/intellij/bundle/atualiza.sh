#!/bin/bash
# A gramática do bundle TextMate do IDEA é CÓPIA da do vsix — a fonte única é
# `editor/vscode/syntaxes/poolscript.tmLanguage.json`. Rode sempre que ela
# mudar; o `make intellij` já roda.
#
# O caminho apontava pra `../psl-poolscript-vsix/`, pasta que não existe mais.
# Copiar de um caminho morto não dá erro visível: o `cp` falha, a gramática
# velha fica, e o IDEA colore com regras de outra época.
set -e
cd "$(dirname "$0")"
FONTE=../../vscode/syntaxes/poolscript.tmLanguage.json
[ -f "$FONTE" ] || { echo "atualiza.sh: nao achei $FONTE" >&2; exit 1; }
cp "$FONTE" PoolScript.tmbundle/Syntaxes/
echo "gramatica sincronizada com o vsix"
