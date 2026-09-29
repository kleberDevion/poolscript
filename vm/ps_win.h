/* ps_win.h — o que o Windows chama por outro nome.
 *
 * Só entra no Windows (MinGW-w64, o gcc/clang do MSYS2). Os arquivos do motor
 * incluem ESTE cabeçalho no lugar dos POSIX (`sys/socket.h`, `netdb.h`,
 * `unistd.h`...) e escrevem o código uma vez só. O que é comportamento de
 * sistema de verdade (eventos, fibras, processos, memória) não está aqui: está
 * na camada de sistema (ps_so.h/.c). Aqui ficam só os nomes.
 *
 * O `winsock2.h` tem que vir ANTES do `windows.h` (senão o `windows.h` puxa o
 * winsock 1 e os dois brigam). */
#ifndef PS_WIN_H
#define PS_WIN_H
#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00          /* Windows 10: WSAPoll, AF_UNIX */
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x0A000006     /* Windows 10 1809: o pseudoconsole (ConPTY) */
#endif
#ifndef _POSIX_THREAD_SAFE_FUNCTIONS
#define _POSIX_THREAD_SAFE_FUNCTIONS  /* localtime_r / gmtime_r da mingw-w64 */
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <afunix.h>
#include <iphlpapi.h>
#include <windows.h>
#include <io.h>
#include <direct.h>
#include <process.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

/* ── arquivos e caminhos ──────────────────────────────────────────────── */

/* `mkdir(caminho, modo)`: no Windows não há modo */
static inline int ps_win_mkdir(const char *p, int modo) { (void)modo; return _mkdir(p); }
#define mkdir(p, m) ps_win_mkdir((p), (m))

/* `realpath`: `_fullpath` faz a mesma coisa (caminho absoluto, sem `..`) */
static inline char *realpath(const char *p, char *r)
{
    char *out = _fullpath(r, p, r ? MAX_PATH : 0);
    if (!out) return NULL;
    DWORD a = GetFileAttributesA(out);
    if (a == INVALID_FILE_ATTRIBUTES) {       /* o POSIX recusa caminho que não existe */
        if (!r) free(out);
        errno = ENOENT;
        return NULL;
    }
    for (char *c = out; *c; c++) if (*c == '\\') *c = '/';
    return out;
}

static inline int setenv(const char *n, const char *v, int sobrescreve)
{
    if (!sobrescreve && getenv(n)) return 0;
    return _putenv_s(n, v) == 0 ? 0 : -1;
}
static inline int unsetenv(const char *n) { return _putenv_s(n, "") == 0 ? 0 : -1; }

/* `rename` com a regra do POSIX: SUBSTITUI o destino que já existe. O do CRT
 * (MoveFileEx sem REPLACE_EXISTING) recusava, e o mesmo `os.rename` que no
 * Linux e no macOS troca o arquivo levantava erro aqui. Pasta não pode ser
 * substituída pelo MoveFileEx: com origem pasta vai sem a flag. Os nomes vão
 * em UTF-8 pelas funções -A (o manifesto põe a página de código em UTF-8).
 * Entre discos diferentes falha com EXDEV, como no POSIX, e quem move copia. */
static inline int ps_w_rename(const char *de, const char *para)
{
    DWORD a = GetFileAttributesA(de);
    DWORD fl = (a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY)) ? 0 : MOVEFILE_REPLACE_EXISTING;
    if (MoveFileExA(de, para, fl)) return 0;
    switch (GetLastError()) {
        case ERROR_FILE_NOT_FOUND: case ERROR_PATH_NOT_FOUND:
        case ERROR_INVALID_NAME:                       errno = ENOENT; break;
        case ERROR_ACCESS_DENIED: case ERROR_SHARING_VIOLATION:
        case ERROR_LOCK_VIOLATION:                     errno = EACCES; break;
        case ERROR_NOT_SAME_DEVICE:                    errno = EXDEV; break;
        case ERROR_ALREADY_EXISTS: case ERROR_FILE_EXISTS: errno = EEXIST; break;
        case ERROR_DIR_NOT_EMPTY:                      errno = ENOTEMPTY; break;
        case ERROR_DISK_FULL:                          errno = ENOSPC; break;
        default:                                       errno = EIO; break;
    }
    return -1;
}
#define rename(de, para) ps_w_rename((de), (para))

#ifndef S_ISLNK
#define S_ISLNK(m) 0                  /* o `stat` do CRT não vê link simbólico */
#endif
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

/* ── texto ────────────────────────────────────────────────────────────── */

/* Com nome próprio e o nome POSIX por macro: o UCRT novo (mingw-w64 12+) já
 * declara `strndup` no string.h, e uma versão `static` com o mesmo nome não
 * compila lá; num mingw antigo ela não existe. Assim vale nos dois. */
#define strndup    ps_w_strndup
#define memmem     ps_w_memmem
#define strcasestr ps_w_strcasestr
static inline char *ps_w_strndup(const char *s, size_t n)
{
    size_t l = 0;
    while (l < n && s[l]) l++;
    char *r = malloc(l + 1);
    if (!r) return NULL;
    memcpy(r, s, l);
    r[l] = '\0';
    return r;
}

static inline void *ps_w_memmem(const void *h, size_t hl, const void *n, size_t nl)
{
    if (nl == 0) return (void *)h;
    if (nl > hl) return NULL;
    const unsigned char *a = h, *b = n;
    for (size_t i = 0; i + nl <= hl; i++)
        if (a[i] == b[0] && memcmp(a + i, b, nl) == 0) return (void *)(a + i);
    return NULL;
}

static inline char *ps_w_strcasestr(const char *h, const char *n)
{
    size_t nl = strlen(n);
    if (nl == 0) return (char *)h;
    for (; *h; h++)
        if (_strnicmp(h, n, nl) == 0) return (char *)h;
    return NULL;
}

/* ── sockets com cara de POSIX ─────────────────────────────────────────────
 * As funções do Winsock deixam o erro em WSAGetLastError, e o motor lê o
 * `errno` (como no Linux e no macOS). Estas versões fazem a chamada do Winsock
 * e traduzem o erro pro `errno` — o código que as chama não muda. Um `connect`
 * não-bloqueante em andamento é WSAEWOULDBLOCK no Windows e EINPROGRESS no
 * POSIX: a tradução do `connect` diz EINPROGRESS. O `setsockopt`/`getsockopt`
 * do Winsock pedem `char *`; aqui aceitam o ponteiro de sempre. */
static inline int ps_w_errno(void)
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
        case WSAEISCONN:     errno = EISCONN; break;
        case WSAEADDRINUSE:  errno = EADDRINUSE; break;
        case WSAEADDRNOTAVAIL: errno = EADDRNOTAVAIL; break;
        case WSAENETUNREACH: errno = ENETUNREACH; break;
        case WSAEHOSTUNREACH: errno = EHOSTUNREACH; break;
        case WSAEACCES:      errno = EACCES; break;
        case WSAEINVAL:      errno = EINVAL; break;
        case WSAEMSGSIZE:    errno = EMSGSIZE; break;
        case WSAESHUTDOWN:   errno = EPIPE; break;
        case WSAENOTSOCK:    errno = EBADF; break;
        case WSAEAFNOSUPPORT: errno = EAFNOSUPPORT; break;
        case WSAEPROTONOSUPPORT: errno = EPROTONOSUPPORT; break;
        case WSAENOPROTOOPT: errno = ENOPROTOOPT; break;
        case WSAEOPNOTSUPP:  errno = EOPNOTSUPP; break;
        default:             errno = EIO; break;
    }
    return errno;
}
static inline int ps_w_socket(int f, int t, int p)
{
    SOCKET s = socket(f, t, p);
    if (s == INVALID_SOCKET) { ps_w_errno(); return -1; }
    return (int)s;
}
static inline int ps_w_connect(int fd, const struct sockaddr *a, int l)
{
    int r = connect((SOCKET)fd, a, l);
    if (r != 0) { if (WSAGetLastError() == WSAEWOULDBLOCK) errno = EINPROGRESS; else ps_w_errno(); }
    return r;
}
static inline int ps_w_accept(int fd, struct sockaddr *a, int *l)
{
    SOCKET s = accept((SOCKET)fd, a, l);
    if (s == INVALID_SOCKET) { ps_w_errno(); return -1; }
    return (int)s;
}
static inline int ps_w_bind(int fd, const struct sockaddr *a, int l)
{ int r = bind((SOCKET)fd, a, l); if (r != 0) ps_w_errno(); return r; }
static inline int ps_w_listen(int fd, int n)
{ int r = listen((SOCKET)fd, n); if (r != 0) ps_w_errno(); return r; }
static inline long ps_w_recv(int fd, void *b, size_t n, int fl)
{ int r = recv((SOCKET)fd, (char *)b, (int)n, fl); if (r < 0) ps_w_errno(); return r; }
static inline long ps_w_send(int fd, const void *b, size_t n, int fl)
{ int r = send((SOCKET)fd, (const char *)b, (int)n, fl); if (r < 0) ps_w_errno(); return r; }
static inline long ps_w_recvfrom(int fd, void *b, size_t n, int fl, struct sockaddr *a, int *l)
{ int r = recvfrom((SOCKET)fd, (char *)b, (int)n, fl, a, l); if (r < 0) ps_w_errno(); return r; }
static inline long ps_w_sendto(int fd, const void *b, size_t n, int fl, const struct sockaddr *a, int l)
{ int r = sendto((SOCKET)fd, (const char *)b, (int)n, fl, a, l); if (r < 0) ps_w_errno(); return r; }
static inline int ps_w_setsockopt(int fd, int nv, int op, const void *v, int l)
{ int r = setsockopt((SOCKET)fd, nv, op, (const char *)v, l); if (r != 0) ps_w_errno(); return r; }
static inline int ps_w_getsockopt(int fd, int nv, int op, void *v, int *l)
{ int r = getsockopt((SOCKET)fd, nv, op, (char *)v, l); if (r != 0) ps_w_errno(); return r; }
static inline int ps_w_shutdown(int fd, int como)
{ int r = shutdown((SOCKET)fd, como); if (r != 0) ps_w_errno(); return r; }
static inline int ps_w_getsockname(int fd, struct sockaddr *a, int *l)
{ int r = getsockname((SOCKET)fd, a, l); if (r != 0) ps_w_errno(); return r; }
static inline int ps_w_getpeername(int fd, struct sockaddr *a, int *l)
{ int r = getpeername((SOCKET)fd, a, l); if (r != 0) ps_w_errno(); return r; }
static inline int ps_w_poll(WSAPOLLFD *f, ULONG n, int t)
{ int r = WSAPoll(f, n, t); if (r < 0) ps_w_errno(); return r; }

#define socket(f, t, p)            ps_w_socket((f), (t), (p))
#define connect(s, a, l)           ps_w_connect((int)(s), (a), (int)(l))
#define accept(s, a, l)            ps_w_accept((int)(s), (a), (l))
#define bind(s, a, l)              ps_w_bind((int)(s), (a), (int)(l))
#define listen(s, n)               ps_w_listen((int)(s), (n))
#define recv(s, b, n, f)           ps_w_recv((int)(s), (b), (n), (f))
#define send(s, b, n, f)           ps_w_send((int)(s), (b), (n), (f))
#define recvfrom(s, b, n, f, a, l) ps_w_recvfrom((int)(s), (b), (n), (f), (a), (l))
#define sendto(s, b, n, f, a, l)   ps_w_sendto((int)(s), (b), (n), (f), (a), (int)(l))
#define setsockopt(s, nv, o, v, l) ps_w_setsockopt((int)(s), (nv), (o), (v), (int)(l))
#define getsockopt(s, nv, o, v, l) ps_w_getsockopt((int)(s), (nv), (o), (v), (l))
#define shutdown(s, c)             ps_w_shutdown((int)(s), (c))
#define getsockname(s, a, l)       ps_w_getsockname((int)(s), (a), (l))
#define getpeername(s, a, l)       ps_w_getpeername((int)(s), (a), (l))
#define poll(f, n, t)              ps_w_poll((f), (ULONG)(n), (t))
typedef ULONG nfds_t;

#ifndef SHUT_RD
#define SHUT_RD   SD_RECEIVE
#define SHUT_WR   SD_SEND
#define SHUT_RDWR SD_BOTH
#endif

static inline int inet_aton(const char *cp, struct in_addr *inp)
{
    return inet_pton(AF_INET, cp, inp) == 1;
}

static inline const char *hstrerror(int e)
{
    switch (e) {
        case HOST_NOT_FOUND: return "Unknown host";
        case TRY_AGAIN:      return "Host name lookup failure";
        case NO_RECOVERY:    return "Unknown server error";
        case NO_DATA:        return "No address associated with name";
        default:             return "Unknown resolver error";
    }
}

/* if_nameindex: o Windows tem o índice e o nome da interface (iphlpapi), não
 * a lista pronta; a lista sai da tabela de interfaces */
struct if_nameindex { unsigned int if_index; char *if_name; };
static inline struct if_nameindex *if_nameindex(void)
{
    PMIB_IF_TABLE2 tab = NULL;
    if (GetIfTable2(&tab) != NO_ERROR || !tab) return NULL;
    struct if_nameindex *r = calloc((size_t)tab->NumEntries + 1, sizeof(*r));
    if (!r) { FreeMibTable(tab); return NULL; }
    ULONG k = 0;
    for (ULONG i = 0; i < tab->NumEntries; i++) {
        char nome[IF_NAMESIZE + 1];
        if (!if_indextoname(tab->Table[i].InterfaceIndex, nome)) continue;
        r[k].if_index = tab->Table[i].InterfaceIndex;
        r[k].if_name = _strdup(nome);
        k++;
    }
    FreeMibTable(tab);
    return r;
}
static inline void if_freenameindex(struct if_nameindex *p)
{
    if (!p) return;
    for (struct if_nameindex *q = p; q->if_name; q++) free(q->if_name);
    free(p);
}

/* ── tempo ────────────────────────────────────────────────────────────── */

/* as versões reentrantes do POSIX são as `_s` do CRT, com a ordem trocada */
#undef gmtime_r
#undef localtime_r
#define gmtime_r(t, r)    (gmtime_s((r), (t)) == 0 ? (r) : NULL)
#define localtime_r(t, r) (localtime_s((r), (t)) == 0 ? (r) : NULL)

/* ── sistema ──────────────────────────────────────────────────────────── */

#define _SC_PAGESIZE 30
#define _SC_OPEN_MAX 4
static inline long sysconf(int nome)
{
    if (nome == _SC_PAGESIZE) { SYSTEM_INFO si; GetSystemInfo(&si); return (long)si.dwPageSize; }
    if (nome == _SC_OPEN_MAX) return 2048;
    return -1;
}

#endif /* _WIN32 */
#endif
