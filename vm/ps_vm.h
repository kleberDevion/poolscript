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

typedef enum {
    PS_ERRO_NENHUM = 0,
    PS_ERRO_SINTAXE,        /* lexer ou parser */
    PS_ERRO_NAO_SUPORTADO,  /* nó que o compilador ainda não emite */
    PS_ERRO_RUNTIME,
    PS_ERRO_MEMORIA
} PSTipoErro;

typedef struct {
    PSTipoErro tipo;
    char       msg[256];
    char       tipo_nome[64];  /* nome do erro em runtime — o que o `catch` compara */
    int        linha;
    int        col;
} PSErroExec;

/* Roda o `.ps` inteiro. 0 = sucesso; -1 preenche `e`. */
/* `caminho` ancora o `import` de arquivo vizinho; NULL só busca lib global. */
/* Argumentos DO USUÁRIO (o que vem depois do arquivo `.ps`) — é o que o
 * `sys.argv` enxerga. Chamar antes de rodar; sem isso a lista sai vazia. */
void ps_set_argv(int argc, char **argv);

int ps_roda_fonte(const char *fonte, size_t len, const char *caminho, PSErroExec *e);

/* Só VERIFICA (lexer → parser → compilador), NUNCA roda. Para o LSP/editor:
 * usa exatamente a gramática da VM pra apontar erro de sintaxe/compilação sem
 * executar o código do usuário. 0 = sem erro; -1 preenche `e`. */
int ps_verifica_fonte(const char *fonte, size_t len, const char *caminho, PSErroExec *e);

#endif /* PS_VM_H */
