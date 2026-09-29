/* ps_so.h — a camada de SISTEMA OPERACIONAL do motor.
 *
 * O motor é UM só (o mesmo C) em Linux, macOS e Windows. O que muda de um
 * sistema pro outro mora aqui, atrás de uma API pequena — só o que o motor
 * usa de fato:
 *
 *   eventos    o laço de prontidão do jinker e das tarefas async
 *              (epoll no Linux, kqueue no macOS, WSAPoll no Windows)
 *   aviso      a thread do pool acordando o laço (eventfd / pipe / socket)
 *   memória    pilha de fibra com página de guarda e memória executável
 *              do JIT (mmap+mprotect / VirtualAlloc+VirtualProtect)
 *   caminho    o executável em que o motor está rodando
 *
 * O resto do C chama estas funções em vez do sistema. No Linux o código que
 * está dentro delas é o mesmo que estava espalhado pelo motor. */
#ifndef PS_SO_H
#define PS_SO_H

#include <stddef.h>

#if defined(_WIN32)
#  define PS_SO_WINDOWS 1
#  define PS_SO_NOME "windows"
#elif defined(__APPLE__)
#  define PS_SO_MAC 1
#  define PS_SO_NOME "darwin"
#else
#  define PS_SO_LINUX 1
#  define PS_SO_NOME "linux"
#endif

/* ── eventos (nível, só leitura) ─────────────────────────────────────────
 * `ptr` volta intacto no evento: o motor põe ali um ponteiro cujo primeiro
 * campo é o `int tipo` do observador. `ler = 0` registra o fd DESARMADO
 * (o `EPOLL_CTL_MOD` com eventos 0 de antes): não dispara até `ps_ev_arma`. */
typedef struct {
    void *ptr;
    int   leu;      /* há dado pra ler (ou o outro lado fechou) */
    int   hup;      /* o outro lado fechou / erro no fd */
} PsEvento;

int  ps_ev_cria(void);                                    /* -1 = erro (errno) */
void ps_ev_fecha(int ev);
int  ps_ev_poe(int ev, int fd, void *ptr, int ler);       /* 0 = ok */
int  ps_ev_arma(int ev, int fd, void *ptr, int ler);      /* liga/desliga a leitura */
int  ps_ev_tira(int ev, int fd);
int  ps_ev_espera(int ev, PsEvento *out, int max, int timeout_ms);  /* n de eventos, -1 erro */

/* ── aviso entre threads ──────────────────────────────────────────────────
 * Um aviso = um disparo: a thread do pool chama `sinaliza` quando termina a
 * I/O bloqueante; o laço registra `le` em `ps_ev_poe` e, quando dispara,
 * `espera` consome o sinal (bloqueia até ele chegar, se ainda não chegou). */
typedef struct {
    int le;
    int escreve;
} PsAviso;

int  ps_aviso_cria(PsAviso *a);          /* 0 = ok */
void ps_aviso_sinaliza(PsAviso a);
void ps_aviso_espera(PsAviso a);
void ps_aviso_fecha(PsAviso a);

/* ── memória ──────────────────────────────────────────────────────────────
 * Pilha de fibra: `util` bytes utilizáveis com uma página sem acesso logo
 * ABAIXO (onde a pilha chega ao estourar): o estouro vira falha no ponto
 * exato em vez de escrita silenciosa no heap. */
void *ps_pilha_fibra_nova(size_t util);
void  ps_pilha_fibra_solta(void *p, size_t util);

/* Memória executável do JIT: copia `n` bytes de código pra uma região nova
 * que termina só LEITURA+EXECUÇÃO (nunca escrita e execução ao mesmo tempo).
 * `*tam` recebe o tamanho reservado, que `ps_mem_exec_solta` devolve. */
void *ps_mem_exec_nova(const void *codigo, size_t n, size_t *tam);
void  ps_mem_exec_solta(void *p, size_t tam);

/* ── caminhos ─────────────────────────────────────────────────────────────
 * O caminho absoluto do executável que está rodando (o `-o`, o
 * `sys.executable`, o reload do jinker). 0 = ok. */
int ps_meu_executavel(char *buf, size_t cap);

#endif
