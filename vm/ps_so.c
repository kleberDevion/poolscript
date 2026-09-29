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
    send(a.escreve, &um, 1, 0);
}

void ps_aviso_espera(PsAviso a)
{
    char drena;
    recv(a.le, &drena, 1, 0);
}

int ps_socketpair(int par[2])
{
    PsAviso a;
    if (ps_aviso_cria(&a) != 0) return -1;
    par[0] = a.le;
    par[1] = a.escreve;
    return 0;
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

void *ps_mem_zerada(size_t n)
{
    return VirtualAlloc(NULL, n, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void ps_mem_zerada_solta(void *p, size_t n)
{
    (void)n;
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

/* ── processos ──────────────────────────────────────────────────────────── */

/* o GetLastError do CreateProcess no errno de sempre */
static void w_erro_proc(DWORD e)
{
    switch (e) {
        case ERROR_FILE_NOT_FOUND: case ERROR_PATH_NOT_FOUND:
        case ERROR_INVALID_NAME:   case ERROR_DIRECTORY:           errno = ENOENT; break;
        case ERROR_ACCESS_DENIED:                                   errno = EACCES; break;
        case ERROR_BAD_EXE_FORMAT: case ERROR_EXE_MACHINE_TYPE_MISMATCH: errno = ENOEXEC; break;
        case ERROR_NOT_ENOUGH_MEMORY: case ERROR_OUTOFMEMORY:       errno = ENOMEM; break;
        case ERROR_FILENAME_EXCED_RANGE:                            errno = ENAMETOOLONG; break;
        default:                                                    errno = EINVAL; break;
    }
}

/* texto crescente pra montar a linha de comando */
typedef struct { char *b; size_t n, cap; } WLinha;

static int wl_poe(WLinha *l, const char *s, size_t n)
{
    if (l->n + n + 1 > l->cap) {
        size_t nc = l->cap ? l->cap : 256;
        while (nc < l->n + n + 1) nc *= 2;
        char *nb = realloc(l->b, nc);
        if (!nb) return -1;
        l->b = nb; l->cap = nc;
    }
    memcpy(l->b + l->n, s, n);
    l->n += n;
    l->b[l->n] = '\0';
    return 0;
}

static int wl_texto(WLinha *l, const char *s) { return wl_poe(l, s, strlen(s)); }

static int wl_barras(WLinha *l, size_t n)
{
    for (size_t i = 0; i < n; i++) if (wl_poe(l, "\\", 1) != 0) return -1;
    return 0;
}

/* Um argumento com as aspas que o programa do outro lado desfaz. O Windows
 * entrega ao filho UMA linha de texto, e é o CRT dele que a parte em argv: a
 * `\` só é especial antes de `"`, então as barras antes de uma aspa (e antes
 * da aspa que fecha) vão dobradas. Sem espaço nem aspa, vai cru. */
static int wl_argumento(WLinha *l, const char *a)
{
    if (*a && !strpbrk(a, " \t\n\v\"")) return wl_texto(l, a);
    if (wl_texto(l, "\"") != 0) return -1;
    for (const char *c = a; ; c++) {
        size_t barras = 0;
        while (*c == '\\') { barras++; c++; }
        if (!*c) {
            if (wl_barras(l, barras * 2) != 0) return -1;
            break;
        }
        if (*c == '"') {
            if (wl_barras(l, barras * 2 + 1) != 0 || wl_texto(l, "\"") != 0) return -1;
        } else {
            if (wl_barras(l, barras) != 0 || wl_poe(l, c, 1) != 0) return -1;
        }
    }
    return wl_texto(l, "\"");
}

/* UTF-8 -> UTF-16, o que o CreateProcessW recebe. NULL = sem memória. */
static wchar_t *w_largo(const char *s)
{
    int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
    if (n <= 0) return NULL;
    wchar_t *w = malloc(sizeof(wchar_t) * (size_t)n);
    if (w && MultiByteToWideChar(CP_UTF8, 0, s, -1, w, n) <= 0) { free(w); return NULL; }
    return w;
}

/* A linha inteira: `cmd.exe /d /s /c "<texto>"` (o `/s` tira só as aspas de
 * fora e o texto chega ao cmd como foi escrito) ou os argumentos com aspas.
 * `*app` recebe o caminho do cmd.exe (o ComSpec) quando é pelo shell. */
static wchar_t *w_linha(const char *cmd_sh, char *const *argv, wchar_t **app)
{
    WLinha l = {0};
    int ok;
    *app = NULL;
    if (cmd_sh) {
        const char *cs = getenv("ComSpec");
        if (!cs || !*cs) cs = "C:\\Windows\\System32\\cmd.exe";
        *app = w_largo(cs);
        ok = *app && wl_argumento(&l, cs) == 0 && wl_texto(&l, " /d /s /c \"") == 0
             && wl_texto(&l, cmd_sh) == 0 && wl_texto(&l, "\"") == 0;
    } else {
        ok = 1;
        for (int i = 0; argv[i] && ok; i++)
            ok = (i == 0 || wl_texto(&l, " ") == 0) && wl_argumento(&l, argv[i]) == 0;
    }
    wchar_t *w = ok ? w_largo(l.b ? l.b : "") : NULL;
    free(l.b);
    if (!w) { free(*app); *app = NULL; errno = ENOMEM; }
    return w;
}

/* Uma cópia HERDÁVEL de `fd` (com -1, do canal padrão `qual` do motor) pro
 * filho. NULL quando o motor não tem aquele canal (rodando sem console). */
static HANDLE w_herdavel(int fd, DWORD qual)
{
    HANDLE h = fd >= 0 ? (HANDLE)_get_osfhandle(fd) : GetStdHandle(qual);
    if (h == NULL || h == INVALID_HANDLE_VALUE) return NULL;
    HANDLE d = NULL;
    if (!DuplicateHandle(GetCurrentProcess(), h, GetCurrentProcess(), &d, 0, TRUE, DUPLICATE_SAME_ACCESS))
        return NULL;
    return d;
}

/* A lista de atributos do CreateProcess com UM atributo. NULL = sem memória. */
static LPPROC_THREAD_ATTRIBUTE_LIST w_atributos(void)
{
    SIZE_T tam = 0;
    InitializeProcThreadAttributeList(NULL, 1, 0, &tam);
    LPPROC_THREAD_ATTRIBUTE_LIST at = malloc(tam);
    if (at && !InitializeProcThreadAttributeList(at, 1, 0, &tam)) { free(at); at = NULL; }
    if (!at) errno = ENOMEM;
    return at;
}

static void w_atributos_solta(LPPROC_THREAD_ATTRIBUTE_LIST at)
{
    if (at) { DeleteProcThreadAttributeList(at); free(at); }
}

int ps_w_proc_cria(const char *cmd_sh, char *const *argv,
                   int fd_in, int fd_out, int fd_err, int solto, PsWProc *p)
{
    memset(p, 0, sizeof(*p));
    wchar_t *app = NULL;
    wchar_t *linha = w_linha(cmd_sh, argv, &app);
    if (!linha) return -1;
    HANDLE hs[3] = { w_herdavel(fd_in,  STD_INPUT_HANDLE),
                     w_herdavel(fd_out, STD_OUTPUT_HANDLE),
                     w_herdavel(fd_err, STD_ERROR_HANDLE) };
    HANDLE lista[3];
    DWORD nl = 0;
    for (int i = 0; i < 3; i++) if (hs[i]) lista[nl++] = hs[i];
    STARTUPINFOEXW si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    int rc = -1;
    LPPROC_THREAD_ATTRIBUTE_LIST at = w_atributos();
    if (!at) goto fim;
    /* Só estes três atravessam. Sem a lista, TODO handle herdável do motor ia
     * junto (o socket de escuta do jinker, as conexões de banco): é o
     * `closefrom` do Linux. */
    if (nl && !UpdateProcThreadAttribute(at, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                         lista, sizeof(HANDLE) * nl, NULL, NULL)) {
        w_erro_proc(GetLastError());
        goto fim;
    }
    si.StartupInfo.cb = sizeof(si);
    si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    si.StartupInfo.hStdInput  = hs[0];
    si.StartupInfo.hStdOutput = hs[1];
    si.StartupInfo.hStdError  = hs[2];
    si.lpAttributeList = at;
    /* segundo plano: grupo próprio, o Ctrl+C do terminal de quem disparou não
     * chega nele (o `setsid` do Linux) */
    DWORD flags = EXTENDED_STARTUPINFO_PRESENT | (solto ? CREATE_NEW_PROCESS_GROUP : 0);
    if (!CreateProcessW(app, linha, NULL, NULL, nl ? TRUE : FALSE, flags,
                        NULL, NULL, &si.StartupInfo, &pi)) {
        w_erro_proc(GetLastError());
        goto fim;
    }
    CloseHandle(pi.hThread);
    p->h = pi.hProcess;
    p->pid = (int)pi.dwProcessId;
    rc = 0;
fim:
    w_atributos_solta(at);
    for (int i = 0; i < 3; i++) if (hs[i]) CloseHandle(hs[i]);
    free(linha);
    free(app);
    return rc;
}

/* O pseudoconsole é dividido entre o dono (o objeto do processo) e a thread
 * que vigia o fim do filho; quem sai por último o solta. */
struct PsWPty {
    HPCON   pc;
    SRWLOCK mx;
    int     fechado;
    LONG    refs;
};

/* Fecha o console uma vez só. A chamada fica FORA da trava: o
 * ClosePseudoConsole espera o último quadro ser lido, e quem lê pode precisar
 * da trava (um `resize`) enquanto isso. */
static void pty_fecha(PsWPty *t)
{
    AcquireSRWLockExclusive(&t->mx);
    HPCON pc = t->fechado ? NULL : t->pc;
    t->fechado = 1;
    ReleaseSRWLockExclusive(&t->mx);
    if (pc) ClosePseudoConsole(pc);
}

static void pty_solta(PsWPty *t)
{
    if (InterlockedDecrement(&t->refs) == 0) free(t);
}

typedef struct { PsWPty *t; HANDLE h; } PtyVigia;

/* O fim da saída de um pseudoconsole não vem com o fim do filho: o console
 * segura o cano aberto até o ClosePseudoConsole. Esta thread espera o filho
 * sair e fecha o console; o último quadro chega e o `read` vê o fim, como o
 * EIO do mestre do pty no Linux. */
static DWORD WINAPI pty_vigia(LPVOID v)
{
    PtyVigia *g = (PtyVigia *)v;
    WaitForSingleObject(g->h, INFINITE);
    CloseHandle(g->h);
    pty_fecha(g->t);
    pty_solta(g->t);
    free(g);
    return 0;
}

int ps_w_proc_cria_pty(char *const *argv, int cols, int rows,
                       int *fd_escreve, int *fd_le, PsWProc *p)
{
    memset(p, 0, sizeof(*p));
    *fd_escreve = *fd_le = -1;
    HANDLE ent_le = NULL, ent_esc = NULL, sai_le = NULL, sai_esc = NULL, vigiado = NULL;
    HPCON pc = NULL;
    PsWPty *t = NULL;
    PtyVigia *g = NULL;
    LPPROC_THREAD_ATTRIBUTE_LIST at = NULL;
    wchar_t *app = NULL;
    wchar_t *linha = NULL;
    STARTUPINFOEXW si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    int rc = -1;

    linha = w_linha(NULL, argv, &app);
    if (!linha) goto fim;
    t = calloc(1, sizeof(*t));
    g = calloc(1, sizeof(*g));
    if (!t || !g) { errno = ENOMEM; goto fim; }
    if (!CreatePipe(&ent_le, &ent_esc, NULL, 0) || !CreatePipe(&sai_le, &sai_esc, NULL, 0)) {
        w_erro_proc(GetLastError());
        goto fim;
    }
    COORD tam = { (SHORT)cols, (SHORT)rows };
    HRESULT hr = CreatePseudoConsole(tam, ent_le, sai_esc, 0, &pc);
    if (FAILED(hr)) { errno = hr == E_OUTOFMEMORY ? ENOMEM : EIO; pc = NULL; goto fim; }
    at = w_atributos();
    if (!at) goto fim;
    if (!UpdateProcThreadAttribute(at, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, pc, sizeof(pc), NULL, NULL)) {
        w_erro_proc(GetLastError());
        goto fim;
    }
    si.StartupInfo.cb = sizeof(si);
    /* canais NULOS de propósito: sem isto, um motor com a saída redirecionada
     * passaria os canais dele ao filho no lugar do terminal */
    si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    si.lpAttributeList = at;
    if (!CreateProcessW(app, linha, NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT,
                        NULL, NULL, &si.StartupInfo, &pi)) {
        w_erro_proc(GetLastError());
        goto fim;
    }
    CloseHandle(pi.hThread);
    if (!DuplicateHandle(GetCurrentProcess(), pi.hProcess, GetCurrentProcess(), &vigiado,
                         SYNCHRONIZE, FALSE, 0)) {
        w_erro_proc(GetLastError());
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        goto fim;
    }
    InitializeSRWLock(&t->mx);
    t->pc = pc;
    t->refs = 2;
    g->t = t;
    g->h = vigiado;
    HANDLE th = CreateThread(NULL, 64 * 1024, pty_vigia, g, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);
    if (!th) {
        errno = ENOMEM;
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(vigiado);
        goto fim;
    }
    CloseHandle(th);
    p->h = pi.hProcess;
    p->pty = t;
    p->pid = (int)pi.dwProcessId;
    pc = NULL; t = NULL; g = NULL;            /* agora são da thread e do dono */
    rc = 0;
fim:
    if (rc == 0) {
        *fd_escreve = _open_osfhandle((intptr_t)ent_esc, _O_BINARY | _O_WRONLY);
        *fd_le = _open_osfhandle((intptr_t)sai_le, _O_BINARY | _O_RDONLY);
        ent_esc = sai_le = NULL;
    }
    /* as pontas do console ficaram com ele (depois do CreateProcess) */
    if (ent_le) CloseHandle(ent_le);
    if (sai_esc) CloseHandle(sai_esc);
    if (ent_esc) CloseHandle(ent_esc);
    if (sai_le) CloseHandle(sai_le);
    if (pc) ClosePseudoConsole(pc);
    w_atributos_solta(at);
    free(t);
    free(g);
    free(linha);
    free(app);
    return rc;
}

int ps_w_proc_espera(PsWProc *p, int *codigo)
{
    if (!p->h) { errno = ECHILD; return -1; }
    if (WaitForSingleObject(p->h, INFINITE) != WAIT_OBJECT_0) { errno = EINVAL; return -1; }
    DWORD c = 0;
    if (!GetExitCodeProcess(p->h, &c)) { errno = EINVAL; return -1; }
    *codigo = (int)c;
    return 0;
}

int ps_w_proc_terminou(PsWProc *p)
{
    return !p->h || WaitForSingleObject(p->h, 0) == WAIT_OBJECT_0;
}

/* O Windows não tem sinal: todo `kill` termina o processo, com o código de
 * saída `-sinal` (o `wait()` devolve -15 depois de um `kill()`, como no
 * Linux). O sinal 0 só pergunta se ainda existe. Um processo que já saiu mas
 * ainda não foi esperado conta como existente, como o zumbi do Linux. */
int ps_w_proc_mata(PsWProc *p, int sinal)
{
    if (!p->h) { errno = ESRCH; return -1; }
    if (sinal < 0 || sinal > 64) { errno = EINVAL; return -1; }
    if (sinal == 0 || ps_w_proc_terminou(p)) return 0;
    if (!TerminateProcess(p->h, (UINT)(-sinal))) {
        if (ps_w_proc_terminou(p)) return 0;
        errno = EPERM;
        return -1;
    }
    return 0;
}

int ps_w_proc_redimensiona(PsWProc *p, int cols, int rows)
{
    if (!p->pty) { errno = ENOTTY; return -1; }
    PsWPty *t = p->pty;
    int rc = 0;
    AcquireSRWLockExclusive(&t->mx);
    /* depois que o filho saiu não há tela, e no Linux o `resize` também só
     * volta sem efeito */
    if (!t->fechado) {
        COORD tam = { (SHORT)cols, (SHORT)rows };
        if (FAILED(ResizePseudoConsole(t->pc, tam))) { errno = EINVAL; rc = -1; }
    }
    ReleaseSRWLockExclusive(&t->mx);
    return rc;
}

void ps_w_proc_fecha_terminal(PsWProc *p)
{
    if (p->pty) pty_fecha(p->pty);
}

void ps_w_proc_solta(PsWProc *p)
{
    if (p->pty) { pty_fecha(p->pty); pty_solta(p->pty); p->pty = NULL; }
    if (p->h) { CloseHandle(p->h); p->h = NULL; }
}

int ps_w_cano(int fds[2])
{
    return _pipe(fds, 65536, _O_BINARY | _O_NOINHERIT);
}

/* O link no Windows é um "ponto de nova análise" (reparse point): o link
 * simbólico e a junção de pasta. O atributo é do próprio link. */
int ps_e_link(const char *caminho, int *e_pasta)
{
    *e_pasta = 0;
    wchar_t *w = w_largo(caminho);
    if (!w) return 0;
    DWORD a = GetFileAttributesW(w);
    free(w);
    if (a == INVALID_FILE_ATTRIBUTES || !(a & FILE_ATTRIBUTE_REPARSE_POINT)) return 0;
    *e_pasta = (a & FILE_ATTRIBUTE_DIRECTORY) != 0;
    return 1;
}

/* ── o começo do processo ──────────────────────────────────────────────── */

/* Os recados do jinker pro processo filho chegam em variáveis de ambiente e
 * saem do ambiente logo na partida: um programa que o filho rodar não herda
 * um número de handle que no processo dele não é nada. */
static char g_jk_worker[64];
static char g_jk_recarga[32];

static void w_recado(const char *nome, char *dst, DWORD cap)
{
    DWORD n = GetEnvironmentVariableA(nome, dst, cap);
    if (n == 0 || n >= cap) { dst[0] = '\0'; if (n == 0) return; }
    _putenv_s(nome, "");
}

void ps_w_inicia(void)
{
    WSADATA w;
    WSAStartup(MAKEWORD(2, 2), &w);
    /* Arquivos sem tradução de fim de linha: `\n` é o byte que o programa
     * escreveu, como no Linux e no macOS. A saída também; o console com VT
     * desce a linha e volta à coluna 0 com o `\n` sozinho. A entrada do
     * teclado fica em modo texto: o Enter do console é `\r\n`. */
    _set_fmode(_O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    DWORD qual[2] = { STD_OUTPUT_HANDLE, STD_ERROR_HANDLE };
    for (int i = 0; i < 2; i++) {
        HANDLE h = GetStdHandle(qual[i]);
        DWORD modo;
        if (h && h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &modo))
            SetConsoleMode(h, modo | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    w_recado("JINGA_JK_WORKER", g_jk_worker, sizeof(g_jk_worker));
    w_recado("JINGA_JK_RECARGA", g_jk_recarga, sizeof(g_jk_recarga));
}

/* ── jinker sem fork ───────────────────────────────────────────────────── */

/* O programa de novo: o mesmo executável, a mesma linha de comando, os
 * canais padrão do motor e mais os handles de `extra` (já herdáveis). O
 * recado vai numa variável de ambiente que só o filho vê. */
static int w_relanca(HANDLE extra, const char *var, const char *valor, PsWProc *p)
{
    memset(p, 0, sizeof(*p));
    static wchar_t exe[32768];
    DWORD n = GetModuleFileNameW(NULL, exe, 32768);
    if (n == 0 || n >= 32768) { errno = ENOENT; return -1; }
    wchar_t *linha = _wcsdup(GetCommandLineW());
    if (!linha) { errno = ENOMEM; return -1; }
    HANDLE hs[3] = { w_herdavel(-1, STD_INPUT_HANDLE),
                     w_herdavel(-1, STD_OUTPUT_HANDLE),
                     w_herdavel(-1, STD_ERROR_HANDLE) };
    HANDLE lista[4];
    DWORD nl = 0;
    for (int i = 0; i < 3; i++) if (hs[i]) lista[nl++] = hs[i];
    lista[nl++] = extra;
    STARTUPINFOEXW si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    int rc = -1;
    LPPROC_THREAD_ATTRIBUTE_LIST at = w_atributos();
    if (!at) goto fim;
    if (!UpdateProcThreadAttribute(at, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                   lista, sizeof(HANDLE) * nl, NULL, NULL)) {
        w_erro_proc(GetLastError());
        goto fim;
    }
    si.StartupInfo.cb = sizeof(si);
    si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    si.StartupInfo.hStdInput  = hs[0];
    si.StartupInfo.hStdOutput = hs[1];
    si.StartupInfo.hStdError  = hs[2];
    si.lpAttributeList = at;
    SetEnvironmentVariableA(var, valor);
    BOOL ok = CreateProcessW(exe, linha, NULL, NULL, TRUE, EXTENDED_STARTUPINFO_PRESENT,
                             NULL, NULL, &si.StartupInfo, &pi);
    DWORD e = GetLastError();
    SetEnvironmentVariableA(var, NULL);
    if (!ok) { w_erro_proc(e); goto fim; }
    CloseHandle(pi.hThread);
    p->h = pi.hProcess;
    p->pid = (int)pi.dwProcessId;
    rc = 0;
fim:
    w_atributos_solta(at);
    for (int i = 0; i < 3; i++) if (hs[i]) CloseHandle(hs[i]);
    free(linha);
    return rc;
}

/* O WSADuplicateSocket precisa do pid do filho, que só existe depois do
 * CreateProcess: o socket vai por um cano que o filho lê ao chegar no `run`. */
int ps_w_jk_worker(int fd_escuta, int indice, PsWProc *p)
{
    HANDLE le = NULL, esc = NULL;
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    if (!CreatePipe(&le, &esc, &sa, 0)) { w_erro_proc(GetLastError()); return -1; }
    SetHandleInformation(esc, HANDLE_FLAG_INHERIT, 0);    /* só a leitura atravessa */
    char valor[64];
    snprintf(valor, sizeof(valor), "%d:%llu", indice, (unsigned long long)(uintptr_t)le);
    int rc = w_relanca(le, "JINGA_JK_WORKER", valor, p);
    CloseHandle(le);
    if (rc != 0) { CloseHandle(esc); return -1; }
    WSAPROTOCOL_INFOW info;
    DWORD escritos = 0;
    int ok = WSADuplicateSocketW((SOCKET)fd_escuta, (DWORD)p->pid, &info) == 0
             && WriteFile(esc, &info, sizeof(info), &escritos, NULL) && escritos == sizeof(info);
    CloseHandle(esc);
    if (!ok) {
        TerminateProcess(p->h, 1);
        ps_w_proc_solta(p);
        errno = EIO;
        return -1;
    }
    return 0;
}

int ps_w_jk_socket_herdado(int *indice)
{
    if (!g_jk_worker[0]) return -1;
    int w = -1;
    unsigned long long h = 0;
    int campos = sscanf(g_jk_worker, "%d:%llu", &w, &h);
    g_jk_worker[0] = '\0';                  /* um `run` só recebe */
    if (campos != 2 || w < 0) { errno = EINVAL; return -2; }
    HANDLE cano = (HANDLE)(uintptr_t)h;
    WSAPROTOCOL_INFOW info;
    DWORD tot = 0, lidos = 0;
    while (tot < sizeof(info)
           && ReadFile(cano, (char *)&info + tot, (DWORD)(sizeof(info) - tot), &lidos, NULL) && lidos > 0)
        tot += lidos;
    CloseHandle(cano);
    if (tot != sizeof(info)) { errno = EIO; return -2; }
    SOCKET s = WSASocketW(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, &info, 0, 0);
    if (s == INVALID_SOCKET) { ps_w_errno(); return -2; }
    ps_sock_nao_bloqueia((int)s, 1);        /* como o ps_jk_listen deixa */
    *indice = w;
    return (int)s;
}

void ps_w_jk_espera_workers(PsWProc *kids, int nk, volatile sig_atomic_t *parar)
{
    HANDLE hs[MAXIMUM_WAIT_OBJECTS];
    for (;;) {
        if (*parar) break;
        int vivos = 0;
        for (int i = 0; i < nk; i++)
            if (!ps_w_proc_terminou(&kids[i]) && vivos < MAXIMUM_WAIT_OBJECTS) hs[vivos++] = kids[i].h;
        if (vivos == 0) break;
        /* acorda quando um sai, e de tempos em tempos pra ver o Ctrl+C */
        WaitForMultipleObjects((DWORD)vivos, hs, FALSE, 250);
    }
    ULONGLONG limite = GetTickCount64() + 5000;
    for (int i = 0; i < nk; i++) {
        ULONGLONG agora = GetTickCount64();
        DWORD falta = agora < limite ? (DWORD)(limite - agora) : 0;
        if (kids[i].h && WaitForSingleObject(kids[i].h, falta) != WAIT_OBJECT_0) {
            ps_w_proc_mata(&kids[i], 15);
            WaitForSingleObject(kids[i].h, INFINITE);
        }
        ps_w_proc_solta(&kids[i]);
    }
}

int ps_w_jk_pede_recarga(void)
{
    if (!g_jk_recarga[0]) return 0;
    HANDLE ev = (HANDLE)(uintptr_t)strtoull(g_jk_recarga, NULL, 10);
    SetEvent(ev);
    return 1;
}

int ps_w_jk_supervisiona(void)
{
    /* o Ctrl+C chega também no filho, que para e sai; o supervisor só espera
     * e sai com o código dele */
    signal(SIGINT, SIG_IGN);
    signal(SIGBREAK, SIG_IGN);
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE ev = CreateEventW(&sa, FALSE, FALSE, NULL);
    if (!ev) { w_erro_proc(GetLastError()); return -1; }
    char valor[32];
    snprintf(valor, sizeof(valor), "%llu", (unsigned long long)(uintptr_t)ev);
    int primeira = 1;
    for (;;) {
        PsWProc p;
        if (w_relanca(ev, "JINGA_JK_RECARGA", valor, &p) != 0) {
            CloseHandle(ev);
            return primeira ? -1 : 1;
        }
        primeira = 0;
        int codigo = 1;
        ps_w_proc_espera(&p, &codigo);
        ps_w_proc_solta(&p);
        /* saiu sem pedir reload (Ctrl+C, erro, fim): o supervisor sai junto */
        if (WaitForSingleObject(ev, 0) != WAIT_OBJECT_0) { CloseHandle(ev); return codigo; }
    }
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
#include <sys/stat.h>
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

void *ps_mem_zerada(size_t n)
{
    void *p = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    return p == MAP_FAILED ? NULL : p;
}

/* ── links ─────────────────────────────────────────────────────────────── */

int ps_e_link(const char *caminho, int *e_pasta)
{
    struct stat st;
    *e_pasta = 0;                    /* no POSIX todo link sai com unlink */
    return lstat(caminho, &st) == 0 && S_ISLNK(st.st_mode);
}

void ps_mem_zerada_solta(void *p, size_t n)
{
    if (p) munmap(p, n);
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
