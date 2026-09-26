/*
 * Entrada pura da VM da Jinga.
 *
 * O mesmo `jinga_vm.c` compila de dois jeitos:
 *
 *   com a macro da extensão de teste diferencial → módulo carregável `.so`
 *   sem ela                                       → objeto de um binário standalone
 *
 * As duas formas chamam `ps_roda_fonte`, então não existe "funciona no plugin
 * mas não no binário": é literalmente o mesmo caminho.
 */
#ifndef PS_VM_H
#define PS_VM_H

#include <stddef.h>
#include <stdio.h>

#include "ps_lexer.h"   /* PSAviso: o `--check` devolve os avisos junto */

typedef enum {
    PS_ERRO_NENHUM = 0,
    PS_ERRO_SINTAXE,        /* lexer ou parser */
    PS_ERRO_NAO_SUPORTADO,  /* nó que o compilador ainda não emite */
    PS_ERRO_RUNTIME,
    PS_ERRO_MEMORIA,
    PS_ERRO_TIPO,           /* tipagem estática: o programa nem rodou */
    /* O programa correu até o fim, mas uma tarefa `async` que ninguém
     * aguardou quebrou: o erro JÁ SAIU no stderr, e o código de saída é 1
     * (o mesmo de uma exceção não pega no principal). Saía 0, e um script de
     * build ou um CI tratava a falha como sucesso. */
    PS_ERRO_TAREFA
} PSTipoErro;

/* Um quadro do traceback: função, arquivo e linha. */
typedef struct {
    char nome[64];      /* nome da função (ou "<module>") */
    /* Arquivo-fonte do quadro (vazio = desconhecido). 1024 é o tamanho do
     * `abspath` que alimenta este campo: cortar aqui produziria um caminho
     * que não abre, e o quadro sairia sem a linha do fonte — sem dizer que
     * cortou. */
    char arquivo[1024];
    int  linha;
    int  col;           /* coluna do cursor `^^^` (0 = 1ª não-branco da linha) */
} PSQuadroTB;

typedef struct {
    PSTipoErro tipo;
    char       msg[256];
    char       tipo_nome[64];  /* nome do erro em runtime — o que o `catch` compara */
    int        linha;
    int        col;
    /* Traceback do runtime: do mais externo (<module>) ao mais interno.
     * `ntb == 0` quando não há (erro de sintaxe, etc.). */
    PSQuadroTB tb[64];
    int        ntb;
    /* PS_ERRO_TIPO: TODOS os erros da tipagem estática do arquivo, na ordem
     * do fonte (o primeiro também em msg/linha/col). Nenhum fica de fora da
     * saída: o vetor é do chamador liberar (`free`). */
    /* `arquivo`/`linha_arq`/`col_arq`: o erro está em OUTRO arquivo (módulo
     * importado que não compila), e o quadro dele sai junto com o do import. */
    /* `linha_fim`/`col_fim`: onde o trecho acusado termina (0 = não medido). */
    struct PSErroTipoExec { char msg[256]; char classe[32]; int linha; int col;
                            int linha_fim; int col_fim;
                            char arquivo[1024]; int linha_arq; int col_arq;
                            char nome_arq[64]; } *tipos;
    int        ntipos;
    /* Nota do runtime depois do traceback: o acesso a um nome `private` de
     * módulo mostra ONDE ele foi declarado (`'f' e private: declarada em
     * lib.pr, linha 1` + a linha do fonte). Vazio = sem nota. */
    char       nota_nome[64];
    char       nota_arquivo[1024];
    int        nota_linha;
    int        nota_col;
} PSErroExec;

/* Roda o `.pr` inteiro. 0 = sucesso; -1 preenche `e`. */
/* `caminho` ancora o `import` de arquivo vizinho; NULL só busca lib global. */
/* Argumentos DO USUÁRIO (o que vem depois do arquivo `.pr`) — é o que o
 * `sys.argv` enxerga. Chamar antes de rodar; sem isso a lista sai vazia. */
void ps_set_argv(int argc, char **argv);

/* Liga o depurador: o motor escuta o Debug Adapter Protocol em
 * 127.0.0.1:<porta> e só executa a primeira instrução depois que o editor
 * conectar e terminar o aperto de mão — senão os breakpoints chegariam tarde.
 * A saída do programa continua no stdout dele; o protocolo vai pelo socket. */
void ps_debug_porta(int porta);

int ps_roda_fonte(const char *fonte, size_t len, const char *caminho, PSErroExec *e);

/* `jinga --bytecode`: compila como pra rodar e imprime o bytecode de cada
 * proto em `saida`; NUNCA executa. 0 = ok; -1 preenche `e` como ao rodar. */
int ps_desmonta_fonte(const char *fonte, size_t len, const char *caminho, FILE *saida, PSErroExec *e);

/* ── executável gerado por `-o`: fontes EMBUTIDOS ──────────────────────────
 * O `-o` grudava só o arquivo principal: `import banco` no executável ia
 * procurar `banco.pr` no disco da máquina de quem roda. Agora o binário leva
 * todo `.pr` que o programa alcança por import, e o import olha esta tabela
 * antes do disco. A chave é o caminho que a resolução de módulo calcula — o
 * mesmo cálculo na compilação e na partida, com a mesma pasta do script
 * (`ps_emb_raiz`) e a mesma pasta de libs (`ps_emb_libs`) da máquina que
 * compilou. `real` é o realpath na máquina que compilou (quem roda pode ter
 * os arquivos ou não; os dois nomes valem). */
void        ps_emb_poe(const char *textual, const char *real, const char *fonte, size_t tam);
void        ps_emb_raiz(const char *caminho_main);
void        ps_emb_libs(const char *pasta);
/* O fonte embutido com esse caminho (textual ou real), ou NULL. Também é o
 * que o quadro do traceback lê: a linha vem daqui, não do disco. */
const char *ps_emb_busca(const char *caminho, size_t *tam);

/* Quem imprime UM quadro de traceback no stderr (arquivo, linha, o trecho do
 * fonte e o `^^^`). O `main` põe o dele, que é o mesmo do erro não pego; sem
 * ninguém (o alvo de fuzz não tem `main`), a VM imprime só "em arquivo, linha
 * N". Usado quando a VM avisa de um erro que NÃO parou o programa — o do
 * `int funct` que devolveu 500 no lugar do erro. */
extern void (*ps_gancho_quadro)(const char *arquivo, int linha, int col);

/* Os `.pr` que `caminho_main` alcança por import, transitivamente — o
 * principal em [0] — resolvidos exatamente como o import resolve rodando.
 * `libs` recebe a pasta de libs desta máquina (vai no executável). Módulo que
 * não se acha, ou que não parseia, é erro (-1, mensagem em `erro`): um
 * executável que só descobre isso na máquina de quem roda nasce quebrado. O
 * chamador libera com ps_embutidos_solta. */
typedef struct { char textual[1024]; char real[1024]; char *fonte; size_t tam; } PSEmbutido;
int  ps_embute_deps(const char *caminho_main, PSEmbutido **lista, int32_t *n,
                    char *libs, size_t libs_cap, char *erro, size_t erro_cap);
void ps_embutidos_solta(PSEmbutido *lista, int32_t n);

/* Só VERIFICA (lexer → parser → compilador), NUNCA roda. Para o LSP/editor:
 * usa exatamente a gramática da VM pra apontar erro de sintaxe/compilação sem
 * executar o código do usuário. 0 = sem erro; -1 preenche `e`. */
/* `avisos`/`navisos` sao OPCIONAIS (passe NULL/NULL pra ignorar): recebem os
 * avisos do lexer — o programa compila, mas alguma coisa quase certamente nao
 * e o que se quis, como `"C:\pasta"` com um `\p` que nao e escape. O
 * `--check` os devolve no JSON pra o editor sublinhar; sem isso o aviso so
 * existiria pra quem roda no terminal. O vetor e do CHAMADOR liberar. */
int ps_verifica_fonte(const char *fonte, size_t len, const char *caminho, PSErroExec *e,
                      PSAviso **avisos, int32_t *navisos);

/* Despeja o MODELO DE TIPOS da linguagem em JSON: todo módulo com seus
 * membros e os nomes dos parâmetros, e os métodos de cada tipo/objeto nativo.
 *
 * Sai do próprio motor (tabelas MODULOS[]/METODOS_*), que é a única fonte que
 * não tem como ficar defasada. É o que o editor (autocomplete) e a auditoria
 * de assinatura da doc consomem — antes isso era introspecção da stdlib do
 * interpretador, o que amarrava o tooling a ele. */
void ps_metadata_json(FILE *saida);

/* Os módulos importáveis, separados por vírgula (com quebras de linha), da
 * mesma tabela do `import` — o `--help` imprime isto em vez de uma lista
 * digitada que envelhece. */
const char *ps_modulos_publicos(void);

/* Marca a base da pilha do processo. É a referência da medição de folga que
 * impede a recursão profunda de estourar a pilha (ver `ps_pilha_apertada` em
 * jinga_vm.c). Tem que ser chamada no início do `main`, com a pilha ainda
 * praticamente intocada. */
void ps_pilha_marca_processo(void);

#endif /* PS_VM_H */
