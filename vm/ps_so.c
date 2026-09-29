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

#include "ps_win.h"
#include <wincrypt.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>

/* ── eventos: WSAPoll sobre uma tabela ─────────────────────────────────────
 * O Windows não tem epoll; o WSAPoll é o `poll` dos sockets. Cada laço é uma
 * tabela (fd, ptr, armado); `ps_ev_espera` monta o vetor dos armados e chama
 * o WSAPoll. O epoll esquece sozinho um fd fechado; aqui, um socket que o
 * Windows já invalidou (POLLNVAL) sai da tabela na volta seguinte, sem virar
 * evento — o ptr dele pode já ter sido liberado. */
typedef struct {
    int    usado;
    int    n, cap;
    int   *fd;
    void **ptr;
    int   *armado;
} WinLaco;

static WinLaco g_lacos[16];
static CRITICAL_SECTION g_lacos_mx;
static INIT_ONCE g_lacos_uma = INIT_ONCE_STATIC_INIT;

static BOOL CALLBACK lacos_inicia(PINIT_ONCE u, PVOID p, PVOID *c)
{
    (void)u; (void)p; (void)c;
    InitializeCriticalSection(&g_lacos_mx);
    return TRUE;
}

int ps_ev_cria(void)
{
    InitOnceExecuteOnce(&g_lacos_uma, lacos_inicia, NULL, NULL);
    EnterCriticalSection(&g_lacos_mx);
    for (int i = 0; i < 16; i++) {
        if (!g_lacos[i].usado) {
            memset(&g_lacos[i], 0, sizeof(g_lacos[i]));
            g_lacos[i].usado = 1;
            LeaveCriticalSection(&g_lacos_mx);
            return i;
        }
    }
    LeaveCriticalSection(&g_lacos_mx);
    errno = EMFILE;
    return -1;
}

void ps_ev_fecha(int ev)
{
    if (ev < 0 || ev >= 16) return;
    WinLaco *l = &g_lacos[ev];
    free(l->fd); free(l->ptr); free(l->armado);
    memset(l, 0, sizeof(*l));
}

static int laco_acha(WinLaco *l, int fd)
{
    for (int i = 0; i < l->n; i++) if (l->fd[i] == fd) return i;
    return -1;
}

int ps_ev_poe(int ev, int fd, void *ptr, int ler)
{
    if (ev < 0 || ev >= 16 || !g_lacos[ev].usado) { errno = EBADF; return -1; }
    WinLaco *l = &g_lacos[ev];
    int i = laco_acha(l, fd);
    if (i < 0) {
        if (l->n == l->cap) {
            int nc = l->cap ? l->cap * 2 : 64;
            int *nf = realloc(l->fd, sizeof(int) * (size_t)nc);
            if (!nf) return -1;
            l->fd = nf;
            void **np = realloc(l->ptr, sizeof(void *) * (size_t)nc);
            if (!np) return -1;
            l->ptr = np;
            int *na = realloc(l->armado, sizeof(int) * (size_t)nc);
            if (!na) return -1;
            l->armado = na;
            l->cap = nc;
        }
        i = l->n++;
    }
    l->fd[i] = fd;
    l->ptr[i] = ptr;
    l->armado[i] = ler ? 1 : 0;
    return 0;
}

int ps_ev_arma(int ev, int fd, void *ptr, int ler)
{
    if (ev < 0 || ev >= 16 || !g_lacos[ev].usado) { errno = EBADF; return -1; }
    WinLaco *l = &g_lacos[ev];
    int i = laco_acha(l, fd);
    if (i < 0) { errno = ENOENT; return -1; }
    l->ptr[i] = ptr;
    l->armado[i] = ler ? 1 : 0;
    return 0;
}

static void laco_remove(WinLaco *l, int i)
{
    l->n--;
    l->fd[i] = l->fd[l->n];
    l->ptr[i] = l->ptr[l->n];
    l->armado[i] = l->armado[l->n];
}

int ps_ev_tira(int ev, int fd)
{
    if (ev < 0 || ev >= 16 || !g_lacos[ev].usado) { errno = EBADF; return -1; }
    WinLaco *l = &g_lacos[ev];
    int i = laco_acha(l, fd);
    if (i < 0) { errno = ENOENT; return -1; }
    laco_remove(l, i);
    return 0;
}

int ps_ev_espera(int ev, PsEvento *out, int max, int timeout_ms)
{
    if (ev < 0 || ev >= 16 || !g_lacos[ev].usado) { errno = EBADF; return -1; }
    WinLaco *l = &g_lacos[ev];
    WSAPOLLFD pf[256];
    int idx[256];
    int np = 0;
    for (int i = 0; i < l->n && np < 256; i++) {
        if (!l->armado[i]) continue;
        pf[np].fd = (SOCKET)l->fd[i];
        pf[np].events = POLLRDNORM;
        pf[np].revents = 0;
        idx[np] = i;
        np++;
    }
    if (np == 0) {                       /* nada armado: só o tempo */
        Sleep(timeout_ms < 0 ? INFINITE : (DWORD)timeout_ms);
        return 0;
    }
    int r = WSAPoll(pf, (ULONG)np, timeout_ms);
    if (r < 0) { ps_sock_errno(); return -1; }
    int n = 0;
    int mortos[256], nm = 0;
    for (int k = 0; k < np && n < max; k++) {
        short rv = pf[k].revents;
        if (!rv) continue;
        if (rv & POLLNVAL) { mortos[nm++] = l->fd[idx[k]]; continue; }
        out[n].ptr = l->ptr[idx[k]];
        out[n].leu = (rv & (POLLRDNORM | POLLHUP)) != 0;
        out[n].hup = (rv & (POLLHUP | POLLERR)) != 0;
        n++;
    }
    for (int m = 0; m < nm; m++) {
        int i = laco_acha(l, mortos[m]);
        if (i >= 0) laco_remove(l, i);
    }
    return n;
}

/* ── aviso: um par de sockets no loopback ──────────────────────────────────
 * O WSAPoll só espera socket, então o aviso da thread do pool é um par
 * conectado em 127.0.0.1: a thread escreve 1 byte, o laço vê o outro lado
 * pronto pra ler. */
int ps_aviso_cria(PsAviso *a)
{
    SOCKET esc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (esc == INVALID_SOCKET) { ps_sock_errno(); return -1; }
    struct sockaddr_in end;
    memset(&end, 0, sizeof(end));
    end.sin_family = AF_INET;
    end.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    end.sin_port = 0;
    int tam = sizeof(end);
    SOCKET cli = INVALID_SOCKET, srv = INVALID_SOCKET;
    if (bind(esc, (struct sockaddr *)&end, sizeof(end)) != 0
            || listen(esc, 1) != 0
            || getsockname(esc, (struct sockaddr *)&end, &tam) != 0)
        goto falha;
    cli = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (cli == INVALID_SOCKET || connect(cli, (struct sockaddr *)&end, sizeof(end)) != 0) goto falha;
    srv = accept(esc, NULL, NULL);
    if (srv == INVALID_SOCKET) goto falha;
    closesocket(esc);
    int um = 1;
    setsockopt(cli, IPPROTO_TCP, TCP_NODELAY, (const char *)&um, sizeof(um));
    a->le = (int)srv;
    a->escreve = (int)cli;
    return 0;
falha:
    ps_sock_errno();
    if (cli != INVALID_SOCKET) closesocket(cli);
    closesocket(esc);
    return -1;
}

void ps_aviso_sinaliza(PsAviso a)
{
    char um = 1;
    send((SOCKET)a.escreve, &um, 1, 0);
}

void ps_aviso_espera(PsAviso a)
{
    char drena;
    recv((SOCKET)a.le, &drena, 1, 0);
}

void ps_aviso_fecha(PsAviso a)
{
    if (a.le >= 0) closesocket((SOCKET)a.le);
    if (a.escreve >= 0) closesocket((SOCKET)a.escreve);
}

/* ── memória ───────────────────────────────────────────────────────────────
 * A pilha da fibra no Windows é do próprio sistema: `CreateFiberEx` reserva a
 * dela (ver o PS_CTX em jinga_vm.c). O que o motor guarda aqui é só o registro
 * de que o slot tem pilha — uma página, sem uso. */
void *ps_pilha_fibra_nova(size_t util)
{
    (void)util;
    return VirtualAlloc(NULL, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void ps_pilha_fibra_solta(void *p, size_t util)
{
    (void)util;
    if (p) VirtualFree(p, 0, MEM_RELEASE);
}

void *ps_mem_exec_nova(const void *codigo, size_t n, size_t *tam)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t pg = si.dwPageSize ? si.dwPageSize : 4096;
    size_t t = (n + pg - 1) & ~(pg - 1);
    void *mem = VirtualAlloc(NULL, t, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!mem) return NULL;
    memcpy(mem, codigo, n);
    DWORD antes;
    if (!VirtualProtect(mem, t, PAGE_EXECUTE_READ, &antes)) { VirtualFree(mem, 0, MEM_RELEASE); return NULL; }
    FlushInstructionCache(GetCurrentProcess(), mem, t);
    *tam = t;
    return mem;
}

void ps_mem_exec_solta(void *p, size_t tam)
{
    (void)tam;
    if (p) VirtualFree(p, 0, MEM_RELEASE);
}

/* ── certificados: o repositório ROOT do Windows ───────────────────────── */

void ps_so_tls_cas(void *ctx)
{
    SSL_CTX *c = (SSL_CTX *)ctx;
    SSL_CTX_set_default_verify_paths(c);
    X509_STORE *st = SSL_CTX_get_cert_store(c);
    HCERTSTORE raiz = CertOpenSystemStoreA(0, "ROOT");
    if (!st || !raiz) return;
    PCCERT_CONTEXT cert = NULL;
    while ((cert = CertEnumCertificatesInStore(raiz, cert)) != NULL) {
        const unsigned char *p = cert->pbCertEncoded;
        X509 *x = d2i_X509(NULL, &p, (long)cert->cbCertEncoded);
        if (x) { X509_STORE_add_cert(st, x); X509_free(x); }
    }
    CertCloseStore(raiz, 0);
    ERR_clear_error();
}

/* ── resolução de nomes: as comuns (thread-local no Winsock) ───────────── */

struct hostent *ps_host_por_nome(const char *host, struct hostent *he,
                                 char *buf, size_t cap, int *herr)
{
    (void)he; (void)buf; (void)cap;
    *herr = 0;
    struct hostent *r = gethostbyname(host);
    if (!r) *herr = WSAGetLastError() == WSATRY_AGAIN ? TRY_AGAIN
                  : WSAGetLastError() == WSANO_RECOVERY ? NO_RECOVERY : HOST_NOT_FOUND;
    return r;
}

struct hostent *ps_host_por_ip4(const void *v4, struct hostent *he,
                                char *buf, size_t cap, int *herr)
{
    (void)he; (void)buf; (void)cap;
    *herr = 0;
    struct hostent *r = gethostbyaddr((const char *)v4, (int)sizeof(struct in_addr), AF_INET);
    if (!r) *herr = HOST_NOT_FOUND;
    return r;
}

/* ── caminhos ──────────────────────────────────────────────────────────── */

int ps_meu_executavel(char *buf, size_t cap)
{
    if (!buf || cap < 2) return -1;
    DWORD n = GetModuleFileNameA(NULL, buf, (DWORD)cap);
    if (n == 0 || n >= cap) return -1;
    return 0;
}

#else

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>

#if defined(PS_SO_LINUX)
#  include <sys/epoll.h>
#  include <sys/eventfd.h>
#else
#  include <sys/types.h>
#  include <sys/event.h>
#  include <sys/time.h>
#  include <mach-o/dyld.h>
#  include <Security/Security.h>
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

/* ── certificados TLS ──────────────────────────────────────────────────── */

void ps_so_tls_cas(void *ctx)
{
    SSL_CTX *c = (SSL_CTX *)ctx;
    SSL_CTX_set_default_verify_paths(c);
#if defined(PS_SO_MAC)
    /* as âncoras do Keychain do sistema, no formato DER, pro repositório do
     * OpenSSL (um que já esteja lá dá erro de "já existe": inofensivo) */
    X509_STORE *st = SSL_CTX_get_cert_store(c);
    CFArrayRef ancoras = NULL;
    if (st && SecTrustCopyAnchorCertificates(&ancoras) == errSecSuccess && ancoras) {
        CFIndex n = CFArrayGetCount(ancoras);
        for (CFIndex i = 0; i < n; i++) {
            SecCertificateRef cert = (SecCertificateRef)CFArrayGetValueAtIndex(ancoras, i);
            CFDataRef der = SecCertificateCopyData(cert);
            if (!der) continue;
            const unsigned char *p = CFDataGetBytePtr(der);
            X509 *x = d2i_X509(NULL, &p, (long)CFDataGetLength(der));
            if (x) { X509_STORE_add_cert(st, x); X509_free(x); }
            CFRelease(der);
        }
        CFRelease(ancoras);
        ERR_clear_error();
    }
#endif
}

/* ── resolução de nomes ────────────────────────────────────────────────── */

struct hostent *ps_host_por_nome(const char *host, struct hostent *he,
                                 char *buf, size_t cap, int *herr)
{
    *herr = 0;
#if defined(PS_SO_LINUX)
    struct hostent *r = NULL;
    if (gethostbyname_r(host, he, buf, cap, &r, herr) != 0) return NULL;
    return r;
#else
    (void)he; (void)buf; (void)cap;
    struct hostent *r = gethostbyname(host);
    if (!r) *herr = h_errno;
    return r;
#endif
}

struct hostent *ps_host_por_ip4(const void *v4, struct hostent *he,
                                char *buf, size_t cap, int *herr)
{
    *herr = 0;
#if defined(PS_SO_LINUX)
    struct hostent *r = NULL;
    if (gethostbyaddr_r(v4, sizeof(struct in_addr), AF_INET, he, buf, cap, &r, herr) != 0) return NULL;
    return r;
#else
    (void)he; (void)buf; (void)cap;
    struct hostent *r = gethostbyaddr(v4, (socklen_t)sizeof(struct in_addr), AF_INET);
    if (!r) *herr = h_errno;
    return r;
#endif
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
