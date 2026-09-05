/*
 * Compilador AST → bytecode da PoolScript, em C puro.
 *
 * Fronteira de acoplamento: o compilador NÃO conhece `Value`, `Obj` nem o
 * GC da VM. Ele emite constantes numa forma neutra (`PSConst`), e é a VM que
 * as converte em valores gerenciados ao carregar. Sem isso o compilador
 * precisaria alocar objetos sob o coletor antes de existir uma VM — e
 * strings de constante nasceriam como lixo coletável no meio da compilação.
 *
 * A tabela de protótipos já sai PLANA (o achatamento acontece em
 * Python acontece aqui direto): `MAKE_FUNCTION` carrega índice de protótipo,
 * não índice de constante.
 */
#ifndef PS_COMPILER_H
#define PS_COMPILER_H

#include <stdint.h>

#include "ps_ast.h"

/* Um aviso do compilador. Mesma forma do `PSAviso` do lexer, declarado à parte
 * pra este header não depender do outro — quem apresenta os dois (o `--check`,
 * o `pool`, o editor) trata igual. */
typedef struct {
    /* 160, o MESMO do `PSAviso` do lexer: os dois vão pra mesma lista quando o
     * `--check` responde, e um buffer maior aqui só faria a cópia truncar lá —
     * cortando o fim da frase, que é onde está o conselho. */
    char    msg[160];
    int32_t linha;
    int32_t col;
} PSAvisoC;

/* Constante numa forma que não depende do runtime. */
typedef enum { K_NULL = 0, K_BOOL, K_INT, K_FLO, K_STR, K_BIGINT } PSConstKind;

typedef struct {
    PSConstKind kind;
    int64_t     i;      /* K_INT, K_BOOL */
    double      d;      /* K_FLO */
    char       *s;      /* K_STR — dono da memória */
    int32_t     slen;
} PSConst;

/* Uma variável capturada por uma action ANINHADA. `em_local` diz de onde ela
 * vem quando o closure é montado: 1 = célula que está num slot local do frame
 * de fora, 0 = célula que o closure de fora já tinha (captura em cadeia,
 * quando o aninhamento tem mais de um nível). */
typedef struct {
    int32_t em_local;
    int32_t idx;
} PSUpval;

typedef struct {
    char    *nome;
    int32_t *code;      /* pares [opcode, arg] */
    int32_t *linhas;    /* linha do fonte de cada palavra do code (paralelo) —
                         * pro erro de runtime dizer ONDE aconteceu */
    int32_t *colunas;   /* coluna do fonte de cada palavra (paralelo a code) —
                         * pro cursor `^^^` cair na posição certa, igual ao interp */
    int32_t  ncode;
    PSConst *consts;
    int32_t  nconsts;
    int32_t  nlocals;
    int32_t  nparams;
    int32_t  ndefaults;   /* quantos parâmetros finais têm valor padrão */
    int32_t  eh_gerador;  /* contém `yield` — chamar cria gerador, não frame */
    int32_t  eh_async;    /* `async action` — chamar cria fibra+future, não roda inline */
    /* `@static`: chamável direto na Entity (`Classe.metodo()`), sem instância.
     * Sem esta marca a VM tinha que ADIVINHAR pelo 1º parâmetro chamar-se
     * `self` — e aí `Classe.metodoNormal()` passava batido, dropava o self em
     * silêncio e o erro saía no parâmetro seguinte. */
    int32_t  eh_static;
    /* Nome de cada parâmetro, na ordem. Só existe pra resolver argumento
     * nomeado em runtime — o call site não sabe qual função vai chamar. */
    char   **param_nomes;
    /* Variáveis de fora que esta action captura (closure). Vazio na maioria
     * das actions: só uma action DECLARADA DENTRO de outra tem upvalue. */
    PSUpval *upvals;
    int32_t  nupvals;
    char   **upval_nomes;   /* nome de cada upvalue — só pra mensagem de erro */
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
    /* nomes de membros `private` (campo ou método) — a VM barra acesso a eles
     * de fora da classe (encapsulamento). */
    char   **priv_nomes;
    int32_t  npriv;
    /* `private class Nome()` — a classe INTEIRA não é exportada no import. */
    int32_t  classe_privada;
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

/* Membro de `enum`, na forma neutra do compilador. O VALOR explícito não fica
 * aqui: é expressão, compilada e empilhada antes do MAKE_ENUM. `tem_valor`
 * diz se este membro empilha um valor (explícito) ou é auto-numerado. */
typedef struct {
    char   *nome;
    int32_t tem_valor;   /* 1 = valor explícito na pilha; 0 = auto */
} PSEnumMembroDef;

typedef struct {
    char            *nome;
    PSEnumMembroDef *membros;
    int32_t          nmembros;
} PSEnumDef;

typedef struct {
    PSProto *protos;    /* índice 0 = módulo */
    int32_t  nprotos;

    char   **globais;   /* nome de cada global, indexado */
    int32_t  nglobais;

    PSClassDef *classes;
    int32_t     nclasses;

    PSModelDef *models;
    int32_t     nmodels;

    PSEnumDef  *enums;
    int32_t     nenums;

    int      ok;
    char     erro[256];
    int32_t  erro_linha;
    int32_t  erro_col;
    /* 1 = o PROGRAMA está errado (SyntaxError); 0 = o compilador é que ainda
     * não emite este nó (NotImplementedError). Sem isto todo erro de compilação
     * saía como "NotImplementedError", inclusive `base()` fora de lugar — o
     * nome do erro não tinha nada a ver com o problema. */
    int      erro_do_programa;

    /* AVISOS do compilador — o programa compila e roda, mas alguma coisa quase
     * certamente não é o que se quis. Mesma ideia dos avisos do lexer
     * (`PSAviso` em ps_lexer.h), num nível que só o compilador enxerga: aqui
     * já se sabe o que é função, o que é módulo e o que cada uma liga.
     *
     * O primeiro caso, e a razão de isto existir: escrever dentro de uma
     * função num nome que existe no módulo ESCREVE NO MÓDULO. É o design da
     * linguagem (31 casos da suíte dependem dele), e ao mesmo tempo é a coisa
     * mais silenciosa que ela faz — um `i = 0` dentro de uma action zera o `i`
     * do laço de quem chamou, sem erro nenhum, e o defeito aparece longe da
     * causa. O aviso não muda a semântica: torna a colisão visível. */
    PSAvisoC *avisos;
    int32_t   navisos;
    int32_t   cap_avisos;
} PSPrograma;

/* Compila a AST. Sempre devolve algo que precisa de ps_compila_free,
 * inclusive em erro. */
PSPrograma *ps_compila(PSNode *programa);
void        ps_compila_free(PSPrograma *p);

/* Nome legível do opcode — usado no desmonte e no teste diferencial. */
const char *ps_op_nome(int32_t op);

#endif /* PS_COMPILER_H */
