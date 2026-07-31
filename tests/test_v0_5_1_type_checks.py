from pathlib import Path

from poolscript.interpreter import Interpreter, run_source
from poolscript.parser import parse_source


CASES_DIR = Path(__file__).parent / "poolscript_cases"


def test_poolscript_type_validation_program():
    case_path = CASES_DIR / "v0_5_1_type_checks.ps"
    source = case_path.read_text(encoding="utf-8")
    assert run_source(source, str(case_path)) == [
        "dyn-int-ok",
        "typed-int-ok",
        "typed-flo-ok",
        "str-not-none-ok",
        "is-not-none-ok",
        "collections-ok",
        "multi-and-ok",
        "truthy-name-ok",
        "equality-ok",
        "magnitude-ok",
        "membership-ok",
        "casts-ok",
    ]


def test_poolscript_input_always_returns_str(monkeypatch):
    """input() SEMPRE retorna str — igual ao Python.
    Nenhuma adivinhação de tipo acontece sem declaração explícita."""
    values = iter(["123", "1.5", "True", "texto"])
    monkeypatch.setattr("builtins.input", lambda prompt="": next(values))
    source = '''
numero = input("DIGITAR: ")
real = input("DIGITAR: ")
flag = input("DIGITAR: ")
texto = input("DIGITAR: ")
if (numero is str and real is str and flag is str and texto is str) {
    post("input-always-str-ok")
}
'''
    assert run_source(source) == ["input-always-str-ok"]


def test_poolscript_input_explicit_int_type_converts(monkeypatch):
    """int x = input() converte explicitamente — igual int(input())."""
    monkeypatch.setattr("builtins.input", lambda prompt="": "123")
    source = '''
int numero = input("DIGITAR: ")
if (numero is int and numero == 123) {
    post("explicit-int-ok")
}
'''
    assert run_source(source) == ["explicit-int-ok"]


def test_poolscript_input_explicit_flo_type_converts(monkeypatch):
    """flo x = input() converte explicitamente — igual float(input())."""
    monkeypatch.setattr("builtins.input", lambda prompt="": "1.5")
    source = '''
flo real = input("DIGITAR: ")
if (real is flo and real == 1.5) {
    post("explicit-flo-ok")
}
'''
    assert run_source(source) == ["explicit-flo-ok"]


def test_poolscript_input_explicit_int_invalid_raises(monkeypatch):
    """int x = input() com texto não numérico levanta erro claro."""
    monkeypatch.setattr("builtins.input", lambda prompt="": "abc")
    source = '''
int numero = input("DIGITAR: ")
'''
    try:
        run_source(source)
        assert False, "deveria ter levantado erro de conversão"
    except Exception as e:
        assert "abc" in str(e)
        assert "int" in str(e)


def test_poolscript_dynamic_and_primitive_negative_paths():
    source = '''
x = "123"
int y = 123
if (x not is int and y is int and None is None) {
    post("negative-path-ok")
}
'''
    assert run_source(source) == ["negative-path-ok"]


def test_poolscript_parser_accepts_all_comparison_forms():
    source = '''
a = 10
b = 20
items = [10, 30]
if (a < b and b > a and a <= 10 and b >= 20 and a in items and b not in items and a is int and b is not None) {
    post("all-comparisons-ok")
}
'''
    program = parse_source(source)
    interp = Interpreter(source=source)
    interp.run(program)
    assert interp.output == ["all-comparisons-ok"]


def test_poolscript_is_operator_isinstance_check_against_stdlib_class(tmp_path):
    """`valor is AlgumaClasse` deve funcionar como isinstance() pra qualquer
    classe exportada da stdlib (ex: PoolFile), não só pros tipos primitivos."""
    arquivo = tmp_path / "relatorio.pdf"
    arquivo.write_bytes(b"%PDF-1.4 fake")
    source = f'''
from os import loadFile

texto = "so um texto"
arq = loadFile(r"{arquivo}")

if (arq is PoolFile) {{ post("arquivo-e-poolfile") }}
if (texto not is PoolFile) {{ post("texto-nao-e-poolfile") }}
'''
    assert run_source(source) == ["arquivo-e-poolfile", "texto-nao-e-poolfile"]
