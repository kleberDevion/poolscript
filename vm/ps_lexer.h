/*
 * Lexer da Jinga em C puro.
 *
 * Este cabeçalho é a fronteira da etapa "tudo nativo": nada aqui depende de
 * nada além da libc, então o mesmo código serve tanto pra extensão (durante a
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
    /* literal de bytes `b"..."`: texto = os bytes CRUS (NUL dentro vale;
     * texto_len é o comprimento) */
    T_BYTES,
    /* Só existe no modo do editor (`ps_lexer_tokenize_editor`): o lexer
     * normal DESCARTA comentário, e o parser nunca vê este tipo. Fica por
     * ultimo pra nao mexer no valor numerico de nenhum outro. */
    T_COMMENT,
    /* So existe no MODO DE RECUPERACAO: o trecho que o lexer nao entendeu
     * (caractere estranho), pra o realce saber onde ele esta em vez de perder
     * o arquivo inteiro. Tambem por ultimo, pelo mesmo motivo. */
    T_ERRO,
    /* So existe no MODO DE RECUPERACAO: o ponto onde o lexer FECHOU A FORCA um
     * `(`, `[` ou `{` que o codigo deixou aberto (o erro vai pro abridor). Nao
     * ocupa texto. Pro parser e uma fronteira dura: nenhuma construcao aceita
     * este token, e a recuperacao para nele — sem ele, um `(` esquecido fazia o
     * resto do arquivo virar uma expressao so, e a arvore perdia tudo depois. */
    T_SINC
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
    /* Onde o token TERMINA (a posição logo depois do último caractere dele).
     * `nchars` só serve pra token de uma linha só; uma string de três linhas
     * ou um comentário de bloco não tinham fim nenhum, e o editor não tinha
     * como sublinhar nem dobrar. 0 = não medido. */
    int32_t   linha_fim;
    int32_t   col_fim;
} PSToken;

/* Um aviso do lexer: o programa compila, mas provavelmente não faz o que
 * parece. Ver o campo `avisos` de PSTokenList. */
typedef struct {
    char    msg[160];
    int32_t linha;
    int32_t col;
} PSAviso;

/* As palavras-chave da linguagem, terminadas em NULL. Fonte única: a tabela
 * que o próprio lexer consulta. */
const char *const *ps_lexer_keywords(void);

typedef struct {
    PSToken *tokens;
    int32_t  n;
    int32_t  cap;

    /* Erro: `ok == 0` quando a análise falhou. */
    int      ok;
    char     erro[256];
    int32_t  erro_linha;
    int32_t  erro_col;

    /* AVISOS: o programa compila, mas alguma coisa quase certamente não é o
     * que quem escreveu quis. Hoje só um caso, e ele custou caro:
     * `"C:\pasta"` — o `\p` não é escape, e o lexer engolia a barra CALADO,
     * devolvendo `C:pasta`. O certo é manter a barra E avisar; nós fazíamos
     * o contrário nas duas metades.
     *
     * Aviso não é erro: `ok` continua 1 e o programa roda. Quem apresenta é
     * quem chamou — o `pool` imprime no stderr, o `--check` devolve no JSON,
     * e o editor sublinha. */
    PSAviso *avisos;
    int32_t  navisos;
    int32_t  cap_avisos;

    /* TODOS os erros, no MODO DE RECUPERAÇÃO (ver `ps_lexer_tokenize_modo`).
     * O editor precisa do que sobrou: com o modo de parar no primeiro erro, um
     * caractere estranho no meio do arquivo apagava os tokens das linhas boas
     * e a tela inteira perdia a cor. Rodando o programa nada disso vale — o
     * primeiro erro para, como sempre. */
    PSAviso *erros;
    int32_t  nerros;
    int32_t  cap_erros;

    /* As INTERPOLAÇÕES das f-strings: um registro por `{...}`, com o pedaço no
     * FONTE (offset em bytes e tamanho) e a linha/coluna onde ele começa.
     *
     * Tem que ser anotado aqui, na varredura do fonte cru: o `texto` do token
     * sai DECODIFICADO (`\t` vira um byte), então recalcular a posição a
     * partir dele erra a coluna sempre que houver escape antes da chave. Sem
     * isso a f-string era UM token: `nome.upper()` dentro de `{}` não existia
     * pro realce, pro "ir pra definição" nem pro completion. */
    struct PSInterp { int32_t tok; size_t off; int32_t len; int32_t linha; int32_t col;
                      char *txt; /* o trecho, cru, terminado em NUL */ } *interps;
    int32_t  ninterps;
    int32_t  cap_interps;

    /* Quantos grupos (`(`, `[`, `{`) o lexer teve que fechar a forca, no modo
     * de recuperacao. Diferente de zero = o codigo tem parentese/colchete/chave
     * sem par, e vale a segunda passada (ver `ps_lexer_tokenize_modo`). */
    int32_t  grupos_forcados;
    /* O TRECHO de cada grupo fechado a forca: do abridor (l1, c1) ate o ponto
     * onde o lexer o fechou (l2, c2). O que o parser acusar ali dentro e
     * sintoma do mesmo grupo sem par — `funct f( {` e um `(` esquecido, nao
     * tambem um "esperado nome de parametro" no `{` — e sai da lista dele. */
    struct PSFechado { int32_t l1, c1, l2, c2; } *fechados;
    int32_t  nfechados;
    int32_t  cap_fechados;

    /* MODO FLUXO (`ps_lexer_fluxo`): o token nasce quando o parser pede
     * (`ps_lexer_tok`) e a lista nunca tem o arquivo inteiro. Guardar o vetor
     * de todos os tokens antes de parsear custava ~1,5 KB por linha: um
     * arquivo de 1 milhão de linhas pedia gigabytes só pra isso, e a máquina
     * entrava na swap. Os tokens ficam em BLOCOS que nunca mudam de lugar — o
     * parser segura `PSToken *` durante uma classe ou um bloco inteiro — e o
     * parser solta os blocos de trás a cada declaração de topo
     * (`ps_lexer_solta_ate`). `tokens` fica NULL; `n` conta os que já
     * nasceram, e o índice de cada um é o mesmo do modo de lista inteira. */
    int      fluxo;
    struct PSTokBloco **blocos;
    int32_t  nblocos;
    int32_t  cap_blocos;
    int32_t  soltos;         /* blocos [0, soltos) já foram soltos */
    void    *lexer;          /* o lexer no meio do arquivo; NULL = chegou no fim */
    /* Quantos `{...}` já foram anotados desde o começo do arquivo: o teto de
     * `interps` conta o arquivo inteiro, mesmo depois que os velhos saem. */
    int32_t  interps_total;
} PSTokenList;

/* Imprime no stderr os avisos que a lista juntou, no formato
 * `<arquivo>:<linha>: SyntaxWarning: ...`. stderr, e nao stdout, porque
 * stdout e o canal de dado do programa e do JSON do `--check`. Definida em
 * jinga_vm.c. */
void ps_avisos_para_stderr(const PSTokenList *toks, const char *caminho);

/* Analisa `fonte` (UTF-8, terminada em NUL). Sempre devolve uma lista que
 * precisa ser liberada com ps_lexer_free, mesmo em caso de erro. */
PSTokenList *ps_lexer_tokenize(const char *fonte, size_t len);
/* Igual, mas guarda tambem os COMENTARIOS como T_COMMENT. Serve ao realce do
 * editor, que precisa saber onde eles estao; o compilador usa a de cima. */
PSTokenList *ps_lexer_tokenize_editor(const char *fonte, size_t len);
/* As duas de cima, com os dois interruptores na mao:
 *   `comentarios` = guarda os comentarios como token (realce);
 *   `recupera`    = SEGUE depois do erro, juntando todos em `erros`, em vez de
 *                   parar no primeiro. So os comandos de editor usam; rodar o
 *                   programa continua parando no primeiro erro.
 *
 * No modo de recuperacao, `(`, `[` e `{` sem par viram erro NO ABRIDOR, e o
 * lexer fecha o grupo a forca (token T_SINC) no fechador que nao casa ou no fim
 * do arquivo. Se isso aconteceu, roda uma SEGUNDA passada que tambem fecha o
 * grupo de expressao aberto quando uma linha nova so pode ser comeco de
 * declaracao (`funct nome`, `int funct nome`, `class Nome`, `@`, `import`...).
 * Essa regra e heuristica, por isso so vale quando ja ha grupo sem par: arquivo
 * valido nunca chega nela, e o `--check` nao recusa codigo valido. */
PSTokenList *ps_lexer_tokenize_modo(const char *fonte, size_t len,
                                    int comentarios, int recupera);
/* Lista em MODO FLUXO (ver o campo `fluxo`): nada é lido ainda. `fonte`
 * tem que viver até a lista ser liberada. `recupera` e `sincroniza` são os
 * das passadas de `ps_lexer_tokenize_modo`. */
PSTokenList *ps_lexer_fluxo(const char *fonte, size_t len, int recupera, int sincroniza);
/* O token `i` (índice absoluto), lendo o fonte até ele se preciso. NULL = o
 * arquivo acabou antes dele. Nas duas formas de lista. No modo fluxo, se o
 * lexer parar num erro sem o modo de recuperação, a lista ganha um EOF pra
 * quem está parseando parar — `ok == 0` continua dizendo que o arquivo não
 * vale. */
PSToken     *ps_lexer_tok(PSTokenList *lista, int32_t i);
/* Lê o fonte até o fim: `ok`, `erro`, `avisos`, `erros` e `fechados` ficam
 * completos, como na lista inteira. */
void         ps_lexer_drena(PSTokenList *lista);
/* Modo fluxo: solta os blocos cujos tokens vêm TODOS antes do índice `i`. Ler
 * um token solto depois disso é erro de quem chamou. Na lista inteira não faz
 * nada. */
void         ps_lexer_solta_ate(PSTokenList *lista, int32_t i);
void         ps_lexer_free(PSTokenList *lista);

/* Nome do tipo de token — usado nas mensagens e no teste diferencial. */
const char  *ps_tok_nome(PSTokType t);

#endif /* PS_LEXER_H */
