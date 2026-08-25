/*
 * `pool` — o executável da PoolScript. Zero CPython.
 *
 * Faz o que o `vm_executa_fonte` do plugin faz, chamando a MESMA
 * `ps_roda_fonte`. A diferença é só o que acontece com o erro: aqui vira
 * texto no stderr e código de saída, lá vira exceção do Python.
 *
 * Os comandos de RUNTIME da CLI vivem aqui (rodar, repl, build, help…). Os de
 * PACOTE (`install`/`uninstall`/`list`/`registry`) NÃO: dependem de pip e da
 * árvore de pacotes do Python, que não existem num binário sem CPython — esses
 * são do `psl`. Chamá-los aqui dá um aviso claro em vez de fingir que faz.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>   /* getcwd — encurta o caminho do traceback pro relativo */

#include "ps_vm.h"
#include "ps_pkg.h"

#include "ps_versao.h"

#define SPEC_URL "https://github.com/kleberDevion/poolscript-lang"

static void ajuda(void)
{
    printf(
"PoolScript %s — PSVM (VM em C, runtime standalone)\n"
"\n"
"Uso:\n"
"  pool arquivo.ps           Roda um arquivo\n"
"  pool -e \"<codigo>\"        Roda codigo inline (uma linha)\n"
"  pool build                Roda todos os .ps da pasta atual\n"
"  pool --check [arq.ps]     So analisa (nao roda); JSON com o erro. Sem\n"
"                            arquivo, le da entrada padrao\n"
"  pool //doc                Mostra a URL da especificacao\n"
"  pool --version / -V       Mostra a versao\n"
"  pool --help / -h          Mostra esta ajuda\n"
"\n"
"Pacotes (lib e comando .ps):\n"
"  psl install <arq.ps>          Instala (o arquivo decide via #!lib / #!cmd)\n"
"  psl install <arq.ps> -asLib   Forca lib importavel (import nome)\n"
"  psl install <nome>            Busca <nome> no registry configurado\n"
"  psl uninstall <nome>          Remove (acha sozinho: comando ou lib)\n"
"  psl uninstall <nome> -asLib   Forca a categoria lib\n"
"  psl list                      Lista comandos e libs instalados\n"
"  psl registry set-url <url>    Configura o indice de pacotes\n"
"  psl registry show             Mostra o registry configurado\n"
"\n"
"Libs internas: json, date, regex, hash, jwt, sys, dotenv, os, datasentity,\n"
"  Parsing, sqlite3, mail, request, qrcode, manpu, psodbc, jinker\n"
"\n"
"Docs: " SPEC_URL "\n", PS_VERSAO);
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

/* Imprime a localização + o trecho do fonte, como o interpretador:
 *     -> arquivo, linha N
 *     | <a linha de código>
 *     | ^
 * Só quando há linha e o arquivo abre (origem "<-e>"/stdin não tem trecho). */
/* Encurta o caminho pra exibição: relativo-ao-cwd se estiver sob ele, senão
 * só o basename — espelha o `_clean_filename` do interpretador (a autoridade),
 * pra que os dois motores mostrem o MESMO nome no traceback. O caminho cheio
 * ainda é usado pra abrir o arquivo e ler o trecho. */
static const char *limpa_arquivo(const char *fn, char *buf, size_t cap)
{
    if (!fn || !fn[0] || fn[0] == '<') return (fn && fn[0]) ? fn : "<script>";
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        size_t lc = strlen(cwd);
        if (strncmp(fn, cwd, lc) == 0 && fn[lc] == '/') {   /* sob o cwd => relativo */
            snprintf(buf, cap, "%s", fn + lc + 1);
            return buf;
        }
    }
    const char *b = strrchr(fn, '/');           /* fora do cwd => basename */
    return b ? b + 1 : fn;
}

/* Um quadro: "  em <arq>, linha N" + a linha do fonte + o cursor `^^^`. Igual
 * ao `_fmt_frame` do interpretador. `col<=0` => cursor na 1ª não-branco. */
static void imprime_quadro(const char *origem, int linha, int col)
{
    char nome_buf[1024];
    fprintf(stderr, "  em %s, linha %d\n", limpa_arquivo(origem, nome_buf, sizeof(nome_buf)), linha);
    if (linha <= 0 || !origem || origem[0] == '<') return;
    FILE *f = fopen(origem, "rb");
    if (!f) return;
    char buf[4096];
    int atual = 0;
    while (fgets(buf, sizeof(buf), f)) {
        if (++atual != linha) continue;
        size_t l = strlen(buf);
        while (l > 0 && (buf[l-1] == '\n' || buf[l-1] == '\r')) buf[--l] = '\0';
        int sp;
        if (col > 0) sp = col - 1;
        else { sp = 0; while (buf[sp] == ' ' || buf[sp] == '\t') sp++; }
        fprintf(stderr, "  | %s\n  | %*s^^^\n", buf, sp, "");
        break;
    }
    fclose(f);
}

/* Erro do usuário sai EXATAMENTE como o interpretador (a autoridade): a
 * mensagem primeiro, depois o traceback do mais interno ao mais externo, e o
 * quadro do erro por último. O header só aparece quando há chamadores. */
static int reporta(const PSErroExec *e, const char *origem)
{
    switch (e->tipo) {
        case PS_ERRO_SINTAXE:
            fprintf(stderr, "SyntaxError: %s\n", e->msg);
            imprime_quadro(origem, e->linha, e->col);
            return 2;
        case PS_ERRO_NAO_SUPORTADO:
            fprintf(stderr, "NotImplementedError: %s\n", e->msg);
            imprime_quadro(origem, e->linha, e->col);
            return 3;
        case PS_ERRO_MEMORIA:
            fprintf(stderr, "MemoryError: %s\n", e->msg);
            return 4;
        default:
            fprintf(stderr, "%s: %s\n",
                    e->tipo_nome[0] ? e->tipo_nome : "RuntimeError", e->msg);
            if (e->ntb > 0) {
                if (e->ntb > 1)
                    fprintf(stderr, "\nTraceback (arquivo mais recente por último):\n");
                for (int i = 0; i < e->ntb; i++) {
                    const char *arq = e->tb[i].arquivo[0] ? e->tb[i].arquivo : origem;
                    imprime_quadro(arq, e->tb[i].linha, e->tb[i].col);
                }
            } else if (e->linha > 0) {
                imprime_quadro(origem, e->linha, e->col);
            }
            return 1;
    }
}

/* `build`: roda todos os .ps da pasta atual, em ordem, e conta OK/erro.
 * Espelha o `_cmd_build` da cli.py. */
static int cmd_build(void)
{
    DIR *d = opendir(".");
    if (!d) { fprintf(stderr, "pool: nao consegui abrir a pasta atual\n"); return 1; }

    /* coleta os arquivos .ps/.psl/.p e ordena */
    char **nomes = NULL; int n = 0, cap = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        size_t l = strlen(ent->d_name);
        const char *nm = ent->d_name;
        int eh = (l > 3 && strcmp(nm + l - 3, ".ps")  == 0)
              || (l > 4 && strcmp(nm + l - 4, ".psl") == 0)
              || (l > 2 && strcmp(nm + l - 2, ".p")   == 0);
        if (!eh) continue;
        if (n == cap) { cap = cap ? cap * 2 : 16; nomes = realloc(nomes, sizeof(char *) * (size_t)cap); }
        nomes[n++] = strdup(ent->d_name);
    }
    closedir(d);
    if (n == 0) { fprintf(stderr, "nenhum arquivo .ps/.psl/.p encontrado na pasta atual\n"); free(nomes); return 1; }
    for (int i = 0; i < n; i++)          /* ordenação simples (n pequeno) */
        for (int j = i + 1; j < n; j++)
            if (strcmp(nomes[i], nomes[j]) > 0) { char *t = nomes[i]; nomes[i] = nomes[j]; nomes[j] = t; }

    int rc = 0, ok = 0, falhou = 0;
    for (int i = 0; i < n; i++) {
        printf("=== %s ===\n", nomes[i]);
        fflush(stdout);
        size_t tam = 0;
        char *fonte = le_arquivo(nomes[i], &tam);
        if (!fonte) { falhou++; rc = 1; free(nomes[i]); continue; }
        PSErroExec e;
        int r = ps_roda_fonte(fonte, tam, nomes[i], &e);
        free(fonte);
        if (r != 0) { reporta(&e, nomes[i]); falhou++; rc = r; }
        else ok++;
        free(nomes[i]);
    }
    free(nomes);
    printf("\n%d arquivo(s) OK, %d com erro(s).\n", ok, falhou);
    return rc;
}

/* escreve `s` como string JSON (aspas + escapes) no stdout */
static void json_str(const char *s)
{
    putchar('"');
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        switch (*p) {
            case '"':  fputs("\\\"", stdout); break;
            case '\\': fputs("\\\\", stdout); break;
            case '\n': fputs("\\n", stdout);  break;
            case '\r': fputs("\\r", stdout);  break;
            case '\t': fputs("\\t", stdout);  break;
            default:
                if (*p < 0x20) printf("\\u%04x", *p);
                else putchar(*p);
        }
    }
    putchar('"');
}

/* `pool --check [arquivo.ps]` — lexer/parser/compilador da VM, SEM rodar, com
 * o resultado em JSON pro editor. Sem arquivo, lê o buffer do stdin (o editor
 * manda o conteúdo não salvo). NUNCA executa o código. */
static int cmd_check(const char *arquivo)
{
    size_t tam = 0;
    char *fonte = NULL;
    if (arquivo) {
        fonte = le_arquivo(arquivo, &tam);
        if (!fonte) { printf("{\"ok\":false,\"tipo\":\"IOError\",\"msg\":\"nao consegui abrir o arquivo\",\"linha\":1,\"coluna\":1}\n"); return 0; }
    } else {
        size_t cap = 65536; tam = 0;
        fonte = malloc(cap);
        if (!fonte) { printf("{\"ok\":false,\"tipo\":\"MemoryError\",\"msg\":\"sem memoria\",\"linha\":1,\"coluna\":1}\n"); return 0; }
        size_t r;
        while ((r = fread(fonte + tam, 1, cap - tam, stdin)) > 0) {
            tam += r;
            if (tam == cap) { cap *= 2; char *nb = realloc(fonte, cap); if (!nb) { free(fonte); printf("{\"ok\":false,\"tipo\":\"MemoryError\",\"msg\":\"sem memoria\",\"linha\":1,\"coluna\":1}\n"); return 0; } fonte = nb; }
        }
        fonte[tam] = '\0';
    }

    PSErroExec e;
    int rc = ps_verifica_fonte(fonte, tam, arquivo, &e);
    free(fonte);
    if (rc == 0) { printf("{\"ok\":true}\n"); return 0; }

    const char *tipo = e.tipo == PS_ERRO_SINTAXE ? "SyntaxError"
                     : e.tipo == PS_ERRO_NAO_SUPORTADO ? "NotImplementedError"
                     : e.tipo == PS_ERRO_MEMORIA ? "MemoryError" : "RuntimeError";
    printf("{\"ok\":false,\"tipo\":\"%s\",\"msg\":", tipo);
    json_str(e.msg);
    printf(",\"linha\":%d,\"coluna\":%d}\n", e.linha, e.col);
    return 0;
}

/* -asLib nos argumentos? (força instalar/remover como lib) */
static int tem_flag(int argc, char **argv, int de, const char *flag)
{
    for (int i = de; i < argc; i++) if (!strcmp(argv[i], flag)) return 1;
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) { ajuda(); return 0; }

    const char *cmd = argv[1];

    if (!strcmp(cmd, "--version") || !strcmp(cmd, "-V") || !strcmp(cmd, "-v")) {
        printf("PoolScript %s [PSVM]\n", PS_VERSAO);
        return 0;
    }
    if (!strcmp(cmd, "--help") || !strcmp(cmd, "-h") || !strcmp(cmd, "help")) {
        ajuda();
        return 0;
    }
    if (!strcmp(cmd, "//doc")) {
        printf("Especificacao da PoolScript:\n  %s\n", SPEC_URL);
        return 0;
    }
    if (!strcmp(cmd, "--check") || !strcmp(cmd, "check"))
        return cmd_check(argc >= 3 ? argv[2] : NULL);
    /* modelo de tipos direto do motor — o editor e a auditoria de doc leem
     * daqui em vez de introspectar a stdlib do interpretador */
    if (!strcmp(cmd, "--metadata") || !strcmp(cmd, "metadata")) {
        ps_metadata_json(stdout);
        return 0;
    }
    if (!strcmp(cmd, "build")) return cmd_build();
    if (!strcmp(cmd, "compile")) {
        printf("compile: nao disponivel nesta versao.\n");
        return 0;
    }
    /* ── pacotes (só .ps: lib/comando) ──────────────────────────────── */
    if (!strcmp(cmd, "install")) {
        if (argc < 3) { fprintf(stderr, "uso: psl install <arquivo.ps | nome> [-asLib]\n"); return 1; }
        int modo = tem_flag(argc, argv, 3, "-asLib") ? PS_PKG_LIB : PS_PKG_AUTO;
        return ps_pkg_install(argv[2], modo);
    }
    if (!strcmp(cmd, "uninstall")) {
        if (argc < 3) { fprintf(stderr, "uso: psl uninstall <nome> [-asLib]\n"); return 1; }
        int cat = tem_flag(argc, argv, 3, "-asLib") ? PS_PKG_LIB : PS_PKG_AUTO;
        return ps_pkg_uninstall(argv[2], cat);
    }
    if (!strcmp(cmd, "list")) return ps_pkg_list();
    if (!strcmp(cmd, "registry")) return ps_pkg_registry(argc - 2, argv + 2);
    if (!strcmp(cmd, "repl")) {
        fprintf(stderr,
            "pool: o REPL interativo ainda nao esta no binario C "
            "(precisa de estado persistente na VM).\n"
            "      Por enquanto: `pool arquivo.ps` ou `pool -e \"<codigo>\"`.\n");
        return 64;
    }

    PSErroExec e;

    if (!strcmp(cmd, "-e")) {
        if (argc < 3) { ajuda(); return 64; }
        if (ps_roda_fonte(argv[2], strlen(argv[2]), NULL, &e) != 0)
            return reporta(&e, "<-e>");
        return 0;
    }

    /* tudo depois do arquivo é do usuário — é o que o `sys.argv` devolve */
    ps_set_argv(argc - 2, argv + 2);

    size_t tam = 0;
    char *fonte = le_arquivo(cmd, &tam);
    if (!fonte) return 66;
    int rc = ps_roda_fonte(fonte, tam, cmd, &e);
    free(fonte);
    if (rc != 0) return reporta(&e, cmd);
    return 0;
}
