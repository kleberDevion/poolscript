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

os.environ.setdefault("GUZER_HEADLESS", "1")

RAIZ = Path(__file__).resolve().parent.parent
NL = chr(10)

from poolscript.stdlib.guzer_lib import UI, Window, Button, Popup, _px  # noqa: E402


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
    assert app.button()._bg() == "#2196F7"      # botão default
    assert app.popup()._style["width"] == "260"  # popup default


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


def test_popup_event_child_vira_handler():
    app = UI()
    def child():
        return 42
    p = app.popup(event_child=child)
    assert p._handler is child and p._handler() == 42


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
        'app.popup(event_child=Clicker)' + NL +
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
