/*
 * Enquadramento do Debug Adapter Protocol. Ver `ps_debug.h`.
 *
 * A leitura é byte a byte de propósito. O adaptador conversa por um pipe com o
 * editor, e ler em bloco exigiria um buffer de sobra entre mensagens: o excesso
 * lido junto do cabeçalho pertence à mensagem seguinte, e perdê-lo trava a
 * sessão inteira sem erro nenhum. Byte a byte não tem esse estado. O volume é
 * de dezenas de mensagens por interação humana, não de um laço quente.
 */
#include "ps_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* Um byte, retomando em EINTR. 1 = leu, 0 = fim, -1 = erro. */
static int le_byte(int fd, char *c)
{
    for (;;) {
        ssize_t r = read(fd, c, 1);
        if (r == 1) return 1;
        if (r == 0) return 0;
        if (errno == EINTR) continue;
        return -1;
    }
}

/* Uma linha de cabeçalho, sem o CRLF. Devolve o tamanho, ou -1 no fim. */
static int le_linha(int fd, char *buf, size_t max)
{
    size_t n = 0;
    for (;;) {
        char c;
        int r = le_byte(fd, &c);
        if (r <= 0) return -1;
        if (c == '\r') continue;          /* o \n é quem termina */
        if (c == '\n') { buf[n] = '\0'; return (int)n; }
        if (n + 1 < max) buf[n++] = c;
        /* linha maior que o buffer: o excesso é descartado, não estoura */
    }
}

int psdbg_le(int fd, char **corpo, size_t *tam)
{
    long tamanho = -1;
    char linha[512];

    for (;;) {
        int n = le_linha(fd, linha, sizeof linha);
        if (n < 0) return -1;
        if (n == 0) break;                /* linha vazia: acabou o cabeçalho */
        /* `Content-Length:` é o único que interessa; o resto o protocolo manda
         * ignorar em vez de recusar. Comparação sem caso porque o cabeçalho é
         * insensível a maiúscula. */
        if (strncasecmp(linha, "Content-Length:", 15) == 0) {
            char *fim = NULL;
            tamanho = strtol(linha + 15, &fim, 10);
            if (fim == linha + 15 || tamanho < 0) tamanho = -1;
        }
    }
    if (tamanho < 0) return -1;           /* sem tamanho não dá pra ler o corpo */

    char *b = malloc((size_t)tamanho + 1);
    if (!b) return -1;
    for (long i = 0; i < tamanho; i++) {
        if (le_byte(fd, &b[i]) <= 0) { free(b); return -1; }
    }
    b[tamanho] = '\0';
    *corpo = b;
    *tam   = (size_t)tamanho;
    return 0;
}

int psdbg_escreve(int fd, const char *json, size_t tam)
{
    char cab[64];
    int nc = snprintf(cab, sizeof cab, "Content-Length: %zu\r\n\r\n", tam);
    if (nc <= 0) return -1;

    /* Cabeçalho e corpo saem no mesmo laço: `write` em pipe pode levar menos do
     * que se pede, e parar na primeira escrita parcial entregaria uma mensagem
     * cortada — que do outro lado dessincroniza tudo o que vem depois. */
    const char *partes[2] = { cab, json };
    size_t      tams[2]   = { (size_t)nc, tam };
    for (int k = 0; k < 2; k++) {
        size_t escrito = 0;
        while (escrito < tams[k]) {
            ssize_t w = write(fd, partes[k] + escrito, tams[k] - escrito);
            if (w > 0) { escrito += (size_t)w; continue; }
            if (w < 0 && errno == EINTR) continue;
            return -1;
        }
    }
    return 0;
}
