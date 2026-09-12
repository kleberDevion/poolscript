/*
 * SMTP + IMAP + MIME. Ver ps_mail.h para o contrato.
 *
 * A conexão é uma só (`PSMailConn`) para os dois protocolos: socket + SSL
 * opcional + um buffer de leitura por linhas. O que muda é a conversa em
 * cima — SMTP responde com código numérico, IMAP com tag.
 */
/* getaddrinfo/struct addrinfo (POSIX) e strcasestr (GNU) só ficam
 * visíveis com esta macro — sem ela um build -std=c11 estrito não
 * enxerga os tipos, embora o gcc em modo GNU (o padrão) compile. */
#define _GNU_SOURCE 1

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "ps_hash.h"
#include "ps_mail.h"

struct PSMailConn {
    int      fd;
    SSL     *ssl;       /* NULL antes do STARTTLS */
    SSL_CTX *ctx;
    char     buf[8192]; /* sobra de leitura entre linhas */
    int      nbuf;
    int      tag;       /* contador de tag do IMAP (A1, A2, ...) */
    char     auth[256]; /* mecanismos AUTH anunciados no EHLO */
};

#define FALHA(erro, cap, ...) do { \
    if (erro) snprintf(erro, cap, __VA_ARGS__); \
    return -1; \
} while (0)

/* ── transporte ─────────────────────────────────────────────────────────── */

static int tcp_conecta(const char *host, int porta, char *erro, size_t cap)
{
    char pstr[16];
    snprintf(pstr, sizeof(pstr), "%d", porta);
    struct addrinfo dicas, *res = NULL;
    memset(&dicas, 0, sizeof(dicas));
    dicas.ai_family = AF_UNSPEC;
    dicas.ai_socktype = SOCK_STREAM;
    int rc = getaddrinfo(host, pstr, &dicas, &res);
    if (rc != 0) {
        /* o motivo REAL do getaddrinfo: dizia sempre "-2 Name or service not
         * known", também pra DNS fora do ar (EAI_AGAIN) e afins */
        if (erro) snprintf(erro, cap, "[Errno %d] %s", rc, gai_strerror(rc));
        return -1;
    }
    int fd = -1;
    for (struct addrinfo *a = res; a; a = a->ai_next) {
        fd = socket(a->ai_family, a->ai_socktype, a->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, a->ai_addr, a->ai_addrlen) == 0) break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);
    if (fd < 0) {
        if (erro) snprintf(erro, cap, "[Errno %d] %s", errno, strerror(errno));
        return -1;
    }
    return fd;
}

static int cru_le(PSMailConn *c, char *out, int cap)
{
    if (c->ssl) return SSL_read(c->ssl, out, cap);
    return (int)read(c->fd, out, (size_t)cap);
}

static int cru_escreve(PSMailConn *c, const char *dados, int n)
{
    int feito = 0;
    while (feito < n) {
        int k = c->ssl ? SSL_write(c->ssl, dados + feito, n - feito)
                       : (int)write(c->fd, dados + feito, (size_t)(n - feito));
        if (k <= 0) return -1;
        feito += k;
    }
    return 0;
}

/* Uma linha, sem o \r\n. Devolve o tamanho ou -1. */
static int le_linha(PSMailConn *c, char *out, int cap)
{
    int n = 0;
    for (;;) {
        for (int i = 0; i < c->nbuf; i++) {
            if (c->buf[i] != '\n') continue;
            int fim = i;
            if (fim > 0 && c->buf[fim - 1] == '\r') fim--;
            if (fim > cap - 1) fim = cap - 1;
            memcpy(out, c->buf, (size_t)fim);
            out[fim] = '\0';
            n = fim;
            memmove(c->buf, c->buf + i + 1, (size_t)(c->nbuf - i - 1));
            c->nbuf -= i + 1;
            return n;
        }
        if (c->nbuf >= (int)sizeof(c->buf)) return -1;   /* linha gigante */
        int k = cru_le(c, c->buf + c->nbuf, (int)sizeof(c->buf) - c->nbuf);
        if (k <= 0) return -1;
        c->nbuf += k;
    }
}

/* Exatamente `n` bytes (o literal {n} do IMAP). */
static int le_bytes(PSMailConn *c, char *out, size_t n)
{
    size_t feito = 0;
    while (feito < n) {
        if (c->nbuf > 0) {
            size_t usa = (size_t)c->nbuf < n - feito ? (size_t)c->nbuf : n - feito;
            memcpy(out + feito, c->buf, usa);
            memmove(c->buf, c->buf + usa, (size_t)c->nbuf - usa);
            c->nbuf -= (int)usa;
            feito += usa;
            continue;
        }
        int k = cru_le(c, out + feito, (int)(n - feito));
        if (k <= 0) return -1;
        feito += (size_t)k;
    }
    return 0;
}

/* 1 = pular a verificação do certificado, ligado por `PS_MAIL_TLS_INSEGURO=1`.
 *
 * Existe pro servidor de teste com certificado autoassinado, e é a ÚNICA forma
 * de desligar: variável de ambiente explícita, não o padrão. */
static int mail_tls_inseguro(void)
{
    const char *v = getenv("PS_MAIL_TLS_INSEGURO");
    return v && v[0] == '1';
}

static int liga_tls(PSMailConn *c, const char *host, char *erro, size_t cap)
{
    c->ctx = SSL_CTX_new(TLS_client_method());
    if (!c->ctx) FALHA(erro, cap, "sem memoria para o TLS");

    /* VERIFICA a cadeia E o hostname. Estava `SSL_VERIFY_NONE`.
     *
     * Não é detalhe de conformidade. Logo abaixo, `ps_smtp_login` manda
     * `AUTH PLAIN`/`AUTH LOGIN` (usuário e senha em base64) e `ps_imap_login`
     * manda `LOGIN user senha`. Sem verificação, qualquer intermediário no
     * caminho apresenta o certificado dele, o handshake "funciona", e recebe
     * as credenciais. O cliente HTTP deste mesmo projeto já fazia o certo
     * (`ps_http.c`: `SSL_CTX_set_verify` + `SSL_set1_host`) — faltava aplicar
     * aqui. */
    if (mail_tls_inseguro()) {
        SSL_CTX_set_verify(c->ctx, SSL_VERIFY_NONE, NULL);
    } else {
        SSL_CTX_set_default_verify_paths(c->ctx);      /* CAs do sistema */
        SSL_CTX_set_verify(c->ctx, SSL_VERIFY_PEER, NULL);
    }

    c->ssl = SSL_new(c->ctx);
    if (!c->ssl) FALHA(erro, cap, "sem memoria para o TLS");
    SSL_set_tlsext_host_name(c->ssl, host);
    /* casa o hostname contra o CN/SAN — sem isto, certificado VÁLIDO de outro
     * domínio passa, que é metade do ataque */
    if (!mail_tls_inseguro()) SSL_set1_host(c->ssl, host);
    SSL_set_fd(c->ssl, c->fd);
    if (SSL_connect(c->ssl) != 1) {
        /* Só é "certificado invalido" quando a VERIFICAÇÃO reprovou; porta de
         * texto puro, versão de TLS ou conexão caída diziam a mesma frase e
         * mandavam o usuário pular a verificação de um certificado que nem
         * chegou a ser visto. */
        unsigned long e = ERR_get_error();
        int en = errno;
        const char *motivo = e ? ERR_reason_error_string(e)
                           : en ? strerror(en) : "conexao encerrada";
        if (SSL_get_verify_result(c->ssl) != X509_V_OK)
            FALHA(erro, cap, "certificado TLS invalido para %s "
                             "(self-signed? PS_MAIL_TLS_INSEGURO=1 pula a verificacao)", host);
        FALHA(erro, cap, "o servidor em %s nao completou o handshake TLS (porta de texto puro?): %s",
              host, motivo ? motivo : "conexao encerrada");
    }
    return 0;
}

static PSMailConn *conn_nova(int fd)
{
    PSMailConn *c = calloc(1, sizeof(PSMailConn));
    if (!c) { close(fd); return NULL; }
    c->fd = fd;
    return c;
}

void ps_mail_solta(PSMailConn *c)
{
    if (!c) return;
    if (c->ssl) SSL_free(c->ssl);
    if (c->ctx) SSL_CTX_free(c->ctx);
    if (c->fd >= 0) close(c->fd);
    free(c);
}

/* ── SMTP ───────────────────────────────────────────────────────────────── */

/* Lê a resposta inteira (linhas `250-x` de continuação) e devolve o código.
 * `extras`, se dado, acumula o texto das linhas — é onde o EHLO anuncia os
 * mecanismos de AUTH. */
static int smtp_resposta(PSMailConn *c, char *extras, size_t cap_extras)
{
    char lin[1024];
    if (extras) extras[0] = '\0';
    for (;;) {
        int n = le_linha(c, lin, sizeof(lin));
        if (n < 3) return -1;
        if (extras) {
            size_t j = strlen(extras);
            snprintf(extras + j, cap_extras - j, "%s\n", lin + (n > 4 ? 4 : n));
        }
        if (lin[3] != '-') return atoi(lin);
    }
}

static int smtp_manda(PSMailConn *c, const char *cmd)
{
    char lin[1100];
    int n = snprintf(lin, sizeof(lin), "%s\r\n", cmd);
    /* o retorno é o tamanho que TERIA: mandar `n` bytes de um lin[1100]
     * truncado põe pilha na conexão SMTP */
    if (n < 0 || (size_t)n >= sizeof(lin)) return -1;
    return cru_escreve(c, lin, n);
}

PSMailConn *ps_smtp_conecta(const char *host, int porta, char *erro, size_t cap)
{
    int fd = tcp_conecta(host, porta, erro, cap);
    if (fd < 0) return NULL;
    PSMailConn *c = conn_nova(fd);
    if (!c) { if (erro) snprintf(erro, cap, "sem memoria"); return NULL; }

    char extras[1024];
    if (smtp_resposta(c, NULL, 0) != 220) {
        if (erro) snprintf(erro, cap, "servidor nao respondeu 220 no greeting");
        goto falha;
    }
    if (smtp_manda(c, "EHLO poolscript.local") != 0
            || smtp_resposta(c, extras, sizeof(extras)) != 250) {
        if (erro) snprintf(erro, cap, "EHLO recusado");
        goto falha;
    }
    /* Três falhas diferentes davam a mesma frase ("extension not supported"):
     * o servidor não anunciar STARTTLS, a escrita cair, e o servidor recusar
     * o comando. Cada uma diz o que houve. */
    if (!strstr(extras, "STARTTLS")) {
        if (erro) snprintf(erro, cap, "o servidor em %s nao anunciou STARTTLS no EHLO "
                                      "(porta de texto puro sem TLS?)", host);
        goto falha;
    }
    if (smtp_manda(c, "STARTTLS") != 0) {
        if (erro) snprintf(erro, cap, "a conexao com %s caiu ao mandar STARTTLS", host);
        goto falha;
    }
    {
        char resp[512];
        int cod = smtp_resposta(c, resp, sizeof(resp));
        if (cod != 220) {
            if (erro) snprintf(erro, cap, cod < 0
                               ? "a conexao com %s caiu depois do STARTTLS%s"
                               : "o servidor em %s recusou STARTTLS: %s",
                               host, cod < 0 ? "" : resp);
            goto falha;
        }
    }
    c->nbuf = 0;                       /* nada legível atravessa o handshake */
    if (liga_tls(c, host, erro, cap) != 0) goto falha;
    if (smtp_manda(c, "EHLO poolscript.local") != 0
            || smtp_resposta(c, extras, sizeof(extras)) != 250) {
        if (erro) snprintf(erro, cap, "EHLO recusado depois do TLS");
        goto falha;
    }
    /* guarda a linha AUTH pro login escolher o mecanismo */
    c->auth[0] = '\0';
    for (char *p = extras; *p; ) {
        char *fim = strchr(p, '\n');
        size_t tam = fim ? (size_t)(fim - p) : strlen(p);
        if (tam > 5 && strncmp(p, "AUTH ", 5) == 0 && tam < sizeof(c->auth)) {
            memcpy(c->auth, p, tam);
            c->auth[tam] = '\0';
        }
        if (!fim) break;
        p = fim + 1;
    }
    return c;
falha:
    ps_mail_solta(c);
    return NULL;
}

int ps_smtp_login(PSMailConn *c, const char *user, const char *senha,
                  char *erro, size_t cap)
{
    /* PLAIN quando anunciado, senão LOGIN — a ordem do smtplib sem CRAM. */
    int plain = strstr(c->auth, "PLAIN") != NULL || c->auth[0] == '\0';
    char cru[512], b64[1024], cmd[1100];
    /* o snprintf TRUNCA em cru[512] mas DEVOLVE o tamanho que a string TERIA
     * (C99 7.21.6.5). Usar esse retorno fazia o base64 ler fora de `cru` e
     * escrever fora de `b64`: com usuario de 4000 chars, stack smashing;
     * com 1500, a pilha adjacente saia codificada pro servidor. */
    size_t nu = strlen(user), ns = strlen(senha);
    if (nu + ns + 2 > sizeof(cru) || ((nu + ns + 4) / 3) * 4 + 1 > sizeof(b64))
        FALHA(erro, cap, "usuario ou senha longos demais para o AUTH");
    if (plain) {
        int n = snprintf(cru, sizeof(cru), "%c%s%c%s", 0, user, 0, senha);
        size_t nb = ps_base64_encode((const unsigned char *)cru, (size_t)n, b64);
        snprintf(cmd, sizeof(cmd), "AUTH PLAIN %.*s", (int)nb, b64);
        if (smtp_manda(c, cmd) != 0 || smtp_resposta(c, NULL, 0) != 235)
            FALHA(erro, cap, "usuario ou senha recusados pelo servidor");
        return 0;
    }
    if (smtp_manda(c, "AUTH LOGIN") != 0 || smtp_resposta(c, NULL, 0) != 334)
        FALHA(erro, cap, "servidor nao aceitou AUTH LOGIN");
    size_t nb = ps_base64_encode((const unsigned char *)user, strlen(user), b64);
    b64[nb] = '\0';
    if (smtp_manda(c, b64) != 0 || smtp_resposta(c, NULL, 0) != 334)
        FALHA(erro, cap, "usuario recusado pelo servidor");
    nb = ps_base64_encode((const unsigned char *)senha, strlen(senha), b64);
    b64[nb] = '\0';
    if (smtp_manda(c, b64) != 0 || smtp_resposta(c, NULL, 0) != 235)
        FALHA(erro, cap, "usuario ou senha recusados pelo servidor");
    return 0;
}

/* Byte de controle no endereço vira COMANDO SMTP novo: um `to()` com \r\n no
 * meio saia como dois `RCPT TO:` — destinatário oculto plantado por quem
 * controla o endereço. Some com eles. */
static void tira_controle(char *s)
{
    size_t j = 0;
    for (size_t i = 0; s[i]; i++)
        if ((unsigned char)s[i] >= 0x20 && (unsigned char)s[i] != 0x7f) s[j++] = s[i];
    s[j] = '\0';
}

/* "Nome <a@b>" → a@b. Sem <>, o próprio texto sem espaços das pontas. */
static void extrai_endereco(const char *txt, size_t n, char *out, size_t cap)
{
    const char *abre = memchr(txt, '<', n);
    if (abre) {
        const char *fecha = memchr(abre, '>', n - (size_t)(abre - txt));
        if (fecha && fecha > abre + 1) {
            size_t tam = (size_t)(fecha - abre - 1);
            if (tam > cap - 1) tam = cap - 1;
            memcpy(out, abre + 1, tam);
            out[tam] = '\0';
            tira_controle(out);
            return;
        }
    }
    while (n > 0 && (*txt == ' ' || *txt == '\t')) { txt++; n--; }
    while (n > 0 && (txt[n-1] == ' ' || txt[n-1] == '\t')) n--;
    if (n > cap - 1) n = cap - 1;
    memcpy(out, txt, n);
    out[n] = '\0';
    tira_controle(out);
}

int ps_smtp_envia(PSMailConn *c, const char *de, const char *para,
                  const char *msg, size_t n, char *erro, size_t cap)
{
    char addr[256], cmd[300];
    extrai_endereco(de, strlen(de), addr, sizeof(addr));
    snprintf(cmd, sizeof(cmd), "MAIL FROM:<%s>", addr);
    if (smtp_manda(c, cmd) != 0 || smtp_resposta(c, NULL, 0) != 250)
        FALHA(erro, cap, "MAIL FROM recusado");

    /* um RCPT por endereço — `to("a@x, b@y")` vale pros dois */
    const char *p = para;
    while (*p) {
        const char *virgula = strchr(p, ',');
        size_t tam = virgula ? (size_t)(virgula - p) : strlen(p);
        extrai_endereco(p, tam, addr, sizeof(addr));
        if (addr[0]) {
            snprintf(cmd, sizeof(cmd), "RCPT TO:<%s>", addr);
            if (smtp_manda(c, cmd) != 0) FALHA(erro, cap, "RCPT TO falhou");
            int r = smtp_resposta(c, NULL, 0);
            if (r != 250 && r != 251) FALHA(erro, cap, "destinatario recusado: %s", addr);
        }
        if (!virgula) break;
        p = virgula + 1;
    }

    if (smtp_manda(c, "DATA") != 0 || smtp_resposta(c, NULL, 0) != 354)
        FALHA(erro, cap, "DATA recusado");

    /* \n vira \r\n e linha começando com '.' ganha outro '.' na frente */
    char *fio = malloc(n * 2 + 8);
    if (!fio) FALHA(erro, cap, "sem memoria");
    size_t j = 0;
    int inicio_linha = 1;
    for (size_t i = 0; i < n; i++) {
        char ch = msg[i];
        if (inicio_linha && ch == '.') fio[j++] = '.';
        inicio_linha = 0;
        if (ch == '\n') {
            if (j == 0 || fio[j-1] != '\r') fio[j++] = '\r';
            fio[j++] = '\n';
            inicio_linha = 1;
            continue;
        }
        fio[j++] = ch;
    }
    if (j < 2 || fio[j-2] != '\r' || fio[j-1] != '\n') { fio[j++] = '\r'; fio[j++] = '\n'; }
    memcpy(fio + j, ".\r\n", 3);
    j += 3;
    int rc = cru_escreve(c, fio, (int)j);
    free(fio);
    if (rc != 0 || smtp_resposta(c, NULL, 0) != 250)
        FALHA(erro, cap, "servidor recusou a mensagem");
    return 0;
}

void ps_smtp_quit(PSMailConn *c)
{
    if (!c) return;
    char lin[256];
    if (smtp_manda(c, "QUIT") == 0) le_linha(c, lin, sizeof(lin));
    ps_mail_solta(c);
}

/* ── IMAP ───────────────────────────────────────────────────────────────── */

PSMailConn *ps_imap_conecta(const char *host, int porta, char *erro, size_t cap)
{
    int fd = tcp_conecta(host, porta, erro, cap);
    if (fd < 0) return NULL;
    PSMailConn *c = conn_nova(fd);
    if (!c) { if (erro) snprintf(erro, cap, "sem memoria"); return NULL; }
    if (liga_tls(c, host, erro, cap) != 0) { ps_mail_solta(c); return NULL; }
    char lin[1024];
    if (le_linha(c, lin, sizeof(lin)) < 4 || strncmp(lin, "* OK", 4) != 0) {
        if (erro) snprintf(erro, cap, "servidor IMAP nao deu boas-vindas");
        ps_mail_solta(c);
        return NULL;
    }
    return c;
}

/* Manda `TAGn <cmd>` e consome até a linha `TAGn OK/NO/BAD`. Linhas `*` de
 * dados passam pelo callback (que pode ser NULL). */
typedef void (*ImapDados)(const char *linha, void *ctx, PSMailConn *c);

/* Lê uma linha de tamanho ARBITRÁRIO num buffer que cresce.
 *
 * O `le_linha` de buffer fixo devolvia -1 quando a linha não cabia, e quem
 * chamava reportava "conexao IMAP caiu" — mentira: a conexão estava boa, a
 * LINHA é que era grande. Um `SEARCH ALL` numa caixa real devolve milhares de
 * ids numa linha só, dezenas de kB, então o `.search("ALL")` simplesmente não
 * funcionava em caixa de verdade.
 *
 * Devolve o tamanho, ou -1 se a conexão morreu de fato. */
static int le_linha_din(PSMailConn *c, char **out, size_t *cap)
{
    size_t n = 0;
    for (;;) {
        for (int i = 0; i < c->nbuf; i++) {
            if (c->buf[i] != '\n') continue;
            size_t fim = (size_t)i;
            if (fim > 0 && c->buf[fim - 1] == '\r') fim--;
            if (n + fim + 1 > *cap) {
                size_t novo = (n + fim + 1) * 2;
                char *nb = realloc(*out, novo);
                if (!nb) return -1;
                *out = nb; *cap = novo;
            }
            memcpy(*out + n, c->buf, fim);
            n += fim;
            (*out)[n] = '\0';
            memmove(c->buf, c->buf + i + 1, (size_t)(c->nbuf - i - 1));
            c->nbuf -= i + 1;
            return (int)n;
        }
        /* sem '\n' no que chegou: ESCOA o buffer pro destino e continua lendo,
         * em vez de desistir por "linha gigante" */
        if (c->nbuf > 0) {
            if (n + (size_t)c->nbuf + 1 > *cap) {
                size_t novo = (n + (size_t)c->nbuf + 1) * 2;
                char *nb = realloc(*out, novo);
                if (!nb) return -1;
                *out = nb; *cap = novo;
            }
            memcpy(*out + n, c->buf, (size_t)c->nbuf);
            n += (size_t)c->nbuf;
            c->nbuf = 0;
        }
        int k = cru_le(c, c->buf, (int)sizeof(c->buf));
        if (k <= 0) return -1;
        c->nbuf = k;
    }
}

static int imap_cmd(PSMailConn *c, const char *cmd, ImapDados fn, void *ctx,
                    char *erro, size_t cap)
{
    /* CR/LF no argumento vira OUTRO comando pro servidor: um `select(pasta)`
     * ou `body(id)` com \r\n no meio virava DELETE/CREATE na caixa. O
     * imap_aspas cuidava de `"` e `\\` e ignorava justo esses dois bytes. */
    if (strpbrk(cmd, "\r\n"))
        FALHA(erro, cap, "comando IMAP com CR ou LF no argumento");
    char tag[16];
    snprintf(tag, sizeof(tag), "A%d", ++c->tag);
    char lin[2100];
    int n = snprintf(lin, sizeof(lin), "%s %s\r\n", tag, cmd);
    if (n < 0 || (size_t)n >= sizeof(lin)) FALHA(erro, cap, "comando IMAP longo demais");
    if (cru_escreve(c, lin, n) != 0) FALHA(erro, cap, "conexao IMAP caiu");
    size_t ntag = strlen(tag);
    size_t cap_l = 4096;
    char *l = malloc(cap_l);
    if (!l) FALHA(erro, cap, "sem memoria");
    for (;;) {
        int k = le_linha_din(c, &l, &cap_l);
        if (k < 0) { free(l); FALHA(erro, cap, "conexao IMAP caiu"); }
        if ((size_t)k > ntag && strncmp(l, tag, ntag) == 0 && l[ntag] == ' ') {
            if (strncmp(l + ntag + 1, "OK", 2) == 0) { free(l); return 0; }
            char msg[220];
            snprintf(msg, sizeof(msg), "%.200s", l + ntag + 1);
            free(l);
            FALHA(erro, cap, "%s", msg);
        }
        if (l[0] == '*' && fn) fn(l, ctx, c);
    }
}

/* String IMAP entre aspas, com \ e " escapados — o formato do _imap_quote. */
static void imap_aspas(const char *txt, char *out, size_t cap)
{
    size_t j = 0;
    if (j < cap - 1) out[j++] = '"';
    for (const char *p = txt; *p && j + 3 < cap; p++) {
        if (*p == '\\' || *p == '"') out[j++] = '\\';
        out[j++] = *p;
    }
    if (j < cap - 1) out[j++] = '"';
    out[j] = '\0';
}

int ps_imap_login(PSMailConn *c, const char *user, const char *senha,
                  char *erro, size_t cap)
{
    char u[300], s[300], cmd[700];
    imap_aspas(user, u, sizeof(u));
    imap_aspas(senha, s, sizeof(s));
    snprintf(cmd, sizeof(cmd), "LOGIN %s %s", u, s);
    return imap_cmd(c, cmd, NULL, NULL, erro, cap);
}

int ps_imap_select(PSMailConn *c, const char *pasta, int readonly,
                   char *erro, size_t cap)
{
    char p[300], cmd[350];
    imap_aspas(pasta, p, sizeof(p));
    snprintf(cmd, sizeof(cmd), "%s %s", readonly ? "EXAMINE" : "SELECT", p);
    return imap_cmd(c, cmd, NULL, NULL, erro, cap);
}

static void pega_search(const char *linha, void *ctx, PSMailConn *c)
{
    (void)c;
    char **ids = ctx;
    if (strncmp(linha, "* SEARCH", 8) != 0) return;
    const char *resto = linha + 8;
    while (*resto == ' ') resto++;
    free(*ids);
    *ids = strdup(resto);
}

/* SEARCH com termo em literal. Fluxo do IMAP:
 *
 *   C: A1 SEARCH CHARSET UTF-8 SUBJECT {12}
 *   S: + ready
 *   C: relatório<CRLF>
 *   S: * SEARCH 3 7 12
 *   S: A1 OK
 *
 * A continuação `+` é o que separa isto de um comando comum, por isso não dá
 * pra reusar o `imap_cmd`. */
static int imap_cmd_literal(PSMailConn *c, const char *criterio, const char *termo,
                            char **ids, char *erro, size_t cap)
{
    char tag[16];
    snprintf(tag, sizeof(tag), "A%d", ++c->tag);
    size_t nt = strlen(termo);
    char lin[900];
    if (strpbrk(criterio, "\r\n"))
        FALHA(erro, cap, "criterio de busca com CR ou LF");
    int n = snprintf(lin, sizeof(lin), "%s SEARCH CHARSET UTF-8 %s {%zu}\r\n",
                     tag, criterio, nt);
    /* snprintf devolve o tamanho que TERIA: mandar `n` bytes de um lin[900]
     * truncado punha 2 KB de pilha (ponteiros crus) na conexao */
    if (n < 0 || (size_t)n >= sizeof(lin))
        FALHA(erro, cap, "criterio de busca longo demais");
    if (cru_escreve(c, lin, n) != 0) FALHA(erro, cap, "conexao IMAP caiu");

    size_t cap_l = 4096;
    char *l = malloc(cap_l);
    if (!l) FALHA(erro, cap, "sem memoria");
    int k = le_linha_din(c, &l, &cap_l);
    if (k < 0 || l[0] != '+') {
        char msg[220];
        snprintf(msg, sizeof(msg), "%.200s", k < 0 ? "conexao IMAP caiu" : l);
        free(l);
        FALHA(erro, cap, "%s", msg);
    }
    if (cru_escreve(c, termo, (int)nt) != 0 || cru_escreve(c, "\r\n", 2) != 0) {
        free(l);
        FALHA(erro, cap, "conexao IMAP caiu");
    }
    size_t ntag = strlen(tag);
    for (;;) {
        k = le_linha_din(c, &l, &cap_l);
        if (k < 0) { free(l); FALHA(erro, cap, "conexao IMAP caiu"); }
        if ((size_t)k > ntag && strncmp(l, tag, ntag) == 0 && l[ntag] == ' ') {
            int ok = strncmp(l + ntag + 1, "OK", 2) == 0;
            char msg[220];
            snprintf(msg, sizeof(msg), "%.200s", l + ntag + 1);
            free(l);
            if (ok) return 0;
            FALHA(erro, cap, "%s", msg);
        }
        if (l[0] == '*') pega_search(l, ids, c);
    }
}

/* Tem byte fora do ASCII? Então o termo NÃO pode ir entre aspas: o
 * quoted-string do IMAP é 7-bit (RFC 3501) e o servidor responde
 * "BAD Could not parse command". O jeito certo é literal `{n}`. */
static int tem_nao_ascii(const char *s)
{
    for (const unsigned char *p = (const unsigned char *)s; *p; p++)
        if (*p >= 0x80) return 1;
    return 0;
}

int ps_imap_search(PSMailConn *c, const char *criterio, const char *termo,
                   char **ids, char *erro, size_t cap)
{
    char cmd[1200];
    if (termo && tem_nao_ascii(termo)) {
        /* `SEARCH CHARSET UTF-8 SUBJECT {12}` -> servidor responde `+` ->
         * mandamos os bytes crus. É o único jeito de buscar "relatório". */
        *ids = strdup("");
        if (!*ids) FALHA(erro, cap, "sem memoria");
        if (imap_cmd_literal(c, criterio, termo, ids, erro, cap) != 0) {
            free(*ids); *ids = NULL; return -1;
        }
        return 0;
    }
    if (termo) {
        char t[600];
        imap_aspas(termo, t, sizeof(t));
        snprintf(cmd, sizeof(cmd), "SEARCH CHARSET UTF-8 %s %s", criterio, t);
    } else {
        snprintf(cmd, sizeof(cmd), "SEARCH %s", criterio);
    }
    *ids = strdup("");
    if (!*ids) FALHA(erro, cap, "sem memoria");
    if (imap_cmd(c, cmd, pega_search, ids, erro, cap) != 0) {
        free(*ids);
        *ids = NULL;
        return -1;
    }
    return 0;
}

/* O FETCH devolve a mensagem como literal `{n}` — o callback lê os n bytes
 * do fio na hora, porque eles vêm ANTES do fim da linha lógica. */
typedef struct { char *msg; size_t n; } FetchCtx;

static void pega_fetch(const char *linha, void *ctx, PSMailConn *c)
{
    FetchCtx *f = ctx;
    const char *abre = strrchr(linha, '{');
    if (!abre || f->msg) return;
    /* `{-1}` virava (size_t)-1: malloc(0) e le_bytes escrevendo sem fim.
     * O imaplib do Python so casa `{` 1*DIGIT `}` — negativo nao e literal. */
    long ln = atol(abre + 1);
    if (ln < 0 || ln > 256L * 1024 * 1024) return;
    size_t n = (size_t)ln;
    char *m = malloc(n + 1);
    if (!m) return;
    if (le_bytes(c, m, n) != 0) { free(m); return; }
    m[n] = '\0';
    f->msg = m;
    f->n = n;
    /* consome o resto da resposta da linha lógica (o `)` final) */
    char resto[512];
    le_linha(c, resto, sizeof(resto));
}

int ps_imap_fetch(PSMailConn *c, const char *id, int corpo,
                  char **msg, size_t *n, char *erro, size_t cap)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "FETCH %s (%s)", id,
             corpo ? "RFC822" : "RFC822.HEADER");
    FetchCtx f = { NULL, 0 };
    if (imap_cmd(c, cmd, pega_fetch, &f, erro, cap) != 0) {
        free(f.msg);
        return -1;
    }
    if (!f.msg) FALHA(erro, cap, "servidor nao devolveu a mensagem %s", id);
    *msg = f.msg;
    *n = f.n;
    return 0;
}

void ps_imap_close(PSMailConn *c, int teve_select)
{
    if (!c) return;
    char e[64];
    if (teve_select) imap_cmd(c, "CLOSE", NULL, NULL, e, sizeof(e));
    imap_cmd(c, "LOGOUT", NULL, NULL, e, sizeof(e));
    ps_mail_solta(c);
}

/* ── MIME ───────────────────────────────────────────────────────────────── */

/* latin-1 → UTF-8 (anexa em `out`, avançando `*j`). */
static void latin1_para_utf8(const unsigned char *in, size_t n, char *out, size_t *j)
{
    for (size_t i = 0; i < n; i++) {
        if (in[i] < 0x80) out[(*j)++] = (char)in[i];
        else {
            out[(*j)++] = (char)(0xC0 | (in[i] >> 6));
            out[(*j)++] = (char)(0x80 | (in[i] & 0x3F));
        }
    }
}

static int charset_eh_latin1(const char *cs)
{
    return strncasecmp(cs, "iso-8859-1", 10) == 0
        || strncasecmp(cs, "latin-1", 7) == 0 || strncasecmp(cs, "latin1", 6) == 0
        || strncasecmp(cs, "windows-1252", 12) == 0;
}

/* quoted-printable. `header` liga o `_` = espaço do RFC 2047. */
static size_t qp_decode(const char *in, size_t n, char *out, int header)
{
    size_t j = 0;
    for (size_t i = 0; i < n; i++) {
        if (header && in[i] == '_') { out[j++] = ' '; continue; }
        if (in[i] == '=' && i + 2 < n) {
            if (in[i+1] == '\r' && in[i+2] == '\n') { i += 2; continue; }  /* quebra suave */
            if (in[i+1] == '\n') { i += 1; continue; }
            int hi = -1, lo = -1;
            char a = in[i+1], b = in[i+2];
            if (a >= '0' && a <= '9') hi = a - '0';
            else if (a >= 'A' && a <= 'F') hi = a - 'A' + 10;
            else if (a >= 'a' && a <= 'f') hi = a - 'a' + 10;
            if (b >= '0' && b <= '9') lo = b - '0';
            else if (b >= 'A' && b <= 'F') lo = b - 'A' + 10;
            else if (b >= 'a' && b <= 'f') lo = b - 'a' + 10;
            if (hi >= 0 && lo >= 0) { out[j++] = (char)(hi * 16 + lo); i += 2; continue; }
        }
        out[j++] = in[i];
    }
    return j;
}

char *ps_mime_decodifica_header(const char *v, size_t n)
{
    /* pior caso: latin-1 dobra */
    char *out = malloc(n * 2 + 4);
    if (!out) return NULL;
    size_t j = 0;
    size_t i = 0;
    int anterior_codificado = 0;
    size_t espacos_pendentes = 0;
    while (i < n) {
        if (v[i] == '=' && i + 1 < n && v[i+1] == '?') {
            /* =?charset?B|Q?dados?= */
            size_t c1 = i + 2;
            const char *q1 = memchr(v + c1, '?', n - c1);
            if (q1 && (size_t)(q1 - v) + 3 < n && q1[2] == '?') {
                char enc = q1[1];
                size_t d0 = (size_t)(q1 - v) + 3;
                const char *fim = NULL;
                for (size_t k = d0; k + 1 < n; k++)
                    if (v[k] == '?' && v[k+1] == '=') { fim = v + k; break; }
                if (fim && (enc == 'B' || enc == 'b' || enc == 'Q' || enc == 'q')) {
                    char cs[32];
                    size_t ncs = (size_t)(q1 - v) - c1;
                    if (ncs > sizeof(cs) - 1) ncs = sizeof(cs) - 1;
                    memcpy(cs, v + c1, ncs);
                    cs[ncs] = '\0';

                    size_t nd = (size_t)(fim - v) - d0;
                    unsigned char *cru = malloc(nd + 4);
                    if (!cru) { free(out); return NULL; }
                    size_t ncru;
                    if (enc == 'B' || enc == 'b') {
                        long r = ps_base64_decode(v + d0, nd, cru, nd + 4);
                        ncru = r < 0 ? 0 : (size_t)r;
                    } else {
                        ncru = qp_decode(v + d0, nd, (char *)cru, 1);
                    }
                    /* espaço ENTRE palavras codificadas some (RFC 2047) */
                    if (!anterior_codificado)
                        for (size_t k = 0; k < espacos_pendentes; k++) out[j++] = ' ';
                    espacos_pendentes = 0;
                    if (charset_eh_latin1(cs)) latin1_para_utf8(cru, ncru, out, &j);
                    else { memcpy(out + j, cru, ncru); j += ncru; }
                    free(cru);
                    i = (size_t)(fim - v) + 2;
                    anterior_codificado = 1;
                    continue;
                }
            }
        }
        if (v[i] == ' ' || v[i] == '\t') {
            espacos_pendentes++;
            i++;
            continue;
        }
        for (size_t k = 0; k < espacos_pendentes; k++) out[j++] = ' ';
        espacos_pendentes = 0;
        out[j++] = v[i++];
        anterior_codificado = 0;
    }
    if (!anterior_codificado)
        for (size_t k = 0; k < espacos_pendentes; k++) out[j++] = ' ';
    out[j] = '\0';
    return out;
}

/* fim dos headers: primeira linha vazia */
static size_t fim_headers(const char *msg, size_t n)
{
    for (size_t i = 0; i + 1 < n; i++) {
        if (msg[i] == '\n' && (msg[i+1] == '\n'
                || (i + 2 < n && msg[i+1] == '\r' && msg[i+2] == '\n')))
            return i + 1;
    }
    return n;
}

char *ps_mime_header(const char *msg, size_t n, const char *nome)
{
    size_t fim = fim_headers(msg, n);
    size_t nn = strlen(nome);
    size_t i = 0;
    while (i < fim) {
        size_t fim_lin = i;
        while (fim_lin < fim && msg[fim_lin] != '\n') fim_lin++;
        if (fim - i > nn && strncasecmp(msg + i, nome, nn) == 0 && msg[i + nn] == ':') {
            size_t v0 = i + nn + 1;
            while (v0 < fim_lin && (msg[v0] == ' ' || msg[v0] == '\t')) v0++;
            /* desdobra: continuação começa com espaço/tab */
            char *out = malloc(fim - v0 + 1);
            if (!out) return NULL;
            size_t j = 0;
            size_t fl = fim_lin;
            size_t vv = v0;
            for (;;) {
                size_t f2 = fl;
                while (f2 > vv && (msg[f2-1] == '\r' || msg[f2-1] == '\n')) f2--;
                memcpy(out + j, msg + vv, f2 - vv);
                j += f2 - vv;
                size_t prox = fl + 1;
                if (prox >= fim || (msg[prox] != ' ' && msg[prox] != '\t')) break;
                vv = prox;
                while (vv < fim && (msg[vv] == ' ' || msg[vv] == '\t')) vv++;
                out[j++] = ' ';
                fl = vv;
                while (fl < fim && msg[fl] != '\n') fl++;
            }
            out[j] = '\0';
            return out;
        }
        i = fim_lin + 1;
    }
    return NULL;
}

/* parâmetro de um header estruturado: boundary="x" ou charset=utf-8 */
static int header_param(const char *v, const char *nome, char *out, size_t cap)
{
    size_t nn = strlen(nome);
    for (const char *p = v; *p; p++) {
        if (strncasecmp(p, nome, nn) != 0 || p[nn] != '=') continue;
        const char *d = p + nn + 1;
        char aspas = (*d == '"') ? *d++ : 0;
        size_t j = 0;
        while (*d && j < cap - 1) {
            if (aspas ? *d == aspas : (*d == ';' || *d == ' ' || *d == '\r' || *d == '\n')) break;
            out[j++] = *d++;
        }
        out[j] = '\0';
        return 0;
    }
    return -1;
}

/* Decodifica o corpo de UMA parte (headers + conteúdo) já isolada. */
static char *decodifica_parte(const char *parte, size_t n)
{
    size_t h = fim_headers(parte, n);
    size_t c0 = h;
    while (c0 < n && (parte[c0] == '\n' || parte[c0] == '\r')) c0++;
    size_t nc = n - c0;

    char *cte = ps_mime_header(parte, n, "Content-Transfer-Encoding");
    char *ct = ps_mime_header(parte, n, "Content-Type");
    char cs[64] = "utf-8";
    if (ct) header_param(ct, "charset", cs, sizeof(cs));

    unsigned char *cru = malloc(nc + 4);
    if (!cru) { free(cte); free(ct); return NULL; }
    size_t ncru;
    if (cte && strncasecmp(cte, "base64", 6) == 0) {
        long r = ps_base64_decode(parte + c0, nc, cru, nc + 4);
        if (r < 0) {
            /* base64 sujo: o Python (validate=False) joga fora o byte invalido
             * e decodifica o resto. Perder o CORPO INTEIRO por um byte de um
             * mailer velho e perda de dado calada. */
            char *lp = malloc(nc + 1);
            if (lp) {
                size_t lj = 0;
                for (size_t z = 0; z < nc; z++) {
                    char cz = parte[c0 + z];
                    if ((cz >= 'A' && cz <= 'Z') || (cz >= 'a' && cz <= 'z')
                        || (cz >= '0' && cz <= '9') || cz == '+' || cz == '/' || cz == '=')
                        lp[lj++] = cz;
                }
                r = ps_base64_decode(lp, lj, cru, nc + 4);
                free(lp);
            }
        }
        ncru = r < 0 ? 0 : (size_t)r;
    } else if (cte && strncasecmp(cte, "quoted-printable", 16) == 0) {
        ncru = qp_decode(parte + c0, nc, (char *)cru, 0);
    } else {
        memcpy(cru, parte + c0, nc);
        ncru = nc;
    }
    free(cte);
    free(ct);

    char *out = malloc(ncru * 2 + 1);
    if (!out) { free(cru); return NULL; }
    size_t j = 0;
    if (charset_eh_latin1(cs)) latin1_para_utf8(cru, ncru, out, &j);
    else { memcpy(out, cru, ncru); j = ncru; }
    out[j] = '\0';
    free(cru);
    return out;
}

/* Caminha as partes procurando text/plain (preferido) e text/html. */
static void acha_corpo(const char *msg, size_t n, char **plain, char **html)
{
    char *ct = ps_mime_header(msg, n, "Content-Type");
    char boundary[256];
    if (ct && strncasecmp(ct, "multipart/", 10) == 0
            && header_param(ct, "boundary", boundary, sizeof(boundary)) == 0) {
        free(ct);
        char marca[300];
        int nm = snprintf(marca, sizeof(marca), "--%s", boundary);
        size_t h = fim_headers(msg, n);
        size_t i = h;
        size_t ini = 0;
        while (i < n) {
            size_t fim_lin = i;
            while (fim_lin < n && msg[fim_lin] != '\n') fim_lin++;
            if ((int)(fim_lin - i) >= nm && strncmp(msg + i, marca, (size_t)nm) == 0) {
                if (ini > 0) {
                    size_t fim_parte = i;
                    while (fim_parte > ini && (msg[fim_parte-1] == '\n' || msg[fim_parte-1] == '\r'))
                        fim_parte--;
                    acha_corpo(msg + ini, fim_parte - ini, plain, html);
                }
                ini = fim_lin + 1;
                if ((int)(fim_lin - i) >= nm + 2 && msg[i + nm] == '-' && msg[i + nm + 1] == '-')
                    break;             /* --boundary-- encerra */
            }
            i = fim_lin + 1;
        }
        return;
    }

    /* parte folha */
    char *cd = ps_mime_header(msg, n, "Content-Disposition");
    int anexo = cd && strstr(cd, "attachment") != NULL;
    free(cd);
    if (anexo) { free(ct); return; }

    int eh_html = ct && strncasecmp(ct, "text/html", 9) == 0;
    int eh_plain = !ct || strncasecmp(ct, "text/plain", 10) == 0
                 || strncasecmp(ct, "text/", 5) != 0;   /* sem tipo: trata como plano */
    if (ct && strncasecmp(ct, "text/", 5) != 0 && strncasecmp(ct, "message/", 8) != 0) {
        /* parte binária sem disposition — não é corpo */
        free(ct);
        return;
    }
    free(ct);
    if (eh_html) {
        if (!*html) *html = decodifica_parte(msg, n);
    } else if (eh_plain) {
        if (!*plain) *plain = decodifica_parte(msg, n);
    }
}

char *ps_mime_corpo(const char *msg, size_t n)
{
    char *plain = NULL, *html = NULL;
    acha_corpo(msg, n, &plain, &html);
    if (plain) { free(html); return plain; }
    if (html) return html;
    return strdup("");
}
