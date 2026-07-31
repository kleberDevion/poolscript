"""Consistência de API: contains/has/len em dict/str/list, e alias json/get_json."""
import io
from contextlib import redirect_stdout

from poolscript import run_source
from poolscript.stdlib.jinker_lib import JinkerRequest, RequestProxy


def _run(src: str) -> list[str]:
    out = io.StringIO()
    with redirect_stdout(out):
        run_source(src, "<test>")
    return out.getvalue().strip().splitlines()


# ── contains / has: mesmo comportamento em dict, str, list e tuple ───────────
def test_contains_and_has_consistent_across_dict_str_list_tuple():
    lines = _run('''
d = {"a": 1, "b": 2}
s = "poolscript"
l = [1, 2, 3]
t = (1, 2, 3)
post(d.contains("a"))
post(s.contains("pool"))
post(l.contains(2))
post(t.contains(2))
post(d.has("x"))
post(s.has("java"))
post(l.has(9))
post(t.has(9))
''')
    assert lines == ["True", "True", "True", "True", "False", "False", "False", "False"]


# ── .len() como MÉTODO (com parênteses) uniforme em dict/str/list/tuple ───────
def test_len_method_uniform_with_parens():
    lines = _run('''
d = {"a": 1, "b": 2}
s = "abc"
l = [1, 2, 3, 4]
t = (1, 2, 3)
post(d.len())
post(s.len())
post(l.len())
post(t.len())
''')
    assert lines == ["2", "3", "4", "3"]


# ── list mantém os métodos nativos (não quebrou nada ao adicionar contains) ───
def test_list_native_methods_still_work():
    lines = _run('''
l = [3, 1, 2]
post(l.count(1))
post(l.index(2))
''')
    assert lines == ["1", "2"]


# ── os dois jeitos convivem: forma Python (in/len) E método (atalho PoolScript) ─
def test_python_form_and_method_form_coexist():
    lines = _run('''
l = [1, 2, 3]
d = {"a": 1}
// forma Python
post(2 in l)
post("a" in d)
post(len(l))
// forma método (atalho)
post(l.contains(2))
post(d.has("a"))
post(l.len())
''')
    assert lines == ["True", "True", "3", "True", "True", "3"]


# ── request: json() é alias de get_json() (o nome intuitivo passa a funcionar) ─
def test_request_proxy_json_is_alias_of_get_json():
    proxy = RequestProxy()
    proxy._set(JinkerRequest("POST", "/x", {}, b'{"a": 1, "b": 2}', {}))
    assert proxy.json() == proxy.get_json() == {"a": 1, "b": 2}
