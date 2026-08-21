"""async/await/gather REAL no binário `pool`.

`async action` no top-level vira uma fibra (lazy); `gather`/`await` dirigem o
escalonador e as tasks correm CONCORRENTES (o `sleep` de cada uma cede). Prova
por TEMPO (3x sleep(0.3) em ~0.3s, não 0.9s) e por PARIDADE de output com o
interpretador (que já tinha async via thread).
"""
import os
import subprocess
import sys
import time
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

pytestmark = pytest.mark.skipif(not POOL.exists(), reason="binário pool não compilado")

GATHER = (
    "async action t(n):\n"
    "    sleep(0.3)\n"
    "    return n * 2\n"
    "\n"
    "post(gather(t(1), t(2), t(3)))\n"
)

AWAIT = (
    "async action t(n):\n"
    "    sleep(0.3)\n"
    "    return n + 100\n"
    "\n"
    "a = t(1)\n"
    "b = t(2)\n"
    "post(await a)\n"
    "post(await b)\n"
)


def _pool(src, tmp_path):
    p = tmp_path / "a.ps"
    p.write_text(src, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    t0 = time.monotonic()
    r = subprocess.run([str(POOL), str(p)], capture_output=True, text=True, env=env, timeout=20)
    return r, time.monotonic() - t0


def _interp(src, tmp_path):
    p = tmp_path / "b.ps"
    p.write_text(src, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run([sys.executable, "-m", "poolscript", str(p)],
                          capture_output=True, text=True, env=env, timeout=20)


def test_gather_concorrente(tmp_path):
    r, dt = _pool(GATHER, tmp_path)
    assert r.returncode == 0, r.stderr
    assert r.stdout.strip() == "[2, 4, 6]", r.stdout
    # serial = 3*0.3 = 0.9s; concorrente ~0.3s (+ startup). Margem folgada.
    assert dt < 0.7, f"async serializou? {dt:.2f}s"


def test_gather_parity(tmp_path):
    r, _ = _pool(GATHER, tmp_path)
    i = _interp(GATHER, tmp_path)
    assert r.stdout == i.stdout == "[2, 4, 6]\n", (r.stdout, i.stdout)


def test_await_concorrente(tmp_path):
    r, dt = _pool(AWAIT, tmp_path)
    assert r.returncode == 0, r.stderr
    assert r.stdout.strip() == "101\n102", r.stdout
    # duas tasks disparadas antes do 1o await -> dormem juntas; serial = 0.6s.
    assert dt < 0.55, f"await serializou? {dt:.2f}s"


def test_await_parity(tmp_path):
    r, _ = _pool(AWAIT, tmp_path)
    i = _interp(AWAIT, tmp_path)
    assert r.stdout == i.stdout == "101\n102\n", (r.stdout, i.stdout)
