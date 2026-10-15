/*
 * A biblioteca padrão do C dentro da Jinga (Plano 1): os tipos do C e a
 * chamada de função do C pela libffi.
 *
 * Este módulo não depende da VM. Ele sabe:
 *   - descrever um tipo do C (escalar, ponteiro, array, struct, union,
 *     função), com o tamanho e o alinhamento do sistema em que o motor foi
 *     compilado — o `long` tem 8 bytes no Linux e no macOS e 4 no Windows,
 *     como o C de cada um;
 *   - achar os tipos do padrão pelo nome (`int`, `unsigned long`, `size_t`,
 *     `int32_t`, `time_t`...);
 *   - chamar uma função do C com valores já no formato do C, pela libffi
 *     (1996, licença MIT; é a mesma que o `ctypes` do Python usa), inclusive
 *     as de argumento variável (`printf`), em que cada chamada diz o tipo de
 *     cada argumento a mais;
 *   - fazer de uma função de quem chama um ponteiro de função do C de
 *     verdade (o comparador do `qsort`), por closure da libffi.
 *
 * Quem converte valor da Jinga pra memória do C (e de volta) é a VM; aqui
 * entra e sai só memória do C.
 */
#ifndef PS_C_H
#define PS_C_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    PSC_VOID,
    PSC_INT,       /* char, short, int, long, long long, com e sem sinal */
    PSC_FLUT,      /* float, double, long double */
    PSC_BOOL,      /* _Bool */
    PSC_PONT,      /* T * */
    PSC_ARRAY,     /* T[N] */
    PSC_STRUCT,
    PSC_UNION,
    PSC_FUNC,      /* tipo de função: retorno, parâmetros, variádica */
    PSC_OPACO      /* FILE, mbstate_t: só existe atrás de ponteiro */
} PSCKind;

typedef struct PSCTipo PSCTipo;

typedef struct {
    const char *nome;
    PSCTipo    *tipo;
    size_t      desloc;    /* deslocamento dentro do struct, em bytes */
} PSCCampo;

struct PSCTipo {
    PSCKind     kind;
    const char *nome;      /* como o C escreve: "unsigned long", "struct tm", "char *" */
    size_t      tam;
    size_t      alinha;
    int         sem_sinal; /* PSC_INT: 1 = unsigned */
    int         eh_char;   /* PSC_INT: char, signed char, unsigned char */
    int         constante; /* o tipo é `const T` */
    PSCTipo    *sem_const; /* `const T`: o T; os outros: NULL */
    PSCTipo    *canon;     /* typedef (size_t): o tipo que ele é; os outros: NULL */
    PSCTipo    *alvo;      /* PSC_PONT: o apontado; PSC_ARRAY: o elemento */
    size_t      n;         /* PSC_ARRAY: quantos elementos */
    PSCCampo   *campos;    /* PSC_STRUCT e PSC_UNION */
    int         ncampos;
    PSCTipo    *ret;       /* PSC_FUNC */
    PSCTipo   **params;
    int         nparams;
    int         variadica;
    void       *ffi;       /* o ffi_type montado sob demanda (struct) */
    void       *cif;       /* PSC_FUNC não variádica: o ffi_cif preparado uma vez */
};

/* Os tipos escalares do C, um objeto cada (comparar ponteiro = comparar tipo).
 * O `nome` de cada um vale depois da primeira chamada a uma função daqui que
 * monta ou devolve tipo: ele não é escrito no dado estático, pra o
 * carregador não ter relocação pra aplicar na partida. */
extern PSCTipo ps_c_void, ps_c_char, ps_c_schar, ps_c_uchar, ps_c_short, ps_c_ushort,
               ps_c_int, ps_c_uint, ps_c_long, ps_c_ulong, ps_c_llong, ps_c_ullong,
               ps_c_float, ps_c_double, ps_c_ldouble, ps_c_bool;

/* O tipo pelo nome que o C usa: os escalares ("unsigned long int", "long
 * double"...), os tipos do padrão ("size_t", "int32_t", "FILE", "struct tm")
 * e os derivados ("const char *"). NULL se o texto não é tipo do C. */
PSCTipo *ps_c_tipo_por_nome(const char *nome);

/* Lê uma declaração do C como o compilador C lê (C11 6.7): os
 * especificadores dão o tipo de base, e o declarador monta em volta dele.
 * Vale "const char *s", "unsigned long", "struct tm *",
 * "int (*compar)(const void *, const void *)" e um protótipo inteiro.
 * Devolve o tipo e grava em `nome` o nome declarado ("" se a declaração não
 * tem nome). NULL se o texto não é declaração que se entende, com o motivo
 * em `erro`. `nome` e `erro` podem ser NULL. */
PSCTipo *ps_c_le_decl(const char *texto, char *nome, size_t cap, char *erro, size_t ecap);

/* Tipos montados. Os resultados são guardados: pedir duas vezes o mesmo
 * tipo devolve o mesmo objeto. Devolvem NULL só sem memória. */
PSCTipo *ps_c_ponteiro(PSCTipo *alvo);
PSCTipo *ps_c_const(PSCTipo *t);                  /* `const T` */
PSCTipo *ps_c_array(PSCTipo *elem, size_t n);
PSCTipo *ps_c_funcao(PSCTipo *ret, PSCTipo **params, int nparams, int variadica);
/* struct e union: os campos com tipo; os deslocamentos, o tamanho e o
 * alinhamento saem da regra do C (o que a libffi calcula), a não ser que
 * `deslocs` venha preenchido (os structs do padrão, medidos pelo compilador
 * com `offsetof`, porque o padrão não fixa a ordem nem os campos a mais). */
PSCTipo *ps_c_struct(const char *nome, int eh_union, const char **nomes, PSCTipo **tipos,
                     int n, const size_t *deslocs, size_t tam, size_t alinha);
PSCTipo *ps_c_opaco(const char *nome, size_t tam, size_t alinha);

/* ── a biblioteca padrão do C: a tabela de vm/c_padrao.decl ────────────── */
/* O que cada nome da tabela é (a ordem é a de ESPECIES em
 * scripts/gera_c_padrao.pr). */
typedef enum {
    PSC_NOME_FUNCAO = 1,  /* função da biblioteca */
    PSC_NOME_MACRO,       /* macro com cara de função (isnan): a VM chama uma função nossa */
    PSC_NOME_LITERAL,     /* INT64_C(value): o valor, com o tipo da macro */
    PSC_NOME_CONST,       /* constante inteira */
    PSC_NOME_CONSTFLO,    /* constante de ponto flutuante */
    PSC_NOME_TEXTO,       /* constante de texto (PRId64) */
    PSC_NOME_TALVEZ,      /* constante que o sistema define ou não (FP_FAST_FMA) */
    PSC_NOME_VALOR,       /* expressão lida na hora (stdin, MB_CUR_MAX, errno) */
    PSC_NOME_NULO,        /* NULL */
    PSC_NOME_TIPO,        /* typedef de escalar (size_t) */
    PSC_NOME_OPACO,       /* tipo que só se usa pelo tamanho (FILE) */
    PSC_NOME_ESTRUTURA,   /* struct do padrão (struct tm, div_t) */
    PSC_NOME_SINTAXE,     /* operador com cara de função (offsetof) */
    PSC_NOME_FORA         /* nome do padrão que fica de fora: a declaração é o motivo */
} PSCEspecie;

#define PSC_MARCA_VM     1   /* a VM faz o papel (atexit, exit, errno) */
#define PSC_MARCA_PRINTF 2   /* variádica: o último parâmetro fixo é um formato do printf */
#define PSC_MARCA_SCANF  4   /* variádica: o último parâmetro fixo é um formato do scanf */

/* O índice do nome na tabela, ou -1. "struct tm" acha a struct pela etiqueta. */
int         ps_c_busca(const char *nome);
/* O nome que vale como VALOR num programa (função, macro, literal, constante,
 * valor lido na hora, NULL; a `talvez` só se o sistema a define): o índice na
 * tabela, ou -1. */
int         ps_c_valor_indice(const char *nome);
/* Por que um nome da tabela não vale como valor: o que fica de fora (com o
 * motivo), o tipo usado como valor, a `talvez` que o sistema não define, o
 * operador. A frase vai em `buf`; NULL se o nome não é do C ou se vale. */
const char *ps_c_por_que_nao(const char *nome, char *buf, size_t cap);
int         ps_c_nnomes(void);
const char *ps_c_nome(int i);
PSCEspecie  ps_c_especie(int i);
/* O protótipo, a declaração ou o motivo de ficar de fora; "" se não tem. */
const char *ps_c_decl(int i);
const char *ps_c_cabecalho(int i);   /* "stdio.h" */
int         ps_c_marcas(int i);
/* Função, macro, literal e valor: o endereço do que se chama. A função vem
 * do sistema, achada pelo nome na primeira vez (o carregador não resolve
 * nada disso na partida); NULL quando o sistema não tem a função e nas que a
 * VM faz (PSC_MARCA_VM). */
void      (*ps_c_endereco(int i))(void);
/* Função, macro e literal: o tipo da função. Valor: o tipo do valor (o
 * endereço é de uma função sem parâmetro que devolve esse tipo). Tipo,
 * opaco e estrutura: o próprio tipo. NULL nas outras espécies. */
PSCTipo    *ps_c_tipo_de(int i);
/* Constante inteira (e a `talvez` definida, que vale 1): o valor em 64 bits
 * (o tipo diz se tem sinal). Devolve 0, ou -1 se não é constante inteira ou
 * se o sistema não define a `talvez`. */
int         ps_c_const_int(int i, PSCTipo **tipo, uint64_t *valor);
int         ps_c_const_flo(int i, PSCTipo **tipo, long double *valor);
const char *ps_c_const_texto(int i);   /* NULL se não é constante de texto */
/* Valor lido na hora (stdin, MB_CUR_MAX): grava em `dest`, que tem espaço pro
 * tipo de ps_c_tipo_de(i), no tipo do padrão (o MB_CUR_MAX sai size_t mesmo
 * no macOS, onde a macro é int). Devolve 0, ou -1 com o motivo em `erro`. */
int         ps_c_le_valor(int i, void *dest, char *erro, size_t ecap);

/* A libffi é aberta na primeira chamada ao C, no Linux e no macOS (no
 * Windows ela é estática). Devolve 0, ou -1 com o motivo em `erro` (a
 * biblioteca que não foi achada); `erro` pode ser NULL. As chamadas abaixo
 * abrem sozinhas: esta serve pra quem quer a frase do motivo. */
int         ps_c_pronto(char *erro, size_t ecap);

/* Chama `fn` com os argumentos já na memória do C: `args[i]` aponta pro valor
 * do i-ésimo argumento. `ret` recebe o retorno, com espaço de pelo menos
 * `ps_c_tam_ret(ftipo->ret)` bytes (a libffi devolve inteiro estreito num
 * registro inteiro). Nas variádicas, `tipos_var` diz o tipo de cada um dos
 * `nargs - nparams` argumentos a mais, já promovido (float vira double,
 * inteiro menor que int vira int: a regra das variádicas do C).
 * Devolve 0, ou -1 se a libffi recusou o tipo. */
int ps_c_chama(void (*fn)(void), PSCTipo *ftipo, void **args, int nargs,
               PSCTipo **tipos_var, void *ret);
size_t ps_c_tam_ret(PSCTipo *ret);

/* Lê o retorno guardado por `ps_c_chama` no tipo de retorno (o inteiro
 * estreito vem alargado no registro). */
int64_t  ps_c_ret_int(PSCTipo *ret, const void *buf);
uint64_t ps_c_ret_uint(PSCTipo *ret, const void *buf);

/* No callback, o tratador grava o retorno inteiro com esta: o tipo mais
 * estreito que um registro vai alargado num `ffi_arg` (a regra da libffi),
 * depois de dar a volta no tamanho dele, como a conversão do C. */
void ps_c_ret_grava_int(PSCTipo *ret, void *buf, int64_t v);

/* Callback: um ponteiro de função do C de verdade que, chamado pelo C, chama
 * `tratador(ret, args, dados)` com os argumentos na memória do C. O tipo da
 * função não pode ser variádico (a libffi não tem closure variádica).
 * Devolve o objeto da closure (pra soltar com `ps_c_closure_solta`) e grava
 * em `*codigo` o endereço que o C chama; NULL se não deu. */
typedef void (*PSCTratador)(void *ret, void **args, void *dados);
void *ps_c_closure_nova(PSCTipo *ftipo, PSCTratador tratador, void *dados, void (**codigo)(void));
void  ps_c_closure_solta(void *closure);

#endif /* PS_C_H */
