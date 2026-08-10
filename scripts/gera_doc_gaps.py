#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Auditoria código × doc + gerador do que falta.

Introspecta a stdlib REAL (mesma fonte do LSP/metadata) e cruza com docs/:
todo membro de lib sem página ganha uma — com assinatura, parâmetros,
docstring COMPLETA e exemplo. Classes ganham/completam a própria página.

    PYTHONPATH=src python3 scripts/gera_doc_gaps.py           # audita e gera
    PYTHONPATH=src python3 scripts/gera_doc_gaps.py --so-audita
"""
from __future__ import annotations

import inspect
import re
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(RAIZ / "src"))
DOCS = RAIZ / "docs"

from poolscript.stdlib import _LAZY_LOADERS, _load  # noqa: E402


def doc_completa(obj) -> str:
    return inspect.getdoc(obj) or ""


def params_de(fn):
    try:
        sig = inspect.signature(fn)
    except (ValueError, TypeError):
        return []
    out = []
    for nome, p in sig.parameters.items():
        if nome in ("self", "cls") or p.kind in (p.VAR_POSITIONAL, p.VAR_KEYWORD):
            continue
        default = None if p.default is inspect.Parameter.empty else repr(p.default)
        out.append((nome, default))
    return out


def assinatura(alias, nome, fn, prefixo=""):
    partes = []
    for pnome, default in params_de(fn):
        partes.append(f"{pnome}={default}" if default is not None else pnome)
    dono = f"{alias}." if alias else prefixo
    return f"{dono}{nome}({', '.join(partes)})"


def exemplo_do_docstring(doc: str) -> str | None:
    """Pega o primeiro bloco de código do docstring (linhas indentadas com
    cara de chamada) — exemplo REAL escrito por quem fez a função."""
    linhas = doc.splitlines()
    bloco = []
    for ln in linhas:
        if re.match(r"\s{4,}\S", ln) and ("(" in ln or "=" in ln):
            bloco.append(ln.strip())
        elif bloco:
            break
    return "\n".join(bloco) if bloco else None


def pagina_funcao(alias, nome, fn, indice_rel):
    doc = doc_completa(fn)
    sig = assinatura(alias, nome, fn)
    md = [MARCADOR, f"# `{sig}`", ""]
    if doc:
        md += [doc, ""]
    ps = params_de(fn)
    if ps:
        md += ["## Parâmetros", "", "| nome | default |", "|---|---|"]
        for pnome, default in ps:
            md.append(f"| `{pnome}` | {default if default is not None else '(obrigatório)'} |")
        md.append("")
    ex = exemplo_do_docstring(doc)
    md += ["## Exemplo", "", "```"]
    if ex:
        md.append(ex)
    else:
        chamada = ", ".join(p for p, d in ps if d is None) or ""
        md.append(f"import {alias}")
        md.append(f"r = {alias}.{nome}({chamada})")
        md.append("post(r)")
    md += ["```", "", f"[← índice]({indice_rel})", ""]
    return "\n".join(md)


MARCADOR = "<!-- gerado: gera_doc_gaps.py — pode regenerar -->"


def pagina_classe(alias, cnome, cls, indice_rel):
    doc = doc_completa(cls)
    md = [MARCADOR, f"# `{cnome}`", "",
          f"> **Objeto interno da linguagem** — você não cria `{cnome}` na mão:",
          f"> é o TIPO de um objeto que a lib `{alias}` te entrega pronto.",
          f"> Confira com `type(obj)`, que mostra exatamente este nome.",
          ""]
    if doc:
        md += [doc, ""]
    md += ["## Métodos e propriedades", "", "| Acesso | O que faz |", "|---|---|"]
    corpo = []
    for mnome, val in sorted(inspect.getmembers(cls)):
        if mnome.startswith("_"):
            continue
        if isinstance(val, property):
            resumo = (doc_completa(val.fget).splitlines() or [""])[0] if val.fget else ""
            md.append(f"| `.{mnome}` | {resumo} |")
        elif inspect.isfunction(val) or inspect.ismethod(val):
            partes = []
            for pnome, default in params_de(val):
                partes.append(f"{pnome}={default}" if default is not None else pnome)
            resumo = (doc_completa(val).splitlines() or [""])[0]
            md.append(f"| `.{mnome}({', '.join(partes)})` | {resumo} |")
            dcompleto = doc_completa(val)
            if dcompleto and "\n" in dcompleto:
                corpo.append(f"### `.{mnome}(...)`\n\n{dcompleto}\n")
    # atributos de instância (self.x no __init__)
    try:
        src = inspect.getsource(cls.__init__)
        for a in sorted(set(re.findall(r"self\.([A-Za-z_]\w*)\s*[:=]", src))):
            if not a.startswith("_"):
                md.append(f"| `.{a}` | atributo |")
    except (OSError, TypeError):
        pass
    md.append("")
    md += corpo
    md += [f"[← índice]({indice_rel})", ""]
    return "\n".join(md)


OBJ_DIR = DOCS / "objetos-internos"


def _paginas_a_mao() -> dict:
    """Mapa Nome -> caminho relativo (a partir de OBJ_DIR) das páginas de
    classe escritas À MÃO (sem o marcador do gerador). O índice único aponta
    pra elas em vez de duplicar com uma versão fina."""
    a_mao = {}
    for md in DOCS.rglob("*.md"):
        if OBJ_DIR in md.parents:
            continue
        nome = md.stem
        if not (nome[:1].isupper()):
            continue
        try:
            if MARCADOR in md.read_text(encoding="utf-8"):
                continue
        except OSError:
            continue
        import os as _os
        a_mao[nome] = _os.path.relpath(md, OBJ_DIR)
    return a_mao


def main():
    so_audita = "--so-audita" in sys.argv
    vistos_modulos = set()
    gaps_fn, gaps_cls, geradas = [], [], 0
    catalogo = []   # (Nome, alias, destino_rel) de cada objeto interno
    a_mao = _paginas_a_mao()

    for alias, module_name in _LAZY_LOADERS.items():
        if module_name in vistos_modulos:
            continue           # JSON/json, db/psodbc... uma página só
        dir_lib = DOCS / alias
        if not dir_lib.is_dir():
            outro = next((a for a, m in _LAZY_LOADERS.items()
                          if m == module_name and (DOCS / a).is_dir()), None)
            if outro:
                dir_lib = DOCS / outro
                alias = outro
            else:
                dir_lib.mkdir(parents=True, exist_ok=True)
        vistos_modulos.add(module_name)
        try:
            exports = _load(alias)
        except Exception:
            continue

        indice = f"../{alias}.md"
        for nome, val in sorted(exports.items()):
            if nome.startswith("_"):
                continue
            destino = dir_lib / nome / f"{nome}.md"
            if destino.is_file() and MARCADOR not in destino.read_text(encoding="utf-8"):
                continue   # página escrita à mão: nunca sobrescreve
            if destino.is_file() and so_audita:
                continue
            if inspect.isclass(val):
                if nome in a_mao:
                    catalogo.append((nome, alias, a_mao[nome]))   # já tem à mão
                    continue
                gaps_cls.append(f"{alias}.{nome}")
                catalogo.append((nome, alias, f"{nome}.md"))
                if not so_audita:
                    OBJ_DIR.mkdir(parents=True, exist_ok=True)
                    (OBJ_DIR / f"{nome}.md").write_text(
                        pagina_classe(alias, nome, val, "objetos-internos.md"),
                        encoding="utf-8")
                    geradas += 1
                continue
            elif callable(val):
                gaps_fn.append(f"{alias}.{nome}")
                if not so_audita:
                    destino.parent.mkdir(parents=True, exist_ok=True)
                    destino.write_text(pagina_funcao(alias, nome, val, indice),
                                       encoding="utf-8")
                    geradas += 1

        # classes internas (não exportadas) que aparecem em cadeias: Response,
        # DbConnection, DbCursor, JinkerRequest...
        mod = sys.modules.get(f"poolscript.stdlib.{module_name}")
        if mod:
            for cnome, val in inspect.getmembers(mod, inspect.isclass):
                if val.__module__ != mod.__name__ or cnome.startswith("_"):
                    continue
                if cnome in a_mao:
                    catalogo.append((cnome, alias, a_mao[cnome]))
                    continue
                destino = OBJ_DIR / f"{cnome}.md"
                if destino.is_file() and so_audita:
                    continue
                gaps_cls.append(f"{alias}.{cnome}")
                catalogo.append((cnome, alias, f"{cnome}.md"))
                if not so_audita:
                    OBJ_DIR.mkdir(parents=True, exist_ok=True)
                    destino.write_text(pagina_classe(alias, cnome, val,
                                                     "objetos-internos.md"),
                                       encoding="utf-8")
                    geradas += 1

    # índice único de todos os objetos internos
    if not so_audita and catalogo:
        OBJ_DIR.mkdir(parents=True, exist_ok=True)
        vistos_cat = {}
        for nome, alias, destino_rel in catalogo:
            vistos_cat.setdefault(nome, (alias, destino_rel))
        md = [MARCADOR, "# Objetos internos da linguagem", "",
              "Tipos que você **não cria na mão** — cada um é o que uma lib te",
              "entrega pronto (o `type(obj)` mostra esse nome). Um índice só,",
              "todos juntos.", "",
              "| Objeto | Vem da lib |", "|---|---|"]
        for nome in sorted(vistos_cat):
            alias, destino_rel = vistos_cat[nome]
            md.append(f"| [`{nome}`]({destino_rel}) | `{alias}` |")
        md.append("")
        (OBJ_DIR / "objetos-internos.md").write_text("\n".join(md), encoding="utf-8")

    print(f"funções sem página: {len(gaps_fn)}")
    print(f"classes sem página: {len(gaps_cls)}")
    if gaps_fn:
        print("  fn:", ", ".join(gaps_fn[:40]), "..." if len(gaps_fn) > 40 else "")
    if gaps_cls:
        print("  cls:", ", ".join(gaps_cls[:40]), "..." if len(gaps_cls) > 40 else "")
    if not so_audita:
        print(f"páginas geradas: {geradas}")


if __name__ == "__main__":
    main()
