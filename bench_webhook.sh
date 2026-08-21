#!/usr/bin/env bash
# Prova que webhook (request de saída) dentro de um handler jinker NÃO trava o
# worker: N requests simultâneos, cada um faz um webhook a um alvo que dorme
# SLEEP s. Concorrente ~SLEEP; serial seria N*SLEEP.
# Uso:  bash bench_webhook.sh [N] [SLEEP]   (ex: bash bench_webhook.sh 5 0.5)
set -u
N="${1:-5}"; SLEEP="${2:-0.5}"
HERE="$(cd "$(dirname "$0")" && pwd)"; POOL="$HERE/pool"
porta(){ python3 -c 'import socket;s=socket.socket();s.bind(("127.0.0.1",0));print(s.getsockname()[1]);s.close()'; }
PT="$(porta)"; PP="$(porta)"
python3 - "$PT" "$SLEEP" <<'PY' &
import sys,time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
S=float(sys.argv[2])
class H(BaseHTTPRequestHandler):
    def do_GET(self):
        time.sleep(S); self.send_response(200); self.end_headers(); self.wfile.write(b"ok")
    def log_message(self,*a): pass
ThreadingHTTPServer(("127.0.0.1",int(sys.argv[1])),H).serve_forever()
PY
TPID=$!
cat > "/tmp/_webapp_$$.ps" <<PS
import request as web
from jinker import Jinker
app = Jinker()
@app.route("/proxy", methods=["GET"])
action proxy():
    r = web.get("http://127.0.0.1:$PT/x")
    return {"status": r.status}
app(debug=false, host="127.0.0.1", port=$PP)
PS
PYTHONPATH="$HERE/src" "$POOL" "/tmp/_webapp_$$.ps" >/dev/null 2>&1 &
SPID=$!
python3 - "$PP" "$N" "$SLEEP" <<'PY'
import sys,time,threading,urllib.request,socket
pp,N,S=int(sys.argv[1]),int(sys.argv[2]),float(sys.argv[3])
fim=time.time()+6
while time.time()<fim:
    try: socket.create_connection(("127.0.0.1",pp),timeout=0.2).close(); break
    except OSError: time.sleep(0.05)
time.sleep(0.3)
ok=[0]
def bate(i):
    try:
        with urllib.request.urlopen(f"http://127.0.0.1:{pp}/proxy",timeout=15) as r:
            if b'"status": 200' in r.read(): ok[0]+=1
    except Exception as e: print("  falha:",e)
t0=time.time()
ts=[threading.Thread(target=bate,args=(i,)) for i in range(N)]
[t.start() for t in ts]; [t.join() for t in ts]
tot=time.time()-t0
print(f"{N} webhooks simultâneos (alvo dorme {S}s cada):")
print(f"  ok={ok[0]}/{N}  total={tot:.2f}s   (serial seria ~{N*S:.1f}s, concorrente ~{S}s)")
print("  =>", "CONCORRENTE (offload OK)" if tot < (N*S)/2 else f"SERIALIZOU ({tot:.1f}s)")
PY
kill $TPID $SPID 2>/dev/null; rm -f "/tmp/_webapp_$$.ps"; wait 2>/dev/null
