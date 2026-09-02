/*
 * Binding TEMPORÁRIO do parser em C — só para o teste diferencial.
 *
 * Serializa a AST em S-expression. O lado Python gera o mesmo formato a
 * partir dos dataclasses, e o teste exige igualdade textual: qualquer
 * diferença de estrutura, ordem de filhos ou associatividade aparece na
 * hora, e a mensagem de falha mostra exatamente onde.
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "ps_lexer.h"
#include "ps_parser.h"

typedef struct { char *b; size_t n, cap; } SBuf;

static int sb_put(SBuf *s, const char *txt, size_t n)
{
    if (s->n + n + 1 > s->cap) {
        size_t novo = s->cap < 256 ? 256 : s->cap;
        while (s->n + n + 1 > novo) novo *= 2;
        char *p = realloc(s->b, novo);
        if (!p) return -1;
        s->b = p; s->cap = novo;
    }
    memcpy(s->b + s->n, txt, n);
    s->n += n;
    s->b[s->n] = '\0';
    return 0;
}

static int sb_str(SBuf *s, const char *txt) { return sb_put(s, txt, strlen(txt)); }

static int serializa(SBuf *s, const PSNode *n);

static int serializa_lista(SBuf *s, const PSNodeVec *v)
{
    if (sb_str(s, "[") != 0) return -1;
    for (int32_t i = 0; i < v->n; i++) {
        if (i && sb_str(s, " ") != 0) return -1;
        if (serializa(s, v->itens[i]) != 0) return -1;
    }
    return sb_str(s, "]");
}

static int serializa(SBuf *s, const PSNode *n)
{
    if (!n) return sb_str(s, "nil");

    char tmp[512];
    if (sb_str(s, "(") != 0) return -1;
    if (sb_str(s, ps_node_nome(n->kind)) != 0) return -1;

    switch (n->kind) {
        case N_LITERAL:
            switch (n->lit) {
                case L_INT:  snprintf(tmp, sizeof(tmp), " int %lld", (long long)n->i); break;
                case L_FLO:  snprintf(tmp, sizeof(tmp), " flo %.17g", n->d); break;
                case L_BOOL: snprintf(tmp, sizeof(tmp), " bool %s", n->i ? "True" : "False"); break;
                case L_NULL: snprintf(tmp, sizeof(tmp), " null"); break;
                case L_STR:  snprintf(tmp, sizeof(tmp), " str "); break;
                case L_FSTRING: snprintf(tmp, sizeof(tmp), " fstring "); break;
                /* bigint = int arbitrário; o lado Python também serializa como
                 * `int <valor>` (Python int já é bignum), então casa o formato. */
                case L_BIGINT: snprintf(tmp, sizeof(tmp), " int %s", n->texto ? n->texto : "0"); break;
            }
            if (sb_str(s, tmp) != 0) return -1;
            if (n->lit == L_STR || n->lit == L_FSTRING) {
                if (sb_str(s, "\"") != 0) return -1;
                if (sb_str(s, n->texto ? n->texto : "") != 0) return -1;
                if (sb_str(s, "\"") != 0) return -1;
            }
            break;

        case N_NAME:
        case N_MEMBER_ACCESS:
        case N_UNARY_OP:
        case N_BINARY_OP:
        case N_FOR_EACH_STMT:
        case N_TYPE_NAME:
        case N_POSTFIX_OP:
        case N_COLOR_STR_EXPR:
        case N_RUN_SELFWITH_STMT:
        case N_ENUM_DECL:
        case N_ENUM_MEMBER:
            snprintf(tmp, sizeof(tmp), " %s", n->texto ? n->texto : "");
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_VAR_DECL:
        case N_ASSIGNMENT:
        case N_FIELD_DECL:
            snprintf(tmp, sizeof(tmp), " %s %s", n->texto2 ? n->texto2 : "",
                     n->texto ? n->texto : "");
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_ACTION_DECL:
            snprintf(tmp, sizeof(tmp), " %s%s%s", n->texto ? n->texto : "",
                     n->is_async ? " async" : "",
                     n->texto2 ? " " : "");
            if (sb_str(s, tmp) != 0) return -1;
            if (n->texto2 && sb_str(s, n->texto2) != 0) return -1;
            break;

        case N_CALL_ARG:
            if (n->texto) {
                snprintf(tmp, sizeof(tmp), " %s=", n->texto);
                if (sb_str(s, tmp) != 0) return -1;
            }
            break;

        case N_BLOCK:
            snprintf(tmp, sizeof(tmp), " %s", n->estilo ? n->estilo : "");
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_CATCH_CLAUSE:
            snprintf(tmp, sizeof(tmp), " %s %s", n->texto ? n->texto : "",
                     n->texto2 ? n->texto2 : "nil");
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_USING_STMT:
            snprintf(tmp, sizeof(tmp), " %s", n->texto ? n->texto : "");
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_UNPACK_TARGET:
            snprintf(tmp, sizeof(tmp), " %d %d", n->i2, n->is_async);
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_ENTITY_DECL:
        case N_INDEX_ASSIGNMENT:
        case N_MEMBER_ASSIGNMENT:
            snprintf(tmp, sizeof(tmp), " %s", n->texto ? n->texto : "");
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_ENTITY_FIELD:
            snprintf(tmp, sizeof(tmp), " %s %s", n->texto ? n->texto : "",
                     n->texto2 ? n->texto2 : "");
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_IMPORT_STMT:
            snprintf(tmp, sizeof(tmp), " %s %s %d", n->texto ? n->texto : "",
                     n->texto2 ? n->texto2 : "nil", n->i2);
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_MODEL_DECL:
            snprintf(tmp, sizeof(tmp), " %s", n->texto ? n->texto : "");
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_MODEL_FIELD:
            if (n->i2 >= 0) snprintf(tmp, sizeof(tmp), " %s %s %d",
                                     n->texto ? n->texto : "", n->texto2 ? n->texto2 : "", n->i2);
            else            snprintf(tmp, sizeof(tmp), " %s %s nil",
                                     n->texto ? n->texto : "", n->texto2 ? n->texto2 : "");
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_COUNT_EXPR:
            snprintf(tmp, sizeof(tmp), " %s %s", n->texto ? n->texto : "",
                     n->texto2 ? n->texto2 : "");
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_COUNT_EACH_STMT:
        case N_COUNT_EACH_EXPR:
            snprintf(tmp, sizeof(tmp), " %s", n->texto ? n->texto : "");
            if (sb_str(s, tmp) != 0) return -1;
            break;

        case N_MATCH_PATTERN:
            snprintf(tmp, sizeof(tmp), " %s %s", n->texto ? n->texto : "",
                     n->texto2 ? n->texto2 : "nil");
            if (sb_str(s, tmp) != 0) return -1;
            break;

        default:
            break;
    }

    /* filhos, na ordem canônica */
    if (n->a) { if (sb_str(s, " ") != 0 || serializa(s, n->a) != 0) return -1; }
    else if (n->kind == N_IF_BRANCH || n->kind == N_RETURN_STMT
             || n->kind == N_YIELD_STMT || n->kind == N_SLICE_ACCESS
             || n->kind == N_MATCH_PATTERN || n->kind == N_ENUM_MEMBER) {
        if (sb_str(s, " nil") != 0) return -1;
    }
    if (n->b) { if (sb_str(s, " ") != 0 || serializa(s, n->b) != 0) return -1; }
    else if (n->kind == N_SLICE_ACCESS || n->kind == N_MATCH_PATTERN
             || n->kind == N_COUNT_EXPR || n->kind == N_COUNT_EACH_EXPR
             || n->kind == N_COUNT_EACH_STMT) {
        if (sb_str(s, " nil") != 0) return -1;
    }

    /* ternário: 3º filho (else) sai depois de a (then) e b (cond) */
    if (n->kind == N_CONDITIONAL && n->c) {
        if (sb_str(s, " ") != 0 || serializa(s, n->c) != 0) return -1;
    }

    /* MatchPattern: guard e valor saem como `a`/`b` na parte genérica
     * acima; aqui só entram as chaves (dict) e os subpadrões. */
    if (n->kind == N_MATCH_PATTERN) {
        if (sb_str(s, " ") != 0 || serializa_lista(s, &n->lista2) != 0) return -1;
        if (sb_str(s, " ") != 0 || serializa_lista(s, &n->lista) != 0) return -1;
        goto fim_lista;
    }
    if (n->kind == N_ENUM_DECL) {
        if (sb_str(s, " ") != 0 || serializa_lista(s, &n->lista) != 0) return -1;   /* membros */
        goto fim_lista;
    }
    if (n->kind == N_ENTITY_DECL) {
        if (sb_str(s, " ") != 0 || serializa_lista(s, &n->lista2) != 0) return -1;      /* pais */
        if (sb_str(s, " ") != 0 || serializa_lista(s, &n->lista) != 0) return -1;       /* corpo */
        if (sb_str(s, " ") != 0 || serializa_lista(s, &n->lista2_alias) != 0) return -1;/* campos */
        goto fim_lista;
    }
    if (n->kind == N_ENTITY_FIELD && !n->a) { if (sb_str(s, " nil") != 0) return -1; }
    if (n->kind == N_IMPORT_STMT) {
        if (sb_str(s, " ") != 0 || serializa_lista(s, &n->lista) != 0) return -1;
        if (sb_str(s, " ") != 0 || serializa_lista(s, &n->lista2) != 0) return -1;
        if (sb_str(s, " ") != 0 || serializa_lista(s, &n->lista2_alias) != 0) return -1;
        goto fim_lista;
    }
    if (n->kind == N_DECORATOR_CALL) {
        if (sb_str(s, " ") != 0 || serializa_lista(s, &n->lista) != 0) return -1;
        if (sb_str(s, " ") != 0 || serializa_lista(s, &n->lista2) != 0) return -1;
        goto fim_lista;
    }
    if (n->kind == N_DECORATOR_STMT && !n->b) {
        if (sb_str(s, " nil") != 0) return -1;
    }
    if (n->kind == N_COUNT_EXPR || n->kind == N_COUNT_EACH_EXPR
            || n->kind == N_COUNT_EACH_STMT) {
        if (sb_str(s, " ") != 0 || serializa(s, n->c) != 0) return -1;   /* container */
        if (n->kind == N_COUNT_EACH_STMT) {
            if (sb_str(s, " ") != 0 || serializa(s, n->e) != 0) return -1;  /* bloco */
        }
        goto fim_lista;
    }
    if (n->kind == N_TRY_CATCH_STMT) {
        if (sb_str(s, " ") != 0 || serializa_lista(s, &n->lista) != 0) return -1;
        if (sb_str(s, " ") != 0 || serializa(s, n->c) != 0) return -1;
    }
    /* IndexAssignment: o valor mora em `c`, depois do container e do índice */
    if (n->kind == N_INDEX_ASSIGNMENT) {
        if (sb_str(s, " ") != 0 || serializa(s, n->c) != 0) return -1;
    }
    /* SliceAccess tem 4 filhos: os slots c/e vêm depois de a/b */
    if (n->kind == N_SLICE_ACCESS) {
        if (sb_str(s, " ") != 0 || serializa(s, n->c) != 0) return -1;
        if (sb_str(s, " ") != 0 || serializa(s, n->e) != 0) return -1;
    }
    if (n->lista.n > 0 || n->kind == N_PROGRAM || n->kind == N_BLOCK
            || n->kind == N_LIST_LITERAL || n->kind == N_DICT_LITERAL
            || n->kind == N_TUPLE_LITERAL
            || n->kind == N_CALL || n->kind == N_IF_STMT || n->kind == N_ACTION_DECL
            || n->kind == N_LAMBDA_EXPR || n->kind == N_GLOBAL_STMT
            || n->kind == N_MATCH_STMT || n->kind == N_INTERPOLATED_STRING
            || n->kind == N_MODEL_DECL || n->kind == N_BASE_CALL_NODE
            || n->kind == N_UNPACK_TARGET) {
        if (n->kind == N_TRY_CATCH_STMT) goto fim_lista;
        if (sb_str(s, " ") != 0) return -1;
        if (serializa_lista(s, &n->lista) != 0) return -1;
    }
fim_lista:

    return sb_str(s, ")");
}

static PyObject *bind_parse(PyObject *self, PyObject *args)
{
    const char *fonte;
    Py_ssize_t len;
    if (!PyArg_ParseTuple(args, "s#", &fonte, &len)) return NULL;

    PSTokenList *toks = ps_lexer_tokenize(fonte, (size_t)len);
    if (!toks) return PyErr_NoMemory();
    if (!toks->ok) {
        PyObject *e = Py_BuildValue("(sii)", toks->erro, toks->erro_linha, toks->erro_col);
        ps_lexer_free(toks);
        if (!e) return NULL;
        PyErr_SetObject(PyExc_SyntaxError, e);
        Py_DECREF(e);
        return NULL;
    }

    PSParseResult *r = ps_parse(toks->tokens, toks->n);
    ps_lexer_free(toks);
    if (!r) return PyErr_NoMemory();

    if (!r->ok) {
        PyObject *e = Py_BuildValue("(sii)", r->erro, r->erro_linha, r->erro_col);
        ps_parse_free(r);
        if (!e) return NULL;
        PyErr_SetObject(PyExc_SyntaxError, e);
        Py_DECREF(e);
        return NULL;
    }

    SBuf s = {0};
    int rc = serializa(&s, r->programa);
    ps_parse_free(r);
    if (rc != 0) { free(s.b); return PyErr_NoMemory(); }

    PyObject *saida = PyUnicode_FromStringAndSize(s.b ? s.b : "", (Py_ssize_t)s.n);
    free(s.b);
    return saida;
}

static PyMethodDef metodos[] = {
    {"parse_sexp", bind_parse, METH_VARARGS,
     "parse_sexp(fonte) -> S-expression da AST"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef modulo = {
    PyModuleDef_HEAD_INIT, "ps_parser_c",
    "binding temporario do parser em C (teste diferencial)", -1, metodos
};

PyMODINIT_FUNC PyInit_ps_parser_c(void) { return PyModule_Create(&modulo); }
