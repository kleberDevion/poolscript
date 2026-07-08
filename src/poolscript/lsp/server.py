"""
Servidor LSP da PoolScript — roda sobre stdio, usado pela extensão do VSCode
(vscode-poolscript) via vscode-languageclient.

Reaproveita o lexer/parser reais da linguagem (poolscript.lexer/parser),
então diagnostics e símbolos refletem exatamente o que o interpretador aceita
— nada de regex adivinhando sintaxe.

Escopo do v1:
  - diagnostics: erro de lexer/parser em tempo real
  - completion: símbolos do próprio arquivo (funções, variáveis) + nomes
    trazidos por `import`/`from ... import ...` (libs internas ou outro
    arquivo .ps do workspace). Nunca sugere nome de arquivo não importado.
  - hover: assinatura de action/reaction + docstring (`\"\"\"...\"\"\"` como
    primeira linha do corpo), igual o Pylance faz com Python.
"""
from __future__ import annotations

import dataclasses
import re
from pathlib import Path
from typing import Any

from lsprotocol import types as lsp
from pygls.lsp.server import LanguageServer

from ..lexer import Lexer, KEYWORDS, PoolSyntaxError
from ..parser import (
    Parser, PoolParseError, ActionDecl, ImportStmt, VarDecl, Assignment,
    UnpackAssignment, UnpackTarget,
)
from ..stdlib import resolve_module

_IDENT_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
_MEMBER_ACCESS_RE = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\.$")


# ─────────────────────────────────────────────────────────────────────
# Índice de símbolos de um arquivo — construído a cada parse bem-sucedido
# ─────────────────────────────────────────────────────────────────────

@dataclasses.dataclass
class ImportedModule:
    alias: str
    export_names: list[str]
    doc_by_name: dict[str, str] = dataclasses.field(default_factory=dict)


@dataclasses.dataclass
class PoolIndex:
    source_lines: list[str] = dataclasses.field(default_factory=list)
    functions: dict[str, ActionDecl] = dataclasses.field(default_factory=dict)
    signatures: dict[str, str] = dataclasses.field(default_factory=dict)
    docstrings: dict[str, str] = dataclasses.field(default_factory=dict)
    modules: dict[str, ImportedModule] = dataclasses.field(default_factory=dict)   # alias -> module
    imported_names: dict[str, str] = dataclasses.field(default_factory=dict)       # nome -> "de onde veio"
    local_names: set = dataclasses.field(default_factory=set)


def _walk(node: Any):
    """Percorre recursivamente qualquer Node/list/dict do AST."""
    if node is None or isinstance(node, (str, int, float, bool)):
        return
    if isinstance(node, (list, tuple)):
        for item in node:
            yield from _walk(item)
        return
    if isinstance(node, dict):
        for v in node.values():
            yield from _walk(v)
        return
    if dataclasses.is_dataclass(node):
        yield node
        for f in dataclasses.fields(node):
            yield from _walk(getattr(node, f.name))


def _unpack_names(target: UnpackTarget) -> list[str]:
    names: list[str] = []
    for el in target.elements:
        if isinstance(el, str):
            names.append(el)
        elif isinstance(el, UnpackTarget):
            names.extend(_unpack_names(el))
    return names


def _extract_docstring(source_lines: list[str], decl_line_1based: int) -> str | None:
    """Acha um `\"\"\" ... \"\"\"` logo no início do corpo (estilo `{ }`)."""
    n = len(source_lines)
    start = decl_line_1based - 1
    brace_line = brace_col = None
    for li in range(start, min(start + 6, n)):
        if "{" in source_lines[li]:
            brace_line = li
            brace_col = source_lines[li].index("{")
            break
    if brace_line is None:
        return None

    rest = source_lines[brace_line][brace_col + 1:].strip()
    scan_from = brace_line
    if rest:
        first_line = rest
    else:
        scan_from = brace_line + 1
        while scan_from < n and not source_lines[scan_from].strip():
            scan_from += 1
        if scan_from >= n:
            return None
        first_line = source_lines[scan_from]

    stripped = first_line.strip()
    if not stripped.startswith('"""'):
        return None

    after = stripped[3:]
    if after.endswith('"""') and len(after) >= 3:
        return after[:-3].strip() or None

    doc_lines = [after]
    j = scan_from + 1
    while j < n:
        line = source_lines[j]
        if '"""' in line:
            doc_lines.append(line[:line.index('"""')])
            break
        doc_lines.append(line)
        j += 1
    text = "\n".join(l.rstrip() for l in doc_lines).strip()
    return text or None


def _signature_of(decl: ActionDecl, source_lines: list[str]) -> str:
    # ActionDecl não guarda se foi declarado `action` ou `reaction` — o parser
    # normaliza os dois no mesmo node. Lê a palavra-chave real da linha fonte.
    prefix = "async " if decl.is_async else ""
    line_text = source_lines[decl.line - 1] if 0 < decl.line <= len(source_lines) else ""
    kind = "reaction" if re.search(r"\breaction\b", line_text) else "action"
    ret = f" {decl.return_type}" if decl.return_type else ""
    return f"{prefix}{kind}{ret} {decl.name}({', '.join(decl.params)})"


def _resolve_ps_file(module_parts: list[str], workspace_roots: list[Path]) -> Path | None:
    rel = Path(*module_parts).with_suffix(".ps")
    for root in workspace_roots:
        candidate = root / rel
        if candidate.is_file():
            return candidate
    return None


def _index_ps_file(path: Path) -> tuple[dict[str, ActionDecl], dict[str, str], dict[str, str]]:
    """Faz parse leve de um .ps importado só pra listar nomes exportados."""
    try:
        text = path.read_text(encoding="utf-8")
        tokens = Lexer(text, str(path)).tokenize()
        program = Parser(tokens, text, str(path)).parse()
    except Exception:
        return {}, {}, {}
    funcs: dict[str, ActionDecl] = {}
    docs: dict[str, str] = {}
    sigs: dict[str, str] = {}
    lines = text.splitlines()
    for node in _walk(program):
        if isinstance(node, ActionDecl):
            funcs[node.name] = node
            sigs[node.name] = _signature_of(node, lines)
            doc = _extract_docstring(lines, node.line)
            if doc:
                docs[node.name] = doc
    return funcs, docs, sigs


def build_index(program, source_text: str, workspace_roots: list[Path]) -> PoolIndex:
    idx = PoolIndex(source_lines=source_text.splitlines())

    for node in _walk(program):
        if isinstance(node, ActionDecl):
            idx.functions[node.name] = node
            idx.signatures[node.name] = _signature_of(node, idx.source_lines)
            doc = _extract_docstring(idx.source_lines, node.line)
            if doc:
                idx.docstrings[node.name] = doc

        elif isinstance(node, VarDecl):
            idx.local_names.add(node.name)

        elif isinstance(node, Assignment):
            idx.local_names.add(node.target)

        elif isinstance(node, UnpackAssignment):
            for name in _unpack_names(node.targets):
                idx.local_names.add(name)

        elif isinstance(node, ImportStmt):
            _index_import(idx, node, workspace_roots)

    return idx


def _index_import(idx: PoolIndex, node: ImportStmt, workspace_roots: list[Path]) -> None:
    module_name = ".".join(node.module)
    exports = resolve_module(node.module)
    func_docs: dict[str, ActionDecl] = {}
    py_docs: dict[str, str] = {}
    py_sigs: dict[str, str] = {}

    if exports is None:
        # não é lib interna — tenta como arquivo .ps do próprio workspace
        ps_path = _resolve_ps_file(node.module, workspace_roots)
        if ps_path is not None:
            funcs, docs, sigs = _index_ps_file(ps_path)
            exports = {name: decl for name, decl in funcs.items()}
            func_docs = funcs
            py_docs = docs
            py_sigs = sigs

    if exports is None:
        return   # não conseguimos resolver — não inventa sugestão

    export_names = sorted(exports.keys())

    if node.mode == "import":
        alias = node.module_alias or node.module[-1]
        idx.modules[alias] = ImportedModule(alias=alias, export_names=export_names, doc_by_name=py_docs)
    else:  # "from" / "push"
        names = node.names or export_names
        for name in names:
            bind_name = node.name_aliases.get(name, name)
            idx.imported_names[bind_name] = module_name
            if name in func_docs:
                idx.functions.setdefault(bind_name, func_docs[name])
                idx.signatures.setdefault(bind_name, py_sigs[name])
                doc = py_docs.get(name)
                if doc:
                    idx.docstrings.setdefault(bind_name, doc)
            elif exports.get(name) is not None and getattr(exports[name], "__doc__", None):
                idx.docstrings.setdefault(bind_name, exports[name].__doc__.strip())
        if not node.names:
            alias = node.module_alias or node.module[-1]
            idx.modules[alias] = ImportedModule(alias=alias, export_names=export_names, doc_by_name=py_docs)


# ─────────────────────────────────────────────────────────────────────
# Servidor
# ─────────────────────────────────────────────────────────────────────

server = LanguageServer("poolscript-lsp", "v1")

_INDEXES: dict[str, PoolIndex] = {}


def _workspace_roots(ls: LanguageServer) -> list[Path]:
    roots = []
    for folder in getattr(ls.workspace, "folders", {}).values():
        try:
            roots.append(Path(folder.uri.replace("file:///", "").replace("file://", "")))
        except Exception:
            pass
    if not roots and getattr(ls.workspace, "root_path", None):
        roots.append(Path(ls.workspace.root_path))
    return roots


def _diagnose_and_index(ls: LanguageServer, uri: str, text: str) -> None:
    diagnostics: list[lsp.Diagnostic] = []
    try:
        tokens = Lexer(text, uri).tokenize()
        program = Parser(tokens, text, uri).parse()
        _INDEXES[uri] = build_index(program, text, _workspace_roots(ls))
    except (PoolSyntaxError, PoolParseError) as e:
        line = getattr(e, "line", None)
        col = getattr(e, "col", None)
        token = getattr(e, "token", None)
        if line is None and token is not None:
            line = getattr(token, "line", 1)
            col = getattr(token, "col", 1)
        line = max((line or 1) - 1, 0)
        col = max((col or 1) - 1, 0)
        diagnostics.append(lsp.Diagnostic(
            range=lsp.Range(
                start=lsp.Position(line=line, character=col),
                end=lsp.Position(line=line, character=col + 1),
            ),
            message=getattr(e, "msg", str(e)),
            severity=lsp.DiagnosticSeverity.Error,
            source="poolscript",
        ))
    except Exception:
        # nunca deixa o server cair por causa de um arquivo problemático
        pass
    ls.text_document_publish_diagnostics(
        lsp.PublishDiagnosticsParams(uri=uri, diagnostics=diagnostics)
    )


@server.feature(lsp.TEXT_DOCUMENT_DID_OPEN)
def did_open(ls: LanguageServer, params: lsp.DidOpenTextDocumentParams):
    _diagnose_and_index(ls, params.text_document.uri, params.text_document.text)


@server.feature(lsp.TEXT_DOCUMENT_DID_CHANGE)
def did_change(ls: LanguageServer, params: lsp.DidChangeTextDocumentParams):
    doc = ls.workspace.get_text_document(params.text_document.uri)
    _diagnose_and_index(ls, params.text_document.uri, doc.source)


def _word_at(line_text: str, character: int) -> str | None:
    for m in _IDENT_RE.finditer(line_text):
        if m.start() <= character <= m.end():
            return m.group(0)
    return None


@server.feature(
    lsp.TEXT_DOCUMENT_COMPLETION,
    lsp.CompletionOptions(trigger_characters=["."]),
)
def completions(ls: LanguageServer, params: lsp.CompletionParams):
    idx = _INDEXES.get(params.text_document.uri)
    doc = ls.workspace.get_text_document(params.text_document.uri)
    pos = params.position
    line_text = doc.lines[pos.line] if pos.line < len(doc.lines) else ""
    prefix = line_text[:pos.character]

    m = _MEMBER_ACCESS_RE.search(prefix)
    if m and idx is not None:
        alias = m.group(1)
        mod = idx.modules.get(alias)
        if mod is None:
            return lsp.CompletionList(is_incomplete=False, items=[])
        return lsp.CompletionList(is_incomplete=False, items=[
            lsp.CompletionItem(
                label=name, kind=lsp.CompletionItemKind.Function,
                documentation=mod.doc_by_name.get(name),
            )
            for name in mod.export_names
        ])

    items: list[lsp.CompletionItem] = [
        lsp.CompletionItem(label=kw, kind=lsp.CompletionItemKind.Keyword)
        for kw in sorted(KEYWORDS)
    ]
    if idx is not None:
        for name in idx.functions:
            items.append(lsp.CompletionItem(
                label=name, kind=lsp.CompletionItemKind.Function,
                detail=idx.signatures.get(name),
                documentation=idx.docstrings.get(name),
            ))
        for name in idx.imported_names:
            items.append(lsp.CompletionItem(label=name, kind=lsp.CompletionItemKind.Function))
        for name in idx.modules:
            items.append(lsp.CompletionItem(label=name, kind=lsp.CompletionItemKind.Module))
        for name in idx.local_names:
            items.append(lsp.CompletionItem(label=name, kind=lsp.CompletionItemKind.Variable))

    return lsp.CompletionList(is_incomplete=False, items=items)


@server.feature(lsp.TEXT_DOCUMENT_HOVER)
def hover(ls: LanguageServer, params: lsp.HoverParams):
    idx = _INDEXES.get(params.text_document.uri)
    if idx is None:
        return None
    doc = ls.workspace.get_text_document(params.text_document.uri)
    pos = params.position
    line_text = doc.lines[pos.line] if pos.line < len(doc.lines) else ""
    word = _word_at(line_text, pos.character)
    if not word:
        return None

    decl = idx.functions.get(word)
    if decl is not None:
        value = f"```poolscript\n{idx.signatures.get(word, decl.name)}\n```"
        docstring = idx.docstrings.get(word)
        if docstring:
            value += f"\n\n{docstring}"
        return lsp.Hover(contents=lsp.MarkupContent(kind=lsp.MarkupKind.Markdown, value=value))

    if word in idx.imported_names:
        value = f"```poolscript\n(importado de {idx.imported_names[word]}) {word}\n```"
        docstring = idx.docstrings.get(word)
        if docstring:
            value += f"\n\n{docstring}"
        return lsp.Hover(contents=lsp.MarkupContent(kind=lsp.MarkupKind.Markdown, value=value))

    if word in idx.modules:
        mod = idx.modules[word]
        names = ", ".join(mod.export_names[:12]) + ("…" if len(mod.export_names) > 12 else "")
        value = f"```poolscript\nmodule {word}\n```\n\nexporta: {names}"
        return lsp.Hover(contents=lsp.MarkupContent(kind=lsp.MarkupKind.Markdown, value=value))

    return None


def main() -> None:
    server.start_io()


if __name__ == "__main__":
    main()
