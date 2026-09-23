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

    /* TODOS os erros de sintaxe, no MODO DE RECUPERAÇÃO (`ps_parse_modo`): o
     * statement que falha é registrado, o parser pula pro próximo e segue. O
     * editor precisa da árvore INTEIRA enquanto o arquivo está pela metade —
     * com a parada no primeiro erro, o painel de estrutura e o "ir pro
     * símbolo" esvaziavam da linha do erro pra baixo. Rodar o programa
     * continua parando no primeiro. */
    PSAviso *erros;
    int32_t  nerros;
    int32_t  cap_erros;
} PSParseResult;

/* Consome os tokens do lexer. O resultado sempre precisa ser liberado com
 * ps_parse_free, inclusive em caso de erro — a arena leva a AST parcial
 * junto, então não há vazamento no caminho de falha. */
PSParseResult *ps_parse(PSToken *toks, int32_t n);
/* Com `recupera`, o parser não para no primeiro erro: registra, pula pro
 * próximo statement e continua, e a árvore sai com o que veio DEPOIS do erro.
 * Só os comandos de editor usam. */
PSParseResult *ps_parse_modo(PSToken *toks, int32_t n, int recupera);
/* Com a LISTA do lexer (e não só o vetor de tokens), a árvore sai mais
 * completa: as expressões de dentro de cada f-string viram nós filhos do
 * literal, com a linha e a coluna REAIS no fonte. Sem a lista (parse de um
 * trecho solto), a f-string continua sendo só o literal. */
PSParseResult *ps_parse_lista(PSTokenList *tl, int recupera);
void           ps_parse_free(PSParseResult *r);

/* Os tipos que abrem declaração (`list l = []`), terminados em NULL. Fonte
 * única: a tabela que o próprio parser consulta; o `--metadata` a publica. */
const char *const *ps_parser_tipos_decl(void);

#endif /* PS_PARSER_H */
