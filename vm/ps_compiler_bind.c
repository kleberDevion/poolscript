/*
 * Binding TEMPORÁRIO do compilador em C — só para o teste diferencial.
 *
 * Desmonta o bytecode num texto canônico. O lado Python gera o mesmo formato
 * a partir de `vm/compiler.py` + `vm/flatten.py`, e o teste exige igualdade:
 * qualquer diferença de opcode, argumento, ordem de constante ou índice de
 * local aparece como diff legível.
 *
 * Formato (uma linha por instrução, protótipos separados por cabeçalho):
 *
 *   proto 0 <module> nlocals=0 nparams=0
 *     0 LOAD_CONST 0        ; int 1
 *     2 STORE_GLOBAL 0      ; x
 *     4 HALT 0
 *   proto 1 f nlocals=1 nparams=1
 *     ...
 *
 * O comentário depois de `;` é o que torna a divergência diagnosticável:
 * sem ele, um índice trocado vira só um número diferente.
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "ps_ast.h"
#include "ps_compiler.h"
#include "ps_lexer.h"
#include "ps_parser.h"

typedef struct { char *b; size_t n, cap; } SBuf;

static int sb_put(SBuf *s, const char *txt, size_t n)
{
    if (s->n + n + 1 > s->cap) {
        size_t novo = s->cap < 512 ? 512 : s->cap;
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

static int sb_str(SBuf *s, const char *t) { return sb_put(s, t, strlen(t)); }

/* Opcodes que carregam índice de constante / global / local / proto —
 * decide qual tabela consultar para o comentário. */
enum { B_LOAD_CONST = 0, B_LOAD_LOCAL = 1, B_STORE_LOCAL = 2,
       B_LOAD_GLOBAL = 3, B_STORE_GLOBAL = 4, B_MAKE_FUNCTION = 22 };

static int desmonta_const(SBuf *s, const PSConst *k)
{
    char tmp[256];
    switch (k->kind) {
        case K_NULL: return sb_str(s, "null");
        case K_BOOL: return sb_str(s, k->i ? "bool True" : "bool False");
        case K_INT:  snprintf(tmp, sizeof(tmp), "int %lld", (long long)k->i); return sb_str(s, tmp);
        case K_FLO:  snprintf(tmp, sizeof(tmp), "flo %.17g", k->d); return sb_str(s, tmp);
        case K_STR:
            if (sb_str(s, "str \"") != 0) return -1;
            if (sb_put(s, k->s ? k->s : "", (size_t)k->slen) != 0) return -1;
            return sb_str(s, "\"");
        /* bigint = int arbitrário (dígitos em k->s); o Python dumpa como
         * `int <valor>`. sb_str direto pra não esbarrar no tmp[256]. */
        case K_BIGINT:
            if (sb_str(s, "int ") != 0) return -1;
            return sb_str(s, k->s ? k->s : "0");
    }
    return sb_str(s, "?");
}

static PyObject *bind_compila(PyObject *self, PyObject *args)
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

    PSPrograma *prog = ps_compila(r->programa);
    ps_parse_free(r);              /* a AST já foi consumida */
    if (!prog) return PyErr_NoMemory();
    if (!prog->ok) {
        PyObject *e = Py_BuildValue("(sii)", prog->erro, prog->erro_linha, prog->erro_col);
        ps_compila_free(prog);
        if (!e) return NULL;
        PyErr_SetObject(PyExc_NotImplementedError, e);
        Py_DECREF(e);
        return NULL;
    }

    SBuf s = {0};
    char tmp[512];
    int rc = 0;

    for (int32_t pi = 0; pi < prog->nprotos && rc == 0; pi++) {
        PSProto *p = &prog->protos[pi];
        snprintf(tmp, sizeof(tmp), "proto %d %s nlocals=%d nparams=%d\n",
                 pi, p->nome ? p->nome : "?", p->nlocals, p->nparams);
        if ((rc = sb_str(&s, tmp)) != 0) break;

        for (int32_t i = 0; i < p->ncode && rc == 0; i += 2) {
            int32_t op = p->code[i], arg = p->code[i + 1];
            snprintf(tmp, sizeof(tmp), "  %d %s %d", i, ps_op_nome(op), arg);
            if ((rc = sb_str(&s, tmp)) != 0) break;

            /* comentário: o que o índice significa */
            if (op == B_LOAD_CONST && arg >= 0 && arg < p->nconsts) {
                if ((rc = sb_str(&s, " ; ")) != 0) break;
                if ((rc = desmonta_const(&s, &p->consts[arg])) != 0) break;
            } else if ((op == B_LOAD_GLOBAL || op == B_STORE_GLOBAL)
                       && arg >= 0 && arg < prog->nglobais) {
                snprintf(tmp, sizeof(tmp), " ; %s", prog->globais[arg]);
                if ((rc = sb_str(&s, tmp)) != 0) break;
            } else if (op == B_MAKE_FUNCTION && arg >= 0 && arg < prog->nprotos) {
                snprintf(tmp, sizeof(tmp), " ; -> proto %d %s", arg,
                         prog->protos[arg].nome ? prog->protos[arg].nome : "?");
                if ((rc = sb_str(&s, tmp)) != 0) break;
            }
            if ((rc = sb_str(&s, "\n")) != 0) break;
        }
    }

    ps_compila_free(prog);
    if (rc != 0) { free(s.b); return PyErr_NoMemory(); }

    PyObject *saida = PyUnicode_FromStringAndSize(s.b ? s.b : "", (Py_ssize_t)s.n);
    free(s.b);
    return saida;
}

static PyMethodDef metodos[] = {
    {"desmonta", bind_compila, METH_VARARGS,
     "desmonta(fonte) -> texto do bytecode compilado em C"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef modulo = {
    PyModuleDef_HEAD_INIT, "ps_compiler_c",
    "binding temporario do compilador em C (teste diferencial)", -1, metodos
};

PyMODINIT_FUNC PyInit_ps_compiler_c(void) { return PyModule_Create(&modulo); }
