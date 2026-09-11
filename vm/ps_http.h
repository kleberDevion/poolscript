/*
 * Cliente HTTP/HTTPS em C — sem libcurl, sem `Python.h`.
 *
 * Mesmo princípio do ps_mail.c: socket cru + OpenSSL pro TLS, protocolo
 * escrito à mão. Não linka biblioteca de rede — só a OpenSSL que já entra
 * (estática) pelo TLS. Zero dependência de runtime no binário.
 *
 * Diferente do mail, aqui o TLS VERIFICA o certificado: é o que o
 * `urllib.request.urlopen` faz por padrão (contexto SSL default), e afrouxar
 * deixaria o cliente aceitar man-in-the-middle que o interpretador recusa.
 */
#ifndef PS_HTTP_H
#define PS_HTTP_H

#include <stddef.h>

typedef struct {
    long   status;        /* código HTTP; -1 em falha de transporte */
    char  *url_final;     /* URL após redirecionamentos (malloc) */
    char  *headers;       /* bloco cru "Nome: valor\n..." (malloc) */
    char  *corpo;         /* bytes do corpo (malloc) */
    size_t ncorpo;
    char   erro[256];     /* preenchido quando status == -1 */
    char   erro_tipo[32]; /* NetworkError / TimeoutError */
} PSHttpResp;

/* `metodo` é "GET"/"POST"/etc. `cabs` são linhas "Nome: valor\n" já prontas
 * (ou NULL). `corpo`/`ncorpo` é o body a enviar (ou NULL/0). `teto` limita o
 * tamanho do corpo lido: 0 = sem limite; estourar vira erro MemoryError.
 * `timeout` em segundos.
 *
 * Devolve 0 e preenche `r`. Em erro de rede devolve 0 também, mas com
 * `r->status == -1` e `r->erro`/`r->erro_tipo` — resposta HTTP de erro
 * (404, 500) NÃO é erro de rede: volta com o status certo e o corpo. */
int ps_http_request(const char *metodo, const char *url, const char *cabs,
                    const char *corpo, size_t ncorpo, int timeout,
                    long teto, PSHttpResp *r);

/* O mesmo, com um DESTINO: cada pedaço do corpo é escrito nele assim que
 * chega e nada se acumula, então dá pra baixar arquivo maior que a memória.
 * `destino` NULL faz exatamente o que o `ps_http_request` faz. Em modo de
 * fluxo o `r->corpo` volta vazio e `r->ncorpo` é quantos bytes desceram. */
int ps_http_baixa(const char *metodo, const char *url, const char *cabs,
                  const char *corpo, size_t ncorpo, int timeout,
                  long teto, FILE *destino, PSHttpResp *r);

void ps_http_resp_solta(PSHttpResp *r);

#endif /* PS_HTTP_H */
