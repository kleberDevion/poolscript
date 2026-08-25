#!/usr/bin/env python3
"""Gera teste/casos_equivalencia.c a partir das familias de equivalencia.

   python3 teste/geradores/equivalencia.py        # monta as familias
   python3 teste/geradores/gera_c_equivalencia.py # vira caso de teste em C

Uma familia so vira teste se TODAS as formas dela concordarem agora. Familia
divergente e RELATADA e fica de fora: teste nao nasce verde escondendo defeito.
"""
import json, pathlib, subprocess, tempfile, os, sys
S = "/tmp/claude-1000/-home-kleberdevion-poolscript-lang/6d969e59-69de-417b-8aa4-653e4a35fc16/scratchpad/equiv"
POOL = "./pool"

def roda(src):
    with tempfile.NamedTemporaryFile("w", suffix=".ps", delete=False, dir="/tmp",
                                     encoding="utf-8") as t:
        t.write(src if src.endswith("\n") else src + "\n"); c = t.name
    try:
        r = subprocess.run([POOL, c], capture_output=True, timeout=30,
                           stdin=subprocess.DEVNULL)
        return (r.returncode, r.stdout.decode("utf-8", "replace"),
                r.stderr.decode("utf-8", "replace").split("\n")[0])
    finally:
        os.unlink(c)

def cstr(s):
    return "\n  ".join('"' + l.replace("\\", "\\\\").replace('"', '\\"')
                          .replace("\t", "\\t") + '\\n"' for l in s.split("\n"))
def esc(s):
    return s.rstrip("\n").replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")

fam = json.load(open(f"{S}/familias.json"))
out = ['/*',
 ' * EQUIVALENCIA INTERNA — a mesma computação escrita de várias formas.',
 ' *',
 ' * Onde o Python não serve de oráculo (Entity, closure, async, gerador,',
 ' * match, using, count each, herança, private, @static), a linguagem oferece',
 ' * caminhos REDUNDANTES pra dizer a mesma coisa: `for each` x `while` x',
 ' * gerador x recursão; bloco `:` x bloco `{}`; `match` x `if/elif`; closure x',
 ' * parâmetro x Entity; `using` x close() explícito.',
 ' *',
 ' * Se duas formas da mesma conta divergem, uma das duas está quebrada — e não',
 ' * interessa o que o Python faria. Foi assim que apareceu o bug de bloco `:`',
 ' * dentro de bloco `{}`: cada peça passava sozinha, a combinação é que quebrava.',
 ' *',
 ' * GERADO. Para adicionar uma família, edite teste/geradores/equivalencia.py',
 ' * e rode os dois geradores de novo — não edite este arquivo.',
 ' */',
 '#include "ps_teste.h"', '', 'const Caso CASOS_EQUIVALENCIA[] = {']
n = 0; fora = []
for f in fam:
    res = [roda(p) for p in f["progs"]]
    if len({(r[0], r[1]) for r in res}) > 1:
        fora.append(f["nome"]); continue
    rc, saida, err = res[0]
    for k, p in enumerate(f["progs"]):
        out.append('{ "equiv: %s [%d/%d]",' % (f["nome"].replace('"', "'"), k + 1, len(f["progs"])))
        out.append("  " + cstr(p if p.endswith("\n") else p + "\n") + ",")
        if rc == 0: out.append('  "%s", NULL, 0 },' % esc(saida))
        else:       out.append('  NULL, "%s", -1 },' % esc(err)[:80])
        n += 1
out += ['};', 'const int NC_EQUIVALENCIA = N_CASOS(CASOS_EQUIVALENCIA);']
pathlib.Path("teste/casos_equivalencia.c").write_text("\n".join(out) + "\n", encoding="utf-8")
print(f"casos gerados: {n}   familias fora (divergentes): {len(fora)}")
for x in fora: print("   DIVERGENTE:", x)
sys.exit(1 if fora else 0)
