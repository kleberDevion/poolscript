/*
 * Cliente HTTP/HTTPS. Ver ps_http.h para o contrato.
 *
 * Fluxo: parse da URL, conecta (TLS se https, verificando o certificado),
 * escreve a request, lê status + headers + corpo (Content-Length, chunked ou
 * até fechar), segue redirecionamento 3xx com Location. Sem libcurl.
 */
/* getaddrinfo/struct addrinfo (POSIX) e strcasestr (GNU) só ficam
 * visíveis com esta macro — sem ela um build -std=c11 estrito não
 * enxerga os tipos, embora o gcc em modo GNU (o padrão) compile. */
#define _GNU_SOURCE 1

#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include "ps_http.h"

typedef struct {
    int      fd;
    SSL     *ssl;
    SSL_CTX *ctx;
    char     buf[8192];
    int      nbuf;
    int      expirou;   /* último read falhou por timeout (SO_RCVTIMEO) */
} Conn;

static void conn_fecha(Conn *c)
{
    if (c->ssl) SSL_free(c->ssl);
    if (c->ctx) SSL_CTX_free(c->ctx);
    if (c->fd >= 0) close(c->fd);
    c->ssl = NULL; c->ctx = NULL; c->fd = -1; c->nbuf = 0;
}

#define REDE(r, tipo_, ...) do { \
    (r)->status = -1; \
    snprintf((r)->erro, sizeof((r)->erro), __VA_ARGS__); \
    snprintf((r)->erro_tipo, sizeof((r)->erro_tipo), tipo_); \
    return -1; \
} while (0)

/* scheme://host[:porta][/caminho] — devolve 0 e preenche as partes. */
static int parse_url(const char *url, int *https, char *host, size_t hcap,
                     int *porta, char *caminho, size_t ccap)
{
    if (strncmp(url, "https://", 8) == 0) { *https = 1; url += 8; *porta = 443; }
    else if (strncmp(url, "http://", 7) == 0) { *https = 0; url += 7; *porta = 80; }
    else return -1;

    size_t i = 0;
    while (*url && *url != '/' && *url != ':' && i < hcap - 1) host[i++] = *url++;
    host[i] = '\0';
    if (i == 0) return -1;
    if (*url == ':') {
        url++;
        int p = 0;
        while (*url >= '0' && *url <= '9') p = p * 10 + (*url++ - '0');
        if (p > 0) *porta = p;
    }
    if (*url == '\0') { snprintf(caminho, ccap, "/"); return 0; }
    snprintf(caminho, ccap, "%s", url);
    return 0;
}

static int tcp_conecta(const char *host, int porta, int timeout, PSHttpResp *r)
{
    char pstr[16];
    snprintf(pstr, sizeof(pstr), "%d", porta);
    struct addrinfo dicas, *res = NULL;
    memset(&dicas, 0, sizeof(dicas));
    dicas.ai_family = AF_UNSPEC;
    dicas.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, pstr, &dicas, &res) != 0)
        REDE(r, "NetworkError", "falha de conexão: nome não resolvido: %.100s", host);
    int fd = -1;
    for (struct addrinfo *a = res; a; a = a->ai_next) {
        fd = socket(a->ai_family, a->ai_socktype, a->ai_protocol);
        if (fd < 0) continue;
        if (timeout > 0) {
            struct timeval tv = { timeout, 0 };
            setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        }
        if (connect(fd, a->ai_addr, a->ai_addrlen) == 0) break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);
    if (fd < 0)
        REDE(r, "NetworkError", "falha de conexão: %s", strerror(errno));
    return fd;
}

static int liga_tls(Conn *c, const char *host, PSHttpResp *r)
{
    c->ctx = SSL_CTX_new(TLS_client_method());
    if (!c->ctx) REDE(r, "NetworkError", "falha de conexão: sem contexto TLS");
    /* verifica o certificado — usa as CAs do sistema */
    SSL_CTX_set_default_verify_paths(c->ctx);
    SSL_CTX_set_verify(c->ctx, SSL_VERIFY_PEER, NULL);
    c->ssl = SSL_new(c->ctx);
    if (!c->ssl) REDE(r, "NetworkError", "falha de conexão: sem TLS");
    SSL_set_tlsext_host_name(c->ssl, host);
    /* casa o hostname contra o CN/SAN do certificado */
    SSL_set1_host(c->ssl, host);
    SSL_set_fd(c->ssl, c->fd);
    if (SSL_connect(c->ssl) != 1) {
        /* a frase dizia "certificado inválido" pra TODA falha do handshake —
         * porta de texto puro, versão de TLS, conexão caída. O motivo real
         * vem da pilha de erros do TLS; certificado ruim continua dizendo
         * "certificate verify failed". */
        unsigned long e = ERR_get_error();
        int en = errno;
        const char *motivo = e ? ERR_reason_error_string(e)
                           : en ? strerror(en) : "conexao encerrada pelo servidor";
        REDE(r, "NetworkError", "falha no handshake TLS com %.100s: %s", host,
             motivo ? motivo : "conexao encerrada pelo servidor");
    }
    return 0;
}

static int cru_le(Conn *c, char *out, int cap)
{
    errno = 0;
    int k = c->ssl ? SSL_read(c->ssl, out, cap) : (int)read(c->fd, out, (size_t)cap);
    /* SO_RCVTIMEO estourado sai como EAGAIN — marca pra virar TimeoutError,
     * como no interp (no TLS o SSL_read propaga o errno do fd por baixo).
     *
     * `EAGAIN || EWOULDBLOCK` é o idioma portável, mas no Linux os dois são o
     * MESMO valor e o `-Wlogical-op` acusa "or de expressões iguais" — aviso
     * legítimo, porque a segunda metade nunca é avaliada com resultado
     * diferente. O `#if` mantém a portabilidade sem a redundância. */
    if (k <= 0 && (errno == EAGAIN
#if EWOULDBLOCK != EAGAIN
                   || errno == EWOULDBLOCK
#endif
                  )) c->expirou = 1;
    return k;
}
static int cru_escreve(Conn *c, const char *d, int n)
{
    int feito = 0;
    while (feito < n) {
        int k = c->ssl ? SSL_write(c->ssl, d + feito, n - feito)
                       : (int)write(c->fd, d + feito, (size_t)(n - feito));
        if (k <= 0) return -1;
        feito += k;
    }
    return 0;
}

/* Uma linha sem \r\n. -1 se a conexão fechou antes. */
static int le_linha(Conn *c, char *out, int cap)
{
    int n = 0;
    for (;;) {
        for (int i = 0; i < c->nbuf; i++) {
            if (c->buf[i] != '\n') continue;
            int fim = i;
            if (fim > 0 && c->buf[fim-1] == '\r') fim--;
            if (fim > cap - 1) fim = cap - 1;
            memcpy(out, c->buf, (size_t)fim);
            out[fim] = '\0';
            n = fim;
            memmove(c->buf, c->buf + i + 1, (size_t)(c->nbuf - i - 1));
            c->nbuf -= i + 1;
            return n;
        }
        if (c->nbuf >= (int)sizeof(c->buf)) return -1;
        int k = cru_le(c, c->buf + c->nbuf, (int)sizeof(c->buf) - c->nbuf);
        if (k <= 0) return -1;
        c->nbuf += k;
    }
}

/* Acumula em `buf` (realloc). Devolve 0.
 *
 * Com `destino` != NULL o corpo NÃO é acumulado: cada pedaço vai direto pro
 * arquivo e a memória não cresce. `n` segue contando o total, que é o tamanho
 * baixado. Os três leitores de corpo (Content-Length, chunked e até-fechar)
 * passam por aqui, então o modo de fluxo vale para os três sem uma segunda
 * cópia da lógica de leitura. */
/* `descarta`: o corpo é lido do fio e JOGADO FORA — é o caso do 3xx que vai
 * ser seguido, cujo corpo ninguém quer. Antes ele era acumulado inteiro em
 * memória só pra ser liberado no salto seguinte: um redirecionamento com corpo
 * de gigabytes bufferizava tudo. Contar `n` continua, pra o total bater. */
typedef struct { char *b; size_t n, cap; FILE *destino; int descarta; } Acc;
static int acc_add(Acc *a, const char *d, size_t n, long teto)
{
    if (teto > 0 && a->n + n > (size_t)teto) return -2;   /* passou do limite */
    if (a->descarta) { a->n += n; return 0; }
    if (a->destino) {
        if (n && fwrite(d, 1, n, a->destino) != n) return -1;
        a->n += n;
        return 0;
    }
    if (a->n + n + 1 > a->cap) {
        size_t nc = a->cap < 4096 ? 4096 : a->cap;
        while (nc < a->n + n + 1) nc *= 2;
        char *nn = realloc(a->b, nc);
        if (!nn) return -1;
        a->b = nn; a->cap = nc;
    }
    memcpy(a->b + a->n, d, n);
    a->n += n;
    a->b[a->n] = '\0';
    return 0;
}

/* Lê exatamente `n` bytes do fio pro acumulador. */
static int le_corpo_fixo(Conn *c, Acc *a, size_t n, long teto)
{
    while (n > 0) {
        if (c->nbuf > 0) {
            size_t usa = (size_t)c->nbuf < n ? (size_t)c->nbuf : n;
            int rc = acc_add(a, c->buf, usa, teto);
            if (rc != 0) return rc;
            memmove(c->buf, c->buf + usa, (size_t)c->nbuf - usa);
            c->nbuf -= (int)usa;
            n -= usa;
            continue;
        }
        char tmp[8192];
        int k = cru_le(c, tmp, sizeof(tmp));
        if (k <= 0) return -1;
        size_t usa = (size_t)k < n ? (size_t)k : n;
        int rc = acc_add(a, tmp, usa, teto);
        if (rc != 0) return rc;
        /* sobra volta pro buffer da conexão */
        if ((size_t)k > usa) {
            memcpy(c->buf, tmp + usa, (size_t)k - usa);
            c->nbuf = (int)((size_t)k - usa);
        }
        n -= usa;
    }
    return 0;
}

/* Transfer-Encoding: chunked. */
static int le_corpo_chunked(Conn *c, Acc *a, long teto)
{
    for (;;) {
        char lin[64];
        if (le_linha(c, lin, sizeof(lin)) < 0) return -1;
        long tam = strtol(lin, NULL, 16);        /* tamanho em hexa */
        if (tam <= 0) {                           /* 0 = fim; lê o trailer */
            char t[256];
            while (le_linha(c, t, sizeof(t)) > 0) { /* pula trailers */ }
            return 0;
        }
        int rc = le_corpo_fixo(c, a, (size_t)tam, teto);
        if (rc != 0) return rc;
        char crlf[4];
        le_linha(c, crlf, sizeof(crlf));          /* \r\n após o chunk */
    }
}

/* corpo até a conexão fechar (sem Content-Length nem chunked) */
static int le_corpo_ate_fechar(Conn *c, Acc *a, long teto)
{
    if (c->nbuf > 0) {
        int rc = acc_add(a, c->buf, (size_t)c->nbuf, teto);
        c->nbuf = 0;
        if (rc != 0) return rc;
    }
    char tmp[8192];
    int k;
    while ((k = cru_le(c, tmp, sizeof(tmp))) > 0) {
        int rc = acc_add(a, tmp, (size_t)k, teto);
        if (rc != 0) return rc;
    }
    return 0;
}

static int header_igual(const char *linha, const char *nome)
{
    size_t n = strlen(nome);
    return strncasecmp(linha, nome, n) == 0 && linha[n] == ':';
}
static const char *header_valor(const char *linha)
{
    const char *p = strchr(linha, ':');
    if (!p) return "";
    p++;
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

/* Uma volta do request (sem seguir redirecionamento). `*local` recebe o
 * Location se houver 3xx. */
static int uma_request(const char *metodo, const char *url, const char *cabs,
                       const char *corpo, size_t ncorpo, int timeout, long teto,
                       FILE *destino, PSHttpResp *r, char *local, size_t lcap)
{
    int https, porta;
    char host[256], caminho[2048];
    if (parse_url(url, &https, host, sizeof(host), &porta, caminho, sizeof(caminho)) != 0)
        REDE(r, "NetworkError", "falha de conexão: URL inválida: %.200s", url);

    Conn c = { -1, NULL, NULL, {0}, 0, 0 };
    c.fd = tcp_conecta(host, porta, timeout, r);
    if (c.fd < 0) return -1;
    if (https && liga_tls(&c, host, r) != 0) { conn_fecha(&c); return -1; }

    /* monta a request */
    Acc req = {0};
    char lin[2400];
    int n = snprintf(lin, sizeof(lin), "%s %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n",
                     metodo, caminho, host);
    /* snprintf devolve o tamanho que a linha TERIA, não o que coube: com uma
     * URL de mais de 2350 chars isso mandava pilha nossa pro servidor do outro
     * lado. É o mesmo defeito que vazava a pilha do jinker num 404. */
    if (n < 0 || (size_t)n >= sizeof(lin)) {
        free(req.b);
        conn_fecha(&c);
        REDE(r, "OSError", "URL longa demais para a linha de pedido");
    }
    acc_add(&req, lin, (size_t)n, 0);
    if (cabs) acc_add(&req, cabs, strlen(cabs), 0);   /* já em "Nome: v\r\n" */
    if (corpo && ncorpo > 0) {
        n = snprintf(lin, sizeof(lin), "Content-Length: %zu\r\n", ncorpo);
        acc_add(&req, lin, (size_t)n, 0);
    }
    acc_add(&req, "\r\n", 2, 0);
    if (corpo && ncorpo > 0) acc_add(&req, corpo, ncorpo, 0);

    int rc = cru_escreve(&c, req.b, (int)req.n);
    free(req.b);
    if (rc != 0) { conn_fecha(&c); REDE(r, "NetworkError", "falha de conexão: escrita falhou"); }

    /* status line */
    char status_line[1024];
    if (le_linha(&c, status_line, sizeof(status_line)) < 0) {
        int exp = c.expirou;
        conn_fecha(&c);
        /* espirra o mesmo tipo/mensagem do interp: read que estoura o
         * SO_RCVTIMEO é TimeoutError, não falha de conexão */
        if (exp)
            REDE(r, "TimeoutError",
                 "operação expirou: requisição passou de %ds (url=%.200s)", timeout, url);
        REDE(r, "NetworkError", "falha de conexão: sem resposta");
    }
    const char *sp = strchr(status_line, ' ');
    r->status = sp ? atol(sp + 1) : 0;

    /* headers */
    Acc hbloco = {0};
    long clen = -1;
    int chunked = 0;
    local[0] = '\0';
    for (;;) {
        int k = le_linha(&c, lin, sizeof(lin));
        if (k <= 0) break;                       /* linha vazia = fim dos headers */
        acc_add(&hbloco, lin, strlen(lin), 0);
        acc_add(&hbloco, "\n", 1, 0);
        if (header_igual(lin, "Content-Length")) clen = atol(header_valor(lin));
        else if (header_igual(lin, "Transfer-Encoding")
                 && strcasestr(lin, "chunked")) chunked = 1;
        else if (header_igual(lin, "Location"))
            snprintf(local, lcap, "%s", header_valor(lin));
    }
    if (c.expirou) {
        free(hbloco.b);
        conn_fecha(&c);
        REDE(r, "TimeoutError",
             "operação expirou: requisição passou de %ds (url=%.200s)", timeout, url);
    }
    r->headers = hbloco.b ? hbloco.b : strdup("");

    /* corpo — HEAD e 204/304 não têm */
    Acc corpo_acc = {0};
    /* O corpo de um 3xx que vamos SEGUIR não é o download: ele iria pro
     * arquivo antes do conteúdo de verdade. Só a resposta final escreve. */
    if (r->status >= 300 && r->status < 400 && local[0]) corpo_acc.descarta = 1;
    else                                                   corpo_acc.destino  = destino;
    int sem_corpo = (strcmp(metodo, "HEAD") == 0) || r->status == 204 || r->status == 304;
    if (!sem_corpo) {
        int br;
        if (chunked)       br = le_corpo_chunked(&c, &corpo_acc, teto);
        else if (clen >= 0) br = le_corpo_fixo(&c, &corpo_acc, (size_t)clen, teto);
        else                br = le_corpo_ate_fechar(&c, &corpo_acc, teto);
        if (br == -2) {
            free(corpo_acc.b);
            conn_fecha(&c);
            r->status = -1;
            snprintf(r->erro, sizeof(r->erro), "download passou do limite de %ld bytes", teto);
            snprintf(r->erro_tipo, sizeof(r->erro_tipo), "MemoryError");
            return -1;
        }
        /* timeout no meio do corpo também é TimeoutError (o interp estoura
         * no resp.read()); conexão derrubada sem timeout segue tolerada */
        if (c.expirou) {
            free(corpo_acc.b);
            conn_fecha(&c);
            REDE(r, "TimeoutError",
                 "operação expirou: requisição passou de %ds (url=%.200s)", timeout, url);
        }
    }
    conn_fecha(&c);
    /* `ncorpo` descreve o que está EM `corpo`, sempre. Em modo de fluxo o
     * corpo foi pro arquivo e não há buffer nenhum: `ncorpo` é 0, e o total
     * que desceu vai em `nbaixado`.
     *
     * Isto já foi um SIGSEGV: `ncorpo` recebia `corpo_acc.n` mesmo em fluxo,
     * então a struct dizia ter 64 MB num buffer de 1 byte, e o consumidor
     * copiava os 64 MB. Com corpo pequeno não havia sinal — o `.content`
     * voltava com heap do próprio processo. */
    r->corpo = corpo_acc.b ? corpo_acc.b : strdup("");
    r->ncorpo = corpo_acc.b ? corpo_acc.n : 0;
    r->nbaixado = corpo_acc.n;
    r->url_final = strdup(url);
    return 0;
}

int ps_http_request(const char *metodo, const char *url, const char *cabs,
                    const char *corpo, size_t ncorpo, int timeout,
                    long teto, PSHttpResp *r)
{
    return ps_http_baixa(metodo, url, cabs, corpo, ncorpo, timeout, teto, NULL, r);
}

/* Igual ao `ps_http_request`, com um DESTINO: cada pedaço do corpo vai direto
 * pro arquivo e nada se acumula. É o que permite baixar um arquivo maior que a
 * memória — sem isto o corpo inteiro vira `char*`, e depois vira string da
 * linguagem, então um download de 237 MB custava 480 MB de RSS (medido). */
int ps_http_baixa(const char *metodo, const char *url, const char *cabs,
                  const char *corpo, size_t ncorpo, int timeout,
                  long teto, FILE *destino, PSHttpResp *r)
{
    memset(r, 0, sizeof(*r));
    char atual[4096];
    snprintf(atual, sizeof(atual), "%s", url);
    for (int salto = 0; salto < 10; salto++) {
        char local[2048];
        if (uma_request(metodo, atual, cabs, corpo, ncorpo, timeout, teto, destino, r, local, sizeof(local)) != 0)
            return -1;
        /* segue 3xx com Location. 303 (e 301/302 em POST) viram GET sem
         * corpo; 307/308 preservam o método. */
        if (r->status >= 300 && r->status < 400 && local[0]) {
            char prox[4096];
            if (strncmp(local, "http://", 7) == 0 || strncmp(local, "https://", 8) == 0) {
                snprintf(prox, sizeof(prox), "%s", local);
            } else {
                /* Location relativo: reaproveita scheme+host+PORTA do atual —
                 * esquecer a porta mandava o redirect pra 80/443 e dava
                 * "connection refused" contra servidor em porta alta */
                int https, porta;
                char host[256], cam[2048], hostp[280];
                parse_url(atual, &https, host, sizeof(host), &porta, cam, sizeof(cam));
                if ((https && porta == 443) || (!https && porta == 80))
                    snprintf(hostp, sizeof(hostp), "%s", host);
                else
                    snprintf(hostp, sizeof(hostp), "%s:%d", host, porta);
                snprintf(prox, sizeof(prox), "%s://%s%s%.1900s", https ? "https" : "http",
                         hostp, local[0] == '/' ? "" : "/", local);
            }
            int vira_get = (r->status == 303)
                        || ((r->status == 301 || r->status == 302) && strcmp(metodo, "GET") != 0
                            && strcmp(metodo, "HEAD") != 0);
            ps_http_resp_solta(r);
            memset(r, 0, sizeof(*r));
            snprintf(atual, sizeof(atual), "%s", prox);
            if (vira_get) { metodo = "GET"; corpo = NULL; ncorpo = 0; }
            continue;
        }
        return 0;
    }
    REDE(r, "NetworkError", "falha de conexão: redirecionamentos demais");
}

void ps_http_resp_solta(PSHttpResp *r)
{
    free(r->url_final);
    free(r->headers);
    free(r->corpo);
    r->url_final = r->headers = r->corpo = NULL;
}
