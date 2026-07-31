/*
 * Compilador AST → bytecode da PoolScript, em C puro.
 *
 * Fronteira de acoplamento: o compilador NÃO conhece `Value`, `Obj` nem o
 * GC da VM. Ele emite constantes numa forma neutra (`PSConst`), e é a VM que
 * as converte em valores gerenciados ao carregar. Sem isso o compilador
 * precisaria alocar objetos sob o coletor antes de existir uma VM — e
 * strings de constante nasceriam como lixo coletável no meio da compilação.
 *
 * A tabela de protótipos já sai PLANA (o achatamento que flatten.py fazia em
 * Python acontece aqui direto): `MAKE_FUNCTION` carrega índice de protótipo,
 * não índice de constante.
 */
#ifndef PS_COMPILER_H
#define PS_COMPILER_H

#include <stdint.h>

#include "ps_ast.h"

/* Constante numa forma que não depende do runtime. */
typedef enum { K_NULL = 0, K_BOOL, K_INT, K_FLO, K_STR } PSConstKind;

typedef struct {
    PSConstKind kind;
    int64_t     i;      /* K_INT, K_BOOL */
    double      d;      /* K_FLO */
    char       *s;      /* K_STR — dono da memória */
    int32_t     slen;
} PSConst;

typedef struct {
    char    *nome;
    int32_t *code;      /* pares [opcode, arg] */
    int32_t  ncode;
    PSConst *consts;
    int32_t  nconsts;
    int32_t  nlocals;
    int32_t  nparams;
    int32_t  ndefaults;   /* quantos parâmetros finais têm valor padrão */
    int32_t  eh_gerador;  /* contém `yield` — chamar cria gerador, não frame */
    /* Nome de cada parâmetro, na ordem. Só existe pra resolver argumento
     * nomeado em runtime — o call site não sabe qual função vai chamar. */
    char   **param_nomes;
} PSProto;

/* Descritor de Entity produzido pela compilação. Os PAIS não entram aqui:
 * eles são resolvidos em runtime (podem ser declarados depois), então o
 * MAKE_CLASS os recebe pela pilha. */
typedef struct {
    char    *nome;
    char   **met_nomes;
    int32_t *met_protos;
    int32_t  nmetodos;
    int32_t  npais;
} PSClassDef;

/* Campo de `model`, na forma neutra do compilador. */
typedef struct {
    char   *nome;
    int32_t tipo;      /* TIPO_* da VM */
    int32_t length;    /* -1 = sem limite */
} PSModelCampoDef;

typedef struct {
    char            *nome;
    PSModelCampoDef *campos;
    int32_t          ncampos;
} PSModelDef;

typedef struct {
    PSProto *protos;    /* índice 0 = módulo */
    int32_t  nprotos;

    char   **globais;   /* nome de cada global, indexado */
    int32_t  nglobais;

    PSClassDef *classes;
    int32_t     nclasses;

    PSModelDef *models;
    int32_t     nmodels;

    int      ok;
    char     erro[256];
    int32_t  erro_linha;
    int32_t  erro_col;
} PSPrograma;

/* Compila a AST. Sempre devolve algo que precisa de ps_compila_free,
 * inclusive em erro. */
PSPrograma *ps_compila(PSNode *programa);
void        ps_compila_free(PSPrograma *p);

/* Nome legível do opcode — usado no desmonte e no teste diferencial. */
const char *ps_op_nome(int32_t op);

#endif /* PS_COMPILER_H */
