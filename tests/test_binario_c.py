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


def test_sem_argumento_mostra_ajuda():
    # sem REPL no binário C, `pool` sem args mostra a ajuda (stdout, exit 0),
    # em vez de reclamar — é o mesmo que `pool --help`.
    r = roda()
    assert r.returncode == 0 and "Uso:" in r.stdout and "pool install" in r.stdout


# ── execução ────────────────────────────────────────────────────────────────

def test_executa_expressao():
    assert saida("-e", 'post("oi")') == ["oi"]


# ── gerenciador de pacotes (só .ps: lib e comando) ──────────────────────────

def test_pkgmgr_lib_e_comando(tmp_path):
    """install/list/import/uninstall de lib e comando .ps, isolados num
    POOLSCRIPT_HOME temporário — sem tocar o ~/.poolscript real."""
    home = tmp_path / "pshome"
    env = {"POOLSCRIPT_HOME": str(home)}

    lib = tmp_path / "saud.ps"
    lib.write_text("#!lib\naction ola(n) { return \"oi \" + n }\n")
    cmd = tmp_path / "diz.ps"
    cmd.write_text("#!cmd\npost(\"comando rodou\")\n")

    # install (auto pelo marcador)
    assert roda("install", str(lib), env=env).returncode == 0
    assert roda("install", str(cmd), env=env).returncode == 0
    assert (home / "libs" / "saud.ps").is_file()
    assert (home / "commands" / "diz.ps").is_file()

    # list mostra os dois
    out = saida("list", env=env)
    assert any("saud" in l for l in out) and any("diz" in l for l in out)

    # a lib instalada é importável
    prog = tmp_path / "usa.ps"
    prog.write_text("import saud\npost(saud.ola(\"ana\"))\n")
    assert saida(str(prog), env=env) == ["oi ana"]

    # o comando roda pelo shim
    shim = home / "bin" / "diz"
    assert shim.is_file()
    r = subprocess.run([str(shim)], capture_output=True, text=True,
                       env=dict(os.environ, PATH=f"{RAIZ}:{os.environ['PATH']}", **env))
    assert "comando rodou" in r.stdout

    # uninstall remove tudo
    assert roda("uninstall", "saud", env=env).returncode == 0
    assert roda("uninstall", "diz", env=env).returncode == 0
    assert not (home / "libs" / "saud.ps").exists()
    assert not (home / "commands" / "diz.ps").exists()


def test_pkgmgr_registry_config(tmp_path):
    env = {"POOLSCRIPT_HOME": str(tmp_path / "h")}
    assert "nenhum registry" in roda("registry", "show", env=env).stdout
    assert roda("registry", "set-url", "https://ex.com/i.json", env=env).returncode == 0
    assert "https://ex.com/i.json" in roda("registry", "show", env=env).stdout


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


# ── import relativo e pontuado (from .mod / from pkg.mod) ────────────────────

def test_import_relativo_e_pontuado(tmp_path):
    """from .sibling (relativo mesmo dir), from ..logIn (relativo pai) e
    from pkg.mod (absoluto da raiz do projeto) — tudo resolvido pela VM."""
    (tmp_path / "pkg" / "deep").mkdir(parents=True)
    (tmp_path / "pkg" / "sibling.ps").write_text("valor = 42\n")
    (tmp_path / "pkg" / "mod.ps").write_text(
        "from .sibling import valor\naction pega() { return valor }\n")
    (tmp_path / "pkg" / "logIn.ps").write_text("chave = \"LOG\"\n")
    (tmp_path / "pkg" / "deep" / "worker.ps").write_text(
        "from ..logIn import chave\naction run() { return chave }\n")
    (tmp_path / "main.ps").write_text(
        "from pkg.mod import pega\n"
        "from pkg.deep.worker import run\n"
        "post(pega(), run())\n")

    r = roda("main.ps", cwd=str(tmp_path))
    assert r.returncode == 0, r.stderr
    assert r.stdout.strip() == "42 LOG"


def test_import_relativo_inexistente_erra(tmp_path):
    (tmp_path / "main.ps").write_text("from .nao_existe import x\npost(x)\n")
    r = roda("main.ps", cwd=str(tmp_path))
    assert r.returncode != 0 and "nao encontrado" in r.stderr


# ── pool --check (diagnóstico pro LSP, sem rodar) ────────────────────────────

def test_check_valido_nao_roda(tmp_path):
    import json as _j
    # tem post(), mas --check NAO executa — a saida e so o JSON
    r = roda("--check", input=None) if False else subprocess.run(
        [POOL, "--check"], input='post("NAO DEVE APARECER")\n',
        capture_output=True, text=True)
    d = _j.loads(r.stdout.strip().splitlines()[-1])
    assert d["ok"] is True
    assert "NAO DEVE APARECER" not in r.stdout


def test_check_erro_sintaxe_json():
    import json as _j
    r = subprocess.run([POOL, "--check"], input="x = (1\n",
                       capture_output=True, text=True)
    d = _j.loads(r.stdout.strip().splitlines()[-1])
    assert d["ok"] is False and d["tipo"] == "SyntaxError"
    assert d["linha"] >= 1 and d["coluna"] >= 1


def test_check_arquivo(tmp_path):
    import json as _j
    f = tmp_path / "ok.ps"
    f.write_text("action f(){ return 1 }\n")
    r = subprocess.run([POOL, "--check", str(f)], capture_output=True, text=True)
    assert _j.loads(r.stdout.strip())["ok"] is True
