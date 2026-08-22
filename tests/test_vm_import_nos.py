"""Suíte do VM em C (`pool`) — import de MÓDULO .ps definindo cada construto de
topo, rodando DIRETO no VM com saída esperada FIXA.

Foi aqui que morava o segfault: `anexa_programa` não copiava os descritores de
`model`/`enum` do módulo importado nem relocava OP_MAKE_MODEL/OP_MAKE_ENUM. Um
módulo importado com `model` fazia o VM ler `vm->model_nomes[idx]` com o array
NULL -> crash. O interpretador não tem esse caminho (globais/descritores são
objetos Python coletados), então testar só o interp não pegava.

Sem o binário `pool`, FALHA (não pula): skip silencioso dava falso verde.
"""
import subprocess
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"

# nome, {arquivo: fonte}, entrada, saída ESPERADA no VM
CASOS = [
    ("action",
     {"zzlib.ps": "reaction dobro(n) { return n * 2 }\n",
      "main.ps": "from zzlib import dobro\npost(dobro(21))\n"},
     "main.ps", "42\n"),
    ("class",
     {"zzlib.ps": "public class Contador() {\n    public reaction __init__(self, n) { self.n = n }\n    public reaction get(self) { return self.n }\n}\n",
      "main.ps": "from zzlib import Contador\npost(Contador(9).get())\n"},
     "main.ps", "9\n"),
    ("static",
     {"zzlib.ps": "public class Mat() {\n    @static\n    public reaction soma(self, a, b) { return a + b }\n}\n",
      "main.ps": "from zzlib import Mat\npost(Mat.soma(2, 3))\n"},
     "main.ps", "5\n"),
    ("heranca",
     {"zzlib.ps": "public class A() {\n    public reaction nome(self) { return \"A\" }\n}\npublic class B(A) {\n}\n",
      "main.ps": "from zzlib import B\npost(B().nome())\n"},
     "main.ps", "A\n"),
    # ── o caso do crash ───────────────────────────────────────────────────
    ("model",
     {"zzlib.ps": "model Usuario() {\n    nome: str\n    idade: int\n}\n",
      "main.ps": 'from zzlib import Usuario\npost({"nome": "a", "idade": 5} == Usuario)\npost({"nome": "a"} == Usuario)\n'},
     "main.ps", "True\nFalse\n"),
    ("enum",
     {"zzlib.ps": "enum Status {\n    ATIVO\n    INATIVO\n}\n",
      "main.ps": "from zzlib import Status\npost(Status.ATIVO, Status.INATIVO)\n"},
     "main.ps", "0 1\n"),
    ("model_e_enum",
     {"zzlib.ps": "model Produto() {\n    nome: str\n    preco: int\n}\nenum Cor {\n    R = 10,\n    G,\n    B\n}\n",
      "main.ps": 'from zzlib import Produto, Cor\npost({"nome": "x", "preco": 3} == Produto)\npost(Cor.R, Cor.G, Cor.B)\n'},
     "main.ps", "True\n10 11 12\n"),
    # ── relocação de base ≠ 0: o main JÁ tem model/enum/class próprios ─────
    ("base_nao_zero",
     {"zzlib.ps": "model Produto() {\n    nome: str\n}\nenum Status {\n    ON\n    OFF\n}\npublic class Helper() {\n    public reaction oi(self) { return \"lib\" }\n}\n",
      "main.ps": 'model Cliente() {\n    id: int\n}\nenum Local {\n    A\n    B\n}\npublic class Meu() {\n    public reaction oi(self) { return \"main\" }\n}\nfrom zzlib import Produto, Status, Helper\npost({"id": 1} == Cliente)\npost({"nome": "x"} == Produto)\npost(Local.A, Status.ON, Status.OFF)\npost(Meu().oi(), Helper().oi())\n'},
     "main.ps", "True\nTrue\n0 0 1\nmain lib\n"),
    # ── vários models/enums no mesmo módulo (relocação intra-módulo) ───────
    ("varios_no_modulo",
     {"zzlib.ps": "model M1() {\n    a: int\n}\nmodel M2() {\n    b: str\n}\nmodel M3() {\n    c: bool\n}\nenum E1 {\n    X\n    Y\n}\nenum E2 {\n    P = 5,\n    Q\n}\n",
      "main.ps": 'from zzlib import M1, M2, M3, E1, E2\npost({"a": 1} == M1, {"b": "x"} == M2, {"c": true} == M3)\npost(E1.X, E1.Y, E2.P, E2.Q)\n'},
     "main.ps", "True True True\n0 1 5 6\n"),
    # ── import ANINHADO: libA importa libB (que tem model) ────────────────
    ("aninhado",
     {"zzb.ps": "model Interno() {\n    v: int\n}\n",
      "zza.ps": 'from zzb import Interno\nreaction checa(d) { return d == Interno }\n',
      "main.ps": 'from zza import checa\npost(checa({"v": 7}))\npost(checa({"v": "x"}))\n'},
     "main.ps", "True\nFalse\n"),
]


def _escreve(tmp_path, arquivos):
    for nome, fonte in arquivos.items():
        (tmp_path / nome).write_text(fonte, encoding="utf-8")


@pytest.mark.parametrize("nome,arquivos,entry,esperado", CASOS, ids=[c[0] for c in CASOS])
def test_import_no_vm(tmp_path, nome, arquivos, entry, esperado):
    # SEM skip: pool ausente -> FALHA (não pula). Ver test_vm_nos._vm.
    assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."
    _escreve(tmp_path, arquivos)
    r = subprocess.run([str(POOL), str(tmp_path / entry)], capture_output=True,
                       text=True, timeout=30)
    assert r.returncode == 0, (nome, "VM crashou/erro:", r.stdout + r.stderr)
    assert r.stdout == esperado, (nome, "VM:", repr(r.stdout), "esperado:", repr(esperado))
