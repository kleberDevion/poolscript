/*
 * Parser da PoolScript em C puro.
 */
#ifndef PS_PARSER_H
#define PS_PARSER_H

#include "ps_ast.h"
#include "ps_lexer.h"

typedef struct {
    PSNode  *programa;      /* vive na arena abaixo */
    PSArena  arena;

    int      ok;
    char     erro[256];
    int32_t  erro_linha;
    int32_t  erro_col;
} PSParseResult;

/* Consome os tokens do lexer. O resultado sempre precisa ser liberado com
 * ps_parse_free, inclusive em caso de erro — a arena leva a AST parcial
 * junto, então não há vazamento no caminho de falha. */
PSParseResult *ps_parse(PSToken *toks, int32_t n);
void           ps_parse_free(PSParseResult *r);

#endif /* PS_PARSER_H */
