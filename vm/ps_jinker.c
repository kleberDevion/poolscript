/*
 * Transporte do jinker — ver ps_jinker.h.
 *
 * Leitura bufferizada por conexão: keep-alive entrega N requisições no mesmo
 * fd, e um recv pode trazer o fim de uma e o começo da outra. O buffer
 * acumula e cada ps_jk_le_request consome exatamente uma requisição.
 */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <poll.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#include "ps_jinker.h"
#include "ps_hash.h"

/* corpo de requisição no máximo 64 MB — teto de sanidade, não de contrato */
#define JK_CORPO_MAX (64u * 1024u * 1024u)

struct PSJkConn {
    int   fd;
    SSL  *ssl;      /* NULL sem TLS */
    char *buf;      /* bytes já lidos e ainda não consumidos */
    size_t n, cap;
};

/* ── E/S básica ─────────────────────────────────────────────────────────── */

static long conn_le(PSJkConn *c, char *dst, size_t cap)
{
    if (c->ssl) {
        int r = SSL_read(c->ssl, dst, (int)cap);
        return r > 0 ? r : -1;
    }
    long r;
    do { r = recv(c->fd, dst, cap, 0); } while (r < 0 && errno == EINTR);
    return r > 0 ? r : -1;
}

static int conn_escreve(PSJkConn *c, const char *src, size_t n)
{
    size_t feito = 0;
    while (feito < n) {
        long r;
        if (c->ssl) {
            int w = SSL_write(c->ssl, src + feito, (int)(n - feito));
            r = w > 0 ? w : -1;
        } else {
            do { r = send(c->fd, src + feito, n - feito, MSG_NOSIGNAL); }
            while (r < 0 && errno == EINTR);
        }
        if (r <= 0) return -1;
        feito += (size_t)r;
    }
    return 0;
}

/* garante mais `extra` bytes no buffer da conexão lendo do socket */
static int conn_enche(PSJkConn *c)
{
    if (c->n + 8192 > c->cap) {
        size_t nc = c->cap ? c->cap * 2 : 16384;
        while (c->n + 8192 > nc) nc *= 2;
        char *nb = realloc(c->buf, nc);
        if (!nb) return -1;
        c->buf = nb; c->cap = nc;
    }
    long r = conn_le(c, c->buf + c->n, c->cap - c->n);
    if (r <= 0) return -1;
    c->n += (size_t)r;
    return 0;
}

static void conn_consome(PSJkConn *c, size_t n)
{
    if (n >= c->n) { c->n = 0; return; }
    memmove(c->buf, c->buf + n, c->n - n);
    c->n -= n;
}

/* ── escuta / aceita ────────────────────────────────────────────────────── */

int ps_jk_listen(const char *host, int porta, char *erro, size_t ecap)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { snprintf(erro, ecap, "socket: %s", strerror(errno)); return -1; }
    int um = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &um, sizeof(um));

    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons((unsigned short)porta);
    if (!host || !host[0] || strcmp(host, "0.0.0.0") == 0)
        a.sin_addr.s_addr = INADDR_ANY;
    else if (inet_pton(AF_INET, host, &a.sin_addr) != 1) {
        /* nome tipo "localhost" — resolve pro loopback, que é o uso real */
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }
    if (bind(fd, (struct sockaddr *)&a, sizeof(a)) != 0) {
        snprintf(erro, ecap, "bind %s:%d: %s", host ? host : "", porta, strerror(errno));
        close(fd); return -1;
    }
    if (listen(fd, 64) != 0) {
        snprintf(erro, ecap, "listen: %s", strerror(errno));
        close(fd); return -1;
    }
    /* fd de escuta NÃO-BLOQUEANTE: com multi-processo (prefork) vários workers
     * acordam no mesmo `poll` (thundering herd) e chamam accept(); um pega, os
     * outros recebem EAGAIN -> ps_jk_accept devolve NULL e o worker segue, em
     * vez de travar bloqueado no accept(). A conexão aceita continua bloqueante
     * (com o SO_RCVTIMEO dela), só o socket de escuta muda. */
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl != -1) fcntl(fd, F_SETFL, fl | O_NONBLOCK);
    return fd;
}

PSJkConn *ps_jk_accept(int fd_escuta, void *ssl_ctx, char *ip, size_t ipcap)
{
    struct sockaddr_in a; socklen_t al = sizeof(a);
    int fd;
    do { fd = accept(fd_escuta, (struct sockaddr *)&a, &al); }
    while (fd < 0 && errno == EINTR);
    if (fd < 0) return NULL;

    if (ip && ipcap) {
        ip[0] = '\0';
        inet_ntop(AF_INET, &a.sin_addr, ip, (socklen_t)ipcap);
    }
    /* Nagle desligado — o análogo do disable_nagle_algorithm do wrapper */
    int um = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &um, sizeof(um));
    /* timeout de leitura: keep-alive ocioso não segura o loop pra sempre */
    struct timeval tv = { 30, 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    PSJkConn *c = calloc(1, sizeof(PSJkConn));
    if (!c) { close(fd); return NULL; }
    c->fd = fd;

    if (ssl_ctx) {
        c->ssl = SSL_new((SSL_CTX *)ssl_ctx);
        if (!c->ssl || SSL_set_fd(c->ssl, fd) != 1 || SSL_accept(c->ssl) != 1) {
            if (c->ssl) SSL_free(c->ssl);
            close(fd); free(c);
            return NULL;
        }
    }
    return c;
}

int ps_jk_fd(const PSJkConn *c) { return c->fd; }

void ps_jk_close(PSJkConn *c)
{
    if (!c) return;
    if (c->ssl) { SSL_shutdown(c->ssl); SSL_free(c->ssl); }
    close(c->fd);
    free(c->buf);
    free(c);
}

/* Solta o buffer de leitura (16KB+) de uma conexão OCIOSA — keep-alive esperando
 * a próxima requisição. Sem nada bufferizado (n==0), o buffer não serve pra nada
 * enquanto a conexão dorme; realoca sozinho no próximo `conn_garante`. Isso faz
 * 100k conexões ociosas custarem ~200 bytes cada em vez de ~16KB. Se há bytes
 * pendentes (pipelining), NÃO solta. */
void ps_jk_conn_solta_buf(PSJkConn *c)
{
    if (c && c->n == 0 && c->buf) { free(c->buf); c->buf = NULL; c->cap = 0; }
}

/* ── parse da requisição ────────────────────────────────────────────────── */

const char *ps_jk_header(const PSJkReq *r, const char *nome)
{
    for (int i = 0; i < r->ncabs; i++)
        if (strcasecmp(r->cabs[i].nome, nome) == 0) return r->cabs[i].valor;
    return NULL;
}

static char *dup_faixa(const char *a, const char *b)
{
    size_t n = (size_t)(b - a);
    char *s = malloc(n + 1);
    if (!s) return NULL;
    memcpy(s, a, n); s[n] = '\0';
    return s;
}

int ps_jk_le_request(PSJkConn *c, PSJkReq *r)
{
    memset(r, 0, sizeof(*r));

    /* junta até ter o fim dos headers */
    const char *fim = NULL;
    for (;;) {
        if (c->n >= 4) {
            c->buf[c->n < c->cap ? c->n : c->cap - 1] = '\0';
            fim = memmem(c->buf, c->n, "\r\n\r\n", 4);
            if (fim) break;
        }
        if (c->n > 64 * 1024) return -1;   /* headers gigantes = lixo */
        if (conn_enche(c) != 0) return -1;
    }
    size_t nhead = (size_t)(fim - c->buf) + 4;

    /* linha de pedido: METODO alvo HTTP/1.x */
    char *p = c->buf;
    char *eol = memmem(p, nhead, "\r\n", 2);
    if (!eol) return -1;
    char *sp1 = memchr(p, ' ', (size_t)(eol - p));
    if (!sp1) return -1;
    size_t nm = (size_t)(sp1 - p);
    if (nm >= sizeof(r->metodo)) return -1;
    memcpy(r->metodo, p, nm); r->metodo[nm] = '\0';
    char *sp2 = memchr(sp1 + 1, ' ', (size_t)(eol - sp1 - 1));
    if (!sp2) return -1;
    char *alvo = dup_faixa(sp1 + 1, sp2);
    if (!alvo) return -1;

    /* separa query e decodifica o path */
    char *q = strchr(alvo, '?');
    if (q) { *q = '\0'; r->query = strdup(q + 1); }
    else     r->query = strdup("");
    ps_jk_urldecode(alvo);
    r->path = alvo;
    if (!r->query) { ps_jk_req_solta(r); return -1; }

    /* headers */
    int cap_c = 8;
    r->cabs = malloc(sizeof(PSJkHdr) * (size_t)cap_c);
    if (!r->cabs) { ps_jk_req_solta(r); return -1; }
    p = eol + 2;
    while (p < c->buf + nhead - 4) {
        char *e2 = memmem(p, nhead - (size_t)(p - c->buf), "\r\n", 2);
        if (!e2 || e2 == p) break;
        char *dois = memchr(p, ':', (size_t)(e2 - p));
        if (dois) {
            if (r->ncabs == cap_c) {
                cap_c *= 2;
                PSJkHdr *nh = realloc(r->cabs, sizeof(PSJkHdr) * (size_t)cap_c);
                if (!nh) { ps_jk_req_solta(r); return -1; }
                r->cabs = nh;
            }
            const char *v = dois + 1;
            while (v < e2 && (*v == ' ' || *v == '\t')) v++;
            r->cabs[r->ncabs].nome  = dup_faixa(p, dois);
            r->cabs[r->ncabs].valor = dup_faixa(v, e2);
            if (!r->cabs[r->ncabs].nome || !r->cabs[r->ncabs].valor) { ps_jk_req_solta(r); return -1; }
            r->ncabs++;
        }
        p = e2 + 2;
    }

    /* keep-alive: HTTP/1.1 salvo "Connection: close" */
    r->keep_alive = 1;
    const char *con = ps_jk_header(r, "Connection");
    if (con && strcasecmp(con, "close") == 0) r->keep_alive = 0;
    const char *upg = ps_jk_header(r, "Upgrade");
    if (upg && strcasecmp(upg, "websocket") == 0) r->eh_ws = 1;

    /* corpo por Content-Length */
    size_t ncorpo = 0;
    const char *cl = ps_jk_header(r, "Content-Length");
    if (cl) {
        long v = atol(cl);
        if (v < 0 || (unsigned long)v > JK_CORPO_MAX) { ps_jk_req_solta(r); return -1; }
        ncorpo = (size_t)v;
    }
    while (c->n < nhead + ncorpo)
        if (conn_enche(c) != 0) { ps_jk_req_solta(r); return -1; }

    if (ncorpo) {
        r->corpo = malloc(ncorpo + 1);
        if (!r->corpo) { ps_jk_req_solta(r); return -1; }
        memcpy(r->corpo, c->buf + nhead, ncorpo);
        r->corpo[ncorpo] = '\0';
        r->ncorpo = ncorpo;
    }
    conn_consome(c, nhead + ncorpo);
    return 0;
}

void ps_jk_req_solta(PSJkReq *r)
{
    free(r->path); free(r->query); free(r->corpo);
    for (int i = 0; i < r->ncabs; i++) { free(r->cabs[i].nome); free(r->cabs[i].valor); }
    free(r->cabs);
    memset(r, 0, sizeof(*r));
}

/* ── resposta ───────────────────────────────────────────────────────────── */

static const char *frase(int status)
{
    switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 429: return "Too Many Requests";
        case 500: return "Internal Server Error";
        default:  return "OK";
    }
}

int ps_jk_responde(PSJkConn *c, int status, const char *ctype,
                   const char *corpo, size_t ncorpo,
                   const char *extra, int keep_alive)
{
    /* Date no formato do email.utils.formatdate(usegmt=True) do wrapper */
    char data[64];
    time_t t = time(NULL);
    struct tm gm;
    gmtime_r(&t, &gm);
    strftime(data, sizeof(data), "%a, %d %b %Y %H:%M:%S GMT", &gm);

    char cab[4096];
    int n = snprintf(cab, sizeof(cab),
                     "HTTP/1.1 %d %s\r\n"
                     "Server: Jinker\r\n"
                     "Date: %s\r\n"
                     "%s%s%s"
                     "Content-Length: %zu\r\n"
                     "%s"
                     "%s"
                     "\r\n",
                     status, frase(status), data,
                     ctype ? "Content-Type: " : "", ctype ? ctype : "", ctype ? "\r\n" : "",
                     ncorpo,
                     keep_alive ? "" : "Connection: close\r\n",
                     extra ? extra : "");
    if (n < 0 || n >= (int)sizeof(cab)) return -1;
    if (conn_escreve(c, cab, (size_t)n) != 0) return -1;
    if (ncorpo && conn_escreve(c, corpo, ncorpo) != 0) return -1;
    return 0;
}

/* ── WebSocket ──────────────────────────────────────────────────────────── */

int ps_jk_ws_handshake(PSJkConn *c, const PSJkReq *r)
{
    const char *chave = ps_jk_header(r, "Sec-WebSocket-Key");
    if (!chave) return -1;

    /* accept = base64(SHA1(chave + GUID)) — RFC 6455 §4.2.2 */
    char cat[128];
    snprintf(cat, sizeof(cat), "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", chave);
    unsigned char dig[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char *)cat, strlen(cat), dig);
    char b64[64];
    size_t nb = ps_base64_encode(dig, sizeof(dig), b64);
    b64[nb] = '\0';

    char resp[256];
    int n = snprintf(resp, sizeof(resp),
                     "HTTP/1.1 101 Switching Protocols\r\n"
                     "Upgrade: websocket\r\n"
                     "Connection: Upgrade\r\n"
                     "Sec-WebSocket-Accept: %s\r\n\r\n", b64);
    return conn_escreve(c, resp, (size_t)n);
}

/* precisa de `n` bytes prontos no buffer da conexão */
static int ws_precisa(PSJkConn *c, size_t n)
{
    while (c->n < n)
        if (conn_enche(c) != 0) return -1;
    return 0;
}

static int ws_manda_frame(PSJkConn *c, int opcode, const char *dados, size_t n)
{
    unsigned char cab[10];
    size_t nc;
    cab[0] = (unsigned char)(0x80 | opcode);   /* FIN + opcode; servidor não mascara */
    if (n < 126) { cab[1] = (unsigned char)n; nc = 2; }
    else if (n < 65536) {
        cab[1] = 126;
        cab[2] = (unsigned char)(n >> 8); cab[3] = (unsigned char)n;
        nc = 4;
    } else {
        cab[1] = 127;
        for (int i = 0; i < 8; i++) cab[2 + i] = (unsigned char)(n >> (8 * (7 - i)));
        nc = 10;
    }
    if (conn_escreve(c, (const char *)cab, nc) != 0) return -1;
    return n ? conn_escreve(c, dados, n) : 0;
}

int ps_jk_ws_envia_texto(PSJkConn *c, const char *msg, size_t n)
{ return ws_manda_frame(c, 0x1, msg, n); }

int ps_jk_ws_envia_close(PSJkConn *c, int codigo, const char *motivo)
{
    char corpo[128];
    size_t nm = motivo ? strlen(motivo) : 0;
    if (nm > 120) nm = 120;
    corpo[0] = (char)(codigo >> 8);
    corpo[1] = (char)(codigo & 0xff);
    if (nm) memcpy(corpo + 2, motivo, nm);
    return ws_manda_frame(c, 0x8, corpo, nm + 2);
}

int ps_jk_ws_le_frame(PSJkConn *c, char **msg, size_t *n)
{
    /* Acumula fragmentos até o FIN — mensagem em texto pode vir fatiada. */
    char *acc = NULL; size_t nacc = 0;
    for (;;) {
        if (ws_precisa(c, 2) != 0) { free(acc); return -1; }
        unsigned char b0 = (unsigned char)c->buf[0];
        unsigned char b1 = (unsigned char)c->buf[1];
        int fin = b0 & 0x80, opcode = b0 & 0x0f;
        int mascarado = b1 & 0x80;
        size_t np = b1 & 0x7f, off = 2;
        if (np == 126) {
            if (ws_precisa(c, 4) != 0) { free(acc); return -1; }
            np = ((size_t)(unsigned char)c->buf[2] << 8) | (unsigned char)c->buf[3];
            off = 4;
        } else if (np == 127) {
            if (ws_precisa(c, 10) != 0) { free(acc); return -1; }
            np = 0;
            for (int i = 0; i < 8; i++) np = (np << 8) | (unsigned char)c->buf[2 + i];
            off = 10;
        }
        if (np > JK_CORPO_MAX) { free(acc); return -1; }
        unsigned char masc[4] = {0};
        if (mascarado) {
            if (ws_precisa(c, off + 4) != 0) { free(acc); return -1; }
            memcpy(masc, c->buf + off, 4);
            off += 4;
        }
        if (ws_precisa(c, off + np) != 0) { free(acc); return -1; }

        char *dados = malloc(np + 1);
        if (!dados) { free(acc); return -1; }
        for (size_t i = 0; i < np; i++)
            dados[i] = (char)(c->buf[off + i] ^ (mascarado ? masc[i % 4] : 0));
        dados[np] = '\0';
        conn_consome(c, off + np);

        if (opcode == 0x8) {                       /* close */
            free(dados); free(acc);
            ps_jk_ws_envia_close(c, 1000, "");
            return 1;
        }
        if (opcode == 0x9) {                       /* ping -> pong */
            ws_manda_frame(c, 0xA, dados, np);
            free(dados);
            continue;
        }
        if (opcode == 0xA) { free(dados); continue; }   /* pong: ignora */

        /* texto/binário/continuação */
        char *nn = realloc(acc, nacc + np + 1);
        if (!nn) { free(dados); free(acc); return -1; }
        acc = nn;
        memcpy(acc + nacc, dados, np);
        nacc += np;
        acc[nacc] = '\0';
        free(dados);
        if (fin) { *msg = acc; *n = nacc; return 0; }
    }
}

/* ── WebSocket cliente ──────────────────────────────────────────────────── */

PSJkConn *ps_jk_ws_conecta(const char *host, int porta, const char *path,
                           char *erro, size_t ecap)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { snprintf(erro, ecap, "socket: %s", strerror(errno)); return NULL; }
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons((unsigned short)porta);
    if (inet_pton(AF_INET, host && host[0] ? host : "127.0.0.1", &a.sin_addr) != 1)
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);   /* "localhost" e afins */
    struct timeval tv = { 10, 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    int um = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &um, sizeof(um));
    if (connect(fd, (struct sockaddr *)&a, sizeof(a)) != 0) {
        snprintf(erro, ecap, "falha de conexão: %s", strerror(errno));
        close(fd); return NULL;
    }
    PSJkConn *c = calloc(1, sizeof(PSJkConn));
    if (!c) { close(fd); snprintf(erro, ecap, "sem memoria"); return NULL; }
    c->fd = fd;

    /* chave aleatória de 16 bytes em base64 */
    unsigned char cru[16];
    if (ps_random_bytes(cru, sizeof(cru)) != 0)
        for (int i = 0; i < 16; i++) cru[i] = (unsigned char)(rand() & 0xff);
    char chave[32];
    size_t nk = ps_base64_encode(cru, sizeof(cru), chave);
    chave[nk] = '\0';

    char req[512];
    int n = snprintf(req, sizeof(req),
                     "GET %s HTTP/1.1\r\n"
                     "Host: %s:%d\r\n"
                     "Upgrade: websocket\r\n"
                     "Connection: Upgrade\r\n"
                     "Sec-WebSocket-Key: %s\r\n"
                     "Sec-WebSocket-Version: 13\r\n\r\n",
                     path && path[0] ? path : "/", host, porta, chave);
    if (conn_escreve(c, req, (size_t)n) != 0) {
        snprintf(erro, ecap, "falha ao enviar handshake");
        ps_jk_close(c); return NULL;
    }
    /* espera o 101 (fim dos headers) */
    for (;;) {
        if (c->n >= 4 && memmem(c->buf, c->n, "\r\n\r\n", 4)) break;
        if (c->n > 16384) { snprintf(erro, ecap, "resposta invalida"); ps_jk_close(c); return NULL; }
        if (conn_enche(c) != 0) { snprintf(erro, ecap, "conexao caiu no handshake"); ps_jk_close(c); return NULL; }
    }
    if (c->n < 12 || memcmp(c->buf, "HTTP/1.1 101", 12) != 0) {
        snprintf(erro, ecap, "servidor recusou o upgrade");
        ps_jk_close(c); return NULL;
    }
    /* valida o Sec-WebSocket-Accept — RFC 6455 §4.1 */
    char esp[128];
    snprintf(esp, sizeof(esp), "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", chave);
    unsigned char dig[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char *)esp, strlen(esp), dig);
    char b64[64];
    size_t nb = ps_base64_encode(dig, sizeof(dig), b64);
    b64[nb] = '\0';
    const char *fimh = memmem(c->buf, c->n, "\r\n\r\n", 4);
    size_t nhead = (size_t)(fimh - c->buf) + 4;
    char cab[16384];
    size_t ncab = nhead < sizeof(cab) - 1 ? nhead : sizeof(cab) - 1;
    memcpy(cab, c->buf, ncab); cab[ncab] = '\0';
    if (!strcasestr(cab, "Sec-WebSocket-Accept") || !strstr(cab, b64)) {
        snprintf(erro, ecap, "Sec-WebSocket-Accept invalido");
        ps_jk_close(c); return NULL;
    }
    conn_consome(c, nhead);
    return c;
}

int ps_jk_ws_envia_texto_cli(PSJkConn *c, const char *msg, size_t n)
{
    unsigned char cab[14];
    size_t nc;
    cab[0] = 0x81;   /* FIN + texto */
    unsigned char masc[4];
    if (ps_random_bytes(masc, 4) != 0)
        for (int i = 0; i < 4; i++) masc[i] = (unsigned char)(rand() & 0xff);
    if (n < 126) { cab[1] = (unsigned char)(0x80 | n); nc = 2; }
    else if (n < 65536) {
        cab[1] = 0x80 | 126;
        cab[2] = (unsigned char)(n >> 8); cab[3] = (unsigned char)n;
        nc = 4;
    } else {
        cab[1] = 0x80 | 127;
        for (int i = 0; i < 8; i++) cab[2 + i] = (unsigned char)(n >> (8 * (7 - i)));
        nc = 10;
    }
    memcpy(cab + nc, masc, 4); nc += 4;
    if (conn_escreve(c, (const char *)cab, nc) != 0) return -1;
    /* aplica a máscara num buffer próprio pra não alterar o do chamador */
    char pilha[1024];
    char *tmp = n <= sizeof(pilha) ? pilha : malloc(n);
    if (!tmp) return -1;
    for (size_t i = 0; i < n; i++) tmp[i] = (char)(msg[i] ^ masc[i % 4]);
    int rc = conn_escreve(c, tmp, n);
    if (tmp != pilha) free(tmp);
    return rc;
}

int ps_jk_ws_tem_dados(PSJkConn *c, int timeout_ms)
{
    if (c->n > 0) return 1;
    struct pollfd p = { c->fd, POLLIN, 0 };
    return poll(&p, 1, timeout_ms) > 0 && (p.revents & (POLLIN | POLLHUP));
}

/* ── multipart/form-data ────────────────────────────────────────────────── */

/* Procura, dentro do bloco de headers da parte, `chave="valor"` (aspas). */
static char *acha_atr(const char *cabs, const char *chave)
{
    const char *p = cabs;
    size_t nk = strlen(chave);
    while ((p = strcasestr(p, chave)) != NULL) {
        const char *v = p + nk;
        while (*v == ' ') v++;
        if (*v == '=') {
            v++;
            while (*v == ' ') v++;
            if (*v == '"') {
                const char *fim2 = strchr(v + 1, '"');
                if (fim2) return dup_faixa(v + 1, fim2);
            }
        }
        p += nk;
    }
    return NULL;
}

int ps_jk_multipart(const char *corpo, size_t n, const char *boundary,
                    PSJkParte **partes)
{
    char sep[256];
    int nsep = snprintf(sep, sizeof(sep), "--%s", boundary);
    if (nsep <= 2 || nsep >= (int)sizeof(sep)) return -1;

    *partes = NULL;
    int np = 0, cap = 4;
    PSJkParte *out = malloc(sizeof(PSJkParte) * (size_t)cap);
    if (!out) return -1;

    const char *p = corpo, *fim_tudo = corpo + n;
    /* pula até o primeiro boundary */
    const char *b = memmem(p, (size_t)(fim_tudo - p), sep, (size_t)nsep);
    while (b) {
        p = b + nsep;
        if (p + 2 <= fim_tudo && p[0] == '-' && p[1] == '-') break;   /* --sep-- final */
        if (p + 2 <= fim_tudo && p[0] == '\r' && p[1] == '\n') p += 2;

        const char *prox = memmem(p, (size_t)(fim_tudo - p), sep, (size_t)nsep);
        const char *fim_parte = prox ? prox : fim_tudo;
        /* o \r\n antes do boundary pertence ao separador, não aos dados */
        if (fim_parte - 2 >= p && fim_parte[-2] == '\r' && fim_parte[-1] == '\n')
            fim_parte -= 2;

        /* separa headers do corpo da parte */
        const char *hb = memmem(p, (size_t)(fim_parte - p), "\r\n\r\n", 4);
        size_t salto = 4;
        if (!hb) { hb = memmem(p, (size_t)(fim_parte - p), "\n\n", 2); salto = 2; }
        if (hb) {
            char *cabs = dup_faixa(p, hb);
            if (!cabs) { ps_jk_partes_solta(out, np); return -1; }
            char *campo = acha_atr(cabs, "name");
            if (campo) {
                if (np == cap) {
                    cap *= 2;
                    PSJkParte *no = realloc(out, sizeof(PSJkParte) * (size_t)cap);
                    if (!no) { free(cabs); free(campo); ps_jk_partes_solta(out, np); return -1; }
                    out = no;
                }
                const char *db = hb + salto;
                size_t nd = (size_t)(fim_parte - db);
                out[np].campo    = campo;
                out[np].filename = acha_atr(cabs, "filename");
                out[np].ctype    = NULL;
                const char *ct = strcasestr(cabs, "Content-Type:");
                if (ct) {
                    ct += 13;
                    while (*ct == ' ') ct++;
                    const char *ce = ct;
                    while (*ce && *ce != '\r' && *ce != '\n') ce++;
                    out[np].ctype = dup_faixa(ct, ce);
                }
                out[np].dados = malloc(nd + 1);
                if (!out[np].dados) { free(cabs); ps_jk_partes_solta(out, np); return -1; }
                memcpy(out[np].dados, db, nd);
                out[np].dados[nd] = '\0';
                out[np].ndados = nd;
                np++;
            }
            free(cabs);
        }
        b = prox;
    }
    *partes = out;
    return np;
}

void ps_jk_partes_solta(PSJkParte *p, int n)
{
    if (!p) return;
    for (int i = 0; i < n; i++) {
        free(p[i].campo); free(p[i].filename); free(p[i].ctype); free(p[i].dados);
    }
    free(p);
}

/* ── utilidades ─────────────────────────────────────────────────────────── */

const char *ps_jk_mime(const char *caminho)
{
    const char *ponto = strrchr(caminho, '.');
    if (!ponto) return "application/octet-stream";
    static const struct { const char *ext, *mime; } TAB[] = {
        { ".html", "text/html" },  { ".htm",  "text/html" },
        { ".css",  "text/css" },   { ".js",   "text/javascript" },
        { ".mjs",  "text/javascript" },
        { ".json", "application/json" }, { ".txt",  "text/plain" },
        { ".md",   "text/markdown" },    { ".csv",  "text/csv" },
        { ".xml",  "text/xml" },
        { ".png",  "image/png" },  { ".jpg",  "image/jpeg" },
        { ".jpeg", "image/jpeg" }, { ".gif",  "image/gif" },
        { ".svg",  "image/svg+xml" }, { ".webp", "image/webp" },
        { ".ico",  "image/vnd.microsoft.icon" }, { ".bmp", "image/bmp" },
        { ".pdf",  "application/pdf" },
        { ".zip",  "application/zip" },
        { ".mp3",  "audio/mpeg" }, { ".wav",  "audio/x-wav" },
        { ".ogg",  "audio/ogg" },
        { ".mp4",  "video/mp4" },  { ".webm", "video/webm" },
        { ".mov",  "video/quicktime" },
        { ".woff", "font/woff" },  { ".woff2","font/woff2" },
        { ".ttf",  "font/ttf" },   { ".otf",  "font/otf" },
    };
    for (size_t i = 0; i < sizeof(TAB) / sizeof(TAB[0]); i++)
        if (strcasecmp(ponto, TAB[i].ext) == 0) return TAB[i].mime;
    return "application/octet-stream";
}

static int hexval(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void urldecode_base(char *s, int mais_vira_espaco)
{
    char *w = s;
    for (char *p2 = s; *p2; p2++) {
        if (*p2 == '%' && hexval(p2[1]) >= 0 && hexval(p2[2]) >= 0) {
            *w++ = (char)(hexval(p2[1]) * 16 + hexval(p2[2]));
            p2 += 2;
        } else if (mais_vira_espaco && *p2 == '+') {
            *w++ = ' ';
        } else {
            *w++ = *p2;
        }
    }
    *w = '\0';
}

void ps_jk_urldecode(char *s)    { urldecode_base(s, 0); }
void ps_jk_urldecode_qs(char *s) { urldecode_base(s, 1); }

/* ── TLS ────────────────────────────────────────────────────────────────── */

void *ps_jk_tls_ctx(const char *cert, const char *key, char *erro, size_t ecap)
{
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) { snprintf(erro, ecap, "SSL_CTX_new falhou"); return NULL; }
    if (SSL_CTX_use_certificate_chain_file(ctx, cert) != 1 ||
        SSL_CTX_use_PrivateKey_file(ctx, key ? key : cert, SSL_FILETYPE_PEM) != 1) {
        snprintf(erro, ecap, "falha ao carregar certificado: %s",
                 ERR_reason_error_string(ERR_get_error()));
        SSL_CTX_free(ctx);
        return NULL;
    }
    return ctx;
}

void ps_jk_tls_ctx_solta(void *ctx)
{
    if (ctx) SSL_CTX_free((SSL_CTX *)ctx);
}

int ps_jk_tls_autogera(const char *cert_path, const char *key_path,
                       char *erro, size_t ecap)
{
    int rc = -1;
    EVP_PKEY *pk = EVP_RSA_gen(2048);
    if (!pk) { snprintf(erro, ecap, "geracao de chave RSA falhou"); return -1; }

    X509 *x = X509_new();
    if (!x) { EVP_PKEY_free(pk); snprintf(erro, ecap, "X509_new falhou"); return -1; }
    X509_set_version(x, 2);          /* v3: obrigatório pra ter extensões (SAN) */
    ASN1_INTEGER_set(X509_get_serialNumber(x), (long)time(NULL));
    X509_gmtime_adj(X509_getm_notBefore(x), 0);
    X509_gmtime_adj(X509_getm_notAfter(x), 365L * 24 * 3600);
    X509_set_pubkey(x, pk);
    X509_NAME *nome = X509_get_subject_name(x);
    X509_NAME_add_entry_by_txt(nome, "CN", MBSTRING_ASC,
                               (const unsigned char *)"localhost", -1, -1, 0);
    X509_set_issuer_name(x, nome);   /* self-signed: emissor = sujeito */

    /* Extensões v3. Sem Subject Alternative Name o cliente moderno (browser,
     * curl, urllib) REJEITA mesmo confiando na CA — CN sozinho não vale mais.
     * CA:TRUE permite adicionar o cert ao trust store como sua própria CA. */
    X509V3_CTX ctx;
    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, x, x, NULL, NULL, 0);
    X509_EXTENSION *ext;
    ext = X509V3_EXT_conf_nid(NULL, &ctx, NID_subject_alt_name,
                              "DNS:localhost,IP:127.0.0.1,IP:0:0:0:0:0:0:0:1");
    if (ext) { X509_add_ext(x, ext, -1); X509_EXTENSION_free(ext); }
    ext = X509V3_EXT_conf_nid(NULL, &ctx, NID_basic_constraints, "critical,CA:TRUE");
    if (ext) { X509_add_ext(x, ext, -1); X509_EXTENSION_free(ext); }

    if (X509_sign(x, pk, EVP_sha256()) == 0) {
        snprintf(erro, ecap, "assinatura do certificado falhou");
        goto fim;
    }

    FILE *f = fopen(key_path, "w");
    if (!f) { snprintf(erro, ecap, "nao criou %s: %s", key_path, strerror(errno)); goto fim; }
    PEM_write_PrivateKey(f, pk, NULL, NULL, 0, NULL, NULL);
    fclose(f);
    f = fopen(cert_path, "w");
    if (!f) { snprintf(erro, ecap, "nao criou %s: %s", cert_path, strerror(errno)); goto fim; }
    PEM_write_X509(f, x);
    fclose(f);
    rc = 0;
fim:
    X509_free(x);
    EVP_PKEY_free(pk);
    return rc;
}
