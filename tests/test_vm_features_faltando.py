"""Features que a VM em C simplesmente não tinha (ou ignorava em silêncio).

Da caça de 2026-08-25 (notas/caca-bugs-2026-08-25.md). Todas já funcionavam no
interpretador e/ou estão na documentação:

  - `for each i` DESTRUÍA a variável (ou a action!) de mesmo nome de fora;
  - f-string com UMA expressão devolvia o valor cru — `f"{lista}"` era a
    PRÓPRIA lista, dava pra `.append` no resultado e mutava o original;
  - `bytes[1:3]` respondia "tipo nao fatiavel";
  - `f.read(3)` sugava o arquivo inteiro e a leitura seguinte vinha vazia;
  - `@NonNull` dentro de Entity era descartado junto com o nó do decorador;
  - `import pacote.modulo` (documentado em docs/linguagem/09-imports.md) era
    NotImplementedError.
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


def _run(cmd, cwd, arquivo):
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run(cmd + [arquivo], capture_output=True, text=True,
                          env=env, cwd=str(cwd), timeout=30)


def ambos(tmp_path, fonte, arquivo="t.ps"):
    (tmp_path / arquivo).write_text(fonte, encoding="utf-8")
    v = _run([str(POOL)], tmp_path, arquivo)
    i = _run([sys.executable, "-m", "poolscript"], tmp_path, arquivo)
    assert v.returncode == i.returncode, (v.stderr[:200], i.stderr[:200])
    assert v.stdout == i.stdout, f"VM {v.stdout!r} != interp {i.stdout!r}"
    return v.stdout.strip().split("\n")


def test_for_each_sombreia_variavel_de_fora(tmp_path):
    assert ambos(tmp_path, 'i = "importante"\nfor each i in [1, 2]:\n'
                           '    post(i)\npost(i)\n') == ["1", "2", "importante"]


def test_for_each_nao_apaga_action_de_mesmo_nome(tmp_path):
    assert ambos(tmp_path, 'action f():\n    return "sou action"\n'
                           'for each f in [1, 2]:\n    post(f)\n'
                           'post(f())\n') == ["1", "2", "sou action"]


def test_for_each_sem_nome_de_fora_nao_vaza(tmp_path):
    """A variável do laço continua NÃO existindo depois (isso já valia)."""
    (tmp_path / "t.ps").write_text("for each z in [1]:\n    post(z)\npost(z)\n",
                                   encoding="utf-8")
    r = _run([str(POOL)], tmp_path, "t.ps")
    assert r.returncode != 0 and "não definida" in r.stderr


@pytest.mark.parametrize("expr,tipo", [
    ('f"{[1, 2]}"', "str"),
    ('f"{42}"', "str"),
    ('f"{ { \\"a\\": 1 } }"', "str"),
    ('f"{\\"txt\\"}"', "str"),
])
def test_fstring_de_uma_expressao_vira_string(tmp_path, expr, tipo):
    assert ambos(tmp_path, f"post(type({expr}))\n") == [tipo]


def test_fstring_nao_devolve_a_mesma_referencia(tmp_path):
    """`s = f"{l}"` seguido de mutação no `s` não pode tocar no `l`."""
    (tmp_path / "t.ps").write_text(
        'l = [1, 2]\ns = f"{l}"\npost(type(s))\npost(l)\n', encoding="utf-8")
    r = _run([str(POOL)], tmp_path, "t.ps")
    assert r.returncode == 0, r.stderr
    assert r.stdout.split("\n")[:2] == ["str", "[1, 2]"]


def test_slice_de_bytes(tmp_path):
    assert ambos(tmp_path, 'c = "ola".encode()\n'
                           'post(c[0:2], c[-1:], len(c[:]))\n') == ["b'ol' b'a' 3"]


def test_read_com_limite_nao_consome_o_arquivo(tmp_path):
    (tmp_path / "x.txt").write_text("abcdef", encoding="utf-8")
    assert ambos(tmp_path, 'using open("x.txt") as f:\n'
                           '    post(f.read(3))\n    post(f.read(2))\n'
                           '    post(f.read())\n') == ["abc", "de", "f"]


def test_nonnull_dentro_de_entity(tmp_path):
    (tmp_path / "t.ps").write_text(
        'class C():\n    @NonNull\n    action f(self, a):\n        return a\n'
        'post(C().f(null))\n', encoding="utf-8")
    r = _run([str(POOL)], tmp_path, "t.ps")
    assert r.returncode != 0, "a VM ignorou o @NonNull"
    assert "NonNull" in r.stderr and "'a'" in r.stderr, r.stderr


def test_nonnull_com_valor_valido_passa(tmp_path):
    assert ambos(tmp_path, 'class C():\n    @NonNull\n    action f(self, a):\n'
                           '        return a\npost(C().f(7))\n') == ["7"]


@pytest.mark.parametrize("fonte,esperado", [
    ("import sub.mod\npost(mod.oi())\n", "do sub"),
    ("import sub.mod as sm\npost(sm.oi())\n", "do sub"),
    ("from sub.mod import oi\npost(oi())\n", "do sub"),
])
def test_import_pontuado(tmp_path, fonte, esperado):
    """docs/linguagem/09-imports.md: o nome ligado é o ÚLTIMO segmento."""
    (tmp_path / "sub").mkdir()
    (tmp_path / "sub" / "mod.ps").write_text(
        'action oi():\n    return "do sub"\n', encoding="utf-8")
    assert ambos(tmp_path, fonte) == [esperado]
