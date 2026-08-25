"""Casos que DERRUBAVAM o processo da VM em C — segfault, SIGFPE ou OOM.

Achados em 2026-08-25 pela caça com agentes (notas/caca-bugs-2026-08-25.md).
Todos matavam o `pool` sem mensagem nenhuma; um deles (`zfill` com largura de
64 bits) comia a RAM até o OOM killer levar a sessão inteira da máquina junto.

O teste roda o binário de VERDADE em subprocesso: crash aqui é código de saída
negativo/139/136, que `assert rc == 0` pega. Nada de importar a extensão — um
segfault derrubaria o pytest junto.
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

# VM em C NÃO se pula: sem o binário o teste FALHA (skip = falso verde).
assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."


def roda(tmp_path, fonte, arquivo="t.ps", timeout=20):
    ps = tmp_path / arquivo
    ps.write_text(fonte, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run([str(POOL), str(ps)], capture_output=True, text=True,
                          env=env, timeout=timeout)


def sem_crash(r):
    """rc negativo (sinal) ou 139/136 = o processo MORREU, não errou."""
    assert r.returncode not in (139, 136, -11, -8), (
        f"a VM caiu (rc={r.returncode}) — stderr: {r.stderr[:200]}")


@pytest.mark.parametrize("fonte,esperado", [
    ("l = [1, 2]\nl.append(l)\npost(l)\n", "[1, 2, [...]]"),
    ('d = { "a": 1 }\nd["eu"] = d\npost(d)\n', "{'a': 1, 'eu': {...}}"),
    ("t = [1]\nl = [t]\nt.append(l)\npost(t)\n", "[1, [[...]]]"),
])
def test_imprimir_estrutura_ciclica(tmp_path, fonte, esperado):
    """Antes: SEGFAULT (a recursão da impressão descia até estourar a pilha do
    C). Agora o container que já está sendo impresso vira `[...]`, como no
    Python — sem crash e sem saída gigante."""
    r = roda(tmp_path, fonte)
    sem_crash(r)
    assert r.returncode == 0, r.stderr
    assert r.stdout.strip() == esperado, r.stdout


def test_comparar_estruturas_mutuamente_recursivas(tmp_path):
    """Antes: SEGFAULT em `==` (a e b se contêm). Agora tem teto de
    profundidade: responde, não morre."""
    r = roda(tmp_path, "a = [1]\nb = [1]\na.append(b)\nb.append(a)\n"
                       "post(a == b)\npost(a.contains(b))\n")
    sem_crash(r)
    assert r.returncode == 0, r.stderr
    assert len(r.stdout.split()) == 2


def test_modulo_int64_min_por_menos_um(tmp_path):
    """Antes: SIGFPE (core dumped) — `INT64_MIN % -1` é UB no C. O resto é 0."""
    r = roda(tmp_path, "post(-9223372036854775808 % -1)\n")
    sem_crash(r)
    assert r.returncode == 0, r.stderr
    assert r.stdout.strip() == "0"


@pytest.mark.parametrize("chamada", [
    'post("a".zfill(9223372036854775807))',
    'post("a".ljust(9223372036854775807))',
    'post("a".rjust(9223372036854775807))',
    'post("a".center(9223372036854775807))',
])
def test_largura_absurda_erra_em_vez_de_comer_a_ram(tmp_path, chamada):
    """Antes: a VM saía alocando em laço e o OOM killer derrubava a SESSÃO.
    Agora recusa na hora com MemoryError (teto de 256 MB)."""
    r = roda(tmp_path, chamada + "\n", timeout=15)
    sem_crash(r)
    assert r.returncode != 0, "deveria recusar a largura"
    assert "MemoryError" in r.stderr, r.stderr


def test_largura_normal_continua_funcionando(tmp_path):
    """O teto não pode ter pegado uso legítimo."""
    r = roda(tmp_path, 'post("a".zfill(5))\npost("ab".ljust(5, "-"))\n'
                       'post("ab".center(6, "."))\npost("7".rjust(3, "0"))\n')
    assert r.returncode == 0, r.stderr
    assert r.stdout.split("\n")[:4] == ["0000a", "ab---", "..ab..", "007"]


def test_import_de_modulo_ps_dentro_de_async(tmp_path):
    """Antes: SEGFAULT. O `import` realoca `vm->protos` e a fibra principal
    seguia com o ponteiro velho da função em execução."""
    (tmp_path / "mymod.ps").write_text(
        'action saudar(nome) { return "ola " + nome }\nstr VALOR = "1.0"\n',
        encoding="utf-8")
    r = roda(tmp_path, "async action f() {\n    import mymod\n"
                       "    return mymod.saudar(\"ana\")\n}\npost(await f())\n")
    sem_crash(r)
    assert r.returncode == 0, r.stderr
    assert r.stdout.strip() == "ola ana"
