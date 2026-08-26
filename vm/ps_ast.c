/*
 * Arena e construtores de nó da AST. C puro.
 */
#include "ps_ast.h"

#include <stdlib.h>
#include <string.h>

#define BLOCO_PADRAO (64 * 1024)

struct PSArenaBloco {
    PSArenaBloco *prox;
    size_t        cap;
    size_t        usado;
    char          dados[];
};

void ps_arena_init(PSArena *a)
{
    a->blocos = NULL;
    a->total = 0;
}

void *ps_arena_alloc(PSArena *a, size_t n)
{
    n = (n + 15u) & ~(size_t)15u;          /* alinha em 16 */

    if (!a->blocos || a->blocos->usado + n > a->blocos->cap) {
        size_t cap = n > BLOCO_PADRAO ? n : BLOCO_PADRAO;
        PSArenaBloco *b = malloc(sizeof(PSArenaBloco) + cap);
        if (!b) return NULL;
        b->prox = a->blocos;
        b->cap = cap;
        b->usado = 0;
        a->blocos = b;
    }
    void *p = a->blocos->dados + a->blocos->usado;
    a->blocos->usado += n;
    a->total += n;
    return p;
}

char *ps_arena_strdup(PSArena *a, const char *s, int n)
{
    char *p = ps_arena_alloc(a, (size_t)n + 1);
    if (!p) return NULL;
    if (n > 0) memcpy(p, s, (size_t)n);
    p[n] = '\0';
    return p;
}

void ps_arena_free(PSArena *a)
{
    PSArenaBloco *b = a->blocos;
    while (b) {
        PSArenaBloco *prox = b->prox;
        free(b);
        b = prox;
    }
    a->blocos = NULL;
    a->total = 0;
}

PSNode *ps_node_novo(PSArena *a, PSNodeKind k, int32_t line, int32_t col)
{
    PSNode *n = ps_arena_alloc(a, sizeof(PSNode));
    if (!n) return NULL;
    memset(n, 0, sizeof(*n));
    n->kind = k;
    n->line = line;
    n->col = col;
    return n;
}

int ps_vec_push(PSArena *a, PSNodeVec *v, PSNode *no)
{
    if (v->n + 1 > v->cap) {
        int32_t novo = v->cap < 8 ? 8 : v->cap * 2;
        /* A arena não realoca no lugar: copia pro bloco novo e abandona o
         * antigo. Desperdiça um pouco, mas mantém o free trivial (um único
         * ps_arena_free no fim) — trade justo pra uma estrutura efêmera. */
        PSNode **itens = ps_arena_alloc(a, sizeof(PSNode *) * (size_t)novo);
        if (!itens) return -1;
        if (v->n > 0) memcpy(itens, v->itens, sizeof(PSNode *) * (size_t)v->n);
        v->itens = itens;
        v->cap = novo;
    }
    v->itens[v->n++] = no;
    return 0;
}

const char *ps_node_nome(PSNodeKind k)
{
    switch (k) {
        case N_PROGRAM:         return "Program";
        case N_LITERAL:         return "Literal";
        case N_NAME:            return "Name";
        case N_BINARY_OP:       return "BinaryOp";
        case N_CONDITIONAL:     return "Conditional";
        case N_UNARY_OP:        return "UnaryOp";
        case N_CALL:            return "Call";
        case N_CALL_ARG:        return "CallArg";
        case N_LIST_LITERAL:    return "ListLiteral";
        case N_LIST_COMP:       return "ListComp";
        case N_DICT_LITERAL:    return "DictLiteral";
        case N_DICT_ENTRY:      return "DictEntry";
        case N_INDEX_ACCESS:    return "IndexAccess";
        case N_MEMBER_ACCESS:   return "MemberAccess";
        case N_VAR_DECL:        return "VarDecl";
        case N_ASSIGNMENT:      return "Assignment";
        case N_EXPRESSION_STMT: return "ExpressionStmt";
        case N_BLOCK:           return "Block";
        case N_IF_STMT:         return "IfStmt";
        case N_IF_BRANCH:       return "IfBranch";
        case N_WHILE_STMT:      return "WhileStmt";
        case N_FOR_EACH_STMT:   return "ForEachStmt";
        case N_ACTION_DECL:     return "ActionDecl";
        case N_RETURN_STMT:     return "ReturnStmt";
        case N_BREAK_STMT:      return "BreakStmt";
        case N_CONTINUE_STMT:   return "ContinueStmt";
        case N_PASS_STMT:       return "PassStmt";
        case N_RAISE_STMT:      return "RaiseStmt";
        case N_YIELD_STMT:      return "YieldStmt";
        case N_GLOBAL_STMT:     return "GlobalStmt";
        case N_RUN_SELFWITH_STMT: return "RunSelfWithStmt";
        case N_TUPLE_LITERAL:   return "TupleLiteral";
        case N_SLICE_ACCESS:    return "SliceAccess";
        case N_POSTFIX_OP:      return "PostfixOp";
        case N_TYPE_NAME:       return "TypeName";
        case N_INTERPOLATED_STRING: return "InterpolatedString";
        case N_COLOR_STR_EXPR:  return "ColorStrExpr";
        case N_LAMBDA_EXPR:     return "LambdaExpr";
        case N_AWAIT_EXPR:      return "AwaitExpr";
        case N_TRY_CATCH_STMT:  return "TryCatchStmt";
        case N_CATCH_CLAUSE:    return "CatchClause";
        case N_USING_STMT:      return "UsingStmt";
        case N_MATCH_STMT:      return "MatchStmt";
        case N_MATCH_CASE:      return "MatchCase";
        case N_MATCH_PATTERN:   return "MatchPattern";
        case N_MODEL_DECL:      return "ModelDecl";
        case N_MODEL_FIELD:     return "ModelField";
        case N_ENUM_DECL:       return "EnumDecl";
        case N_ENUM_MEMBER:     return "EnumMember";
        case N_COUNT_EXPR:      return "CountExpr";
        case N_COUNT_EACH_STMT: return "CountEachStmt";
        case N_COUNT_EACH_EXPR: return "CountEachExpr";
        case N_IMPORT_STMT:     return "ImportStmt";
        case N_DECORATOR_STMT:  return "DecoratorStmt";
        case N_DECORATOR_CALL:  return "DecoratorCall";
        case N_ENTITY_DECL:     return "EntityDecl";
        case N_ENTITY_FIELD:    return "EntityField";
        case N_BASE_CALL_NODE:  return "BaseCall";
        case N_MEMBER_ASSIGNMENT: return "MemberAssignment";
        case N_INDEX_ASSIGNMENT: return "IndexAssignment";
        case N_UNPACK_ASSIGNMENT: return "UnpackAssignment";
        case N_UNPACK_TARGET:   return "UnpackTarget";
    }
    return "?";
}
