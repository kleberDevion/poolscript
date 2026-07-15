#!/usr/bin/env python3
"""Gera bridge/stdlib_metadata.json a partir da stdlib REAL do poolscript.

Antes, a extensão mantinha uma cópia manual (STDLIB_MEMBERS, em extension.js)
do que cada lib exporta — toda vez que a stdlib mudava (renomear lib, tirar
função, mudar assinatura) e ninguém lembrava de atualizar a cópia, o
autocomplete ficava errado pra aquele módulo específico ("expõe módulo de um
jeito e de outro não"). Esse script introspecciona a stdlib de verdade
(poolscript.stdlib) e gera o metadata — sem cópia manual pra dessincronizar.

Rodar de novo sempre que a stdlib mudar (nova lib, nova função, rename):
    python bridge/gen_stdlib_metadata.py

Precisa do poolscript instalado no Python usado (pip install -e . na raiz
do repo poolscript-lang).
"""
from __future__ import annotations
import inspect
import json
import sys
from pathlib import Path

from poolscript.stdlib import _LAZY_LOADERS, _load  # noqa: E402


def _kind_of(value) -> str:
    if inspect.isclass(value):
        return "class"
    if inspect.isfunction(value) or inspect.ismethod(value) or inspect.isbuiltin(value):
        return "function"
    if callable(value):
        return "method"
    return "property"


def _signature_str(name: str, value) -> str | None:
    try:
        sig = inspect.signature(value)
    except (TypeError, ValueError):
        return None
    params = []
    for p in sig.parameters.values():
        if p.default is not inspect.Parameter.empty:
            params.append(f"{p.name}?")
        else:
            params.append(p.name)
    return f"{name}({', '.join(params)})"


def _detail_for(lib_name: str, member_name: str, value) -> str:
    sig = _signature_str(member_name, value)
    doc = inspect.getdoc(value)
    first_doc_line = doc.splitlines()[0].strip() if doc else None
    call_form = f"{lib_name}.{sig}" if sig else f"{lib_name}.{member_name}"
    if first_doc_line:
        return f"{call_form} — {first_doc_line}"
    return call_form


def build_metadata() -> dict:
    metadata: dict[str, list[dict]] = {}
    for lib_name in _LAZY_LOADERS:
        try:
            exports = _load(lib_name)
        except Exception as e:  # lib com dependência opcional não instalada, etc.
            print(f"[aviso] pulando '{lib_name}': {e}", file=sys.stderr)
            continue
        members = []
        for member_name, value in exports.items():
            if member_name.startswith("__"):
                continue
            members.append({
                "name": member_name,
                "kind": _kind_of(value),
                "detail": _detail_for(lib_name, member_name, value),
            })
        # Sem sort alfabético: mantém a ordem de declaração em EXPORTS (dict
        # do Python preserva ordem de inserção), que reflete a ordem lógica
        # escolhida por quem escreveu a lib — extension.js usa esse índice
        # como sortText pra completions de membro não caírem no A-Z padrão.
        metadata[lib_name] = members
    return metadata


def main():
    metadata = build_metadata()
    out_path = Path(__file__).parent / "stdlib_metadata.json"
    out_path.write_text(json.dumps(metadata, ensure_ascii=False, indent=2), encoding="utf-8")
    total_members = sum(len(v) for v in metadata.values())
    print(f"gerado {out_path} — {len(metadata)} módulos, {total_members} membros")


if __name__ == "__main__":
    main()
