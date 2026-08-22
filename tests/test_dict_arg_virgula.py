"""Um dict `{...}` logo depois de um argumento SEM vírgula é vírgula esquecida,
não um novo argumento justaposto. Antes, a justaposição fazia o dict virar um
argumento posicional a mais — e uma reaction de 1 parâmetro cuspia o confuso
"argumentos demais" (ou, no caminho nomeado, silenciava o dict / divergia entre
os motores). Agora é erro de sintaxe CLARO, igual nos dois motores, apontando a
vírgula que falta. Dict com vírgula continua funcionando normalmente.
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"


def _engines():
    yield [sys.executable, "-m", "poolscript"]
    # VM em C NÃO se pula: se o pool não existe, o teste FALHA (skip = falso verde).
    assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."
    yield [str(POOL)]


def _run(cmd, tmp_path, prog):
    ps = tmp_path / "t.ps"
    ps.write_text(prog, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run(cmd + [str(ps)], capture_output=True, text=True,
                          env=env, timeout=15)


# dict justaposto (vírgula esquecida) -> erro de sintaxe claro
ERRO = 'faltou'
JUSTAPOSTOS = [
    'reaction f(x) { return x }\npost(f({"a": 1} {"b": 2}))\n',
    'reaction f(a, b=0) { return a }\npost(f({"a": 1} {"b": 2}, b=9))\n',
]


@pytest.mark.parametrize("prog", JUSTAPOSTOS)
def test_dict_sem_virgula_erro_claro(tmp_path, prog):
    for cmd in _engines():
        r = _run(cmd, tmp_path, prog)
        assert r.returncode != 0, (cmd, r.stdout)
        saida = r.stdout + r.stderr
        assert "dicionario" in saida and ERRO in saida, (cmd, saida)
        # NÃO deve mais aparecer o erro confuso
        assert "argumentos demais" not in saida, (cmd, saida)


# dict com vírgula (uso normal) continua funcionando, idêntico nos 2 motores
OK = [
    ('reaction f(x) { return x }\npost(f({"nome": "valor"}))\n', "{'nome': 'valor'}"),
    ('reaction f(a, b) { return b }\npost(f(1, {"k": 2}))\n', "{'k': 2}"),
    ('reaction f(a=0, b=none) { return b }\npost(f(a=1, b={"k": 2}))\n', "{'k': 2}"),
]


@pytest.mark.parametrize("prog,esperado", OK)
def test_dict_com_virgula_ok(tmp_path, prog, esperado):
    for cmd in _engines():
        r = _run(cmd, tmp_path, prog)
        assert r.stdout.strip() == esperado, (cmd, r.stdout, r.stderr)
