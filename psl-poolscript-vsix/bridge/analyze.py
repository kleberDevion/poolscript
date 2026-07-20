#!/usr/bin/env python3
"""Bridge de análise estática pra extensão VS Code do PoolScript.

Processo persistente: lê uma linha JSON por vez do stdin (uma requisição),
escreve uma linha JSON no stdout (a resposta). NUNCA importa/roda o
interpretador — só lexa e parseia com o parser REAL da linguagem
(poolscript_pkg, cópia local de lexer.py/parser.py/ps_errors.py), a mesma
garantia de "não executa código do usuário" que uma ferramenta como o
Pylance dá pra Python.

Protocolo (uma linha JSON por request/response, LF no fim):
  request:  {"id": 1, "path": "C:/.../foo.ps", "text": "<conteúdo do arquivo>"}
  response: {"id": 1, "ok": true, "errors": [], "functions": [...],
             "entities": [...], "variables": [...], "imports": [...],
             "references": {"nome": [{"line":.., "col":..}, ...]}}
          ou, se o parse falhar:
            {"id": 1, "ok": false, "errors": [{"line":.., "col":.., "message":..}]}
"""
import json
import sys
import os
import dataclasses
from collections import defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from poolscript_pkg import Lexer, parse_source, PoolParseError, PoolSyntaxError  # noqa: E402


def _error_position(exc):
    """PoolSyntaxError (lexer) guarda .line/.col direto; PoolParseError
    (parser) guarda via .token.line/.token.col."""
    if hasattr(exc, "line") and hasattr(exc, "col"):
        return exc.line, exc.col
    tok = getattr(exc, "token", None)
    return getattr(tok, "line", 1), getattr(tok, "col", 1)


def _node_type(node):
    return type(node).__name__


def _deep_max_line(node, _seen=None):
    """Acha a maior linha tocada por `node` e todos os seus descendentes,
    percorrendo genericamente qualquer node dataclass (via introspecção de
    campos), listas e dicts — sem precisar manter uma segunda lista manual
    de todos os tipos de node só pra isso. Usado pra estimar o `endLine` de
    uma função (o parser não guarda isso diretamente, só a linha de início).
    """
    if _seen is None:
        _seen = set()
    if node is None or isinstance(node, (str, int, float, bool)):
        return 0
    if isinstance(node, (list, tuple, set)):
        return max((_deep_max_line(v, _seen) for v in node), default=0)
    if isinstance(node, dict):
        return max((_deep_max_line(v, _seen) for v in node.values()), default=0)
    if not dataclasses.is_dataclass(node):
        return 0
    key = id(node)
    if key in _seen:
        return 0
    _seen.add(key)
    best = getattr(node, "line", 0) or 0
    for f in dataclasses.fields(node):
        best = max(best, _deep_max_line(getattr(node, f.name, None), _seen))
    return best


def _extract_docstring(source_lines, decl_line_1based):
    """Acha um `\"\"\" ... \"\"\"` logo no início do corpo (estilo `{ }`) de
    uma action/reaction — mesma ideia de docstring que o Pylance mostra no
    hover pra funções Python."""
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
    doc_text = "\n".join(l.rstrip() for l in doc_lines).strip()
    return doc_text or None


def analyze_source(text):
    try:
        program = parse_source(text, "<bridge>")
    except (PoolSyntaxError, PoolParseError) as e:
        line, col = _error_position(e)
        return {
            "ok": False,
            "errors": [{"line": line, "col": col, "message": str(getattr(e, "msg", e))}],
        }
    except Exception as e:  # nunca deixa uma exceção inesperada derrubar o daemon
        return {"ok": False, "errors": [{"line": 1, "col": 1, "message": f"erro interno do analisador: {e}"}]}

    functions = []
    entities = []
    variables = []
    imports = []
    references = defaultdict(list)
    # Um scope por função/método: {id, startLine, endLine}. "module" (o
    # escopo do arquivo) não entra aqui — é implícito e sempre visível.
    scopes = []
    source_lines = text.splitlines()

    def record_ref(name, node):
        if name:
            references[name].append({"line": node.line, "col": node.col})

    def walk_expr(node):
        if node is None:
            return
        t = _node_type(node)
        if t == "Name":
            record_ref(node.value, node)
        elif t == "MemberAccess":
            walk_expr(node.target)
        elif t == "IndexAccess":
            walk_expr(node.target)
            walk_expr(node.index)
        elif t == "SliceAccess":
            walk_expr(node.target)
            walk_expr(node.start)
            walk_expr(node.stop)
            walk_expr(node.step)
        elif t == "Call":
            walk_expr(node.callee)
            for a in node.args:
                walk_expr(a.value)
        elif t == "BaseCall":
            for a in node.args:
                walk_expr(a.value)
        elif t == "BinaryOp":
            walk_expr(node.left)
            walk_expr(node.right)
        elif t == "UnaryOp":
            walk_expr(node.operand)
        elif t == "PostfixOp":
            walk_expr(node.operand)
        elif t in ("ListLiteral", "TupleLiteral"):
            for it in node.items:
                walk_expr(it)
        elif t == "DictLiteral":
            for entry in node.entries:
                walk_expr(entry.key)
                walk_expr(entry.value)
        elif t == "InterpolatedString":
            for p in node.parts:
                if _node_type(p) != "Literal":
                    walk_expr(p)
        elif t == "LambdaExpr":
            walk_block(node.block)
        elif t == "AwaitExpr":
            walk_expr(node.value)
        elif t == "ColorStrExpr":
            walk_expr(node.expr)
        elif t in ("CountExpr", "CountEachExpr"):
            walk_expr(node.container)
            walk_expr(node.value_node)
        # Literal / TypeName: nada a percorrer

    def infer_type(value_node):
        """Descreve o LADO DIREITO de uma atribuição de forma estruturada,
        pra extension.js resolver o tipo da variável sem precisar refazer
        essa pergunta com regex em cima do texto bruto (que é frágil: pega
        a linha errada em reatribuição, não entende chamada multi-linha,
        casa string/comentário por engano, etc). Isso é calculado UMA VEZ
        aqui, a partir da árvore real — extension.js só consome o resultado.

        Retorna None (sem informação) ou:
          {"kind": "dict"}
          {"kind": "call", "callee": ["Pessoa"]}            # Pessoa(...)
          {"kind": "call", "callee": ["modulo", "Pessoa"]}  # modulo.Pessoa(...)
        """
        if value_node is None:
            return None
        t = _node_type(value_node)
        if t == "DictLiteral":
            return {"kind": "dict"}
        if t == "Call":
            path = []
            node = value_node.callee
            while True:
                nt = _node_type(node)
                if nt == "Name":
                    path.insert(0, node.value)
                    break
                if nt == "MemberAccess":
                    path.insert(0, node.member)
                    node = node.target
                    continue
                return None  # callee complexo demais (índice, chamada encadeada, etc.) — sem inferência
            return {"kind": "call", "callee": path}
        return None

    def walk_unpack_target(target, node, scope_id):
        for el in target.elements:
            if isinstance(el, str):
                variables.append({"name": el, "line": node.line, "col": node.col, "declaredType": None, "scope": scope_id})
                record_ref(el, node)
            else:
                walk_unpack_target(el, node, scope_id)

    def register_function(fn_node, methods_list=None, is_static=False):
        own_scope_id = f"L{fn_node.line}C{fn_node.col}"
        end_line = _deep_max_line(fn_node.block) or fn_node.line
        entry = {
            "name": fn_node.name,
            "params": list(fn_node.params),
            "defaults": list((fn_node.defaults or {}).keys()),
            "returnType": fn_node.return_type,
            "isAsync": bool(fn_node.is_async),
            "isStatic": is_static,
            "line": fn_node.line,
            "col": fn_node.col,
            "scopeId": own_scope_id,
            "docstring": _extract_docstring(source_lines, fn_node.line),
        }
        (methods_list if methods_list is not None else functions).append(entry)
        scopes.append({"id": own_scope_id, "startLine": fn_node.line, "endLine": end_line})
        record_ref(fn_node.name, fn_node)
        # Os PARÂMETROS da função só existem dentro do próprio corpo dela —
        # ficam visíveis apenas no escopo que estamos abrindo agora. Chama
        # record_ref aqui (igual VarDecl/Assignment fazem pra si mesmos) pra
        # a própria declaração já contar como 1ª ocorrência — senão um
        # parâmetro usado só 1 vez no corpo (o caso mais comum!) seria
        # marcado como "não usado" por engano.
        for p in fn_node.params:
            variables.append({"name": p, "line": fn_node.line, "col": fn_node.col, "declaredType": None, "scope": own_scope_id})
            record_ref(p, fn_node)
        walk_block(fn_node.block, own_scope_id)

    def collect_self_fields(block, seen_names, out_fields):
        """Percorre um corpo de método coletando `self.nome = ...` (em
        qualquer nível de aninhamento) como campos inferidos — equivalente ao
        que o extractEntityMembers() (regex) do extension.js já fazia, só que
        agora via AST real."""
        if block is None:
            return

        def visit_stmt(s):
            t = _node_type(s)
            if t == "MemberAssignment":
                target = s.target
                if _node_type(target) == "Name" and target.value == "self" and s.member not in seen_names:
                    seen_names.add(s.member)
                    out_fields.append({"name": s.member, "type": None, "line": s.line, "col": s.col})
            elif t == "IfStmt":
                for b in s.branches:
                    visit_block(b.block)
            elif t == "WhileStmt":
                visit_block(s.block)
            elif t == "ForEachStmt":
                visit_block(s.block)
            elif t == "TryCatchStmt":
                visit_block(s.try_block)
                for c in s.catches:
                    visit_block(c.block)
                visit_block(s.finally_block)
            elif t == "MatchStmt":
                for c in s.cases:
                    visit_block(c.body)

        def visit_block(b):
            if b is None:
                return
            for s in b.statements:
                visit_stmt(s)

        visit_block(block)

    def register_entity(node):
        fields = []
        seen_field_names = set()
        if node.fields:
            for f in node.fields:
                fields.append({"name": f.field_name, "type": f.type_name, "line": f.line, "col": f.col})
                seen_field_names.add(f.field_name)

        methods = []
        method_blocks = []
        pending_static = False
        for item in node.body:
            t = _node_type(item)
            if t == "DecoratorStmt":
                if item.block is None:
                    # `@static` seguido do `action` como próximo item do body
                    pending_static = list(getattr(item.decorator, "path", [])) == ["static"]
                    continue
                # decorator que capturou a action dentro do próprio bloco (ex: @NonNull)
                for s in item.block.statements:
                    if _node_type(s) == "ActionDecl":
                        register_function(s, methods_list=methods, is_static=pending_static)
                        method_blocks.append(s.block)
                pending_static = False
                continue
            if t == "ActionDecl":
                register_function(item, methods_list=methods, is_static=pending_static)
                method_blocks.append(item.block)
                pending_static = False
                continue
            pending_static = False

        # Campos inferidos de `self.x = ...` dentro de qualquer método
        # (inclusive __init__) — não substitui campos tipados já declarados.
        for mb in method_blocks:
            collect_self_fields(mb, seen_field_names, fields)

        entities.append({
            "name": node.name,
            "parents": list(node.parents),
            "line": node.line,
            "col": node.col,
            "fields": fields,
            "methods": methods,
        })
        record_ref(node.name, node)

    def register_import(node):
        imports.append({
            "module": ".".join(node.module),
            "alias": node.module_alias,
            "names": [{"name": n, "alias": node.name_aliases.get(n)} for n in node.names],
            "mode": node.mode,
            "line": node.line,
            "col": node.col,
            # 0 = import absoluto (resolve a partir da raiz do projeto);
            # N>0 = import relativo (`from .x` / `from ..pkg.x`), N pontos —
            # ver Interpreter._exec_import no poolscript-lang.
            "level": getattr(node, "level", 0),
        })

    def walk_stmt(node, scope_id):
        t = _node_type(node)
        if t == "ActionDecl":
            register_function(node)
        elif t == "EntityDecl":
            register_entity(node)
        elif t == "ImportStmt":
            register_import(node)
        elif t == "VarDecl":
            variables.append({
                "name": node.name, "line": node.line, "col": node.col,
                "declaredType": node.declared_type, "scope": scope_id,
                "inferredType": infer_type(node.value),
            })
            record_ref(node.name, node)
            walk_expr(node.value)
        elif t == "Assignment":
            variables.append({
                "name": node.target, "line": node.line, "col": node.col,
                "declaredType": None, "scope": scope_id,
                "inferredType": infer_type(node.value),
            })
            record_ref(node.target, node)
            walk_expr(node.value)
        elif t == "UnpackAssignment":
            walk_unpack_target(node.targets, node, scope_id)
            walk_expr(node.value)
        elif t == "MemberAssignment":
            walk_expr(node.target)
            walk_expr(node.value)
        elif t == "ExpressionStmt":
            walk_expr(node.expression)
        elif t == "IfStmt":
            for b in node.branches:
                walk_expr(b.condition)
                walk_block(b.block, scope_id)
        elif t == "WhileStmt":
            walk_expr(node.condition)
            walk_block(node.block, scope_id)
        elif t == "ForEachStmt":
            walk_expr(node.iterable)
            # `for each item in ...` — `item` só existe dentro do corpo do loop.
            variables.append({"name": node.item_name, "line": node.line, "col": node.col, "declaredType": None, "scope": scope_id})
            record_ref(node.item_name, node)
            walk_block(node.block, scope_id)
        elif t == "TryCatchStmt":
            walk_block(node.try_block, scope_id)
            for c in node.catches:
                variables.append({"name": c.error_name, "line": c.line, "col": c.col, "declaredType": None, "scope": scope_id})
                record_ref(c.error_name, c)
                walk_block(c.block, scope_id)
            walk_block(node.finally_block, scope_id)
        elif t == "ReturnStmt":
            walk_expr(node.value)
        elif t == "RaiseStmt":
            walk_expr(node.value)
        elif t == "YieldStmt":
            walk_expr(node.value)
        elif t == "DecoratorStmt":
            if node.block:
                for s in node.block.statements:
                    walk_stmt(s, scope_id)
        elif t == "MatchStmt":
            walk_expr(node.subject)
            for c in node.cases:
                walk_block(c.body, scope_id)
        elif t == "UsingStmt":
            walk_expr(node.resource)
            variables.append({"name": node.var_name, "line": node.line, "col": node.col, "declaredType": None, "scope": scope_id})
            record_ref(node.var_name, node)
            walk_block(node.block, scope_id)
        elif t == "CountEachStmt":
            walk_expr(node.container)
            walk_expr(node.value_node)
            walk_block(node.block, scope_id)
        elif t == "RunSelfWithStmt":
            walk_block(node.block, scope_id)
        # ContinueStmt/BreakStmt/ModelDecl e outros: nada a percorrer nesta v1

    def walk_block(block, scope_id):
        if block is None:
            return
        for s in block.statements:
            walk_stmt(s, scope_id)

    for stmt in program.statements:
        walk_stmt(stmt, "module")

    return {
        "ok": True,
        "errors": [],
        "functions": functions,
        "entities": entities,
        "variables": variables,
        "imports": imports,
        "references": dict(references),
        "scopes": scopes,
    }


def main():
    for raw_line in sys.stdin:
        raw_line = raw_line.strip()
        if not raw_line:
            continue
        try:
            request = json.loads(raw_line)
            result = analyze_source(request.get("text", ""))
            result["id"] = request.get("id")
        except Exception as e:  # protocolo malformado ou erro totalmente inesperado
            result = {"id": None, "ok": False, "errors": [{"line": 1, "col": 1, "message": f"bridge error: {e}"}]}
        sys.stdout.write(json.dumps(result) + "\n")
        sys.stdout.flush()


if __name__ == "__main__":
    main()
