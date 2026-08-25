"""EQUIVALENCIA INTERNA: a mesma computacao escrita de N formas diferentes.

Nao precisa de oraculo externo. A linguagem oferece caminhos redundantes
(`for each` x `while`, bloco `:` x bloco `{}`, `match` x `if`, gerador x
lista, closure x parametro, metodo x action). Se duas formas da MESMA conta
dao resultados diferentes, uma das duas esta quebrada — e nao interessa qual
o Python faria.

Cada FAMILIA abaixo e uma lista de programas que TEM que imprimir a mesma
coisa. Divergencia dentro da familia = defeito.
"""
import json, pathlib, itertools
S="/tmp/claude-1000/-home-kleberdevion-poolscript-lang/6d969e59-69de-417b-8aa4-653e4a35fc16/scratchpad/equiv"

fam = []
def F(nome, *progs): fam.append({"nome": nome, "progs": list(progs)})

# ── laço: for each x while x gerador x recursão ─────────────────────────────
for n in [0, 1, 5, 17]:
    F(f"soma 0..{n}",
      f"s = 0\nfor each i in range({n}):\n    s = s + i\npost(s)",
      f"s = 0\nn = 0\nwhile n < {n}:\n    s = s + n\n    n = n + 1\npost(s)",
      f"s = 0\nfor each i in range({n}) {{\n    s = s + i\n}}\npost(s)",
      f"action g():\n    for each i in range({n}):\n        yield i\ns = 0\nfor each v in g():\n    s = s + v\npost(s)",
      f"action r(i, acc):\n    if i >= {n}:\n        return acc\n    return r(i + 1, acc + i)\npost(r(0, 0))",
      f"post(sum(range({n})))",
      f"l = []\nfor each i in range({n}):\n    l.append(i)\npost(sum(l))")

# ── bloco `:` x bloco `{}` em cada construção ───────────────────────────────
F("if",       'if true:\n    post("A")', 'if (true) {\n    post("A")\n}')
F("if/else",  'if false:\n    post("X")\nelse:\n    post("A")',
              'if (false) {\n    post("X")\n} else {\n    post("A")\n}')
F("while",    'n = 0\nwhile n < 3:\n    n = n + 1\npost(n)',
              'n = 0\nwhile (n < 3) {\n    n = n + 1\n}\npost(n)')
F("for each", 'for each i in [1,2]:\n    post(i)', 'for each i in [1,2] {\n    post(i)\n}')
F("action",   'action f():\n    return 7\npost(f())', 'action f() {\n    return 7\n}\npost(f())')
F("try",      'try:\n    raise B("x")\ncatch(e):\n    post("peguei")',
              'try {\n    raise B("x")\n} catch (e) {\n    post("peguei")\n}')
F("Entity",   'Entity P():\n    action m(self):\n        return 3\npost(P().m())',
              'Entity P() {\n    action m(self) {\n        return 3\n    }\n}\npost(P().m())',
              'Entity P():\n    action m(self) {\n        return 3\n    }\npost(P().m())',
              'Entity P() {\n    action m(self):\n        return 3\n}\npost(P().m())')

# ── match x if/elif x dict ──────────────────────────────────────────────────
for v, esp in [("1", "um"), ("2", "dois"), ("9", "outro")]:
    F(f"match {v}",
      f'x = {v}\nmatch x:\n    case 1:\n        post("um")\n    case 2:\n        post("dois")\n    case _:\n        post("outro")',
      f'x = {v}\nmatch x {{\n case 1 {{ post("um") }}\n case 2 {{ post("dois") }}\n case _ {{ post("outro") }}\n}}',
      f'x = {v}\nif x == 1:\n    post("um")\nelif x == 2:\n    post("dois")\nelse:\n    post("outro")')

# ── closure x parâmetro explícito x Entity ──────────────────────────────────
for k in [0, 3, -2]:
    F(f"captura {k}",
      f'action mult(k):\n    action f(x):\n        return x * k\n    return f\npost(mult({k})(7))',
      f'action f(x, k):\n    return x * k\npost(f(7, {k}))',
      f'Entity M():\n    action __init__(self, k):\n        self.k = k\n    action ap(self, x):\n        return x * self.k\npost(M({k}).ap(7))',
      f'k = {k}\naction f(x):\n    return x * k\npost(f(7))')

# ── contador: closure x Entity x lista ──────────────────────────────────────
F("contador 3x",
  'action faz():\n    n = 0\n    action inc():\n        n = n + 1\n        return n\n    return inc\nc = faz()\npost(c(), c(), c())',
  'Entity C():\n    action __init__(self):\n        self.n = 0\n    action inc(self):\n        self.n = self.n + 1\n        return self.n\nc = C()\npost(c.inc(), c.inc(), c.inc())',
  'l = [0]\naction inc():\n    l[0] = l[0] + 1\n    return l[0]\npost(inc(), inc(), inc())')

# ── gerador x lista x map ───────────────────────────────────────────────────
F("dobro de 0..4",
  'action g():\n    for each i in range(5):\n        yield i * 2\npost(list(g()))',
  'l = []\nfor each i in range(5):\n    l.append(i * 2)\npost(l)',
  'action d(x):\n    return x * 2\npost(map(list(range(5)), d))',
  'action d2(x):\n    return x * 2\npost(map(range(5), d2))')

# ── finally sempre roda ─────────────────────────────────────────────────────
# `catch` é obrigatório: não existe try/finally puro (ver docs 10.2).
F("finally com return",
  'action f():\n    try:\n        return "R"\n    catch(e):\n        post("C")\n    finally:\n        post("F")\npost(f())',
  'action f() {\n try {\n  return "R"\n } catch (e) {\n  post("C")\n } finally {\n  post("F")\n }\n}\npost(f())')
F("finally com break",
  'for each i in [1,2,3]:\n    try:\n        if i == 2:\n            break\n    catch(e):\n        post("C")\n    finally:\n        post("F" + str(i))\npost("fim")',
  'for each i in [1,2,3] {\n try {\n  if (i == 2) { break }\n } catch (e) {\n  post("C")\n } finally {\n  post("F" + str(i))\n }\n}\npost("fim")')
F("finally com continue",
  'for each i in [1,2]:\n    try:\n        continue\n    catch(e):\n        post("C")\n    finally:\n        post("F" + str(i))\npost("fim")',
  'for each i in [1,2] {\n try {\n  continue\n } catch (e) {\n  post("C")\n } finally {\n  post("F" + str(i))\n }\n}\npost("fim")')
F("finally roda com erro propagando",
  'action f():\n    try:\n        raise B("x")\n    catch(e):\n        post("C")\n    finally:\n        post("F")\n    return "R"\npost(f())',
  'action f() {\n try {\n  raise B("x")\n } catch (e) {\n  post("C")\n } finally {\n  post("F")\n }\n return "R"\n}\npost(f())')

# ── async x síncrono ────────────────────────────────────────────────────────
F("async dobro",
  'async action d(n):\n    return n * 2\npost(await d(4))',
  'action d(n):\n    return n * 2\npost(d(4))',
  'async action d(n):\n    return n * 2\npost(gather(d(4))[0])')
F("async lista",
  'async action d(n):\n    return n * 2\npost(await [d(1), d(2), d(3)])',
  'action d(n):\n    return n * 2\npost([d(1), d(2), d(3)])',
  'async action d(n):\n    return n * 2\npost(gather(d(1), d(2), d(3)))')

# ── declaração tipada x conversão explícita ─────────────────────────────────
for v, t in [('"7"', "int"), ("1", "flo"), ('64', "char")]:
    conv = {"int":"int", "flo":"flo", "char":"chr"}[t]
    F(f"{t} de {v}", f'{t} x = {v}\npost(x)', f'post({conv}({v}))')

# ── string: método x fatia x laço ───────────────────────────────────────────
F("inverter texto",
  'post("abcdef"[::-1])',
  's = ""\nfor each c in "abcdef":\n    s = c + s\npost(s)',
  'post("".join(reversed(list("abcdef"))))')
F("contar caractere",
  'post("banana".count("a"))',
  'n = 0\nfor each c in "banana":\n    if c == "a":\n        n = n + 1\npost(n)',
  'post(count each char in "aaa"[0:3])')


# ── herança: base() x campo direto x método do pai ──────────────────────────
for v in [0, 5, -3]:
    F(f"heranca {v}",
      f'Entity A():\n    action __init__(self, x):\n        self.x = x\n'
      f'Entity B(A):\n    action __init__(self, x):\n        base(x)\n'
      f'post(B({v}).x)',
      f'Entity A():\n    action __init__(self, x):\n        self.x = x\npost(A({v}).x)',
      f'Entity B():\n    action __init__(self, x):\n        self.x = x\n'
      f'    action pega(self):\n        return self.x\npost(B({v}).pega())')

# ── private: acesso de dentro x action solta ────────────────────────────────
F("private lido de dentro",
  'Entity P():\n    private s: int\n    action ve(self):\n        return self.s\npost(P(9).ve())',
  'Entity Q():\n    s: int\n    action ve(self):\n        return self.s\npost(Q(9).ve())',
  'action ve(s):\n    return s\npost(ve(9))')

# ── @static x action solta ──────────────────────────────────────────────────
F("static",
  'Entity K():\n    @static\n    action f(self, n):\n        return n + 1\npost(K.f(4))',
  'action f(n):\n    return n + 1\npost(f(4))')

# ── unpacking x índice ──────────────────────────────────────────────────────
F("desempacotar par",
  'a, b = 1, 2\npost(a, b)',
  't = (1, 2)\npost(t[0], t[1])',
  'l = [1, 2]\npost(l[0], l[1])')
F("desempacotar troca",
  'a = 1\nb = 2\na, b = b, a\npost(a, b)',
  'a = 1\nb = 2\nt = a\na = b\nb = t\npost(a, b)')

# ── enum x dict x constante ─────────────────────────────────────────────────
F("enum valor",
  'enum Cor { A, B }\npost(Cor.A, Cor.B)',
  'd = {"A": 0, "B": 1}\npost(d["A"], d["B"])')

# ── count each x laço x len(filter) ─────────────────────────────────────────
for lst, alvo in [("[1, 7, 7, 2, 7]", "7"), ("[1, 2, 3]", "9")]:
    F(f"contar {alvo} em {lst}",
      f'l = {lst}\npost(count int({alvo}) in l)',
      f'l = {lst}\nn = 0\nfor each x in l:\n    if x == {alvo}:\n        n = n + 1\npost(n)',
      f'l = {lst}\npost(l.count({alvo}))')

# ── using x try/finally ─────────────────────────────────────────────────────
# `catch` é obrigatório na linguagem (não existe try/finally puro), então a
# forma equivalente ao `using` usa close() explícito.
F("using fecha o arquivo",
  'using open("/tmp/ps_eq.txt", "w") as f:\n    f.write("x")\nusing open("/tmp/ps_eq.txt") as g:\n    post(g.read())',
  'f = open("/tmp/ps_eq2.txt", "w")\nf.write("x")\nf.close()\ng = open("/tmp/ps_eq2.txt")\npost(g.read())\ng.close()')

# ── f-string x concatenação x format ────────────────────────────────────────
for a2, b2 in [("1", '"x"'), ("-3", '"ção"')]:
    F(f"montar texto {a2} {b2}",
      f'a = {a2}\nb = {b2}\npost(f"{{a}}-{{b}}")',
      f'a = {a2}\nb = {b2}\npost(str(a) + "-" + b)',
      f'a = {a2}\nb = {b2}\npost("{{}}-{{}}".format(a, b))')

# ── closure aninhada 2 niveis x parametro ───────────────────────────────────
F("dois niveis",
  'action n1(a):\n    action n2(b):\n        action n3(c):\n            return a + b + c\n        return n3(3)\n    return n2(2)\npost(n1(1))',
  'action f(a, b, c):\n    return a + b + c\npost(f(1, 2, 3))')

# ── dict: acesso por [] x .get x .chave ─────────────────────────────────────
F("ler chave existente",
  'd = {"a": 1}\npost(d["a"])',
  'd = {"a": 1}\npost(d.get("a"))',
  'd = {"a": 1}\npost(d.a)')

# ── erro: raise x int action x try ──────────────────────────────────────────
F("erro capturado vira valor",
  'action f():\n    try:\n        raise B("x")\n    catch(e):\n        return 0\npost(f())',
  'int action g():\n    return null\npost(g())',
  'post(0)')

# ── fatia x laço com índice ─────────────────────────────────────────────────
for ini, fim in [(0, 3), (1, 4), (2, 2)]:
    F(f"fatia {ini}:{fim}",
      f'post("abcdef"[{ini}:{fim}])',
      f's = ""\nfor each i in range({ini}, {fim}):\n    s = s + "abcdef"[i]\npost(s)')

# ── ordenar: sort x sorted x min/max ────────────────────────────────────────
F("ordenar",
  'l = [3, 1, 2]\nl.sort()\npost(l)',
  'post(sorted([3, 1, 2]))',
  'post([min([3,1,2]), 2, max([3,1,2])])')

pathlib.Path(f"{S}/familias.json").write_text(json.dumps(fam, ensure_ascii=False))
print(f"familias: {len(fam)}   programas: {sum(len(f['progs']) for f in fam)}")
