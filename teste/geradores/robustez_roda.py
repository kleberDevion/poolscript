import json, subprocess, tempfile, os, sys, pathlib, collections
S="/tmp/claude-1000/-home-kleberdevion-poolscript-lang/6d969e59-69de-417b-8aa4-653e4a35fc16/scratchpad/robusto"
POOL="/home/kleberdevion/poolscript-lang/pool"
IMPORTS="import regex\nimport mail\nimport jinker\nimport qrcode\nimport guzer\nimport sockets\n"
casos=json.load(open(f"{S}/casos.json"))
ini=int(sys.argv[1]); n=int(sys.argv[2])
lote=casos[ini:ini+n]
L=[IMPORTS]
for i,(t,m,rot,expr) in enumerate(lote):
    L+=["try:", f'    _r = {expr}', f'    post("<<{i}>>PASSOU")',
        "catch(e):", f'    post("<<{i}>>ERRO ", e)']
src="\n".join(L)+"\n"
with tempfile.NamedTemporaryFile("w",suffix=".ps",delete=False,dir="/tmp",encoding="utf-8") as f:
    f.write(src); cam=f.name
env=dict(os.environ, GUZER_HEADLESS="1")
try:
    r=subprocess.run([POOL,cam],capture_output=True,timeout=180,stdin=subprocess.DEVNULL,env=env)
    rc=r.returncode; out=r.stdout.decode("utf-8","replace"); err=r.stderr.decode("utf-8","replace")
except subprocess.TimeoutExpired:
    rc,out,err=-99,"","TIMEOUT"
finally: os.unlink(cam)
vis={}
for ln in out.split("\n"):
    if not ln.startswith("<<"): continue
    j=ln.index(">>"); vis[int(ln[2:j])]=ln[j+2:]
pathlib.Path(f"{S}/res_{ini}.json").write_text(json.dumps(
  {"rc":rc,"err":err[:400],"vis":{str(k):v for k,v in vis.items()},"n":len(lote)},ensure_ascii=False))
print(f"{ini}: rc={rc} respondeu={len(vis)}/{len(lote)}")
