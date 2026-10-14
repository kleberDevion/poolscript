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
#include <stdio.h>
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

/* ── quantidade e tipo dos argumentos (scripts/mede_aridades.pr) ── */

/* Cada texto da tabela é uma POSIÇÃO no bloco `AR_TEXTO` (0 = NULL), não um
 * ponteiro. Ponteiro em dado inicializado num executável PIE é uma relocação
 * que o carregador faz em toda partida, antes do `main`: com ponteiros, esta
 * tabela somava 2.975 relocações e 39 mil instruções a cada programa, até o
 * vazio (medido com callgrind no 17.0.17: `partida` +4,3%). */
#include <stdint.h>
typedef struct {
    uint32_t dono, membro, nome, params;
    int32_t  minimo, maximo;
    uint32_t recusa[PS_ARIDADE_NTIPOS];
} PSAridadeBruta;

#include "aridades_medidas.inc"   /* AR_TEXTO e AR_TAB */

#define N_ARIDADES (sizeof(AR_TAB) / sizeof(AR_TAB[0]))
#define AR_TXT(o)  ((o) ? AR_TEXTO + (o) : NULL)

static int compara_ar(const void *chave, const void *item)
{
    const char *const *k = chave;          /* k[0] = dono, k[1] = membro */
    const PSAridadeBruta *b = item;
    int c = strcmp(k[0], AR_TEXTO + b->dono);
    return c ? c : strcmp(k[1], AR_TEXTO + b->membro);
}

int ps_aridade_de(const char *dono, const char *membro, PSAridade *out)
{
    if (!dono || !membro) return 0;
    const char *k[2] = { dono, membro };
    const PSAridadeBruta *b = bsearch(k, AR_TAB, N_ARIDADES, sizeof(AR_TAB[0]), compara_ar);
    if (!b) return 0;
    if (out) {
        out->dono = AR_TEXTO + b->dono;
        out->membro = AR_TEXTO + b->membro;
        out->nome = AR_TXT(b->nome);
        out->params = AR_TXT(b->params);
        out->minimo = b->minimo;
        out->maximo = b->maximo;
        for (int i = 0; i < PS_ARIDADE_NTIPOS; i++) out->recusa[i] = AR_TXT(b->recusa[i]);
    }
    return 1;
}

int ps_aridade_tipo(const char *tipo)
{
    static const char *const T[PS_ARIDADE_NTIPOS] = {
        "int", "flo", "str", "list", "tup", "dict", "byte", "bool", "Null"
    };
    if (!tipo) return -1;
    for (int i = 0; i < PS_ARIDADE_NTIPOS; i++)
        if (!strcmp(tipo, T[i])) return i;
    return -1;
}

void ps_frase_aridade(char *buf, size_t cap, const char *nome, int minimo, int maximo, int n)
{
    char quanto[64];
    if (maximo == 0)
        snprintf(quanto, sizeof(quanto), "no arguments");
    else if (minimo == maximo)
        snprintf(quanto, sizeof(quanto),
                 minimo == 1 ? "exactly one argument" : "exactly %d arguments", minimo);
    else if (n < minimo)
        snprintf(quanto, sizeof(quanto), "at least %d argument%s",
                 minimo, minimo == 1 ? "" : "s");
    else
        snprintf(quanto, sizeof(quanto), "at most %d argument%s",
                 maximo, maximo == 1 ? "" : "s");
    snprintf(buf, cap, "%s() takes %s (%d given)", nome, quanto, n);
}
