#!/usr/bin/env python3
"""Introspecta a stdlib REAL e despeja as assinaturas num JSON — a única parte
da auditoria de doc que a PoolScript não alcança (inspect.signature do Python).
A varredura/reescrita das páginas é o scripts/audita_doc.ps.

    PYTHONPATH=src python3 scripts/audita_doc_sigs.py            # -> .audita_sigs.json
    ./pool scripts/audita_doc.ps                                  # relata
    APLICA=1 ./pool scripts/audita_doc.ps                         # reescreve os títulos
"""
import inspect
import json
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(RAIZ / "src"))
from poolscript.stdlib import _LAZY_LOADERS, _load   # noqa: E402


def lit(v):
    """default em literal da PoolScript (o que vai no título da página)."""
    if v is None:
        return "None"
    if v is True:
        return "true"
    if v is False:
        return "false"
    if isinstance(v, (int, float)):
        return str(v)
    if isinstance(v, str):
        s = (v.replace("\\", "\\\\").replace('"', '\\"')
              .replace("\n", "\\n").replace("\t", "\\t").replace("\r", "\\r"))
        return '"' + s + '"'
    if isinstance(v, (tuple, list, dict)) and not v:
        return {tuple: "()", list: "[]", dict: "{}"}[type(v)]
    return repr(v)


def params(fn):
    try:
        sig = inspect.signature(fn)
    except (ValueError, TypeError):
        return None
    out = []
    for nome, p in sig.parameters.items():
        if nome in ("self", "cls"):
            continue
        if p.kind in (p.VAR_POSITIONAL, p.VAR_KEYWORD):
            return None          # dinâmico — sem assinatura fixa
        out.append({"name": nome,
                    "default": None if p.default is inspect.Parameter.empty else lit(p.default),
                    "kw": p.kind is p.KEYWORD_ONLY})
    return out


classes = {}


def classe(cls):
    nome = cls.__name__
    if nome in classes:
        return nome
    membros = {}
    classes[nome] = membros
    for mnome, val in inspect.getmembers(cls):
        if mnome.startswith("_"):
            continue
        if isinstance(val, property):
            membros[mnome] = {"kind": "property"}
        elif inspect.isfunction(val) or inspect.ismethod(val):
            membros[mnome] = {"kind": "method", "params": params(val)}
    return nome


def membro_de(val):
    if inspect.isclass(val):
        return {"kind": "class", "classe": classe(val), "params": params(val.__init__)}
    if inspect.isroutine(val):
        info = {"kind": "function", "params": params(val)}
        if inspect.ismethod(val):          # método bound de uma instância (proxy)
            info["classe"] = classe(type(val.__self__))
        return info
    if callable(val):                      # instância chamável (cors, app.route)
        return {"kind": "function", "params": params(val.__call__),
                "classe": classe(type(val))}
    return {"kind": "value", "classe": classe(type(val))
            if type(val).__module__.startswith("poolscript") else None}


def main(saida):
    modulos = {}
    classes_do_modulo = {}
    for alias, module_name in _LAZY_LOADERS.items():
        try:
            exports = _load(alias)
        except Exception as e:
            print("pulei", alias, e, file=sys.stderr)
            continue
        membros = {}
        for nome, val in exports.items():
            if nome.startswith("__"):
                continue
            membros[nome] = membro_de(val)
        modulos[alias] = membros
        # classes soltas do módulo (Response, DbCursor...) — páginas
        # docs/<lib>/<Classe>/... e a cadeia de returns
        soltas = []
        mod = sys.modules.get(f"poolscript.stdlib.{module_name}")
        if mod:
            for cnome, val in inspect.getmembers(mod, inspect.isclass):
                if val.__module__ == mod.__name__ and not cnome.startswith("_"):
                    soltas.append(classe(val))
        classes_do_modulo[alias] = sorted(set(soltas) | {m["classe"] for m in membros.values()
                                                         if m.get("classe")})

    # jinker: route/socket/middleware/channel/static_* são ATRIBUTOS DE
    # INSTÂNCIA do Jinker (setados no __init__) — inspect na classe não vê.
    from poolscript.stdlib import jinker_lib as jk
    app = jk.Jinker("dump")
    for nome in ("route", "socket", "middleware", "channel", "static_folder",
                 "static_url", "run", "cors"):
        if hasattr(app, nome) and nome not in classes["Jinker"]:
            classes["Jinker"][nome] = membro_de(getattr(app, nome))

    # guzer: elementos são métodos dinâmicos (kwargs) — a assinatura real é a
    # lista _ATRIBUTOS, toda por nome (tag(typeinp=, placeholder=, ...))
    from poolscript.stdlib import guzer_lib as g
    attrs = [{"name": a, "default": None, "kw": True} for a in g._ATRIBUTOS]
    for tag in list(g._TAGS_BOX) + [g._TAG_DIALOG]:
        modulos["guzer"][tag] = {"kind": "function", "params": attrs}
    modulos["guzer"]["UI"] = {"kind": "class", "classe": "UI", "params": params(g.UI.__init__)}
    for nome in ("window", "button", "show", "getitemByIdentify"):
        modulos["guzer"][nome] = {"kind": "function", "params": params(getattr(g.UI, nome))}
    for nome in ("stylesheet", "text"):
        modulos["guzer"][nome] = {"kind": "function", "params": params(getattr(g._Widget, nome))}
    modulos["guzer"]["POOLHTMLElements"] = {"kind": "property"}
    modulos["guzer"]["value"] = {"kind": "property"}

    # Parsing: objeto GLOBAL (builtin), não é import — docs/Parsing/<m>/<m>.md
    from poolscript.stdlib.parsing_lib import Parsing
    modulos["Parsing"] = {n: {"kind": "function", "params": params(getattr(Parsing, n))}
                          for n, v in inspect.getmembers(Parsing, inspect.ismethod)
                          if not n.startswith("_")}
    classes_do_modulo["Parsing"] = []

    json.dump({"modulos": modulos, "classes": classes,
               "classes_do_modulo": classes_do_modulo},
              open(saida, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
    print(f"{saida}: {len(modulos)} módulos, {len(classes)} classes")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else str(RAIZ / ".audita_sigs.json"))
