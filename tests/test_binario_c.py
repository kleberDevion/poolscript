"""O binário `pool` — zero CPython.

O mesmo `poolscript_vm.c` compila de dois jeitos: com `-DPS_MODULO_PYTHON` vira
a extensão que os outros testes usam, sem ela vira objeto deste executável. Os
dois chamam `ps_roda_fonte`, então o que estes testes checam não é "a linguagem
funciona" (isso os outros arquivos já fazem) e sim **que o empacotamento não
mudou o comportamento** — e que nada de Python sobrou.
"""
import os
import shutil
import subprocess
import sys

import pytest

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
POOL = os.path.join(RAIZ, "pool")

pytestmark = pytest.mark.skipif(
    not os.path.isfile(POOL),
    reason="binário não compilado — rode: make",
)

NL = chr(10)


def roda(*args, cwd=None, env=None):
    e = dict(os.environ)
    if env:
        e.update(env)
    return subprocess.run([POOL, *args], capture_output=True, text=True, cwd=cwd, env=e)


def saida(*args, **kw):
    r = roda(*args, **kw)
    assert r.returncode == 0, r.stderr
    return r.stdout.splitlines()


# ── o empacotamento ─────────────────────────────────────────────────────────

def test_nao_depende_de_python():
    """O ponto inteiro do binário. `ldd` não pode listar libpython, e não pode
    haver símbolo de Python por resolver."""
    ldd = subprocess.run(["ldd", POOL], capture_output=True, text=True).stdout
    assert "python" not in ldd.lower(), ldd
    nm = subprocess.run(["nm", "-uD", POOL], capture_output=True, text=True).stdout
    assert not [l for l in nm.splitlines() if "Py" in l or "python" in l.lower()]


def test_version_e_help():
    assert saida("--version")[0].startswith("PoolScript ")
    assert roda("--help").returncode == 0


def test_sem_argumento_reclama():
    r = roda()
    assert r.returncode != 0 and "uso:" in r.stderr


# ── execução ────────────────────────────────────────────────────────────────

def test_executa_expressao():
    assert saida("-e", 'post("oi")') == ["oi"]


def test_executa_arquivo(tmp_path):
    f = tmp_path / "a.ps"
    f.write_text('post(1 + 1)' + NL, encoding="utf-8")
    assert saida(str(f)) == ["2"]


def test_arquivo_inexistente():
    r = roda("/nao/existe.ps")
    assert r.returncode == 66 and "nao consegui abrir" in r.stderr


@pytest.mark.parametrize("src,esperado,rc", [
    ("post(1/0)",            "SomeValueUnexpected", 1),
    ("x = (1",               "SyntaxError",       2),
    ("post(await 1)",        "NotImplementedError", 3),
])
def test_erro_sai_com_codigo_proprio(src, esperado, rc):
    """Cada fase tem seu código de saída — script de shell precisa distinguir
    erro de sintaxe de erro em execução."""
    r = roda("-e", src)
    assert r.returncode == rc, r.stderr
    assert esperado in r.stderr


# ── import de `.ps` ─────────────────────────────────────────────────────────

def test_importa_modulo_vizinho(tmp_path):
    (tmp_path / "mat.ps").write_text(
        "action dobro(n) {" + NL + " return n * 2" + NL + "}" + NL, encoding="utf-8")
    uso = tmp_path / "uso.ps"
    uso.write_text("import mat" + NL + "post(mat.dobro(21))" + NL, encoding="utf-8")
    assert saida(str(uso)) == ["42"]


def test_from_import(tmp_path):
    (tmp_path / "mat.ps").write_text(
        "action soma(a, b) {" + NL + " return a + b" + NL + "}" + NL, encoding="utf-8")
    uso = tmp_path / "uso.ps"
    uso.write_text("from mat import soma" + NL + "post(soma(1, 2))" + NL, encoding="utf-8")
    assert saida(str(uso)) == ["3"]


def test_importa_lib_instalada_de_outra_pasta(tmp_path):
    """O ciclo que liga o `psl` ao binário: instalado em ~/.poolscript/libs,
    importável de qualquer diretório."""
    lar = tmp_path / "lar"
    (lar / "libs").mkdir(parents=True)
    (lar / "libs" / "glob.ps").write_text(
        "#!lib" + NL + "action oi() {" + NL + ' return "da lib"' + NL + "}" + NL,
        encoding="utf-8")
    outra = tmp_path / "outra"
    outra.mkdir()
    uso = outra / "uso.ps"
    uso.write_text("import glob" + NL + "post(glob.oi())" + NL, encoding="utf-8")
    assert saida(str(uso), env={"POOLSCRIPT_HOME": str(lar)}) == ["da lib"]


def test_arquivo_vizinho_ganha_da_lib_instalada(tmp_path):
    """Instalar uma lib não pode sequestrar um módulo local de mesmo nome."""
    lar = tmp_path / "lar"
    (lar / "libs").mkdir(parents=True)
    (lar / "libs" / "m.ps").write_text(
        'action q() {' + NL + ' return "global"' + NL + '}' + NL, encoding="utf-8")
    proj = tmp_path / "proj"
    proj.mkdir()
    (proj / "m.ps").write_text(
        'action q() {' + NL + ' return "local"' + NL + '}' + NL, encoding="utf-8")
    uso = proj / "uso.ps"
    uso.write_text("import m" + NL + "post(m.q())" + NL, encoding="utf-8")
    assert saida(str(uso), env={"POOLSCRIPT_HOME": str(lar)}) == ["local"]


def test_modulo_nativo_ganha_de_arquivo_local(tmp_path):
    """`json.ps` na pasta não pode sequestrar o módulo `json` da linguagem."""
    (tmp_path / "json.ps").write_text(
        'action stringify(x) {' + NL + ' return "sequestrado"' + NL + '}' + NL,
        encoding="utf-8")
    uso = tmp_path / "uso.ps"
    uso.write_text("import json" + NL + "post(json.stringify([1]))" + NL, encoding="utf-8")
    assert saida(str(uso)) == ["[1]"]


def test_corpo_do_modulo_roda_uma_vez_so(tmp_path):
    (tmp_path / "ef.ps").write_text(
        'post("corpo")' + NL + "action f() {" + NL + " return 1" + NL + "}" + NL,
        encoding="utf-8")
    uso = tmp_path / "uso.ps"
    uso.write_text("import ef" + NL + "import ef" + NL + "post(ef.f())" + NL, encoding="utf-8")
    assert saida(str(uso)) == ["corpo", "1"]


def test_modulo_inexistente(tmp_path):
    uso = tmp_path / "uso.ps"
    uso.write_text("import naoexiste" + NL, encoding="utf-8")
    r = roda(str(uso))
    assert r.returncode != 0 and "modulo nao encontrado" in r.stderr


def test_erro_dentro_do_modulo_aponta_o_modulo(tmp_path):
    (tmp_path / "ruim.ps").write_text("x = (1" + NL, encoding="utf-8")
    uso = tmp_path / "uso.ps"
    uso.write_text("import ruim" + NL, encoding="utf-8")
    r = roda(str(uso))
    assert r.returncode != 0 and "ruim" in r.stderr


def test_entity_e_gerador_atravessam_o_modulo(tmp_path):
    """Entity e gerador definidos num módulo precisam funcionar do lado de
    fora — é o que prova que a realocação de protótipo e classe está certa."""
    (tmp_path / "lib.ps").write_text(
        "Entity Ponto():" + NL + "    x: int" + NL + "    y: int" + NL
        + "action conta(n) {" + NL + " i = 0" + NL + " while i < n {" + NL
        + "  yield i" + NL + "  i++" + NL + " }" + NL + "}" + NL,
        encoding="utf-8")
    uso = tmp_path / "uso.ps"
    uso.write_text(
        "import lib" + NL + "p = lib.Ponto(3, 4)" + NL + "post(p.x + p.y)" + NL
        + "post(list(lib.conta(3)))" + NL, encoding="utf-8")
    assert saida(str(uso)) == ["7", "[0, 1, 2]"]


def test_modulo_usa_builtin(tmp_path):
    """As globais do módulo ficam numa faixa própria — os builtins têm que ser
    religados lá, senão `str()` dentro da lib é variável indefinida."""
    (tmp_path / "lib.ps").write_text(
        "action rotulo(n) {" + NL + ' return "n=" + str(n)' + NL + "}" + NL,
        encoding="utf-8")
    uso = tmp_path / "uso.ps"
    uso.write_text("import lib" + NL + "post(lib.rotulo(7))" + NL, encoding="utf-8")
    assert saida(str(uso)) == ["n=7"]


# ── mesma resposta que a extensão ───────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'post("olá ção".upper())',
    "post([3,1,2])",
    'post({"b":1,"a":2})',
    "post(1/3)",
    "post(150.0)",
    'post(<red>"x")',
    "post(count int in [1,\"a\",2])",
])
def test_binario_e_extensao_concordam(src):
    vm = pytest.importorskip("poolscript.vm.poolscript_vm")
    import io
    from contextlib import redirect_stdout
    r, w = os.pipe()
    orig = os.dup(1)
    sys.stdout.flush()
    try:
        os.dup2(w, 1)
        os.close(w)
        try:
            vm.executa_fonte(src)
        finally:
            sys.stdout.flush()
            os.dup2(orig, 1)
    finally:
        os.close(orig)
    with os.fdopen(r, "rb") as f:
        pela_extensao = f.read().decode("utf-8").splitlines()
    assert saida("-e", src) == pela_extensao
