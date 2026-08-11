/*
 * psodbc SQL — ver ps_db.h. sqlite + postgres agora; mysql/odbc entram no
 * mesmo molde (o result buffer é o mesmo).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>
#include <libpq-fe.h>
#include <mysql.h>
#include <sql.h>
#include <sqlext.h>

#include "ps_db.h"
#include "ps_pgerr.h"

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

static void cel_texto(PSCel *cel, const char *s, int n)
{
    cel->tipo = PS_CEL_STR;
    cel->txt = malloc((size_t)n + 1);
    if (cel->txt) { memcpy(cel->txt, s, (size_t)n); cel->txt[n] = 0; }
}
static void cel_int(PSCel *cel, int64_t v)   { cel->tipo = PS_CEL_INT; cel->i = v; char b[32]; snprintf(b,sizeof(b),"%lld",(long long)v); cel->txt = strdup(b); }
static void cel_float(PSCel *cel, double v)   { cel->tipo = PS_CEL_FLOAT; cel->f = v; char b[40]; snprintf(b,sizeof(b),"%g",v); cel->txt = strdup(b); }
static void cel_bool(PSCel *cel, int v)       { cel->tipo = PS_CEL_BOOL; cel->b = v; cel->txt = strdup(v?"true":"false"); }

/* ── sqlite ─────────────────────────────────────────────────────────────── */
static int sqlite_exec(PSDbConn *c, const char *sql, const char **params, int nparams,
                       PSDbRes *res, char *erro, size_t ecap, char *tipo_out, size_t tcap)
{
    if (tipo_out && tcap) snprintf(tipo_out, tcap, "DatabaseError");
    sqlite3_stmt *st = NULL;
    if (sqlite3_prepare_v2(c->sq, sql, -1, &st, NULL) != SQLITE_OK) {
        snprintf(erro, ecap, "erro de banco de dados: %s", sqlite3_errmsg(c->sq));
        return -1;
    }
    for (int i = 0; i < nparams; i++) {
        if (params[i]) sqlite3_bind_text(st, i + 1, params[i], -1, SQLITE_TRANSIENT);
        else sqlite3_bind_null(st, i + 1);
    }
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
    res->cols = calloc((size_t)ncol, sizeof(char *));
    for (int i = 0; i < ncol; i++) res->cols[i] = strdup(sqlite3_column_name(st, i));
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
static int pg_exec(PSDbConn *c, const char *sql, const char **params, int nparams,
                   PSDbRes *res, char *erro, size_t ecap, char *tipo_out, size_t tcap)
{
    PGresult *r;
    if (nparams > 0) {
        /* troca cada `?` por $1,$2,... (o estilo do postgres) */
        char conv[8192]; int j = 0, k = 1;
        for (const char *p = sql; *p && j < (int)sizeof(conv) - 8; p++) {
            if (p[0] == '%' && p[1] == 's') { j += snprintf(conv + j, sizeof(conv) - j, "$%d", k++); p++; }
            else conv[j++] = *p;
        }
        conv[j] = 0;
        r = PQexecParams(c->pg, conv, nparams, NULL, params, NULL, NULL, 0);
    } else {
        r = PQexec(c->pg, sql);
    }
    ExecStatusType st = PQresultStatus(r);
    if (st == PGRES_TUPLES_OK) {
        int ncol = PQnfields(r), nrow = PQntuples(r);
        res->tem_result = 1;
        res->rowcount = nrow;   /* psycopg2: rowcount de SELECT = nº de linhas */
        res->ncols = ncol;
        res->cols = calloc((size_t)ncol, sizeof(char *));
        for (int i = 0; i < ncol; i++) res->cols[i] = strdup(PQfname(r, i));
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

static int mysql_exec(PSDbConn *c, const char *sql, const char **params, int nparams,
                      PSDbRes *res, char *erro, size_t ecap, char *tipo_out, size_t tcap)
{
    if (tipo_out && tcap) snprintf(tipo_out, tcap, "DatabaseError");
    /* monta a query final: substitui cada `%s`/`?` por o param escapado.
     * (o DbCursor troca `?`->`%s` pro mysql; aceito os dois.) */
    char q[16384]; int j = 0, k = 0;
    for (const char *p = sql; *p && j < (int)sizeof(q) - 4; p++) {
        int marca = (p[0]=='%' && p[1]=='s') ? 2 : (p[0]=='?') ? 1 : 0;
        if (marca && k < nparams) {
            if (marca == 2) p++;
            const char *pv = params[k++];
            if (!pv) { j += snprintf(q+j, sizeof(q)-j, "NULL"); }
            else {
                q[j++] = '\'';
                char esc[4096];
                unsigned long en = mysql_real_escape_string(c->my, esc, pv, strlen(pv));
                if (j + (int)en + 2 < (int)sizeof(q)) { memcpy(q+j, esc, en); j += en; }
                q[j++] = '\'';
            }
        } else q[j++] = *p;
    }
    q[j] = 0;
    if (mysql_real_query(c->my, q, (unsigned long)j) != 0) {
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
    res->cols = calloc((size_t)ncol, sizeof(char *));
    for (int i = 0; i < ncol; i++) res->cols[i] = strdup(campos[i].name);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(r))) {
        unsigned long *lens = mysql_fetch_lengths(r);
        PSCel *lin;
        if (res_nova_linha(res, &lin) != 0) { mysql_free_result(r); snprintf(erro,ecap,"sem memoria"); return -1; }
        for (int i = 0; i < ncol; i++) {
            if (!row[i]) { lin[i].tipo = PS_CEL_NULL; continue; }
            if (my_eh_int(campos[i].type)) cel_int(&lin[i], strtoll(row[i], NULL, 10));
            else if (my_eh_float(campos[i].type)) cel_float(&lin[i], strtod(row[i], NULL));
            else cel_texto(&lin[i], row[i], (int)lens[i]);
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

static int odbc_exec(PSDbConn *c, const char *sql, const char **params, int nparams,
                     PSDbRes *res, char *erro, size_t ecap, char *tipo_out, size_t tcap)
{
    if (tipo_out && tcap) snprintf(tipo_out, tcap, "DatabaseError");
    SQLHSTMT st;
    if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, c->od_dbc, &st))) { snprintf(erro,ecap,"erro de banco de dados: sem statement"); return -1; }
    /* liga params (?): como texto, o driver converte pro tipo da coluna */
    for (int i = 0; i < nparams; i++) {
        const char *v = params[i];
        SQLBindParameter(st, (SQLUSMALLINT)(i+1), SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                         0, 0, (SQLPOINTER)(v ? v : ""), v ? (SQLLEN)strlen(v) : 0,
                         v ? NULL : (SQLLEN[]){SQL_NULL_DATA});
    }
    if (!SQL_SUCCEEDED(SQLExecDirect(st, (SQLCHAR *)sql, SQL_NTS))) {
        od_erro(SQL_HANDLE_STMT, st, erro, ecap);
        SQLFreeHandle(SQL_HANDLE_STMT, st); return -1;
    }
    SQLSMALLINT ncol = 0;
    SQLNumResultCols(st, &ncol);
    if (ncol == 0) {                      /* DML */
        SQLLEN aff = -1; SQLRowCount(st, &aff);
        res->tem_result = 0; res->rowcount = (int64_t)aff;
        SQLFreeHandle(SQL_HANDLE_STMT, st);
        return 0;
    }
    res->tem_result = 1; res->ncols = ncol;
    res->cols = calloc((size_t)ncol, sizeof(char *));
    SQLSMALLINT *ctipo = calloc((size_t)ncol, sizeof(SQLSMALLINT));
    for (int i = 0; i < ncol; i++) {
        SQLCHAR nome[128]; SQLSMALLINT nl, dt, dd, nul; SQLULEN tam;
        SQLDescribeCol(st, (SQLUSMALLINT)(i+1), nome, sizeof(nome), &nl, &dt, &tam, &dd, &nul);
        res->cols[i] = strdup((char *)nome);
        ctipo[i] = dt;
    }
    while (SQL_SUCCEEDED(SQLFetch(st))) {
        PSCel *lin;
        if (res_nova_linha(res, &lin) != 0) { free(ctipo); SQLFreeHandle(SQL_HANDLE_STMT, st); snprintf(erro,ecap,"sem memoria"); return -1; }
        for (int i = 0; i < ncol; i++) {
            char buf[4096]; SQLLEN ind;
            if (!SQL_SUCCEEDED(SQLGetData(st, (SQLUSMALLINT)(i+1), SQL_C_CHAR, buf, sizeof(buf), &ind))
                    || ind == SQL_NULL_DATA) { lin[i].tipo = PS_CEL_NULL; continue; }
            SQLSMALLINT dt = ctipo[i];
            if (dt==SQL_INTEGER||dt==SQL_SMALLINT||dt==SQL_TINYINT||dt==SQL_BIGINT)
                cel_int(&lin[i], strtoll(buf, NULL, 10));
            else if (dt==SQL_REAL||dt==SQL_FLOAT||dt==SQL_DOUBLE||dt==SQL_DECIMAL||dt==SQL_NUMERIC)
                cel_float(&lin[i], strtod(buf, NULL));
            else if (dt==SQL_BIT)
                cel_bool(&lin[i], buf[0]=='1');
            else
                cel_texto(&lin[i], buf, (int)(ind >= 0 && ind < (SQLLEN)sizeof(buf) ? (size_t)ind : strlen(buf)));
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

int ps_db_exec(PSDbConn *c, const char *sql, const char **params, int nparams,
               PSDbRes *res, char *erro, size_t ecap, char *tipo_out, size_t tcap)
{
    memset(res, 0, sizeof(*res));
    res->rowcount = -1;
    if (tipo_out && tcap) snprintf(tipo_out, tcap, "DatabaseError");
    if (c->fechado) { snprintf(erro, ecap, "erro de banco de dados: conexao fechada"); return -1; }
    switch (c->drv) {
        case PS_DB_SQLITE:   return sqlite_exec(c, sql, params, nparams, res, erro, ecap, tipo_out, tcap);
        case PS_DB_POSTGRES: return pg_exec(c, sql, params, nparams, res, erro, ecap, tipo_out, tcap);
        case PS_DB_MYSQL:    return mysql_exec(c, sql, params, nparams, res, erro, ecap, tipo_out, tcap);
        case PS_DB_MSSQL:    return odbc_exec(c, sql, params, nparams, res, erro, ecap, tipo_out, tcap);
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
