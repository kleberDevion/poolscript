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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ps_so.h"      /* socket de cada sistema (no Windows, Winsock) */
#if !defined(_WIN32)
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#endif

/* O macOS não tem MSG_NOSIGNAL; o SIGPIPE já é ignorado pelo processo
 * inteiro (jinga_vm.c), então sem a flag o `send` num cano fechado continua
 * só devolvendo erro. */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
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
    /* HEAD: a resposta leva os MESMOS headers do GET (Content-Length
     * inclusive) e NENHUM corpo (RFC 9110). Quem roteia liga isto ao ler a
     * requisição, e ps_jk_responde pula a escrita do corpo. */
    int   sem_corpo;
};

/* ── E/S básica ─────────────────────────────────────────────────────────── */

static long conn_le(PSJkConn *c, char *dst, size_t cap)
{
    if (c->ssl) {
        int r = SSL_read(c->ssl, dst, (int)cap);
        return r > 0 ? r : -1;
    }
    long r;
    do { r = ps_sock_le(c->fd, dst, cap); } while (r < 0 && errno == EINTR);
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
            do { r = ps_sock_escreve(c->fd, src + feito, n - feito, MSG_NOSIGNAL); }
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

/* Resolve `host` pra IPv4. Todo nome que não era IP literal caía no loopback
 * SEM resolver: `app(host="meu-servidor")` escutava só em 127.0.0.1 e o log
 * dizia o nome pedido. Nome que não resolve é erro, não fallback. */
static int jk_resolve_ipv4(const char *host, struct in_addr *out, char *erro, size_t ecap)
{
    if (inet_pton(AF_INET, host, out) == 1) return 0;
    struct addrinfo dicas, *res = NULL;
    memset(&dicas, 0, sizeof(dicas));
    dicas.ai_family = AF_INET;
    dicas.ai_socktype = SOCK_STREAM;
    int rc = getaddrinfo(host, NULL, &dicas, &res);
    if (rc != 0 || !res) {
        snprintf(erro, ecap, "host '%s' nao resolve: %s", host,
                 rc != 0 ? gai_strerror(rc) : "sem endereco IPv4");
        if (res) freeaddrinfo(res);
        return -1;
    }
    *out = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
    freeaddrinfo(res);
    return 0;
}

int ps_jk_listen(const char *host, int porta, char *erro, size_t ecap)
{
    /* a porta tem 16 bits: o `(unsigned short)` abaixo punha 70000 na 4464, e
     * a do WebSocket (HTTP + 1) de um servidor na 65535 numa porta sorteada
     * pelo sistema (jinker-14) */
    if (porta < 0 || porta > 65535) {
        snprintf(erro, ecap, "porta fora da faixa 0..65535: %d", porta);
        return -1;
    }
    int fd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { snprintf(erro, ecap, "socket: %s", strerror(ps_sock_errno())); return -1; }
    ps_sock_reusa_endereco(fd);

    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons((unsigned short)porta);
    if (!host || !host[0] || strcmp(host, "0.0.0.0") == 0)
        a.sin_addr.s_addr = INADDR_ANY;
    else if (jk_resolve_ipv4(host, &a.sin_addr, erro, ecap) != 0) {
        ps_sock_fecha(fd); return -1;
    }
    if (bind(fd, (struct sockaddr *)&a, sizeof(a)) != 0) {
        snprintf(erro, ecap, "bind %s:%d: %s", host ? host : "", porta, strerror(ps_sock_errno()));
        ps_sock_fecha(fd); return -1;
    }
    if (listen(fd, 64) != 0) {
        snprintf(erro, ecap, "listen: %s", strerror(ps_sock_errno()));
        ps_sock_fecha(fd); return -1;
    }
    /* fd de escuta NÃO-BLOQUEANTE: com multi-processo (prefork) vários workers
     * acordam no mesmo `poll` (thundering herd) e chamam accept(); um pega, os
     * outros recebem EAGAIN -> ps_jk_accept devolve NULL e o worker segue, em
     * vez de travar bloqueado no accept(). A conexão aceita é bloqueante (com
     * o SO_RCVTIMEO dela): o ps_jk_accept garante, porque no macOS e no
     * Windows ela herdaria o não-bloqueante desta escuta. */
    ps_sock_nao_bloqueia(fd, 1);
    return fd;
}

PSJkConn *ps_jk_accept(int fd_escuta, void *ssl_ctx, char *ip, size_t ipcap)
{
    struct sockaddr_in a; socklen_t al = sizeof(a);
    int fd;
    do { fd = (int)accept(fd_escuta, (struct sockaddr *)&a, &al); if (fd < 0) ps_sock_errno(); }
    while (fd < 0 && errno == EINTR);
    if (fd < 0) return NULL;
    /* A conexão é bloqueante (com timeout): o pedido e o handshake do TLS são
     * lidos numa thread do pool, que espera os bytes. O socket aceito herda o
     * não-bloqueante da escuta no macOS (BSD) e no Windows; no Linux não. Só
     * o Windows desligava, e no macOS a leitura voltava EAGAIN quando o
     * cliente ainda não tinha mandado nada: o servidor dava a conexão por
     * caída e fechava (o `ws_connect` caía no handshake, o 403 chegava como
     * RST, o HTTPS fechava depois do ClientHello). */
    ps_sock_nao_bloqueia(fd, 0);

    if (ip && ipcap) {
        ip[0] = '\0';
        inet_ntop(AF_INET, &a.sin_addr, ip, (socklen_t)ipcap);
    }
    /* Nagle desligado — o análogo do disable_nagle_algorithm do wrapper */
    int um = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&um, sizeof(um));
    /* timeout de leitura: keep-alive ocioso não segura o loop pra sempre */
    ps_sock_timeout(fd, 30000, -1);

    PSJkConn *c = calloc(1, sizeof(PSJkConn));
    if (!c) { ps_sock_fecha(fd); return NULL; }
    c->fd = fd;

    if (ssl_ctx) {
        c->ssl = SSL_new((SSL_CTX *)ssl_ctx);
        if (!c->ssl || SSL_set_fd(c->ssl, fd) != 1 || SSL_accept(c->ssl) != 1) {
            if (c->ssl) SSL_free(c->ssl);
            ps_sock_fecha(fd); free(c);
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
    ps_sock_fecha(c->fd);
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

/* Content-Length, tratando a REPETIÇÃO.
 *
 * `ps_jk_header` devolve o primeiro que achar, e era isso que acontecia com
 * dois `Content-Length` na mesma requisição: o servidor usava um, o
 * intermediário na frente podia usar o outro, e o pedaço entre os dois
 * tamanhos vira uma requisição que só um dos dois enxerga. É o smuggling
 * clássico (RFC 9112 §6.3): valores DIFERENTES têm que ser recusados;
 * repetição do MESMO valor é aceita e colapsa em um.
 *
 * Devolve o valor, NULL se não houver, ou JK_CL_INVALIDO se discordarem. */
#define JK_CL_INVALIDO ((const char *)-1)

static const char *jk_content_length(const PSJkReq *r)
{
    const char *achado = NULL;
    for (int i = 0; i < r->ncabs; i++) {
        if (strcasecmp(r->cabs[i].nome, "Content-Length") != 0) continue;
        const char *v = r->cabs[i].valor;
        if (!achado) { achado = v; continue; }
        if (strcmp(achado, v) != 0) return JK_CL_INVALIDO;
    }
    return achado;
}

static char *dup_faixa(const char *a, const char *b)
{
    size_t n = (size_t)(b - a);
    char *s = malloc(n + 1);
    if (!s) return NULL;
    memcpy(s, a, n); s[n] = '\0';
    return s;
}

/* ── arena do request ───────────────────────────────────────────────────────
 * Bump allocator: as alocações PEQUENAS do parse (path, query, vetor de
 * headers, nome/valor de cada header) saem de UM bloco de 4KB (encadeia outro
 * se estourar) em vez de ~10-20 mallocs por requisição; ps_jk_req_solta solta a
 * cadeia inteira de uma vez — impossível vazar um campo isolado. A arena vive
 * na PSJkReq (não na conexão) pra 100k conexões keep-alive OCIOSAS não pagarem
 * 4KB cada (mesma razão do ps_jk_conn_solta_buf). */
typedef struct JkArBloco { struct JkArBloco *prox; size_t cap, usado; char mem[]; } JkArBloco;

static void *jk_ar_alloc(PSJkReq *r, size_t n)
{
    n = (n + 7) & ~(size_t)7;
    JkArBloco *b = (JkArBloco *)r->ar;
    if (!b || b->usado + n > b->cap) {
        size_t cap = n > 4096 ? n : 4096;
        JkArBloco *nb = malloc(sizeof(JkArBloco) + cap);
        if (!nb) return NULL;
        nb->prox = b; nb->cap = cap; nb->usado = 0;
        r->ar = nb; b = nb;
    }
    void *p = b->mem + b->usado;
    b->usado += n;
    return p;
}

static char *jk_ar_faixa(PSJkReq *r, const char *a, const char *b)
{
    size_t n = (size_t)(b - a);
    char *s = jk_ar_alloc(r, n + 1);
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
            /* LF PURO: `\n\n` sem `\r` termina cabeçalho em cliente relaxado, e
             * aqui NÃO termina — aceitar LF puro é vetor de request smuggling,
             * porque um intermediário na frente corta a requisição num ponto
             * diferente do nosso. O nginx recusa pelo mesmo motivo.
             *
             * O que se ganha reconhecendo o `\n\n` é só ISTO: responder 400 e
             * fechar. Antes o parser seguia pedindo mais bytes que nunca vinham
             * e a conexão ficava presa até o timeout — recurso segurado por
             * lixo. Reconhecer pra RECUSAR é estritamente mais seguro que
             * esperar: não aceita nada a mais e não paga o timeout.
             *
             * A busca só acontece quando o `\r\n\r\n` NÃO foi achado, então um
             * corpo legítimo com `\n\n` dentro nunca chega aqui: naquele caso o
             * fim de cabeçalho já foi encontrado e o laço saiu acima. */
            if (memmem(c->buf, c->n, "\n\n", 2)) return PSJK_MALFORM;
        }
        /* MALFORM, nao FECHA: 64 KB de header e lixo mandado por alguem, e
         * lixo se recusa COM RESPOSTA. Fechar calado e o que deixa um
         * intermediario na frente ver uma requisicao diferente da nossa. */
        if (c->n > 64 * 1024) return PSJK_MALFORM;
        if (conn_enche(c) != 0) return -1;
    }
    size_t nhead = (size_t)(fim - c->buf) + 4;

    /* linha de pedido: METODO alvo HTTP/1.x */
    char *p = c->buf;
    char *eol = memmem(p, nhead, "\r\n", 2);
    /* Daqui ate o fim da linha de pedido, todo erro e SINTAXE do cliente e
     * sai como PSJK_MALFORM -> 400. Antes saiam como -1, que e PSJK_FECHA,
     * e o servidor fechava calado — o mesmo defeito que 0f54f28 corrigiu nos
     * headers e nao tocou aqui. */
    if (!eol) return PSJK_MALFORM;
    char *sp1 = memchr(p, ' ', (size_t)(eol - p));
    if (!sp1) return PSJK_MALFORM;
    size_t nm = (size_t)(sp1 - p);
    if (nm >= sizeof(r->metodo)) return PSJK_MALFORM;
    memcpy(r->metodo, p, nm); r->metodo[nm] = '\0';
    /* HEAD: mesma resposta do GET, sem o corpo (ver PSJkConn.sem_corpo) */
    c->sem_corpo = (strcmp(r->metodo, "HEAD") == 0);
    char *sp2 = memchr(sp1 + 1, ' ', (size_t)(eol - sp1 - 1));
    if (!sp2) return PSJK_MALFORM;
    char *alvo = jk_ar_faixa(r, sp1 + 1, sp2);
    if (!alvo) { ps_jk_req_solta(r); return -1; }

    /* separa query e decodifica o path */
    char *q = strchr(alvo, '?');
    if (q) { *q = '\0'; r->query = jk_ar_faixa(r, q + 1, q + 1 + strlen(q + 1)); }
    else     r->query = jk_ar_faixa(r, "", "");
    /* `%00` decodifica pra byte 0 e TRUNCA o path antes do roteamento:
     * `/users/7%00/photos` virava `/users/7` e a rota curta respondia 200
     * por um caminho que só existe como POST. Byte nulo não é caminho; é
     * sintaxe do cliente, 400 como o resto desta linha. */
    if (strstr(alvo, "%00")) { ps_jk_req_solta(r); return PSJK_MALFORM; }
    ps_jk_urldecode(alvo);
    r->path = alvo;
    if (!r->query) { ps_jk_req_solta(r); return -1; }

    /* headers */
    int cap_c = 8;
    r->cabs = jk_ar_alloc(r, sizeof(PSJkHdr) * (size_t)cap_c);
    if (!r->cabs) { ps_jk_req_solta(r); return -1; }
    p = eol + 2;
    while (p < c->buf + nhead - 4) {
        char *e2 = memmem(p, nhead - (size_t)(p - c->buf), "\r\n", 2);
        if (!e2 || e2 == p) break;
        char *dois = memchr(p, ':', (size_t)(e2 - p));
        /* Linha de header SEM `:` não é header — é lixo, e ignorar em silêncio
         * deixa o intermediário da frente enxergar uma requisição diferente da
         * nossa. RFC 9112 §2.2. */
        if (!dois) { ps_jk_req_solta(r); return PSJK_MALFORM; }
        /* ESPAÇO ANTES DO `:` é citado nominalmente pela RFC 9112 §5.1 como
         * vetor de smuggling, e ela manda responder 400. Antes, `Content-Length
         * : 5` virava um header de nome "Content-Length " (com o espaço), que
         * nenhuma busca acha — o header sumia e o corpo era descartado. */
        if (dois > p && (dois[-1] == ' ' || dois[-1] == '\t')) {
            ps_jk_req_solta(r); return PSJK_MALFORM;
        }
        {
            if (r->ncabs == cap_c) {
                /* bump não realloca: pega vetor maior da arena e copia; o
                 * antigo fica abandonado no bloco (o reset recolhe tudo) */
                cap_c *= 2;
                PSJkHdr *nh = jk_ar_alloc(r, sizeof(PSJkHdr) * (size_t)cap_c);
                if (!nh) { ps_jk_req_solta(r); return -1; }
                memcpy(nh, r->cabs, sizeof(PSJkHdr) * (size_t)r->ncabs);
                r->cabs = nh;
            }
            const char *v = dois + 1;
            while (v < e2 && (*v == ' ' || *v == '\t')) v++;
            r->cabs[r->ncabs].nome  = jk_ar_faixa(r, p, dois);
            r->cabs[r->ncabs].valor = jk_ar_faixa(r, v, e2);
            if (!r->cabs[r->ncabs].nome || !r->cabs[r->ncabs].valor) { ps_jk_req_solta(r); return -1; }
            r->ncabs++;
        }
        p = e2 + 2;
    }

    /* keep-alive: HTTP/1.1 salvo "Connection: close" */
    r->keep_alive = 1;
    const char *con = ps_jk_header(r, "Connection");
    if (con && strcasecmp(con, "close") == 0) r->keep_alive = 0;

    /* Transfer-Encoding não é implementado, e "não implementado" tem que ser
     * DITO. Antes, um POST `chunked` era servido com 200 e corpo VAZIO: o
     * cliente acha que mandou, o servidor acha que não veio nada, e a
     * diferença entre os dois é exatamente o que o smuggling explora. RFC 9112
     * §6.3 e §7.1: quem não entende a codificação responde 501.
     *
     * Content-Length JUNTO com Transfer-Encoding é pior — os dois dizem onde o
     * corpo acaba, e discordar é o ataque clássico. Aí é 400. */
    const char *te = ps_jk_header(r, "Transfer-Encoding");
    if (te) {
        /* A consulta vem ANTES do `solta`: depois dele a arena já morreu e o
         * header não existe mais — era o que fazia TE+CL responder 501 em vez
         * do 400 que o par exige. */
        int com_cl = ps_jk_header(r, "Content-Length") != NULL;
        ps_jk_req_solta(r);
        return com_cl ? PSJK_MALFORM : PSJK_NAOIMPL;
    }

    /* corpo por Content-Length */
    size_t ncorpo = 0;
    const char *cl = jk_content_length(r);
    if (cl == JK_CL_INVALIDO) { ps_jk_req_solta(r); return PSJK_MALFORM; }
    if (cl) {
        /* `atol` engolia tudo: "abc" virava 0 e o corpo sumia. O valor tem que
         * ser dígito do começo ao fim — RFC 9112 §6.2 não admite sinal, espaço
         * nem sufixo. */
        const char *d = cl;
        if (!*d) { ps_jk_req_solta(r); return PSJK_MALFORM; }
        for (; *d; d++)
            if (*d < '0' || *d > '9') { ps_jk_req_solta(r); return PSJK_MALFORM; }
        errno = 0;
        long v = strtol(cl, NULL, 10);
        if (errno || v < 0 || (unsigned long)v > JK_CORPO_MAX) {
            ps_jk_req_solta(r); return PSJK_MALFORM;
        }
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
    /* path/query/cabs/nome/valor vivem na ARENA — a cadeia sai de uma vez.
     * Só o corpo (alocação única, potencialmente grande) é malloc avulso. */
    free(r->corpo);
    JkArBloco *b = (JkArBloco *)r->ar;
    while (b) { JkArBloco *px = b->prox; free(b); b = px; }
    memset(r, 0, sizeof(*r));
}

/* ── resposta ───────────────────────────────────────────────────────────── */

static const char *frase(int status)
{
    switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 204: return "No Content";
        case 206: return "Partial Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 409: return "Conflict";
        case 416: return "Range Not Satisfiable";
        case 422: return "Unprocessable Content";
        case 429: return "Too Many Requests";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        default:
            /* código sem frase própria: a da CLASSE. Saía "OK" em qualquer
             * um, inclusive `503 OK`. */
            if (status >= 500) return "Server Error";
            if (status >= 400) return "Client Error";
            if (status >= 300) return "Redirection";
            if (status >= 200) return "Success";
            return "Informational";
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
    if (c->sem_corpo) return 0;          /* HEAD: headers sim, corpo não */
    if (ncorpo && conn_escreve(c, corpo, ncorpo) != 0) return -1;
    return 0;
}

/* ── WebSocket (o cliente: o `ws_connect` do módulo request) ────────────── */

/* precisa de `n` bytes prontos no buffer da conexão */
static int ws_precisa(PSJkConn *c, size_t n)
{
    while (c->n < n)
        if (conn_enche(c) != 0) return -1;
    return 0;
}

/* Um frame do cliente, MASCARADO: todo frame que o cliente manda leva
 * máscara, os de controle também (o close, o pong) — RFC 6455 §5.1. Eles
 * saíam sem máscara, do tempo em que esta função era do servidor da jinker.
 *
 * A chave de máscara TEM que ser imprevisível — RFC 6455 §5.3. Ela é a
 * defesa contra envenenamento de cache num intermediário que não entende
 * WebSocket: sem entropia, o atacante escolhe os bytes que o proxy vê. O
 * fallback aqui era `rand()` sem `srand()` em lugar nenhum do projeto, ou
 * seja, a MESMA sequência em todo processo. Falhar abrindo, com chave
 * previsível, é pior que falhar: agora a operação falha. */
static int ws_manda_frame(PSJkConn *c, int opcode, const char *dados, size_t n)
{
    unsigned char cab[14];
    size_t nc;
    cab[0] = (unsigned char)(0x80 | opcode);   /* FIN + opcode */
    unsigned char masc[4];
    if (ps_random_bytes(masc, 4) != 0) return -1;
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
    if (n == 0) return 0;
    /* aplica a máscara num buffer próprio pra não alterar o do chamador */
    char pilha[1024];
    char *tmp = n <= sizeof(pilha) ? pilha : malloc(n);
    if (!tmp) return -1;
    for (size_t i = 0; i < n; i++) tmp[i] = (char)(dados[i] ^ masc[i % 4]);
    int rc = conn_escreve(c, tmp, n);
    if (tmp != pilha) free(tmp);
    return rc;
}

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
        /* o servidor não mascara; frame mascarado vindo dele é erro de
         * protocolo, e o cliente fecha — RFC 6455 §5.1 */
        if (mascarado) { free(acc); return -1; }
        if (ws_precisa(c, off + np) != 0) { free(acc); return -1; }

        char *dados = malloc(np + 1);
        if (!dados) { free(acc); return -1; }
        memcpy(dados, c->buf + off, np);
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
    int fd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { snprintf(erro, ecap, "socket: %s", strerror(ps_sock_errno())); return NULL; }
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons((unsigned short)porta);
    if (jk_resolve_ipv4(host && host[0] ? host : "127.0.0.1", &a.sin_addr, erro, ecap) != 0) {
        ps_sock_fecha(fd); return NULL;
    }
    ps_sock_timeout(fd, 10000, -1);
    int um = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&um, sizeof(um));
    if (connect(fd, (struct sockaddr *)&a, sizeof(a)) != 0) {
        snprintf(erro, ecap, "falha de conexão: %s", strerror(ps_sock_errno()));
        ps_sock_fecha(fd); return NULL;
    }
    PSJkConn *c = calloc(1, sizeof(PSJkConn));
    if (!c) { ps_sock_fecha(fd); snprintf(erro, ecap, "sem memoria"); return NULL; }
    c->fd = fd;

    /* chave aleatória de 16 bytes em base64 */
    /* Mesma razão da máscara: sem entropia, não sobe a conexão. */
    unsigned char cru[16];
    if (ps_random_bytes(cru, sizeof(cru)) != 0) {
        snprintf(erro, ecap, "sem fonte de entropia do sistema");
        ps_sock_fecha(fd); free(c); return NULL;
    }
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
    /* o retorno é o tamanho que TERIA, não o que coube: com path ou host
     * longos isso mandava pilha nossa pro servidor. Mesmo defeito que vazava a
     * pilha do jinker num 404 com URL grande. */
    if (n < 0 || (size_t)n >= sizeof(req)) {
        snprintf(erro, ecap, "endereco do websocket longo demais");
        ps_jk_close(c); return NULL;
    }
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
{ return ws_manda_frame(c, 0x1, msg, n); }

/* 1 = ainda há bytes NO BUFFER, já lidos do socket e não consumidos.
 *
 * É o que o pipelining precisa: duas requisições no mesmo `write` chegam
 * juntas, `ps_jk_le_request` consome a primeira e a segunda fica aqui. Quem
 * espera o epoll disparar espera pra sempre — o dado já saiu do socket. */
int ps_jk_conn_pendente(const PSJkConn *c) { return c && c->n > 0; }

int ps_jk_ws_tem_dados(PSJkConn *c, int timeout_ms)
{
    if (c->n > 0) return 1;
    return ps_sock_espera_leitura(c->fd, timeout_ms);
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
                if (!out[np].dados) {
                    /* `np` ainda não avançou, então o `solta` abaixo varre só
                     * até a parte ANTERIOR — os três campos desta parte já
                     * estão alocados e ficariam para trás. O jinker é processo
                     * longo: vazamento aqui acumula requisição após requisição. */
                    free(out[np].campo); free(out[np].filename); free(out[np].ctype);
                    free(cabs); ps_jk_partes_solta(out, np); return -1;
                }
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

/* O tipo do arquivo pela extensão.
 *
 * A tabela é a `_types_map_default` do `mimetypes` do Python 3.15 (213
 * extensões), a mesma que o `http.server` usa, traduzida pra cá em ordem
 * alfabética pra busca binária. Era uma tabela escrita à mão, com 36
 * extensões: o `.wasm` saía `application/octet-stream`, e o navegador recusa
 * esse tipo no `WebAssembly.instantiateStreaming` ("If mimeType is not a
 * byte-case-insensitive match for application/wasm, reject returnValue with a
 * TypeError", WebAssembly Web API). A única extensão da tabela antiga que
 * mudou de tipo é `.wav`: `audio/x-wav` virou `audio/vnd.wave`, o do Python.
 *
 * Extensão desconhecida é `application/octet-stream`, como o
 * `default_content_type` do Python. Pra pôr uma extensão nova, a fonte é a
 * tabela do Python, e a ordem alfabética tem que se manter.
 *
 * O texto mora DENTRO da entrada (vetor de char), não num ponteiro: ponteiro
 * em dado inicializado num executável PIE é uma relocação que o carregador
 * faz na partida de TODO programa, use ele a jinker ou não. Com ponteiros
 * esta tabela somava 366 relocações a mais (medido no Linux, 25.367 ->
 * 25.733) e a partida subia 0,58%. A maior extensão tem 12 letras
 * (".webmanifest") e o maior tipo 73. */
typedef struct { char ext[13]; char mime[74]; } JkMime;

static const JkMime JK_MIMES[] = {
    { ".3g2", "audio/3gpp2" }, { ".3gp", "audio/3gpp" }, { ".3gpp", "audio/3gpp" },
    { ".3gpp2", "audio/3gpp2" }, { ".7z", "application/x-7z-compressed" },
    { ".a", "application/octet-stream" }, { ".aac", "audio/aac" }, { ".adts", "audio/aac" },
    { ".ai", "application/pdf" }, { ".aif", "audio/x-aiff" }, { ".aifc", "audio/x-aiff" },
    { ".aiff", "audio/x-aiff" }, { ".ass", "audio/aac" }, { ".au", "audio/basic" },
    { ".avi", "video/vnd.avi" }, { ".avif", "image/avif" }, { ".bat", "text/plain" },
    { ".bcpio", "application/x-bcpio" }, { ".bin", "application/octet-stream" },
    { ".bmp", "image/bmp" }, { ".c", "text/plain" },
    { ".cab", "application/vnd.ms-cab-compressed" }, { ".cdf", "application/x-netcdf" },
    { ".chm", "application/vnd.ms-htmlhelp" }, { ".cjs", "application/node" },
    { ".cpio", "application/x-cpio" }, { ".csh", "application/x-csh" }, { ".css", "text/css" },
    { ".csv", "text/csv" }, { ".dcm", "application/dicom" },
    { ".deb", "application/x-debian-package" },
    { ".dll", "application/vnd.microsoft.portable-executable" },
    { ".doc", "application/msword" },
    { ".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
    { ".dot", "application/msword" }, { ".dvi", "application/x-dvi" },
    { ".efi", "application/efi" }, { ".emf", "image/emf" }, { ".eml", "message/rfc822" },
    { ".eot", "application/vnd.ms-fontobject" }, { ".eps", "application/postscript" },
    { ".epub", "application/epub+zip" }, { ".etx", "text/x-setext" },
    { ".exe", "application/vnd.microsoft.portable-executable" }, { ".fits", "image/fits" },
    { ".flac", "audio/flac" }, { ".g3", "image/g3fax" }, { ".gif", "image/gif" },
    { ".glb", "model/gltf-binary" }, { ".gltf", "model/gltf+json" },
    { ".gtar", "application/x-gtar" }, { ".gz", "application/gzip" }, { ".h", "text/plain" },
    { ".h5", "application/x-hdf5" }, { ".hdf", "application/x-hdf" }, { ".heic", "image/heic" },
    { ".heif", "image/heif" }, { ".hjif", "haptics/hjif" }, { ".hmpg", "haptics/hmpg" },
    { ".htm", "text/html" }, { ".html", "text/html" }, { ".ico", "image/vnd.microsoft.icon" },
    { ".ief", "image/ief" }, { ".ivs", "haptics/ivs" }, { ".jp2", "image/jp2" },
    { ".jpe", "image/jpeg" }, { ".jpeg", "image/jpeg" }, { ".jpg", "image/jpeg" },
    { ".jpm", "image/jpm" }, { ".jpx", "image/jpx" }, { ".js", "text/javascript" },
    { ".json", "application/json" }, { ".jxl", "image/jxl" }, { ".ksh", "text/plain" },
    { ".latex", "application/x-latex" }, { ".loas", "audio/aac" }, { ".m1v", "video/mpeg" },
    { ".m3u", "application/vnd.apple.mpegurl" }, { ".m3u8", "application/vnd.apple.mpegurl" },
    { ".m4a", "audio/mp4" }, { ".m4v", "video/x-m4v" }, { ".man", "application/x-troff-man" },
    { ".markdown", "text/markdown" }, { ".md", "text/markdown" },
    { ".me", "application/x-troff-me" }, { ".mht", "message/rfc822" },
    { ".mhtml", "message/rfc822" }, { ".mif", "application/x-mif" },
    { ".mjs", "text/javascript" }, { ".mk3d", "video/matroska-3d" },
    { ".mka", "audio/matroska" }, { ".mkv", "video/matroska" }, { ".mov", "video/quicktime" },
    { ".movie", "video/x-sgi-movie" }, { ".mp2", "audio/mpeg" }, { ".mp3", "audio/mpeg" },
    { ".mp4", "video/mp4" }, { ".mpa", "video/mpeg" }, { ".mpe", "video/mpeg" },
    { ".mpeg", "video/mpeg" }, { ".mpg", "video/mpeg" }, { ".ms", "application/x-troff-ms" },
    { ".n3", "text/n3" }, { ".nc", "application/x-netcdf" }, { ".nq", "application/n-quads" },
    { ".nt", "application/n-triples" }, { ".nws", "message/rfc822" },
    { ".o", "application/octet-stream" }, { ".obj", "application/octet-stream" },
    { ".oda", "application/oda" }, { ".odg", "application/vnd.oasis.opendocument.graphics" },
    { ".odp", "application/vnd.oasis.opendocument.presentation" },
    { ".ods", "application/vnd.oasis.opendocument.spreadsheet" },
    { ".odt", "application/vnd.oasis.opendocument.text" }, { ".ogg", "audio/ogg" },
    { ".ogv", "video/ogg" }, { ".ogx", "application/ogg" }, { ".opus", "audio/opus" },
    { ".otf", "font/otf" }, { ".p12", "application/x-pkcs12" },
    { ".p7c", "application/pkcs7-mime" }, { ".pbm", "image/x-portable-bitmap" },
    { ".pdf", "application/pdf" }, { ".pfx", "application/x-pkcs12" },
    { ".pgm", "image/x-portable-graymap" }, { ".php", "application/x-httpd-php" },
    { ".pl", "text/plain" }, { ".png", "image/png" }, { ".pnm", "image/x-portable-anymap" },
    { ".pot", "application/vnd.ms-powerpoint" }, { ".ppa", "application/vnd.ms-powerpoint" },
    { ".ppm", "image/x-portable-pixmap" }, { ".pps", "application/vnd.ms-powerpoint" },
    { ".ppt", "application/vnd.ms-powerpoint" },
    { ".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation" },
    { ".ps", "application/postscript" }, { ".pwz", "application/vnd.ms-powerpoint" },
    { ".py", "text/x-python" }, { ".pyc", "application/x-python-code" },
    { ".pyo", "application/x-python-code" }, { ".qt", "video/quicktime" },
    { ".ra", "audio/x-pn-realaudio" }, { ".ram", "application/x-pn-realaudio" },
    { ".rar", "application/vnd.rar" }, { ".ras", "image/x-cmu-raster" },
    { ".rdf", "application/xml" }, { ".rgb", "image/x-rgb" },
    { ".roff", "application/x-troff" }, { ".rpm", "application/x-rpm" },
    { ".rst", "text/x-rst" }, { ".rtf", "application/rtf" }, { ".rtx", "text/richtext" },
    { ".sgm", "text/x-sgml" }, { ".sgml", "text/x-sgml" }, { ".sh", "application/x-sh" },
    { ".shar", "application/x-shar" }, { ".snd", "audio/basic" },
    { ".so", "application/octet-stream" }, { ".sql", "application/sql" },
    { ".sqlite", "application/vnd.sqlite3" }, { ".sqlite3", "application/vnd.sqlite3" },
    { ".src", "application/x-wais-source" }, { ".srt", "text/plain" }, { ".stl", "model/stl" },
    { ".sv4cpio", "application/x-sv4cpio" }, { ".sv4crc", "application/x-sv4crc" },
    { ".svg", "image/svg+xml" }, { ".swf", "application/x-shockwave-flash" },
    { ".t", "application/x-troff" }, { ".t38", "image/t38" }, { ".tar", "application/x-tar" },
    { ".tcl", "application/x-tcl" }, { ".tex", "application/x-tex" },
    { ".texi", "application/texinfo" }, { ".texinfo", "application/texinfo" },
    { ".tfx", "image/tiff-fx" }, { ".thmx", "application/vnd.ms-officetheme" },
    { ".tif", "image/tiff" }, { ".tiff", "image/tiff" }, { ".toml", "application/toml" },
    { ".tr", "application/x-troff" }, { ".trig", "application/trig" },
    { ".tsv", "text/tab-separated-values" }, { ".ttc", "font/collection" },
    { ".ttf", "font/ttf" }, { ".txt", "text/plain" }, { ".ustar", "application/x-ustar" },
    { ".vcf", "text/x-vcard" }, { ".vtt", "text/vtt" }, { ".wasm", "application/wasm" },
    { ".wav", "audio/vnd.wave" }, { ".weba", "audio/webm" }, { ".webm", "video/webm" },
    { ".webmanifest", "application/manifest+json" }, { ".webp", "image/webp" },
    { ".wiz", "application/msword" }, { ".wmf", "image/wmf" }, { ".wmv", "video/x-ms-wmv" },
    { ".woff", "font/woff" }, { ".woff2", "font/woff2" }, { ".wsdl", "application/xml" },
    { ".xbm", "image/x-xbitmap" }, { ".xlb", "application/vnd.ms-excel" },
    { ".xls", "application/vnd.ms-excel" },
    { ".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
    { ".xml", "text/xml" }, { ".xpdl", "application/xml" }, { ".xpm", "image/x-xpixmap" },
    { ".xsl", "application/xml" }, { ".xwd", "image/x-xwindowdump" },
    { ".yaml", "application/yaml" }, { ".yml", "application/yaml" },
    { ".zip", "application/zip" },
};

static int jk_mime_cmp(const void *chave, const void *item)
{
    return strcasecmp((const char *)chave, ((const JkMime *)item)->ext);
}

const char *ps_jk_mime(const char *caminho)
{
    const char *ponto = strrchr(caminho, '.');
    /* o ponto tem que estar no NOME, não numa pasta do caminho
     * ("/srv/v1.2/LEIA" não tem extensão) */
    const char *barra = strrchr(caminho, '/');
    if (!ponto || (barra && barra > ponto)) return "application/octet-stream";
    const JkMime *m = bsearch(ponto, JK_MIMES, sizeof(JK_MIMES) / sizeof(JK_MIMES[0]),
                              sizeof(JK_MIMES[0]), jk_mime_cmp);
    return m ? m->mime : "application/octet-stream";
}

/* ── arquivo inteiro, por pedaço e com cache ─────────────────────────────── */

/* O que o servidor faz aqui segue o `ServeContent` do Go (`net/http`) e a
 * RFC 9110. Antes os dois servidores de arquivo do jinker (`/static/` e o
 * `static_folder`) liam o arquivo INTEIRO pra memória (um `malloc` do tamanho
 * dele, por pedido: um vídeo de 2 GB pedia 2 GB) e respondiam 200 sempre:
 * sem `Range` o Safari não toca vídeo, e sem `Last-Modified`/`ETag` o
 * navegador baixa tudo de novo a cada visita. */

#define JK_BLOCO (64u * 1024u)

static int jk_mes(const char *s)
{
    static const char M[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    for (int i = 0; i < 12; i++)
        if (strncmp(s, M + 3 * i, 3) == 0) return i + 1;
    return 0;
}

/* dias de 1970-01-01 até a data (o `days_from_civil` de Howard Hinnant, a
 * conta que o `chrono` do C++ usa); `m` de 1 a 12 */
static long long jk_dias(long long a, int m, int d)
{
    a -= m <= 2;
    long long era = (a >= 0 ? a : a - 399) / 400;
    long long ano_da_era = a - era * 400;
    long long dia_do_ano = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    long long dia_da_era = ano_da_era * 365 + ano_da_era / 4 - ano_da_era / 100 + dia_do_ano;
    return era * 146097 + dia_da_era - 719468;
}

/* `min` a `max` dígitos a partir de `p`; devolve onde parou, ou NULL */
static const char *jk_digitos(const char *p, int min, int max, int *out)
{
    int v = 0, k = 0;
    while (k < max && *p >= '0' && *p <= '9') { v = v * 10 + (*p - '0'); p++; k++; }
    if (k < min) return NULL;
    *out = v;
    return p;
}

/* "hh:mm:ss" */
static const char *jk_hora(const char *p, int *h, int *mi, int *se)
{
    if (!(p = jk_digitos(p, 2, 2, h)) || *p++ != ':') return NULL;
    if (!(p = jk_digitos(p, 2, 2, mi)) || *p++ != ':') return NULL;
    return jk_digitos(p, 2, 2, se);
}

/* HTTP-date (RFC 9110 5.6.7) em segundos desde 1970, ou -1 se não for data.
 * A RFC manda aceitar os três formatos: o IMF-fixdate (o que todo navegador
 * manda e o que o `Last-Modified` daqui escreve) e os dois obsoletos.
 *
 * Lida à mão, sem `sscanf`: no Linux o `sscanf` não estava entre os nomes que
 * o binário importa, e nome importado a mais é resolvido na partida de TODO
 * programa (o binário é BIND_NOW). Medido: com `sscanf` aqui entrava o
 * `__isoc23_sscanf`, 311 nomes contra 310. */
static long long jk_le_data(const char *s)
{
    const char *p = s;
    int d = 0, m = 0, a = 0, h = 0, mi = 0, se = 0;
    while (*p == ' ' || *p == '\t') p++;
    const char *ini = p;
    while ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z')) p++;   /* o dia da semana */
    if (p == ini) return -1;
    if (p[0] == ',' && p[1] == ' ') {
        p += 2;
        if (!(p = jk_digitos(p, 2, 2, &d))) return -1;
        char sep = *p++;
        if (sep != ' ' && sep != '-') return -1;
        if (!(m = jk_mes(p))) return -1;
        p += 3;
        if (*p++ != sep) return -1;
        if (sep == ' ') {
            /* IMF-fixdate: "Sun, 06 Nov 1994 08:49:37 GMT" */
            if (!(p = jk_digitos(p, 4, 4, &a))) return -1;
        } else {
            /* RFC 850: "Sunday, 06-Nov-94 08:49:37 GMT"; o ano de 2 dígitos é
             * o mais recente que não passa de 50 anos no futuro (5.6.7) */
            if (!(p = jk_digitos(p, 2, 2, &a))) return -1;
            time_t agora = time(NULL);
            struct tm gm; gmtime_r(&agora, &gm);
            int ano = gm.tm_year + 1900;
            a += ano - ano % 100;
            if (a > ano + 50) a -= 100;
        }
        if (*p++ != ' ' || !(p = jk_hora(p, &h, &mi, &se))) return -1;
        if (strncmp(p, " GMT", 4) != 0) return -1;
        p += 4;
    } else if (p[0] == ' ') {
        /* asctime: "Sun Nov  6 08:49:37 1994" (dia de um dígito vem com um
         * espaço na frente) */
        p++;
        if (!(m = jk_mes(p))) return -1;
        p += 3;
        if (*p++ != ' ') return -1;
        if (*p == ' ') p++;
        if (!(p = jk_digitos(p, 1, 2, &d))) return -1;
        if (*p++ != ' ' || !(p = jk_hora(p, &h, &mi, &se))) return -1;
        if (*p++ != ' ' || !(p = jk_digitos(p, 4, 4, &a))) return -1;
    } else {
        return -1;
    }
    while (*p == ' ' || *p == '\t') p++;
    if (*p || d < 1 || d > 31 || h > 23 || mi > 59 || se > 60) return -1;
    return jk_dias(a, m, d) * 86400LL + h * 3600LL + mi * 60LL + se;
}

/* Algum ETag da lista de um `If-None-Match` é este? Comparação FRACA (RFC
 * 9110 13.1.2): o `W/` não conta. "*" casa com qualquer arquivo que exista. */
static int jk_etag_na_lista(const char *lista, const char *etag)
{
    const char *p = lista;
    size_t ne = strlen(etag);
    while (*p) {
        while (*p == ' ' || *p == '\t' || *p == ',') p++;
        if (!*p) break;
        if (*p == '*') return 1;
        if (p[0] == 'W' && p[1] == '/') p += 2;
        const char *ini = p;
        if (*p == '"') {
            p++;
            while (*p && *p != '"') p++;
            if (*p == '"') p++;
        } else {
            while (*p && *p != ',' && *p != ' ' && *p != '\t') p++;
        }
        if ((size_t)(p - ini) == ne && strncmp(ini, etag, ne) == 0) return 1;
    }
    return 0;
}

/* lê um número decimal sem sinal; 0 se não houver dígito ou se estourar */
static int jk_le_num(const char **pp, long long *out)
{
    const char *p = *pp;
    long long v = 0;
    if (*p < '0' || *p > '9') return 0;
    while (*p >= '0' && *p <= '9') {
        if (v > (0x7fffffffffffffffLL - 9) / 10) return 0;
        v = v * 10 + (*p++ - '0');
    }
    *pp = p; *out = v;
    return 1;
}

/* O `Range:` do pedido contra um arquivo de `total` bytes (RFC 9110 14.1 e
 * 14.2). Devolve:
 *    1  um intervalo que dá pra servir, em [*ini, *fim] (os dois inclusos);
 *    0  sem Range, de outra unidade, malformado ou com VÁRIOS intervalos: vale
 *       o arquivo inteiro (a RFC deixa o servidor ignorar o Range; navegador e
 *       player de vídeo pedem um intervalo por vez);
 *   -1  intervalo fora do arquivo: 416. */
static int jk_le_range(const char *v, long long total, long long *ini, long long *fim)
{
    if (!v || total <= 0) return 0;
    while (*v == ' ' || *v == '\t') v++;
    if (strncasecmp(v, "bytes=", 6) != 0) return 0;
    const char *p = v + 6;
    while (*p == ' ' || *p == '\t') p++;
    if (strchr(p, ',')) return 0;
    long long a, b = -1;
    if (*p == '-') {
        /* "bytes=-n": os últimos n bytes; n = 0 não satisfaz (14.1.1) */
        p++;
        long long n;
        if (!jk_le_num(&p, &n)) return 0;
        while (*p == ' ' || *p == '\t') p++;
        if (*p) return 0;
        if (n == 0) return -1;
        *ini = n >= total ? 0 : total - n;
        *fim = total - 1;
        return 1;
    }
    if (!jk_le_num(&p, &a)) return 0;
    if (*p++ != '-') return 0;
    if (*p >= '0' && *p <= '9' && !jk_le_num(&p, &b)) return 0;
    while (*p == ' ' || *p == '\t') p++;
    if (*p) return 0;
    if (b >= 0 && b < a) return 0;          /* inválido: ignora (14.1.1) */
    if (a >= total) return -1;              /* começa depois do fim: 416 */
    *ini = a;
    *fim = (b < 0 || b >= total) ? total - 1 : b;
    return 1;
}

int ps_jk_serve_arquivo(PSJkConn *c, const PSJkReq *r, const char *caminho,
                        const char *ctype, const char *extra, int keep_alive)
{
    FILE *f = fopen(caminho, "rb");
    if (!f) return -2;
    struct stat st;
    if (fstat(fileno(f), &st) != 0 || !S_ISREG(st.st_mode)) { fclose(f); return -2; }
    long long total = (long long)st.st_size;
    long long mtime = (long long)st.st_mtime;

    char data[64], modif[64];
    time_t agora = time(NULL);
    struct tm gm;
    gmtime_r(&agora, &gm);
    strftime(data, sizeof(data), "%a, %d %b %Y %H:%M:%S GMT", &gm);
    time_t tm_arq = (time_t)mtime;
    gmtime_r(&tm_arq, &gm);
    strftime(modif, sizeof(modif), "%a, %d %b %Y %H:%M:%S GMT", &gm);
    /* ETag forte do tamanho e da data, no formato do nginx */
    char etag[64];
    snprintf(etag, sizeof(etag), "\"%llx-%llx\"", (unsigned long long)mtime, (unsigned long long)total);

    int status = 200;
    long long ini = 0, fim = total - 1;
    int get_head = strcmp(r->metodo, "GET") == 0 || strcmp(r->metodo, "HEAD") == 0;
    if (get_head) {
        /* precondições na ordem da RFC 9110 13.2.2 */
        const char *inm = ps_jk_header(r, "If-None-Match");
        const char *ims = ps_jk_header(r, "If-Modified-Since");
        if (inm) {
            if (jk_etag_na_lista(inm, etag)) status = 304;
        } else if (ims) {
            /* ignorado com If-None-Match ou com data inválida (13.1.3) */
            long long t = jk_le_data(ims);
            if (t >= 0 && mtime <= t) status = 304;
        }
        if (status == 200) {
            const char *rg = ps_jk_header(r, "Range");
            const char *ifr = ps_jk_header(r, "If-Range");
            int usa_range = rg != NULL;
            if (rg && ifr) {
                /* If-Range: ETag compara FORTE (W/ nunca casa); data tem que
                 * ser a do Last-Modified, exata. Falso = arquivo inteiro. */
                while (*ifr == ' ' || *ifr == '\t') ifr++;
                if (ifr[0] == '"') usa_range = strncmp(ifr, etag, strlen(etag)) == 0 && ifr[strlen(etag)] == '\0';
                else if (ifr[0] == 'W' && ifr[1] == '/') usa_range = 0;
                else usa_range = jk_le_data(ifr) == mtime;
            }
            if (usa_range) {
                int rr = jk_le_range(rg, total, &ini, &fim);
                if (rr == 1) status = 206;
                else if (rr == -1) status = 416;
            }
        }
    }

    char cab[2048];
    int n;
    const char *fecha = keep_alive ? "" : "Connection: close\r\n";
    const char *mais = extra ? extra : "";
    long long nbytes = 0;
    if (status == 304) {
        /* 304: sem corpo e sem Content-Length/Content-Type (RFC 9110 15.4.5
         * e 8.6); vão Date, ETag e Last-Modified */
        n = snprintf(cab, sizeof(cab),
                     "HTTP/1.1 304 Not Modified\r\nServer: Jinker\r\nDate: %s\r\n"
                     "ETag: %s\r\nLast-Modified: %s\r\n%s%s\r\n",
                     data, etag, modif, fecha, mais);
    } else if (status == 416) {
        /* 416 leva o tamanho atual em "bytes *\/total" (15.5.17) */
        n = snprintf(cab, sizeof(cab),
                     "HTTP/1.1 416 Range Not Satisfiable\r\nServer: Jinker\r\nDate: %s\r\n"
                     "Content-Range: bytes */%lld\r\nContent-Length: 0\r\nAccept-Ranges: bytes\r\n%s%s\r\n",
                     data, total, fecha, mais);
    } else {
        nbytes = status == 206 ? fim - ini + 1 : total;
        char faixa[96] = "";
        if (status == 206)
            snprintf(faixa, sizeof(faixa), "Content-Range: bytes %lld-%lld/%lld\r\n", ini, fim, total);
        n = snprintf(cab, sizeof(cab),
                     "HTTP/1.1 %d %s\r\nServer: Jinker\r\nDate: %s\r\n"
                     "Content-Type: %s\r\nContent-Length: %lld\r\n%s"
                     "Accept-Ranges: bytes\r\nLast-Modified: %s\r\nETag: %s\r\n%s%s\r\n",
                     status, frase(status), data, ctype ? ctype : "application/octet-stream",
                     nbytes, faixa, modif, etag, fecha, mais);
    }
    if (n < 0 || n >= (int)sizeof(cab) || conn_escreve(c, cab, (size_t)n) != 0) { fclose(f); return -1; }
    if (c->sem_corpo || nbytes <= 0) { fclose(f); return 0; }

    /* o corpo em blocos: a memória fica em 64 KB, seja o arquivo do tamanho que for */
    if (ini > 0) {
#ifdef _WIN32
        if (_fseeki64(f, ini, SEEK_SET) != 0) { fclose(f); return -1; }
#else
        if (fseek(f, (long)ini, SEEK_SET) != 0) { fclose(f); return -1; }
#endif
    }
    char *bloco = malloc(JK_BLOCO);
    if (!bloco) { fclose(f); return -1; }
    long long falta = nbytes;
    int rc = 0;
    while (falta > 0) {
        size_t quer = falta > (long long)JK_BLOCO ? JK_BLOCO : (size_t)falta;
        size_t lido = fread(bloco, 1, quer, f);
        /* o arquivo encolheu no meio: o Content-Length já saiu, então a
         * conexão fecha pra o cliente não esperar bytes que não vêm */
        if (lido == 0) { rc = -1; break; }
        if (conn_escreve(c, bloco, lido) != 0) { rc = -1; break; }
        falta -= (long long)lido;
    }
    free(bloco);
    fclose(f);
    return rc;
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
