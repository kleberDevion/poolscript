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


# ── traceback completo (call stack) ─────────────────────────────────────────

def test_traceback_call_stack_completo(tmp_path):
    """Erro não-capturado mostra a pilha inteira com arquivo/linha/trecho em
    cada quadro — e no MESMO formato do interpretador (a autoridade): mensagem
    primeiro, header, quadros `em <arq>, linha N` + trecho + cursor `^^^`."""
    f = tmp_path / "t.ps"
    f.write_text(
        "action c(x) {" + NL + " return x + naoexiste" + NL + "}" + NL
        + "action b(x) {" + NL + " return c(x)" + NL + "}" + NL
        + "post(b(1))" + NL, encoding="utf-8")
    r = roda(str(f))
    assert r.returncode == 1
    assert r.stderr.startswith("RuntimeError: variável não definida: naoexiste")
    assert "Traceback (arquivo mais recente por último):" in r.stderr
    # os três quadros (linhas 5, 7, 2) aparecem, na ordem do interp
    i5 = r.stderr.find("linha 5")
    i7 = r.stderr.find("linha 7")
    i2 = r.stderr.find("linha 2")
    assert 0 <= i5 < i7 < i2
    assert "return x + naoexiste" in r.stderr and "^^^" in r.stderr


def test_traceback_atravessa_modulo(tmp_path):
    (tmp_path / "u.ps").write_text(
        "action quebra() {" + NL + " return sumiu" + NL + "}" + NL, encoding="utf-8")
    f = tmp_path / "t.ps"
    f.write_text("from u import quebra" + NL + "post(quebra())" + NL, encoding="utf-8")
    r = roda(str(f))
    assert r.returncode == 1 and "Traceback" in r.stderr
    # o quadro do módulo aponta o arquivo certo (u.ps) e mostra o trecho
    assert "u.ps" in r.stderr and "return sumiu" in r.stderr


def test_traceback_identico_ao_interp(tmp_path):
    """A prova de paridade: a saída de erro do binário é IGUAL à do interp
    (a autoridade). Simples e aninhado."""
    import io
    from contextlib import redirect_stdout
    from poolscript import ps_errors as pe
    from poolscript.interpreter import Interpreter
    from poolscript.parser import parse_source

    def do_interp(src, path):
        pe._USE_COLOR = False
        try:
            with redirect_stdout(io.StringIO()):
                Interpreter(source=src, filename=path).run(parse_source(src, path))
        except BaseException as ex:
            return ex.format() if hasattr(ex, "format") else str(ex)
        return ""

    casos = {
        "simp.ps": "x = 5" + NL + "y = naoexiste" + NL,
        "nest.ps": ("action c(x) {" + NL + " return x + naoexiste" + NL + "}" + NL
                    + "action b(x) {" + NL + " return c(x)" + NL + "}" + NL
                    + "post(b(1))" + NL),
    }
    for nome, src in casos.items():
        f = tmp_path / nome
        f.write_text(src, encoding="utf-8")
        # uso real: `pool arquivo.ps` com path relativo, a partir do dir dele —
        # é assim que os dois mostram o mesmo nome de arquivo.
        vm_out = roda(nome, cwd=str(tmp_path)).stderr.rstrip("\n")
        interp_out = do_interp(src, nome).rstrip("\n")
        assert vm_out == interp_out, f"\n--VM--\n{vm_out}\n--INTERP--\n{interp_out}"


def test_using_open_com_mode_kwarg(tmp_path):
    """open() aceita `mode=` (paridade com o interp) e o `using` grava/fecha."""
    f = tmp_path / "t.ps"
    f.write_text(
        'using open("saida.txt", mode="w") as arq:' + NL
        + '    arq.write("oi")' + NL
        + '    post("ok")' + NL, encoding="utf-8")
    assert saida(str(f), cwd=str(tmp_path)) == ["ok"]
    assert (tmp_path / "saida.txt").read_text() == "oi"


def test_using_nao_mascara_erro_de_aquisicao(tmp_path):
    """Se a expressão do `using` falha, o erro REAL propaga — não vira o
    enganoso "variavel nao definida: f" da limpeza."""
    f = tmp_path / "t.ps"
    f.write_text(
        'using open("x.txt", argento="w") as f:' + NL
        + '    f.write("hi")' + NL, encoding="utf-8")
    r = roda(str(f), cwd=str(tmp_path))
    assert r.returncode != 0
    assert "variavel nao definida: f" not in r.stderr


def test_metodo_inexistente_nomeia_o_metodo(tmp_path):
    """Método que não existe no tipo nomeia o método (paridade com o interp:
    'membro inexistente: save'), não o genérico 'esse tipo nao tem esse metodo'."""
    f = tmp_path / "t.ps"
    f.write_text("u = 5" + NL + 'u.save(".")' + NL, encoding="utf-8")
    r = roda(str(f))
    assert r.returncode == 1
    assert "membro inexistente: save" in r.stderr


def test_tipo_como_valor_primeira_classe(tmp_path):
    """Tipos (`str`/`int`/...) usáveis como VALOR: `f = str; f("5")`,
    `map(l, str)`, `filter(l, bool)`. E `is` entre tipos = identidade
    (`int is int` True, `int is str` False). Binário IDÊNTICO ao interp."""
    import io
    from contextlib import redirect_stdout
    from poolscript import ps_errors as pe
    from poolscript.interpreter import Interpreter
    from poolscript.parser import parse_source

    src = (
        'f = str' + NL + 'post(f("5"))' + NL
        + 'post(str is str)' + NL + 'post(int is int)' + NL
        + 'post(int is str)' + NL + 'post(list is list)' + NL
        + 'post(str is type)' + NL + 'post(json is dict)' + NL
        + 'x = 5' + NL + 'post(x is int)' + NL
        + 'post(map([1,2,3], str))' + NL
        + 'post(map(["1","2"], int))' + NL
        + 'post(filter([0,1,2,0], bool))' + NL
        + 'post(map([1,2], flo))' + NL
        + 'g = int' + NL + 'post(g("42") + 1)' + NL
    )
    f = tmp_path / "tv.ps"
    f.write_text(src, encoding="utf-8")

    vm_out = saida(str(f))

    pe._USE_COLOR = False
    interp = Interpreter(source=src, filename=str(f))
    buf = io.StringIO()
    with redirect_stdout(buf):
        interp.run(parse_source(src, str(f)))
    interp_out = buf.getvalue().splitlines()

    assert vm_out == interp_out, f"\nVM={vm_out}\nINTERP={interp_out}"
    # e o valor certo, pra não passar por dois errados iguais
    # 0:"5"  2:int is int=True  3:int is str=False
    assert vm_out[0] == "5" and vm_out[2] == "True" and vm_out[3] == "False"


def test_jinker_multiprocesso_workers(tmp_path):
    """`app(..., workers=N)` forka N processos (prefork) que dividem o socket
    HTTP. Verifica: sobe, serve, tem >1 processo, e derruba limpo."""
    import socket
    import time

    with socket.socket() as s:
        s.bind(("127.0.0.1", 0)); porta = s.getsockname()[1]
    app = tmp_path / "mp.ps"
    app.write_text(
        'import jinker' + NL
        + 'app = jinker.Jinker("mp")' + NL
        + '@app.route("/ping", methods=["GET"])' + NL
        + 'action ping() { return jinker.jsonify({"ok": true}) }' + NL
        + 'app(host="127.0.0.1", port=' + str(porta) + ', workers=3)' + NL, encoding="utf-8")

    proc = subprocess.Popen([POOL, str(app)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        for _ in range(50):
            try:
                socket.create_connection(("127.0.0.1", porta), timeout=0.5).close(); break
            except OSError:
                time.sleep(0.1)
        else:
            pytest.skip("servidor multi-processo não subiu")

        # serve corretamente
        c = socket.create_connection(("127.0.0.1", porta)); c.settimeout(5)
        c.sendall(b"GET /ping HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n")
        assert b"200" in c.recv(4096); c.close()

        # prefork: o processo pai tem filhos (workers)
        try:
            filhos = subprocess.run(["pgrep", "-P", str(proc.pid)], capture_output=True, text=True).stdout.split()
            assert len(filhos) >= 2, f"esperava múltiplos workers, achei {len(filhos)}"
        except FileNotFoundError:
            pass  # sem pgrep, pula a checagem de contagem
    finally:
        proc.terminate()
        try: proc.wait(timeout=5)
        except subprocess.TimeoutExpired: proc.kill()


def test_jinker_keepalive_ocioso_nao_bloqueia(tmp_path):
    """Uma conexão keep-alive OCIOSA não pode segurar o loop: uma conexão NOVA
    tem que ser atendida na hora (era o bug dos ~30s, antes da multiplexação)."""
    import socket
    import time

    with socket.socket() as s:
        s.bind(("127.0.0.1", 0)); porta = s.getsockname()[1]
    app = tmp_path / "srv.ps"
    app.write_text(
        'import jinker' + NL
        + 'app = jinker.Jinker("t")' + NL
        + '@app.route("/ping", methods=["GET"])' + NL
        + 'action ping() { return jinker.jsonify({"ok": true}) }' + NL
        + 'app(host="127.0.0.1", port=' + str(porta) + ')' + NL, encoding="utf-8")

    proc = subprocess.Popen([POOL, str(app)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        # espera subir
        for _ in range(50):
            try:
                socket.create_connection(("127.0.0.1", porta), timeout=0.5).close(); break
            except OSError:
                time.sleep(0.1)
        else:
            pytest.skip("servidor jinker não subiu")

        # c1: keep-alive, manda 1 request, lê, e fica OCIOSO (não fecha)
        c1 = socket.create_connection(("127.0.0.1", porta)); c1.settimeout(5)
        c1.sendall(b"GET /ping HTTP/1.1\r\nHost: x\r\nConnection: keep-alive\r\n\r\n")
        assert b"200" in c1.recv(4096)

        # c2: conexão NOVA enquanto c1 está ocioso — tem que ser rápida
        t = time.time()
        c2 = socket.create_connection(("127.0.0.1", porta)); c2.settimeout(5)
        c2.sendall(b"GET /ping HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n")
        assert b"200" in c2.recv(4096)
        dt = time.time() - t
        c1.close(); c2.close()
        assert dt < 3.0, f"conexão nova demorou {dt:.1f}s — keep-alive ocioso bloqueou o loop"
    finally:
        proc.terminate()
        try: proc.wait(timeout=5)
        except subprocess.TimeoutExpired: proc.kill()


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


# ── erro de runtime mostra ONDE caiu (arquivo/linha/trecho) ──────────────────
# A suite era so de PARIDADE e nao pegava isto: a VM dava erro de runtime SEM
# localizacao ("pra cego"). Agora o erro aponta o arquivo, a linha e o trecho.

def test_runtime_erro_mostra_linha_e_trecho(tmp_path):
    f = tmp_path / "e.ps"
    f.write_text(
        "Entity Ctrl() {" + NL +
        "    int reaction m(self, name=none) { return 1 }" + NL +
        "}" + NL +
        "post(\"antes\")" + NL +
        "x = Ctrl().naoExiste(name=1)" + NL, encoding="utf-8")
    r = roda(str(f))
    assert r.returncode != 0
    assert "antes" in r.stdout                    # rodou ate a linha do erro
    assert "membro inexistente: naoExiste" in r.stderr   # mensagem especifica
    assert "linha 5" in r.stderr                  # a linha certa
    assert "Ctrl().naoExiste" in r.stderr         # o trecho do fonte


def test_runtime_erro_linha_interna_da_action(tmp_path):
    # erro DENTRO de uma action chamada aponta a linha INTERNA, nao a chamada
    f = tmp_path / "e.ps"
    f.write_text(
        "action f(n) {" + NL +
        "    return n / 0" + NL +
        "}" + NL +
        "post(f(3))" + NL, encoding="utf-8")
    r = roda(str(f))
    assert r.returncode != 0
    assert "linha 2" in r.stderr and "n / 0" in r.stderr


def test_runtime_divisao_por_zero_com_linha(tmp_path):
    f = tmp_path / "e.ps"
    f.write_text("a = 10" + NL + "post(a / 0)" + NL, encoding="utf-8")
    r = roda(str(f))
    assert r.returncode != 0 and "linha 2" in r.stderr


def test_arg_nomeado_errado_diz_o_nome(tmp_path):
    f = tmp_path / "e.ps"
    f.write_text(
        "action f(a, b) { return a }" + NL +
        "post(f(a=1, zzz=2))" + NL, encoding="utf-8")
    r = roda(str(f))
    assert r.returncode != 0
    assert "zzz" in r.stderr and "linha 2" in r.stderr
