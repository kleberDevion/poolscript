/*
 * Parser da Jinga em C puro.
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
/* Lê E parseia o fonte sem nunca ter o vetor de tokens do arquivo inteiro: o
 * lexer produz cada token quando o parser pede, e os de uma declaração de
 * topo terminada saem da memória (ver `fluxo` em ps_lexer.h). O resultado é o
 * mesmo de `ps_lexer_tokenize_modo(fonte, len, 0, recupera)` seguido de
 * `ps_parse_lista`, inclusive a segunda passada. Em `*lexer` volta a lista do
 * lexer — sem os tokens, com `ok`, `erro`, `avisos`, `erros` e `fechados` do
 * arquivo inteiro —, que o chamador libera com `ps_lexer_free`. Se `ok` dela
 * for 0, o arquivo não vale e a árvore não deve ser usada (é o erro do lexer
 * que se reporta, como antes). */
PSParseResult *ps_parse_fonte(const char *fonte, size_t len, int recupera, PSTokenList **lexer);
void           ps_parse_free(PSParseResult *r);

/* PARSER INCREMENTAL: uma declaração de topo por vez, e a anterior morre
 * quando a seguinte é pedida. É o que deixa o compilador ler um arquivo de
 * qualquer tamanho sem ter a árvore inteira na memória: ele guarda só o que
 * precisa de cada declaração e pede a próxima. O lexer é o de fluxo (ver
 * `fluxo` em ps_lexer.h), então nem o vetor de tokens existe inteiro.
 *
 * `ps_parse_inc_proxima` devolve a próxima declaração (na arena do parser,
 * reaproveitada na chamada seguinte) ou NULL no fim — ou quando o parse
 * parou num erro, fora do modo de recuperação. Depois do NULL, o resultado
 * (`ok`, `erro`, `erros`) e a lista do lexer (`ok`, `erro`, `avisos`,
 * `erros`, `fechados`) são os do arquivo inteiro. `ps_parse_inc_fecha` libera
 * tudo; com `lexer` não-NULL, entrega a lista do lexer em vez de liberá-la.
 *
 * Não faz a segunda passada de sincronia (ver `ps_lexer_tokenize_modo`): no
 * modo de recuperação com grupo sem par, a lista definitiva de erros é a de
 * `ps_parse_fonte`. */
typedef struct PSParserInc PSParserInc;
PSParserInc   *ps_parse_inc_abre(const char *fonte, size_t len, int recupera);
PSNode        *ps_parse_inc_proxima(PSParserInc *pi);
PSParseResult *ps_parse_inc_resultado(PSParserInc *pi);
PSTokenList   *ps_parse_inc_lexer(PSParserInc *pi);
void           ps_parse_inc_fecha(PSParserInc *pi, PSTokenList **lexer);

/* Os tipos que abrem declaração (`list l = []`), terminados em NULL. Fonte
 * única: a tabela que o próprio parser consulta; o `--metadata` a publica. */
const char *const *ps_parser_tipos_decl(void);

#endif /* PS_PARSER_H */
