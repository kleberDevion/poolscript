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
#define _WIN32_WINNT 0x0A00          /* Windows 10: WSAPoll, AF_UNIX, ConPTY */
#endif
#ifndef _POSIX_THREAD_SAFE_FUNCTIONS
#define _POSIX_THREAD_SAFE_FUNCTIONS  /* localtime_r / gmtime_r da mingw-w64 */
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
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

#ifndef S_ISLNK
#define S_ISLNK(m) 0                  /* o `stat` do CRT não vê link simbólico */
#endif
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

/* ── texto ────────────────────────────────────────────────────────────── */

static inline char *strndup(const char *s, size_t n)
{
    size_t l = 0;
    while (l < n && s[l]) l++;
    char *r = malloc(l + 1);
    if (!r) return NULL;
    memcpy(r, s, l);
    r[l] = '\0';
    return r;
}

static inline void *memmem(const void *h, size_t hl, const void *n, size_t nl)
{
    if (nl == 0) return (void *)h;
    if (nl > hl) return NULL;
    const unsigned char *a = h, *b = n;
    for (size_t i = 0; i + nl <= hl; i++)
        if (a[i] == b[0] && memcmp(a + i, b, nl) == 0) return (void *)(a + i);
    return NULL;
}

static inline char *strcasestr(const char *h, const char *n)
{
    size_t nl = strlen(n);
    if (nl == 0) return (char *)h;
    for (; *h; h++)
        if (_strnicmp(h, n, nl) == 0) return (char *)h;
    return NULL;
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
