"""
Servidor LSP da PoolScript — o "Pylance" da linguagem, sobre stdio.

Arquitetura igual à do Pylance: o editor (VS Code, IntelliJ via LSP4IJ,
Neovim, ...) é um cliente fino; o cérebro mora aqui e usa:

  - o LEXER/PARSER REAIS da linguagem (poolscript.lexer/parser) — diagnóstico
    de sintaxe e símbolos refletem exatamente o que o interpretador aceita;
  - o modelo de tipos VIVO (lsp/metadados.py) — introspecção da stdlib real,
    a mesma cadeia type-aware da extensão VS Code:
        conn = psodbc.connect() -> DbConnection -> conn.cursor() -> DbCursor

Capacidades:
  - diagnostics: erro de lexer/parser em tempo real + import/variável NÃO
    usados (apagado, estilo Pylance — DiagnosticTag.Unnecessary)
  - completion: membros por TIPO (cadeias, self., request do jinker),
    argumento nomeado dentro da chamada, from/import (libs e arquivos .ps),
    keywords/builtins/tipos com doc rica
  - hover: assinatura + resumo + exemplo (mesma fonte da doc viva)
"""
from __future__ import annotations

import dataclasses
import re
from pathlib import Path
from typing import Any
from urllib.parse import unquote, urlparse

from lsprotocol import types as lsp
from pygls.lsp.server import LanguageServer

from ..lexer import Lexer, KEYWORDS, PoolSyntaxError
from ..parser import (
    Parser, PoolParseError, ActionDecl, Assignment, Call, DecoratorCall,
    EntityDecl, EnumDecl, ForEachStmt, ImportStmt, LambdaExpr, Literal,
    MemberAccess, MemberAssignment, Name, ReturnStmt, UnpackAssignment,
    UnpackTarget, VarDecl,
)
from ..stdlib import resolve_module, _LAZY_LOADERS
from .metadados import modelo

_IDENT_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
_RE_FROM_PATH = re.compile(r"^\s*from\s+([\w.]*)$")
_RE_IMPORT_PATH = re.compile(r"^\s*(?:import|push)\s+([\w.]*)$")
_RE_FROM_IMPORT = re.compile(r"^\s*from\s+([\w.]+)\s+import\s+(?:[\w]+\s*,\s*)*(\w*)$")
_RE_DECORATOR = re.compile(r"@([\w.]*)$")

_PRIM_LABEL = {"str", "int", "flo", "bool", "list", "dict", "tup", "bytes"}


# ─────────────────────────────────────────────────────────────────────
# Índice de um arquivo — construído a cada parse bem-sucedido
# ─────────────────────────────────────────────────────────────────────

@dataclasses.dataclass
class ImportedModule:
    alias: str
    export_names: list[str]
    doc_by_name: dict[str, str] = dataclasses.field(default_factory=dict)


@dataclasses.dataclass
class EntityInfo:
    name: str
    fields: set = dataclasses.field(default_factory=set)
    methods: dict[str, ActionDecl] = dataclasses.field(default_factory=dict)
    line_ini: int = 0
    line_fim: int = 0
    parents: list[str] = dataclasses.field(default_factory=list)


@dataclasses.dataclass
class Declarado:
    """Um nome declarado (import ou variável) — pra marcar não-usado."""
    nome: str
    line: int      # 1-based
    col: int       # 1-based
    tipo: str      # "import" | "var"


@dataclasses.dataclass
class PoolIndex:
    source_lines: list[str] = dataclasses.field(default_factory=list)
    functions: dict[str, ActionDecl] = dataclasses.field(default_factory=dict)
    signatures: dict[str, str] = dataclasses.field(default_factory=dict)
    docstrings: dict[str, str] = dataclasses.field(default_factory=dict)
    fn_retorno: dict[str, str] = dataclasses.field(default_factory=dict)
    modules: dict[str, ImportedModule] = dataclasses.field(default_factory=dict)
    imported_names: dict[str, str] = dataclasses.field(default_factory=dict)
    local_names: set = dataclasses.field(default_factory=set)
    var_tipos: dict[str, str] = dataclasses.field(default_factory=dict)
    entities: dict[str, EntityInfo] = dataclasses.field(default_factory=dict)
    enums: dict[str, list] = dataclasses.field(default_factory=dict)
    usa_jinker: bool = False
    nao_usados: list[Declarado] = dataclasses.field(default_factory=list)


def _walk(node: Any):
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
    prefix = "async " if decl.is_async else ""
    line_text = source_lines[decl.line - 1] if 0 < decl.line <= len(source_lines) else ""
    kind = "reaction" if re.search(r"\breaction\b", line_text) else "action"
    ret = f" {decl.return_type}" if decl.return_type else ""
    priv = "private " if decl.is_private else ""
    return f"{priv}{prefix}{kind}{ret} {decl.name}({', '.join(decl.params)})"


def _resolve_ps_file(module_parts: list[str], roots: list[Path]) -> Path | None:
    rel = Path(*module_parts)
    for root in roots:
        for ext in (".ps", ".psl", ".p"):
            candidate = root / rel.with_suffix(ext)
            if candidate.is_file():
                return candidate
    return None


def _index_ps_file(path: Path):
    try:
        text = path.read_text(encoding="utf-8")
        tokens = Lexer(text, str(path)).tokenize()
        program = Parser(tokens, text, str(path)).parse()
    except Exception:
        return {}, {}, {}
    funcs, docs, sigs = {}, {}, {}
    lines = text.splitlines()
    for node in _walk(program):
        if isinstance(node, ActionDecl):
            funcs[node.name] = node
            sigs[node.name] = _signature_of(node, lines)
            doc = _extract_docstring(lines, node.line)
            if doc:
                docs[node.name] = doc
    return funcs, docs, sigs


# ─────────────────────────────────────────────────────────────────────
# Inferência de tipo de expressão (o coração type-aware)
# ─────────────────────────────────────────────────────────────────────

def _tipo_expr(node, idx: "PoolIndex") -> str | None:
    """Rótulo de tipo de uma expressão do AST: nome de classe (encadeável)
    ou primitivo. None = desconhecido (e aí NÃO se sugere nada)."""
    m = modelo()
    if isinstance(node, Literal):
        v = node.value
        if isinstance(v, bool):
            return "bool"
        if isinstance(v, int):
            return "int"
        if isinstance(v, float):
            return "flo"
        if isinstance(v, str):
            return "str"
        return None
    if isinstance(node, Name):
        return idx.var_tipos.get(node.value)
    if isinstance(node, Call):
        callee = node.callee
        if isinstance(callee, Name):
            nome = callee.value
            if nome in idx.fn_retorno:
                return idx.fn_retorno[nome]
            if nome in idx.entities:
                return nome                      # Construtor() -> instância
            origem = idx.imported_names.get(nome)
            if origem and origem in m.modulos:
                return m.retorno_de(origem, nome)
            return None
        if isinstance(callee, MemberAccess):
            base = _tipo_expr_base(callee.target, idx)
            if base is None:
                return None
            categoria, rotulo = base
            if categoria == "module":
                return m.retorno_de(rotulo, callee.member)
            membros = m.membros_da_classe(rotulo) or {}
            info = membros.get(callee.member)
            return info.get("returns") if info else None
        return None
    if isinstance(node, MemberAccess):
        base = _tipo_expr_base(node.target, idx)
        if base is None:
            return None
        categoria, rotulo = base
        if categoria == "module":
            info = (modelo().membros_do_modulo(rotulo) or {}).get(node.member)
            return info.get("returns") if info else None
        membros = modelo().membros_da_classe(rotulo) or {}
        info = membros.get(node.member)
        return info.get("returns") if info else None
    return None


def _tipo_expr_base(node, idx: "PoolIndex"):
    """(categoria, rótulo) da BASE de um acesso: ("module", alias) ou
    ("class", Nome). None = desconhecido."""
    if isinstance(node, Name):
        if node.value in idx.modules:
            return ("module", node.value)
        t = _tipo_expr(node, idx)
        return ("class", t) if t else None
    t = _tipo_expr(node, idx)
    return ("class", t) if t else None


# ─────────────────────────────────────────────────────────────────────
# Construção do índice
# ─────────────────────────────────────────────────────────────────────

def _col_do_nome(source_lines: list[str], line_1based: int, nome: str, col_node: int,
                 apos: str = "") -> int:
    """Coluna (1-based) do NOME declarado na linha — o marcador de não-usado
    tem que sublinhar `os` em `import os` e `x` em `int x = 1`, nunca a
    keyword nem o tipo. Fallback: a coluna do nó."""
    if 0 < line_1based <= len(source_lines):
        linha = source_lines[line_1based - 1]
        base = 0
        if apos:
            i = linha.find(apos)
            if i >= 0:
                base = i + len(apos)
        m = re.search(rf"\b{re.escape(nome)}\b", linha[base:])
        if m:
            return base + m.start() + 1
    return col_node


def build_index(program, source_text: str, roots: list[Path]) -> PoolIndex:
    idx = PoolIndex(source_lines=source_text.splitlines())
    declarados: list[Declarado] = []

    for node in _walk(program):
        if isinstance(node, ActionDecl):
            if node.name not in idx.functions:
                idx.functions[node.name] = node
                idx.signatures[node.name] = _signature_of(node, idx.source_lines)
                doc = _extract_docstring(idx.source_lines, node.line)
                if doc:
                    idx.docstrings[node.name] = doc
        elif isinstance(node, EntityDecl):
            info = EntityInfo(name=node.name, parents=list(node.parents or []),
                              line_ini=node.line, line_fim=node.line)
            for sub in _walk(node.body):
                if getattr(sub, "line", 0) > info.line_fim:
                    info.line_fim = sub.line
                if isinstance(sub, MemberAssignment) and isinstance(sub.target, Name) \
                        and sub.target.value == "self":
                    info.fields.add(sub.member)
                elif isinstance(sub, ActionDecl):
                    info.methods[sub.name] = sub
            for f in (node.fields or []):
                nome_campo = getattr(f, "name", None)
                if nome_campo:
                    info.fields.add(nome_campo)
            idx.entities[node.name] = info
        elif isinstance(node, EnumDecl):
            idx.enums[node.name] = [getattr(mb, "name", str(mb)) for mb in node.members]
        elif isinstance(node, ForEachStmt):
            idx.local_names.add(node.item_name)
        elif isinstance(node, ImportStmt):
            _index_import(idx, node, roots)
            if node.module and node.module[0] == "jinker":
                idx.usa_jinker = True
            if node.mode == "import":
                alias = node.module_alias or node.module[-1]
                col = _col_do_nome(idx.source_lines, node.line, alias, node.col)
                declarados.append(Declarado(alias, node.line, col, "import"))
            else:
                for nome in (node.names or []):
                    bind = node.name_aliases.get(nome, nome)
                    col = _col_do_nome(idx.source_lines, node.line, bind, node.col,
                                       apos="import")
                    declarados.append(Declarado(bind, node.line, col, "import"))

    # 2ª passada: tipos de variável (as funções/entidades/imports já existem)
    for node in _walk(program):
        if isinstance(node, VarDecl):
            idx.local_names.add(node.name)
            t = node.declared_type or _tipo_expr(node.value, idx)
            if t:
                idx.var_tipos[node.name] = t
            col = _col_do_nome(idx.source_lines, node.line, node.name, node.col)
            declarados.append(Declarado(node.name, node.line, col, "var"))
        elif isinstance(node, Assignment):
            if node.target not in idx.local_names:
                col = _col_do_nome(idx.source_lines, node.line, node.target, node.col)
                declarados.append(Declarado(node.target, node.line, col, "var"))
            idx.local_names.add(node.target)
            t = _tipo_expr(node.value, idx)
            if t and node.operator == "=":
                idx.var_tipos[node.target] = t
        elif isinstance(node, UnpackAssignment):
            for name in _unpack_names(node.targets):
                idx.local_names.add(name)
        elif isinstance(node, ActionDecl):
            for r in _walk(node.block):
                if isinstance(r, ReturnStmt) and r.value is not None:
                    t = _tipo_expr(r.value, idx)
                    if t:
                        idx.fn_retorno[node.name] = t
                        break

    # usos — quem aparece lendo (Name) ou como base de decorator (@app.route)
    usados: set = set()
    for node in _walk(program):
        if isinstance(node, Name):
            usados.add(node.value)
        elif isinstance(node, DecoratorCall):
            if node.path:
                usados.add(node.path[0])
        elif isinstance(node, (Assignment, MemberAssignment)):
            # `x = ...` não é USO de x; `x += 1` é (lê antes de gravar)
            if isinstance(node, Assignment) and node.operator != "=":
                usados.add(node.target)

    vistos: set = set()
    for d in declarados:
        if d.nome in usados or d.nome in vistos or d.nome.startswith("_"):
            continue
        vistos.add(d.nome)
        idx.nao_usados.append(d)
    return idx


def _index_import(idx: PoolIndex, node: ImportStmt, roots: list[Path]) -> None:
    module_name = ".".join(node.module)
    exports = resolve_module(node.module)
    func_docs: dict[str, ActionDecl] = {}
    py_docs: dict[str, str] = {}
    py_sigs: dict[str, str] = {}

    if exports is None:
        ps_path = _resolve_ps_file(node.module, roots)
        if ps_path is not None:
            funcs, docs, sigs = _index_ps_file(ps_path)
            exports = dict(funcs)
            func_docs, py_docs, py_sigs = funcs, docs, sigs

    if exports is None:
        return

    export_names = sorted(exports.keys())
    if node.mode == "import":
        alias = node.module_alias or node.module[-1]
        idx.modules[alias] = ImportedModule(alias, export_names, py_docs)
    else:
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
        if not node.names:
            alias = node.module_alias or node.module[-1]
            idx.modules[alias] = ImportedModule(alias, export_names, py_docs)


# ─────────────────────────────────────────────────────────────────────
# Cadeia textual no cursor: "conn.cursor()." -> [conn, cursor()]
# ─────────────────────────────────────────────────────────────────────

def _cadeia(prefix: str):
    """Extrai a cadeia de acesso imediatamente antes do cursor. Devolve
    (segmentos, parcial): segmentos = [(nome, é_chamada)], parcial = o que
    já foi digitado depois do último ponto ("fet" em `cur.fet`)."""
    i = len(prefix)
    parcial = ""
    m = re.search(r"([A-Za-z_]\w*)$", prefix)
    if m:
        parcial = m.group(1)
        i = m.start()
    if i == 0 or prefix[i - 1] != ".":
        return None, parcial
    i -= 1  # consome o ponto
    segs: list[tuple[str, bool]] = []
    while True:
        chamada = False
        if i > 0 and prefix[i - 1] == ")":
            chamada = True
            profundidade = 0
            j = i - 1
            while j >= 0:
                if prefix[j] == ")":
                    profundidade += 1
                elif prefix[j] == "(":
                    profundidade -= 1
                    if profundidade == 0:
                        break
                j -= 1
            if j < 0:
                return None, parcial
            i = j
        m = re.search(r"([A-Za-z_]\w*)$", prefix[:i])
        if not m:
            return None, parcial
        segs.append((m.group(1), chamada))
        i = m.start()
        if i > 0 and prefix[i - 1] == ".":
            i -= 1
            continue
        break
    segs.reverse()
    return segs, parcial


def _resolve_cadeia(segs, idx: PoolIndex, linha_cursor: int):
    """Resolve a cadeia até o fim. Devolve ("module", alias) |
    ("class", Nome) | ("entity", Nome) | ("enum", Nome) | None."""
    m = modelo()
    nome, chamada = segs[0]

    if nome == "self":
        ent = _entity_na_linha(idx, linha_cursor)
        atual = ("entity", ent.name) if ent else None
    elif nome in idx.modules:
        atual = ("module", nome)
    elif nome == "request" and idx.usa_jinker:
        atual = ("class", "RequestProxy")
    elif nome in idx.entities:
        atual = ("class", nome) if chamada else ("entity-ref", nome)
    elif nome in idx.enums:
        atual = ("enum", nome)
    elif chamada and nome in idx.fn_retorno:
        atual = ("class", idx.fn_retorno[nome])
    elif chamada and nome in idx.imported_names:
        origem = idx.imported_names[nome]
        ret = m.retorno_de(origem, nome) if origem in m.modulos else None
        atual = ("class", ret) if ret else None
    elif nome in idx.var_tipos:
        atual = ("class", idx.var_tipos[nome])
    else:
        atual = None

    for nome_seg, chamada in segs[1:]:
        if atual is None:
            return None
        categoria, rotulo = atual
        if categoria == "module":
            info = (m.membros_do_modulo(rotulo) or {}).get(nome_seg)
        elif categoria in ("class", "entity"):
            if categoria == "entity":
                ent = idx.entities.get(rotulo)
                if ent and nome_seg in ent.methods:
                    decl = ent.methods[nome_seg]
                    if chamada and decl.name in idx.fn_retorno:
                        atual = ("class", idx.fn_retorno[decl.name])
                        continue
                    return None
                if ent and nome_seg in ent.fields:
                    return None   # tipo do campo é desconhecido — não inventa
                info = None
            else:
                info = (m.membros_da_classe(rotulo) or {}).get(nome_seg)
        else:
            return None
        if not info:
            return None
        ret = info.get("returns")
        atual = ("class", ret) if ret else None
    return atual


def _entity_na_linha(idx: PoolIndex, linha_1based: int) -> EntityInfo | None:
    for ent in idx.entities.values():
        if ent.line_ini <= linha_1based <= ent.line_fim + 1:
            return ent
    return None


# ─────────────────────────────────────────────────────────────────────
# Servidor
# ─────────────────────────────────────────────────────────────────────

server = LanguageServer("poolscript-lsp", "v2")
_INDEXES: dict[str, PoolIndex] = {}


def _uri_para_path(uri: str) -> Path | None:
    try:
        p = urlparse(uri)
        if p.scheme != "file":
            return None
        return Path(unquote(p.path))
    except Exception:
        return None


def _workspace_roots(ls: LanguageServer, uri: str | None = None) -> list[Path]:
    roots = []
    for folder in getattr(ls.workspace, "folders", {}).values():
        fp = _uri_para_path(folder.uri)
        if fp:
            roots.append(fp)
    if getattr(ls.workspace, "root_path", None):
        roots.append(Path(ls.workspace.root_path))
    if uri:
        fp = _uri_para_path(uri)
        if fp:
            roots.append(fp.parent)
    # libs instaladas pelo pkgmgr também são importáveis — mesmas raízes que
    # o interpretador e o cérebro embutido do vsix enxergam
    for r in list(roots):
        libs = r / ".poolscript" / "libs"
        if libs.is_dir():
            roots.append(libs)
    return roots


def _parse_com_reparo(text: str, uri: str):
    """Parse tolerante a digitação: se falhar, neutraliza a linha do erro e
    tenta de novo (até 3 linhas) — é como o índice continua vivo enquanto o
    usuário está no meio de um `cur.`. Devolve (program, texto_usado) ou
    levanta o PRIMEIRO erro (que vira diagnóstico)."""
    try:
        tokens = Lexer(text, uri).tokenize()
        return Parser(tokens, text, uri).parse(), text
    except (PoolSyntaxError, PoolParseError) as primeiro:
        linhas = text.splitlines()
        erro = primeiro
        for _ in range(3):
            token = getattr(erro, "token", None)
            line = getattr(erro, "line", None) or getattr(token, "line", None)
            if not line or not linhas:
                raise primeiro
            # erro em EOF aponta pra linha vazia — anda pra trás até a linha
            # que tem conteúdo (o `connect(` incompleto de verdade)
            alvo = min(line, len(linhas))
            while alvo >= 1 and not linhas[alvo - 1].strip():
                alvo -= 1
            if alvo < 1:
                raise primeiro
            linhas[alvo - 1] = ""
            reparado = "\n".join(linhas)
            try:
                tokens = Lexer(reparado, uri).tokenize()
                programa = Parser(tokens, reparado, uri).parse()
                raise _Reparado(programa, reparado, primeiro)
            except (PoolSyntaxError, PoolParseError) as e:
                erro = e
        raise primeiro


class _Reparado(Exception):
    """Parse só passou com reparo — carrega o programa E o erro original."""
    def __init__(self, programa, texto, erro):
        self.programa = programa
        self.texto = texto
        self.erro = erro


def _diagnose_and_index(ls: LanguageServer, uri: str, text: str) -> None:
    diagnostics: list[lsp.Diagnostic] = []
    try:
        try:
            program, texto_usado = _parse_com_reparo(text, uri)
            erro_original = None
        except _Reparado as r:
            program, texto_usado = r.programa, r.texto
            erro_original = r.erro
        idx = build_index(program, texto_usado, _workspace_roots(ls, uri))
        _INDEXES[uri] = idx
        if erro_original is not None:
            raise erro_original
        for d in idx.nao_usados:
            line0 = max(d.line - 1, 0)
            col0 = max(d.col - 1, 0)
            rotulo = "import" if d.tipo == "import" else "variável"
            diagnostics.append(lsp.Diagnostic(
                range=lsp.Range(
                    start=lsp.Position(line=line0, character=col0),
                    end=lsp.Position(line=line0, character=col0 + max(len(d.nome), 1)),
                ),
                message=f"{rotulo} '{d.nome}' não é usado",
                severity=lsp.DiagnosticSeverity.Hint,
                tags=[lsp.DiagnosticTag.Unnecessary],
                source="poolscript",
            ))
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
        pass   # nunca derruba o servidor por causa de um arquivo
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


# ── completion ───────────────────────────────────────────────────────

_KIND = {
    "function": lsp.CompletionItemKind.Function,
    "method": lsp.CompletionItemKind.Method,
    "property": lsp.CompletionItemKind.Property,
    "class": lsp.CompletionItemKind.Class,
    "value": lsp.CompletionItemKind.Constant,
}


def _item_membro(nome: str, info: dict) -> lsp.CompletionItem:
    kind = _KIND.get(info.get("kind", ""), lsp.CompletionItemKind.Field)
    detail = info.get("sig") or (info.get("returns") or None)
    return lsp.CompletionItem(label=nome, kind=kind, detail=detail,
                              documentation=info.get("doc"))


def _doc_rica(spec: dict) -> lsp.MarkupContent:
    partes = []
    if spec.get("sig"):
        partes.append(f"```poolscript\n{spec['sig']}\n```")
    if spec.get("resumo"):
        partes.append(spec["resumo"])
    ex = spec.get("ex") or []
    if ex:
        cod = ex[0][0] if isinstance(ex[0], (list, tuple)) else str(ex[0])
        partes.append(f"```poolscript\n{cod}\n```")
    return lsp.MarkupContent(kind=lsp.MarkupKind.Markdown, value="\n\n".join(partes))


def _completa_caminho_import(parcial: str, roots: list[Path]) -> list[lsp.CompletionItem]:
    """`import <parc>` / `from <parc>` — libs internas + arquivos .ps."""
    itens = []
    if parcial.startswith("."):
        # relativo: arquivos do diretório do próprio arquivo
        base = parcial.lstrip(".")
        for root in roots:
            try:
                for f in root.iterdir():
                    if f.suffix in (".ps", ".psl", ".p") and f.stem != base:
                        itens.append(lsp.CompletionItem(
                            label=f.stem, kind=lsp.CompletionItemKind.File,
                            detail=f.name))
            except OSError:
                pass
        return itens
    for alias in sorted(_LAZY_LOADERS):
        itens.append(lsp.CompletionItem(label=alias, kind=lsp.CompletionItemKind.Module))
    for root in roots:
        try:
            for f in root.iterdir():
                if f.suffix in (".ps", ".psl", ".p"):
                    itens.append(lsp.CompletionItem(
                        label=f.stem, kind=lsp.CompletionItemKind.File, detail=f.name))
        except OSError:
            pass
    return itens


def _exports_de(caminho: str, roots: list[Path]):
    """Exports de `from X import |` — lib interna ou arquivo .ps do workspace."""
    parts = [p for p in caminho.lstrip(".").split(".") if p]
    if not parts:
        return None, {}
    exports = resolve_module(parts) if not caminho.startswith(".") else None
    if exports is not None:
        docs = {n: (getattr(v, "__doc__", None) or "").strip()[:200]
                for n, v in exports.items() if not n.startswith("__")}
        return sorted(n for n in exports if not n.startswith("__")), docs
    ps_path = _resolve_ps_file(parts, roots)
    if ps_path is None:
        return None, {}
    funcs, docs, _sigs = _index_ps_file(ps_path)
    return sorted(funcs.keys()), docs


def _chamada_aberta(prefix: str):
    """Se o cursor está dentro de `f(...` sem fechar, devolve a cadeia do
    callee (pra sugerir argumento nomeado)."""
    profundidade = 0
    for i in range(len(prefix) - 1, -1, -1):
        ch = prefix[i]
        if ch == ")":
            profundidade += 1
        elif ch == "(":
            if profundidade == 0:
                antes = prefix[:i]
                m = re.search(r"([A-Za-z_][\w.]*(?:\(\))?)$", antes)
                if not m:
                    return None
                return antes, m.group(1)
            profundidade -= 1
    return None


def _params_do_callee(texto_callee: str, idx: PoolIndex, linha: int):
    m = modelo()
    partes = texto_callee.split(".")
    if len(partes) == 1:
        nome = partes[0]
        decl = idx.functions.get(nome)
        if decl:
            return [{"name": p, "opt": (decl.defaults or {}).get(p) is not None
                     or p in (decl.defaults or {})} for p in decl.params]
        origem = idx.imported_names.get(nome)
        if origem and origem in m.modulos:
            info = m.modulos[origem].get(nome)
            return info.get("params") if info else None
        return None
    segs, _ = _cadeia(texto_callee.rsplit(".", 1)[0] + ".")
    ultimo = partes[-1].replace("()", "")
    if segs is None:
        # base simples: alias de módulo ou variável
        base = partes[0]
        if base in idx.modules:
            info = (m.membros_do_modulo(base) or {}).get(ultimo)
            return info.get("params") if info else None
        segs = [(base, False)]
    alvo = _resolve_cadeia(segs, idx, linha)
    if alvo is None:
        return None
    categoria, rotulo = alvo
    if categoria == "module":
        info = (m.membros_do_modulo(rotulo) or {}).get(ultimo)
    else:
        info = (m.membros_da_classe(rotulo) or {}).get(ultimo)
    return info.get("params") if info else None


@server.feature(
    lsp.TEXT_DOCUMENT_COMPLETION,
    lsp.CompletionOptions(trigger_characters=[".", "(", ",", "@"]),
)
def completions(ls: LanguageServer, params: lsp.CompletionParams):
    uri = params.text_document.uri
    idx = _INDEXES.get(uri)
    doc = ls.workspace.get_text_document(uri)
    pos = params.position
    line_text = doc.lines[pos.line] if pos.line < len(doc.lines) else ""
    prefix = line_text[:pos.character]
    roots = _workspace_roots(ls, uri)
    m = modelo()
    vazio = lsp.CompletionList(is_incomplete=False, items=[])
    # caractere que disparou o popup ('.'/'('/','/'@') — None quando o usuário
    # está digitando um nome ou pediu Ctrl+Espaço. Gatilho pontual responde SÓ
    # o que lhe diz respeito: vírgula sem argumento nomeado = popup NENHUM,
    # nunca a lista genérica (que fazia o Enter "escrever sozinho" no editor).
    ctx = getattr(params, "context", None)
    gatilho = getattr(ctx, "trigger_character", None) if ctx else None

    # 1) `from X import |` — exports do módulo/arquivo, NUNCA builtins
    fi = _RE_FROM_IMPORT.match(prefix)
    if fi:
        nomes, docs = _exports_de(fi.group(1), roots)
        if nomes is None:
            return vazio
        return lsp.CompletionList(is_incomplete=False, items=[
            lsp.CompletionItem(label=n, kind=lsp.CompletionItemKind.Function,
                               documentation=docs.get(n) or None)
            for n in nomes])

    # 2) `from |` / `import |` — libs + arquivos .ps
    fp = _RE_FROM_PATH.match(prefix) or _RE_IMPORT_PATH.match(prefix)
    if fp:
        return lsp.CompletionList(is_incomplete=False,
                                  items=_completa_caminho_import(fp.group(1), roots))

    # 3) decorator `@|` — aliases de módulo (e membros com `@alias.`)
    dm = _RE_DECORATOR.search(prefix)
    if dm and idx is not None:
        caminho = dm.group(1)
        if "." in caminho:
            alias = caminho.split(".")[0]
            t = idx.var_tipos.get(alias)
            membros = (m.membros_da_classe(t) or {}) if t else {}
            itens = [_item_membro(n, i) for n, i in sorted(membros.items())
                     if i.get("kind") == "method"]
            return lsp.CompletionList(is_incomplete=False, items=itens)
        itens = [lsp.CompletionItem(label=v, kind=lsp.CompletionItemKind.Variable)
                 for v in sorted(idx.var_tipos)
                 if idx.var_tipos[v] not in _PRIM_LABEL]
        return lsp.CompletionList(is_incomplete=False, items=itens)
    if gatilho == "@":
        return vazio

    # 4) membro: `cadeia.|`
    segs, _parcial = _cadeia(prefix)
    if segs:
        if idx is None:
            return vazio
        alvo = _resolve_cadeia(segs, idx, pos.line + 1)
        if alvo is None:
            return vazio    # tipo desconhecido: NÃO inventa método
        categoria, rotulo = alvo
        if categoria == "module":
            membros = m.membros_do_modulo(rotulo) or {}
            itens = [_item_membro(n, i) for n, i in sorted(membros.items())]
            mod_idx = idx.modules.get(rotulo)
            if mod_idx:
                nomes_meta = set(membros)
                for n in mod_idx.export_names:
                    if n not in nomes_meta:
                        itens.append(lsp.CompletionItem(
                            label=n, kind=lsp.CompletionItemKind.Function,
                            documentation=mod_idx.doc_by_name.get(n)))
            return lsp.CompletionList(is_incomplete=False, items=itens)
        if categoria in ("entity", "entity-ref"):
            ent = idx.entities.get(rotulo)
            if not ent:
                return vazio
            dentro = _entity_na_linha(idx, pos.line + 1) is ent
            itens = []
            for n in sorted(ent.fields):
                itens.append(lsp.CompletionItem(label=n, kind=lsp.CompletionItemKind.Field))
            for n, decl in sorted(ent.methods.items()):
                if decl.is_private and not dentro:
                    continue
                if n == "__init__":
                    continue
                itens.append(lsp.CompletionItem(
                    label=n, kind=lsp.CompletionItemKind.Method,
                    detail=_signature_of(decl, idx.source_lines),
                    documentation=idx.docstrings.get(n)))
            return lsp.CompletionList(is_incomplete=False, items=itens)
        if categoria == "enum":
            return lsp.CompletionList(is_incomplete=False, items=[
                lsp.CompletionItem(label=n, kind=lsp.CompletionItemKind.EnumMember)
                for n in idx.enums.get(rotulo, [])])
        membros = m.membros_da_classe(rotulo)
        if membros is None:
            return vazio
        return lsp.CompletionList(is_incomplete=False, items=[
            _item_membro(n, i) for n, i in sorted(membros.items())])
    if gatilho == ".":
        return vazio   # ponto sem cadeia resolvível (ex: `1.`): nada

    # 5) argumento nomeado dentro de chamada aberta
    aberta = _chamada_aberta(prefix)
    if aberta and idx is not None:
        _antes, callee = aberta
        ps = _params_do_callee(callee, idx, pos.line + 1)
        if ps:
            itens = [lsp.CompletionItem(
                label=f"{p['name']}=",
                kind=lsp.CompletionItemKind.Variable,
                sort_text=f"0{i:02d}",
                detail="opcional" if p.get("opt") else "obrigatório")
                for i, p in enumerate(ps)]
            # além dos args nomeados, o usuário pode digitar valores: soma locals
            if idx:
                itens += [lsp.CompletionItem(label=n, kind=lsp.CompletionItemKind.Variable)
                          for n in sorted(idx.local_names)]
            return lsp.CompletionList(is_incomplete=False, items=itens)
    if gatilho in (",", "("):
        return vazio   # sem argumento nomeado pra sugerir: popup NENHUM

    # 6) topo: keywords/builtins/tipos ricos + símbolos do arquivo
    itens: list[lsp.CompletionItem] = []
    for kw in sorted(KEYWORDS):
        spec = m.keywords.get(kw)
        itens.append(lsp.CompletionItem(
            label=kw, kind=lsp.CompletionItemKind.Keyword,
            documentation=_doc_rica(spec) if spec else None))
    for nome, spec in sorted(m.builtins.items()):
        itens.append(lsp.CompletionItem(
            label=nome, kind=lsp.CompletionItemKind.Function,
            detail=spec.get("sig") or None, documentation=_doc_rica(spec)))
    for nome, spec in sorted(m.tipos.items()):
        itens.append(lsp.CompletionItem(
            label=nome, kind=lsp.CompletionItemKind.Class,
            documentation=_doc_rica(spec)))
    if idx is not None:
        for name in idx.functions:
            itens.append(lsp.CompletionItem(
                label=name, kind=lsp.CompletionItemKind.Function,
                detail=idx.signatures.get(name),
                documentation=idx.docstrings.get(name)))
        for name in idx.imported_names:
            if name not in idx.functions:
                itens.append(lsp.CompletionItem(label=name,
                                                kind=lsp.CompletionItemKind.Function))
        for name in idx.modules:
            itens.append(lsp.CompletionItem(label=name, kind=lsp.CompletionItemKind.Module))
        for name in idx.entities:
            itens.append(lsp.CompletionItem(label=name, kind=lsp.CompletionItemKind.Class))
        for name in idx.enums:
            itens.append(lsp.CompletionItem(label=name, kind=lsp.CompletionItemKind.Enum))
        for name in sorted(idx.local_names):
            detail = idx.var_tipos.get(name)
            itens.append(lsp.CompletionItem(label=name,
                                            kind=lsp.CompletionItemKind.Variable,
                                            detail=detail))
    return lsp.CompletionList(is_incomplete=False, items=itens)


# ── hover ────────────────────────────────────────────────────────────

def _word_at(line_text: str, character: int):
    for m in _IDENT_RE.finditer(line_text):
        if m.start() <= character <= m.end():
            return m.group(0), m.start()
    return None, 0


@server.feature(lsp.TEXT_DOCUMENT_HOVER)
def hover(ls: LanguageServer, params: lsp.HoverParams):
    uri = params.text_document.uri
    idx = _INDEXES.get(uri)
    doc = ls.workspace.get_text_document(uri)
    pos = params.position
    line_text = doc.lines[pos.line] if pos.line < len(doc.lines) else ""
    word, ini = _word_at(line_text, pos.character)
    if not word:
        return None
    m = modelo()

    def md(value: str):
        return lsp.Hover(contents=lsp.MarkupContent(
            kind=lsp.MarkupKind.Markdown, value=value))

    # membro de uma cadeia: `cur.fetchall` com o cursor em fetchall
    if ini > 0 and line_text[ini - 1] == "." and idx is not None:
        segs, _ = _cadeia(line_text[:ini])
        if segs:
            alvo = _resolve_cadeia(segs, idx, pos.line + 1)
            if alvo:
                categoria, rotulo = alvo
                if categoria == "module":
                    info = (m.membros_do_modulo(rotulo) or {}).get(word)
                else:
                    info = (m.membros_da_classe(rotulo) or {}).get(word)
                if info:
                    sig = info.get("sig") or word
                    valor = f"```poolscript\n{rotulo}.{sig}\n```"
                    if info.get("returns"):
                        valor += f"\n\n→ `{info['returns']}`"
                    if info.get("doc"):
                        valor += f"\n\n{info['doc']}"
                    return md(valor)

    if idx is not None:
        decl = idx.functions.get(word)
        if decl is not None:
            valor = f"```poolscript\n{idx.signatures.get(word, decl.name)}\n```"
            ds = idx.docstrings.get(word)
            if ds:
                valor += f"\n\n{ds}"
            return md(valor)
        if word in idx.modules:
            mod = idx.modules[word]
            nomes = ", ".join(mod.export_names[:12]) + ("…" if len(mod.export_names) > 12 else "")
            return md(f"```poolscript\nmodule {word}\n```\n\nexporta: {nomes}")
        if word in idx.imported_names:
            valor = f"```poolscript\n(importado de {idx.imported_names[word]}) {word}\n```"
            ds = idx.docstrings.get(word)
            if ds:
                valor += f"\n\n{ds}"
            return md(valor)
        if word in idx.entities:
            ent = idx.entities[word]
            met = ", ".join(sorted(n for n in ent.methods if n != "__init__")[:10])
            return md(f"```poolscript\nEntity {word}\n```\n\nmétodos: {met or '—'}")
        if word in idx.enums:
            return md(f"```poolscript\nenum {word}\n```\n\n"
                      f"membros: {', '.join(idx.enums[word])}")
        if word in idx.var_tipos:
            return md(f"```poolscript\n{word}: {idx.var_tipos[word]}\n```")

    for tabela in (m.keywords, m.builtins, m.tipos):
        spec = tabela.get(word)
        if spec:
            h = _doc_rica(spec)
            return lsp.Hover(contents=h)
    return None


def main() -> None:
    modelo()   # aquece a introspecção antes do primeiro request
    server.start_io()


if __name__ == "__main__":
    main()
