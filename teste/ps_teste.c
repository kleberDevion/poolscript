/*
 * Runner da suíte. Roda `./pool` num subprocesso pra cada caso e compara.
 *
 *   make teste && ./teste            roda tudo
 *   ./teste crash                    só os casos cujo nome/grupo casa "crash"
 *   ./teste -v                       mostra também os que passaram
 *
 * Sai com 1 se algum caso falhar — é o que serve pra CI e pra mim não seguir
 * em frente achando que está verde.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#include "ps_teste.h"

#define POOL "./pool"
#define TEMPO_MAX 20        /* segundos por caso: caso que trava é FALHA */

static const Grupo GRUPOS[] = {
    { "crash",     CASOS_CRASH,     0 },
    { "inteiros",  CASOS_INTEIROS,  0 },
    { "erros",     CASOS_ERROS,     0 },
    { "linguagem", CASOS_LINGUAGEM, 0 },
    { "pendentes", CASOS_PENDENTES, 0 },
    { "cobertura", CASOS_COBERTURA, 0 },
    { "diferencial", CASOS_DIFERENCIAL, 0 },
    { "equivalencia", CASOS_EQUIVALENCIA, 0 },
};

/* preenchido em tempo de execução (os `extern int` vivem nos casos_*.c) */
static int tamanho_do_grupo(const char *g)
{
    if (!strcmp(g, "crash"))     return NC_CRASH;
    if (!strcmp(g, "inteiros"))  return NC_INTEIROS;
    if (!strcmp(g, "erros"))     return NC_ERROS;
    if (!strcmp(g, "linguagem")) return NC_LINGUAGEM;
    if (!strcmp(g, "pendentes")) return NC_PENDENTES;
    if (!strcmp(g, "cobertura")) return NC_COBERTURA;
    if (!strcmp(g, "diferencial")) return NC_DIFERENCIAL;
    if (!strcmp(g, "equivalencia")) return NC_EQUIVALENCIA;
    return 0;
}

/* ── execução de um caso ─────────────────────────────────────────────────── */

typedef struct {
    char *saida;      /* stdout */
    char *erro;       /* stderr */
    int   rc;         /* código de saída (ou -1) */
    int   sinal;      /* != 0 = MORREU com este sinal (segfault, SIGFPE...) */
    int   estourou;   /* 1 = passou do TEMPO_MAX */
} Resultado;

static char *le_tudo(int fd)
{
    size_t cap = 4096, n = 0;
    char *b = malloc(cap);
    if (!b) return NULL;
    for (;;) {
        if (n + 1024 > cap) { cap *= 2; char *nb = realloc(b, cap); if (!nb) { free(b); return NULL; } b = nb; }
        ssize_t r = read(fd, b + n, cap - n - 1);
        if (r <= 0) break;
        n += (size_t)r;
    }
    b[n] = '\0';
    return b;
}

static void solta(Resultado *r) { free(r->saida); free(r->erro); r->saida = r->erro = NULL; }

static int roda(const Caso *c, Resultado *out)
{
    memset(out, 0, sizeof(*out));

    char arquivo[256];
    char fonte_ajustada[8192];
    const char *fonte = c->fonte;
    int fd;
    if (c->arquivo) {
        /* Nome fixo colidia entre execuções concorrentes: uma apagava o
         * arquivo enquanto a outra ainda rodava ("nao consegui abrir"), e o
         * caso falhava de forma intermitente. O nome ganha o PID, e o MESMO
         * sufixo é aplicado ao fonte — que precisa se importar pelo nome. */
        char base[128];
        snprintf(base, sizeof(base), "%s", c->arquivo);
        char *ponto = strrchr(base, '.');
        if (ponto) *ponto = '\0';
        char unico[192];
        snprintf(unico, sizeof(unico), "%s_%d", base, (int)getpid());
        snprintf(arquivo, sizeof(arquivo), "/tmp/%s.ps", unico);
        /* troca cada ocorrência do nome base pelo nome único, no fonte */
        size_t nb = strlen(base), j = 0;
        for (const char *q = c->fonte; *q && j < sizeof(fonte_ajustada) - 256; ) {
            if (strncmp(q, base, nb) == 0) {
                j += (size_t)snprintf(fonte_ajustada + j, sizeof(fonte_ajustada) - j,
                                      "%s", unico);
                q += nb;
            } else {
                fonte_ajustada[j++] = *q++;
            }
        }
        fonte_ajustada[j] = '\0';
        fonte = fonte_ajustada;
        fd = open(arquivo, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd < 0) { perror("open"); return -1; }
    } else {
        snprintf(arquivo, sizeof(arquivo), "/tmp/ps_teste_XXXXXX");
        fd = mkstemp(arquivo);
        if (fd < 0) { perror("mkstemp"); return -1; }
    }
    if (write(fd, fonte, strlen(fonte)) < 0) { close(fd); unlink(arquivo); return -1; }
    close(fd);

    int po[2], pe[2];
    if (pipe(po) != 0 || pipe(pe) != 0) { unlink(arquivo); return -1; }

    pid_t pid = fork();
    if (pid < 0) { unlink(arquivo); return -1; }
    if (pid == 0) {
        dup2(po[1], STDOUT_FILENO); dup2(pe[1], STDERR_FILENO);
        close(po[0]); close(po[1]); close(pe[0]); close(pe[1]);
        alarm(TEMPO_MAX);                       /* trava = SIGALRM, não espera eterna */
        /* Nenhum caso pode abrir janela: `guzer.UI()` é exibido no fim do
         * script e ficaria esperando o usuário fechar — o caso "travava" por
         * 20s e virava falha. Headless monta a árvore e não exibe. */
        setenv("GUZER_HEADLESS", "1", 1);
        execl(POOL, POOL, arquivo, (char *)NULL);
        _exit(127);
    }
    close(po[1]); close(pe[1]);
    out->saida = le_tudo(po[0]);
    out->erro  = le_tudo(pe[0]);
    close(po[0]); close(pe[0]);

    int st = 0;
    waitpid(pid, &st, 0);
    unlink(arquivo);
    if (WIFSIGNALED(st)) {
        out->sinal = WTERMSIG(st);
        out->estourou = (out->sinal == SIGALRM);
        out->rc = -1;
    } else {
        out->rc = WEXITSTATUS(st);
    }
    return 0;
}

/* ── comparação ──────────────────────────────────────────────────────────── */

static void sem_fim_de_linha(char *s)
{
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
}

/* devolve NULL se passou; senão, o motivo (buffer estático) */
static const char *confere(const Caso *c, Resultado *r)
{
    static char motivo[4096];

    if (r->estourou) {
        snprintf(motivo, sizeof motivo, "TRAVOU (passou de %ds)", TEMPO_MAX);
        return motivo;
    }
    if (r->sinal) {
        snprintf(motivo, sizeof motivo,
                 "a VM MORREU com sinal %d (%s)", r->sinal,
                 r->sinal == SIGSEGV ? "segfault" :
                 r->sinal == SIGFPE  ? "erro aritmético" :
                 r->sinal == SIGABRT ? "abort" : "?");
        return motivo;
    }
    sem_fim_de_linha(r->saida);
    sem_fim_de_linha(r->erro);

    if (c->saida && strcmp(r->saida ? r->saida : "", c->saida) != 0) {
        snprintf(motivo, sizeof motivo, "saída\n     esperada: %s\n     obtida  : %s",
                 c->saida, r->saida ? r->saida : "");
        return motivo;
    }
    if (c->erro) {
        if (!r->erro || !strstr(r->erro, c->erro)) {
            snprintf(motivo, sizeof motivo, "erro esperado com %s\n     obtido  : %s",
                     c->erro, (r->erro && r->erro[0]) ? r->erro : "(nenhum)");
            return motivo;
        }
    } else if (r->rc != 0) {
        snprintf(motivo, sizeof motivo, "esperava sucesso, saiu com rc=%d\n     erro: %s",
                 r->rc, r->erro ? r->erro : "");
        return motivo;
    }
    if (c->rc >= 0 && r->rc != c->rc) {
        snprintf(motivo, sizeof motivo, "código de saída: esperado %d, obtido %d", c->rc, r->rc);
        return motivo;
    }
    return NULL;
}

/* ── principal ───────────────────────────────────────────────────────────── */

int main(int argc, char **argv)
{
    const char *filtro = NULL;
    int verboso = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) verboso = 1;
        else filtro = argv[i];
    }

    if (access(POOL, X_OK) != 0) {
        fprintf(stderr, "teste: `%s` não existe ou não é executável — rode `make pool`\n", POOL);
        return 2;
    }

    int total = 0, passou = 0, falhou = 0;
    for (int g = 0; g < (int)(sizeof(GRUPOS) / sizeof(GRUPOS[0])); g++) {
        const char *nome_g = GRUPOS[g].grupo;
        int n = tamanho_do_grupo(nome_g);
        int cabecalho = 0;
        for (int i = 0; i < n; i++) {
            const Caso *c = &GRUPOS[g].casos[i];
            if (filtro && !strstr(nome_g, filtro) && !strstr(c->nome, filtro)) continue;
            if (!cabecalho) { printf("\n── %s ──\n", nome_g); cabecalho = 1; }
            total++;
            Resultado r;
            if (roda(c, &r) != 0) {
                printf("  ERRO INTERNO  %s\n", c->nome);
                falhou++;
                continue;
            }
            const char *motivo = confere(c, &r);
            if (motivo) {
                printf("  FALHOU  %s\n     %s\n", c->nome, motivo);
                /* Grava também em arquivo. Uma falha INTERMITENTE some da tela
                 * na próxima execução e aí não dá pra investigar; aqui ela
                 * fica, com o fonte e a saída completa. */
                FILE *lg = fopen("teste/ultima_falha.txt", "a");
                if (lg) {
                    fprintf(lg, "=== %s\n--- motivo: %s\n--- fonte:\n%s"
                                "--- rc: %d  sinal: %d\n--- saida:\n%s"
                                "--- erro:\n%s\n\n",
                            c->nome, motivo, c->fonte, r.rc, r.sinal,
                            r.saida ? r.saida : "", r.erro ? r.erro : "");
                    fclose(lg);
                }
                falhou++;
            } else {
                passou++;
                if (verboso) printf("  ok      %s\n", c->nome);
            }
            solta(&r);
        }
    }

    printf("\n%d casos: %d passaram, %d falharam\n", total, passou, falhou);
    if (total == 0) {
        printf("nenhum caso casou com o filtro — isso é falha, não sucesso\n");
        return 2;
    }
    return falhou ? 1 : 0;
}
