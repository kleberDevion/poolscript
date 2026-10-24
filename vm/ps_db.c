/*
 * psodbc SQL — ver ps_db.h. sqlite, postgres, mysql e odbc no mesmo molde:
 * cada driver preenche o mesmo PSDbRes, e os placeholders (`?`/`%s`) passam
 * por UM varredor (`sql_varre_placeholders`) que cada driver só alimenta.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sqlite3.h>
#include "ps_win.h"     /* no Windows os tipos do ODBC (sqltypes.h) vêm do windows.h */
#include <libpq-fe.h>
#include <mysql.h>
#include <sql.h>
#include <sqlext.h>

#include "ps_db.h"
#include "ps_pgerr.h"
#include "ps_dl.h"

/* ── os clientes de banco: carregados no PRIMEIRO connect de cada driver ───
 *
 * O sqlite vem dentro do `pool`. O Postgres, o MySQL e o ODBC NÃO são mais
 * ligados ao executável: cada um é aberto na primeira conexão daquele driver
 * (ver ps_dl.h). Programa que não usa banco não carrega nenhum; o que só usa
 * sqlite também não. As listas abaixo são TODAS as funções que este arquivo
 * chama de cada biblioteca — chamar uma que não está aqui não liga (o nome
 * não existe sem a biblioteca), então esquecer uma não passa calado. */
#define PG_FUNCS(X) X(PQclear) X(PQcmdTuples) X(PQconnectdbParams) X(PQerrorMessage)  \
    X(PQexec) X(PQexecParams) X(PQfinish) X(PQfname) X(PQftype) X(PQgetisnull)         \
    X(PQgetvalue) X(PQnfields) X(PQntuples) X(PQresultErrorField) X(PQresultStatus)    \
    X(PQstatus)
#define MY_FUNCS(X) X(mysql_affected_rows) X(mysql_close) X(mysql_error)                \
    X(mysql_fetch_fields) X(mysql_fetch_lengths) X(mysql_fetch_row) X(mysql_field_count) \
    X(mysql_free_result) X(mysql_init) X(mysql_num_fields) X(mysql_num_rows)              \
    X(mysql_real_connect) X(mysql_real_escape_string) X(mysql_real_query)                 \
    X(mysql_store_result)
#define OD_FUNCS(X) X(SQLAllocHandle) X(SQLBindParameter) X(SQLDescribeCol)             \
    X(SQLDisconnect) X(SQLDriverConnect) X(SQLExecDirect) X(SQLFetch) X(SQLFreeHandle)   \
    X(SQLGetData) X(SQLGetDiagRec) X(SQLNumResultCols) X(SQLRowCount) X(SQLSetEnvAttr)

PG_FUNCS(PS_DL_PONTEIRO)
MY_FUNCS(PS_DL_PONTEIRO)
OD_FUNCS(PS_DL_PONTEIRO)

static int  g_pg_estado, g_my_estado, g_od_estado;
static char g_pg_motivo[320], g_my_motivo[320], g_od_motivo[320];
#if defined(_WIN32)
/* Windows: DLLs. O pacote do Windows leva as do MSYS2 (libpq, libmariadb) na
 * pasta do executável; o ODBC é o do próprio Windows. */
static const char *const PG_NOMES[] = { "libpq.dll", NULL };
static const char *const MY_NOMES[] = { "libmariadb.dll", NULL };
static const char *const OD_NOMES[] = { "odbc32.dll", NULL };
#elif defined(__APPLE__)
/* macOS: o nome da biblioteca é `.dylib`, e o Homebrew instala o libpq e o
 * cliente MariaDB fora do caminho padrão (keg-only / subpasta), então o
 * caminho inteiro entra na lista. /usr/local é o Homebrew do Mac Intel,
 * /opt/homebrew o do Apple Silicon. */
static const char *const PG_NOMES[] = {
    "libpq.5.dylib",
    "@executable_path/../lib/jinga/libpq.5.dylib",      /* o pacote (build_bundle.sh) */
    "/usr/local/opt/libpq/lib/libpq.5.dylib",
    "/opt/homebrew/opt/libpq/lib/libpq.5.dylib", NULL };
static const char *const MY_NOMES[] = {
    "libmariadb.3.dylib",
    "@executable_path/../lib/jinga/libmariadb.3.dylib",
    "/usr/local/lib/mariadb/libmariadb.3.dylib",
    "/opt/homebrew/lib/mariadb/libmariadb.3.dylib",
    "/usr/local/opt/mariadb-connector-c/lib/mariadb/libmariadb.3.dylib",
    "/opt/homebrew/opt/mariadb-connector-c/lib/mariadb/libmariadb.3.dylib", NULL };
static const char *const OD_NOMES[] = {
    "libodbc.2.dylib",
    "@executable_path/../lib/jinga/libodbc.2.dylib",
    "/usr/local/lib/libodbc.2.dylib",
    "/opt/homebrew/lib/libodbc.2.dylib", NULL };
#else
static const char *const PG_NOMES[] = { "libpq.so.5", "libpq.so", NULL };
static const char *const MY_NOMES[] = { "libmariadb.so.3", "libmysqlclient.so.21", "libmariadb.so", NULL };
static const char *const OD_NOMES[] = { "libodbc.so.2", "libodbc.so", NULL };
#endif

/* Daqui pra baixo, o nome da função é o ponteiro. */
#define PQclear dl_PQclear
#define PQcmdTuples dl_PQcmdTuples
#define PQconnectdbParams dl_PQconnectdbParams
#define PQerrorMessage dl_PQerrorMessage
#define PQexec dl_PQexec
#define PQexecParams dl_PQexecParams
#define PQfinish dl_PQfinish
#define PQfname dl_PQfname
#define PQftype dl_PQftype
#define PQgetisnull dl_PQgetisnull
#define PQgetvalue dl_PQgetvalue
#define PQnfields dl_PQnfields
#define PQntuples dl_PQntuples
#define PQresultErrorField dl_PQresultErrorField
#define PQresultStatus dl_PQresultStatus
#define PQstatus dl_PQstatus
#define mysql_affected_rows dl_mysql_affected_rows
#define mysql_close dl_mysql_close
#define mysql_error dl_mysql_error
#define mysql_fetch_fields dl_mysql_fetch_fields
#define mysql_fetch_lengths dl_mysql_fetch_lengths
#define mysql_fetch_row dl_mysql_fetch_row
#define mysql_field_count dl_mysql_field_count
#define mysql_free_result dl_mysql_free_result
#define mysql_init dl_mysql_init
#define mysql_num_fields dl_mysql_num_fields
#define mysql_num_rows dl_mysql_num_rows
#define mysql_real_connect dl_mysql_real_connect
#define mysql_real_escape_string dl_mysql_real_escape_string
#define mysql_real_query dl_mysql_real_query
#define mysql_store_result dl_mysql_store_result
#define SQLAllocHandle dl_SQLAllocHandle
#define SQLBindParameter dl_SQLBindParameter
#define SQLDescribeCol dl_SQLDescribeCol
#define SQLDisconnect dl_SQLDisconnect
#define SQLDriverConnect dl_SQLDriverConnect
#define SQLExecDirect dl_SQLExecDirect
#define SQLFetch dl_SQLFetch
#define SQLFreeHandle dl_SQLFreeHandle
#define SQLGetData dl_SQLGetData
#define SQLGetDiagRec dl_SQLGetDiagRec
#define SQLNumResultCols dl_SQLNumResultCols
#define SQLRowCount dl_SQLRowCount
#define SQLSetEnvAttr dl_SQLSetEnvAttr

struct PSDbConn {
    PSDbDriver drv;
    sqlite3   *sq;
    PGconn    *pg;
    MYSQL     *my;
    SQLHENV    od_env;
    SQLHDBC    od_dbc;
    int        fechado;
};

/* ── result buffer ──────────────────────────────────────────────────────── */
void ps_db_res_libera(PSDbRes *r)
{
    for (int i = 0; i < r->ncols; i++) free(r->cols[i]);
    free(r->cols);
    for (int i = 0; i < r->nlinhas; i++) {
        for (int c = 0; c < r->ncols; c++) free(r->linhas[i][c].txt);
        free(r->linhas[i]);
    }
    free(r->linhas);
    memset(r, 0, sizeof(*r));
}

static int res_nova_linha(PSDbRes *r, PSCel **out)
{
    if (r->nlinhas >= r->cap) {
        int nc = r->cap < 8 ? 8 : r->cap * 2;
        PSCel **nn = realloc(r->linhas, sizeof(PSCel *) * (size_t)nc);
        if (!nn) return -1;
        r->linhas = nn; r->cap = nc;
    }
    PSCel *lin = calloc((size_t)r->ncols, sizeof(PSCel));
    if (!lin) return -1;
    r->linhas[r->nlinhas++] = lin;
    *out = lin;
    return 0;
}

/* Reserva o vetor de nomes de coluna e grava um nome.
 *
 * Os quatro drivers (sqlite, postgres, mysql, odbc) faziam
 * `res->cols = calloc(...)` seguido de `res->cols[i] = strdup(...)` sem olhar
 * nenhum dos dois. Com memória curta o `calloc` devolve NULL e a linha
 * seguinte escreve nele — o processo morre em vez de o erro subir como "sem
 * memoria", e o `strdup` que já tinha dado certo vaza junto. Passa tudo por
 * aqui pra que seja um lugar só. */
static int res_cols_reserva(PSDbRes *res, int ncol, char *erro, size_t ecap)
{
    res->cols = calloc((size_t)ncol, sizeof(char *));
    if (!res->cols) { snprintf(erro, ecap, "sem memoria"); return -1; }
    return 0;
}

/* `nome` NULL vira string vazia: coluna sem nome é caso do banco, não erro. */
static int res_col_nome(PSDbRes *res, int i, const char *nome, char *erro, size_t ecap)
{
    res->cols[i] = strdup(nome ? nome : "");
    if (!res->cols[i]) { snprintf(erro, ecap, "sem memoria"); return -1; }
    return 0;
}

static void cel_texto(PSCel *cel, const char *s, int n)
{
    cel->tipo = PS_CEL_STR;
    cel->txt = malloc((size_t)n + 1);
    if (cel->txt) { memcpy(cel->txt, s, (size_t)n); cel->txt[n] = 0; cel->n = n; }
}
static void cel_int(PSCel *cel, int64_t v)   { cel->tipo = PS_CEL_INT; cel->i = v; char b[32]; snprintf(b,sizeof(b),"%lld",(long long)v); cel->txt = strdup(b); }
static void cel_float(PSCel *cel, double v)   { cel->tipo = PS_CEL_FLOAT; cel->f = v; char b[40]; snprintf(b,sizeof(b),"%g",v); cel->txt = strdup(b); }
static void cel_bool(PSCel *cel, int v)       { cel->tipo = PS_CEL_BOOL; cel->b = v; cel->txt = strdup(v?"true":"false"); }

/* ── sqlite ─────────────────────────────────────────────────────────────── */
/* O parâmetro com o tipo do valor, como o atalho `psodbc.query` já ligava: bool
 * e int viram inteiro, flo vira real, o resto texto. Tudo como texto, `True`
 * era 'True' e `WHERE ativo = ?` não casava com linha nenhuma. Inteiro que não
 * cabe em 64 bits (bignum) segue como texto. O texto vai com o tamanho `n`
 * (-1 = até o NUL): o texto da linguagem guarda o NUL, e o `"a\x00b"` que ia até
 * o NUL gravava `"a"` (motor-30). */
static void sqlite_liga(sqlite3_stmt *st, int i, const char *pv, int n, char tipo)
{
    if (!pv) { sqlite3_bind_null(st, i + 1); return; }
    if (tipo == 'b') { sqlite3_bind_int64(st, i + 1, (pv[0] == 't' || pv[0] == '1') ? 1 : 0); return; }
    if (tipo == 'i') {
        errno = 0;
        char *fim;
        long long v = strtoll(pv, &fim, 10);
        if (errno == 0 && *fim == '\0' && fim != pv) { sqlite3_bind_int64(st, i + 1, (sqlite3_int64)v); return; }
    } else if (tipo == 'f') {
        char *fim;
        double d = strtod(pv, &fim);
        if (*fim == '\0' && fim != pv) { sqlite3_bind_double(st, i + 1, d); return; }
    }
    sqlite3_bind_text(st, i + 1, pv, n, SQLITE_TRANSIENT);
}

/* O resto do SQL depois do primeiro comando, sem os brancos e os comentários
 * (o de linha, de `--` até o fim dela, e o de bloco): NULL quando não sobra
 * nada. É o `lstrip_sql` do `sqlite3` do CPython (Modules/_sqlite/statement.c),
 * com os mesmos brancos. */
const char *ps_db_sql_resto(const char *sql)
{
    for (const char *p = sql; *p; p++) {
        switch (*p) {
            case ' ': case '\t': case '\f': case '\n': case '\r':
                break;
            case '-':
                if (p[1] != '-') return p;
                p += 2;
                while (*p && *p != '\n') p++;
                if (!*p) return NULL;
                break;
            case '/':
                if (p[1] != '*') return p;
                p += 2;
                while (*p && (p[0] != '*' || p[1] != '/')) p++;
                if (!*p) return NULL;
                p++;
                break;
            default:
                return p;
        }
    }
    return NULL;
}

static int sqlite_exec(PSDbConn *c, const char *sql, const char **params, const int *lens, const char *tipos,
                       int nparams, PSDbRes *res, char *erro, size_t ecap, char *tipo_out, size_t tcap)
{
    if (tipo_out && tcap) snprintf(tipo_out, tcap, "DatabaseError");
    sqlite3_stmt *st = NULL;
    const char *resto = NULL;
    if (sqlite3_prepare_v2(c->sq, sql, -1, &st, &resto) != SQLITE_OK) {
        snprintf(erro, ecap, "erro de banco de dados: %s", sqlite3_errmsg(c->sq));
        return -1;
    }
    /* O `prepare` só lê o primeiro comando, e o resto era jogado fora: em
     * `DELETE FROM t WHERE id = 1; DELETE FROM t` só o primeiro rodava, calado
     * (motor-31). Como o `sqlite3` do Python, sobrou comando = erro, antes de
     * rodar qualquer um; a frase é a de lá. */
    if (resto && ps_db_sql_resto(resto)) {
        sqlite3_finalize(st);
        snprintf(erro, ecap, "You can only execute one statement at a time.");
        return -1;
    }
    for (int i = 0; i < nparams; i++)
        sqlite_liga(st, i, params[i], lens ? lens[i] : -1, tipos ? tipos[i] : 's');
    int ncol = sqlite3_column_count(st);
    if (ncol == 0) {                     /* DML/DDL */
        int rc = sqlite3_step(st);
        if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
            snprintf(erro, ecap, "erro de banco de dados: %s", sqlite3_errmsg(c->sq));
            sqlite3_finalize(st); return -1;
        }
        res->rowcount = sqlite3_changes(c->sq);
        res->tem_result = 0;
        sqlite3_finalize(st);
        return 0;
    }
    res->tem_result = 1;
    res->rowcount = -1;   /* sqlite3 DBAPI: rowcount de SELECT é -1 (igual interp) */
    res->ncols = ncol;
    if (res_cols_reserva(res, ncol, erro, ecap) != 0) { sqlite3_finalize(st); return -1; }
    for (int i = 0; i < ncol; i++)
        if (res_col_nome(res, i, sqlite3_column_name(st, i), erro, ecap) != 0) {
            sqlite3_finalize(st); return -1;
        }
    int rc;
    while ((rc = sqlite3_step(st)) == SQLITE_ROW) {
        PSCel *lin;
        if (res_nova_linha(res, &lin) != 0) { sqlite3_finalize(st); snprintf(erro,ecap,"sem memoria"); return -1; }
        for (int i = 0; i < ncol; i++) {
            switch (sqlite3_column_type(st, i)) {
                case SQLITE_INTEGER: cel_int(&lin[i], sqlite3_column_int64(st, i)); break;
                case SQLITE_FLOAT:   cel_float(&lin[i], sqlite3_column_double(st, i)); break;
                case SQLITE_NULL:    lin[i].tipo = PS_CEL_NULL; break;
                default: {
                    const unsigned char *t = sqlite3_column_text(st, i);
                    cel_texto(&lin[i], t ? (const char *)t : "", sqlite3_column_bytes(st, i));
                }
            }
        }
    }
    sqlite3_finalize(st);
    if (rc != SQLITE_DONE) { snprintf(erro,ecap,"erro de banco de dados: %s", sqlite3_errmsg(c->sq)); return -1; }
    return 0;
}

/* ── postgres ───────────────────────────────────────────────────────────── */
/* OIDs dos tipos numéricos do postgres (de pg_type.h — estáveis) */
#define PG_BOOL 16
#define PG_INT8 20
#define PG_INT2 21
#define PG_INT4 23
#define PG_FLOAT4 700
#define PG_FLOAT8 701
#define PG_NUMERIC 1700

static void pg_tipo_erro(PGresult *r, char *tipo_out, size_t tcap)
{
    if (!tipo_out || !tcap) return;
    const char *ss = PQresultErrorField(r, PG_DIAG_SQLSTATE);
    snprintf(tipo_out, tcap, "DatabaseError");
    if (!ss) return;
    for (int i = 0; i < PG_ERR_N; i++)
        if (strcmp(PG_ERR_NOMES[i].sqlstate, ss) == 0) { snprintf(tipo_out, tcap, "%s", PG_ERR_NOMES[i].nome); return; }
}

/* A árvore de exceções pergunta AQUI se um nome é de erro do banco, pra dar
 * `DatabaseError` como pai aos 251 de uma vez. Sem isto, `catch (DatabaseError
 * e)` não pegava `UniqueViolation` — e a MESMA violação de UNIQUE no sqlite já
 * dava `DatabaseError`, então o tipo mudava conforme o driver. */
int ps_db_eh_nome_erro(const char *nome)
{
    if (!nome || !*nome) return 0;
    for (int i = 0; i < PG_ERR_N; i++)
        if (strcmp(PG_ERR_NOMES[i].nome, nome) == 0) return 1;
    return 0;
}
/* ── placeholders: UMA regra pros três drivers que reescrevem a query ─────
 * `?` e `%s` são placeholders só FORA de literal entre aspas simples (`''` é
 * aspa escapada): `WHERE s = 'e ai?'` não consome parâmetro. A regra vivia só
 * no postgres; a cópia do mysql não pulava literal e um `?` dentro de texto
 * comia o parâmetro seguinte e corrompia a query. Cada driver só decide o que
 * escrever no lugar do marcador (`emite`): postgres `$k`, mysql o valor
 * escapado, odbc `?`.
 *
 * A query reescrita vai num texto que cresce. Era um buffer fixo (8 KB no
 * postgres, 16 KB no mysql e no odbc), e o SQL maior era recusado com "sql
 * grande demais": um `INSERT` em lote é uso válido (motor-4). */
typedef struct { char *b; size_t n, cap; } TxtCresce;

static int tc_reserva(TxtCresce *t, size_t mais)
{
    if (mais > SIZE_MAX - t->n - 1) return -1;
    if (t->n + mais + 1 <= t->cap) return 0;
    size_t nc = t->cap ? t->cap : 256;
    while (nc < t->n + mais + 1) {
        if (nc > SIZE_MAX / 2) return -1;
        nc *= 2;
    }
    char *nb = realloc(t->b, nc);
    if (!nb) return -1;
    t->b = nb; t->cap = nc;
    return 0;
}

static int tc_poe(TxtCresce *t, const char *s, size_t n)
{
    if (tc_reserva(t, n) != 0) return -1;
    memcpy(t->b + t->n, s, n);
    t->n += n;
    t->b[t->n] = '\0';
    return 0;
}

typedef int (*PhEmite)(void *ctx, int k, TxtCresce *out);

/* Devolve a query reescrita (malloc: quem chama libera), com o tamanho em
 * `*nout`; NULL é falta de memória. */
static char *sql_varre_placeholders(const char *sql, size_t *nout, PhEmite emite, void *ctx)
{
    TxtCresce t = { NULL, 0, 0 };
    if (tc_reserva(&t, strlen(sql)) != 0 || !t.b) return NULL;
    t.b[0] = '\0';
    int k = 0, em_texto = 0, ok = 1;
    for (const char *p = sql; *p && ok; p++) {
        if (*p == '\'') { em_texto = !em_texto; ok = tc_poe(&t, p, 1) == 0; continue; }
        int marca = em_texto ? 0 : (p[0] == '%' && p[1] == 's') ? 2 : (p[0] == '?') ? 1 : 0;
        if (!marca) { ok = tc_poe(&t, p, 1) == 0; continue; }
        ok = emite(ctx, k++, &t) == 0;
        if (marca == 2) p++;
    }
    if (!ok) { free(t.b); return NULL; }
    *nout = t.n;
    return t.b;
}

static int ph_pg(void *ctx, int k, TxtCresce *out)
{
    (void)ctx;
    char b[24];
    int n = snprintf(b, sizeof(b), "$%d", k + 1);
    return tc_poe(out, b, (size_t)n);
}

static int ph_odbc(void *ctx, int k, TxtCresce *out)
{
    (void)ctx; (void)k;
    return tc_poe(out, "?", 1);
}

static int pg_exec(PSDbConn *c, const char *sql, const char **params, const int *lens, int nparams,
                   PSDbRes *res, char *erro, size_t ecap, char *tipo_out, size_t tcap)
{
    PGresult *r;
    if (nparams > 0) {
        /* O parâmetro vai como texto do C, e o tipo text do postgres não guarda
         * NUL: com NUL no meio ele chegava cortado, calado (motor-30). A frase
         * é a do psycopg2. */
        for (int i = 0; i < nparams; i++)
            if (params[i] && lens && (size_t)lens[i] != strlen(params[i])) {
                snprintf(erro, ecap, "A string literal cannot contain NUL (0x00) characters.");
                return -1;
            }
        /* `?` vira $1,$2,... (o estilo do postgres); `%s` segue aceito */
        size_t nconv;
        char *conv = sql_varre_placeholders(sql, &nconv, ph_pg, NULL);
        if (!conv) {
            snprintf(erro, ecap, "sem memoria");
            if (tipo_out && tcap) snprintf(tipo_out, tcap, "MemoryError");
            return -1;
        }
        r = PQexecParams(c->pg, conv, nparams, NULL, params, NULL, NULL, 0);
        free(conv);
    } else {
        r = PQexec(c->pg, sql);
    }
    ExecStatusType st = PQresultStatus(r);
    if (st == PGRES_TUPLES_OK) {
        int ncol = PQnfields(r), nrow = PQntuples(r);
        res->tem_result = 1;
        res->rowcount = nrow;   /* rowcount de SELECT = nº de linhas */
        res->ncols = ncol;
        if (res_cols_reserva(res, ncol, erro, ecap) != 0) { PQclear(r); return -1; }
        for (int i = 0; i < ncol; i++)
            if (res_col_nome(res, i, PQfname(r, i), erro, ecap) != 0) { PQclear(r); return -1; }
        for (int row = 0; row < nrow; row++) {
            PSCel *lin;
            if (res_nova_linha(res, &lin) != 0) { PQclear(r); snprintf(erro,ecap,"sem memoria"); return -1; }
            for (int i = 0; i < ncol; i++) {
                if (PQgetisnull(r, row, i)) { lin[i].tipo = PS_CEL_NULL; continue; }
                const char *v = PQgetvalue(r, row, i);
                switch (PQftype(r, i)) {
                    case PG_INT2: case PG_INT4: case PG_INT8:
                        cel_int(&lin[i], strtoll(v, NULL, 10)); break;
                    case PG_FLOAT4: case PG_FLOAT8: case PG_NUMERIC:
                        cel_float(&lin[i], strtod(v, NULL)); break;
                    case PG_BOOL:
                        cel_bool(&lin[i], v[0] == 't'); break;
                    default:
                        cel_texto(&lin[i], v, (int)strlen(v));
                }
            }
        }
        PQclear(r);
        return 0;
    }
    if (st == PGRES_COMMAND_OK) {
        res->tem_result = 0;
        const char *aff = PQcmdTuples(r);
        res->rowcount = (aff && aff[0]) ? strtoll(aff, NULL, 10) : -1;
        PQclear(r);
        return 0;
    }
    snprintf(erro, ecap, "erro de banco de dados: %s", PQerrorMessage(c->pg));
    pg_tipo_erro(r, tipo_out, tcap);
    PQclear(r);
    return -1;
}

/* ── mysql ──────────────────────────────────────────────────────────────── */
/* tipos numéricos do MySQL (mysql_com.h): DECIMAL=0 TINY=1 SHORT=2 LONG=3
 * FLOAT=4 DOUBLE=5 LONGLONG=8 INT24=9 NEWDECIMAL=246 */
static int my_eh_int(enum enum_field_types t) {
    return t==MYSQL_TYPE_TINY||t==MYSQL_TYPE_SHORT||t==MYSQL_TYPE_LONG
         ||t==MYSQL_TYPE_LONGLONG||t==MYSQL_TYPE_INT24;
}
static int my_eh_float(enum enum_field_types t) {
    return t==MYSQL_TYPE_FLOAT||t==MYSQL_TYPE_DOUBLE
         ||t==MYSQL_TYPE_DECIMAL||t==MYSQL_TYPE_NEWDECIMAL;
}

typedef struct { MYSQL *my; const char **params; const int *lens; const char *tipos; int nparams; } PhMy;

/* Texto de número que a VM formatou (int ou flo): só dígito, sinal, ponto e
 * expoente. É o que pode ir SEM aspas pro mysql — `inf`/`nan` e bignum com
 * outro formato seguem entre aspas, como antes. */
static int eh_numero_literal(const char *s)
{
    if (!*s) return 0;
    for (const char *p = s; *p; p++)
        if (!((*p >= '0' && *p <= '9') || *p == '-' || *p == '+' || *p == '.' || *p == 'e' || *p == 'E'))
            return 0;
    return 1;
}

static int ph_mysql(void *ctx, int k, TxtCresce *out)
{
    PhMy *m = ctx;
    if (k >= m->nparams)              /* marcador sem valor: fica como veio */
        return tc_poe(out, "?", 1);
    const char *pv = m->params[k];
    if (!pv) return tc_poe(out, "NULL", 4);
    /* o mysql não tem placeholder no protocolo de texto: o valor entra na
     * query. Número e bool entram SEM aspas — entre aspas, `LIMIT '10'` era
     * erro de sintaxe, `'True'` virava 0 (casava com FALSE) e `'41' + 1` virava
     * float. O texto de número vem da própria VM, nunca do usuário. */
    char tipo = m->tipos ? m->tipos[k] : 's';
    if (tipo == 'b') {
        const char *s = (pv[0] == 't' || pv[0] == '1') ? "TRUE" : "FALSE";
        return tc_poe(out, s, strlen(s));
    }
    if ((tipo == 'i' || tipo == 'f') && eh_numero_literal(pv))
        return tc_poe(out, pv, strlen(pv));
    /* o tamanho vem de quem chama: o texto guarda o NUL, e o escape do mysql
     * o escreve como `\0` (motor-30) */
    size_t lv = m->lens ? (size_t)m->lens[k] : strlen(pv);
    if (lv > (SIZE_MAX - 3) / 2) return -1;
    if (tc_reserva(out, 2 * lv + 2) != 0) return -1;   /* o escape pode dobrar */
    out->b[out->n++] = '\'';
    unsigned long en = mysql_real_escape_string(m->my, out->b + out->n, pv, (unsigned long)lv);
    out->n += en;
    out->b[out->n++] = '\'';
    out->b[out->n] = '\0';
    return 0;
}

static int mysql_exec(PSDbConn *c, const char *sql, const char **params, const int *lens, const char *tipos,
                      int nparams, PSDbRes *res, char *erro, size_t ecap, char *tipo_out, size_t tcap)
{
    if (tipo_out && tcap) snprintf(tipo_out, tcap, "DatabaseError");
    /* monta a query final: cada `?`/`%s` fora de literal recebe o param
     * escapado (o mysql não tem placeholder nativo no protocolo de texto) */
    PhMy ctx = { c->my, params, lens, tipos, nparams };
    size_t j;
    char *q = sql_varre_placeholders(sql, &j, ph_mysql, &ctx);
    if (!q) {
        snprintf(erro, ecap, "sem memoria");
        if (tipo_out && tcap) snprintf(tipo_out, tcap, "MemoryError");
        return -1;
    }
    int rq = mysql_real_query(c->my, q, (unsigned long)j);
    free(q);
    if (rq != 0) {
        snprintf(erro, ecap, "erro de banco de dados: %s", mysql_error(c->my));
        return -1;
    }
    MYSQL_RES *r = mysql_store_result(c->my);
    if (!r) {
        if (mysql_field_count(c->my) == 0) {   /* DML */
            res->tem_result = 0;
            res->rowcount = (int64_t)mysql_affected_rows(c->my);
            return 0;
        }
        snprintf(erro, ecap, "erro de banco de dados: %s", mysql_error(c->my));
        return -1;
    }
    int ncol = (int)mysql_num_fields(r);
    MYSQL_FIELD *campos = mysql_fetch_fields(r);
    res->tem_result = 1;
    res->rowcount = (int64_t)mysql_num_rows(r);   /* SELECT bufferizado: nº de linhas */
    res->ncols = ncol;
    if (res_cols_reserva(res, ncol, erro, ecap) != 0) { mysql_free_result(r); return -1; }
    for (int i = 0; i < ncol; i++)
        if (res_col_nome(res, i, campos[i].name, erro, ecap) != 0) {
            mysql_free_result(r); return -1;
        }
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(r))) {
        unsigned long *tams = mysql_fetch_lengths(r);
        PSCel *lin;
        if (res_nova_linha(res, &lin) != 0) { mysql_free_result(r); snprintf(erro,ecap,"sem memoria"); return -1; }
        for (int i = 0; i < ncol; i++) {
            if (!row[i]) { lin[i].tipo = PS_CEL_NULL; continue; }
            if (my_eh_int(campos[i].type)) cel_int(&lin[i], strtoll(row[i], NULL, 10));
            else if (my_eh_float(campos[i].type)) cel_float(&lin[i], strtod(row[i], NULL));
            else cel_texto(&lin[i], row[i], (int)tams[i]);
        }
    }
    mysql_free_result(r);
    return 0;
}

/* ── odbc (mssql e qualquer driver ODBC) ────────────────────────────────── */
static void od_erro(SQLSMALLINT tipo, SQLHANDLE h, char *erro, size_t ecap)
{
    SQLCHAR est[6], msg[512]; SQLINTEGER nativo; SQLSMALLINT ml;
    if (SQLGetDiagRec(tipo, h, 1, est, &nativo, msg, sizeof(msg), &ml) == SQL_SUCCESS)
        snprintf(erro, ecap, "erro de banco de dados: %s", msg);
    else snprintf(erro, ecap, "erro de banco de dados");
}

/* O valor inteiro da coluna `col` da linha corrente, como texto. O SQLGetData
 * entrega em pedaços: enquanto o valor não cabe, ele escreve o que coube (com
 * o NUL no fim), avisa com SQL_SUCCESS_WITH_INFO, e a chamada seguinte continua
 * dali. Uma chamada só, num buffer de 4 KB, devolvia o valor maior cortado,
 * calado (motor-32). Devolve 1 pro NULL, 0 com o valor em `*out` (malloc) e -1
 * com erro do driver ou sem memória. */
static int od_le_coluna(SQLHSTMT st, SQLUSMALLINT col, char **out, size_t *nout)
{
    size_t cap = 4096, n = 0;
    char *b = malloc(cap);
    if (!b) return -1;
    for (;;) {
        size_t espaco = cap - n;                  /* conta o NUL que o driver põe */
        SQLLEN ind = 0;
        SQLRETURN rc = SQLGetData(st, col, SQL_C_CHAR, b + n, (SQLLEN)espaco, &ind);
        if (rc == SQL_NO_DATA) break;
        if (!SQL_SUCCEEDED(rc)) { free(b); return -1; }
        if (ind == SQL_NULL_DATA) { free(b); return 1; }
        if (ind != SQL_NO_TOTAL && ind >= 0 && (size_t)ind < espaco) {   /* coube: o último pedaço */
            n += (size_t)ind;
            break;
        }
        n += espaco - 1;                          /* não coube: veio o que cabia */
        size_t falta = (ind == SQL_NO_TOTAL || ind < 0) ? cap : (size_t)ind - (espaco - 1);
        size_t nc = n + falta + 1;
        if (nc < 2 * cap) nc = 2 * cap;
        char *nb = realloc(b, nc);
        if (!nb) { free(b); return -1; }
        b = nb; cap = nc;
    }
    b[n] = '\0';
    *out = b; *nout = n;
    return 0;
}

static int odbc_exec(PSDbConn *c, const char *sql, const char **params, const int *lens, int nparams,
                     PSDbRes *res, char *erro, size_t ecap, char *tipo_out, size_t tcap)
{
    if (tipo_out && tcap) snprintf(tipo_out, tcap, "DatabaseError");
    SQLHSTMT st;
    if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, c->od_dbc, &st))) { snprintf(erro,ecap,"erro de banco de dados: sem statement"); return -1; }
    /* liga params (?): como texto, o driver converte pro tipo da coluna. O
     * tamanho vai no indicador de cada um: sem ele (ponteiro nulo) o driver lê
     * até o NUL, e o texto com NUL no meio entrava cortado (motor-30). Os
     * indicadores vivem até o fim da execução, que é quando o driver os lê. */
    SQLLEN *ind = calloc((size_t)(nparams > 0 ? nparams : 1), sizeof(SQLLEN));
    if (!ind) {
        snprintf(erro, ecap, "sem memoria");
        if (tipo_out && tcap) snprintf(tipo_out, tcap, "MemoryError");
        SQLFreeHandle(SQL_HANDLE_STMT, st); return -1;
    }
    for (int i = 0; i < nparams; i++) {
        const char *v = params[i];
        ind[i] = v ? (SQLLEN)(lens ? (size_t)lens[i] : strlen(v)) : SQL_NULL_DATA;
        SQLBindParameter(st, (SQLUSMALLINT)(i+1), SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                         0, 0, (SQLPOINTER)(v ? v : ""), v ? ind[i] : 0, &ind[i]);
    }
    /* `%s` vira `?` (o placeholder do odbc); `?` dentro de literal fica */
    size_t nconv;
    char *conv = sql_varre_placeholders(sql, &nconv, ph_odbc, NULL);
    if (!conv) {
        snprintf(erro, ecap, "sem memoria");
        if (tipo_out && tcap) snprintf(tipo_out, tcap, "MemoryError");
        free(ind); SQLFreeHandle(SQL_HANDLE_STMT, st); return -1;
    }
    SQLRETURN xr = SQLExecDirect(st, (SQLCHAR *)conv, (SQLINTEGER)nconv);
    free(conv);
    if (!SQL_SUCCEEDED(xr)) {
        od_erro(SQL_HANDLE_STMT, st, erro, ecap);
        free(ind); SQLFreeHandle(SQL_HANDLE_STMT, st); return -1;
    }
    free(ind);
    SQLSMALLINT ncol = 0;
    SQLNumResultCols(st, &ncol);
    if (ncol == 0) {                      /* DML */
        SQLLEN aff = -1; SQLRowCount(st, &aff);
        res->tem_result = 0; res->rowcount = (int64_t)aff;
        SQLFreeHandle(SQL_HANDLE_STMT, st);
        return 0;
    }
    res->tem_result = 1; res->ncols = ncol;
    if (res_cols_reserva(res, ncol, erro, ecap) != 0) { SQLFreeHandle(SQL_HANDLE_STMT, st); return -1; }
    SQLSMALLINT *ctipo = calloc((size_t)ncol, sizeof(SQLSMALLINT));
    if (!ctipo) { SQLFreeHandle(SQL_HANDLE_STMT, st); snprintf(erro, ecap, "sem memoria"); return -1; }
    for (int i = 0; i < ncol; i++) {
        /* `nome` e `dt` são inicializados ANTES da chamada porque o
         * SQLDescribeCol só promete tê-los escrito quando devolve sucesso. Sem
         * isso, um SQL_ERROR mandava 128 bytes de pilha pro `strdup` — nome de
         * coluna formado por lixo, sem terminador garantido. */
        SQLCHAR nome[256] = {0}; SQLSMALLINT nl = 0, dt = SQL_UNKNOWN_TYPE, dd = 0, nul = 0;
        SQLULEN tam = 0;
        SQLRETURN dr = SQLDescribeCol(st, (SQLUSMALLINT)(i+1), nome, sizeof(nome),
                                      &nl, &dt, &tam, &dd, &nul);
        if (!SQL_SUCCEEDED(dr)) { nome[0] = 0; dt = SQL_UNKNOWN_TYPE; }
        /* nome que não coube: `nl` diz o tamanho inteiro, e a segunda chamada
         * pega ele todo (o buffer fixo cortava o nome) */
        char *grande = NULL;
        if (SQL_SUCCEEDED(dr) && nl >= (SQLSMALLINT)sizeof(nome)) {
            grande = calloc((size_t)nl + 1, 1);
            SQLSMALLINT nl2 = 0;
            if (grande && !SQL_SUCCEEDED(SQLDescribeCol(st, (SQLUSMALLINT)(i+1), (SQLCHAR *)grande,
                                                        (SQLSMALLINT)(nl + 1), &nl2, &dt, &tam, &dd, &nul))) {
                free(grande); grande = NULL;
            }
        }
        int rn = res_col_nome(res, i, grande ? grande : (char *)nome, erro, ecap);
        free(grande);
        if (rn != 0) {
            free(ctipo); SQLFreeHandle(SQL_HANDLE_STMT, st); return -1;
        }
        ctipo[i] = dt;
    }
    while (SQL_SUCCEEDED(SQLFetch(st))) {
        PSCel *lin;
        if (res_nova_linha(res, &lin) != 0) { free(ctipo); SQLFreeHandle(SQL_HANDLE_STMT, st); snprintf(erro,ecap,"sem memoria"); return -1; }
        for (int i = 0; i < ncol; i++) {
            /* erro do driver na leitura é erro, não NULL: o valor que não veio
             * virava NULL calado */
            char *v = NULL; size_t nv = 0;
            int lr = od_le_coluna(st, (SQLUSMALLINT)(i+1), &v, &nv);
            if (lr < 0) {
                od_erro(SQL_HANDLE_STMT, st, erro, ecap);
                free(ctipo); SQLFreeHandle(SQL_HANDLE_STMT, st); return -1;
            }
            if (lr == 1) { lin[i].tipo = PS_CEL_NULL; continue; }
            SQLSMALLINT dt = ctipo[i];
            if (dt==SQL_INTEGER||dt==SQL_SMALLINT||dt==SQL_TINYINT||dt==SQL_BIGINT)
                cel_int(&lin[i], strtoll(v, NULL, 10));
            else if (dt==SQL_REAL||dt==SQL_FLOAT||dt==SQL_DOUBLE||dt==SQL_DECIMAL||dt==SQL_NUMERIC)
                cel_float(&lin[i], strtod(v, NULL));
            else if (dt==SQL_BIT)
                cel_bool(&lin[i], v[0]=='1');
            else
                cel_texto(&lin[i], v, (int)nv);
            free(v);
        }
    }
    free(ctipo);
    SQLFreeHandle(SQL_HANDLE_STMT, st);
    return 0;
}

/* ── API ────────────────────────────────────────────────────────────────── */
PSDbConn *ps_db_conecta(PSDbDriver drv, const char *host, int porta,
                        const char *user, const char *senha, const char *db,
                        const char *base, char *erro, size_t ecap)
{
    PSDbConn *c = calloc(1, sizeof(PSDbConn));
    if (!c) { snprintf(erro, ecap, "sem memoria"); return NULL; }
    c->drv = drv;
    if (drv == PS_DB_SQLITE) {
        if (sqlite3_open(base && base[0] ? base : db, &c->sq) != SQLITE_OK) {
            snprintf(erro, ecap, "erro de banco de dados: %s", c->sq ? sqlite3_errmsg(c->sq) : "sem memoria");
            if (c->sq) sqlite3_close(c->sq);
            free(c); return NULL;
        }
        return c;
    }
    if (drv == PS_DB_POSTGRES) {
        PS_DL_CARREGA(g_pg_estado, g_pg_motivo, PG_NOMES, "o driver postgres", PG_FUNCS, erro, ecap);
        if (g_pg_estado != 1) { free(c); return NULL; }
        char pstr[16]; snprintf(pstr, sizeof(pstr), "%d", porta ? porta : 5432);
        const char *kw[6], *vl[6]; int k = 0;
        kw[k]="host"; vl[k++]= host && host[0] ? host : "localhost";
        kw[k]="port"; vl[k++]= pstr;
        if (user && user[0]) { kw[k]="user"; vl[k++]=user; }
        if (senha && senha[0]) { kw[k]="password"; vl[k++]=senha; }
        if (db && db[0]) { kw[k]="dbname"; vl[k++]=db; }
        kw[k]=NULL; vl[k]=NULL;
        c->pg = PQconnectdbParams(kw, vl, 0);
        if (PQstatus(c->pg) != CONNECTION_OK) {
            snprintf(erro, ecap, "falha de conexão: %s", PQerrorMessage(c->pg));
            PQfinish(c->pg); free(c); return NULL;
        }
        return c;
    }
    if (drv == PS_DB_MYSQL) {
        PS_DL_CARREGA(g_my_estado, g_my_motivo, MY_NOMES, "o driver mysql", MY_FUNCS, erro, ecap);
        if (g_my_estado != 1) { free(c); return NULL; }
        c->my = mysql_init(NULL);
        if (!c->my) { snprintf(erro, ecap, "sem memoria"); free(c); return NULL; }
        if (!mysql_real_connect(c->my, host && host[0] ? host : "localhost",
                                user ? user : "", senha ? senha : "",
                                db && db[0] ? db : NULL, porta ? porta : 3306, NULL, 0)) {
            snprintf(erro, ecap, "falha de conexão: %s", mysql_error(c->my));
            mysql_close(c->my); free(c); return NULL;
        }
        return c;
    }
    if (drv == PS_DB_MSSQL) {
        PS_DL_CARREGA(g_od_estado, g_od_motivo, OD_NOMES, "o driver odbc", OD_FUNCS, erro, ecap);
        if (g_od_estado != 1) { free(c); return NULL; }
        /* `base` carrega o connection string ODBC completo (o VM monta) */
        if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &c->od_env))) {
            snprintf(erro, ecap, "sem memoria (odbc)"); free(c); return NULL;
        }
        SQLSetEnvAttr(c->od_env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
        if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, c->od_env, &c->od_dbc))) {
            SQLFreeHandle(SQL_HANDLE_ENV, c->od_env); snprintf(erro, ecap, "sem memoria (odbc)"); free(c); return NULL;
        }
        SQLCHAR out[1024]; SQLSMALLINT outl;
        SQLRETURN rc = SQLDriverConnect(c->od_dbc, NULL, (SQLCHAR *)(base && base[0] ? base : db),
                                        SQL_NTS, out, sizeof(out), &outl, SQL_DRIVER_NOPROMPT);
        if (!SQL_SUCCEEDED(rc)) {
            od_erro(SQL_HANDLE_DBC, c->od_dbc, erro, ecap);
            /* falha de conexão vira NetworkError-ish, mas a msg já diz */
            SQLFreeHandle(SQL_HANDLE_DBC, c->od_dbc); SQLFreeHandle(SQL_HANDLE_ENV, c->od_env);
            free(c); return NULL;
        }
        return c;
    }
    snprintf(erro, ecap, "driver ainda nao suportado na VM");
    free(c);
    return NULL;
}

int ps_db_exec(PSDbConn *c, const char *sql, const char **params, const int *lens, const char *tipos,
               int nparams, PSDbRes *res, char *erro, size_t ecap, char *tipo_out, size_t tcap)
{
    memset(res, 0, sizeof(*res));
    res->rowcount = -1;
    if (tipo_out && tcap) snprintf(tipo_out, tcap, "DatabaseError");
    if (c->fechado) { snprintf(erro, ecap, "erro de banco de dados: conexao fechada"); return -1; }
    switch (c->drv) {
        case PS_DB_SQLITE:   return sqlite_exec(c, sql, params, lens, tipos, nparams, res, erro, ecap, tipo_out, tcap);
        case PS_DB_POSTGRES: return pg_exec(c, sql, params, lens, nparams, res, erro, ecap, tipo_out, tcap);
        case PS_DB_MYSQL:    return mysql_exec(c, sql, params, lens, tipos, nparams, res, erro, ecap, tipo_out, tcap);
        case PS_DB_MSSQL:    return odbc_exec(c, sql, params, lens, nparams, res, erro, ecap, tipo_out, tcap);
        default: snprintf(erro, ecap, "driver nao suportado"); return -1;
    }
}

void ps_db_fecha(PSDbConn *c)
{
    if (!c || c->fechado) return;
    if (c->sq) { sqlite3_close(c->sq); c->sq = NULL; }
    if (c->pg) { PQfinish(c->pg); c->pg = NULL; }
    if (c->my) { mysql_close(c->my); c->my = NULL; }
    if (c->od_dbc) { SQLDisconnect(c->od_dbc); SQLFreeHandle(SQL_HANDLE_DBC, c->od_dbc); c->od_dbc = NULL; }
    if (c->od_env) { SQLFreeHandle(SQL_HANDLE_ENV, c->od_env); c->od_env = NULL; }
    c->fechado = 1;
}

void ps_db_solta(PSDbConn *c)
{
    if (!c) return;
    ps_db_fecha(c);
    free(c);
}
