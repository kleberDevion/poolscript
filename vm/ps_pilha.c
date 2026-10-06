/*
 * Folga da pilha do C. Contrato e o PORQUÊ em `ps_pilha.h`.
 *
 * Em módulo próprio porque parser, VM e o teste de unidade precisam dele, e
 * porque medir folga não tem nada a ver com o que qualquer um dos três faz.
 */
#include <stddef.h>
#include <stdint.h>
#if defined(_WIN32)
#include "ps_win.h"
#else
#include <sys/resource.h>
#endif

#include "ps_pilha.h"

/* Base da pilha da execução corrente e quanto ela tem. Por thread porque o
 * pool de fibras e o jinker multi-processo rodam em contextos diferentes, cada
 * um com a sua pilha. Zero = ninguém marcou; aí não há o que medir. */
__thread const char *ps_pilha_base = NULL;
__thread size_t ps_pilha_tam = 0;

/* O MAIOR degrau já visto entre duas conferências seguidas (o quanto a pilha
 * desceu de uma pra outra) e onde foi a última. A margem fixa foi medida com o
 * gcc, que põe 12 KB no quadro do laço da VM; o clang põe 23 KB (medido com
 * `-fstack-usage`) e o clang da Apple estourava a página de guarda da fibra
 * antes do RecursionError. A folga passa a ser pelo menos DOIS degraus reais
 * deste binário, seja qual for o compilador. */
__thread const char *ps_pilha_ultimo = NULL;
__thread size_t ps_pilha_passo = 0;

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
    ps_pilha_ultimo = NULL;          /* outra pilha: o degrau recomeça dela */
}

/* Entrada do processo: a pilha é a do sistema. `getrlimit` dá o tamanho real
 * (8 MB no padrão do Linux, mas `ulimit -s` muda o valor — por isso é medido,
 * não uma constante). */
void ps_pilha_marca_processo(void)
{
    size_t tam = 8u * 1024 * 1024;
#if defined(_WIN32)
    /* Windows: a pilha da thread é reservada no executável (-Wl,--stack); os
     * limites dela o próprio sistema diz. O `usado` é medido do quadro atual,
     * então o tamanho é o que resta daqui até o limite baixo. */
    ULONG_PTR baixo = 0, alto = 0;
    GetCurrentThreadStackLimits(&baixo, &alto);
    const char *aqui = (const char *)__builtin_frame_address(0);
    if (baixo && (ULONG_PTR)aqui > baixo) tam = (size_t)((ULONG_PTR)aqui - baixo);
#else
    struct rlimit rl;
    if (getrlimit(RLIMIT_STACK, &rl) == 0 && rl.rlim_cur != RLIM_INFINITY
            && rl.rlim_cur > 64u * 1024)
        tam = (size_t)rl.rlim_cur;
#endif
    ps_pilha_marca(tam);
}

/* 1 = está perto do fim da pilha. A pilha cresce PARA BAIXO em toda plataforma
 * que este motor roda, então o consumido é `base - atual`. */
/* embutida sempre: a pergunta roda por comparação no sort e por nó no repr,
 * e um quadro a mais aqui seria um quadro a mais em cada uma */
static inline __attribute__((always_inline)) int apertada_com(const char *aqui, size_t vezes)
{
    /* degrau: só descida, e só dentro da mesma pilha (a troca de fibra pula
     * pra outro mapeamento — distância que não é quadro nenhum) */
    if (ps_pilha_ultimo && aqui < ps_pilha_ultimo) {
        size_t d = (size_t)(ps_pilha_ultimo - aqui);
        if (d > ps_pilha_passo && d < ps_pilha_tam / 2) ps_pilha_passo = d;
    }
    ps_pilha_ultimo = aqui;
    ptrdiff_t usado = ps_pilha_base - aqui;
    if (usado < 0) usado = -usado;          /* pilha que cresce pra cima */
    size_t margem = ps_pilha_margem(ps_pilha_tam);
    if (margem < 2 * ps_pilha_passo) margem = 2 * ps_pilha_passo;
    margem *= vezes;
    return (size_t)usado + margem >= ps_pilha_tam;
}

int ps_pilha_apertada(void)
{
    if (!ps_pilha_base || ps_pilha_tam == 0) return 0;
    return apertada_com((const char *)__builtin_frame_address(0), 1);
}

/* O freio dos blocos: o dobro da margem (ver `ps_pilha.h`). */
int ps_pilha_apertada_bloco(void)
{
    if (!ps_pilha_base || ps_pilha_tam == 0) return 0;
    return apertada_com((const char *)__builtin_frame_address(0), 2);
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
    ps_pilha_ultimo = NULL;
}
