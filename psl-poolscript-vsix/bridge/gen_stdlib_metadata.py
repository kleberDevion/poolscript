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
from poolscript.builtins import GLOBAL_BUILTINS  # noqa: E402

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


def _instance_members(instance, type_name: str) -> list[dict]:
    """Introspecciona uma instância JÁ EXISTENTE (não a classe) — pega tanto
    método de classe quanto atributo de instância setado em __init__ (ex:
    Jinker.socket/channel), e objeto-callable (tem __call__, ex:
    ChannelManager) tratado igual a método (inspect.signature introspecciona
    o __call__ automaticamente)."""
    members: dict[str, dict] = {}
    for member_name, value in inspect.getmembers(instance):
        if member_name.startswith("_"):
            continue
        if callable(value):
            members[member_name] = {
                "name": member_name,
                "kind": "method",
                "detail": _detail_for(type_name, member_name, value),
            }
        else:
            members[member_name] = {
                "name": member_name,
                "kind": "property",
                "detail": f"{type_name}.{member_name}",
            }
    return list(members.values())


def _class_members(cls) -> list[dict]:
    members: dict[str, dict] = {}
    for member_name, value in inspect.getmembers(cls):
        if member_name.startswith("_"):
            continue
        if not (inspect.isfunction(value) or inspect.ismethod(value)):
            continue
        members[member_name] = {
            "name": member_name,
            "kind": "method",
            "detail": _detail_for(cls.__name__, member_name, value),
        }
    # Muita classe da stdlib guarda a API de verdade em ATRIBUTO DE INSTÂNCIA
    # setado no __init__ (ex: Jinker.socket / Jinker.channel são objetos
    # atribuídos em self.socket = ..., não métodos da classe) — só olhar a
    # classe (getmembers(cls)) não vê isso. Instancia sem args pra pegar
    # também — se falhar (construtor exige argumento obrigatório), fica só
    # com o que já foi achado acima.
    try:
        instance_members = {m["name"]: m for m in _instance_members(cls(), cls.__name__)}
        for name, m in instance_members.items():
            members.setdefault(name, m)
    except Exception:
        pass
    return list(members.values())


_PRIMITIVE_TYPE_NAMES = {"str", "int", "float", "bool", "dict", "list", "tuple", "Any"}


def _looks_constructible(cls) -> bool:
    """False pra classe cujo __init__ pede um tipo que PoolScript não tem
    como fornecer direto (ex: `path: Path` do PoolFile — path é sempre str
    do lado da linguagem, nunca um pathlib.Path de verdade). Essas classes
    normalmente só existem no builtin/lib pra comparação de tipo (`x is
    PoolFile`), nunca pra ser instanciadas pelo usuário — anunciar
    "PoolFile(path)" como se fosse construtor de verdade é enganoso.
    Continua entrando em __classes__ (útil se algum dia vier de outro
    jeito), só não vira sugestão de "chame isso pra criar um".
    """
    try:
        sig = inspect.signature(cls.__init__)
    except (TypeError, ValueError):
        return True
    for pname, p in sig.parameters.items():
        if pname in ("self", "args", "kwargs"):
            continue
        ann = p.annotation
        if ann is inspect.Parameter.empty:
            continue
        ann_str = ann if isinstance(ann, str) else getattr(ann, "__name__", str(ann))
        m = re.match(r"[A-Za-z_][A-Za-z0-9_]*", ann_str)
        base = m.group(0) if m else ann_str
        if base not in _PRIMITIVE_TYPE_NAMES:
            return False
    return True


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
    # lib_name vazio = builtin global, sem prefixo de módulo (post(...), não .post(...))
    prefix = f"{lib_name}." if lib_name else ""
    call_form = f"{prefix}{sig}" if sig else f"{prefix}{member_name}"
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
            kind = _kind_of(value)
            member = {
                "name": member_name,
                "kind": kind,
                "detail": _detail_for(lib_name, member_name, value),
            }
            # Classe exportada direto (ex: jinker.Jinker) — instanciada como
            # `app = jinker.Jinker(...)` — introspecciona os métodos dela
            # também, senão `app.<TAB>` (route/socket/channel...) não tinha
            # nenhuma sugestão porque só função-fábrica com "-> Tipo" virava
            # entrada em __classes__, nunca a classe em si.
            if kind == "class":
                if member_name not in classes:
                    classes[member_name] = _class_members(value)
                if _looks_constructible(value):
                    member["returns"] = member_name
            # Função que devolve um objeto rico (WsConnection, Response, ...)
            # — resolve a classe e introspecciona os métodos dela também,
            # pra extension.js oferecer completion em `x = lib.func(); x.`
            return_name = _return_class_name(value) if kind == "function" else None
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

    # ── builtins globais (sem import) ─────────────────────────────────
    # Antes: GLOBAL_BUILTINS era uma cópia manual dentro do PRÓPRIO
    # extension.js (mesmo anti-padrão que motivou esse script pras libs) —
    # e além de poder dessincronizar a lista em si, objetos-instância
    # como `Parsing` (builtin global, ver builtins.py) nunca tinham member
    # completion: "Parsing.<TAB>" não mostrava nada porque nada
    # introspeccionava esse objeto. Agora entra no mesmo mecanismo das libs.
    builtins_list = []
    for name, value in GLOBAL_BUILTINS.items():
        kind = _kind_of(value)
        builtins_list.append({
            "name": name,
            "kind": kind,
            "detail": _detail_for("", name, value),
        })
        if kind == "class":
            if name not in classes:
                classes[name] = _class_members(value)
            if _looks_constructible(value):
                builtins_list[-1]["returns"] = name
        elif kind not in ("class", "function") and not inspect.isbuiltin(value):
            # objeto-instância singleton com API própria (ex: Parsing) —
            # vira um pseudo-módulo de primeira classe no metadata: quem
            # digita "Parsing." bate no MESMO caminho de "os." (tier 1 de
            # completion), sem precisar de nenhuma mudança no extension.js.
            member_list = _instance_members(value, name)
            if member_list and name not in metadata:
                metadata[name] = member_list
    metadata["__builtins__"] = builtins_list
    return metadata


def _alt(names) -> str:
    """Alternância de regex ordenada por tamanho decrescente — nome mais longo
    primeiro, senão 'db' casaria antes de 'dbx' e cortaria o realce no meio."""
    return "|".join(sorted(set(names), key=lambda n: (-len(n), n)))


def update_grammar(metadata: dict) -> str:
    """Reescreve as listas de nomes da grammar TextMate a partir da stdlib REAL.

    As listas de módulos/builtins/exports eram mantidas À MÃO no
    syntaxes/poolscript.tmLanguage.json — mesmo anti-padrão que já tinha
    quebrado o autocomplete. O sintoma: 'psodbc' nunca esteve na lista de
    módulos (só 'db'/'sqlite3'), então não recebia realce; e 'jsonify' só era
    reconhecido como '.jsonify' (com ponto), enquanto o uso real é sem ponto
    ('from jinker import jsonify' → 'return jsonify({...})').

    Aqui as três listas passam a ser geradas por introspecção:
      stdlib_modules  — todo nome importável (inclui psodbc, datasentity…)
      builtins        — GLOBAL_BUILTINS reais + os registrados no interpreter
      stdlib_exports  — nomes exportados pelas libs, usados SEM ponto depois
                        de `from X import Y` (jsonify, Jinker, cors, asdict…)
    """
    grammar_path = Path(__file__).parent.parent / "syntaxes" / "poolscript.tmLanguage.json"
    grammar = json.loads(grammar_path.read_text(encoding="utf-8"))

    modules = [k for k in _LAZY_LOADERS]
    # builtins do builtins.py + os que só existem via interpreter.globals.define
    interp_only = ["post", "input", "load", "map", "filter", "sleep", "gather",
                   "addEnd", "removeEnd", "addStart", "removeStart"]
    builtins_names = [b["name"] for b in metadata.get("__builtins__", [])] + interp_only

    exports = set()
    for lib_name in _LAZY_LOADERS:
        for m in metadata.get(lib_name, []) or []:
            if not m["name"].startswith("_"):
                exports.add(m["name"])
    # não duplica o que já é módulo ou builtin (o escopo mais específico ganha)
    exports -= set(modules) | set(builtins_names)

    grammar["repository"]["stdlib_modules"]["patterns"][0]["match"] = (
        rf"\b({_alt(modules)})\b"
    )
    grammar["repository"]["builtins"]["patterns"][0]["match"] = (
        rf"\b({_alt(builtins_names)})\b(?=\s*(\(|$))"
    )
    # nomes importados usados sem ponto — só quando chamados/referenciados
    grammar["repository"]["stdlib_exports"] = {
        "patterns": [{
            "name": "support.function.stdlib.poolscript",
            "match": rf"\b({_alt(exports)})\b",
        }]
    }
    if {"include": "#stdlib_exports"} not in grammar["patterns"]:
        # depois de builtins pra não roubar match de nome mais específico
        idx = grammar["patterns"].index({"include": "#builtins"}) + 1
        grammar["patterns"].insert(idx, {"include": "#stdlib_exports"})

    grammar_path.write_text(
        json.dumps(grammar, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    return (f"{len(modules)} módulos, {len(set(builtins_names))} builtins, "
            f"{len(exports)} exports")


def main():
    metadata = build_metadata()
    out_path = Path(__file__).parent / "stdlib_metadata.json"
    out_path.write_text(json.dumps(metadata, ensure_ascii=False, indent=2), encoding="utf-8")
    total_members = sum(len(v) for v in metadata.values())
    print(f"gerado {out_path} — {len(metadata)} módulos, {total_members} membros")
    print(f"grammar atualizada — {update_grammar(metadata)}")


if __name__ == "__main__":
    main()
