# -*- coding: utf-8 -*-
"""Modelo de tipos VIVO do LSP — introspecta a stdlib real em memória.

Mesma ideia do bridge/gen_metadata.py da extensão VS Code (que gera um JSON
estático), só que aqui o modelo nasce dentro do servidor, direto das classes
e assinaturas reais — não tem como ficar defasado da stdlib instalada.

O que sai daqui:
  modulos[alias][nome]  -> membro (function/class/value) com params, sig,
                           returns (nome de classe pra encadear) e doc
  classes[Nome].members -> métodos/propriedades da classe, também com returns
  keywords/tipos/builtins -> docs ricas (sig + resumo + exemplos) das mesmas
                           specs da doc viva (scripts/doc_specs_*.py)
"""
from __future__ import annotations

import inspect
import re
import sys
import typing
from pathlib import Path

from ..stdlib import _LAZY_LOADERS, _load

_PRIMITIVOS = {
    "str": "str", "int": "int", "float": "flo", "flo": "flo", "bool": "bool",
    "list": "list", "dict": "dict", "tuple": "tup", "bytes": "bytes",
    "Any": None, "None": None, "NoneType": None,
}

# retornos polimórficos que o hint não resolve — mesma tabela do gen_metadata
_OVERRIDE_RETORNO = {
    ("psodbc", "connect"): "DbConnection",
    ("db", "connect"): "DbConnection",
}

_RE_SELF_ATTR = re.compile(r"self\.([A-Za-z_]\w*)\s*[:=]")


def _nome_tipo(anot):
    if anot is inspect.Signature.empty or anot is None:
        return None
    if isinstance(anot, str):
        base = anot.split("|")[0].strip().strip('"').strip("'")
        return _PRIMITIVOS.get(base, base) if base else None
    origem = typing.get_origin(anot)
    if origem is not None:
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
        return _PRIMITIVOS.get(nome, nome)
    return None


def _doc(obj):
    d = inspect.getdoc(obj)
    if not d:
        return None
    linhas = []
    for ln in d.splitlines():
        if not ln.strip() and linhas:
            break
        linhas.append(ln.rstrip())
    txt = " ".join(l.strip() for l in linhas if l.strip())
    return txt[:400] if txt else None


def _params(fn):
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
    partes = [p["name"] + ("?" if p["opt"] else "") for p in _params(fn)]
    return f"{nome}({', '.join(partes)})"


class ModeloTipos:
    """O cérebro de tipos: módulos, classes e docs ricas, indexados por nome."""

    def __init__(self):
        # alias -> {nome_membro -> info}
        self.modulos: dict[str, dict[str, dict]] = {}
        # NomeClasse -> {"members": {nome -> info}, "doc": str|None}
        self.classes: dict[str, dict] = {}
        self.keywords: dict[str, dict] = {}
        self.tipos: dict[str, dict] = {}
        self.builtins: dict[str, dict] = {}
        self._carrega_modulos()
        self._carrega_docs_ricas()

    # ── consulta ──────────────────────────────────────────────────────

    def membros_do_modulo(self, alias: str) -> dict[str, dict] | None:
        return self.modulos.get(alias)

    def membros_da_classe(self, nome: str) -> dict[str, dict] | None:
        info = self.classes.get(nome)
        return info["members"] if info else None

    def retorno_de(self, alias: str, funcao: str) -> str | None:
        m = self.modulos.get(alias, {}).get(funcao)
        return m.get("returns") if m else None

    # ── introspecção ──────────────────────────────────────────────────

    def _carrega_modulos(self):
        for alias, module_name in _LAZY_LOADERS.items():
            try:
                exports = _load(alias)
            except Exception:
                continue   # dependência opcional ausente — módulo fica fora
            membros: dict[str, dict] = {}
            for nome, val in exports.items():
                if nome.startswith("__"):
                    continue
                if inspect.isclass(val):
                    membros[nome] = {"kind": "class", "returns": self._classe(val),
                                     "doc": _doc(val)}
                elif callable(val):
                    ret = None
                    try:
                        ret = _nome_tipo(inspect.signature(val).return_annotation)
                    except (ValueError, TypeError):
                        pass
                    ret = _OVERRIDE_RETORNO.get((alias, nome), ret)
                    if ret and ret not in _PRIMITIVOS.values():
                        self._classe_por_nome(module_name, ret)
                    membros[nome] = {"kind": "function", "params": _params(val),
                                     "sig": _assinatura(nome, val), "returns": ret,
                                     "doc": _doc(val)}
                else:
                    membros[nome] = {"kind": "value"}
            self.modulos[alias] = membros

            # classes soltas do arquivo (não exportadas) — pra cadeia de returns
            mod = sys.modules.get(f"poolscript.stdlib.{module_name}")
            if mod:
                for cnome, val in inspect.getmembers(mod, inspect.isclass):
                    if val.__module__ == mod.__name__ and not cnome.startswith("_"):
                        self._classe(val)

    def _classe(self, cls) -> str:
        nome = cls.__name__
        if nome in self.classes:
            return nome
        self.classes[nome] = {"members": {}}   # marca antes de recursar
        membros: dict[str, dict] = {}
        for mnome, val in inspect.getmembers(cls):
            if mnome.startswith("_"):
                continue
            if isinstance(val, property):
                ret = None
                if val.fget:
                    ret = _nome_tipo(val.fget.__annotations__.get("return"))
                membros[mnome] = {"kind": "property", "returns": ret,
                                  "doc": _doc(val.fget) if val.fget else None}
                if ret and ret not in _PRIMITIVOS.values() and ret not in self.classes:
                    self._classe_vizinha(cls, ret)
            elif inspect.isfunction(val) or inspect.ismethod(val):
                ret = None
                try:
                    ret = _nome_tipo(inspect.signature(val).return_annotation)
                except (ValueError, TypeError):
                    pass
                membros[mnome] = {"kind": "method", "params": _params(val),
                                  "sig": _assinatura(mnome, val), "returns": ret,
                                  "doc": _doc(val)}
                if ret and ret not in _PRIMITIVOS.values():
                    self._classe_vizinha(cls, ret)
        # atributos de instância (self.x no __init__) — PoolFile.name/ext/size
        try:
            src = inspect.getsource(cls.__init__)
            for a in _RE_SELF_ATTR.findall(src):
                if not a.startswith("_") and a not in membros:
                    membros[a] = {"kind": "property"}
        except (OSError, TypeError):
            pass
        self.classes[nome] = {"members": membros, "doc": _doc(cls)}
        return nome

    def _classe_vizinha(self, cls_contexto, nome_tipo):
        if nome_tipo in self.classes:
            return
        mod = sys.modules.get(cls_contexto.__module__)
        if mod and hasattr(mod, nome_tipo) and inspect.isclass(getattr(mod, nome_tipo)):
            self._classe(getattr(mod, nome_tipo))

    def _classe_por_nome(self, module_name, nome_tipo):
        if nome_tipo in self.classes:
            return
        import importlib
        try:
            mod = importlib.import_module(f"poolscript.stdlib.{module_name}")
        except Exception:
            return
        if hasattr(mod, nome_tipo) and inspect.isclass(getattr(mod, nome_tipo)):
            self._classe(getattr(mod, nome_tipo))

    # ── docs ricas (keywords/tipos/builtins) ──────────────────────────

    def _carrega_docs_ricas(self):
        """As specs da doc viva moram em scripts/ na raiz do repositório.
        Instalado fora do repo, o servidor segue funcionando — só perde a doc
        rica de keyword/builtin no hover (o resto não depende disso)."""
        raiz = Path(__file__).resolve().parents[3]
        scripts = raiz / "scripts"
        if not (scripts / "doc_specs_keywords.py").is_file():
            return
        sys.path.insert(0, str(scripts))
        try:
            from doc_specs_keywords import KEYWORDS_DOC, TYPES_DOC   # noqa
            from doc_specs_builtins import BUILTINS                  # noqa
            pega = lambda d: {"sig": d.get("sig", ""), "resumo": d.get("resumo", ""),
                              "ex": d.get("ex", [])}
            self.keywords = {n: pega(s) for n, s in KEYWORDS_DOC.items()}
            self.tipos = {n: pega(s) for n, s in TYPES_DOC.items()}
            self.builtins = {n: pega(s) for n, s in BUILTINS.items()}
        except Exception:
            pass
        # métodos de STRING (specs da doc viva) viram a "classe" str — é o que
        # faz `nome.` sugerir upper/lower/split/replace... como qualquer tipo
        try:
            from doc_specs_string import STRMET                      # noqa
            membros = {}
            for nome, spec in STRMET.items():
                ret_txt = (spec.get("ret") or "").strip().lower()
                ret = next((t for t in ("str", "int", "flo", "bool", "list",
                                        "dict", "tup", "bytes")
                            if ret_txt.startswith(t)), None)
                params = [{"name": p, "opt": bool(d) and d not in ("—", "-")}
                          for (p, _t, d, _n) in spec.get("params", [])]
                membros[nome] = {"kind": "method", "params": params,
                                 "sig": spec.get("sig", f"{nome}()"),
                                 "returns": ret,
                                 "doc": spec.get("resumo") or None}
            if membros:
                self.classes["str"] = {"members": membros,
                                       "doc": "métodos de string da linguagem"}
        except Exception:
            pass
        finally:
            if sys.path and sys.path[0] == str(scripts):
                sys.path.pop(0)


_MODELO: ModeloTipos | None = None


def modelo() -> ModeloTipos:
    """Singleton — a introspecção roda uma vez por processo do servidor."""
    global _MODELO
    if _MODELO is None:
        _MODELO = ModeloTipos()
    return _MODELO
