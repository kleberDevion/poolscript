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

/* Memória zerada que só vira RAM quando é TOCADA (mmap anônimo /
 * VirtualAlloc). Pra bloco grande do qual se usa só o começo, como os vetores
 * de uma fibra: o `calloc` zera o bloco inteiro e ele fica residente na hora.
 * NULL = sem memória. */
void *ps_mem_zerada(size_t n);
void  ps_mem_zerada_solta(void *p, size_t n);

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
/* o erro do Winsock no `errno` de sempre (EAGAIN, EINTR, ECONNRESET...) —
 * a tradução é a do ps_win.h */
static inline int ps_sock_errno(void) { return ps_w_errno(); }
/* o começo do processo: Winsock, console em UTF-8 com as cores VT, saída
 * sem a troca de `\n` por `\r\n` (o mesmo byte que o programa escreveu, como
 * no Linux) e os recados do jinker pro processo filho (ps_so.c) */
void ps_w_inicia(void);
static inline void ps_so_inicia(void) { ps_w_inicia(); }
static inline int  ps_sock_fecha(int fd) { return closesocket((SOCKET)fd); }
/* cópia do socket no mesmo processo (o `dup` do CRT não conhece SOCKET) */
static inline int ps_sock_dup(int fd)
{
    WSAPROTOCOL_INFOW info;
    if (WSADuplicateSocketW((SOCKET)fd, GetCurrentProcessId(), &info) != 0) { ps_w_errno(); return -1; }
    SOCKET s = WSASocketW(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, &info, 0, 0);
    if (s == INVALID_SOCKET) { ps_w_errno(); return -1; }
    return (int)s;
}
static inline long ps_sock_le(int fd, void *b, size_t n) { return recv(fd, b, n, 0); }
static inline long ps_sock_escreve(int fd, const void *b, size_t n, int flags)
{
    return send(fd, b, n, flags);
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
        rc |= setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
    }
    if (ms_escreve >= 0) {
        DWORD t = (DWORD)ms_escreve;
        rc |= setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t));
    }
    return rc;
}
/* O `SO_REUSEADDR` do Windows NÃO é o do POSIX: lá ele deixa um segundo
 * servidor escutar na MESMA porta de um que está no ar (e roubar as conexões).
 * O que o POSIX pede com ele (reusar a porta logo depois de fechar) o Windows
 * já faz sem pedir; então aqui não se liga nada. */
static inline void ps_sock_reusa_endereco(int fd) { (void)fd; }
/* 1 = há o que ler (ou o outro lado fechou) em até `ms` */
static inline int ps_sock_espera_leitura(int fd, int ms)
{
    WSAPOLLFD p;
    p.fd = (SOCKET)fd; p.events = POLLRDNORM; p.revents = 0;
    return WSAPoll(&p, 1, ms) > 0 && (p.revents & (POLLRDNORM | POLLHUP | POLLERR));
}
/* par de sockets conectados: o Windows não tem socketpair; como em outras
 * linguagens no Windows, é um par TCP no loopback (ps_so.c) */
int ps_socketpair(int par[2]);

/* ── processos (Windows) ──────────────────────────────────────────────────
 * No Linux e no macOS o motor dispara processo com fork+exec direto (o código
 * de sempre, em jinga_vm.c). O Windows não tem fork: o processo nasce inteiro
 * do CreateProcess, com a linha de comando em texto e os canais escolhidos na
 * criação. Os canais do lado do motor são descritores do CRT, então `read`,
 * `write` e `close` seguem iguais.
 *
 * O código de saída de quem foi morto por `ps_w_proc_mata(p, sinal)` é
 * `-sinal`, como o `-15` do Linux depois de um `kill()`. */
typedef struct PsWPty PsWPty;
typedef struct {
    void   *h;      /* HANDLE do processo; NULL depois de solto */
    PsWPty *pty;    /* o pseudoconsole (pty=true), senão NULL */
    int     pid;
} PsWProc;

/* `cmd_sh` != NULL: roda pelo `cmd.exe /d /s /c`; senão `argv` (terminado em
 * NULL) vira a linha de comando com as aspas do Windows. `fd_in/out/err`:
 * descritores do CRT pros canais do filho, -1 = os do motor. `solto`: segundo
 * plano, num grupo de processos próprio (o Ctrl+C do terminal não o mata).
 * 0 = ok; -1 = não disparou, com o motivo no `errno` (ENOENT...). */
int  ps_w_proc_cria(const char *cmd_sh, char *const *argv,
                    int fd_in, int fd_out, int fd_err, int solto, PsWProc *p);
/* num pseudoconsole (ConPTY) de `cols`x`rows`: `*fd_escreve` é a entrada do
 * terminal, `*fd_le` a saída (que chega ao fim quando o filho termina) */
int  ps_w_proc_cria_pty(char *const *argv, int cols, int rows,
                        int *fd_escreve, int *fd_le, PsWProc *p);
int  ps_w_proc_espera(PsWProc *p, int *codigo);      /* bloqueia; 0 = ok */
int  ps_w_proc_terminou(PsWProc *p);                 /* 1 = já saiu */
int  ps_w_proc_mata(PsWProc *p, int sinal);          /* 0 = ok */
int  ps_w_proc_redimensiona(PsWProc *p, int cols, int rows);
/* fecha o pseudoconsole: o filho perde o terminal, como no fechar do mestre
 * do pty no Linux */
void ps_w_proc_fecha_terminal(PsWProc *p);
/* fecha os handles; sem pty não mata (o processo segue), com pty fecha o
 * terminal junto */
void ps_w_proc_solta(PsWProc *p);
/* cano anônimo com as duas pontas do CRT, binário e não herdável */
int  ps_w_cano(int fds[2]);

/* ── jinker sem fork (Windows) ────────────────────────────────────────────
 * Workers: cada worker é o MESMO programa rodado de novo (a mesma linha de
 * comando); ao chegar no `run()`, em vez de abrir a porta ele recebe do pai o
 * socket de escuta já aberto (WSADuplicateSocket). */
#include <signal.h>
int  ps_w_jk_worker(int fd_escuta, int indice, PsWProc *p);
/* no worker: o socket recebido e o índice dele. -1 = este processo não é
 * worker; -2 = é worker e o socket não chegou (errno). */
int  ps_w_jk_socket_herdado(int *indice);
/* O pai: espera até todos saírem ou `*parar` ligar (o Ctrl+C, que chega a
 * todos do mesmo console); quem não sair em 5 s é encerrado (código -15, o
 * SIGTERM do Linux). Solta os handles. */
void ps_w_jk_espera_workers(PsWProc *kids, int nk, volatile sig_atomic_t *parar);
/* Reload: sem `execv`, o processo que o terminal espera vira supervisor e
 * roda o programa de novo a cada reload pedido pelo filho. `pede_recarga`
 * (no filho): 1 = pediu, e o filho sai; 0 = este não é filho do supervisor.
 * `supervisiona`: devolve o código de saída do último filho, -1 = não
 * conseguiu rodar. */
int  ps_w_jk_pede_recarga(void);
int  ps_w_jk_supervisiona(void);
#else
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
static inline int  ps_sock_errno(void) { return errno; }
static inline void ps_so_inicia(void) { }
static inline int  ps_sock_fecha(int fd) { return close(fd); }
static inline int  ps_sock_dup(int fd) { return dup(fd); }
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
/* reusar a porta logo depois de fechar (sem esperar o TIME_WAIT) */
static inline void ps_sock_reusa_endereco(int fd)
{
    int um = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &um, sizeof(um));
}
#include <poll.h>
static inline int ps_sock_espera_leitura(int fd, int ms)
{
    struct pollfd p = { fd, POLLIN, 0 };
    return poll(&p, 1, ms) > 0 && (p.revents & (POLLIN | POLLHUP));
}
#endif

/* ── terminal ──────────────────────────────────────────────────────────────
 * 1 = o descritor `fd` é um terminal de verdade, onde o modo cru
 * (`sys.stdin.raw`) e a leitura tecla a tecla fazem sentido. No Linux e no
 * macOS é o `isatty`. No Windows o `_isatty` do CRT responde 1 pra QUALQUER
 * dispositivo de caractere, o NUL inclusive (é nele que fica o stdin de quem
 * roda sem terminal: serviço, CI, `< NUL`), e também pra descritor sem handle
 * padrão. Lá a pergunta vai ao console: só um handle de console (o do
 * pseudoconsole do `pty=true` também) responde ao `GetConsoleMode`. */
#if defined(PS_SO_WINDOWS)
static inline int ps_e_terminal(int fd)
{
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    DWORD modo;
    return h != NULL && h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &modo) != 0;
}
#else
static inline int ps_e_terminal(int fd) { return isatty(fd); }
#endif

/* ── links ────────────────────────────────────────────────────────────────
 * 1 = `caminho` é um LINK (simbólico; no Windows também a junção de pasta),
 * olhando o próprio link e não o destino. Quem apaga uma árvore remove o link
 * e NÃO desce nele: descer apagaria o conteúdo do destino. `*e_pasta` = o
 * link é de pasta (no Windows ele sai com rmdir, não com unlink). */
int ps_e_link(const char *caminho, int *e_pasta);

/* A variável da pasta do usuário: HOME no Linux e no macOS, USERPROFILE no
 * Windows (o HOME lá só existe dentro do MSYS2, e aponta pra outra pasta).
 * O motor (as libs do `import`) e o `jpkg` (onde elas são instaladas) leem a
 * MESMA. */
#if defined(PS_SO_WINDOWS)
#  define PS_HOME_VAR "USERPROFILE"
#else
#  define PS_HOME_VAR "HOME"
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
