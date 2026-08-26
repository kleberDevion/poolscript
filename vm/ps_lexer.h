/*
 * Lexer da PoolScript em C puro — sem Python.h.
 *
 * Este cabeçalho é a fronteira da etapa "tudo nativo": nada aqui depende do
 * CPython, então o mesmo código serve tanto pra extensão (durante a
 * transição) quanto pro binário standalone (destino final).
 */
#ifndef PS_LEXER_H
#define PS_LEXER_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    T_EOF = 0,
    T_KW, T_IDENT, T_IDENT_UPPER,
    T_INT, T_FLO, T_STR, T_FSTRING, T_BOOL, T_NULL, T_COLOR,
    T_OP,
    T_NEWLINE, T_INDENT, T_DEDENT,
    T_LPAREN, T_RPAREN, T_LBRACE, T_RBRACE, T_LBRACK, T_RBRACK,
    T_COLON, T_SEMI, T_COMMA, T_DOT, T_AT,
    /* Só existe no modo do editor (`ps_lexer_tokenize_editor`): o lexer
     * normal DESCARTA comentário, e o parser nunca vê este tipo. Fica por
     * ultimo pra nao mexer no valor numerico de nenhum outro. */
    T_COMMENT
} PSTokType;

typedef struct {
    PSTokType type;
    int32_t   line;      /* 1-based */
    int32_t   col;       /* 1-based */
    /* Valor já convertido. Qual campo vale depende de `type`:
     *   T_INT           -> i
     *   T_FLO           -> d
     *   T_BOOL          -> i (0/1)
     *   T_INDENT/DEDENT -> i (nível de indentação)
     *   demais textuais -> texto/texto_len */
    int64_t   i;
    double    d;
    char     *texto;     /* dono da memória; NULL quando não se aplica */
    int32_t   texto_len;
    /* Quantos CARACTERES o token ocupa NO FONTE. Não é o mesmo que
     * `texto_len`: `"oi"` tem texto "oi" (2) e ocupa 4 no fonte. O realce do
     * editor precisa do segundo. 0 = não medido (INDENT/DEDENT/NEWLINE). */
    int32_t   nchars;
} PSToken;

typedef struct {
    PSToken *tokens;
    int32_t  n;
    int32_t  cap;

    /* Erro: `ok == 0` quando a análise falhou. */
    int      ok;
    char     erro[256];
    int32_t  erro_linha;
    int32_t  erro_col;
} PSTokenList;

/* Analisa `fonte` (UTF-8, terminada em NUL). Sempre devolve uma lista que
 * precisa ser liberada com ps_lexer_free, mesmo em caso de erro. */
PSTokenList *ps_lexer_tokenize(const char *fonte, size_t len);
/* Igual, mas guarda tambem os COMENTARIOS como T_COMMENT. Serve ao realce do
 * editor, que precisa saber onde eles estao; o compilador usa a de cima. */
PSTokenList *ps_lexer_tokenize_editor(const char *fonte, size_t len);
void         ps_lexer_free(PSTokenList *lista);

/* Nome do tipo de token — usado nas mensagens e no teste diferencial. */
const char  *ps_tok_nome(PSTokType t);

#endif /* PS_LEXER_H */
