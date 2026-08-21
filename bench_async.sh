#!/usr/bin/env bash
# Compara async PoolScript x Node: N tasks que esperam SLEEP segundos, todas
# concorrentes (gather / Promise.all). Mostra TEMPO de parede e PICO de RAM.
# Uso:  bash bench_async.sh [N] [SLEEP]
#   ex: bash bench_async.sh 200 0.05
set -u
N="${1:-200}"
SLEEP="${2:-0.05}"
HERE="$(cd "$(dirname "$0")" && pwd)"
POOL="$HERE/pool"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

# ---- PoolScript ----
{
  echo "async action t(n):"
  echo "    sleep($SLEEP)"
  echo "    return n"
  echo ""
  echo "fs = []"
  echo "for each i in range($N):"
  echo "    addEnd(fs, t(i))"
  echo "post(len(gather(fs)[0]))"
} > "$TMP/a.ps"

# ---- Node ----
cat > "$TMP/a.js" <<JS
const t=(n)=>new Promise(r=>setTimeout(()=>r(n), ${SLEEP}*1000));
(async()=>{const a=[];for(let i=0;i<${N};i++)a.push(t(i));console.log((await Promise.all(a)).length);})();
JS

medir () {  # $1=rótulo  $2...=comando
  local rot="$1"; shift
  local mf; mf="$(mktemp)"
  local saida wall rss
  saida="$(/usr/bin/time -v -o "$mf" "$@" 2>/dev/null </dev/null)"   # stdout limpo
  wall="$(awk -F'):' '/wall clock/{gsub(/ /,"",$2);print $2}' "$mf")"
  rss="$(awk '/Maximum resident/{printf "%.1f", $NF/1024}' "$mf")"
  rm -f "$mf"
  printf "  %-12s saida=%-6s tempo=%-8s RAM=%s MB\n" "$rot" "$saida" "$wall" "$rss"
}

echo "== async $N tasks, sleep ${SLEEP}s cada, todas concorrentes =="
echo "   (serial seria ~$(awk "BEGIN{printf \"%.1f\", $N*$SLEEP}")s; concorrente ~${SLEEP}s)"
medir "PoolScript" "$POOL" "$TMP/a.ps"
command -v node >/dev/null && medir "Node" node "$TMP/a.js" || echo "  Node: não instalado"
