"""Gera o produto (receptor x metodo x argumentos) pra str/list/dict/tup/bytes
e builtins. Cada caso vira uma expressao que roda NOS DOIS: PoolScript e
Python. O Python e o ORACULO — a doc da linguagem diz "igual ao Python" em
toda parte, entao divergencia e defeito ate prova em contrario."""
import json, itertools, pathlib
S = "/tmp/claude-1000/-home-kleberdevion-poolscript-lang/6d969e59-69de-417b-8aa4-653e4a35fc16/scratchpad/oraculo"

STR = ['""', '"a"', '"abc"', '"Ola Mundo"', '"  espaco  "', '"a,b,,c"',
       '"CAFÉ"', '"café"', '"Ω"', '"ß"', '"123"', '"12.5"', '"a\\tb"',
       '"linha1\\nlinha2"', '"aXbXc"', '"AbC dEf"', '"²³"', '"٣٤"']
SUB = ['""', '"a"', '"b"', '"X"', '","', '"abc"', '" "', '"é"']
NUM = ['0', '1', '2', '-1', '3', '10']

def m(nome, recv, args):
    return {"expr": f'{recv}.{nome}({", ".join(args)})'}

casos = []
# ── métodos de str sem argumento ────────────────────────────────────────────
SEM = ["upper","lower","title","capitalize","swapcase","casefold","strip",
       "lstrip","rstrip","splitlines","isalpha","isdigit","isnumeric",
       "isdecimal","isalnum","isspace","isupper","islower","isascii",
       "istitle","isprintable","isidentifier","encode","expandtabs"]
for s in STR:
    for n in SEM: casos.append(m(n, s, []))
# ── com 1 substring ─────────────────────────────────────────────────────────
UM = ["count","find","rfind","index","rindex","startswith","endswith",
      "partition","rpartition","removeprefix","removesuffix","split","rsplit"]
for s in STR[:10]:
    for n in UM:
        for a in SUB: casos.append(m(n, s, [a]))
# ── com faixa ───────────────────────────────────────────────────────────────
for s in STR[:8]:
    for n in ["find","rfind","count","index","rindex"]:
        for a in SUB[:4]:
            for i in NUM[:4]: casos.append(m(n, s, [a, i]))
# ── replace / just / zfill ──────────────────────────────────────────────────
for s in STR[:10]:
    for a, b in itertools.product(SUB[:5], SUB[:4]):
        casos.append(m("replace", s, [a, b]))
    for w in NUM: 
        for n in ["ljust","rjust","center","zfill"]: casos.append(m(n, s, [w]))
# ── fatias e índices ────────────────────────────────────────────────────────
for s in STR[:12]:
    for i in ["0","1","-1","2","-2"]: casos.append({"expr": f'{s}[{i}]'})
    for a, b in itertools.product(["","0","1","-1","2"], ["","1","-1","3","100"]):
        casos.append({"expr": f'{s}[{a}:{b}]'})
    casos.append({"expr": f'len({s})'})
# ── list ────────────────────────────────────────────────────────────────────
LST = ['[]', '[1]', '[1, 2, 3]', '[3, 1, 2]', '["a", "b"]', '[1, 1, 2]',
       '[[1], [2]]', '[True, False]', '[1, "a"]']
for l in LST:
    for n in ["copy","len"]: casos.append(m(n, l, []))
    for a in ["1","2","0",'"a"']:
        for n in ["count","index","contains"]: casos.append(m(n, l, [a]))
    for i in ["0","1","-1"]: casos.append({"expr": f'{l}[{i}]'})
    for a, b in itertools.product(["","0","1","-1"], ["","1","-1","2"]):
        casos.append({"expr": f'{l}[{a}:{b}]'})
    casos.append({"expr": f'len({l})'})
    casos.append({"expr": f'sorted({l})'})
    casos.append({"expr": f'reversed({l})'})
    casos.append({"expr": f'sum({l})'})
    casos.append({"expr": f'min({l})'})
    casos.append({"expr": f'max({l})'})
# ── dict ────────────────────────────────────────────────────────────────────
DCT = ['{}', '{"a": 1}', '{"a": 1, "b": 2}', '{"a": {"b": 1}}', '{"1": 2}']
for d in DCT:
    for n in ["keys","values","items","copy","len"]: casos.append(m(n, d, []))
    for a in ['"a"', '"z"', '"1"']:
        casos.append(m("get", d, [a]))
        casos.append(m("has", d, [a]))
    casos.append({"expr": f'len({d})'})
# ── tup / bytes ─────────────────────────────────────────────────────────────
for t in ['()', '(1,)', '(1, 2, 3)', '("a", "b")']:
    casos.append({"expr": f'len({t})'})
    for a in ["1","2",'"a"']:
        for n in ["count","index","contains"]: casos.append(m(n, t, [a]))
# ── builtins numéricos e de conversão ───────────────────────────────────────
VAL = ['0','1','-1','2','255','1000','3.14','-2.5','0.0','True','False',
       '"7"','"-3"','"abc"','""','[1,2]','{}']
for v in VAL:
    for f in ["str","int","flo","bool","type","abs","len","round"]:
        casos.append({"expr": f'{f}({v})'})
for v in ['0','1','65','255','1000','-1']:
    for f in ["hex","bin","oct","chr"]: casos.append({"expr": f'{f}({v})'})
for v in ['"a"','"Z"','"0"','"é"']: casos.append({"expr": f'ord({v})'})
# ── operadores ──────────────────────────────────────────────────────────────
OPS = ["+","-","*","/","%","==","!=","<",">","<=",">="]
OPV = ['0','1','2','-3','7','2.5','-0.5','True','False']
for a, b in itertools.product(OPV, OPV):
    for o in OPS: casos.append({"expr": f'{a} {o} {b}'})
for a, b in itertools.product(['""','"a"','"abc"'], ['""','"a"','"b"']):
    for o in ["+","==","!=","<",">"]: casos.append({"expr": f'{a} {o} {b}'})
for a in ['""','"ab"']:
    for n in NUM: casos.append({"expr": f'{a} * {n}'})
# dedup
vis, uniq = set(), []
for c in casos:
    if c["expr"] in vis: continue
    vis.add(c["expr"]); uniq.append(c)
pathlib.Path(f"{S}/casos.json").write_text(json.dumps(uniq, ensure_ascii=False))
print("casos gerados:", len(uniq))
