#!/usr/bin/env python3
"""Gera bridge/stdlib_metadata.json a partir da stdlib REAL do poolscript.

Antes, a extensão mantinha uma cópia manual (STDLIB_MEMBERS, em extension.js)
do que cada lib exporta — toda vez que a stdlib mudava (renomear lib, tirar
função, mudar assinatura) e ninguém lembrava de atualizar a cópia, o
autocomplete ficava errado pra aquele módulo específico ("expõe módulo de um
jeito e de outro não"). Esse script introspecciona a stdlib de verdade
(poolscript.stdlib) e gera o metadata — sem cópia manual pra dessincronizar.

Além dos membros de cada lib, também resolve o TIPO DE RETORNO de cada
função (via type hint, ex: `def ws_connect(url) -> WsConnection:`) e
introspecciona a classe retornada — isso alimenta o "__classes__" do JSON,
usado pela extensão pra dar autocomplete em `conn = request.ws_connect(...);
conn.<TAB>` (instância devolvida por uma stdlib function, não só o módulo
em si). Sem isso, qualquer objeto rico devolvido pela stdlib (WsConnection,
Response, JinkerResponse, PoolFileUpload, ChannelStatus, ...) ficava sem
member-completion — o mesmo tipo de dessincronização que motivou este
script, só que um nível abaixo (retorno de função em vez de módulo).

Rodar de novo sempre que a stdlib mudar (nova lib, nova função, rename):
    python bridge/gen_stdlib_metadata.py

Precisa do poolscript instalado no Python usado (pip install -e . na raiz
do repo poolscript-lang).
"""
from __future__ import annotations
import inspect
import json
import re
import sys
from pathlib import Path

from poolscript.stdlib import _LAZY_LOADERS, _load  # noqa: E402

# Construtores/typing genéricos que aparecem em anotações compostas
# ("PoolFileUpload | None", "list[PoolFileUpload]") mas não são a classe
# que queremos introspeccionar.
_ANNOTATION_NOISE = {
    "None", "Any", "Optional", "Union", "List", "Dict", "Tuple", "Set",
    "Callable", "Type",
}


def _return_class_name(value) -> str | None:
    """Extrai o nome da classe de retorno de uma função, a partir do type
    hint (`-> WsConnection`, `-> "JinkerResponse"`, `-> "PoolFileUpload | None"`).
    Com `from __future__ import annotations` a anotação vem sempre como
    string — por isso o regex em vez de comparar com `inspect.isclass`
    direto.
    """
    try:
        sig = inspect.signature(value)
    except (TypeError, ValueError):
        return None
    ann = sig.return_annotation
    if ann is inspect.Signature.empty:
        return None
    ann_str = ann if isinstance(ann, str) else getattr(ann, "__name__", str(ann))
    candidates = re.findall(r"[A-Z][A-Za-z0-9_]*", ann_str)
    for name in candidates:
        if name not in _ANNOTATION_NOISE:
            return name
    return None


def _find_class_by_name(name: str):
    """Procura `name` como classe em qualquer módulo poolscript.stdlib.* já
    importado — evita ter que saber de antemão em qual arquivo a classe
    devolvida por uma função de outra lib foi definida."""
    for mod_name, mod in list(sys.modules.items()):
        if not mod_name.startswith("poolscript.stdlib"):
            continue
        candidate = getattr(mod, name, None)
        if inspect.isclass(candidate):
            return candidate
    return None


def _class_members(cls) -> list[dict]:
    members = []
    for member_name, value in inspect.getmembers(cls):
        if member_name.startswith("_"):
            continue
        if not (inspect.isfunction(value) or inspect.ismethod(value)):
            continue
        members.append({
            "name": member_name,
            "kind": "method",
            "detail": _detail_for(cls.__name__, member_name, value),
        })
    return members


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
        # métodos de classe pegos via inspect.getmembers(cls) vêm unbound —
        # 'self' não é passado pelo chamador em PoolScript (conn.close(), não
        # conn.close(self)), então não faz sentido mostrar no autocomplete.
        if p.name == "self":
            continue
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
    classes: dict[str, list[dict]] = {}
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
            member = {
                "name": member_name,
                "kind": _kind_of(value),
                "detail": _detail_for(lib_name, member_name, value),
            }
            # Função que devolve um objeto rico (WsConnection, Response, ...)
            # — resolve a classe e introspecciona os métodos dela também,
            # pra extension.js oferecer completion em `x = lib.func(); x.`
            return_name = _return_class_name(value) if _kind_of(value) == "function" else None
            if return_name:
                cls = _find_class_by_name(return_name)
                if cls is not None:
                    member["returns"] = return_name
                    if return_name not in classes:
                        classes[return_name] = _class_members(cls)
            members.append(member)
        # Sem sort alfabético: mantém a ordem de declaração em EXPORTS (dict
        # do Python preserva ordem de inserção), que reflete a ordem lógica
        # escolhida por quem escreveu a lib — extension.js usa esse índice
        # como sortText pra completions de membro não caírem no A-Z padrão.
        metadata[lib_name] = members
    metadata["__classes__"] = classes
    return metadata


def main():
    metadata = build_metadata()
    out_path = Path(__file__).parent / "stdlib_metadata.json"
    out_path.write_text(json.dumps(metadata, ensure_ascii=False, indent=2), encoding="utf-8")
    total_members = sum(len(v) for v in metadata.values())
    print(f"gerado {out_path} — {len(metadata)} módulos, {total_members} membros")


if __name__ == "__main__":
    main()
