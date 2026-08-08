#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Gera metadata.json — o "modelo de tipos" que faz o autocomplete ser
type-aware de verdade (estilo Pylance), introspectando a stdlib REAL.

Ideia central: nada é escrito à mão. Para cada módulo da linguagem
(`_LAZY_LOADERS`) a gente:
  1. lê o EXPORTS (as funções/objetos que o `import x` expõe);
  2. para cada função, captura os PARÂMETROS (nome + se tem default) e o
     TIPO DE RETORNO resolvido pra um nome de classe (via type hint);
  3. varre o módulo atrás de CLASSES (mesmo que não estejam no EXPORTS, ex:
     psodbc.DbConnection/DbCursor) e captura os métodos/propriedades delas,
     também com params e returns.

Com isso o editor sabe a CADEIA:
    conn = psodbc.connect(...)   -> DbConnection
    cur  = conn.cursor()         -> DbCursor
    cur.<TAB>                    -> fetchall/fetchone/fetchmany/rowcount/...

Rode:  PYTHONPATH=src python3 psl-poolscript-vsix/bridge/gen_metadata.py
"""
import inspect
import json
import sys
import typing
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(RAIZ / "src"))

from poolscript.stdlib import _LAZY_LOADERS, _load  # noqa: E402


# Tipos primitivos que o editor já entende (não são "classe" pra encadear,
# mas viram o rótulo de retorno).
_PRIMITIVOS = {
    "str": "str", "int": "int", "float": "flo", "flo": "flo", "bool": "bool",
    "list": "list", "dict": "dict", "tuple": "tup", "bytes": "bytes",
    "Any": None, "None": None, "NoneType": None,
}

# Funções cujo retorno é polimórfico/`Any` e o hint não ajuda — resolvidas à mão
# (é o MENOR mal: escrito uma vez, explícito, e o teste pega se a classe sumir).
# connect(driver="mongo") é tratado no extension.js; o default é DbConnection.
_OVERRIDE_RETORNO = {
    ("psodbc", "connect"): "DbConnection",
    ("db", "connect"): "DbConnection",
}

# classes coletadas (nome -> {members:[...]}) — preenchido sob demanda conforme
# aparecem como tipo de retorno ou definidas dentro de um módulo de lib.
_CLASSES: dict[str, dict] = {}
_MODULOS_VISTOS: set = set()


def _nome_tipo(anot):
    """Resolve uma anotação de tipo pra um rótulo: nome de classe (pra encadear)
    ou primitivo ('str'/'list'/...). Devolve None quando não dá pra saber."""
    if anot is inspect.Signature.empty or anot is None:
        return None
    # forward ref em string: -> "MongoCollection"
    if isinstance(anot, str):
        base = anot.split("|")[0].strip().strip('"').strip("'")
        return _PRIMITIVOS.get(base, base) if base else None
    # typing.Optional[X] / X | None / typing.List etc.
    origem = typing.get_origin(anot)
    if origem is not None:
        # list[...]/dict[...] -> primitivo; Optional[X] -> X
        args = [a for a in typing.get_args(anot) if a is not type(None)]
        if origem in (list, typing.List):
            return "list"
        if origem in (dict, typing.Dict):
            return "dict"
        if len(args) == 1:
            return _nome_tipo(args[0])
        return None
    if isinstance(anot, type):
        nome = anot.__name__
        if nome in _PRIMITIVOS:
            return _PRIMITIVOS[nome]
        return nome
    return None


def _doc(obj):
    """Primeiro parágrafo do docstring — vira a descrição rica no hover."""
    d = inspect.getdoc(obj)
    if not d:
        return None
    # primeiro parágrafo (até linha em branco), limpo
    linhas = []
    for ln in d.splitlines():
        if not ln.strip() and linhas:
            break
        linhas.append(ln.rstrip())
    txt = " ".join(l.strip() for l in linhas if l.strip())
    return txt[:400] if txt else None


def _params(fn):
    """Lista de parâmetros nomeáveis de uma função/método: nome + se é opcional
    (tem default). Pula self/cls e *args/**kwargs (não são nomeáveis)."""
    try:
        sig = inspect.signature(fn)
    except (ValueError, TypeError):
        return []
    out = []
    for nome, p in sig.parameters.items():
        if nome in ("self", "cls"):
            continue
        if p.kind in (p.VAR_POSITIONAL, p.VAR_KEYWORD):
            continue
        out.append({"name": nome, "opt": p.default is not inspect.Parameter.empty})
    return out


def _assinatura(nome, fn):
    """String de assinatura pro hover: nome(param, param=..., ...)."""
    partes = []
    for p in _params(fn):
        partes.append(p["name"] + ("?" if p["opt"] else ""))
    return f"{nome}({', '.join(partes)})"


def _classe(cls):
    """Introspecta uma classe: métodos (com params+returns) e @property.
    Registra em _CLASSES e retorna o nome. Recursivo nos returns."""
    nome = cls.__name__
    if nome in _CLASSES:
        return nome
    _CLASSES[nome] = {"members": []}  # marca antes de recursar (evita loop)
    membros = []
    for mnome, val in inspect.getmembers(cls):
        if mnome.startswith("_"):
            continue
        if isinstance(val, property):
            ret = _nome_tipo(val.fget.__annotations__.get("return")) if val.fget else None
            membros.append({"name": mnome, "kind": "property", "returns": ret,
                            "doc": _doc(val.fget) if val.fget else None})
            if ret and ret not in _PRIMITIVOS.values() and ret not in _CLASSES:
                _talvez_classe_por_nome(cls, ret)
        elif inspect.isfunction(val) or inspect.ismethod(val):
            ret = _nome_tipo(inspect.signature(val).return_annotation)
            membros.append({
                "name": mnome, "kind": "method",
                "params": _params(val),
                "sig": _assinatura(mnome, val),
                "returns": ret,
                "doc": _doc(val),
            })
            if ret and ret not in _PRIMITIVOS.values():
                _talvez_classe_por_nome(cls, ret)
    _CLASSES[nome] = {"members": membros, "doc": _doc(cls)}
    return nome


def _talvez_classe_por_nome(cls_contexto, nome_tipo):
    """Acha uma classe pelo nome no módulo onde `cls_contexto` vive e a
    introspecta — resolve forward refs (-> "MongoCollection") e returns de
    classes definidas no mesmo arquivo."""
    if nome_tipo in _CLASSES:
        return
    mod = sys.modules.get(cls_contexto.__module__)
    if mod and hasattr(mod, nome_tipo):
        alvo = getattr(mod, nome_tipo)
        if inspect.isclass(alvo):
            _classe(alvo)


def _introspecta_modulo(alias, module_name):
    """Um módulo de lib: membros do EXPORTS (funções/classes) + classes soltas
    definidas no arquivo (ex: DbConnection, que não está no EXPORTS)."""
    try:
        exports = _load(alias)
    except Exception as e:  # dependência opcional ausente, etc.
        print(f"[aviso] pulei '{alias}': {e}", file=sys.stderr)
        return None

    membros = []
    for nome, val in exports.items():
        if nome.startswith("__"):
            continue
        if inspect.isclass(val):
            membros.append({"name": nome, "kind": "class", "returns": _classe(val),
                            "doc": _doc(val)})
        elif callable(val):
            ret = _nome_tipo(inspect.signature(val).return_annotation) \
                if _tem_sig(val) else None
            ov = _OVERRIDE_RETORNO.get((alias, nome))
            if ov:
                ret = ov
            if ret and ret not in _PRIMITIVOS.values():
                _talvez_classe_por_nome_mod(module_name, ret)
            membros.append({
                "name": nome, "kind": "function",
                "params": _params(val),
                "sig": _assinatura(nome, val),
                "returns": ret,
                "doc": _doc(val),
            })
        else:
            membros.append({"name": nome, "kind": "value"})

    # classes soltas do arquivo (não exportadas) — pra cadeia de returns
    import importlib
    try:
        mod = importlib.import_module(f"poolscript.stdlib.{module_name}")
    except Exception:
        mod = None
    if mod:
        for cnome, val in inspect.getmembers(mod, inspect.isclass):
            if val.__module__ == mod.__name__ and not cnome.startswith("_"):
                _classe(val)
    return membros


def _tem_sig(fn):
    try:
        inspect.signature(fn)
        return True
    except (ValueError, TypeError):
        return False


def _talvez_classe_por_nome_mod(module_name, nome_tipo):
    if nome_tipo in _CLASSES:
        return
    import importlib
    try:
        mod = importlib.import_module(f"poolscript.stdlib.{module_name}")
    except Exception:
        return
    if hasattr(mod, nome_tipo) and inspect.isclass(getattr(mod, nome_tipo)):
        _classe(getattr(mod, nome_tipo))


def main():
    modulos = {}
    # agrupa aliases pelo módulo real, mas emite uma entrada por NOME que o
    # usuário digita (json E JSON, psodbc E db, request E requests...).
    for alias, module_name in _LAZY_LOADERS.items():
        membros = _introspecta_modulo(alias, module_name)
        if membros is not None:
            modulos[alias] = {"members": membros}

    # docs ricas de keywords/tipos/builtins (mesma fonte da doc viva) — pro
    # hover ser bonito (assinatura + resumo + exemplo), não texto cinza.
    kw, tipos, builtins = {}, {}, {}
    try:
        sys.path.insert(0, str(RAIZ / "scripts"))
        from doc_specs_keywords import KEYWORDS_DOC, TYPES_DOC  # noqa: E402
        from doc_specs_builtins import BUILTINS                 # noqa: E402
        _pega = lambda d: {"sig": d.get("sig", ""), "resumo": d.get("resumo", ""),
                           "ex": d.get("ex", [])}
        kw = {n: _pega(s) for n, s in KEYWORDS_DOC.items()}
        tipos = {n: _pega(s) for n, s in TYPES_DOC.items()}
        builtins = {n: _pega(s) for n, s in BUILTINS.items()}
    except Exception as e:
        print(f"[aviso] docs de keywords/builtins não carregaram: {e}", file=sys.stderr)

    out = {
        "modules": modulos,
        "classes": _CLASSES,
        "keywords": kw,
        "types": tipos,
        "builtins": builtins,
    }
    destino = Path(__file__).resolve().parent / "metadata.json"
    destino.write_text(json.dumps(out, ensure_ascii=False, indent=1), encoding="utf-8")
    nfun = sum(len(m["members"]) for m in modulos.values())
    print(f"metadata.json: {len(modulos)} módulos, {nfun} membros, {len(_CLASSES)} classes, "
          f"{len(kw)} keywords, {len(builtins)} builtins")


if __name__ == "__main__":
    main()
