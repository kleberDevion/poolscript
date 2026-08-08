"""Serializa a AST do parser Python em S-expression.

Formato idêntico ao produzido por `ps_parser_bind.c`. É o que permite o teste
diferencial: as duas implementações geram texto, e o teste exige igualdade —
qualquer diferença de estrutura, ordem de filhos ou associatividade aparece
imediatamente, com o ponto exato visível no diff.

Não é utilitário de produção; existe só enquanto os dois parsers coexistem.
"""
from __future__ import annotations

from poolscript import parser as P

# Só o subconjunto que o parser em C cobre nesta etapa. Nó fora daqui
# levanta NaoCoberto, e o teste pula o caso — assim a suíte nunca "passa"
# por comparar duas coisas que ninguém entendeu.
class NaoCoberto(Exception):
    pass


def _lit(no: P.Literal) -> str:
    v = no.value
    if v is None:
        return "(Literal null)"
    if v is True or v is False:
        return f"(Literal bool {'True' if v else 'False'})"
    if isinstance(v, int):
        return f"(Literal int {v})"
    if isinstance(v, float):
        return f"(Literal flo {v:.17g})"
    if isinstance(v, str):
        # f-string continua Literal, distinguida por `kind` — a interpolação
        # é resolvida em tempo de execução
        marca = "fstring" if getattr(no, "kind", None) == "FSTRING" else "str"
        return f'(Literal {marca} "{v}")'
    raise NaoCoberto(f"literal {type(v).__name__}")


def sexp(no) -> str:
    if no is None:
        return "nil"
    c = no.__class__

    if c is P.Program:
        return f"(Program {_lista(no.statements)})"
    if c is P.Literal:
        return _lit(no)
    if c is P.Name:
        return f"(Name {no.value})"
    if c is P.BinaryOp:
        return f"(BinaryOp {no.operator} {sexp(no.left)} {sexp(no.right)})"
    if c is P.Conditional:
        return f"(Conditional {sexp(no.then_val)} {sexp(no.cond)} {sexp(no.else_val)})"
    if c is P.UnaryOp:
        return f"(UnaryOp {no.operator} {sexp(no.operand)})"
    if c is P.Call:
        return f"(Call {sexp(no.callee)} {_lista(no.args)})"
    if c is P.CallArg:
        nome = f" {no.name}=" if no.name else ""
        return f"(CallArg{nome} {sexp(no.value)})"
    if c is P.ListLiteral:
        return f"(ListLiteral {_lista(no.items)})"
    if c is P.DictLiteral:
        return f"(DictLiteral {_lista(no.entries)})"
    if c is P.DictEntry:
        return f"(DictEntry {sexp(no.key)} {sexp(no.value)})"
    if c is P.IndexAccess:
        return f"(IndexAccess {sexp(no.target)} {sexp(no.index)})"
    if c is P.MemberAccess:
        return f"(MemberAccess {no.member} {sexp(no.target)})"
    if c is P.VarDecl:
        return f"(VarDecl {no.declared_type} {no.name} {sexp(no.value)})"
    if c is P.Assignment:
        return f"(Assignment {no.operator} {no.target} {sexp(no.value)})"
    if c is P.ExpressionStmt:
        return f"(ExpressionStmt {sexp(no.expression)})"
    if c is P.Block:
        return f"(Block {no.style} {_lista(no.statements)})"
    if c is P.IfStmt:
        return f"(IfStmt {_lista(no.branches)})"
    if c is P.IfBranch:
        return f"(IfBranch {sexp(no.condition)} {sexp(no.block)})"
    if c is P.WhileStmt:
        return f"(WhileStmt {sexp(no.condition)} {sexp(no.block)})"
    if c is P.ForEachStmt:
        return f"(ForEachStmt {no.item_name} {sexp(no.iterable)} {sexp(no.block)})"
    if c is P.ActionDecl:
        extra = " async" if no.is_async else ""
        tipo = f" {no.return_type}" if no.return_type else ""
        # parâmetro com valor padrão carrega o default junto, igual ao C
        partes = []
        for p in no.params:
            d = (no.defaults or {}).get(p)
            partes.append(f"(Name {p} {sexp(d)})" if d is not None else f"(Name {p})")
        params = "[" + " ".join(partes) + "]"
        return f"(ActionDecl {no.name}{extra}{tipo} {sexp(no.block)} {params})"
    if c is P.ReturnStmt:
        return f"(ReturnStmt {sexp(no.value)})"
    if c is P.BreakStmt:
        return "(BreakStmt)"
    if c is P.ContinueStmt:
        return "(ContinueStmt)"
    if c is P.RaiseStmt:
        return f"(RaiseStmt {sexp(no.value)})"
    if c is P.YieldStmt:
        return f"(YieldStmt {sexp(no.value)})"
    if c is P.GlobalStmt:
        nomes = "[" + " ".join(f"(Name {x})" for x in no.names) + "]"
        return f"(GlobalStmt {nomes})"
    if c is P.RunSelfWithStmt:
        return f"(RunSelfWithStmt {no.label} {sexp(no.block)})"
    if c is P.TupleLiteral:
        return f"(TupleLiteral {_lista(no.items)})"
    if c is P.SliceAccess:
        return (f"(SliceAccess {sexp(no.target)} {sexp(no.start)} "
                f"{sexp(no.stop)} {sexp(no.step)})")
    if c is P.PostfixOp:
        return f"(PostfixOp {no.operator} {sexp(no.operand)})"
    if c is P.TypeName:
        return f"(TypeName {no.name})"
    if c is P.ColorStrExpr:
        return f"(ColorStrExpr {no.color} {sexp(no.expr)})"
    if c is P.LambdaExpr:
        params = "[" + " ".join(f"(Name {x})" for x in no.params) + "]"
        return f"(LambdaExpr {sexp(no.block)} {params})"
    if c is P.AwaitExpr:
        return f"(AwaitExpr {sexp(no.value)})"
    if c is P.TryCatchStmt:
        cat = "[" + " ".join(sexp(x) for x in no.catches) + "]"
        return f"(TryCatchStmt {sexp(no.try_block)} {cat} {sexp(no.finally_block)})"
    if c is P.CatchClause:
        tipo = no.error_type if no.error_type else "nil"
        return f"(CatchClause {no.error_name} {tipo} {sexp(no.block)})"
    if c is P.UsingStmt:
        return f"(UsingStmt {no.var_name} {sexp(no.resource)} {sexp(no.block)})"
    if c is P.UnpackAssignment:
        return f"(UnpackAssignment {sexp(no.targets)} {sexp(no.value)})"
    if c is P.UnpackTarget:
        si = no.star_index if no.star_index is not None else -1
        els = "[" + " ".join(
            (sexp(e) if not isinstance(e, str) else f"(Name {e})") for e in no.elements) + "]"
        return f"(UnpackTarget {si} {1 if no.trailing_comma else 0} {els})"
    if c is P.EntityDecl:
        pais = "[" + " ".join(f"(Name {x})" for x in no.parents) + "]"
        corpo = "[" + " ".join(sexp(x) for x in no.body) + "]"
        campos = "[" + " ".join(sexp(x) for x in (no.fields or [])) + "]"
        return f"(EntityDecl {no.name} {pais} {corpo} {campos})"
    if c is P.EntityField:
        return f"(EntityField {no.field_name} {no.type_name} {sexp(no.default)})"
    if c is P.BaseCall:
        return "(BaseCall " + "[" + " ".join(sexp(x) for x in no.args) + "]" + ")"
    if c is P.IndexAssignment:
        return f"(IndexAssignment {no.operator} {sexp(no.target)} {sexp(no.index)} {sexp(no.value)})"
    if c is P.MemberAssignment:
        return f"(MemberAssignment {no.member} {sexp(no.target)} {sexp(no.value)})"
    if c is P.ImportStmt:
        mod = "[" + " ".join(f"(Name {x})" for x in no.module) + "]"
        nomes = "[" + " ".join(f"(Name {x})" for x in no.names) + "]"
        al = "[" + " ".join(
            (f"(Name {(no.name_aliases or {})[x]})" if x in (no.name_aliases or {}) else "nil")
            for x in no.names) + "]"
        alias_mod = no.module_alias if no.module_alias else "nil"
        return f"(ImportStmt {no.mode} {alias_mod} {no.level} {mod} {nomes} {al})"
    if c is P.DecoratorStmt:
        return f"(DecoratorStmt {sexp(no.decorator)} {sexp(no.block)})"
    if c is P.DecoratorCall:
        cam = "[" + " ".join(f"(Name {x})" for x in no.path) + "]"
        args = "[" + " ".join(sexp(x) for x in (no.args or [])) + "]"
        return f"(DecoratorCall {cam} {args})"
    if c is P.ModelDecl:
        fs = "[" + " ".join(sexp(x) for x in no.fields) + "]"
        return f"(ModelDecl {no.name} {fs})"
    if c is P.ModelField:
        ln = no.length if no.length is not None else "nil"
        return f"(ModelField {no.name} {no.type_name} {ln})"
    if c is P.CountExpr:
        return (f"(CountExpr {no.target_type} {no.mode} "
                f"{sexp(no.value_node)} {sexp(no.container)})")
    if c is P.CountEachExpr:
        return f"(CountEachExpr {no.target_type} {sexp(no.value_node)} {sexp(no.container)})"
    if c is P.CountEachStmt:
        return (f"(CountEachStmt {no.target_type} {sexp(no.value_node)} "
                f"{sexp(no.container)} {sexp(no.block)})")
    if c is P.InterpolatedString:
        return f"(InterpolatedString {_lista(no.parts)})"
    if c is P.MatchStmt:
        cs = "[" + " ".join(sexp(x) for x in no.cases) + "]"
        return f"(MatchStmt {sexp(no.subject)} {cs})"
    if c is P.MatchCase:
        return f"(MatchCase {sexp(no.pattern)} {sexp(no.body)})"
    if c is P.MatchPattern:
        nome = no.name if no.name else "nil"
        # o valor do padrão é serializado como Literal, igual ao C faz
        val = "nil"
        if no.kind == "value":
            v = no.value
            if v is None: val = "(Literal null)"
            elif v is True or v is False: val = f"(Literal bool {'True' if v else 'False'})"
            elif isinstance(v, int): val = f"(Literal int {v})"
            elif isinstance(v, float): val = f"(Literal flo {v:.17g})"
            else: val = f'(Literal str "{v}")'
        chaves = "[" + " ".join(f"(Name {k})" for k in (no.keys or {})) + "]"
        itens = list(no.items or []) or list((no.keys or {}).values()) or list(no.patterns or [])
        subs = "[" + " ".join(sexp(x) for x in itens) + "]"
        return f"(MatchPattern {no.kind} {nome} {sexp(no.guard)} {val} {chaves} {subs})"

    raise NaoCoberto(c.__name__)


def _lista(itens) -> str:
    return "[" + " ".join(sexp(x) for x in itens) + "]"
