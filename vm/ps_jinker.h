/*
 * Transporte do jinker — servidor HTTP/1.1 + WebSocket (RFC 6455) em C puro.
 *
 * Mesmo princípio do ps_http.c, no sentido inverso: socket cru + OpenSSL,
 * protocolo escrito à mão, zero dependência de runtime. Este arquivo NÃO
 * conhece a VM — só lê/escreve bytes e estrutura requisições. Quem roteia,
 * chama handler e monta resposta é a camada jinker dentro da VM, que é a
 * dona do loop (single-thread: o handler roda na thread da VM, então não
 * existe questão de reentrância nem de GC concorrente).
 *
 * O modelo semântico é o de http.server + websockets: o que o
 * cliente HTTP observa — status, corpo, headers de CORS — tem que bater.
 */
#ifndef PS_JINKER_H
#define PS_JINKER_H

#include <stddef.h>

/* ── conexão ────────────────────────────────────────────────────────────── */
/* Opaca: fd + SSL* (quando TLS) + buffer de leitura acumulado — uma conexão
 * keep-alive entrega várias requisições, e o excedente de uma leitura é o
 * começo da próxima. */
typedef struct PSJkConn PSJkConn;

/* Escuta em host:porta. Devolve o fd de escuta ou -1 (erro em `erro`). */
int ps_jk_listen(const char *host, int porta, char *erro, size_t ecap);

/* Aceita uma conexão. `ssl_ctx` é um SSL_CTX* (ou NULL sem TLS). Preenche o
 * IP do cliente. NULL se o accept falhar (inclusive handshake TLS). */
PSJkConn *ps_jk_accept(int fd_escuta, void *ssl_ctx, char *ip, size_t ipcap);

int  ps_jk_fd(const PSJkConn *c);
void ps_jk_close(PSJkConn *c);
void ps_jk_conn_solta_buf(PSJkConn *c);   /* libera o buffer de leitura na ociosidade */

/* ── requisição HTTP ────────────────────────────────────────────────────── */
typedef struct { char *nome; char *valor; } PSJkHdr;

typedef struct {
    char      metodo[16];
    char     *path;       /* já decodificado (percent-encoding) */
    char     *query;      /* cru, sem o '?'; "" se não houver */
    PSJkHdr  *cabs;
    int       ncabs;
    char     *corpo;
    size_t    ncorpo;
    int       keep_alive; /* HTTP/1.1 sem "Connection: close" */
    /* ARENA do request (opaca, ps_jinker.c): path/query/cabs/nome/valor saem
     * de UM bloco bump-alocado em vez de ~10-20 mallocs; ps_jk_req_solta
     * devolve tudo de uma vez. O corpo (até 64MB, 1 alocação) fica em malloc. */
    void     *ar;
} PSJkReq;

/* Lê UMA requisição. Devolve:
 *    0  ok
 *   -1  conexão acabou / incompleta -> fechar calado
 *   -2  requisição MALFORMADA -> responder 400 e fechar
 *   -3  algo que não implementamos (Transfer-Encoding) -> 501 e fechar
 *
 * Os dois últimos existem porque "fechar calado" diante de requisição torta é
 * o que abre *request smuggling*: um intermediário na frente interpreta o que
 * mandaram de um jeito, nós de outro, e ninguém reclama. A RFC 9112 manda
 * RECUSAR, com resposta. */
#define PSJK_OK        0
#define PSJK_FECHA    (-1)
#define PSJK_MALFORM  (-2)
#define PSJK_NAOIMPL  (-3)

int  ps_jk_le_request(PSJkConn *c, PSJkReq *r);
void ps_jk_req_solta(PSJkReq *r);

/* Valor de um header (case-insensitive) ou NULL. Aponta pra dentro de `r`. */
const char *ps_jk_header(const PSJkReq *r, const char *nome);

/* ── resposta HTTP ──────────────────────────────────────────────────────── */
/* `extra` são linhas "Nome: valor\r\n" já prontas (ou NULL). Sempre manda
 * Content-Length, Server: Jinker e Date — o contrato do BaseHTTPRequestHandler
 * com version_string()="Jinker". 0 ok, -1 erro de escrita. */
int ps_jk_responde(PSJkConn *c, int status, const char *ctype,
                   const char *corpo, size_t ncorpo,
                   const char *extra, int keep_alive);

/* ── WebSocket CLIENTE (o `ws_connect` do módulo request) ───────────────── */
/* Lê um frame de dados. Trata ping (responde pong) e continua. Devolve:
 * 0 = mensagem completa em `msg`/`n` (malloc — caller libera);
 * 1 = close recebido (já respondido); -1 = erro/conexão caiu, ou frame
 * mascarado vindo do servidor (RFC 6455 §5.1). */
int ps_jk_ws_le_frame(PSJkConn *c, char **msg, size_t *n);

/* Frame de close, MASCARADO como todo frame do cliente. */
int ps_jk_ws_envia_close(PSJkConn *c, int codigo, const char *motivo);

/* Conecta e faz o handshake de cliente (Sec-WebSocket-Key + validação do
 * Accept). NULL em erro (mensagem em `erro`). */
PSJkConn *ps_jk_ws_conecta(const char *host, int porta, const char *path,
                           char *erro, size_t ecap);

/* Frame de texto MASCARADO — obrigatório na direção cliente→servidor. */
int ps_jk_ws_envia_texto_cli(PSJkConn *c, const char *msg, size_t n);

/* 1 se há bytes prontos pra ler (buffer ou socket) em até `timeout_ms`. */
int ps_jk_conn_pendente(const PSJkConn *c);   /* bytes já lidos e não consumidos */
int ps_jk_ws_tem_dados(PSJkConn *c, int timeout_ms);

/* ── multipart/form-data ────────────────────────────────────────────────── */
typedef struct {
    char   *campo;     /* name="..." */
    char   *filename;  /* NULL = campo de texto */
    char   *ctype;
    char   *dados;
    size_t  ndados;
} PSJkParte;

/* Divide o corpo pelo boundary. Devolve o nº de partes (0 inclusive) ou -1.
 * O array e cada string saem de malloc — ps_jk_partes_solta libera tudo. */
int  ps_jk_multipart(const char *corpo, size_t n, const char *boundary,
                     PSJkParte **partes);
void ps_jk_partes_solta(PSJkParte *p, int n);

/* ── utilidades ─────────────────────────────────────────────────────────── */
/* MIME por extensão — a tabela padrão do `mimetypes` do Python 3.15;
 * desconhecido = application/octet-stream. */
const char *ps_jk_mime(const char *caminho);

/* Responde um arquivo já conferido (dentro da raiz e publicável): 200, 206,
 * 304 ou 416, com Accept-Ranges, Last-Modified e ETag, e o corpo em blocos de
 * 64 KB (nunca o arquivo inteiro na memória). Range, If-None-Match,
 * If-Modified-Since e If-Range valem em GET e HEAD, pela RFC 9110. `extra` =
 * linhas de cabeçalho a mais, já com "\r\n" (ou NULL). Devolve 0 servido;
 * -1 a conexão caiu no meio (fechar); -2 o arquivo não abriu (nada foi
 * escrito: quem chamou responde o 404). */
int ps_jk_serve_arquivo(PSJkConn *c, const PSJkReq *r, const char *caminho,
                        const char *ctype, const char *extra, int keep_alive);

/* Percent-decoding in-place ('+' NÃO vira espaço — regra do path). */
void ps_jk_urldecode(char *s);
/* Versão de query string ('+' vira espaço). */
void ps_jk_urldecode_qs(char *s);

/* ── TLS ────────────────────────────────────────────────────────────────── */
/* Cria o SSL_CTX de servidor com cert+key. NULL em erro. */
void *ps_jk_tls_ctx(const char *cert, const char *key, char *erro, size_t ecap);
void  ps_jk_tls_ctx_solta(void *ctx);

/* Gera certificado self-signed (CN=localhost, RSA 2048, 365 dias) nos dois
 * caminhos — o equivalente do `openssl req -x509` que o jinker_lib chama. */
int ps_jk_tls_autogera(const char *cert_path, const char *key_path,
                       char *erro, size_t ecap);

#endif /* PS_JINKER_H */
