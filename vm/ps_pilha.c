/*
 * Folga da pilha do C. Contrato e o PORQUÊ em `ps_pilha.h`.
 *
 * Em módulo próprio porque parser, VM e o teste de unidade precisam dele, e
 * porque medir folga não tem nada a ver com o que qualquer um dos três faz.
 */
#include <stddef.h>
#include <stdint.h>
#include <sys/resource.h>

#include "ps_pilha.h"

/* Base da pilha da execução corrente e quanto ela tem. Por thread porque o
 * pool de fibras e o jinker multi-processo rodam em contextos diferentes, cada
 * um com a sua pilha. Zero = ninguém marcou; aí não há o que medir. */
__thread const char *ps_pilha_base = NULL;
__thread size_t ps_pilha_tam = 0;

/* Margem: o que tem que sobrar embaixo, PROPORCIONAL ao tamanho da pilha.
 *
 * Margem fixa não serve, e custou uma rodada aqui: com 24 KB fixos a recursão
 * ainda estourava numa pilha de 8 MB. O motivo é que o espaço utilizável
 * abaixo da base marcada é MENOR que o `ulimit` — o bloco de ambiente, o argv
 * e a inicialização da libc ficam acima dela, e o kernel ainda reserva um
 * pedaço. A conta errava por umas dezenas de KB, que é justamente a ordem de
 * grandeza de uma margem apertada.
 *
 * Um oitavo, limitado entre 16 KB e 512 KB: 512 KB numa pilha de 8 MB (6%, que
 * não faz falta) e 16 KB nos 128 KB da fibra — uns 40 quadros de folga, com a
 * checagem rodando a cada quadro. */
static size_t ps_pilha_margem(size_t tam)
{
    size_t m = tam / 8;
    if (m < 16u * 1024)  m = 16u * 1024;
    if (m > 512u * 1024) m = 512u * 1024;
    return m;
}

/* Registra onde a pilha desta execução começa. Chamado na entrada do processo
 * e no trampolim de cada fibra — sem isso a medição não tem referência. */
void ps_pilha_marca(size_t tam)
{
    /* `__builtin_frame_address(0)`, e NÃO o endereço de um local: guardar
     * `&local` de uma função que já retornou é ponteiro pendurado — o gcc
     * avisa (`-Wdangling-pointer`) e, por ser UB, tem licença pra descartar a
     * comparação que o usa. Foi exatamente o que aconteceu: a medição
     * compilava, não avisava nada em execução, e simplesmente NÃO PROTEGIA. */
    ps_pilha_base = (const char *)__builtin_frame_address(0);
    ps_pilha_tam = tam;
}

/* Entrada do processo: a pilha é a do sistema. `getrlimit` dá o tamanho real
 * (8 MB no padrão do Linux, mas `ulimit -s` muda, e o `make oom` roda com
 * limite apertado de propósito). */
void ps_pilha_marca_processo(void)
{
    struct rlimit rl;
    size_t tam = 8u * 1024 * 1024;
    if (getrlimit(RLIMIT_STACK, &rl) == 0 && rl.rlim_cur != RLIM_INFINITY
            && rl.rlim_cur > 64u * 1024)
        tam = (size_t)rl.rlim_cur;
    ps_pilha_marca(tam);
}

/* 1 = está perto do fim da pilha. A pilha cresce PARA BAIXO em toda plataforma
 * que este motor roda, então o consumido é `base - atual`. */
int ps_pilha_apertada(void)
{
    if (!ps_pilha_base || ps_pilha_tam == 0) return 0;
    ptrdiff_t usado = ps_pilha_base - (const char *)__builtin_frame_address(0);
    if (usado < 0) usado = -usado;          /* pilha que cresce pra cima */
    return (size_t)usado + ps_pilha_margem(ps_pilha_tam) >= ps_pilha_tam;
}

void ps_pilha_le(const char **base, size_t *tam)
{
    *base = ps_pilha_base;
    *tam  = ps_pilha_tam;
}

void ps_pilha_repoe(const char *base, size_t tam)
{
    ps_pilha_base = base;
    ps_pilha_tam  = tam;
}
