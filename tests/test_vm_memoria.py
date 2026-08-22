"""Suíte do VM em C (`pool`) — ESTRESSE DE MEMÓRIA por nó.

Esta é a parte que só faz sentido no VM em C: ele gerencia memória na mão
(malloc/free) com GC mark-and-sweep próprio, sem o coletor nem o GIL do Python.
Um use-after-free, double-free ou raiz perdida no GC não aparece no
interpretador — corrompe ou crasha só aqui. Cada caso cria/descarta MUITOS
objetos (forçando ciclos de GC) mantendo um acumulador vivo, e confere o
resultado exato: se o GC liberar algo vivo, ou o alocador corromper, o número sai
errado ou o processo cai (returncode != 0).

Sem o binário `pool`, FALHA (não pula).
"""
import subprocess
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"

# nó estressado, fonte, saída ESPERADA
CASOS = [
    # listas: 10k elementos, soma sobrevive ao GC
    ("listas",
     "int soma = 0\nlist L = []\nfor each i in range(10000) { addEnd(L, i) }\n"
     "for each x in L { soma = soma + x }\npost(soma)",
     "49995000\n"),
    # strings: 2k concatenações -> 2k strings descartadas (churn de alocação)
    ("strings_churn",
     'str s = ""\nfor each i in range(2000) { s = s + "ab" }\npost(len(s))',
     "4000\n"),
    # dict: 5k chaves (str) -> valor, lookup no fim
    ("dicts",
     'dict d = {}\nfor each i in range(5000) { d[str(i)] = i }\npost(len(d), d["4999"])',
     "5000 4999\n"),
    # instâncias: 5k objetos criados e descartados por iteração
    ("objetos_churn",
     "class Pt() { public reaction __init__(self, v) { self.v = v } }\n"
     "int soma = 0\nfor each i in range(5000) {\n    p = Pt(i)\n    soma = soma + p.v\n}\npost(soma)",
     "12497500\n"),
    # LIXO puro por iteração (list + dict descartados) com acumulador vivo:
    # se o GC coletar o acumulador por engano, o número sai errado.
    ("gc_churn",
     'int soma = 0\nfor each i in range(20000) {\n    lixo = [i, i + 1, i + 2]\n    tmp = {"a": i}\n    soma = soma + i\n}\npost(soma)',
     "199990000\n"),
    # gerador produzindo 10k valores (frame congelado + retomadas)
    ("geradores",
     "reaction conta(n) {\n    int i = 0\n    while (i < n) { yield i\n        i++ }\n}\n"
     "int soma = 0\nfor each v in conta(10000) { soma = soma + v }\npost(soma)",
     "49995000\n"),
    # recursão profunda: 1000 frames (pilha de frames/locais do VM)
    ("recursao_profunda",
     "reaction soma_ate(n) {\n    if (n == 0) { return 0 }\n    return n + soma_ate(n - 1)\n}\npost(soma_ate(1000))",
     "500500\n"),
    # bigint: 200 multiplicações -> mpz alocado/liberado a cada passo
    ("bigint_churn",
     "b = 1\nfor each i in range(200) { b = b * 2 }\npost(b)",
     "1606938044258990275541962092341162602522202993782792835301376\n"),
    # model: 5k dicts criados e comparados contra o descritor do model
    ("model_churn",
     'model M() { a: int }\nint ok = 0\nfor each i in range(5000) {\n    d = {"a": i}\n    if (d == M) { ok = ok + 1 }\n}\npost(ok)',
     "5000\n"),
]


@pytest.mark.parametrize("nome,src,esperado", CASOS, ids=[c[0] for c in CASOS])
def test_estresse_memoria_vm(tmp_path, nome, src, esperado):
    assert POOL.exists(), "binário 'pool' não compilado — rode ./rebuild_vm.sh."
    ps = tmp_path / "t.ps"
    ps.write_text(src, encoding="utf-8")
    r = subprocess.run([str(POOL), str(ps)], capture_output=True, text=True, timeout=60)
    assert r.returncode == 0, (nome, "VM crashou/corrompeu:", r.stdout + r.stderr)
    assert r.stdout == esperado, (nome, "VM:", repr(r.stdout), "esperado:", repr(esperado))
