#!/usr/bin/env bash
# Roda um passo do CI e, se ele falhar, publica as ÚLTIMAS linhas da saída
# como uma anotação de erro do GitHub (`::error::`).
#
# Por quê: o log de um job só se baixa com permissão de administrador do
# repositório, mas as anotações de um repositório público são legíveis por
# qualquer um pela API. É por elas que a falha de um build no macOS ou no
# Windows fica visível pra quem não tem o `gh` logado.
#
#     scripts/ci_anota.sh make pool testar
set -o pipefail
LOG=$(mktemp)
"$@" 2>&1 | tee "$LOG"
rc=${PIPESTATUS[0]}
if [ "$rc" -ne 0 ]; then
    # uma anotação só, com as linhas separadas por %0A (a quebra que o
    # GitHub entende dentro de um comando de workflow)
    # todas as falhas da suíte (cada uma com o contexto) e o fim da saída
    corpo=$( { echo "\$ $*";
               grep -E -A5 "FALHOU|MORREU|TRAVOU|JA FUNCIONA|error:|Error [0-9]" "$LOG" | head -n 400;
               echo "---- fim da saida ----"; tail -n 25 "$LOG"; } \
             | sed -e 's/%/%25/g' -e 's/\r//g' | awk '{printf "%s%%0A", $0}')
    echo "::error title=passo falhou::$corpo"
fi
rm -f "$LOG"
exit "$rc"
