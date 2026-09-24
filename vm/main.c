/*
 * `jinga` — o executável da Jinga.
 *
 * Roda o fonte pela `ps_roda_fonte`; o erro vira texto no stderr e código de
 * saída. Os comandos de RUNTIME (rodar, repl, build, help…) e os de PACOTE
 * (`jpkg install`/`uninstall`/`list`/`registry`, implementados em ps_pkg.c)
 * vivem no MESMO binário: `jinga` roda, `jpkg` gerencia pacotes, e o help e a
 * doc respeitam essa divisão. `pool` e `psl` são os nomes de antes do rename
 * e continuam instalados como atalhos do mesmo binário.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>   /* chmod — o executavel gerado por `-o` nasce +x */
#include <unistd.h>   /* getcwd — encurta o caminho do traceback pro relativo */

#include "ps_vm.h"
#include "ps_lexer.h"   /* --contexto: o editor pergunta pro lexer, nao pro regex */
#include "ps_parser.h"  /* --ast: e pra ARVORE que ele pergunta, pelo mesmo motivo */
#include "ps_pkg.h"
#include "ps_ext.h"

#include "ps_versao.h"
/* A data desta compilação, gerada pelo Makefile. Dois binários da MESMA versão
 * (o instalado e o recém-compilado) respondiam igual ao `--version`, e não
 * havia como saber o que tinha na máquina. O prefixo `Jinga <versao>
 * [PSVM]` não muda: quem lê a saída (o servidor LSP tira a versão dela)
 * continua valendo. */
#include "ps_build.h"

#define SPEC_URL "https://github.com/kleberDevion/poolscript"

static void ajuda(void)
{
    printf(
"Jinga %s — PSVM (VM em C, runtime standalone)\n"
"\n"
"Uso:\n"
"  jinga arquivo.pr           Roda um arquivo\n"
"  jinga arquivo.pr -o nome   Gera um executavel que roda sozinho (sem o fonte\n"
"                             e sem a jinga instalada)\n"
"  jinga -e \"<codigo>\"        Roda codigo inline (uma linha)\n"
"  jinga build                Roda todos os " PS_EXT " da pasta atual\n"
"  jinga --check [arq" PS_EXT "]     So analisa (nao roda); JSON com o erro. Sem\n"
"                             arquivo, le da entrada padrao\n"
"  jinga --check --path <arq> Confere o fonte da entrada padrao COMO SE fosse\n"
"                             esse arquivo (acha os modulos vizinhos)\n"
"  jinga //doc                Mostra a URL da especificacao\n"
"  jinga --contexto L:C [arq] O que o cursor toca (pro editor); sem arquivo, stdin\n"
"  jinga --tokens [arq]       Tokens do lexer em JSON (pro realce); sem arquivo, stdin\n"
"  jinga --ast [arq]          A arvore do parser em JSON (pro editor); sem arquivo, stdin\n"
"  jinga --utf16 ...          Com --tokens/--ast/--contexto/--check: colunas em UTF-16\n"
"  jinga --version / -V       Mostra a versao\n"
"  jinga --help / -h          Mostra esta ajuda\n"
"\n"
"Pacotes (lib e comando " PS_EXT "):\n"
"  jpkg install <arq" PS_EXT ">          Instala (o arquivo decide via #!lib / #!cmd)\n"
"  jpkg install <arq" PS_EXT "> -asLib   Forca lib importavel (import nome)\n"
"  jpkg install <nome>            Busca <nome> no registry configurado\n"
"  jpkg uninstall <nome>          Remove (acha sozinho: comando ou lib)\n"
"  jpkg uninstall <nome> -asLib   Forca a categoria lib\n"
"  jpkg list                      Lista comandos e libs instalados\n"
"  jpkg registry set-url <url>    Configura o indice de pacotes\n"
"  jpkg registry show             Mostra o registry configurado\n"
"\n"
"`pool` e `psl` sao os nomes antigos dos mesmos comandos e continuam valendo.\n"
"\n"
"Libs internas: %s\n"
"\n"
"Docs: " SPEC_URL "\n", PS_VERSAO, ps_modulos_publicos());
}

/* Lê o arquivo inteiro. Devolve NULL e reclama no stderr se não der.
 *
 * Todo arquivo que chega pela linha de comando passa aqui — o que se roda, o
 * `--check`, o `-o` e o `--debug` (que reescreve o argv e cai no caminho de
 * rodar). Por isso a recusa da extensão velha mora aqui, uma vez só: o nome
 * ainda existe no disco, então abrir e falhar lá dentro com "token inesperado"
 * ou "não achei o módulo" seria pior que dizer o conserto. Qualquer outra
 * extensão continua rodando como sempre — a recusa é só pras três de antes. */
static char *le_arquivo(const char *caminho, size_t *tam)
{
    const char *velha = ps_ext_velha(caminho);
    if (velha) {
        fprintf(stderr,
            "jinga: '%s' usa a extensao %s, que a linguagem nao usa mais.\n"
            "      Hoje o arquivo da linguagem e " PS_EXT ".\n"
            "      Pra converter uma pasta inteira (arquivos, referencias e libs):\n"
            "          jinga scripts/migra_pr" PS_EXT " <pasta> --aplica --libs\n",
            caminho, velha);
        return NULL;
    }
    FILE *f = fopen(caminho, "rb");
    if (!f) {
        fprintf(stderr, "jinga: nao consegui abrir '%s'\n", caminho);
        return NULL;
    }
    /* Lê em laço, sem `fseek`. O tamanho de antes vinha de `fseek`+`ftell`, que
     * só funciona em arquivo comum: `jinga /dev/stdin` e qualquer redirecionamento
     * de pipe caíam no `return NULL` e o binário saía 66 SEM dizer nada. */
    size_t cap = 65536, n = 0;
    char *buf = malloc(cap);
    if (!buf) { fclose(f); fprintf(stderr, "jinga: sem memoria\n"); return NULL; }
    for (;;) {
        size_t r = fread(buf + n, 1, cap - n - 1, f);
        n += r;
        if (r == 0) break;
        if (n + 1 >= cap) {
            cap *= 2;
            char *nb = realloc(buf, cap);
            if (!nb) { free(buf); fclose(f); fprintf(stderr, "jinga: sem memoria\n"); return NULL; }
            buf = nb;
        }
    }
    int falhou = ferror(f);
    fclose(f);
    if (falhou) { free(buf); fprintf(stderr, "jinga: erro lendo '%s'\n", caminho); return NULL; }
    buf[n] = '\0';
    *tam = n;
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
static void mostra_linha(const char *buf, size_t l, int col)
{
    while (l > 0 && (buf[l-1] == '\n' || buf[l-1] == '\r')) l--;
    size_t sp;
    if (col > 0) sp = (size_t)(col - 1);
    else { sp = 0; while (sp < l && (buf[sp] == ' ' || buf[sp] == '\t')) sp++; }
    fprintf(stderr, "  | %.*s\n  | %*s^^^\n", (int)l, buf, (int)sp, "");
}

static void imprime_quadro(const char *origem, int linha, int col)
{
    char nome_buf[1024];
    fprintf(stderr, "  em %s, linha %d\n", limpa_arquivo(origem, nome_buf, sizeof(nome_buf)), linha);
    if (linha <= 0 || !origem || origem[0] == '<') return;
    /* Executavel gerado: a linha vem do fonte EMBUTIDO. Abrir `origem` no
     * disco lia a linha N do proprio ELF (quando `origem` era o binario) ou
     * nada (o `.pr` nao esta na maquina de quem roda). */
    size_t tam = 0;
    const char *emb = ps_emb_busca(origem, &tam);
    if (emb) {
        const char *p = emb, *fim = emb + tam;
        int atual = 1;
        while (atual < linha) {
            const char *nl = memchr(p, '\n', (size_t)(fim - p));
            if (!nl) return;                    /* o fonte acaba antes da linha */
            p = nl + 1; atual++;
        }
        if (p > fim) return;
        const char *nl = memchr(p, '\n', (size_t)(fim - p));
        mostra_linha(p, nl ? (size_t)(nl - p) : (size_t)(fim - p), col);
        return;
    }
    FILE *f = fopen(origem, "rb");
    if (!f) return;
    char buf[4096];
    int atual = 0;
    while (fgets(buf, sizeof(buf), f)) {
        if (++atual != linha) continue;
        mostra_linha(buf, strlen(buf), col);
        break;
    }
    fclose(f);
}

/* Erro do usuário sai EXATAMENTE como o interpretador (a autoridade): a
 * mensagem primeiro, depois o traceback do mais interno ao mais externo, e o
 * quadro do erro por último. O header só aparece quando há chamadores. */
static int reporta(PSErroExec *e, const char *origem)
{
    switch (e->tipo) {
        case PS_ERRO_TIPO: {
            /* A tipagem estática acha TODOS os erros do arquivo antes de
             * rodar, e todos saem: consertar um e descobrir o próximo só na
             * rodada seguinte é o que a conferência antes de rodar existe pra
             * evitar. */
            int n = e->ntipos > 0 ? e->ntipos : 0;
            for (int i = 0; i < n; i++) {
                if (i) fputc('\n', stderr);
                fprintf(stderr, "%s: %s\n", e->tipos[i].classe, e->tipos[i].msg);
                imprime_quadro(origem, e->tipos[i].linha, e->tipos[i].col);
                /* modulo importado que nao compila: o quadro de DENTRO dele,
                 * depois do quadro do import (como o traceback: o mais fundo
                 * por ultimo) */
                if (e->tipos[i].arquivo[0])
                    imprime_quadro(e->tipos[i].arquivo, e->tipos[i].linha_arq, e->tipos[i].col_arq);
            }
            if (n == 0) {
                fprintf(stderr, "%s: %s\n", e->tipo_nome[0] ? e->tipo_nome : "AttributedValueError", e->msg);
                imprime_quadro(origem, e->linha, e->col);
            }
            if (n > 1)
                fprintf(stderr, "\n%d erros de tipo — o programa nao rodou.\n", n);
            free(e->tipos);
            e->tipos = NULL;
            e->ntipos = 0;
            return 2;
        }
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

/* ── `-o`: gerar um executavel que roda sozinho ──────────────────────────────
 *
 * `jinga programa.pr -o programa` produz um binario que NAO precisa do fonte
 * nem da `jinga` instalada: e uma copia deste mesmo binario com o programa
 * grudado no fim, mais um rodape que diz onde ele comeca.
 *
 * Rodape (os ultimos RODAPE_TAM bytes do arquivo):
 *
 *     [ ...binario da jinga... ][ fonte ][ tamanho do fonte, 16 digitos ][ MAGIA ]
 *
 * Na partida, o binario le o proprio arquivo, ve a magia no fim e, se estiver
 * la, roda o fonte embutido em vez de olhar os argumentos. Assim o mesmo
 * executavel serve de compilador e de programa compilado, sem toolchain
 * nenhuma no meio — nao ha compilador de C na maquina de quem so quer rodar.
 *
 * Compilar a partir de um binario JA compilado corta o payload velho antes de
 * grudar o novo: senao cada geracao carregaria o programa da anterior. */
/* v1 grudava UM fonte. O executavel entao procurava `import banco` no disco
 * da maquina de quem roda — e nao achava ("No module named 'banco'"). v2 leva
 * o principal E todo `.pr` que ele alcanca por import (transitivo, lib
 * instalada inclusive), resolvidos na compilacao exatamente como o import
 * resolve rodando; o import do executavel le desta tabela antes do disco
 * (ps_emb_* no ps_vm.h). Um v1 gerado antes continua rodando como rodava.
 *
 * Payload v2 (antes do rodape):
 *     [16 digitos: n entradas]
 *     entrada: [16 digitos][textual][16 digitos][real][16 digitos][fonte]
 *     entrada 0 = cabecalho: textual = caminho do principal, real = realpath
 *                 dele, fonte = pasta de libs da maquina que compilou
 *     entrada 1 = o principal; 2.. = os modulos */
#define PS_MAGIA_EMB   "PSPOOLEXE1"
#define PS_MAGIA_EMB2  "PSPOOLEXE2"
#define PS_MAGIA_TAM   10
#define PS_DIG_TAM     16
#define PS_RODAPE_TAM  (PS_DIG_TAM + PS_MAGIA_TAM)

/* Caminho do executavel em execucao. */
static int ps_meu_caminho(char *out, size_t cap)
{
    ssize_t n = readlink("/proc/self/exe", out, cap - 1);
    if (n <= 0) return -1;
    out[n] = '\0';
    return 0;
}

/* O payload embutido neste binario, ou NULL. `base` recebe o tamanho do
 * binario SEM o payload (e o que se copia ao compilar de novo); `versao`, 1
 * (um fonte so) ou 2 (principal + modulos). */
static char *ps_payload(const char *caminho, size_t *tam_out, long *base, int *versao)
{
    FILE *f = fopen(caminho, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long fim = ftell(f);
    if (base) *base = fim;
    if (fim < PS_RODAPE_TAM) { fclose(f); return NULL; }

    char rodape[PS_RODAPE_TAM + 1];
    if (fseek(f, fim - PS_RODAPE_TAM, SEEK_SET) != 0
            || fread(rodape, 1, PS_RODAPE_TAM, f) != PS_RODAPE_TAM) { fclose(f); return NULL; }
    rodape[PS_RODAPE_TAM] = '\0';
    int v = 0;
    if (memcmp(rodape + PS_DIG_TAM, PS_MAGIA_EMB, PS_MAGIA_TAM) == 0)       v = 1;
    else if (memcmp(rodape + PS_DIG_TAM, PS_MAGIA_EMB2, PS_MAGIA_TAM) == 0) v = 2;
    if (!v) { fclose(f); return NULL; }
    if (versao) *versao = v;

    char dig[PS_DIG_TAM + 1];
    memcpy(dig, rodape, PS_DIG_TAM); dig[PS_DIG_TAM] = '\0';
    char *fimp = NULL;
    long tam = strtol(dig, &fimp, 10);
    if (!fimp || *fimp != '\0' || tam <= 0 || tam > fim - PS_RODAPE_TAM) { fclose(f); return NULL; }

    long inicio = fim - PS_RODAPE_TAM - tam;
    char *buf = malloc((size_t)tam + 1);
    if (!buf) { fclose(f); return NULL; }
    if (fseek(f, inicio, SEEK_SET) != 0 || fread(buf, 1, (size_t)tam, f) != (size_t)tam) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);
    buf[tam] = '\0';
    if (tam_out) *tam_out = (size_t)tam;
    if (base) *base = inicio;          /* o binario limpo termina aqui */
    return buf;
}

/* Um campo do payload v2: 16 digitos com o tamanho, e os bytes. */
static int escreve_campo(FILE *out, const char *dado, size_t tam)
{
    char dig[PS_DIG_TAM + 1];
    snprintf(dig, sizeof(dig), "%0*zu", PS_DIG_TAM, tam);
    return fwrite(dig, 1, PS_DIG_TAM, out) == PS_DIG_TAM && fwrite(dado, 1, tam, out) == tam;
}

/* 16 digitos -> tamanho. -1 = nao e numero (payload corrompido). */
static int le16(const char *p, size_t resta, size_t *v)
{
    if (resta < PS_DIG_TAM) return -1;
    char d[PS_DIG_TAM + 1];
    memcpy(d, p, PS_DIG_TAM); d[PS_DIG_TAM] = '\0';
    char *fim = NULL;
    unsigned long long x = strtoull(d, &fim, 10);
    if (!fim || *fim) return -1;
    *v = (size_t)x;
    return 0;
}

/* Le o payload v2: registra cada fonte na tabela de embutidos da VM, ancora a
 * pasta do script e a de libs no que a maquina que compilou tinha, e devolve
 * o fonte do principal (que vive na tabela). -1 = corrompido. */
static int emb_carrega(const char *emb, size_t tam, char *main_textual, size_t cap,
                       const char **fonte_main, size_t *tam_main)
{
    size_t p = 0, n = 0;
    *fonte_main = NULL;
    if (le16(emb, tam, &n) != 0) return -1;
    p += PS_DIG_TAM;
    for (size_t i = 0; i < n; i++) {
        size_t lt = 0, lr = 0, lf = 0;
        if (le16(emb + p, tam - p, &lt) != 0) return -1;
        p += PS_DIG_TAM; if (p + lt > tam) return -1;
        const char *t = emb + p; p += lt;
        if (le16(emb + p, tam - p, &lr) != 0) return -1;
        p += PS_DIG_TAM; if (p + lr > tam) return -1;
        const char *r = emb + p; p += lr;
        if (le16(emb + p, tam - p, &lf) != 0) return -1;
        p += PS_DIG_TAM; if (p + lf > tam) return -1;
        const char *f = emb + p; p += lf;
        char tb[1024], rb[1024];
        snprintf(tb, sizeof(tb), "%.*s", (int)lt, t);
        snprintf(rb, sizeof(rb), "%.*s", (int)lr, r);
        if (i == 0) {                                   /* cabecalho */
            snprintf(main_textual, cap, "%s", tb);
            ps_emb_raiz(tb);
            char lb[600];
            snprintf(lb, sizeof(lb), "%.*s", (int)lf, f);
            ps_emb_libs(lb);
            continue;
        }
        ps_emb_poe(tb, rb, f, lf);
        if (i == 1) *fonte_main = ps_emb_busca(tb, tam_main);
    }
    return *fonte_main ? 0 : -1;
}

/* `jinga fonte.pr -o saida` */
static int cmd_compila(const char *fonte_arq, const char *saida)
{
    size_t tam = 0;
    char *fonte = le_arquivo(fonte_arq, &tam);
    if (!fonte) return 66;

    /* Nao gera executavel que ja nasce quebrado: o principal e cada modulo
     * que ele alcanca passam pela conferencia inteira (lexer, parser e a
     * tipagem estatica) — o mesmo `--check` — e modulo que nao se acha e erro
     * aqui, nao na maquina de quem roda. */
    PSEmbutido *deps = NULL;
    int32_t ndeps = 0;
    char libs[600], erro_dep[1400];
    if (ps_embute_deps(fonte_arq, &deps, &ndeps, libs, sizeof(libs), erro_dep, sizeof(erro_dep)) != 0) {
        fprintf(stderr, "jinga: %s\n", erro_dep);
        free(fonte);
        return 65;
    }
    for (int32_t i = 0; i < ndeps; i++) {
        PSErroExec ve;
        if (ps_verifica_fonte(deps[i].fonte, deps[i].tam, deps[i].textual, &ve, NULL, NULL) != 0) {
            reporta(&ve, deps[i].textual);
            fprintf(stderr, "jinga: %s nao compila — nada gerado\n", deps[i].textual);
            ps_embutidos_solta(deps, ndeps);
            free(fonte);
            return 65;
        }
    }

    char meu[4096];
    if (ps_meu_caminho(meu, sizeof(meu)) != 0) {
        fprintf(stderr, "jinga: nao consegui achar o proprio executavel\n");
        ps_embutidos_solta(deps, ndeps); free(fonte); return 70;
    }
    long base = 0;
    char *velho = ps_payload(meu, NULL, &base, NULL);   /* corta payload anterior */
    free(velho);

    FILE *in = fopen(meu, "rb");
    if (!in) { fprintf(stderr, "jinga: nao consegui ler %s\n", meu); ps_embutidos_solta(deps, ndeps); free(fonte); return 70; }
    FILE *out = fopen(saida, "wb");
    if (!out) {
        fprintf(stderr, "jinga: nao consegui escrever %s\n", saida);
        fclose(in); ps_embutidos_solta(deps, ndeps); free(fonte); return 73;
    }
    char buf[65536];
    long resta = base;
    while (resta > 0) {
        size_t quer = (size_t)(resta < (long)sizeof(buf) ? resta : (long)sizeof(buf));
        size_t lidos = fread(buf, 1, quer, in);
        if (lidos == 0) break;
        if (fwrite(buf, 1, lidos, out) != lidos) {
            fprintf(stderr, "jinga: escrita incompleta em %s\n", saida);
            fclose(in); fclose(out); ps_embutidos_solta(deps, ndeps); free(fonte); return 73;
        }
        resta -= (long)lidos;
    }
    fclose(in);

    /* payload v2: cabecalho + o principal + os modulos (ver o formato acima) */
    size_t total = PS_DIG_TAM
                 + 3 * PS_DIG_TAM + strlen(deps[0].textual) + strlen(deps[0].real) + strlen(libs);
    for (int32_t i = 0; i < ndeps; i++)
        total += 3 * PS_DIG_TAM + strlen(deps[i].textual) + strlen(deps[i].real) + deps[i].tam;
    char dig[PS_DIG_TAM + 1];
    snprintf(dig, sizeof(dig), "%0*zu", PS_DIG_TAM, (size_t)ndeps + 1);
    int ok = fwrite(dig, 1, PS_DIG_TAM, out) == PS_DIG_TAM
          && escreve_campo(out, deps[0].textual, strlen(deps[0].textual))
          && escreve_campo(out, deps[0].real, strlen(deps[0].real))
          && escreve_campo(out, libs, strlen(libs));
    for (int32_t i = 0; ok && i < ndeps; i++)
        ok = escreve_campo(out, deps[i].textual, strlen(deps[i].textual))
          && escreve_campo(out, deps[i].real, strlen(deps[i].real))
          && escreve_campo(out, deps[i].fonte, deps[i].tam);
    char rodape[PS_RODAPE_TAM + 1];
    snprintf(rodape, sizeof(rodape), "%0*zu%s", PS_DIG_TAM, total, PS_MAGIA_EMB2);
    if (!ok || fwrite(rodape, 1, PS_RODAPE_TAM, out) != PS_RODAPE_TAM) {
        fprintf(stderr, "jinga: escrita incompleta em %s\n", saida);
        fclose(out); ps_embutidos_solta(deps, ndeps); free(fonte); return 73;
    }
    fclose(out);
    int32_t nmod = ndeps - 1;
    ps_embutidos_solta(deps, ndeps);
    free(fonte);
    if (chmod(saida, 0755) != 0) {
        fprintf(stderr, "jinga: gerado, mas nao consegui dar permissao de execucao a %s\n", saida);
        return 73;
    }
    if (nmod > 0) printf("gerado: %s (%d modulo%s embutido%s)\n", saida, nmod, nmod == 1 ? "" : "s", nmod == 1 ? "" : "s");
    else          printf("gerado: %s\n", saida);
    return 0;
}

/* `build`: roda todos os arquivos da linguagem da pasta atual, em ordem, e
 * conta OK/erro. */
static int cmd_build(void)
{
    DIR *d = opendir(".");
    if (!d) { fprintf(stderr, "jinga: nao consegui abrir a pasta atual\n"); return 1; }

    /* coleta os arquivos da linguagem e ordena */
    char **nomes = NULL; int n = 0, cap = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (!ps_eh_fonte(ent->d_name)) continue;
        if (n == cap) {
            cap = cap ? cap * 2 : 16;
            /* Por temporária: `x = realloc(x, …)` que falha devolve NULL SEM
             * liberar o bloco antigo — perde-se a única referência a ele, e a
             * linha seguinte escreve em NULL. */
            char **novo = realloc(nomes, sizeof(char *) * (size_t)cap);
            if (!novo) {
                for (int i = 0; i < n; i++) free(nomes[i]);
                free(nomes); closedir(d);
                fprintf(stderr, "sem memoria listando a pasta\n");
                return 1;
            }
            nomes = novo;
        }
        nomes[n++] = strdup(ent->d_name);
    }
    closedir(d);
    if (n == 0) { fprintf(stderr, "nenhum arquivo " PS_EXT " encontrado na pasta atual\n"); free(nomes); return 1; }
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

/* CÓDIGO DE SAÍDA do `--check`: 0 quando compila, 1 quando não.
 *
 * Ele saía 0 SEMPRE, inclusive imprimindo `{"ok":false}`. Isso torna
 * `jinga --check f.pr || exit 1` e `set -e` falso verde — e o `--check` é o que
 * o editor roda a cada tecla, o que a CI roda em arquivo que veio de fora, e o
 * que o `jpkg install` roda em pacote de TERCEIRO.
 *
 * É a convenção de todo conferidor de sintaxe: o código de saída diz se
 * passou. O JSON no stdout não muda — quem lê o JSON continua lendo igual;
 * quem lê o rc passa a poder confiar nele.
 *
 * Erro de LEITURA (arquivo que não abre, sem memória) também sai != 0: não
 * conseguir conferir não é conferir e aprovar. */
#define CHECK_RC_FALHA 1

/* ── `--utf16`: a coluna na conta do editor ──────────────────────────────
 *
 * O motor conta CARACTERE (code point); o protocolo LSP e o JavaScript contam
 * unidades UTF-16, onde todo caractere fora do plano básico vale 2. Pra texto
 * acentuado as duas contagens coincidem; com emoji ou ideograma estendido o
 * cursor do editor caía uma casa à frente a cada um deles. Com `--utf16` o
 * motor entrega a conta do editor, na entrada e na saída; sem a opção, nada
 * muda. */
static int g_utf16 = 0;
static const char *g_fonte = NULL;
static size_t g_fonte_tam = 0;

/* Byte onde a linha começa (1-based). O fim do fonte se a linha não existe. */
static size_t off_da_linha(int linha)
{
    if (!g_fonte) return 0;
    int ln = 1;
    for (size_t k = 0; k < g_fonte_tam; k++) {
        if (ln == linha) return k;
        if (g_fonte[k] == '\n') ln++;
    }
    return g_fonte_tam;
}

/* Quantas unidades UTF-16 o caractere que começa em `b` ocupa: 2 só fora do
 * plano básico (o par substituto), 1 no resto. */
static int u16_do_char(unsigned char b) { return b >= 0xF0 ? 2 : 1; }

/* coluna em CARACTERES -> coluna em unidades UTF-16 */
static int col_para_utf16(int linha, int col)
{
    if (!g_utf16 || col <= 1 || !g_fonte) return col;
    size_t k = off_da_linha(linha);
    int cp = 1, u = 1;
    while (k < g_fonte_tam && g_fonte[k] != '\n' && cp < col) {
        u += u16_do_char((unsigned char)g_fonte[k]);
        k++;
        while (k < g_fonte_tam && ((unsigned char)g_fonte[k] & 0xC0) == 0x80) k++;
        cp++;
    }
    return u;
}

/* coluna em unidades UTF-16 -> coluna em CARACTERES (o que o motor usa) */
static int col_de_utf16(int linha, int col)
{
    if (!g_utf16 || col <= 1 || !g_fonte) return col;
    size_t k = off_da_linha(linha);
    int cp = 1, u = 1;
    while (k < g_fonte_tam && g_fonte[k] != '\n' && u < col) {
        u += u16_do_char((unsigned char)g_fonte[k]);
        k++;
        while (k < g_fonte_tam && ((unsigned char)g_fonte[k] & 0xC0) == 0x80) k++;
        cp++;
    }
    return cp;
}

/* TAMANHO do token (`n`, em caracteres) -> o mesmo trecho em unidades UTF-16.
 * Com `--utf16` a coluna já saía na conta do editor, mas o `n` ficava em
 * caractere: no mesmo token, `c2 - c` dava 4 e `n` dava 3, e quem pinta pelo
 * `n` errava uma casa por caractere fora do plano básico. Anda o trecho real
 * no fonte a partir de (linha, col), atravessando quebra de linha (que vale 1
 * nas duas contas) — então vale também pra string e comentário de várias
 * linhas. */
static int n_para_utf16(int linha, int col, int n)
{
    if (!g_utf16 || n <= 0 || !g_fonte) return n;
    size_t k = off_da_linha(linha);
    for (int cp = 1; cp < col && k < g_fonte_tam && g_fonte[k] != '\n'; cp++) {
        k++;
        while (k < g_fonte_tam && ((unsigned char)g_fonte[k] & 0xC0) == 0x80) k++;
    }
    int u = 0;
    for (int i = 0; i < n && k < g_fonte_tam; i++) {
        u += u16_do_char((unsigned char)g_fonte[k]);
        k++;
        while (k < g_fonte_tam && ((unsigned char)g_fonte[k] & 0xC0) == 0x80) k++;
    }
    return u;
}

/* `jinga --check [arquivo.pr]` — lexer/parser/compilador da VM, SEM rodar, com
 * o resultado em JSON pro editor. Sem arquivo, lê o buffer do stdin (o editor
 * manda o conteúdo não salvo). NUNCA executa o código.
 *
 * `--path <caminho>` diz de QUAL arquivo é o buffer do stdin. Sem ele, o
 * motor não sabe onde o arquivo mora, não acha os módulos vizinhos e pula a
 * conferência entre arquivos CALADO — que é justamente o modo que o editor
 * usa. Com ele, `import util` ao lado resolve igual ao arquivo salvo. */
static int cmd_check(const char *arquivo, const char *como)
{
    size_t tam = 0;
    char *fonte = NULL;
    if (arquivo) {
        fonte = le_arquivo(arquivo, &tam);
        if (!fonte) { printf("{\"ok\":false,\"tipo\":\"IOError\",\"msg\":\"nao consegui abrir o arquivo\",\"linha\":1,\"coluna\":1}\n"); return CHECK_RC_FALHA; }
    } else {
        size_t cap = 65536; tam = 0;
        fonte = malloc(cap);
        if (!fonte) { printf("{\"ok\":false,\"tipo\":\"MemoryError\",\"msg\":\"sem memoria\",\"linha\":1,\"coluna\":1}\n"); return CHECK_RC_FALHA; }
        size_t r;
        while ((r = fread(fonte + tam, 1, cap - tam, stdin)) > 0) {
            tam += r;
            if (tam == cap) { cap *= 2; char *nb = realloc(fonte, cap); if (!nb) { free(fonte); printf("{\"ok\":false,\"tipo\":\"MemoryError\",\"msg\":\"sem memoria\",\"linha\":1,\"coluna\":1}\n"); return CHECK_RC_FALHA; } fonte = nb; }
        }
        fonte[tam] = '\0';
    }

    PSErroExec e;
    PSAviso *avisos = NULL;
    int32_t navisos = 0;
    int rc = ps_verifica_fonte(fonte, tam, arquivo ? arquivo : como, &e, &avisos, &navisos);
    /* o fonte fica vivo até o fim: com `--utf16` a conversão de coluna precisa
     * dele pra saber onde estão os caracteres fora do plano básico */
    g_fonte = fonte; g_fonte_tam = tam;

    /* Os AVISOS entram no mesmo JSON, e entram tanto no caso `ok` quanto no de
     * erro: um `\p` que nao e escape nao impede o programa de compilar, mas o
     * editor tem que poder sublinhar. Sem isto o aviso so existiria pra quem
     * roda no terminal — e o editor e onde ele seria visto. */
    if (rc == 0) {
        printf("{\"ok\":true");
        if (navisos > 0) {
            printf(",\"avisos\":[");
            for (int32_t i = 0; i < navisos; i++) {
                if (i) putchar(',');
                printf("{\"msg\":");
                json_str(avisos[i].msg);
                printf(",\"linha\":%d,\"coluna\":%d}", avisos[i].linha,
                       col_para_utf16(avisos[i].linha, avisos[i].col));
            }
            putchar(']');
        }
        printf("}\n");
        free(avisos);
        free(fonte);
        return 0;
    }
    free(avisos);

    const char *tipo = e.tipo == PS_ERRO_SINTAXE ? "SyntaxError"
                     : e.tipo == PS_ERRO_NAO_SUPORTADO ? "NotImplementedError"
                     : e.tipo == PS_ERRO_MEMORIA ? "MemoryError"
                     : e.tipo == PS_ERRO_TIPO && e.tipo_nome[0] ? e.tipo_nome : "RuntimeError";
    printf("{\"ok\":false,\"tipo\":");
    json_str(tipo);
    printf(",\"msg\":");
    json_str(e.msg);
    printf(",\"linha\":%d,\"coluna\":%d", e.linha, col_para_utf16(e.linha, e.col));
    /* A lista INTEIRA (o primeiro também vai nos campos de cima, que o editor
     * já lia): erros de tipo, e agora também os de SINTAXE — o parser não para
     * mais no primeiro, então dá pra sublinhar todos de uma vez. */
    if ((e.tipo == PS_ERRO_TIPO || e.tipo == PS_ERRO_SINTAXE) && e.ntipos > 0) {
        printf(",\"erros\":[");
        for (int i = 0; i < e.ntipos; i++) {
            if (i) putchar(',');
            printf("{\"tipo\":");
            json_str(e.tipos[i].classe);
            printf(",\"msg\":");
            json_str(e.tipos[i].msg);
            printf(",\"linha\":%d,\"coluna\":%d", e.tipos[i].linha,
                   col_para_utf16(e.tipos[i].linha, e.tipos[i].col));
            /* o erro esta em outro arquivo (modulo importado que nao compila) */
            if (e.tipos[i].arquivo[0]) {
                printf(",\"arquivo\":");
                json_str(e.tipos[i].arquivo);
                printf(",\"linha_arquivo\":%d,\"coluna_arquivo\":%d", e.tipos[i].linha_arq, e.tipos[i].col_arq);
            }
            putchar('}');
        }
        putchar(']');
    }
    printf("}\n");
    free(e.tipos);
    free(fonte);
    return CHECK_RC_FALHA;
}

/* ── `jinga --contexto <linha>:<coluna>` ───────────────────────────────────
 *
 * O que o editor precisa saber pra completar: o que vem antes do cursor.
 * Responde com o LEXER de verdade, não com regex.
 *
 * A extensão adivinhava isso com expressão regular sobre o texto da linha, e
 * errava o previsível: regex não sabe o que é string (`"a.b".up`), não fecha
 * parêntese aninhado (`f(a, b).x`) e quebrava com um nome parcial depois do
 * ponto — `jinker.request.ge` deixava de casar, caía no completion de topo e
 * oferecia módulo e builtin DEPOIS de um ponto.
 *
 * Lê o fonte do stdin (o editor manda o buffer não salvo, igual ao --check) e
 * devolve JSON:
 *
 *   {"contexto":"membro","receptor":"jinker.request","parcial":"ge"}
 *   {"contexto":"topo","parcial":"gath"}
 *   {"contexto":"decorador","parcial":"rou","receptor":"app"}
 *   {"contexto":"import","parcial":"os"}
 *
 * NUNCA executa o código — só tokeniza. */
static char *le_stdin_todo(size_t *tam)
{
    size_t cap = 65536; *tam = 0;
    char *b = malloc(cap);
    if (!b) return NULL;
    size_t r;
    while ((r = fread(b + *tam, 1, cap - *tam, stdin)) > 0) {
        *tam += r;
        if (*tam == cap) {
            cap *= 2;
            char *nb = realloc(b, cap);
            if (!nb) { free(b); return NULL; }
            b = nb;
        }
    }
    b[*tam] = '\0';
    return b;
}

/* O fonte de um comando de editor: o ARQUIVO quando veio caminho, senão o
 * buffer do stdin (o que o editor ainda não salvou). Os dois modos existem
 * porque o editor precisa dos dois: conferir o arquivo do disco e conferir o
 * que está na tela. */
static char *le_fonte_editor(const char *arquivo, size_t *tam);

static char *le_fonte_editor(const char *arquivo, size_t *tam)
{
    char *f = arquivo ? le_arquivo(arquivo, tam) : le_stdin_todo(tam);
    /* o fonte fica visível pra conversão de coluna (`--utf16`) */
    g_fonte = f; g_fonte_tam = f ? *tam : 0;
    return f;
}

/* Quantos CARACTERES o token ocupa no fonte. O lexer mede em `nchars`; quando
 * não mediu, conta os caracteres do texto. Pontuação sem texto ocupa 1.
 *
 * Tudo que compara posição com o cursor passa por aqui: a coluna do lexer
 * conta CARACTERE, e somar `texto_len` (BYTES) desalinhava tudo depois de um
 * acento — `f("é", os.` devolvia `" o"` como receptor. */
static int tok_chars(const PSToken *t)
{
    if (t->nchars > 0) return t->nchars;
    if (!t->texto) return 1;
    int n = 0;
    for (int32_t k = 0; k < t->texto_len; k++)
        if (((unsigned char)t->texto[k] & 0xC0) != 0x80) n++;
    return n > 0 ? n : 1;
}

/* Índice do último token que TERMINA em ou antes de (linha, col). -1 se não há.
 * O lexer dá o início do token; o fim sai do texto (ou de 1 pra pontuação). */
static int tok_antes(const PSTokenList *tl, int linha, int col)
{
    int achado = -1;
    for (int i = 0; i < tl->n; i++) {
        const PSToken *t = &tl->tokens[i];
        if (t->type == T_NEWLINE || t->type == T_INDENT
            || t->type == T_DEDENT || t->type == T_EOF) continue;
        if (t->type == T_COMMENT) continue;
        if (t->line > linha) break;
        if (t->line < linha) { achado = i; continue; }
        int fim = t->col + tok_chars(t);
        if (fim <= col) achado = i;
    }
    return achado;
}

/* Anda pra trás montando a cadeia receptora a partir de `i` (que já é o token
 * ANTES do ponto). Devolve a posição do primeiro token da cadeia, ou -1 se o
 * receptor não é nomeável (literal, por exemplo). */
static int inicio_da_cadeia(const PSTokenList *tl, int i)
{
    int ini = -1;
    for (;;) {
        if (i < 0) break;
        PSTokType t = tl->tokens[i].type;
        if (t == T_RPAREN || t == T_RBRACK) {
            PSTokType abre = (t == T_RPAREN) ? T_LPAREN : T_LBRACK;
            int d = 0;
            for (i--; i >= 0; i--) {
                if (tl->tokens[i].type == t) d++;
                else if (tl->tokens[i].type == abre) { if (d == 0) { i--; break; } d--; }
            }
            continue;                     /* antes do grupo tem que vir o nome */
        }
        if (t == T_IDENT || t == T_IDENT_UPPER || t == T_KW) {
            ini = i;
            if (i >= 2 && tl->tokens[i - 1].type == T_DOT) { i -= 2; continue; }
            break;
        }
        break;                            /* literal, operador: não é receptor */
    }
    return ini;
}

static void jsonf(const char *chave, const char *valor)
{
    printf(",\"%s\":", chave);
    json_str(valor);
}

/* `jinga --tokens` — o fonte vem pelo stdin e sai a lista de tokens do LEXER
 * DE VERDADE, em JSON:
 *
 *   [{"t":"KW","l":1,"c":1,"n":5,"v":"funct"}, ...]
 *
 * É o que dá realce ao editor sem existir uma segunda gramática pra divergir
 * do motor. `n` é o comprimento em CARACTERES (o editor conta caractere, não
 * byte); INDENT/DEDENT/NEWLINE saem com `n` 0, porque não ocupam texto — quem
 * pinta ignora, quem precisa da ESTRUTURA (converter de bloco, por exemplo)
 * usa. Comentário entra como "COMMENT" — o compilador descarta, o realce
 * precisa. NUNCA executa o código: só tokeniza.
 *
 * A saída é um OBJETO, `{"tokens":[…],"erros":[…]}`: era um array puro, e com
 * erro de lexer vinha `[]` — o arquivo inteiro perdia a cor por causa de um
 * caractere. Agora o trecho ruim vira um token `ERRO`, os tokens das linhas
 * boas continuam vindo, e `erros` (só quando há) diz o que sublinhar. */
static int json_escapa(FILE *f, const char *s, int n)
{
    for (int i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '"' || c == '\\') fprintf(f, "\\%c", c);
        else if (c == '\n') fputs("\\n", f);
        else if (c == '\r') fputs("\\r", f);
        else if (c == '\t') fputs("\\t", f);
        else if (c < 0x20) fprintf(f, "\\u%04x", c);
        else fputc(c, f);
    }
    return 0;
}

static int cmd_tokens(const char *arquivo)
{
    size_t tam = 0;
    char *fonte = le_fonte_editor(arquivo, &tam);
    if (!fonte) { printf("[]\n"); return 1; }
    /* Modo de RECUPERAÇÃO: o trecho que o lexer não entendeu vira um token
     * `ERRO` e a análise segue. Antes, um caractere estranho no meio do
     * arquivo fazia isto devolver `[]` — e como o arquivo passa a maior parte
     * do tempo inválido enquanto se digita, a tela inteira perdia a cor. Os
     * erros saem em `erros`, depois da lista. */
    PSTokenList *tl = ps_lexer_tokenize_modo(fonte, tam, 1, 1);
    if (!tl) { free(fonte); printf("[]\n"); return 1; }

    fputs("{\"tokens\":[", stdout);
    int primeiro = 1;
    for (int32_t i = 0; i < tl->n; i++) {
        const PSToken *t = &tl->tokens[i];
        if (t->type == T_EOF) continue;
        /* quanto o token ocupa NO FONTE (com aspas, com prefixo `f`), em
         * caracteres. Só cai no texto quando o lexer não mediu o span. */
        int nch = t->nchars;
        if (nch <= 0 && t->texto) {
            for (int k = 0; k < t->texto_len; k++)
                if (((unsigned char)t->texto[k] & 0xC0) != 0x80) nch++;
        }
        if (!primeiro) fputc(',', stdout);
        primeiro = 0;
        printf("{\"t\":\"%s\",\"l\":%d,\"c\":%d,\"n\":%d",
               ps_tok_nome(t->type), t->line, col_para_utf16(t->line, t->col),
               n_para_utf16(t->line, t->col, nch));
        /* onde ele TERMINA: é o que permite sublinhar e dobrar string de
         * várias linhas e comentário de bloco, que `n` não mede */
        if (t->linha_fim)
            printf(",\"l2\":%d,\"c2\":%d",
                   t->linha_fim, col_para_utf16(t->linha_fim, t->col_fim));
        /* O INTERVALO de cada `{...}` da f-string, no próprio token dela.
         * Sem ele o editor tem os tokens de dentro (abaixo) mas não o que
         * está ENTRE eles: o espaço depois do `{` e o pedaço ainda vazio
         * enquanto se digita voltavam a valer como texto livre, e o
         * completion não abria em `f"{ ` nem logo depois do `{`. A regra de
         * onde a interpolação começa e acaba continua sendo só do lexer. */
        if (t->type == T_FSTRING) {
            int primeira_int = 1;
            for (int32_t k = 0; k < tl->ninterps; k++) {
                if (tl->interps[k].tok != i) continue;
                size_t off = tl->interps[k].off;
                int32_t len = tl->interps[k].len;
                if (len < 0 || off + (size_t)len > tam) continue;
                int lin = tl->interps[k].linha, cl = tl->interps[k].col;
                for (int32_t z = 0; z < len; z++) {
                    unsigned char ch = (unsigned char)fonte[off + z];
                    if (ch == '\n') { lin++; cl = 1; continue; }
                    if ((ch & 0xC0) != 0x80) cl++;
                }
                fputs(primeira_int ? ",\"interp\":[" : ",", stdout);
                primeira_int = 0;
                printf("{\"l\":%d,\"c\":%d,\"l2\":%d,\"c2\":%d}",
                       tl->interps[k].linha,
                       col_para_utf16(tl->interps[k].linha, tl->interps[k].col),
                       lin, col_para_utf16(lin, cl));
            }
            if (!primeira_int) fputc(']', stdout);
        }
        fputs(",\"v\":\"", stdout);
        if (t->texto) json_escapa(stdout, t->texto, t->texto_len);
        fputs("\"}", stdout);
        /* A F-STRING POR DENTRO: o que está entre `{` e `}` é CÓDIGO, e sai
         * como token, com a linha e a coluna REAIS no fonte. Antes a f-string
         * inteira era um token só: o realce não pintava a interpolação e o
         * editor não sabia que `nome` ali é a variável `nome`. O trecho é
         * re-analisado pelo MESMO lexer — não há segunda gramática. */
        if (t->type == T_FSTRING) {
            for (int32_t k = 0; k < tl->ninterps; k++) {
                if (tl->interps[k].tok != i) continue;
                size_t off = tl->interps[k].off;
                int32_t len = tl->interps[k].len;
                if (len <= 0 || off + (size_t)len > tam) continue;
                PSTokenList *sub = ps_lexer_tokenize_modo(fonte + off, (size_t)len, 1, 1);
                if (!sub) continue;
                for (int32_t q = 0; q < sub->n; q++) {
                    const PSToken *s = &sub->tokens[q];
                    if (s->type == T_EOF || s->type == T_NEWLINE
                        || s->type == T_INDENT || s->type == T_DEDENT) continue;
                    int lin = tl->interps[k].linha + s->line - 1;
                    int col = (s->line == 1) ? tl->interps[k].col + s->col - 1 : s->col;
                    int snch = s->nchars;
                    if (snch <= 0 && s->texto) {
                        snch = 0;
                        for (int32_t z = 0; z < s->texto_len; z++)
                            if (((unsigned char)s->texto[z] & 0xC0) != 0x80) snch++;
                    }
                    fputc(',', stdout);
                    printf("{\"t\":\"%s\",\"l\":%d,\"c\":%d,\"n\":%d,\"em\":\"fstring\",\"v\":\"",
                           ps_tok_nome(s->type), lin, col_para_utf16(lin, col),
                           n_para_utf16(lin, col, snch));
                    if (s->texto) json_escapa(stdout, s->texto, s->texto_len);
                    fputs("\"}", stdout);
                }
                ps_lexer_free(sub);
            }
        }
    }
    fputc(']', stdout);
    if (tl->nerros > 0) {
        printf(",\"erros\":[");
        for (int32_t i = 0; i < tl->nerros; i++) {
            if (i) putchar(',');
            printf("{\"msg\":");
            json_str(tl->erros[i].msg);
            printf(",\"linha\":%d,\"coluna\":%d}", tl->erros[i].linha, tl->erros[i].col);
        }
        putchar(']');
    }
    fputs("}\n", stdout);
    int rc = tl->nerros > 0 ? 1 : 0;
    ps_lexer_free(tl);
    free(fonte);
    return rc;
}

/* ── `--ast`: a ÁRVORE, em JSON, pro editor ──────────────────────────────
 *
 * POR QUE ESTE COMANDO EXISTE.
 *
 * O servidor de linguagem tinha `--tokens` (o léxico), `--metadata` (as
 * tabelas do VM) e `--check` (o erro). Faltava a única coisa que responde
 * "o que está em escopo NESTA linha": a árvore.
 *
 * Sem ela, o servidor casava PADRÃO em cima do texto — uma regex pra `self.`,
 * outra pra `alvo.`, uma varredura à mão pra achar parâmetro, outra pra achar
 * Entity. Cada forma nova que o usuário escrevia era um ramo novo escrito à
 * mão, e por isso sempre faltava um: `a.b.c`, método dentro de classe,
 * variável declarada três linhas acima. Não era análise, era remendo.
 *
 * Quem sabe o que é variável, de quem é o membro e o que está em escopo é o
 * PARSER — o mesmo que compila o programa. Publicar a árvore acaba com a
 * segunda gramática do lado do editor.
 *
 * O emissor é GENÉRICO sobre a struct do nó: percorre `a/b/c/e` e as três
 * listas, sem um caso por tipo. Nó novo na linguagem já sai aqui, sem ninguém
 * tocar neste arquivo — que é a mesma razão de `--metadata` sair das tabelas
 * do VM em vez de uma lista digitada.
 *
 * A LISTA DE ERRO NÃO IMPEDE A ÁRVORE: o editor precisa completar enquanto o
 * arquivo está pela metade (é justamente quando se digita). Com erro de
 * sintaxe sai `{"ok":false,...}` COM a árvore parcial que o parser conseguiu,
 * em vez de vazio.
 */
static void ast_json(FILE *f, const PSNode *n);

static void ast_lista(FILE *f, const char *chave, const PSNodeVec *v, int *virg)
{
    if (v->n == 0) return;
    if (*virg) fputc(',', f);
    *virg = 1;
    fprintf(f, "\"%s\":[", chave);
    for (int32_t i = 0; i < v->n; i++) {
        if (i) fputc(',', f);
        ast_json(f, v->itens[i]);
    }
    fputc(']', f);
}

static void ast_filho(FILE *f, const char *chave, const PSNode *c, int *virg)
{
    if (!c) return;
    if (*virg) fputc(',', f);
    *virg = 1;
    fprintf(f, "\"%s\":", chave);
    ast_json(f, c);
}

static void ast_txt(FILE *f, const char *chave, const char *s, int *virg)
{
    if (!s) return;
    if (*virg) fputc(',', f);
    *virg = 1;
    fprintf(f, "\"%s\":\"", chave);
    json_escapa(f, s, (int)strlen(s));
    fputc('"', f);
}

static void ast_json(FILE *f, const PSNode *n)
{
    if (!n) { fputs("null", f); return; }
    int virg = 0;
    fputc('{', f);
    fprintf(f, "\"k\":\"%s\",\"l\":%d,\"c\":%d", ps_node_nome(n->kind), n->line,
            col_para_utf16(n->line, n->col));
    if (n->linha_fim) fprintf(f, ",\"l2\":%d", n->linha_fim);
    if (n->col_fim)   fprintf(f, ",\"c2\":%d", col_para_utf16(n->linha_fim ? n->linha_fim : n->line, n->col_fim));
    virg = 1;
    ast_txt(f, "texto",  n->texto,  &virg);
    ast_txt(f, "texto2", n->texto2, &virg);
    ast_txt(f, "texto3", n->texto3, &virg);
    ast_txt(f, "estilo", n->estilo, &virg);
    if (n->is_async)   { fputs(",\"async\":true", f); }
    if (n->is_private) { fputs(",\"private\":true", f); }
    if (n->is_static)  { fputs(",\"static\":true", f); }
    if (n->is_nonnull) { fputs(",\"nonnull\":true", f); }
    if (n->i2)         { fprintf(f, ",\"i2\":%d", n->i2); }
    /* O literal diz qual é e traz o VALOR: bytes tem o mesmo `texto` que a
     * string (sem a marca o editor tipava `h = b"q"` como str e oferecia
     * `upper` em vez de `decode`), e número/bool não tinham valor nenhum na
     * árvore — o hover do model (`idade: int(min=18)`) precisa dele. */
    if (n->kind == N_LITERAL) {
        switch (n->lit) {
            case L_INT:   fprintf(f, ",\"lit\":\"int\",\"i\":%lld", (long long)n->i); break;
            case L_FLO:
                if (n->d == n->d && n->d - n->d == 0) fprintf(f, ",\"lit\":\"flo\",\"d\":%.17g", n->d);
                else fputs(",\"lit\":\"flo\"", f);      /* inf/nan não é JSON */
                break;
            case L_BOOL:  fprintf(f, ",\"lit\":\"bool\",\"i\":%d", n->i ? 1 : 0); break;
            case L_NULL:  fputs(",\"lit\":\"null\"", f); break;
            case L_STR:   fputs(",\"lit\":\"str\"", f); break;
            case L_BYTES: fputs(",\"lit\":\"bytes\"", f); break;
            default: break;
        }
    }
    ast_filho(f, "a", n->a, &virg);
    ast_filho(f, "b", n->b, &virg);
    ast_filho(f, "c", n->c, &virg);
    ast_filho(f, "e", n->e, &virg);
    ast_lista(f, "lista",  &n->lista,  &virg);
    ast_lista(f, "lista2", &n->lista2, &virg);
    ast_lista(f, "alias",  &n->lista2_alias, &virg);
    fputc('}', f);
}

static int cmd_ast(const char *arquivo)
{
    size_t tam = 0;
    char *fonte = le_fonte_editor(arquivo, &tam);
    if (!fonte) { printf("{\"ok\":false,\"msg\":\"sem entrada\"}\n"); return 1; }

    /* Modo de RECUPERAÇÃO nos dois: o lexer não para no caractere estranho e o
     * parser não para no primeiro erro de sintaxe — o statement quebrado é
     * registrado e ele pula pro próximo. Sem isso a árvore acabava na linha do
     * erro, e o painel de estrutura do editor esvaziava dali pra baixo
     * justamente enquanto se digita. Os erros saem em `erros`. */
    PSTokenList *tl = NULL;
    PSParseResult *r = ps_parse_fonte(fonte, tam, 1, &tl);
    free(fonte);
    if (!r || !tl) {
        if (r) ps_parse_free(r);
        ps_lexer_free(tl);
        printf("{\"ok\":false,\"msg\":\"sem memoria\",\"arvore\":null}\n"); return 1;
    }

    int nerros = tl->nerros + r->nerros;
    if (nerros > 0) {
        /* o primeiro erro continua nos campos de cima, como o editor já lia */
        const char *msg = tl->nerros > 0 ? tl->erros[0].msg : r->erros[0].msg;
        int32_t l = tl->nerros > 0 ? tl->erros[0].linha : r->erros[0].linha;
        int32_t c = tl->nerros > 0 ? tl->erros[0].col : r->erros[0].col;
        printf("{\"ok\":false,\"tipo\":\"SyntaxError\",\"msg\":\"");
        json_escapa(stdout, msg, (int)strlen(msg));
        printf("\",\"linha\":%d,\"coluna\":%d,\"erros\":[", l, c);
        int primeiro = 1;
        for (int32_t i = 0; i < tl->nerros; i++) {
            if (!primeiro) putchar(',');
            primeiro = 0;
            printf("{\"msg\":");
            json_str(tl->erros[i].msg);
            printf(",\"linha\":%d,\"coluna\":%d}", tl->erros[i].linha, tl->erros[i].col);
        }
        for (int32_t i = 0; i < r->nerros; i++) {
            if (!primeiro) putchar(',');
            primeiro = 0;
            printf("{\"msg\":");
            json_str(r->erros[i].msg);
            printf(",\"linha\":%d,\"coluna\":%d}", r->erros[i].linha, r->erros[i].col);
        }
        printf("],\"arvore\":");
    } else {
        printf("{\"ok\":true,\"arvore\":");
    }
    ast_json(stdout, r->programa);
    fputs("}\n", stdout);
    int rc = nerros > 0 ? 1 : 0;
    ps_lexer_free(tl);
    ps_parse_free(r);
    return rc;
}

/* Deslocamento em BYTES da posição (linha, coluna) do cursor. A coluna conta
 * CARACTERE, como o lexer conta — por isso o `& 0xC0` pula os bytes de
 * continuação do UTF-8. Além do fim, devolve o fim. */
static size_t off_de_pos(const char *fonte, size_t tam, int linha, int col)
{
    int ln = 1, cl = 1;
    for (size_t k = 0; k <= tam; k++) {
        if (ln == linha && cl == col) return k;
        if (k == tam) break;
        if (fonte[k] == '\n') { ln++; cl = 1; }
        else if (((unsigned char)fonte[k] & 0xC0) != 0x80) cl++;
    }
    return tam;
}

/* O token que CONTÉM o cursor — não o anterior a ele. É o que diz "o cursor
 * está dentro de uma string/comentário", onde o editor NÃO pode abrir
 * completion, e o que dá o pedaço já digitado no meio de um nome.
 *
 * Comentário vai até o fim da linha, então o cursor logo depois do último
 * caractere ainda está DENTRO dele; string tem fecho, e depois do fecho o
 * cursor já está fora. */
static int tok_no_cursor(const PSTokenList *tl, int linha, int col)
{
    for (int32_t i = 0; i < tl->n; i++) {
        const PSToken *t = &tl->tokens[i];
        if (t->line != linha) continue;
        if (t->type == T_NEWLINE || t->type == T_INDENT
            || t->type == T_DEDENT || t->type == T_EOF) continue;
        int fim = t->col + tok_chars(t);
        if (col > t->col && (col < fim || (t->type == T_COMMENT && col <= fim)))
            return (int)i;
    }
    return -1;
}

/* Dentro de uma f-string, o que está entre `{` e `}` é CÓDIGO: ali o editor
 * completa como em qualquer outro lugar. Devolve 1 quando o cursor está numa
 * interpolação ABERTA e copia em `expr` o texto do `{` até o cursor. `{{` e
 * `}}` são chave literal e não abrem nada. */
static int fstring_interp(const char *fonte, size_t ini, size_t fim, char *expr, size_t cap)
{
    size_t abriu = 0;
    int dentro = 0;
    for (size_t k = ini; k < fim; k++) {
        if (fonte[k] == '{') {
            if (!dentro && k + 1 < fim && fonte[k + 1] == '{') { k++; continue; }
            if (!dentro) { dentro = 1; abriu = k + 1; }
        } else if (fonte[k] == '}') {
            if (!dentro && k + 1 < fim && fonte[k + 1] == '}') { k++; continue; }
            dentro = 0;
        }
    }
    if (!dentro) return 0;
    size_t n = fim - abriu;
    if (n + 1 > cap) n = cap - 1;
    memcpy(expr, fonte + abriu, n);
    expr[n] = '\0';
    return 1;
}

/* A última linha que a subárvore ocupa: o fim de uma funct ou de uma classe
 * não está no nó dela (só o `Block` de chaves registra `linha_fim`), então sai
 * do maior número de linha lá dentro. */
static int32_t no_linha_max(const PSNode *n)
{
    if (!n) return 0;
    int32_t m = n->line > n->linha_fim ? n->line : n->linha_fim;
    const PSNode *fs[4] = { n->a, n->b, n->c, n->e };
    for (int i = 0; i < 4; i++) { int32_t k = no_linha_max(fs[i]); if (k > m) m = k; }
    const PSNodeVec *vs[3] = { &n->lista, &n->lista2, &n->lista2_alias };
    for (int i = 0; i < 3; i++)
        for (int32_t k = 0; k < vs[i]->n; k++) {
            int32_t q = no_linha_max(vs[i]->itens[k]);
            if (q > m) m = q;
        }
    return m;
}

/* Monta em `buf` a funct/classe que contém a linha, do mais de fora pro mais
 * de dentro: `C.m`. Vazio quando o cursor está no corpo do arquivo.
 *
 * `topo` como contexto quer dizer "sem receptor", não "escopo global": sem
 * isto o editor não sabia se o cursor estava dentro de uma função. */
static void escopo_no(const PSNode *n, int linha, char *buf, size_t cap)
{
    if (!n) return;
    if ((n->kind == N_ACTION_DECL || n->kind == N_ENTITY_DECL) && n->texto
        && n->line <= linha && linha <= no_linha_max(n)) {
        size_t u = strlen(buf);
        if (u + 1 < cap) snprintf(buf + u, cap - u, "%s%s", u ? "." : "", n->texto);
    }
    const PSNode *fs[4] = { n->a, n->b, n->c, n->e };
    for (int i = 0; i < 4; i++) escopo_no(fs[i], linha, buf, cap);
    const PSNodeVec *vs[3] = { &n->lista, &n->lista2, &n->lista2_alias };
    for (int i = 0; i < 3; i++)
        for (int32_t k = 0; k < vs[i]->n; k++) escopo_no(vs[i]->itens[k], linha, buf, cap);
}

/* O que o cursor toca, resolvido sobre uma lista de tokens. Fica separado do
 * comando porque o MESMO cálculo roda de novo, sozinho, sobre o pedaço de
 * código de dentro de uma f-string. */
typedef struct {
    const char *ctx;          /* topo | membro | nenhum | decorador | import | texto | comentario */
    char       *recv;         /* malloc do chamador; NULL quando não há nome */
    const char *tipo;         /* receptor literal: str, int, list… */
    char        parcial[256];
} CtxCursor;

static void contexto_resolve(const char *fonte, size_t tam, const PSTokenList *tl,
                             int linha, int col, CtxCursor *s)
{
    s->ctx = "topo";
    int i = tok_antes(tl, linha, col);

    /* nome parcial: o que já foi digitado ANTES do cursor. Encostado no fim
     * (`ge|`) ou no meio da palavra (`rea|dFile`) — no meio, o editor filtra
     * pelo pedaço da esquerda, que é o que o usuário escreveu. */
    int dentro = tok_no_cursor(tl, linha, col);
    if (dentro >= 0) {
        const PSToken *t = &tl->tokens[dentro];
        if ((t->type == T_IDENT || t->type == T_IDENT_UPPER || t->type == T_KW) && t->texto) {
            int nch = col - t->col;
            int bytes = 0, contados = 0;
            while (bytes < t->texto_len && contados < nch) {
                bytes++;
                while (bytes < t->texto_len && ((unsigned char)t->texto[bytes] & 0xC0) == 0x80) bytes++;
                contados++;
            }
            if (bytes > 0 && (size_t)bytes < sizeof(s->parcial)) {
                memcpy(s->parcial, t->texto, (size_t)bytes);
                s->parcial[bytes] = '\0';
            }
            i = dentro - 1;
        }
    } else if (i >= 0) {
        const PSToken *t = &tl->tokens[i];
        if ((t->type == T_IDENT || t->type == T_IDENT_UPPER || t->type == T_KW)
            && t->line == linha && t->col + tok_chars(t) == col) {
            if (t->texto && (size_t)t->texto_len < sizeof(s->parcial)) {
                memcpy(s->parcial, t->texto, (size_t)t->texto_len);
                s->parcial[t->texto_len] = '\0';
            }
            i--;
        }
    }

    if (i >= 0 && tl->tokens[i].type == T_DOT) {
        int ini = inicio_da_cadeia(tl, i - 1);
        if (ini >= 0) {
            /* fatia o fonte do início da cadeia até o ponto — preserva o texto
             * original (`f(a, b)`), que é o que o resolvedor de tipo espera */
            const PSToken *a = &tl->tokens[ini];
            const PSToken *p = &tl->tokens[i];
            size_t off_a = off_de_pos(fonte, tam, a->line, a->col);
            size_t off_p = off_de_pos(fonte, tam, p->line, p->col);
            if (off_p > off_a) {
                s->recv = malloc(off_p - off_a + 1);
                if (s->recv) {
                    memcpy(s->recv, fonte + off_a, off_p - off_a);
                    s->recv[off_p - off_a] = '\0';
                    /* apara espaço da direita (o `.` pode vir depois de espaço) */
                    for (char *e = s->recv + strlen(s->recv); e > s->recv && (e[-1]==' '||e[-1]=='\t'); e--) e[-1] = '\0';
                    s->ctx = "membro";
                }
            }
        }
        /* Receptor LITERAL (`"a.b".up`, `[1,2].so`): não tem nome pra resolver,
         * mas tem tipo — e tipo tem método. O regex devolvia null aqui e o
         * editor não oferecia nada. */
        if (!s->recv && i >= 1) {
            switch (tl->tokens[i - 1].type) {
                case T_STR: case T_FSTRING: s->tipo = "str";  break;
                case T_BYTES:               s->tipo = "byte"; break;
                case T_INT:                 s->tipo = "int";  break;
                case T_FLO:                 s->tipo = "flo";  break;
                case T_BOOL:                s->tipo = "bool"; break;
                case T_RBRACK:              s->tipo = "list"; break;
                case T_RBRACE:              s->tipo = "dict"; break;
                default: break;
            }
            if (s->tipo) { s->ctx = "membro"; return; }
        }
        /* nem nome nem tipo: NADA a oferecer — nunca cair no topo depois de um ponto */
        if (!s->recv) s->ctx = "nenhum";
    } else if (i >= 0 && tl->tokens[i].type == T_AT) {
        s->ctx = "decorador";
    } else {
        /* `import x` / `from x import y` */
        for (int k = i; k >= 0 && tl->tokens[k].line == linha; k--) {
            const PSToken *t = &tl->tokens[k];
            if (t->type == T_KW && t->texto
                && (!strcmp(t->texto, "import") || !strcmp(t->texto, "from"))) {
                s->ctx = "import";
                break;
            }
        }
    }
}

static int cmd_contexto(const char *pos, const char *arquivo)
{
    int linha = 0, col = 0;
    if (!pos || sscanf(pos, "%d:%d", &linha, &col) != 2 || linha < 1 || col < 1) {
        printf("{\"contexto\":\"erro\",\"msg\":\"uso: jinga --contexto <linha>:<coluna> [arquivo" PS_EXT "]\"}\n");
        return 1;
    }
    size_t tam = 0;
    char *fonte = le_fonte_editor(arquivo, &tam);
    if (!fonte) { printf("{\"contexto\":\"erro\",\"msg\":\"sem memoria\"}\n"); return 1; }
    /* com `--utf16` a posição CHEGA na conta do editor: converte pra caractere,
     * que é como o lexer conta */
    col = col_de_utf16(linha, col);

    /* lexer do EDITOR: o comentário vira token, e é assim que se sabe que o
     * cursor está dentro de um (antes ele sumia e o cursor caía no `topo`) */
    PSTokenList *tl = ps_lexer_tokenize_editor(fonte, tam);
    if (!tl) { free(fonte); printf("{\"contexto\":\"erro\",\"msg\":\"lexer falhou\"}\n"); return 1; }

    CtxCursor s;
    memset(&s, 0, sizeof(s));
    s.ctx = "topo";

    int resolvido = 0;
    int dentro = tok_no_cursor(tl, linha, col);
    if (dentro >= 0) {
        const PSToken *t = &tl->tokens[dentro];
        if (t->type == T_COMMENT) { s.ctx = "comentario"; resolvido = 1; }
        else if (t->type == T_FSTRING) {
            size_t ini = off_de_pos(fonte, tam, t->line, t->col);
            size_t fim = off_de_pos(fonte, tam, linha, col);
            char expr[512];
            s.ctx = "texto";
            if (fim > ini && fstring_interp(fonte, ini, fim, expr, sizeof(expr))) {
                size_t nexpr = strlen(expr);
                PSTokenList *sub = ps_lexer_tokenize(expr, nexpr);
                if (sub) {
                    int ccol = 1;
                    for (size_t k = 0; k < nexpr; k++)
                        if (((unsigned char)expr[k] & 0xC0) != 0x80) ccol++;
                    contexto_resolve(expr, nexpr, sub, 1, ccol, &s);
                    ps_lexer_free(sub);
                }
            }
            resolvido = 1;
        }
        else if (t->type == T_STR || t->type == T_BYTES) { s.ctx = "texto"; resolvido = 1; }
    }
    if (!resolvido) contexto_resolve(fonte, tam, tl, linha, col, &s);

    /* em que funct/classe o cursor está: vem da ÁRVORE, não dos tokens */
    char escopo[256] = "";
    {
        PSTokenList *tp = ps_lexer_tokenize(fonte, tam);
        if (tp) {
            PSParseResult *r = ps_parse_lista(tp, 0);
            if (r) {
                escopo_no(r->programa, linha, escopo, sizeof(escopo));
                ps_parse_free(r);
            }
            ps_lexer_free(tp);
        }
    }

    printf("{\"contexto\":\"%s\"", s.ctx);
    jsonf("parcial", s.parcial);
    if (s.recv) jsonf("receptor", s.recv);
    if (s.tipo) jsonf("tipo", s.tipo);
    if (escopo[0]) jsonf("escopo", escopo);
    printf("}\n");

    free(s.recv);
    ps_lexer_free(tl);
    free(fonte);
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
    /* Marca a base da pilha ANTES de tudo: é a referência da medição de folga
     * que impede a recursão de estourar (ver `ps_pilha_apertada`). Feito no
     * main porque aqui a pilha ainda está praticamente intocada. */
    ps_pilha_marca_processo();
    /* o quadro do traceback é o MESMO pro erro que para o programa e pro que
     * só é avisado (o do `int funct` que devolveu 500) */
    ps_gancho_quadro = imprime_quadro;

    /* Executavel gerado por `-o`: o programa esta grudado neste binario. Roda
     * ele e ignora os subcomandos — quem chama um programa compilado espera o
     * PROGRAMA, não a CLI do pool. Os argumentos vão inteiros pro `sys.argv`. */
    {
        char meu[4096];
        if (ps_meu_caminho(meu, sizeof(meu)) == 0) {
            size_t tam_emb = 0;
            int versao = 0;
            char *emb = ps_payload(meu, &tam_emb, NULL, &versao);
            if (emb) {
                PSErroExec ee;
                ps_set_argv(argc - 1, argv + 1);
                /* `meu` segue sendo o nome do programa (sys.argv[0], recarga
                 * do jinker); a pasta do script, os modulos e o quadro do
                 * traceback vem do que foi embutido — o traceback dizia
                 * "em psp-pool, linha 6" e mostrava a linha 6 do ELF. */
                const char *origem = meu, *fonte = emb;
                size_t tam = tam_emb;
                char main_textual[1024] = "";
                if (versao == 2) {
                    if (emb_carrega(emb, tam_emb, main_textual, sizeof(main_textual), &fonte, &tam) != 0) {
                        fprintf(stderr, "jinga: o programa embutido neste executavel esta corrompido\n");
                        free(emb);
                        return 70;
                    }
                    origem = main_textual;
                } else {
                    /* v1 (gerado antes): um fonte so, com a pasta do ELF como
                     * raiz, como sempre foi; o quadro le o embutido */
                    ps_emb_poe(meu, meu, emb, tam_emb);
                }
                int rc = ps_roda_fonte(fonte, tam, meu, &ee);
                free(emb);
                if (rc != 0) return reporta(&ee, origem);
                return 0;
            }
        }
    }

    if (argc < 2) { ajuda(); return 0; }

    /* `--utf16` vale pros comandos de editor, em qualquer posição: linha e
     * coluna passam a ser contadas como o protocolo do editor conta. Sai do
     * argv aqui pra cada comando não ter que tratá-lo. */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--utf16") != 0) continue;
        g_utf16 = 1;
        for (int k = i; k + 1 < argc; k++) argv[k] = argv[k + 1];
        argc--;
        i--;
    }

    const char *cmd = argv[1];

    if (!strcmp(cmd, "--version") || !strcmp(cmd, "-V") || !strcmp(cmd, "-v")) {
        printf("Jinga %s [PSVM] (%s) Runtime standalone\n", PS_VERSAO, PS_BUILD_DATA);
        return 0;
    }
    if (!strcmp(cmd, "--help") || !strcmp(cmd, "-h") || !strcmp(cmd, "help")) {
        ajuda();
        return 0;
    }
    if (!strcmp(cmd, "//doc")) {
        printf("Especificacao da Jinga (%s):\n  %s\n", PS_VERSAO, SPEC_URL);
        return 0;
    }
    if (!strcmp(cmd, "--check") || !strcmp(cmd, "check")) {
        /* `--check arquivo.pr` confere o arquivo; `--check --path arquivo.pr`
         * confere o BUFFER do stdin como se ele fosse aquele arquivo. */
        if (argc >= 4 && (!strcmp(argv[2], "--path") || !strcmp(argv[2], "path")))
            return cmd_check(NULL, argv[3]);
        if (argc == 3 && (!strcmp(argv[2], "--path") || !strcmp(argv[2], "path"))) {
            fprintf(stderr, "uso: jinga --check --path <caminho" PS_EXT ">  (o fonte vem da entrada padrao)\n");
            return 64;
        }
        return cmd_check(argc >= 3 ? argv[2] : NULL, NULL);
    }
    /* modelo de tipos direto do motor — o editor e a auditoria de doc leem
     * daqui em vez de introspectar a stdlib do interpretador */
    /* o editor pergunta o contexto do cursor pro LEXER, nao pra um regex */
    if (!strcmp(cmd, "--tokens") || !strcmp(cmd, "tokens"))
        return cmd_tokens(argc >= 3 ? argv[2] : NULL);
    if (!strcmp(cmd, "--ast") || !strcmp(cmd, "ast"))
        return cmd_ast(argc >= 3 ? argv[2] : NULL);
    if (!strcmp(cmd, "--contexto") || !strcmp(cmd, "contexto"))
        return cmd_contexto(argc >= 3 ? argv[2] : NULL, argc >= 4 ? argv[3] : NULL);
    if (!strcmp(cmd, "--metadata") || !strcmp(cmd, "metadata")) {
        ps_metadata_json(stdout);
        return 0;
    }
    /* Depurador: `jinga --debug <porta> arquivo.pr`. O motor escuta DAP na porta
     * e o editor conecta — é a extensão do VS Code quem escolhe a porta livre e
     * passa aqui. Ver docs/debugger.md. */
    if (!strcmp(cmd, "--debug") || !strcmp(cmd, "debug")) {
        if (argc < 4) {
            fprintf(stderr, "uso: jinga --debug <porta> <arquivo.pr>\n");
            return 64;
        }
        char *fim = NULL;
        long porta = strtol(argv[2], &fim, 10);
        if (fim == argv[2] || *fim || porta < 1 || porta > 65535) {
            fprintf(stderr, "jinga --debug: porta invalida: %s\n", argv[2]);
            return 64;
        }
        ps_debug_porta((int)porta);
        /* Consome o `--debug <porta>` e deixa a linha como se o usuário tivesse
         * escrito `jinga arquivo.pr <args>`: o arquivo volta pra argv[1] e os
         * argumentos dele continuam depois, senão o `sys.argv` do programa
         * receberia o próprio nome do arquivo como primeiro argumento. */
        for (int k = 1; k + 2 < argc; k++) argv[k] = argv[k + 2];
        argc -= 2;
        cmd = argv[1];
    }
    if (!strcmp(cmd, "build")) return cmd_build();
    if (!strcmp(cmd, "compile")) {
        if (argc < 4) { fprintf(stderr, "uso: jinga compile <arquivo.pr> -o <saida>\n"); return 64; }
        if (strcmp(argv[3], "-o") != 0 || argc < 5) {
            fprintf(stderr, "uso: jinga compile <arquivo.pr> -o <saida>\n"); return 64;
        }
        return cmd_compila(argv[2], argv[4]);
    }
    /* ── pacotes (só .pr: lib/comando) ──────────────────────────────── */
    if (!strcmp(cmd, "install")) {
        if (argc < 3) { fprintf(stderr, "uso: jpkg install <arquivo.pr | nome> [-asLib]\n"); return 1; }
        int modo = tem_flag(argc, argv, 3, "-asLib") ? PS_PKG_LIB : PS_PKG_AUTO;
        return ps_pkg_install(argv[2], modo);
    }
    if (!strcmp(cmd, "uninstall")) {
        if (argc < 3) { fprintf(stderr, "uso: jpkg uninstall <nome> [-asLib]\n"); return 1; }
        int cat = tem_flag(argc, argv, 3, "-asLib") ? PS_PKG_LIB : PS_PKG_AUTO;
        return ps_pkg_uninstall(argv[2], cat);
    }
    if (!strcmp(cmd, "list")) return ps_pkg_list();
    if (!strcmp(cmd, "registry")) return ps_pkg_registry(argc - 2, argv + 2);
    if (!strcmp(cmd, "repl")) {
        fprintf(stderr,
            "jinga: o REPL interativo ainda nao esta no binario C "
            "(precisa de estado persistente na VM).\n"
            "      Por enquanto: `jinga arquivo.pr` ou `jinga -e \"<codigo>\"`.\n");
        return 64;
    }

    PSErroExec e;

    if (!strcmp(cmd, "-e")) {
        if (argc < 3) { ajuda(); return 64; }
        if (ps_roda_fonte(argv[2], strlen(argv[2]), NULL, &e) != 0)
            return reporta(&e, "<-e>");
        return 0;
    }

    /* `jinga programa.pr -o saida` — gera o executavel em vez de rodar. */
    if (argc >= 4 && !strcmp(argv[2], "-o"))
        return cmd_compila(cmd, argv[3]);
    if (argc == 3 && !strcmp(argv[2], "-o")) {
        fprintf(stderr, "uso: jinga %s -o <saida>\n", cmd);
        return 64;
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
