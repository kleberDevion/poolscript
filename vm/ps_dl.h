/*
 * Biblioteca do sistema carregada na PRIMEIRA vez que o programa usa o módulo
 * dela — nunca na partida.
 *
 * O `pool` era ligado ao cliente do Postgres, do MySQL, do ODBC e do Mongo, e o
 * sistema carregava todos eles (e o que eles arrastam: Kerberos, LDAP, gnutls,
 * 36 bibliotecas) antes da primeira linha de qualquer programa. Medido com
 * callgrind: 90% do trabalho de um `pool --version` era o carregador dinâmico
 * relocando símbolos dessas bibliotecas, e mais 8% o construtor do gnutls. Um
 * `post(1)` pagava o banco que ele nunca usou — e o programa nem abria numa
 * máquina sem o cliente do Mongo instalado.
 *
 * Agora cada driver declara a lista das funções que usa (uma macro X por
 * biblioteca), guarda um ponteiro por função e resolve todos com `dlsym` no
 * primeiro `connect`. O código que chama continua escrevendo `PQexec(...)`: um
 * `#define PQexec dl_PQexec` troca o nome pelo ponteiro.
 */
#ifndef PS_DL_H
#define PS_DL_H

#include <pthread.h>
#include <stdio.h>
#if defined(_WIN32)
/* Windows: `LoadLibrary`/`GetProcAddress` fazem o papel do `dlopen`/`dlsym`.
 * O carregador procura a DLL na pasta do executável primeiro — é onde o
 * pacote do Windows põe as dos clientes de banco. */
#include "ps_win.h"
static inline void *ps_dl_sim(void *h, const char *nome)
{
    return (void *)GetProcAddress((HMODULE)h, nome);
}
#define PS_DL_SIM(h, nome) ps_dl_sim((h), (nome))
#else
#include <dlfcn.h>
#define PS_DL_SIM(h, nome) dlsym((h), (nome))
#endif

/* Um ponteiro por função da lista, com o tipo da declaração do cabeçalho da
 * biblioteca (o cabeçalho continua incluído; só a LIGAÇÃO sai). */
#define PS_DL_PONTEIRO(f) static __typeof__(f) *dl_##f;

/* Resolve uma função da lista; a primeira que faltar interrompe (`faltou`). */
#define PS_DL_RESOLVE(f)                                                      \
    if (!faltou && !(dl_##f = (__typeof__(dl_##f))PS_DL_SIM(h_, #f))) faltou = #f;

/* Abre a primeira biblioteca da lista (terminada em NULL) que o sistema achar.
 * NULL = nenhuma; o motivo do carregador vai em `erro`. */
static inline void *ps_dl_abre(const char *const *nomes, char *erro, size_t cap)
{
    /* o motivo que vale é o do PRIMEIRO nome (o que a frase cita); os outros
     * são só alternativas de outras distribuições */
    erro[0] = '\0';
    for (int i = 0; nomes[i]; i++) {
#if defined(_WIN32)
        HMODULE h = LoadLibraryA(nomes[i]);
        if (h) return (void *)h;
        if (i == 0) snprintf(erro, cap, "%s: erro %lu do LoadLibrary", nomes[i],
                             (unsigned long)GetLastError());
#else
        void *h = dlopen(nomes[i], RTLD_NOW | RTLD_LOCAL);
        if (h) return h;
        const char *e = dlerror();
        if (i == 0) snprintf(erro, cap, "%s", e ? e : "biblioteca nao encontrada");
#endif
    }
    return NULL;
}

/* Carrega a biblioteca `rotulo` UMA vez (thread-safe: o `connect` roda na
 * thread do pool quando está numa fibra) e resolve a lista de funções.
 *   `estado`  — 0 = nunca tentou, 1 = carregou, -1 = falhou (e não tenta de novo);
 *   `motivo`  — a frase da falha, guardada pra toda conexão seguinte;
 *   `RESOLVE` — a lista da biblioteca aplicada a PS_DL_RESOLVE.
 * Devolve 0, ou -1 com a frase em `erro`. */
#define PS_DL_CARREGA(estado, motivo, nomes, rotulo, lista, erro, ecap)       \
    do {                                                                      \
        static pthread_mutex_t mx_ = PTHREAD_MUTEX_INITIALIZER;               \
        pthread_mutex_lock(&mx_);                                             \
        if ((estado) == 0) {                                                  \
            char por_[160] = "";                                              \
            void *h_ = ps_dl_abre((nomes), por_, sizeof(por_));               \
            const char *faltou = NULL;                                        \
            if (h_) {                                                         \
                lista(PS_DL_RESOLVE)                                          \
                if (faltou) snprintf(por_, sizeof(por_), "falta a funcao %s", faltou); \
            }                                                                 \
            if (h_ && !faltou) (estado) = 1;                                  \
            else {                                                            \
                (estado) = -1;                                                \
                snprintf((motivo), sizeof(motivo),                            \
                         "%s precisa da biblioteca %s, que nao foi encontrada (%s)", \
                         (rotulo), (nomes)[0], por_);                         \
            }                                                                 \
        }                                                                     \
        pthread_mutex_unlock(&mx_);                                           \
        if ((estado) != 1) { snprintf((erro), (ecap), "%s", (motivo)); }      \
    } while (0)

#endif /* PS_DL_H */
