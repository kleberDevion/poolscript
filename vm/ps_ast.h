/*
 * AST da PoolScript em C puro.
 *
 * Representação: union etiquetada + ARENA.
 *
 * A arena é a escolha certa aqui porque a AST tem tempo de vida único: nasce
 * no parse, morre quando a compilação pra bytecode termina. Nada sobrevive,
 * nada é compartilhado, não há ciclos. Então não precisa de GC (como os
 * valores em tempo de execução precisam) nem de free por nó — solta-se o
 * bloco inteiro de uma vez. Isso também torna o parser à prova de vazamento
 * em caminho de erro: falhou no meio, `ps_arena_free` limpa tudo.
 */
#ifndef PS_AST_H
#define PS_AST_H

#include <stdint.h>
#include <stddef.h>

/* O teste diferencial compara a serialização da AST entre execuções, então
 * estes nomes fazem parte do contrato: mudar um muda o diff. */
typedef enum {
    N_PROGRAM = 0,
    N_LITERAL,        /* int, flo, str, bool, null */
    N_NAME,
    N_BINARY_OP,
    N_CONDITIONAL,   /* ternário: A if cond else B (a=A, b=cond, c=B) */
    N_UNARY_OP,
    N_CALL,
    N_CALL_ARG,
    N_LIST_LITERAL,
    /* [<expr> for each <nome> in <iteravel> (if <cond>)?]
     * a=iteravel, b=expr do item, c=condicao (ou NULL), texto=nome da var */
    N_LIST_COMP,
    N_DICT_LITERAL,
    N_DICT_ENTRY,
    N_INDEX_ACCESS,
    N_MEMBER_ACCESS,
    N_VAR_DECL,
    N_ASSIGNMENT,
    N_EXPRESSION_STMT,
    N_BLOCK,
    N_IF_STMT,
    N_IF_BRANCH,
    N_WHILE_STMT,
    N_FOR_EACH_STMT,
    N_ACTION_DECL,
    N_RETURN_STMT,
    /* ── lote 1: statements e expressões simples ── */
    N_BREAK_STMT,
    N_CONTINUE_STMT,
    N_PASS_STMT,       /* no-op: ocupa o lugar de um corpo */
    N_RAISE_STMT,
    N_YIELD_STMT,
    N_GLOBAL_STMT,
    N_RUN_SELFWITH_STMT,
    N_TUPLE_LITERAL,
    N_SLICE_ACCESS,
    N_POSTFIX_OP,
    N_TYPE_NAME,
    N_INTERPOLATED_STRING,
    N_COLOR_STR_EXPR,
    N_LAMBDA_EXPR,
    N_AWAIT_EXPR,
    /* ── lote 2: controle ── */
    N_TRY_CATCH_STMT,
    N_CATCH_CLAUSE,
    N_USING_STMT,
    /* ── lote 3: match ── */
    N_MATCH_STMT,
    N_MATCH_CASE,
    N_MATCH_PATTERN,
    /* ── lote 4: model e count ── */
    N_MODEL_DECL,
    N_MODEL_FIELD,
    N_ENUM_DECL,      /* enum Nome { A, B=v } — texto=nome, lista=membros */
    N_ENUM_MEMBER,    /* membro — texto=nome, a=valor (NULL = auto) */
    N_COUNT_EXPR,
    N_COUNT_EACH_STMT,
    N_COUNT_EACH_EXPR,
    /* ── lote 5: imports e decorators ── */
    N_IMPORT_STMT,
    N_DECORATOR_STMT,
    N_DECORATOR_CALL,
    /* ── lote 6: OOP e unpacking ── */
    N_ENTITY_DECL,
    N_ENTITY_FIELD,
    N_BASE_CALL_NODE,
    N_MEMBER_ASSIGNMENT,
    N_INDEX_ASSIGNMENT,
    N_UNPACK_ASSIGNMENT,
    N_UNPACK_TARGET,
    /* `private str nome = valor` DENTRO de uma action de Entity: campo do
     * objeto declarado no construtor, com visibilidade. Não é o
     * `N_ENTITY_FIELD` (que é `nome: tipo` no corpo da classe) nem o
     * `N_VAR_DECL` (que é local). texto = nome, texto2 = tipo, a = valor. */
    N_FIELD_DECL
} PSNodeKind;

/* Qual campo do literal vale */
typedef enum { L_INT = 0, L_FLO, L_STR, L_BOOL, L_NULL, L_FSTRING, L_BIGINT } PSLitKind;

typedef struct PSNode PSNode;

typedef struct {
    PSNode **itens;
    int32_t  n;
    int32_t  cap;
} PSNodeVec;

struct PSNode {
    PSNodeKind kind;
    int32_t    line;
    int32_t    col;
    /* Onde o nó TERMINA (0 = não registrado). Hoje só o `Block` preenche, que
     * é o que o editor precisa: sem isto o escopo de uma action acabava no
     * último comando, e o cursor numa linha em branco antes do `}` caía FORA
     * dele — parâmetro e variável local sumiam da sugestão exatamente onde se
     * está escrevendo. A árvore tinha só onde cada coisa começa. */
    int32_t    linha_fim;

    /* texto: nome de variável/action/membro/operador/parâmetro.
     * Aponta pra dentro da arena; não precisa de free. */
    const char *texto;

    /* literais */
    PSLitKind   lit;
    int64_t     i;
    double      d;
    /* Bytes de `texto` num literal de string. O `\x00` é um byte válido no
     * meio da string, então strlen() truncaria: o comprimento tem que viajar
     * junto do ponteiro desde o lexer. 0 = não informado (usa strlen). */
    int32_t     texto_len;

    /* filhos — o significado depende de `kind`:
     *   BINARY_OP        a=esq,   b=dir
     *   UNARY_OP         a=operando
     *   CALL             a=callee,           lista=argumentos (CALL_ARG)
     *   CALL_ARG         a=valor             (texto = nome, se nomeado)
     *   INDEX_ACCESS     a=alvo,  b=índice
     *   MEMBER_ACCESS    a=alvo              (texto = membro)
     *   VAR_DECL         a=valor             (texto=nome, texto2=tipo)
     *   ASSIGNMENT       a=valor             (texto=alvo, texto2=operador)
     *   EXPRESSION_STMT  a=expressão
     *   IF_BRANCH        a=condição (NULL no else), b=bloco
     *   WHILE_STMT       a=condição, b=bloco
     *   FOR_EACH_STMT    a=iterável, b=bloco (texto=nome do item)
     *   ACTION_DECL      b=bloco             (lista=parâmetros como NAME)
     *   RETURN_STMT      a=valor (pode ser NULL)
     *   DICT_ENTRY       a=chave, b=valor
     */
    PSNode     *a;
    PSNode     *b;
    /* Slots extras: nós como SliceAccess (alvo/início/fim/passo) e
     * TryCatchStmt (try/catches/finally) precisam de mais de dois filhos.
     * Manter slots genéricos evita uma struct por tipo de nó — o custo é
     * alguns ponteiros nulos, pago pela arena sem fragmentar nada. */
    PSNode     *c;
    PSNode     *e;
    const char *texto2;
    const char *texto3;
    int32_t     i2;           /* star_index, level do import, length do model */
    PSNodeVec   lista;
    PSNodeVec   lista2;       /* nomes importados, chaves de match, args de decorador */
    PSNodeVec   lista2_alias; /* alias de cada nome de import (Name ou NULL), alinhado com lista2 */

    /* BLOCK: "brace" ou "colon"; ACTION_DECL: tipo de retorno ou NULL */
    const char *estilo;
    int         is_async;
    /* ACTION_DECL / ENTITY_FIELD: marcado `private` (encapsulamento). NÃO é
     * serializado no diff de AST/bytecode — é metadado de acesso, não código. */
    int         is_private;
    /* ACTION_DECL: modificadores COLADOS na cabeça (`static funct m(a)`,
     * `nonnull funct f(v)`) — a forma da linguagem; os decoradores `@static`
     * e `@NonNull` continuam valendo e chegam ao compilador por outro caminho
     * (pendente_static / pendente_nonnull). */
    int         is_static;
    int         is_nonnull;
};

/* ── arena ──────────────────────────────────────────────────────────────── */
typedef struct PSArenaBloco PSArenaBloco;

typedef struct {
    PSArenaBloco *blocos;
    size_t        total;
} PSArena;

void  ps_arena_init(PSArena *a);
void *ps_arena_alloc(PSArena *a, size_t n);
char *ps_arena_strdup(PSArena *a, const char *s, int n);
void  ps_arena_free(PSArena *a);

PSNode *ps_node_novo(PSArena *a, PSNodeKind k, int32_t line, int32_t col);
int     ps_vec_push(PSArena *a, PSNodeVec *v, PSNode *no);

const char *ps_node_nome(PSNodeKind k);

#endif /* PS_AST_H */
