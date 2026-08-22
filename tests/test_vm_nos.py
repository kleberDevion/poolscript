"""Suíte do VM em C (`pool`) — cobre CADA nó/construto da linguagem rodando
DIRETO no VM, com saída esperada FIXA (não é comparação com o interpretador).

Por quê não comparar com o interp: o VM em C tem os problemas DELE — malloc/free
manual, GC próprio, sem GIL — que o interpretador (Python, coletado, com GIL) não
tem. Um bug só-do-VM (ex.: o segfault ao importar módulo com `model`) não aparece
testando só o interpretador. Aqui a referência é a SEMÂNTICA da linguagem, travada
em saída literal.

Importante: se o binário `pool` não existir, estes testes FALHAM (não pulam) — um
skip silencioso daria falso verde, que é justamente o que mascarava bug do VM.
Compile antes: `./rebuild_vm.sh`.
"""
import subprocess
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"


def _vm(tmp_path, src):
    # SEM skip: se o pool não existe, FALHA claro — skip silencioso dá falso
    # verde e mascara bug do VM (foi como o segfault se escondeu).
    assert POOL.exists(), (
        "binário 'pool' não compilado — rode ./rebuild_vm.sh. Esta é a suíte do "
        "VM em C; sem o binário não há VM pra testar (e não se pula isso).")
    ps = tmp_path / "t.ps"
    ps.write_text(src, encoding="utf-8")
    return subprocess.run([str(POOL), str(ps)], capture_output=True, text=True,
                          timeout=30)


# (nó/construto, fonte, saída ESPERADA no VM — semântica da linguagem)
CASOS = [
    ("literais", 'post(1, 2.5, "x", true, false, null)', "1 2.5 x True False null\n"),
    ("bigint", "post(123456789012345678901234567890 + 1)", "123456789012345678901234567891\n"),
    ("fstring", 'x = 5\ny = "oi"\npost(f"{y} {x} {x + 1}")', "oi 5 6\n"),
    ("list_dict_tup", 'post([1, 2, 3])\npost({"a": 1, "b": 2})\npost((1, 2, 3))',
     "[1, 2, 3]\n{'a': 1, 'b': 2}\n(1, 2, 3)\n"),
    ("index", 'l = [10, 20, 30]\nd = {"k": 9}\npost(l[0], l[-1], d["k"])', "10 30 9\n"),
    ("slice", 'post([1, 2, 3, 4, 5][1:4])\npost("abcdef"[2:])', "[2, 3, 4]\ncdef\n"),
    ("index_assign", "l = [1, 2, 3]\nl[1] = 99\npost(l)", "[1, 99, 3]\n"),
    ("arit", "post(7 + 3, 7 - 3, 7 * 3, 7 / 2, 7 % 3)", "10 4 21 3.5 1\n"),
    ("compara", "post(1 < 2, 2 <= 2, 3 > 4, 5 == 5, 5 != 6)", "True True False True True\n"),
    ("logico", "post(true and false, true or false, not true)", "False True False\n"),
    ("is_in", 'post(3 in [1, 2, 3], "a" in {"a": 1}, 5 is int)', "True True True\n"),
    ("unario", "x = 5\npost(-x, +x, ~x, not false)", "-5 5 -6 True\n"),
    ("postfix", "int x = 1\nx++\nx++\npost(x)\nx--\npost(x)", "3\n2\n"),
    ("ternario", 'x = 4\npost("par" if x % 2 == 0 else "impar")', "par\n"),
    ("decl_tipada", 'int a = 5\nstr b = "oi"\nc = 3.14\npost(a, b, c)', "5 oi 3.14\n"),
    ("unpack", "a, b, c = 1, 2, 3\npost(a, b, c)\nx, y = [10, 20]\npost(x, y)", "1 2 3\n10 20\n"),
    ("if_elif_else", 'n = 5\nif (n > 10) { post("g") } elif (n > 3) { post("m") } else { post("p") }', "m\n"),
    ("while_break_cont",
     "int i = 0\nwhile (i < 6) {\n    i++\n    if (i == 2) { continue }\n    if (i == 5) { break }\n    post(i)\n}",
     "1\n3\n4\n"),
    ("for_each", "for each x in [1, 2, 3] { post(x) }\nfor each i in range(3) { post(i) }", "1\n2\n3\n0\n1\n2\n"),
    ("try_catch_finally", 'try {\n    raise "boom"\n} catch (e) {\n    post("peguei")\n} finally {\n    post("fim")\n}',
     "peguei\nfim\n"),
    ("using", 'class R() {\n    public reaction close(self) { return 0 }\n}\nusing R() as r { post("usando") }', "usando\n"),
    ("match", 'x = 2\nmatch x {\n    case 1 { post("um") }\n    case 2 { post("dois") }\n    case _ { post("?") }\n}', "dois\n"),
    ("match_list_guarda",
     'p = [3, 3]\nmatch p {\n    case [0, 0] { post("orig") }\n    case [a, b] if a == b { post("diag") }\n    case _ { post("o") }\n}',
     "diag\n"),
    ("count_each", 'count each int in [10, "a", 20, 30] { post(_match, _index, _count) }', "10 0 3\n20 2 3\n30 3 3\n"),
    ("action_defaults_named",
     "reaction f(a, b=10, c=20) { return (a, b, c) }\npost(f(1))\npost(f(1, c=99))\npost(f(1, 2, 3))",
     "(1, 10, 20)\n(1, 10, 99)\n(1, 2, 3)\n"),
    ("recursao", "reaction fib(n) {\n    if (n < 2) { return n }\n    return fib(n - 1) + fib(n - 2)\n}\npost(fib(10))", "55\n"),
    ("lambda_action_expr",
     "f = action(x) { return x * 2 }\npost(f(21))\npost(map([1, 2, 3], action(n) { return n + 1 }))", "42\n[2, 3, 4]\n"),
    ("gerador_yield", "reaction g() {\n    yield 1\n    yield 2\n    yield 3\n}\nfor each v in g() { post(v) }", "1\n2\n3\n"),
    ("global_stmt", "x = 1\nreaction f() {\n    global x\n    x = 99\n}\nf()\npost(x)", "99\n"),
    ("async_await_gather",
     "async reaction d(n) { return n * 2 }\nreaction main() {\n    post(await d(21))\n    post(gather(d(1), d(2), d(3)))\n}\nmain()",
     "42\n[2, 4, 6]\n"),
    ("entity_self_field",
     "class C() {\n    public reaction __init__(self, x) { self.x = x }\n    public reaction get(self) { return self.x }\n}\npost(C(7).get())",
     "7\n"),
    ("entity_static", "class C() {\n    @static\n    public reaction soma(self, a, b) { return a + b }\n}\npost(C.soma(2, 3))", "5\n"),
    ("entity_heranca_base",
     "class A() {\n    public reaction __init__(self, x) { self.x = x }\n    public reaction nome(self) { return \"A\" }\n}\nclass B(A) {\n    public reaction __init__(self, x) { base(x) }\n    public reaction get(self) { return self.x }\n}\nb = B(7)\npost(b.get(), b.nome())",
     "7 A\n"),
    ("entity_private",
     "class C() {\n    private reaction seg(self) { return 42 }\n    public reaction pub(self) { return self.seg() }\n}\npost(C().pub())",
     "42\n"),
    ("model", 'model U() {\n    nome: str\n    idade: int\n}\npost({"nome": "a", "idade": 5} == U)\npost({"nome": "a"} == U)',
     "True\nFalse\n"),
    ("model_length", 'model D() {\n    cpf: str(length=3)\n}\npost({"cpf": "123"} == D)\npost({"cpf": "1234"} == D)', "True\nFalse\n"),
    ("enum", "enum Cor {\n    RED\n    GREEN\n    BLUE\n}\npost(Cor.RED, Cor.GREEN, Cor.BLUE)", "0 1 2\n"),
    ("enum_explicito", "enum S {\n    OK = 200,\n    NF = 404\n}\npost(S.OK, S.NF)", "200 404\n"),
    ("builtins", 'post(len([1, 2, 3]), len("abc"), str(5), int("7"), flo("2.5"), bool(1))', "3 3 5 7 2.5 True\n"),
    ("map_filter",
     "post(map([1, 2, 3], action(x) { return x * 2 }))\npost(filter([1, 2, 3, 4], action(x) { return x % 2 == 0 }))",
     "[2, 4, 6]\n[2, 4]\n"),
    ("list_ops", "l = [3, 1, 2]\naddEnd(l, 4)\naddStart(l, 0)\npost(l)\npost(sorted([3, 1, 2]))", "[0, 3, 1, 2, 4]\n[1, 2, 3]\n"),
    ("string_metodos", 's = "Hello World"\npost(s.upper(), s.lower(), s.replace("o", "0"))\npost(s.split(" "))',
     "HELLO WORLD hello world Hell0 W0rld\n['Hello', 'World']\n"),
    ("dict_metodos", 'd = {"a": 1, "b": 2}\npost(d.keys(), d.values(), d.has("a"), d.get("z"))', "['a', 'b'] [1, 2] True null\n"),
]


def test_pool_existe():
    # Falha explícita (não skip) — skip silencioso mascarava bug do VM.
    assert POOL.exists(), (
        "binário 'pool' não compilado — rode ./rebuild_vm.sh antes. "
        "Estes testes exercitam o VM em C; sem o binário não há o que testar.")


@pytest.mark.parametrize("nome,src,esperado", CASOS, ids=[c[0] for c in CASOS])
def test_no_no_vm(tmp_path, nome, src, esperado):
    r = _vm(tmp_path, src)
    assert r.returncode == 0, (nome, "VM crashou/erro:", r.stdout + r.stderr)
    assert r.stdout == esperado, (nome, "VM:", repr(r.stdout), "esperado:", repr(esperado))
