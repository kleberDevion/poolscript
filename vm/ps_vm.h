/*
 * Entrada pura da VM da PoolScript — sem `Python.h`.
 *
 * O mesmo `poolscript_vm.c` compila de dois jeitos:
 *
 *   com  -DPS_MODULO_PYTHON  → extensão `.so` do CPython (testes diferenciais)
 *   sem  ela                 → objeto de um binário standalone
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
    PS_ERRO_MEMORIA
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
    /* Traceback do runtime: do mais externo (<module>) ao mais interno, na
     * ordem do Python. `ntb == 0` quando não há (erro de sintaxe, etc.). */
    PSQuadroTB tb[64];
    int        ntb;
} PSErroExec;

/* Roda o `.ps` inteiro. 0 = sucesso; -1 preenche `e`. */
/* `caminho` ancora o `import` de arquivo vizinho; NULL só busca lib global. */
/* Argumentos DO USUÁRIO (o que vem depois do arquivo `.ps`) — é o que o
 * `sys.argv` enxerga. Chamar antes de rodar; sem isso a lista sai vazia. */
void ps_set_argv(int argc, char **argv);

/* Liga o depurador: o motor escuta o Debug Adapter Protocol em
 * 127.0.0.1:<porta> e só executa a primeira instrução depois que o editor
 * conectar e terminar o aperto de mão — senão os breakpoints chegariam tarde.
 * A saída do programa continua no stdout dele; o protocolo vai pelo socket. */
void ps_debug_porta(int porta);

int ps_roda_fonte(const char *fonte, size_t len, const char *caminho, PSErroExec *e);

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
 * de assinatura da doc consomem — antes isso era introspecção da stdlib em
 * Python, o que amarrava o tooling ao interpretador. */
void ps_metadata_json(FILE *saida);

/* Marca a base da pilha do processo. É a referência da medição de folga que
 * impede a recursão profunda de estourar a pilha (ver `ps_pilha_apertada` em
 * poolscript_vm.c). Tem que ser chamada no início do `main`, com a pilha ainda
 * praticamente intocada. */
void ps_pilha_marca_processo(void);

#endif /* PS_VM_H */
