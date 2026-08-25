"""Gera `teste/casos_oraculo.c` a partir do produto (receptor x metodo x args)
de `oraculo_python.py`.

O Python e o ORACULO: a doc da linguagem diz "igual ao Python" em toda parte,
entao divergencia e defeito ate prova em contrario. O que este script faz:

  1. avalia cada expressao no Python  -> esperado
  2. avalia cada expressao no ./pool  -> obtido
  3. gera um caso em C pra CADA expressao, com o valor que a linguagem
     realmente produz, e MARCA no comentario as que divergem do Python

Nada e descartado. Divergencia nao vira caso omitido: vira caso com o
comportamento atual travado e o comentario dizendo do que ele difere — pra
mudanca futura aparecer no diff em vez de passar batido.

Rodar:  python3 teste/geradores/gera_c_oraculo.py
"""
import json, subprocess, sys, pathlib, importlib.util

RAIZ = pathlib.Path(__file__).resolve().parents[2]
TMP = pathlib.Path("/tmp/ps_oraculo")
TMP.mkdir(exist_ok=True)

# ── 1. as expressoes, do gerador ────────────────────────────────────────────
spec = importlib.util.spec_from_file_location(
    "oraculo_python", RAIZ / "teste/geradores/oraculo_python.py")
mod = importlib.util.module_from_spec(spec)
# o gerador escreve num S fixo de scratchpad; redireciona pro /tmp daqui
src = (RAIZ / "teste/geradores/oraculo_python.py").read_text()
src = src.replace(src.split("\n")[5], f'S = "{TMP}"')
exec(compile(src, "oraculo_python.py", "exec"), {"__name__": "oraculo"})
casos = json.loads((TMP / "casos.json").read_text())
print(f"expressoes: {len(casos)}")

# ── 2. o oraculo ────────────────────────────────────────────────────────────
def py_avalia(e):
    """Traduz a expressao pro dialeto Python e avalia. None = nao aplicavel."""
    p = e
    for a, b in (("true", "True"), ("false", "False"), ("Null", "None"),
                 (".contains(", ".__contains__("), (".has(", ".__contains__("),
                 (".value()", ".values()"), ("flo(", "float("),
                 ("post(", "print(")):
        p = p.replace(a, b)
    if ".encode()" in p or "reversed(" in p:
        return None            # repr difere por design (bytes/iterador)
    try:
        v = eval(p, {"__builtins__": __builtins__}, {})
    except Exception:
        return "<<ERRO>>"
    return py_repr(v)

def py_repr(v):
    if isinstance(v, bool):  return "True" if v else "False"
    if isinstance(v, str):   return v
    if isinstance(v, float):
        return repr(v)
    if isinstance(v, (list, tuple, dict, int)): return repr(v)
    if v is None: return "null"
    return repr(v)

# ── 3. o lado PoolScript, num programa so ───────────────────────────────────
# Sentinela por caso: expressao cujo valor tem \n (ex.: "linha1\nlinha2".upper())
# imprime mais de uma linha, entao dividir a saida por linha nao serve.
SENT = "<<|>>"
linhas = []
for c in casos:
    linhas.append("try:")
    linhas.append(f"    post({c['expr']})")
    linhas.append("catch (e):")
    linhas.append('    post("<<ERRO>>", e)')
    linhas.append(f'post("{SENT}")')
prog = TMP / "todos.ps"
prog.write_text("\n".join(linhas) + "\n")
r = subprocess.run(["./pool", str(prog)], cwd=RAIZ, capture_output=True, text=True)
ps_out = r.stdout.split(SENT + "\n")
if ps_out and ps_out[-1] == "": ps_out.pop()
ps_out = [s[:-1] if s.endswith("\n") else s for s in ps_out]
if len(ps_out) != len(casos):
    sys.exit(f"ERRO: {len(casos)} expressoes, {len(ps_out)} saidas.\n"
             f"stderr: {r.stderr[:400]}")

# ── 4. classifica ───────────────────────────────────────────────────────────
iguais, difere, sem_oraculo = [], [], []
for c, obtido in zip(casos, ps_out):
    esp = py_avalia(c["expr"])
    if esp is None:      sem_oraculo.append((c["expr"], obtido))
    elif esp == (obtido.split(" ")[0] if obtido.startswith("<<ERRO>>") else obtido):
        iguais.append((c["expr"], obtido))
    else:                difere.append((c["expr"], obtido, esp))

print(f"  bate com o Python : {len(iguais)}")
print(f"  diverge           : {len(difere)}")
print(f"  sem oraculo        : {len(sem_oraculo)}  (repr de bytes/iterador)")
if difere:
    print("\n  --- divergencias (PoolScript | Python) ---")
    for e, o, p in difere[:400]:
        print(f"    {e:52} {o!r:28} {p!r}")

# O runner compara com strlen, entao saida com NUL nao cabe num caso em C.
# Fica de fora — mas REGISTRADA, nunca sumida em silencio.
nul = [(e, o) for e, o in iguais + [(a, b) for a, b, _ in difere] + sem_oraculo
       if "\0" in o]
if nul:
    print(f"\n  fora do C ({len(nul)}): saida com NUL, que strlen nao expressa")
    for e, _ in nul: print(f"    {e}")
iguais      = [(e, o) for e, o in iguais if "\0" not in o]
difere      = [(e, o, p) for e, o, p in difere if "\0" not in o]
sem_oraculo = [(e, o) for e, o in sem_oraculo if "\0" not in o]

# ── 5. o arquivo C ──────────────────────────────────────────────────────────
def cstr(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') \
                  .replace("\n", "\\n").replace("\t", "\\t") + '"'

out = ['/*', ' * GERADO por teste/geradores/gera_c_oraculo.py — nao edite na mao.',
       ' *',
       ' * Cada caso e uma expressao rodada no ./pool, com o valor que a linguagem',
       ' * produz hoje. O Python foi o oraculo na geracao: caso marcado DIVERGE tem',
       ' * o comportamento do Python no comentario, pra a diferenca ficar visivel.',
       ' *', f' * expressoes: {len(casos)}  |  batem com o Python: {len(iguais)}'
       f'  |  divergem: {len(difere)}  |  sem oraculo: {len(sem_oraculo)}',
       f' * fora do C (saida com NUL): {len(nul)}', ' */',
       '#include "ps_teste.h"', '', 'const Caso CASOS_ORACULO[] = {']

import re as _re
def caso(nome, expr, saida):
    fonte = cstr("post(" + expr + ")" + chr(10))
    if saida.startswith("<<ERRO>> "):
        # o texto do erro traz " (linha N)" do programa em LOTE — inutil aqui
        msg = _re.sub(r" \(linha \d+\)$", "", saida[len("<<ERRO>> "):]).strip()
        return f'{{ "{nome}", {fonte}, NULL, {cstr(msg)}, -1 }},'
    return f'{{ "{nome}", {fonte}, {cstr(saida)}, NULL, 0 }},'

n = 0
for e, o in iguais:
    out.append(caso(f"oraculo #{n}", e, o))
    n += 1
out.append('')
out.append('/* ── divergem do Python: comportamento da PoolScript, travado ───────── */')
for e, o, p in difere:
    out.append(f'/* Python daria: {p!r} */')
    out.append(caso(f"oraculo #{n} (DIVERGE)", e, o))
    n += 1
out.append('')
out.append('/* ── sem oraculo (repr de bytes/iterador difere por design) ─────────── */')
for e, o in sem_oraculo:
    out.append(caso(f"oraculo #{n} (sem oraculo)", e, o))
    n += 1
out.append('};')
out.append('const int NC_ORACULO = N_CASOS(CASOS_ORACULO);')
(RAIZ / "teste/casos_oraculo.c").write_text("\n".join(out) + "\n")
print(f"\nteste/casos_oraculo.c: {n} casos")
