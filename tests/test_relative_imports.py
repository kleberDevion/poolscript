"""Testes de import relativo (`from .modulo import x` / `from ..pkg.modulo import x`).

Cobre o caso que motivou a feature: um arquivo importado de dentro de um
subdiretório (ex: api/controll_api/logIn.ps, importado por api/main.ps) precisa
resolver seus próprios imports relativos à SUA pasta, não à raiz do projeto
(que é onde o entry point mora) — igual ao `from .x import y` do Python.
"""
from __future__ import annotations

import io
import sys
from contextlib import redirect_stdout, redirect_stderr

from poolscript import cli


def run_cli(args):
    out, err = io.StringIO(), io.StringIO()
    with redirect_stdout(out), redirect_stderr(err):
        rc = cli.main(args)
    return rc, out.getvalue(), err.getvalue()


def test_relative_import_same_dir(tmp_path):
    # entry/pkg/mod.ps importa entry/pkg/sibling.ps via `from .sibling import x`
    (tmp_path / "pkg").mkdir()
    (tmp_path / "pkg" / "sibling.ps").write_text('valor = "do irmao"', encoding="utf-8")
    (tmp_path / "pkg" / "mod.ps").write_text(
        "from .sibling import valor\npost(valor)\n", encoding="utf-8"
    )
    (tmp_path / "main.ps").write_text(
        "from pkg.mod import valor\npost(valor)\n", encoding="utf-8"
    )
    rc, out, err = run_cli([str(tmp_path / "main.ps")])
    assert rc == 0, err
    assert out.count("do irmao") == 2


def test_relative_import_parent_dir(tmp_path):
    # entry/pkg/deep/worker.ps sobe um nível com `from ..logIn import logger`
    (tmp_path / "pkg" / "deep").mkdir(parents=True)
    (tmp_path / "pkg" / "logIn.ps").write_text(
        'action logger() { return "logIn do nivel acima" }\n', encoding="utf-8"
    )
    (tmp_path / "pkg" / "deep" / "worker.ps").write_text(
        "from ..logIn import logger\n"
        "action run() { return logger() }\n",
        encoding="utf-8",
    )
    (tmp_path / "main.ps").write_text(
        "from pkg.deep.worker import run\npost(run())\n", encoding="utf-8"
    )
    rc, out, err = run_cli([str(tmp_path / "main.ps")])
    assert rc == 0, err
    assert "logIn do nivel acima" in out


def test_relative_import_reproduces_chat_api_bug_scenario(tmp_path):
    # Reprodução exata do caso reportado: api/main.ps -> api/controll_api/logIn.ps
    # -> api/controll_api/classes/user_struct.ps, via `from .classes.user_struct import x`
    # escrito dentro de logIn.ps (não relativo à raiz do projeto).
    api = tmp_path / "api"
    controll_api = api / "controll_api"
    classes = controll_api / "classes"
    classes.mkdir(parents=True)
    (classes / "user_struct.ps").write_text('person_data = "ok"', encoding="utf-8")
    (controll_api / "logIn.ps").write_text(
        "from .classes.user_struct import person_data\n"
        "action logger() { return person_data }\n",
        encoding="utf-8",
    )
    (api / "main.ps").write_text(
        "from controll_api.logIn import logger\npost(logger())\n", encoding="utf-8"
    )
    rc, out, err = run_cli([str(api / "main.ps")])
    assert rc == 0, err
    assert "ok" in out


def test_relative_import_missing_module_raises_clear_error(tmp_path):
    (tmp_path / "pkg").mkdir()
    (tmp_path / "pkg" / "mod.ps").write_text(
        "from .nao_existe import x\n", encoding="utf-8"
    )
    (tmp_path / "main.ps").write_text(
        "import pkg.mod\n", encoding="utf-8"
    )
    rc, out, err = run_cli([str(tmp_path / "main.ps")])
    assert rc != 0
    assert "nao_existe" in err or "não encontrado" in err


def test_relative_import_without_module_after_dots_raises(tmp_path):
    (tmp_path / "pkg").mkdir()
    (tmp_path / "pkg" / "mod.ps").write_text(
        "from . import x\n", encoding="utf-8"
    )
    (tmp_path / "main.ps").write_text(
        "import pkg.mod\n", encoding="utf-8"
    )
    rc, out, err = run_cli([str(tmp_path / "main.ps")])
    assert rc != 0
    assert "import relativo" in err


def test_absolute_import_from_project_root_still_works(tmp_path):
    # Import absoluto (sem ponto) continua resolvendo relativo à RAIZ do
    # projeto (pasta do entry point), independente de quem faz o import —
    # comportamento antigo, não pode quebrar com a feature nova.
    (tmp_path / "services").mkdir()
    (tmp_path / "services" / "controllSmtp.ps").write_text(
        'action enviar() { return "enviado" }\n', encoding="utf-8"
    )
    (tmp_path / "services" / "user.ps").write_text(
        "from services.controllSmtp import enviar\npost(enviar())\n", encoding="utf-8"
    )
    (tmp_path / "main.ps").write_text(
        "import services.user\n", encoding="utf-8"
    )
    rc, out, err = run_cli([str(tmp_path / "main.ps")])
    assert rc == 0, err
    assert "enviado" in out
