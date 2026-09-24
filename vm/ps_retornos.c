/*
 * Tabela de retorno dos nativos. O conteúdo vem INTEIRO de
 * `retornos_medidos.inc`, gerado por scripts/mede_retornos.pr: não há entrada
 * escrita à mão. A tabela à mão que vivia no jinga_vm.c declarou tipo que
 * o motor não devolve (`os.PoolFile -> PoolFile`, quando o valor é o próprio
 * tipo) e deixava sem retorno a maior parte dos membros.
 *
 * O gerador grava em ordem de bytes de (dono, membro), a mesma do `strcmp` em
 * par, e a busca é binária: o compilador consulta a tabela em cada chamada de
 * nativo do programa.
 */
#include <stdlib.h>
#include <string.h>

#include "ps_retornos.h"

typedef struct { const char *dono; const char *membro; const char *tipo; } PSRetorno;

static const PSRetorno RETORNOS[] = {
#include "retornos_medidos.inc"
};

#define N_RETORNOS (sizeof(RETORNOS) / sizeof(RETORNOS[0]))

static int compara(const void *chave, const void *item)
{
    const PSRetorno *a = chave, *b = item;
    int c = strcmp(a->dono, b->dono);
    return c ? c : strcmp(a->membro, b->membro);
}

int ps_retorno_tipo_existe(const char *nome)
{
    if (!nome || !*nome) return 0;
    size_t n = strlen(nome);
    for (size_t i = 0; i < N_RETORNOS; i++) {
        const char *t = RETORNOS[i].tipo;
        while (*t) {
            const char *bar = strchr(t, '|');
            size_t len = bar ? (size_t)(bar - t) : strlen(t);
            if (len == n && memcmp(t, nome, n) == 0) return 1;
            if (!bar) break;
            t = bar + 1;
        }
    }
    return 0;
}

const char *ps_retorno_de(const char *dono, const char *membro)
{
    if (!dono || !membro) return NULL;
    PSRetorno chave = { dono, membro, NULL };
    const PSRetorno *r = bsearch(&chave, RETORNOS, N_RETORNOS, sizeof(RETORNOS[0]), compara);
    return r ? r->tipo : NULL;
}
