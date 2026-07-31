/*
 * Binding TEMPORÁRIO do lexer em C para o Python.
 *
 * Existe só para o teste diferencial da transição: rodar o mesmo fonte pelos
 * dois lexers e exigir token a token igual. O núcleo (ps_lexer.c) não sabe
 * que este arquivo existe — quando o binário standalone estiver pronto, este
 * arquivo é deletado e nada no lexer muda.
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "ps_lexer.h"

static PyObject *bind_tokenize(PyObject *self, PyObject *args)
{
    const char *fonte;
    Py_ssize_t len;
    if (!PyArg_ParseTuple(args, "s#", &fonte, &len)) return NULL;

    PSTokenList *lista = ps_lexer_tokenize(fonte, (size_t)len);
    if (!lista) return PyErr_NoMemory();

    if (!lista->ok) {
        PyObject *e = Py_BuildValue("(sii)", lista->erro, lista->erro_linha, lista->erro_col);
        ps_lexer_free(lista);
        if (!e) return NULL;
        PyErr_SetObject(PyExc_SyntaxError, e);
        Py_DECREF(e);
        return NULL;
    }

    PyObject *saida = PyList_New(lista->n);
    if (!saida) { ps_lexer_free(lista); return NULL; }

    for (int32_t i = 0; i < lista->n; i++) {
        PSToken *t = &lista->tokens[i];
        PyObject *valor = NULL;

        switch (t->type) {
            case T_INT:    valor = PyLong_FromLongLong((long long)t->i); break;
            case T_FLO:    valor = PyFloat_FromDouble(t->d); break;
            case T_BOOL:   valor = PyBool_FromLong((long)t->i); break;
            case T_NULL:   Py_INCREF(Py_None); valor = Py_None; break;
            case T_INDENT:
            case T_DEDENT: valor = PyLong_FromLongLong((long long)t->i); break;
            case T_NEWLINE: Py_INCREF(Py_None); valor = Py_None; break;
            case T_EOF:    Py_INCREF(Py_None); valor = Py_None; break;
            default:
                valor = t->texto ? PyUnicode_FromStringAndSize(t->texto, t->texto_len)
                                 : PyUnicode_FromString("");
                break;
        }
        if (!valor) { Py_DECREF(saida); ps_lexer_free(lista); return NULL; }

        PyObject *tupla = Py_BuildValue("(sNii)", ps_tok_nome(t->type), valor,
                                        t->line, t->col);
        if (!tupla) { Py_DECREF(saida); ps_lexer_free(lista); return NULL; }
        PyList_SET_ITEM(saida, i, tupla);
    }

    ps_lexer_free(lista);
    return saida;
}

static PyMethodDef metodos[] = {
    {"tokenize", bind_tokenize, METH_VARARGS,
     "tokenize(fonte) -> [(tipo, valor, linha, coluna), ...]"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef modulo = {
    PyModuleDef_HEAD_INIT, "ps_lexer_c",
    "binding temporario do lexer em C (teste diferencial)", -1, metodos
};

PyMODINIT_FUNC PyInit_ps_lexer_c(void) { return PyModule_Create(&modulo); }
