/*
 * `pool` — o executável da PoolScript. Zero CPython.
 *
 * Faz o que o `vm_executa_fonte` do plugin faz, chamando a MESMA
 * `ps_roda_fonte`. A diferença é só o que acontece com o erro: aqui vira
 * texto no stderr e código de saída, lá vira exceção do Python.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ps_vm.h"

#ifndef PS_VERSAO
#define PS_VERSAO "8.2.18"
#endif

static void uso(const char *prog)
{
    fprintf(stderr,
        "uso: %s <arquivo.ps>\n"
        "     %s -e <codigo>\n"
        "     %s --version\n", prog, prog, prog);
}

/* Lê o arquivo inteiro. Devolve NULL e reclama no stderr se não der. */
static char *le_arquivo(const char *caminho, size_t *tam)
{
    FILE *f = fopen(caminho, "rb");
    if (!f) {
        fprintf(stderr, "pool: nao consegui abrir '%s'\n", caminho);
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long n = ftell(f);
    if (n < 0) { fclose(f); return NULL; }
    rewind(f);
    char *buf = malloc((size_t)n + 1);
    if (!buf) { fclose(f); fprintf(stderr, "pool: sem memoria\n"); return NULL; }
    size_t lidos = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[lidos] = '\0';
    *tam = lidos;
    return buf;
}

/* Erro do usuário sai no formato que o interpretador já usa, pra mensagem
 * não mudar de cara conforme quem executou. */
static int reporta(const PSErroExec *e, const char *origem)
{
    switch (e->tipo) {
        case PS_ERRO_SINTAXE:
            fprintf(stderr, "SyntaxError: %s\n  -> %s, linha %d, coluna %d\n",
                    e->msg, origem, e->linha, e->col);
            return 2;
        case PS_ERRO_NAO_SUPORTADO:
            fprintf(stderr, "NotImplementedError: %s\n  -> %s, linha %d, coluna %d\n",
                    e->msg, origem, e->linha, e->col);
            return 3;
        case PS_ERRO_MEMORIA:
            fprintf(stderr, "MemoryError: %s\n", e->msg);
            return 4;
        default:
            fprintf(stderr, "%s: %s\n",
                    e->tipo_nome[0] ? e->tipo_nome : "RuntimeError", e->msg);
            return 1;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) { uso(argv[0]); return 64; }

    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
        printf("PoolScript %s\n", PS_VERSAO);
        return 0;
    }
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        uso(argv[0]);
        return 0;
    }

    PSErroExec e;

    if (strcmp(argv[1], "-e") == 0) {
        if (argc < 3) { uso(argv[0]); return 64; }
        if (ps_roda_fonte(argv[2], strlen(argv[2]), NULL, &e) != 0)
            return reporta(&e, "<-e>");
        return 0;
    }

    /* tudo depois do arquivo é do usuário — é o que o `sys.argv` devolve */
    ps_set_argv(argc - 2, argv + 2);

    size_t tam = 0;
    char *fonte = le_arquivo(argv[1], &tam);
    if (!fonte) return 66;
    int rc = ps_roda_fonte(fonte, tam, argv[1], &e);
    free(fonte);
    if (rc != 0) return reporta(&e, argv[1]);
    return 0;
}
