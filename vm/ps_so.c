/* ps_so.c — as implementações da camada de sistema (ver ps_so.h).
 *
 * Três ramos, escolhidos pelo compilador de cada sistema:
 *   Linux    epoll + eventfd + mmap + /proc/self/exe (o código de sempre)
 *   macOS    kqueue + pipe + mmap + _NSGetExecutablePath
 *   Windows  WSAPoll + socket de loopback + VirtualAlloc + GetModuleFileName */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include "ps_so.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(PS_SO_WINDOWS)
#  error "o ramo do Windows da camada de sistema ainda nao foi escrito"
#else

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#if defined(PS_SO_LINUX)
#  include <sys/epoll.h>
#  include <sys/eventfd.h>
#else
#  include <sys/types.h>
#  include <sys/event.h>
#  include <sys/time.h>
#  include <mach-o/dyld.h>
#endif

/* ── eventos ─────────────────────────────────────────────────────────── */

#if defined(PS_SO_LINUX)

int ps_ev_cria(void)
{
    return epoll_create1(EPOLL_CLOEXEC);
}

void ps_ev_fecha(int ev)
{
    if (ev >= 0) close(ev);
}

int ps_ev_poe(int ev, int fd, void *ptr, int ler)
{
    struct epoll_event e;
    memset(&e, 0, sizeof(e));
    e.events = ler ? EPOLLIN : 0;
    e.data.ptr = ptr;
    return epoll_ctl(ev, EPOLL_CTL_ADD, fd, &e);
}

int ps_ev_arma(int ev, int fd, void *ptr, int ler)
{
    struct epoll_event e;
    memset(&e, 0, sizeof(e));
    e.events = ler ? EPOLLIN : 0;
    e.data.ptr = ptr;
    return epoll_ctl(ev, EPOLL_CTL_MOD, fd, &e);
}

int ps_ev_tira(int ev, int fd)
{
    return epoll_ctl(ev, EPOLL_CTL_DEL, fd, NULL);
}

int ps_ev_espera(int ev, PsEvento *out, int max, int timeout_ms)
{
    struct epoll_event evs[256];
    if (max > 256) max = 256;
    int n = epoll_wait(ev, evs, max, timeout_ms);
    for (int i = 0; i < n; i++) {
        out[i].ptr = evs[i].data.ptr;
        out[i].leu = (evs[i].events & EPOLLIN) != 0;
        out[i].hup = (evs[i].events & (EPOLLHUP | EPOLLERR)) != 0;
    }
    return n;
}

#else  /* macOS: kqueue */

/* O kqueue não é herdado no `fork` (o epoll é): quem cria depois do fork
 * (o worker do jinker) já cria o seu, que é o que o motor faz. */
int ps_ev_cria(void)
{
    int k = kqueue();
    if (k >= 0) fcntl(k, F_SETFD, FD_CLOEXEC);
    return k;
}

void ps_ev_fecha(int ev)
{
    if (ev >= 0) close(ev);
}

/* EVFILT_READ sem EV_CLEAR é por nível, como o epoll sem EPOLLET; `ler = 0`
 * é o filtro registrado e DESLIGADO (EV_DISABLE), o mesmo efeito do
 * `EPOLL_CTL_MOD` com eventos 0. */
static int kq_muda(int ev, int fd, void *ptr, unsigned short acao)
{
    struct kevent k;
    EV_SET(&k, (uintptr_t)fd, EVFILT_READ, acao, 0, 0, ptr);
    return kevent(ev, &k, 1, NULL, 0, NULL) < 0 ? -1 : 0;
}

int ps_ev_poe(int ev, int fd, void *ptr, int ler)
{
    return kq_muda(ev, fd, ptr, (unsigned short)(EV_ADD | (ler ? EV_ENABLE : EV_DISABLE)));
}

int ps_ev_arma(int ev, int fd, void *ptr, int ler)
{
    return kq_muda(ev, fd, ptr, (unsigned short)(EV_ADD | (ler ? EV_ENABLE : EV_DISABLE)));
}

int ps_ev_tira(int ev, int fd)
{
    return kq_muda(ev, fd, NULL, EV_DELETE);
}

int ps_ev_espera(int ev, PsEvento *out, int max, int timeout_ms)
{
    struct kevent evs[256];
    if (max > 256) max = 256;
    struct timespec t, *pt = NULL;
    if (timeout_ms >= 0) {
        t.tv_sec = timeout_ms / 1000;
        t.tv_nsec = (long)(timeout_ms % 1000) * 1000000L;
        pt = &t;
    }
    int n = kevent(ev, NULL, 0, evs, max, pt);
    for (int i = 0; i < n; i++) {
        out[i].ptr = evs[i].udata;
        out[i].hup = (evs[i].flags & (EV_EOF | EV_ERROR)) != 0;
        /* EV_ERROR no retorno é erro do registro, não dado */
        out[i].leu = !(evs[i].flags & EV_ERROR);
    }
    return n;
}

#endif

/* ── aviso entre threads ───────────────────────────────────────────────── */

#if defined(PS_SO_LINUX)

/* eventfd: um contador de 8 bytes; `le` e `escreve` são o mesmo fd */
int ps_aviso_cria(PsAviso *a)
{
    int efd = eventfd(0, EFD_CLOEXEC);
    if (efd < 0) return -1;
    a->le = a->escreve = efd;
    return 0;
}

void ps_aviso_sinaliza(PsAviso a)
{
    uint64_t um = 1;
    ssize_t w = write(a.escreve, &um, sizeof(um));
    (void)w;
}

void ps_aviso_espera(PsAviso a)
{
    uint64_t drena;
    ssize_t r = read(a.le, &drena, sizeof(drena));
    (void)r;
}

void ps_aviso_fecha(PsAviso a)
{
    if (a.le >= 0) close(a.le);
}

#else  /* macOS: pipe */

int ps_aviso_cria(PsAviso *a)
{
    int p[2];
    if (pipe(p) != 0) return -1;
    fcntl(p[0], F_SETFD, FD_CLOEXEC);
    fcntl(p[1], F_SETFD, FD_CLOEXEC);
    a->le = p[0];
    a->escreve = p[1];
    return 0;
}

void ps_aviso_sinaliza(PsAviso a)
{
    char um = 1;
    ssize_t w = write(a.escreve, &um, 1);
    (void)w;
}

void ps_aviso_espera(PsAviso a)
{
    char drena;
    ssize_t r = read(a.le, &drena, 1);
    (void)r;
}

void ps_aviso_fecha(PsAviso a)
{
    if (a.le >= 0) close(a.le);
    if (a.escreve >= 0) close(a.escreve);
}

#endif

/* ── memória ───────────────────────────────────────────────────────────── */

static size_t pagina(void)
{
    long pg = sysconf(_SC_PAGESIZE);
    return pg > 0 ? (size_t)pg : 4096;
}

void *ps_pilha_fibra_nova(size_t util)
{
    size_t pg = pagina();
    size_t total = util + pg;                        /* guarda + pilha */
    void *base = mmap(NULL, total, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANON, -1, 0);
    if (base == MAP_FAILED) return NULL;
    /* a guarda vai no começo do mapeamento: é pra lá que a pilha desce */
    if (mprotect(base, pg, PROT_NONE) != 0) { munmap(base, total); return NULL; }
    return (char *)base + pg;                        /* o utilizável começa depois */
}

void ps_pilha_fibra_solta(void *p, size_t util)
{
    if (!p) return;
    size_t pg = pagina();
    munmap((char *)p - pg, util + pg);
}

void *ps_mem_exec_nova(const void *codigo, size_t n, size_t *tam)
{
    size_t pg = pagina();
    size_t t = (n + pg - 1) & ~(pg - 1);
    void *mem = mmap(NULL, t, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (mem == MAP_FAILED) return NULL;
    memcpy(mem, codigo, n);
    if (mprotect(mem, t, PROT_READ | PROT_EXEC) != 0) { munmap(mem, t); return NULL; }
    *tam = t;
    return mem;
}

void ps_mem_exec_solta(void *p, size_t tam)
{
    if (p) munmap(p, tam);
}

/* ── caminhos ──────────────────────────────────────────────────────────── */

int ps_meu_executavel(char *buf, size_t cap)
{
    if (!buf || cap < 2) return -1;
#if defined(PS_SO_LINUX)
    ssize_t n = readlink("/proc/self/exe", buf, cap - 1);
    if (n <= 0) return -1;
    buf[n] = '\0';
    return 0;
#else
    char bruto[4096];
    uint32_t tb = (uint32_t)sizeof(bruto);
    if (_NSGetExecutablePath(bruto, &tb) != 0) return -1;
    char resolvido[4096];
    if (!realpath(bruto, resolvido)) return -1;
    size_t l = strlen(resolvido);
    if (l + 1 > cap) return -1;
    memcpy(buf, resolvido, l + 1);
    return 0;
#endif
}

#endif /* !PS_SO_WINDOWS */
