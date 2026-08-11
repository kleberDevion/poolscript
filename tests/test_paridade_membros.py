"""Cobertura de PARIDADE de membros: todo membro de toda classe de lib que o
interpretador (AUTORIDADE) expõe TEM que existir no VM em C — senão
`obj.membro` funciona num motor e explode no outro.

Foi assim que o `cursor.rowcount` escapou: existia no interp, faltava no VM,
e nenhum teste exercitava esse membro. Este teste fecha a categoria inteira:
introspecta as classes reais da stdlib e cruza com o que o `poolscript_vm.c`
expõe (tabelas METODOS_* + propriedades sem-parêntese no OP_GET_MEMBER).

Estático de propósito: não precisa de banco/rede/instância viva — compara o
CONJUNTO de membros dos dois motores direto da fonte. Um membro novo numa
classe sem o par no VM quebra aqui na hora.
"""
import inspect
import re
from pathlib import Path

import pytest

from poolscript.stdlib import _LAZY_LOADERS, _load  # noqa: E402

RAIZ = Path(__file__).resolve().parent.parent
VM_C = RAIZ / "vm" / "poolscript_vm.c"


# ── membros que o VM expõe (fonte C) ─────────────────────────────────────────

def _membros_do_vm() -> set:
    """Todo nome de membro exposto pelo VM, de duas fontes no
    poolscript_vm.c:
      1. entradas das tabelas METODOS_* :  { "nome", met_..., ... }
      2. propriedades sem-parêntese no OP_GET_MEMBER: strcmp(nome, "X") == 0
    É o vocabulário completo de membros que `obj.X` resolve no VM."""
    src = VM_C.read_text(encoding="utf-8")
    nomes = set()
    # 1. entradas de tabela de método: { "fetchall", met_...,
    for m in re.finditer(r'\{\s*"([A-Za-z_]\w*)"\s*,\s*met_', src):
        nomes.add(m.group(1))
    # 2. propriedades: strcmp(nome, "rowcount") == 0
    for m in re.finditer(r'strcmp\(\s*nome\s*,\s*"([A-Za-z_]\w*)"\s*\)\s*==\s*0', src):
        nomes.add(m.group(1))
    return nomes


# ── membros que o interp expõe por classe (introspecção) ─────────────────────

def _classes_do_interp() -> dict:
    """{NomeClasse: {membros}} de toda classe definida nos módulos da stdlib —
    exportada ou não (Response, DbCursor... aparecem em cadeias)."""
    import importlib
    classes = {}
    for alias, module_name in _LAZY_LOADERS.items():
        try:
            _load(alias)
            mod = importlib.import_module(f"poolscript.stdlib.{module_name}")
        except Exception:
            continue
        for cnome, cls in inspect.getmembers(mod, inspect.isclass):
            if cls.__module__ != mod.__name__ or cnome.startswith("_"):
                continue
            membros = set()
            for mnome, val in inspect.getmembers(cls):
                if mnome.startswith("_"):
                    continue
                if isinstance(val, property) or inspect.isfunction(val) or inspect.ismethod(val):
                    membros.add(mnome)
            if membros:
                classes.setdefault(cnome, set()).update(membros)
    return classes


# classes cujo espelho no VM é OUTRA coisa (não um objeto nativo com métodos),
# ou que são puramente do interpretador — ficam fora da checagem de paridade.
_FORA = {
    # jinker: objetos de servidor com API muito específica por motor; a
    # paridade deles é coberta pelos testes de jinker end-to-end
    "Jinker", "JinkerResponse", "JinkerRequest", "RequestProxy", "CorsConfig",
    "Route", "MiddlewareRegistrar", "SocketNamespace", "SocketEmitter",
    "ChannelManager", "ChannelStatus", "PoolIp", "PoolFileUpload",
    "_RouteRegistrar", "_SocketRegistrar",
    # meta/infra do interp sem contraparte de runtime
    "DataEntityMeta", "PoolModel", "FileHandle",
}

# membros que NÃO viram `obj.membro` no VM por design (helpers internos do
# wrapper Python que não fazem parte da API da linguagem).
_MEMBROS_IGNORADOS = {
    "connection",  # atributo interno do cursor no interp
}


def _classes_a_checar():
    interp = _classes_do_interp()
    return sorted((c, m) for c, m in interp.items() if c not in _FORA)


@pytest.mark.parametrize("classe,membros_interp", _classes_a_checar())
def test_membros_do_interp_existem_no_vm(classe, membros_interp):
    """Todo membro que o interp expõe numa classe de lib existe no VM."""
    vm = _membros_do_vm()
    faltando = {m for m in membros_interp
                if m not in vm and m not in _MEMBROS_IGNORADOS}
    assert not faltando, (
        f"classe {classe}: membros no interp mas AUSENTES no VM "
        f"(obj.X quebra no `pool`): {sorted(faltando)}"
    )


def test_rowcount_esta_coberto():
    """Guard do caso que originou o teste — rowcount tem que estar no VM."""
    assert "rowcount" in _membros_do_vm(), "rowcount sumiu do VM de novo"
