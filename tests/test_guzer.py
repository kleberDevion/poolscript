"""guzer — UI desktop (janela nativa, zero download). Testa o núcleo headless:
modelo de objetos, `.stylesheet()` sobre o design default, coerção de medida
pra pixel e o handler (reaction por referência, roda no evento).

A janela nativa (tkinter) não é testada aqui — precisa de display; roda na
máquina do usuário. `GUZER_HEADLESS=1` impede o auto-show no atexit.
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

os.environ.setdefault("GUZER_HEADLESS", "1")

RAIZ = Path(__file__).resolve().parent.parent
POOL_BIN = RAIZ / "pool"
assert POOL_BIN.exists(), "binário pool não compilado — rode ./rebuild_vm.sh (VM em C não se pula: skip = falso verde)"
NL = chr(10)

from poolscript.stdlib.guzer_lib import UI, Window, Button, _px  # noqa: E402


def test_px_pega_primeiro_numero():
    assert _px("500", 0) == 500
    assert _px("8px 14px", 0) == 8      # shorthand -> primeira medida
    assert _px(None, 34) == 34          # ausente -> default
    assert _px(120, 0) == 120


def test_stylesheet_mescla_sobre_default_e_encadeia():
    app = UI()
    w = app.window()
    assert isinstance(w, Window)
    ret = w.stylesheet({"width": "500", "background": "#000"})
    assert ret is w, "stylesheet tem que ser encadeável (retorna o próprio)"
    assert w._style["width"] == "500" and w._bg() == "#000"
    assert w._style["height"] == "320", "default não sobrevivente"


def test_defaults_por_objeto():
    app = UI()
    assert app.button()._bg() == "#2196F7"       # botão default
    assert app.button()._style["height"] == "34"  # botão default


def test_bg_aceita_background_ou_bg():
    app = UI()
    assert app.button().stylesheet({"bg": "#123456"})._bg() == "#123456"
    assert app.window().stylesheet({"background": "#654321"})._bg() == "#654321"


def test_handler_por_referencia_so_roda_no_evento():
    app = UI()
    marca = []
    def clicker():
        marca.append(1)
        return "Ola"
    b = app.button(onclick=clicker)
    assert b._handler is clicker
    assert marca == [], "a reaction NÃO pode rodar ao criar o objeto"
    assert b._handler() == "Ola" and marca == [1]


def test_text_encadeia():
    app = UI()
    b = app.button().text("OK")
    assert b._text == "OK"
    assert b.stylesheet({"width": "10"}).text("Vai")._text == "Vai"


def test_interpretador_real_import_e_encadeia(tmp_path):
    """Roda de verdade no INTERP: import guzer, objetos e chaining sem erro."""
    entry = tmp_path / "app.ps"
    entry.write_text(
        'int reaction Clicker() { post("clicou") }' + NL +
        'import guzer' + NL +
        'app = guzer.UI("x")' + NL +
        'app.window().stylesheet({ "width": "500", "background": "#101418" })' + NL +
        'app.button(onclick=Clicker).stylesheet({ "width": "100" }).text("Vai")' + NL +
        'post("guzer-ok")' + NL,
        encoding="utf-8",
    )
    env = dict(os.environ)
    env["PYTHONPATH"] = str(RAIZ / "src")
    env["GUZER_HEADLESS"] = "1"
    r = subprocess.run([sys.executable, "-m", "poolscript", str(entry)],
                       capture_output=True, text=True, env=env)
    assert r.returncode == 0, r.stdout + r.stderr
    assert "guzer-ok" in r.stdout, r.stdout + r.stderr
    assert "Traceback" not in r.stderr, r.stderr


def test_paridade_dois_motores(tmp_path):
    """O MESMO script guzer nos DOIS motores (INTERP e VM em C) tem que dar a
    saída IDÊNTICA — o objeto, os tipos, o encadeamento. A janela em si não roda
    (GUZER_HEADLESS), mas o modelo de objetos é paridade de verdade."""
    entry = tmp_path / "app.ps"
    entry.write_text(
        'int reaction Clicker() { post("ok") }' + NL +
        'import guzer' + NL +
        'app = guzer.UI("Demo")' + NL +
        'app.window().stylesheet({ "width": "500", "height": "300", "background": "#101418" })' + NL +
        'b = app.button(onclick=Clicker).stylesheet({ "width": "120" }).text("Vai")' + NL +
        'post(type(app))' + NL +
        'post(type(b))' + NL,
        encoding="utf-8",
    )
    env = dict(os.environ)
    env["GUZER_HEADLESS"] = "1"
    env["PYTHONPATH"] = str(RAIZ / "src")
    a = subprocess.run([sys.executable, "-m", "poolscript", str(entry)],
                       capture_output=True, text=True, env=env)
    b = subprocess.run([str(POOL_BIN), str(entry)],
                       capture_output=True, text=True, env=env)
    assert a.returncode == 0 and b.returncode == 0, a.stdout + a.stderr + b.stdout + b.stderr
    assert a.stdout == b.stdout, "divergência:\nINTERP:\n" + a.stdout + "\nVM C:\n" + b.stdout
    assert a.stdout.strip().splitlines()[-2:] == ["UI", "Button"], a.stdout


# ── TODOS os elementos do HTML, paridade elemento a elemento ────────────────
# A tabela é a MESMA nos dois motores (METODOS_GUZ_UI na VM; _TAGS_BOX no
# interp). Cada elemento é criado, o type() devolve a TAG, stylesheet/text
# encadeiam, e os atributos (placeholder/src/href/onclick) são aceitos.
TAGS = ("div section article aside header footer nav main figure figcaption "
        "address span p a strong em b i u s small mark sub sup code pre "
        "blockquote cite q abbr time kbd samp var del ins hr br h1 h2 h3 h4 "
        "h5 h6 ul ol li dl dt dd table thead tbody tfoot tr td th caption "
        "form entry textarea select option optgroup label fieldset legend "
        "datalist output progress meter img audio video canvas iframe "
        "details summary menu picture dialog").split()


def test_todos_os_elementos_nos_dois_motores(tmp_path):
    corpo = ["import guzer", 'app = guzer.UI("t")']
    for t in TAGS:
        corpo.append(f"post(type(app.{t}()))")
    corpo.append('e = app.entry(placeholder="dica")')
    corpo.append('post(type(e))')
    corpo.append('i = app.img(src="qualquer/caminho/logo.png")')
    corpo.append('post(type(i))')
    corpo.append('post(app.div().stylesheet({"width": "300"}).text("x") is null == false)')
    entry = tmp_path / "app.ps"
    entry.write_text(NL.join(corpo) + NL, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(RAIZ / "src"), GUZER_HEADLESS="1")
    a = subprocess.run([sys.executable, "-m", "poolscript", str(entry)],
                       capture_output=True, text=True, env=env)
    b = subprocess.run([str(POOL_BIN), str(entry)],
                       capture_output=True, text=True, env=env)
    assert a.returncode == 0, "interp: " + a.stdout + a.stderr
    assert b.returncode == 0, "VM: " + b.stdout + b.stderr
    assert a.stdout == b.stdout, "divergência:\nINTERP:\n" + a.stdout + "\nVM:\n" + b.stdout
    linhas = a.stdout.strip().splitlines()
    assert linhas[:len(TAGS)] == TAGS, "type() de algum elemento não devolve a tag"


def test_icon_e_img_resolvem_de_qualquer_caminho(tmp_path):
    """img/ícone vêm de onde o usuário quiser: absoluto, relativo ao script,
    ou ao diretório atual (headless valida o modelo; o desenho é na janela)."""
    from poolscript.stdlib.guzer_lib import _resolve_arquivo
    png = tmp_path / "logo.png"
    png.write_bytes(b"\x89PNG\r\n\x1a\n")          # só pra existir no disco
    assert _resolve_arquivo(str(png)) == str(png)   # absoluto
    assert _resolve_arquivo(str(tmp_path / "nao_existe.png")) is None


def _roda_nos_dois(tmp_path, fonte):
    entry = tmp_path / "app.ps"
    entry.write_text(fonte, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(RAIZ / "src"), GUZER_HEADLESS="1")
    a = subprocess.run([sys.executable, "-m", "poolscript", str(entry)],
                       capture_output=True, text=True, env=env)
    b = subprocess.run([str(POOL_BIN), str(entry)],
                       capture_output=True, text=True, env=env)
    assert a.returncode == 0, "interp: " + a.stdout + a.stderr
    assert b.returncode == 0, "VM: " + b.stdout + b.stderr
    assert a.stdout == b.stdout, "divergência:\nINTERP:\n" + a.stdout + "\nVM:\n" + b.stdout
    return a.stdout


def test_arvore_elemento_dentro_de_container(tmp_path):
    """A ÁRVORE do HTML: entry DENTRO do form, p DENTRO da div — o filho é
    criado A PARTIR do pai e o layout o põe dentro da caixa dele."""
    saida = _roda_nos_dois(tmp_path,
        "import guzer" + NL +
        'app = guzer.UI("t")' + NL +
        "form = app.form(name=\"cadastro\")" + NL +
        'e1 = form.entry(name="nome", placeholder="seu nome")' + NL +
        "bt = form.button()" + NL +
        "d = app.div()" + NL +
        'd.p().text("dentro")' + NL +
        "post(type(form), type(e1), type(bt))" + NL +
        'post(app.POOLHTMLElements.getitemByIdentify("nome").value)' + NL)
    assert saida == "form entry Button" + NL + "seu nome" + NL


def test_registro_poolhtmlelements(tmp_path):
    """app.POOLHTMLElements.getitemByIdentify("nome").value — acha o elemento
    pelo name= (em QUALQUER nível da árvore); .value = value= > .text() >
    placeholder= > ""; sem achar: null."""
    saida = _roda_nos_dois(tmp_path,
        "import guzer" + NL +
        'app = guzer.UI("t")' + NL +
        "form = app.form()" + NL +
        'form.entry(name="nome", value="kleber")' + NL +
        'app.entry(name="email")' + NL +
        'post(app.POOLHTMLElements.getitemByIdentify("nome").value)' + NL +
        'post(app.POOLHTMLElements.getitemByIdentify("email").value == "")' + NL +
        'post(app.POOLHTMLElements.getitemByIdentify("x") == null)' + NL +
        'app.getitemByIdentify("email").text("x@y.z")' + NL +
        'post(app.POOLHTMLElements.getitemByIdentify("email").value)' + NL)
    assert saida == "kleber" + NL + "True" + NL + "True" + NL + "x@y.z" + NL
