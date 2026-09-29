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

/* ── resolução de nomes (o `gethostbyname_ex` e o `gethostbyaddr` do módulo
 * sockets) ────────────────────────────────────────────────────────────────
 * No Linux, as versões reentrantes da glibc (`_r`), que escrevem em `he` e
 * `buf`; no macOS elas não existem, e as comuns guardam o resultado POR THREAD
 * (o que as torna seguras). NULL = não resolveu, com o código em `*herr`
 * (HOST_NOT_FOUND, TRY_AGAIN, NO_RECOVERY). */
struct hostent;
struct hostent *ps_host_por_nome(const char *host, struct hostent *he,
                                 char *buf, size_t cap, int *herr);
struct hostent *ps_host_por_ip4(const void *v4, struct hostent *he,
                                char *buf, size_t cap, int *herr);

/* ── sockets ──────────────────────────────────────────────────────────────
 * No Linux e no macOS um socket é um descritor de arquivo: fecha com `close`,
 * lê com `read`, fica não-bloqueante por `fcntl`, e o timeout é um `timeval`.
 * No Windows é um SOCKET do Winsock: `closesocket`, `recv`/`send`,
 * `ioctlsocket`, timeout em milissegundos, e o erro vem de `WSAGetLastError`
 * (que aqui é traduzido pro `errno` de sempre, pra o resto do código não
 * mudar). Inline: no Linux isto é a própria chamada, sem custo. */
#if defined(PS_SO_WINDOWS)
#include "ps_win.h"
/* o erro do Winsock no `errno` de sempre (EAGAIN, EINTR, ECONNRESET...) */
static inline int ps_sock_errno(void)
{
    int w = WSAGetLastError();
    switch (w) {
        case WSAEWOULDBLOCK: case WSAETIMEDOUT: errno = EAGAIN; break;
        case WSAEINTR:       errno = EINTR; break;
        case WSAEINPROGRESS: case WSAEALREADY: errno = EINPROGRESS; break;
        case WSAECONNRESET:  errno = ECONNRESET; break;
        case WSAECONNREFUSED: errno = ECONNREFUSED; break;
        case WSAECONNABORTED: errno = ECONNABORTED; break;
        case WSAENOTCONN:    errno = ENOTCONN; break;
        case WSAEADDRINUSE:  errno = EADDRINUSE; break;
        case WSAEADDRNOTAVAIL: errno = EADDRNOTAVAIL; break;
        case WSAENETUNREACH: errno = ENETUNREACH; break;
        case WSAEHOSTUNREACH: errno = EHOSTUNREACH; break;
        case WSAEACCES:      errno = EACCES; break;
        case WSAEINVAL:      errno = EINVAL; break;
        case WSAEMSGSIZE:    errno = EMSGSIZE; break;
        case WSAESHUTDOWN:   errno = EPIPE; break;
        default:             errno = EIO; break;
    }
    return errno;
}
static inline void ps_so_inicia(void)
{
    WSADATA w;
    WSAStartup(MAKEWORD(2, 2), &w);
}
static inline int  ps_sock_fecha(int fd) { return closesocket((SOCKET)fd); }
static inline long ps_sock_le(int fd, void *b, size_t n)
{
    int r = recv((SOCKET)fd, (char *)b, (int)n, 0);
    if (r < 0) ps_sock_errno();
    return r;
}
static inline long ps_sock_escreve(int fd, const void *b, size_t n, int flags)
{
    int r = send((SOCKET)fd, (const char *)b, (int)n, flags);
    if (r < 0) ps_sock_errno();
    return r;
}
static inline int ps_sock_nao_bloqueia(int fd, int sim)
{
    u_long m = sim ? 1 : 0;
    return ioctlsocket((SOCKET)fd, FIONBIO, &m) == 0 ? 0 : -1;
}
/* timeout de leitura / escrita em ms; negativo = não mexe naquele lado */
static inline int ps_sock_timeout(int fd, int ms_le, int ms_escreve)
{
    int rc = 0;
    if (ms_le >= 0) {
        DWORD t = (DWORD)ms_le;
        rc |= setsockopt((SOCKET)fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&t, sizeof(t));
    }
    if (ms_escreve >= 0) {
        DWORD t = (DWORD)ms_escreve;
        rc |= setsockopt((SOCKET)fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&t, sizeof(t));
    }
    return rc;
}
/* 1 = há o que ler (ou o outro lado fechou) em até `ms` */
static inline int ps_sock_espera_leitura(int fd, int ms)
{
    WSAPOLLFD p;
    p.fd = (SOCKET)fd; p.events = POLLRDNORM; p.revents = 0;
    return WSAPoll(&p, 1, ms) > 0 && (p.revents & (POLLRDNORM | POLLHUP | POLLERR));
}
#else
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
static inline int  ps_sock_errno(void) { return errno; }
static inline void ps_so_inicia(void) { }
static inline int  ps_sock_fecha(int fd) { return close(fd); }
static inline long ps_sock_le(int fd, void *b, size_t n) { return (long)read(fd, b, n); }
static inline long ps_sock_escreve(int fd, const void *b, size_t n, int flags)
{
    return flags ? (long)send(fd, b, n, flags) : (long)write(fd, b, n);
}
static inline int ps_sock_nao_bloqueia(int fd, int sim)
{
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl < 0) return -1;
    return fcntl(fd, F_SETFL, sim ? (fl | O_NONBLOCK) : (fl & ~O_NONBLOCK));
}
static inline int ps_sock_timeout(int fd, int ms_le, int ms_escreve)
{
    int rc = 0;
    if (ms_le >= 0) {
        struct timeval tv;
        tv.tv_sec = ms_le / 1000;
        tv.tv_usec = (ms_le % 1000) * 1000;
        rc |= setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }
    if (ms_escreve >= 0) {
        struct timeval tv;
        tv.tv_sec = ms_escreve / 1000;
        tv.tv_usec = (ms_escreve % 1000) * 1000;
        rc |= setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    }
    return rc;
}
#include <poll.h>
static inline int ps_sock_espera_leitura(int fd, int ms)
{
    struct pollfd p = { fd, POLLIN, 0 };
    return poll(&p, 1, ms) > 0 && (p.revents & (POLLIN | POLLHUP));
}
#endif

/* ── certificados TLS ──────────────────────────────────────────────────────
 * Os certificados raiz em que o cliente HTTPS/SMTP confia: os do OpenSSL
 * (`set_default_verify_paths`) e, no Windows e no macOS, os do PRÓPRIO
 * sistema (o repositório ROOT do Windows, as âncoras do Keychain do macOS) —
 * senão o pacote levado pra outra máquina recusaria todo certificado.
 * `ctx` é um `SSL_CTX *`. */
void ps_so_tls_cas(void *ctx);

/* ── caminhos ─────────────────────────────────────────────────────────────
 * O caminho absoluto do executável que está rodando (o `-o`, o
 * `sys.executable`, o reload do jinker). 0 = ok. */
int ps_meu_executavel(char *buf, size_t cap);

#endif
