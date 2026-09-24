/*
 * Compilador AST → bytecode da Jinga, em C puro.
 *
 * Fronteira de acoplamento: o compilador NÃO conhece `Value`, `Obj` nem o
 * GC da VM. Ele emite constantes numa forma neutra (`PSConst`), e é a VM que
 * as converte em valores gerenciados ao carregar. Sem isso o compilador
 * precisaria alocar objetos sob o coletor antes de existir uma VM — e
 * strings de constante nasceriam como lixo coletável no meio da compilação.
 *
 * A tabela de protótipos já sai PLANA do compilador: `MAKE_FUNCTION` carrega
 * índice de protótipo, não índice de constante.
 */
#ifndef PS_COMPILER_H
#define PS_COMPILER_H

#include <stdint.h>

#include "ps_ast.h"
#include "ps_lexer.h"
#include "ps_parser.h"

/* Parâmetros FIXOS por funct (o `self` conta; `*args` e `**kwarg` não). O
 * binding da chamada marca cada um num vetor desse tamanho, então o limite é
 * da DECLARAÇÃO: o compilador recusa ali, e a chamada nunca vê mais que isso. */
#define PS_MAX_PARAMS 256

/* Constante numa forma que não depende do runtime. */
typedef enum { K_NULL = 0, K_BOOL, K_INT, K_FLO, K_STR, K_BIGINT, K_BYTES } PSConstKind;

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

/* Uma variável local, com a FAIXA de bytecode em que ela existe.
 *
 * Não dá pra guardar só um vetor `nome[slot]`: `nlocals` é marca d'água e os
 * slots são REAPROVEITADOS entre blocos — o slot 2 é `x` dentro de um `if` e
 * `y` no `for` seguinte. Um vetor plano mostraria o nome errado no debugger.
 * Com a faixa, quem inspeciona um frame parado no `ip` sabe exatamente quais
 * nomes estão vivos ali. */
typedef struct {
    char    *nome;
    int32_t  slot;
    int32_t  ip_ini;    /* primeira palavra de code em que o nome vale */
    int32_t  ip_fim;    /* primeira palavra em que já NÃO vale (-1 = aberta) */
} PSVarDbg;

typedef struct {
    char    *nome;
    int32_t *code;      /* pares [opcode, arg] */
    int32_t *linhas;    /* linha do fonte de cada INSTRUÇÃO (ncode/2 entradas:
                         * a da palavra `ip` é `linhas[ip / 2]`) — pro erro de
                         * runtime dizer ONDE aconteceu */
    int32_t *colunas;   /* coluna do fonte de cada instrução (idem) — pro
                         * cursor `^^^` cair na posição certa */
    int32_t  ncode;
    PSConst *consts;
    int32_t  nconsts;
    int32_t  nlocals;
    int32_t  nparams;     /* só os parâmetros FIXOS (sem `*args`/`**kwarg`) */
    int32_t  ndefaults;   /* quantos parâmetros finais têm valor padrão */
    /* `funct f(a, *args, **kwarg)`: o slot local que recebe a tup dos
     * posicionais excedentes e o do dict dos nomeados sem parâmetro. -1 =
     * a funct não tem. Ficam FORA de `nparams`/`param_nomes`: `args=1` na
     * chamada não casa com o parâmetro, cai no dict como qualquer nome. */
    int32_t  slot_vararg;
    int32_t  slot_kwarg;
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
    /* Tipo declarado de cada parâmetro (`funct f(str nome)`), na mesma ordem.
     * NULL na posição = parâmetro sem tipo; NULL no vetor = nenhum tem. É só
     * CHECAGEM em runtime: a linguagem não converte argumento nenhum. */
    char   **param_tipos;
    /* Variáveis de fora que esta action captura (closure). Vazio na maioria
     * das actions: só uma action DECLARADA DENTRO de outra tem upvalue. */
    PSUpval *upvals;
    int32_t  nupvals;
    char   **upval_nomes;   /* nome de cada upvalue — só pra mensagem de erro */
    /* Tabela de variáveis locais com faixa de vida — só o debugger usa. */
    PSVarDbg *vars;
    int32_t   nvars;
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
    /* Campos de instância com tipo escrito — `nome: str` no corpo e `private
     * str nome = x` dentro de um método —, nome e tipo em paralelo. A VM
     * confere toda escrita neles (OP_SET_MEMBER). */
    char   **tip_nomes;
    char   **tip_tipos;
    int32_t  ntip;
} PSClassDef;

/* Literal de parâmetro do campo de model (`in=[…]`, `not_in=[…]`): o tipo
 * do campo diz qual membro vale (`s` em str, `i` em int/bool, `d` em flo). */
typedef struct {
    int32_t tipo;
    int64_t i;
    double  d;
    char   *s;
} PSModelLit;

/* Campo de `model`, na forma neutra do compilador. Além do tipo e do
 * `length`, os parâmetros que validam o DADO: `regex` (str), `in`/`not_in`
 * (valores permitidos/proibidos), `min`/`max` (int, flo), `optional`
 * (pode faltar ou ser null), `of` (tipo de cada item de uma list) e o
 * tipo que é OUTRO model (estrutura aninhada). Tudo literal, decidido na
 * compilação. */
typedef struct {
    char   *nome;
    int32_t tipo;         /* TIPO_* da VM; -1 = outro model (`tipo_model`) */
    char   *tipo_model;   /* nome do model aninhado, quando tipo == -1 */
    int32_t length;       /* -1 = sem limite (str: caracteres; int: dígitos; list: itens) */
    char   *regex;        /* NULL = sem padrão */
    PSModelLit *in;       int32_t n_in;
    PSModelLit *not_in;   int32_t n_not_in;
    int     tem_min, tem_max;
    double  min, max;
    int     optional;
    int32_t of_tipo;      /* -1 = sem `of`; TIPO_* do item; -2 = model (`of_model`) */
    char   *of_model;
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

/* Um erro da tipagem estática, com a posição. `classe` é o nome do erro como a
 * VM o daria rodando: AttributedValueError (tipo), TypeError (aridade da
 * chamada), AttributeError (membro que o tipo não tem). */
typedef struct {
    char    msg[256];
    char    classe[32];
    int32_t linha;
    int32_t col;
    /* Erro que na verdade está em OUTRO arquivo — o módulo importado que não
     * compila, acusado na linha do `import`: o arquivo e a linha de dentro
     * dele, pro quadro mostrar onde o defeito está (vazio = este arquivo). */
    char    arquivo[1024];
    int32_t linha_arq;
    int32_t col_arq;
} PSErroTipo;

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

    /* Nomes de MODULO marcados `private` (action, reaction): nao saem no
     * `import` nem no `from ... import`. `private class` ja tinha o seu
     * caminho (PSClassDef.classe_privada); action de modulo compilava o
     * `private` e nao fazia nada com ele. */
    char   **priv_globais;
    int32_t  npriv_globais;

    /* O que o `import` deste arquivo enxerga: os nomes que ele liga no nível
     * do arquivo — funct, Entity, enum, model, variável, o que ele importou
     * (inclusive por `*`) e o que uma funct grava com `global x` —, sem
     * `private` e sem nome interno. É a regra de `from m import x`, de `m.x`
     * e de `from m import *`, num lugar só: o escopo que o próprio compilador
     * já calcula. O nome que a VM pré-liga no módulo (builtin, exceção,
     * `__name__`) só entra se o arquivo o redefinir. */
    char   **exportados;
    int32_t  nexportados;

    /* 1 = algum `*` do topo NÃO foi resolvido (o módulo não foi achado, não
     * compila, ou a cadeia de `*` é funda demais pra expandir). A lista de
     * `exportados` então está INCOMPLETA: quem expande este arquivo tem que
     * recusar em vez de entregar uma lista que perdeu nomes em silêncio. */
    int      estrela_incompleta;

    int      ok;
    char     erro[256];
    int32_t  erro_linha;
    int32_t  erro_col;
    /* 1 = o PROGRAMA está errado (SyntaxError); 0 = o compilador é que ainda
     * não emite este nó (NotImplementedError). Sem isto todo erro de compilação
     * saía como "NotImplementedError", inclusive `base()` fora de lugar — o
     * nome do erro não tinha nada a ver com o problema.
     * 2 = erro da TIPAGEM ESTÁTICA: a lista inteira está em `erros_tipo` (o
     * primeiro também em `erro`), e o programa não chega a rodar. */
    int      erro_do_programa;
    PSErroTipo *erros_tipo;
    int32_t     nerros_tipo;
} PSPrograma;

/* `import *` é resolvido NA COMPILAÇÃO: o compilador pergunta quais nomes o
 * módulo exporta e compila como se a lista tivesse sido escrita
 * (`from m import a, b, c`). Escopo de bloco, sombra do
 * `for each`, tipo declarado e reexportação saem da maquinaria de sempre.
 *
 * `nomes_de` recebe o módulo codificado como no OP_IMPORT_MOD e devolve 1 com
 * os nomes (vetor e strings em malloc; o compilador libera), 2 quando a lista
 * veio INCOMPLETA (um ciclo de `*` cortou a expansão: o resto dos nomes só
 * existe rodando, então o checador estático não pode dar nome como
 * inexistente), 3 quando o módulo NÃO EXISTE e quem pergunta é o `--check` (o
 * compilador acusa `ImportError: No module named` na linha do import), ou 0
 * quando não compila, está em ciclo, ou não foi achado mas quem pergunta vai
 * RODAR — aí o `*` fica pro runtime dar o ImportError/SyntaxError de sempre na
 * linha do import. */
/* Um módulo `.pr` importado, como o checador estático o enxerga: a AST (as
 * assinaturas das functs e classes, pra conferir chamada e membro antes de
 * rodar), o que ele exporta (`m.x` que não existe é AttributeError antes de
 * rodar) e se a lista veio cortada por um ciclo de `*` (aí nome ausente não
 * é erro). Tudo é do resolvedor e vale até o fim do processo. */
typedef struct {
    PSNode      *programa;
    const char  *caminho;        /* absoluto — vai na mensagem do ImportError */
    char       **exportados;
    int32_t      nexportados;
    int          incompleto;
    /* O módulo foi achado mas NÃO compila (sintaxe ou tipo): a classe e a
     * frase do primeiro erro, as mesmas que o import daria rodando. */
    int          falhou;
    char         erro_classe[32];
    char         erro_msg[256];
    char         erro_arquivo[1024];   /* onde o erro está de verdade (pode ser mais fundo) */
    int32_t      erro_linha;
    int32_t      erro_col;
} PSModuloAst;

typedef struct {
    int  (*nomes_de)(void *ctx, const char *modulo, char ***nomes, int32_t *n);
    /* O módulo `.pr` que `modulo` (codificado como no `nomes_de`) nomeia: 1 com
     * a AST e os exportados; 2 quando foi achado mas não compila (`falhou`,
     * com o erro — o checador acusa na linha do import); 3 quando NÃO EXISTE e
     * quem pergunta é o `--check` (o checador acusa `No module named`); 0
     * quando é nativo, está em ciclo, ou não foi achado mas quem pergunta vai
     * RODAR (o checador fica cego pra ele, e o runtime dá o erro de sempre).
     * Regra 5 da tipagem estática: chamada a funct/método de `.pr` importado
     * tem aridade, nomes e tipos conferidos antes de rodar. */
    int  (*modulo_de)(void *ctx, const char *modulo, const PSModuloAst **out);
    void  *ctx;
} PSResolvedor;

/* Compila a AST. Sempre devolve algo que precisa de ps_compila_free,
 * inclusive em erro. Sem resolvedor (`--check`), o `*` compila na forma não
 * resolvida. */
PSPrograma *ps_compila(PSNode *programa);
PSPrograma *ps_compila_com(PSNode *programa, const PSResolvedor *resolve);
void        ps_compila_free(PSPrograma *p);

/* Compila DIRETO DO FONTE, sem a árvore do arquivo inteiro na memória: o
 * arquivo é lido uma declaração de topo por vez, duas vezes (colher os
 * cabeçalhos; gerar o código). É o mesmo resultado de lexer + parser +
 * `ps_compila_com`, com a memória de UMA declaração em vez da do arquivo.
 *
 * `fc` recebe a lista do lexer (`ok`, `erro`, `avisos`, `erros`; sem os
 * tokens) e o resultado do parser (`ok`, `erro`, `erros`, e em `programa` a
 * árvore PODADA: os cabeçalhos que outro arquivo consulta ao importar este —
 * é o que o cache de módulos guarda). Os dois são de quem chamou:
 * `ps_lexer_free` e `ps_parse_free`. Com erro de sintaxe (lexer ou parser)
 * devolve NULL e não compila; sem memória, NULL com os dois campos NULL.
 * `recupera` é o do parser: com ele, todos os erros de sintaxe, não só o
 * primeiro (sem a segunda passada de sincronia: pra lista definitiva com
 * grupo sem par, `ps_parse_fonte`). */
typedef struct {
    PSTokenList   *lexer;
    PSParseResult *parse;
} PSFonteCompilado;
PSPrograma *ps_compila_fonte(const char *fonte, size_t len, int recupera,
                             const PSResolvedor *resolve, PSFonteCompilado *fc);

/* Nome legível do opcode — usado no desmonte e no teste diferencial. */
const char *ps_op_nome(int32_t op);

/* O módulo de um nó de import, codificado como o OP_IMPORT_MOD e a expansão
 * do `*` o recebem (`.a.b`, ou \x01 + literal entre aspas). `encoded` tem 512
 * bytes. Exposto pro `-o` embutir os módulos que o programa alcança,
 * resolvendo-os com o MESMO nome que o import vai usar rodando. */
void ps_import_modulo_codificado(const PSNode *n, char *encoded);

#endif /* PS_COMPILER_H */
