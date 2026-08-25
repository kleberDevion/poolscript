"""`finally` que não rodava e chave mutável de dict que a VM aceitava.

Achados de 2026-08-25 (notas/caca-bugs-2026-08-25.md):

  - o bloco `finally` era emitido INLINE nas duas saídas do try (fim normal e
    "nenhum catch casou"). `return`, `break`, `continue` e um `raise` dentro do
    catch saltavam POR FORA — o finally simplesmente não rodava, e é justamente
    nele que se fecha arquivo/conexão;
  - a VM aceitava LISTA e DICT como chave de dicionário (valores mutáveis):
    dava pra mutar a lista depois e deixar a chave pendurada;
  - `json.parse` de inteiro grande saturava em INT64_MAX — valor errado, calado.

Comparado com o interpretador em cada caso.
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."


def _run(cmd, tmp_path, fonte):
    ps = tmp_path / "t.ps"
    ps.write_text(fonte, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run(cmd + [str(ps)], capture_output=True, text=True,
                          env=env, timeout=30)


def ambos(tmp_path, fonte):
    v = _run([str(POOL)], tmp_path, fonte)
    i = _run([sys.executable, "-m", "poolscript"], tmp_path, fonte)
    assert v.returncode == i.returncode, (v.stderr, i.stderr)
    assert v.stdout == i.stdout, f"VM {v.stdout!r} != interp {i.stdout!r}"
    return v.stdout.strip().split("\n")


def test_finally_com_return(tmp_path):
    fonte = ('action f() {\n'
             '    try {\n        return "do try"\n'
             '    } catch (e) {\n        return "do catch"\n'
             '    } finally {\n        post("finally")\n    }\n}\n'
             'post(f())\n')
    assert ambos(tmp_path, fonte) == ["finally", "do try"]


def test_finally_com_break(tmp_path):
    fonte = ('for each i in [1, 2, 3] {\n'
             '    try {\n        if (i == 2) { break }\n        post("corpo", i)\n'
             '    } catch (e) {\n        post("c")\n'
             '    } finally {\n        post("finally", i)\n    }\n}\n'
             'post("fim")\n')
    assert ambos(tmp_path, fonte) == ["corpo 1", "finally 1", "finally 2", "fim"]


def test_finally_com_continue(tmp_path):
    fonte = ('for each i in [1, 2] {\n'
             '    try {\n        continue\n'
             '    } catch (e) {\n        post("c")\n'
             '    } finally {\n        post("finally", i)\n    }\n}\n')
    assert ambos(tmp_path, fonte) == ["finally 1", "finally 2"]


def test_finally_com_raise_dentro_do_catch(tmp_path):
    fonte = ('try {\n'
             '    try {\n        raise ValueError("x")\n'
             '    } catch (e) {\n        raise KeyError("re-raise")\n'
             '    } finally {\n        post("finally")\n    }\n'
             '} catch (e) {\n    post("fora")\n}\n')
    assert ambos(tmp_path, fonte) == ["finally", "fora"]


def test_finally_no_caminho_normal_e_no_erro(tmp_path):
    """Os dois caminhos que já funcionavam não podem ter regredido."""
    fonte = ('try {\n    post("ok")\n} catch (e) {\n    post("nao")\n'
             '} finally {\n    post("f1")\n}\n'
             'try {\n    raise ValueError("x")\n} catch (e) {\n    post("peguei")\n'
             '} finally {\n    post("f2")\n}\n')
    assert ambos(tmp_path, fonte) == ["ok", "f1", "peguei", "f2"]


@pytest.mark.parametrize("chave", ['[1,2]', '{ "x": 1 }'])
def test_chave_mutavel_recusada(tmp_path, chave):
    r = _run([str(POOL)], tmp_path, f'd = {{}}\nd[{chave}] = "a"\n')
    assert r.returncode != 0, "a VM aceitou chave mutável"
    assert "chave de dict" in r.stderr, r.stderr


def test_chave_imutavel_continua_valendo(tmp_path):
    """String, int, flo, bool e TUPLA seguem válidas como chave."""
    fonte = ('d = {}\nd["s"] = 1\nd[2] = "b"\nd[2.5] = "c"\nd[true] = "d"\n'
             'd[(1, 2)] = "tup"\npost(len(d), d["s"], d[2], d[(1, 2)])\n')
    assert ambos(tmp_path, fonte) == ["5 1 b tup"]


def test_json_inteiro_grande_nao_satura(tmp_path):
    fonte = ('import json\n'
             'd = json.parse("{\\"n\\": 123456789012345678901234567890}")\n'
             'post(d["n"])\n')
    assert ambos(tmp_path, fonte) == ["123456789012345678901234567890"]


def test_json_inteiro_normal_nao_regrediu(tmp_path):
    fonte = ('import json\n'
             'd = json.parse("{\\"a\\": 42, \\"b\\": -7, \\"c\\": 2.5}")\n'
             'post(d["a"], d["b"], d["c"])\n')
    assert ambos(tmp_path, fonte) == ["42 -7 2.5"]
