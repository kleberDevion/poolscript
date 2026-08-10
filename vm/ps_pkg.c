/*
 * Gerenciador de pacotes pool-native — ver ps_pkg.h.
 *
 * A verdade do que está instalado é o SISTEMA DE ARQUIVOS (os .ps em
 * commands/ e libs/); o installed.json é reescrito a partir dele a cada op, só
 * pra `psl` (Python) enxergar o mesmo estado. config.json guarda o
 * registry_url. Nada aqui depende de Python nem de pip.
 */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include "ps_pkg.h"
#include "ps_http.h"
#include "ps_hash.h"

/* ── caminhos ───────────────────────────────────────────────────────────── */

static int pkg_home(char *out, size_t cap)
{
    const char *over = getenv("POOLSCRIPT_HOME");
    if (over && *over) { snprintf(out, cap, "%s", over); return 0; }
    const char *h = getenv("HOME");
    if (!h || !*h) return -1;
    snprintf(out, cap, "%s/.poolscript", h);
    return 0;
}

static void mkdirp(const char *caminho)
{
    /* cria cada nível; ignora "já existe" */
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", caminho);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') { *p = '\0'; mkdir(tmp, 0755); *p = '/'; }
    }
    mkdir(tmp, 0755);
}

static int pkg_dirs(char *home, size_t cap)
{
    if (pkg_home(home, cap) != 0) {
        fprintf(stderr, "Erro: nao consegui achar o HOME (defina HOME ou POOLSCRIPT_HOME)\n");
        return -1;
    }
    char d[1200];
    snprintf(d, sizeof(d), "%s/commands", home); mkdirp(d);
    snprintf(d, sizeof(d), "%s/libs", home);     mkdirp(d);
    snprintf(d, sizeof(d), "%s/bin", home);      mkdirp(d);
    return 0;
}

/* ── E/S de arquivo ─────────────────────────────────────────────────────── */

static char *ler_txt(const char *caminho, size_t *tam)
{
    FILE *f = fopen(caminho, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); long n = ftell(f); rewind(f);
    if (n < 0) { fclose(f); return NULL; }
    char *b = malloc((size_t)n + 1);
    if (!b) { fclose(f); return NULL; }
    size_t lidos = fread(b, 1, (size_t)n, f);
    fclose(f);
    b[lidos] = '\0';
    if (tam) *tam = lidos;
    return b;
}

static int escrever_txt(const char *caminho, const char *dados, size_t n)
{
    FILE *f = fopen(caminho, "wb");
    if (!f) return -1;
    if (n && fwrite(dados, 1, n, f) != n) { fclose(f); return -1; }
    fclose(f);
    return 0;
}

static int existe(const char *caminho)
{
    struct stat st;
    return stat(caminho, &st) == 0 && S_ISREG(st.st_mode);
}

/* nome base sem diretório e sem a extensão (.ps / .psl / .p) */
static void derive_name(const char *target, char *out, size_t cap)
{
    const char *base = strrchr(target, '/');
    base = base ? base + 1 : target;
    snprintf(out, cap, "%s", base);
    size_t l = strlen(out);
    if      (l > 4 && strcmp(out + l - 4, ".psl") == 0) out[l - 4] = '\0';
    else if (l > 3 && strcmp(out + l - 3, ".ps")  == 0) out[l - 3] = '\0';
    else if (l > 2 && strcmp(out + l - 2, ".p")   == 0) out[l - 2] = '\0';
}

/* arquivo local da linguagem: .ps, .psl ou .p */
static int termina_em_ps(const char *s)
{
    size_t l = strlen(s);
    return (l > 4 && strcmp(s + l - 4, ".psl") == 0)
        || (l > 3 && strcmp(s + l - 3, ".ps")  == 0)
        || (l > 2 && strcmp(s + l - 2, ".p")   == 0);
}

/* marcador na 1ª linha com conteúdo: "#!lib" -> 1, "#!cmd" -> 0, senão -1 */
static int peek_marker(const char *texto)
{
    const char *p = texto;
    while (*p) {
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (!*p) break;
        const char *fim = p;
        while (*fim && *fim != '\n' && *fim != '\r') fim++;
        size_t n = (size_t)(fim - p);
        while (n && (p[n-1] == ' ' || p[n-1] == '\t')) n--;
        if (n == 5 && !strncmp(p, "#!lib", 5)) return 1;
        if (n == 5 && !strncmp(p, "#!cmd", 5)) return 0;
        return -1;   /* 1ª linha com conteúdo não é marcador */
    }
    return -1;
}

/* ── installed.json (derivado do filesystem, pro psl ver) ───────────────── */

static const char *agora_iso(char *buf, size_t cap)
{
    time_t t = time(NULL);
    struct tm g;
    gmtime_r(&t, &g);
    strftime(buf, cap, "%Y-%m-%dT%H:%M:%S+00:00", &g);
    return buf;
}

/* escreve um bloco {"nome": {"source": "local", "installed_at": "..."}} */
static void escreve_secao(FILE *f, const char *home, const char *sub)
{
    char dir[1200];
    snprintf(dir, sizeof(dir), "%s/%s", home, sub);
    DIR *d = opendir(dir);
    int primeiro = 1;
    char ts[64]; agora_iso(ts, sizeof(ts));
    if (d) {
        struct dirent *e;
        while ((e = readdir(d)) != NULL) {
            size_t l = strlen(e->d_name);
            if (l < 4 || strcmp(e->d_name + l - 3, ".ps") != 0) continue;
            char nome[256];
            snprintf(nome, sizeof(nome), "%.*s", (int)(l - 3), e->d_name);
            fprintf(f, "%s\n    %s\"%s\": {\"source\": \"local\", \"installed_at\": \"%s\"}",
                    primeiro ? "" : ",", "", nome, ts);
            primeiro = 0;
        }
        closedir(d);
    }
    if (!primeiro) fprintf(f, "\n  ");
}

static void regen_installed(const char *home)
{
    char caminho[1200];
    snprintf(caminho, sizeof(caminho), "%s/installed.json", home);
    FILE *f = fopen(caminho, "wb");
    if (!f) return;
    fprintf(f, "{\n  \"commands\": {");
    escreve_secao(f, home, "commands");
    fprintf(f, "},\n  \"libs\": {");
    escreve_secao(f, home, "libs");
    fprintf(f, "},\n  \"py\": {}\n}\n");
    fclose(f);
}

/* ── PATH: garante ~/.poolscript/bin no shell rc (Linux) ────────────────── */

static void ensure_bin_no_path(const char *home)
{
    char bindir[1200];
    snprintf(bindir, sizeof(bindir), "%s/bin", home);

    /* já no PATH da sessão? então nada a fazer */
    const char *path = getenv("PATH");
    if (path) {
        const char *p = path;
        size_t bl = strlen(bindir);
        while (p) {
            const char *sep = strchr(p, ':');
            size_t seg = sep ? (size_t)(sep - p) : strlen(p);
            if (seg == bl && !strncmp(p, bindir, bl)) return;
            p = sep ? sep + 1 : NULL;
        }
    }
    if (getenv("POOLSCRIPT_HOME")) {
        printf("  (adicione %s ao seu PATH para usar os comandos)\n", bindir);
        return;
    }
    const char *h = getenv("HOME");
    if (!h) return;
    char linha[1400];
    snprintf(linha, sizeof(linha),
             "\n# PoolScript — comandos instalados via `psl install`\nexport PATH=\"%s:$PATH\"\n",
             bindir);
    int add = 0;
    const char *rcs[2] = { ".bashrc", ".profile" };
    for (int i = 0; i < 2; i++) {
        char rc[1100];
        snprintf(rc, sizeof(rc), "%s/%s", h, rcs[i]);
        size_t tam = 0;
        char *cont = ler_txt(rc, &tam);
        int tem = cont && strstr(cont, bindir);
        free(cont);
        if (tem) { add = 1; continue; }   /* já configurado antes */
        FILE *f = fopen(rc, "ab");
        if (f) { fwrite(linha, 1, strlen(linha), f); fclose(f); add = 1; }
    }
    if (add)
        printf("  %s adicionado ao PATH — abra um novo terminal para usar os comandos\n", bindir);
    else
        printf("  adicione %s ao seu PATH para usar os comandos instalados\n", bindir);
}

static void write_shim(const char *home, const char *name, const char *ps_path)
{
    char shim[1200];
    snprintf(shim, sizeof(shim), "%s/bin/%s", home, name);
    char corpo[1400];
    int n = snprintf(corpo, sizeof(corpo), "#!/bin/sh\nexec pool \"%s\" \"$@\"\n", ps_path);
    if (escrever_txt(shim, corpo, (size_t)n) == 0) chmod(shim, 0755);
}

/* ── config.json (registry_url) ─────────────────────────────────────────── */

/* extrai o valor de "registry_url" do config.json (parser mínimo) */
static int registry_url(const char *home, char *out, size_t cap)
{
    char caminho[1200];
    snprintf(caminho, sizeof(caminho), "%s/config.json", home);
    char *txt = ler_txt(caminho, NULL);
    if (!txt) return -1;
    char *k = strstr(txt, "\"registry_url\"");
    if (!k) { free(txt); return -1; }
    k = strchr(k + 14, ':');
    if (!k) { free(txt); return -1; }
    k++;
    while (*k == ' ' || *k == '\t') k++;
    if (*k != '"') { free(txt); return -1; }
    k++;
    size_t i = 0;
    while (*k && *k != '"' && i < cap - 1) out[i++] = *k++;
    out[i] = '\0';
    free(txt);
    return i ? 0 : -1;
}

static int registry_set(const char *home, const char *url)
{
    char caminho[1200];
    snprintf(caminho, sizeof(caminho), "%s/config.json", home);
    char corpo[2048];
    int n = snprintf(corpo, sizeof(corpo), "{\n  \"registry_url\": \"%s\"\n}\n", url);
    return escrever_txt(caminho, corpo, (size_t)n);
}

/* ── registry: busca nome -> url [+ sha256] no índice remoto ─────────────── */

static int https_ok(const char *url)
{
    if (!strncmp(url, "https://", 8)) return 1;
    if (!strncmp(url, "http://localhost", 16) || !strncmp(url, "http://127.0.0.1", 16)) return 1;
    return 0;
}

/* acha `"chave"` : valor (string) dentro de um objeto JSON cru. Só string. */
static int json_str_campo(const char *json, const char *chave, char *out, size_t cap)
{
    char alvo[128];
    snprintf(alvo, sizeof(alvo), "\"%s\"", chave);
    const char *k = strstr(json, alvo);
    if (!k) return -1;
    k = strchr(k + strlen(alvo), ':');
    if (!k) return -1;
    k++;
    while (*k == ' ' || *k == '\t' || *k == '\n' || *k == '\r') k++;
    if (*k != '"') return -1;
    k++;
    size_t i = 0;
    while (*k && *k != '"' && i < cap - 1) out[i++] = *k++;
    out[i] = '\0';
    return 0;
}

/* baixa o índice e resolve o nome; devolve 0 e preenche url/sha (sha pode ficar
 * vazio). O índice é `{"nome": "url"}` ou `{"nome": {"url":..,"sha256":..}}`. */
static int registry_lookup(const char *home, const char *nome, char *url, size_t urlcap,
                           char *sha, size_t shacap)
{
    char idx_url[1024];
    if (registry_url(home, idx_url, sizeof(idx_url)) != 0) {
        fprintf(stderr, "Erro: nenhum registry configurado — use `psl registry set-url <url>` "
                        "ou instale de um arquivo local (nome.ps)\n");
        return -1;
    }
    if (!https_ok(idx_url)) {
        fprintf(stderr, "Erro: o registry precisa ser https: %s\n", idx_url);
        return -1;
    }
    PSHttpResp r;
    if (ps_http_request("GET", idx_url, NULL, NULL, 0, 10, 8 * 1024 * 1024, &r) != 0 || r.status == -1) {
        fprintf(stderr, "Erro: nao consegui buscar o registry em %s: %s\n", idx_url,
                r.status == -1 ? r.erro : "");
        ps_http_resp_solta(&r);
        return -1;
    }
    if (r.status != 200) {
        fprintf(stderr, "Erro: registry respondeu HTTP %ld\n", r.status);
        ps_http_resp_solta(&r);
        return -1;
    }
    /* acha o trecho a partir de `"nome"` e resolve string ou objeto */
    char alvo[300];
    snprintf(alvo, sizeof(alvo), "\"%s\"", nome);
    char *k = r.corpo ? strstr(r.corpo, alvo) : NULL;
    if (!k) {
        fprintf(stderr, "Erro: pacote '%s' nao encontrado no registry\n", nome);
        ps_http_resp_solta(&r);
        return -1;
    }
    char *v = strchr(k + strlen(alvo), ':');
    int rc = -1;
    if (v) {
        v++;
        while (*v == ' ' || *v == '\t' || *v == '\n' || *v == '\r') v++;
        sha[0] = '\0';
        if (*v == '"') {                    /* nome -> "url" */
            v++;
            size_t i = 0;
            while (*v && *v != '"' && i < urlcap - 1) url[i++] = *v++;
            url[i] = '\0';
            rc = i ? 0 : -1;
        } else if (*v == '{') {             /* nome -> {"url":.., "sha256":..} */
            char *fim = strchr(v, '}');
            char obj[2048];
            size_t on = fim ? (size_t)(fim - v + 1) : strlen(v);
            if (on >= sizeof(obj)) on = sizeof(obj) - 1;
            memcpy(obj, v, on); obj[on] = '\0';
            if (json_str_campo(obj, "url", url, urlcap) == 0) rc = 0;
            json_str_campo(obj, "sha256", sha, shacap);
        }
    }
    ps_http_resp_solta(&r);
    (void)shacap;
    return rc;
}

/* baixa a fonte de uma url https e confere o sha256 (se dado) */
static char *baixa_fonte(const char *url, const char *sha_esperado, size_t *tam)
{
    if (!https_ok(url)) {
        fprintf(stderr, "Erro: recusando baixar por HTTP inseguro: %s (use https)\n", url);
        return NULL;
    }
    PSHttpResp r;
    if (ps_http_request("GET", url, NULL, NULL, 0, 10, 16 * 1024 * 1024, &r) != 0 || r.status == -1) {
        fprintf(stderr, "Erro: nao consegui baixar %s: %s\n", url, r.status == -1 ? r.erro : "");
        ps_http_resp_solta(&r);
        return NULL;
    }
    if (r.status != 200) {
        fprintf(stderr, "Erro: %s respondeu HTTP %ld\n", url, r.status);
        ps_http_resp_solta(&r);
        return NULL;
    }
    if (sha_esperado && sha_esperado[0]) {
        unsigned char dig[PS_SHA256_TAM];
        ps_sha256((const unsigned char *)r.corpo, r.ncorpo, dig);
        char hex[PS_SHA256_TAM * 2 + 1];
        for (int i = 0; i < PS_SHA256_TAM; i++) snprintf(hex + i * 2, 3, "%02x", dig[i]);
        if (strcasecmp(hex, sha_esperado) != 0) {
            fprintf(stderr, "Erro: hash nao confere para %s — esperado %s, veio %s. "
                            "Pacote possivelmente adulterado; abortado.\n", url, sha_esperado, hex);
            ps_http_resp_solta(&r);
            return NULL;
        }
    }
    char *out = malloc(r.ncorpo + 1);
    if (out) { memcpy(out, r.corpo, r.ncorpo); out[r.ncorpo] = '\0'; if (tam) *tam = r.ncorpo; }
    ps_http_resp_solta(&r);
    return out;
}

/* ── install ────────────────────────────────────────────────────────────── */

int ps_pkg_install(const char *target, int modo)
{
    char home[1024];
    if (pkg_dirs(home, sizeof(home)) != 0) return 1;

    char nome[256];
    char *fonte = NULL;
    size_t ntam = 0;

    if (termina_em_ps(target)) {
        if (!existe(target)) { fprintf(stderr, "Erro: arquivo nao encontrado: %s\n", target); return 1; }
        fonte = ler_txt(target, &ntam);
        if (!fonte) { fprintf(stderr, "Erro: nao consegui ler %s\n", target); return 1; }
        derive_name(target, nome, sizeof(nome));
    } else {
        char url[1024], sha[128];
        if (registry_lookup(home, target, url, sizeof(url), sha, sizeof(sha)) != 0) return 1;
        fonte = baixa_fonte(url, sha, &ntam);
        if (!fonte) return 1;
        snprintf(nome, sizeof(nome), "%s", target);
    }

    int kind;   /* 0 cmd, 1 lib */
    if (modo == PS_PKG_LIB) kind = 1;
    else if (modo == PS_PKG_CMD) kind = 0;
    else { int m = peek_marker(fonte); kind = (m == 1) ? 1 : 0; }   /* auto: padrão cmd */

    char dest[1300];
    snprintf(dest, sizeof(dest), "%s/%s/%s.ps", home, kind ? "libs" : "commands", nome);
    if (escrever_txt(dest, fonte, ntam) != 0) {
        free(fonte);
        fprintf(stderr, "Erro: nao consegui gravar %s\n", dest);
        return 1;
    }
    free(fonte);

    if (kind) {
        printf("lib '%s' instalada — disponivel via `import %s` em qualquer script\n  (%s)\n",
               nome, nome, dest);
    } else {
        write_shim(home, nome, dest);
        printf("comando '%s' instalado (%s)\n", nome, dest);
        ensure_bin_no_path(home);
    }
    regen_installed(home);
    return 0;
}

/* ── uninstall ──────────────────────────────────────────────────────────── */

static int remove_categoria(const char *home, const char *nome, int lib)
{
    char ps[1300];
    snprintf(ps, sizeof(ps), "%s/%s/%s.ps", home, lib ? "libs" : "commands", nome);
    int achou = 0;
    if (existe(ps)) { unlink(ps); achou = 1; }
    if (!lib) {
        char shim[1300];
        snprintf(shim, sizeof(shim), "%s/bin/%s", home, nome);
        if (existe(shim) || access(shim, F_OK) == 0) { unlink(shim); achou = 1; }
    }
    return achou;
}

int ps_pkg_uninstall(const char *nome, int categoria)
{
    char home[1024];
    if (pkg_dirs(home, sizeof(home)) != 0) return 1;

    char pcmd[1300], plib[1300];
    snprintf(pcmd, sizeof(pcmd), "%s/commands/%s.ps", home, nome);
    snprintf(plib, sizeof(plib), "%s/libs/%s.ps", home, nome);
    int tem_cmd = existe(pcmd), tem_lib = existe(plib);

    if (categoria == PS_PKG_LIB) {
        if (!tem_lib) { fprintf(stderr, "Erro: lib '%s' nao esta instalada\n", nome); return 1; }
        remove_categoria(home, nome, 1);
        printf("lib '%s' removida\n", nome);
        regen_installed(home);
        return 0;
    }
    /* auto */
    if (!tem_cmd && !tem_lib) {
        fprintf(stderr, "Erro: '%s' nao esta instalado (nem comando, nem lib)\n", nome);
        return 1;
    }
    if (tem_cmd && tem_lib) {
        fprintf(stderr, "Erro: '%s' esta instalado como comando E como lib. "
                        "Desambigue: `-asLib` remove a lib; sem flag, o comando.\n", nome);
        /* segue e remove o comando (sem flag = comando, como o pkgmgr.py) */
    }
    if (tem_cmd) { remove_categoria(home, nome, 0); printf("comando '%s' removido\n", nome); }
    else         { remove_categoria(home, nome, 1); printf("lib '%s' removida\n", nome); }
    regen_installed(home);
    return 0;
}

/* ── list ───────────────────────────────────────────────────────────────── */

static void lista_dir(const char *home, const char *sub, const char *rotulo)
{
    char dir[1200];
    snprintf(dir, sizeof(dir), "%s/%s", home, sub);
    printf("%s:\n", rotulo);
    DIR *d = opendir(dir);
    /* coleta e ordena */
    char nomes[512][256]; int n = 0;
    if (d) {
        struct dirent *e;
        while ((e = readdir(d)) != NULL && n < 512) {
            size_t l = strlen(e->d_name);
            if (l < 4 || strcmp(e->d_name + l - 3, ".ps") != 0) continue;
            snprintf(nomes[n], sizeof(nomes[n]), "%.*s", (int)(l - 3), e->d_name);
            n++;
        }
        closedir(d);
    }
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (strcmp(nomes[i], nomes[j]) > 0) {
                char t[256]; snprintf(t, sizeof(t), "%s", nomes[i]);
                snprintf(nomes[i], sizeof(nomes[i]), "%s", nomes[j]);
                snprintf(nomes[j], sizeof(nomes[j]), "%s", t);
            }
    if (n == 0) printf("  (nenhum)\n");
    else for (int i = 0; i < n; i++) printf("  %s\n", nomes[i]);
}

int ps_pkg_list(void)
{
    char home[1024];
    if (pkg_dirs(home, sizeof(home)) != 0) return 1;
    lista_dir(home, "commands", "Comandos");
    lista_dir(home, "libs", "Libs PoolScript");
    return 0;
}

/* ── registry ───────────────────────────────────────────────────────────── */

int ps_pkg_registry(int argc, char **argv)
{
    char home[1024];
    if (pkg_dirs(home, sizeof(home)) != 0) return 1;

    if (argc == 0 || !strcmp(argv[0], "show")) {
        char url[1024];
        if (registry_url(home, url, sizeof(url)) == 0) printf("registry atual: %s\n", url);
        else printf("nenhum registry configurado — use `psl registry set-url <url>`\n");
        return 0;
    }
    if (!strcmp(argv[0], "set-url")) {
        if (argc < 2) { fprintf(stderr, "uso: psl registry set-url <url>\n"); return 1; }
        if (registry_set(home, argv[1]) != 0) { fprintf(stderr, "Erro: nao consegui gravar config\n"); return 1; }
        printf("registry configurado: %s\n", argv[1]);
        return 0;
    }
    fprintf(stderr, "uso: psl registry [show | set-url <url>]\n");
    return 1;
}
