"""
Suíte de testes v5.0.1 — cobre todas as features implementadas
desde v4.0.4 até v5.0.1.
"""
import sys, os, tempfile, traceback, inspect

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "src"))
from poolscript.interpreter import run_source, Interpreter, PoolFuture
from poolscript.parser import parse_source


def run(code):
    return run_source(code, "<test>")

def run_file(path):
    return run_source(open(path).read(), path)

def raises(code, keyword=""):
    try:
        run(code)
        return False
    except Exception as e:
        return keyword.lower() in str(e).lower() if keyword else True


# ── 1. reaction ──────────────────────────────────────────────────────────────
def test_reaction_basic():
    assert run("reaction f() { return 1 }\npost(f())") == ["1"]

def test_reaction_params():
    assert run("reaction soma(a,b) { return a+b }\npost(soma(3,4))") == ["7"]

def test_reaction_indented():
    assert run("reaction f():\n    return 42\npost(f())") == ["42"]

def test_reaction_in_entity():
    code = """
Entity C():
    action __init__(self):
        self.v = 10
    reaction dobro(self):
        return self.v * 2
c = C()
post(c.dobro())
"""
    assert run(code) == ["20"]


# ── 2. Brace na próxima linha (action/reaction) ──────────────────────────────
def test_brace_newline_action():
    assert run("action f()\n{\n    return 99\n}\npost(f())") == ["99"]

def test_brace_newline_reaction():
    assert run("reaction f()\n{\n    return 77\n}\npost(f())") == ["77"]

def test_brace_same_line_if():
    assert run("x=5\nif (x>0) {\n    post(\"ok\")\n}") == ["ok"]


# ── 3. int/bool return type ──────────────────────────────────────────────────
def test_int_reaction_success():
    assert run("int reaction f() { return 42 }\npost(f())") == ["42"]

def test_int_reaction_no_return():
    assert run("int reaction f() { x=1 }\npost(f())") == ["0"]

def test_int_reaction_error_500():
    assert run("int reaction f() { return 1/0 }\npost(f())") == ["500"]

def test_bool_reaction_true():
    assert run("bool reaction f() { return true }\npost(f())") == ["True"]

def test_bool_reaction_false():
    assert run("bool reaction f() { return false }\npost(f())") == ["False"]

def test_bool_reaction_error_false():
    assert run("bool reaction f() { return 1/0 }\npost(f())") == ["False"]

def test_async_int_reaction():
    assert run("async int reaction f() { return 21 }\npost(await f())") == ["21"]


# ── 4. async / await ─────────────────────────────────────────────────────────
def test_async_returns_future():
    code = "async action f() { return 1 }"
    prog = parse_source(code)
    interp = Interpreter(code)
    interp.run(prog)
    fn = interp.globals.get("f")
    future = interp._call(fn, [], {}, None)
    assert isinstance(future, PoolFuture)
    assert future.result() == 1

def test_await_resolves():
    assert run("async action f() { return 55 }\npost(await f())") == ["55"]

def test_await_list_gather():
    code = """
async action t(n) { return n * 2 }
fs = [t(1), t(2), t(3)]
rs = await fs
post(sorted(rs))
"""
    assert run(code) == ["[2, 4, 6]"]

def test_await_passthrough():
    assert run("post(await 42)") == ["42"]
    assert run('post(await "texto")') == ["texto"]

def test_async_in_entity():
    code = """
Entity C():
    action __init__(self, n):
        self.n = n
    async action fetch(self):
        return self.n * 10
c = C(5)
post(await c.fetch())
"""
    assert run(code) == ["50"]


# ── 5. DataEntity ─────────────────────────────────────────────────────────────
def test_dataentity_fields():
    code = """
from datasentity import dataentity, asdict
@dataentity
Entity P():
    nome: str
    idade: int
p = P(nome="Kleber", idade=17)
post(p.nome)
post(p.idade)
"""
    assert run(code) == ["Kleber", "17"]

def test_dataentity_asdict():
    code = """
from datasentity import dataentity, asdict
@dataentity
Entity P():
    x: int
    y: int
p = P(x=1, y=2)
d = asdict(p)
post(d["x"])
"""
    assert run(code) == ["1"]

def test_dataentity_astuple():
    code = """
from datasentity import dataentity, astuple
@dataentity
Entity P():
    a: str
    b: int
p = P(a="ok", b=9)
t = astuple(p)
post(t[0])
"""
    assert run(code) == ["ok"]

def test_dataentity_aslist():
    code = """
from datasentity import dataentity, aslist
@dataentity
Entity P():
    x: int
p = P(x=5)
l = aslist(p)
post(l[0])
"""
    assert run(code) == ["5"]

def test_dataentity_asjson():
    code = """
from datasentity import dataentity, asjson
import json as _j
@dataentity
Entity P():
    nome: str
p = P(nome="Ana")
d = _j.parse(asjson(p))
post(d["nome"])
"""
    assert run(code) == ["Ana"]

def test_dataentity_with_method():
    code = """
from datasentity import dataentity
@dataentity
Entity P():
    nome: str
    action upper(self):
        return self.nome.upper()
p = P(nome="kleber")
post(p.upper())
"""
    assert run(code) == ["KLEBER"]


# ── 6. addEnd / removeEnd / addStart / removeStart ───────────────────────────
def test_add_end():
    assert run("l=[1,2]\naddEnd(l,3)\npost(l)") == ["[1, 2, 3]"]

def test_remove_end():
    assert run("l=[1,2,3]\nv=removeEnd(l)\npost(v)\npost(l)") == ["3","[1, 2]"]

def test_add_start():
    assert run("l=[2,3]\naddStart(l,1)\npost(l)") == ["[1, 2, 3]"]

def test_remove_start():
    assert run("l=[1,2,3]\nv=removeStart(l)\npost(v)\npost(l)") == ["1","[2, 3]"]

def test_remove_end_empty():
    assert run("l=[]\npost(removeEnd(l))") == ["null"]

def test_remove_start_empty():
    assert run("l=[]\npost(removeStart(l))") == ["null"]


# ── 7. Strings coloridas ──────────────────────────────────────────────────────
def test_color_named():
    r = run('post(<red>"ola")')
    assert "ola" in r[0] and "\x1b[" in r[0]

def test_color_hex():
    r = run('post(<2196f3>"azul")')
    assert "\x1b[38;2;33;150;243m" in r[0]

def test_color_short_hex():
    r = run('post(<fff>"branco")')
    assert "branco" in r[0]

def test_all_color_names():
    from poolscript.lexer import NAMED_COLORS
    for name in NAMED_COLORS:
        r = run(f'post(<{name}>"x")')
        assert "x" in r[0], f"cor {name} falhou"


# ── 8. Novos builtins ─────────────────────────────────────────────────────────
def test_hex():      assert run("post(hex(255))") == ["0xff"]
def test_bin():      assert run("post(bin(10))") == ["0b1010"]
def test_oct():      assert run("post(oct(8))") == ["0o10"]
def test_ord():      assert run('post(ord("A"))') == ["65"]
def test_chr():      assert run("post(chr(65))") == ["A"]
def test_abs():      assert run("post(abs(-42))") == ["42"]
def test_round():    assert run("post(round(3.7))") == ["4"]
def test_sum():      assert run("post(sum([1,2,3,4,5]))") == ["15"]
def test_min():      assert run("post(min([5,1,3]))") == ["1"]
def test_max():      assert run("post(max([5,1,3]))") == ["5"]
def test_sorted():   assert run("post(sorted([3,1,2]))") == ["[1, 2, 3]"]
def test_reversed(): assert run("post(reversed([1,2,3]))") == ["[3, 2, 1]"]
def test_zip():
    r = run("post(zip([1,2],[3,4]))")
    assert "(1, 3)" in r[0]
def test_id():
    assert run("x='t'\npost(id(x))")[0].isdigit()


# ── 9. isdigit / isalpha em int e flo ────────────────────────────────────────
def test_isdigit_int():
    assert run("x=150\npost(x.isdigit())") == ["True"]

def test_isdigit_flo_false():
    assert run("x=150.5\npost(x.isdigit())") == ["False"]

def test_isdigit_str():
    assert run('post("150".isdigit())') == ["True"]

def test_isdigit_bool_raises():
    assert raises("post(true.isdigit())", "membro")

def test_isalpha_int_false():
    assert run("x=123\npost(x.isalpha())") == ["False"]


# ── 10. input() sempre str / conversão explícita ─────────────────────────────
def test_input_always_str(monkeypatch):
    monkeypatch.setattr("builtins.input", lambda p="": "123")
    assert run('x=input(">")\npost(x.type())') == ["str"]

def test_input_explicit_int(monkeypatch):
    monkeypatch.setattr("builtins.input", lambda p="": "42")
    assert run('int n=input(">")\npost(n.type())') == ["int"]

def test_input_explicit_flo(monkeypatch):
    monkeypatch.setattr("builtins.input", lambda p="": "3.14")
    assert run('flo f=input(">")\npost(f.type())') == ["flo"]

def test_input_explicit_int_invalid(monkeypatch):
    monkeypatch.setattr("builtins.input", lambda p="": "abc")
    assert raises('int n=input(">")', "int")


# ── 11. run_selfwith_ ignorado em imports ─────────────────────────────────────
def test_run_selfwith_ignored_on_import(tmp_path):
    (tmp_path/"lib.ps").write_text(
        'action f():\n    return "oi"\nrun_selfwith_("main"):\n    post("NAO")'
    )
    (tmp_path/"main.ps").write_text('from lib import f\npost(f())')
    assert run_file(str(tmp_path/"main.ps")) == ["oi"]

def test_run_selfwith_executes_direct(tmp_path):
    (tmp_path/"s.ps").write_text('run_selfwith_("main"):\n    post("sim")')
    assert run_file(str(tmp_path/"s.ps")) == ["sim"]


# ── 12. sys.stdout.writeln ───────────────────────────────────────────────────
def test_sys_writeln():
    import io
    from contextlib import redirect_stdout
    buf = io.StringIO()
    with redirect_stdout(buf):
        run('import sys\nsys.stdout.writeln("ola")')
    assert "ola\n" in buf.getvalue()

def test_sys_write_end_true():
    import io
    from contextlib import redirect_stdout
    buf = io.StringIO()
    with redirect_stdout(buf):
        run('import sys\nsys.stdout.write("x", true)')
    assert "x\n" in buf.getvalue()


# ── 13. AST com __slots__ ────────────────────────────────────────────────────
def test_ast_no_dict():
    from poolscript.parser import Literal
    n = Literal(line=1, col=1, value="x", kind="STR")
    assert not hasattr(n, "__dict__")

def test_scope_slots():
    # O que importa é a intenção: Scope não pode ter __dict__ por instância
    # (economia de memória). Interpretado isso vem de __slots__; compilado
    # (mypyc) a classe nativa nem expõe __slots__, mas a garantia é a mesma —
    # por isso o teste verifica a instância, não o atributo de classe.
    from poolscript.interpreter import Scope
    s = Scope()
    assert not hasattr(s, "__dict__")


# ── 14. count — não-regressão ────────────────────────────────────────────────
def test_count_list():
    assert run("post(count int(2) in [1,2,2,3])") == ["2"]

def test_count_string():
    assert run('post(count str("a") in "banana")') == ["3"]

def test_count_int_digits():
    assert run("post(count int(1) in 112211)") == ["4"]


# ── 15. match / case ─────────────────────────────────────────────────────────
def test_match_basic():
    code = "match 200:\n    case 200:\n        post(\"ok\")\n    case _:\n        post(\"nao\")"
    assert run(code) == ["ok"]

def test_match_guard():
    code = "x=30\nmatch x:\n    case p if p<50:\n        post(\"barato\")\n    case _:\n        post(\"caro\")"
    assert run(code) == ["barato"]

def test_match_or():
    code = 'dia="Sabado"\nmatch dia:\n    case "Sabado" | "Domingo":\n        post("fim")\n    case _:\n        post("semana")'
    assert run(code) == ["fim"]


# ── 16. yield / generator ────────────────────────────────────────────────────
def test_yield_basic():
    code = """
action contar(n):
    i = 0
    while i < n:
        yield i
        i += 1
resultado = []
for each v in contar(3) {
    addEnd(resultado, v)
}
post(resultado)
"""
    assert run(code) == ["[0, 1, 2]"]


# ── 17. Entity herança ───────────────────────────────────────────────────────
def test_entity_inheritance():
    code = """
Entity Animal():
    action __init__(self, nome):
        self.nome = nome
    action falar(self):
        return "..."
Entity Cachorro(Animal):
    action __init__(self, nome):
        base(nome)
    action falar(self):
        return "Au!"
c = Cachorro("Rex")
post(c.nome)
post(c.falar())
"""
    assert run(code) == ["Rex", "Au!"]

def test_entity_static():
    code = """
Entity U():
    @static
    action dobrar(n):
        return n * 2
post(U.dobrar(5))
"""
    assert run(code) == ["10"]

def test_json_module_dot_access():
    # regressão: `json` é TYPE_KEYWORD — antes, `json.stringify(...)` parseava
    # como TypeName("json").stringify e falhava com "string não tem método";
    # a lib json inteira era inacessível por ponto (só o alias JSON escapava).
    code = """
import json
s = json.stringify({"ok": true})
post(s)
d = json.parse(s)
post(d["ok"])
x = 1
post(type(x) == "int")
"""
    assert run(code) == ['{"ok": true}', "True", "True"]

def test_class_alias_of_entity():
    # `class` (e `Class`) declaram Entity igualzinho — inclusive herança
    # cruzada entre os dois estilos no mesmo arquivo.
    code = """
class Animal():
    action __init__(self, nome):
        self.nome = nome
    action falar(self):
        return "..."
Entity Gato(Animal):
    action falar(self):
        return f"{self.nome}: miau"
Class Cao(Animal):
    action falar(self):
        return f"{self.nome}: au"
g = Gato("Felix")
c = Cao("Rex")
post(g.falar())
post(c.falar())
"""
    assert run(code) == ["Felix: miau", "Rex: au"]


# ── 18. try / catch / finally ────────────────────────────────────────────────
def test_try_catch():
    assert run("try:\n    x=1/0\ncatch (e):\n    post(\"err\")") == ["err"]

def test_try_catch_finally():
    code = "try:\n    raise \"ops\"\ncatch (e):\n    post(\"catch\")\nfinally:\n    post(\"finally\")"
    assert run(code) == ["catch", "finally"]


# ── runner ───────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    tests = [(n, v) for n, v in globals().items() if n.startswith("test_") and callable(v)]
    passed, failed = 0, []

    for name, fn in tests:
        try:
            sig = inspect.signature(fn)
            params = list(sig.parameters)
            if "monkeypatch" in params:
                class MP:
                    def setattr(self, target, value):
                        t, a = target.rsplit(".", 1)
                        if t == "builtins":
                            import builtins; setattr(builtins, a, value)
                        else:
                            import importlib; m = importlib.import_module(t); setattr(m, a, value)
                fn(MP())
            elif "tmp_path" in params:
                import pathlib
                with tempfile.TemporaryDirectory() as d:
                    fn(pathlib.Path(d))
            else:
                fn()
            passed += 1
        except Exception:
            failed.append((name, traceback.format_exc()))

    total = len(tests)
    print(f"\n{'='*60}")
    print(f"  PoolScript v5.0.1 — Suite de Testes")
    print(f"{'='*60}")
    print(f"  ✅  {passed}/{total} passou")
    if failed:
        print(f"  ❌  {len(failed)} falhou:\n")
        for name, tb in failed:
            print(f"  FAIL: {name}")
            for line in tb.strip().split("\n")[-3:]:
                print(f"    {line}")
    print(f"{'='*60}\n")
