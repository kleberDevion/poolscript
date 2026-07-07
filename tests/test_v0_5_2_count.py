"""Testes do operador `count` (PoolScript v0.5.2).

Inclui:
- Programa PoolScript completo com 14 markers (substring, dígitos, dict, infixo…)
- Testes unitários cobrindo as 3 formas sintáticas do count
- Teste de `count each` devolvendo total via `return;`
"""
from pathlib import Path

from poolscript.interpreter import run_source

CASES_DIR = Path(__file__).parent / "poolscript_cases"


def test_poolscript_count_program():
    case = CASES_DIR / "v0_5_2_count.ps"
    assert run_source(case.read_text(encoding="utf-8"), str(case)) == [
        "prefix-value-ok",
        "infix-ok",
        "truthy-ok",
        "prefix-type-ok",
        "substring-ok",
        "digits-in-int-ok",
        "digit-of-pair-ok",
        "dict-value-ok",
        "count-each-return-total-ok",
        "bool-strict-ok",
        "flo-type-ok",
        "not-count-ok",
        "count-and-count-ok",
        "zero-count-ok",
    ]


def test_count_prefix_value_returns_int():
    src = '''
list nums = [1, 7, 7, 2, 7]
post(count int(7) in nums)
'''
    assert run_source(src) == ["7"] or run_source(src) == ["3"]
    # nota: a chamada acima roda 2x; o que importa é que retorna 3
    assert run_source(src)[-1] == "3"


def test_count_infix_form():
    src = '''
list nums = [1, 7, 7, 2]
post(int(7) count in nums)
'''
    assert run_source(src) == ["2"]


def test_count_prefix_type_only():
    src = '''
list mix = [1, "a", 2, "b", 3]
post(count int in mix)
post(count str in mix)
'''
    assert run_source(src) == ["3", "2"]


def test_count_substring_in_string():
    src = '''
str s = "ana banana"
post(count str("ana") in s)
'''
    # 'ana' aparece em 'ana' (0) e 'banana' (4 e overlapping em 6) → 3
    assert run_source(src) == ["3"]


def test_count_digits_in_int():
    src = '''
int n = 17717
post(count int(7) in n)
'''
    assert run_source(src) == ["3"]


def test_count_in_dict_filters_by_value_type():
    src = '''
json d = {"a": 1, "b": "x", "c": 1, "d": 2}
post(count int(1) in d)
post(count str in d)
'''
    assert run_source(src) == ["2", "1"]


def test_count_zero_is_falsy():
    src = '''
list xs = [1, 2, 3]
if (count int(99) in xs) {
    post("found")
} else {
    post("none")
}
'''
    assert run_source(src) == ["none"]


def test_count_each_return_empty_yields_total():
    src = '''
list xs = [7, 7, 7, 8, 7]
action total() {
    count each int(7) in xs {
        return;
    }
}
post(total())
'''
    assert run_source(src) == ["4"]


def test_count_each_with_explicit_return_value_short_circuits():
    src = '''
list xs = [7, 7, 7]
action firstHit() {
    count each int(7) in xs {
        return _index
    }
}
post(firstHit())
'''
    assert run_source(src) == ["0"]


def test_count_each_executes_block_per_match():
    src = '''
list xs = [7, 8, 7]
count each int(7) in xs {
    post(_match)
}
'''
    assert run_source(src) == ["7", "7"]


def test_count_combined_with_logical_and():
    src = '''
list a = [1, 1]
list b = [3]
if (count int(1) in a and count int(3) in b) {
    post("both")
}
'''
    assert run_source(src) == ["both"]


def test_count_compared_with_equality():
    src = '''
list xs = [9, 9, 9, 1]
if (count int(9) in xs == 3) { post("ok") }
'''
    assert run_source(src) == ["ok"]
